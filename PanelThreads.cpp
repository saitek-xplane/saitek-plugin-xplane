// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <cstring>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "pport.h"
#include "ptypes.h"
#include "pstreams.h"

#include "XPLMDefs.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

#include "defs.h"
#include "PanelThreads.h"
#include "multipanel.h"
#include "utils.h"
#include "nedmalloc.h"
#include "overloaded.h"
#include "hidapi.h"
#include "SaitekProPanels.h"

/*
  index
  -----
    0: NA
    1-5 : top LEDs
    6-10: bottom LEDs
    11  : push buttons
                        bit positions
                        -------------
                7   6   5   4   3   2   1   0
               REV APR  VS ALT IAS NAV HDG  AP

Multi Panel messages:
 - 1 message when mode switch is turned
 - 1 message for autothrottle toggle switch
 - 2 messages for flap switch
 - 2 messages for a button press
*/

USING_PTYPES

enum {
    LED1_BYTE_START = 1, // top
    LED1_BYTE_CNT   = 5, // bottom

    LED2_BYTE_START = 6,
    LED2_BYTE_CNT   = 5,

    BTNS_BYTE_INDEX = 11,
    BTNS_BYTE_CNT   = 1,

    AP_BIT_POS      = 0,
    HDG_BIT_POS     = 1,
    NAV_BIT_POS     = 2,
    IAS_BIT_POS     = 3,
    ALT_BIT_POS     = 4,
    VS_BIT_POS      = 5,
    APR_BIT_POS     = 6,
    REV_BIT_POS     = 7,

    MINUS_SIGN      = 0x0E,
};

//static void toggle_bit(unsigned char* c, int pos);

const int MSG_MYJOB         = MSG_USER + 1;
int volatile pc_run         = false;
int volatile threads_run    = false;

// panel threads
hid_device *volatile gRpHandle = NULL;
hid_device *volatile gMpHandle = NULL;
hid_device *volatile gSpHandle = NULL;

// index[0] - report ID, which is always zero
// TODO: radio panel message
const unsigned char rp_hid_blank_panel[13] = {};

const unsigned char mp_hid_blank_panel[13] = {0x00, 0x0A, 0x0A, 0x0A, 0x0A,
                                              0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                                              0x0A, 0x00, 0x00};

// TODO: switch panel message
const unsigned char sp_hid_blank_panel[13] = {};

trigger     gPcTrigger(true, false);
trigger     gRpTrigger(false, false);
trigger     gMpTrigger(false, false);
trigger     gSpTrigger(false, false);

// Radio Panel resources
jobqueue    gRp_ojq;
jobqueue    gRp_ijq;

// Multi Panel resources
jobqueue    gMp_ijq;
jobqueue    gMp_ojq;

// Switch Panel resources
jobqueue    gSp_ijq;
jobqueue    gSp_ojq;

/**
 *
 */
void close_hid(hid_device* dev) {

// TODO: flush the queues?
    if (dev) {
        if (dev == gRpHandle) {
            hid_send_feature_report(gRpHandle, rp_hid_blank_panel, sizeof(rp_hid_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gRpHandle), NULL);
            gRpTrigger.reset();
        } else if (dev == gMpHandle) {
            hid_send_feature_report(gMpHandle, mp_hid_blank_panel, sizeof(mp_hid_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gMpHandle), NULL);
            gMpTrigger.reset();
        } else if (dev == gSpHandle) {
            hid_send_feature_report(gSpHandle, sp_hid_blank_panel, sizeof(sp_hid_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gSpHandle), NULL);
            gSpTrigger.reset();
        }
    }
}

/**
 *
 */
bool init_hid(hid_device* volatile* dev, unsigned short prod_id) {

//    pexchange((void**)dev, (void*)hid_open(&close_hid, VENDOR_ID, prod_id, NULL));
    pexchange((void**)dev, (void*)hid_open(VENDOR_ID, prod_id, NULL));

    if (*dev) {
        return true;
    }

    return false;
}

/**
 *
 */
void rp_init(hid_device* hid) {

    DPRINTF("Saitek ProPanels Plugin: rp_init\n");

}

/*
    000001   Autothrottle Engage
    000002   Heading Hold Engage
    000004   Wing Leveler Engage
    000008   Airspeed Hold With Pitch Engage
    000010   VVI Climb Engage
    000020   Altitude Hold Arm
    000040   Flight Level Change Engage
    000080   Pitch Sync Engage
    000100   HNAV Armed
    000200   HNAV Engaged
    000400   Glideslope Armed
    000800   Glideslope Engaged
    001000   FMS Armed
    002000   FMS Enaged
    004000   Altitude Hold Engaged
    008000   Horizontal TOGA Engaged
    010000   Vertical TOGA Engaged
    020000   VNAV Armed
    040000   VNAV Engaged

gMpALT
gMpVS
gMpVSSign
gMpIAS
gMpHDG
gMpCRS

      ALT
    0 - 5 | 6 - 10 | 11

sim/cockpit2/autopilot/altitude_dial_ft	            float	y	feet	VVI commanded in FPM.
sim/cockpit2/autopilot/altitude_vnav_ft	            float	n	feet	Target altitude hold in VNAV mode.
sim/cockpit2/autopilot/airspeed_is_mach	            int	y	boolean	Autopilot airspeed is Mach number rather than knots.
sim/cockpit2/autopilot/alt_vvi_is_showing_vvi       int	y	boolean	Is the combined alt/vvi selector showing VVI?

sim/cockpit2/autopilot/altitude_hold_ft	            float	n	feet	Altitude hold commanded in feet indicated.
sim/cockpit2/autopilot/vvi_dial_fpm	                float	y	feet/minute	VVI commanded in FPM.
sim/cockpit2/autopilot/airspeed_dial_kts_mach   	    float	y	knots/mach	Airspeed hold value, knots or Mach depending on km_is_mach.
sim/cockpit2/autopilot/heading_dial_deg_mag_pilot	    float	y	degrees_magnetic	Heading hold commanded, in degrees magnetic.


extern unsigned int gMpALT;
extern unsigned int gMpVS;
extern unsigned int gMpVSSign;
extern unsigned int gMpIAS;
extern unsigned int gMpHDG;
extern unsigned int gMpCRS;

extern XPLMDataRef      gApAltHoldRef;
extern XPLMDataRef      gApVsHoldRef;
extern XPLMDataRef      gApIasHoldRef;
extern XPLMDataRef      gApHdgHoldRef;
extern XPLMDataRef      gApCrsHoldRef;

*/

/*
 *
 */
void mp_init(hid_device* hid) {

    DPRINTF("Saitek ProPanels Plugin: mp_init\n");

    float tmp;
    uint8_t buf[4];

// XPLMGetDatai(gApStateRef)

    hid_set_nonblocking(hid, 1);
    hid_read(hid, buf, sizeof(buf));
    hid_send_feature_report(hid, mp_hid_blank_panel, sizeof(mp_hid_blank_panel));
    hid_set_nonblocking(hid, 0);

//    int res;

//    if((hid_get_feature_report(hid, buf, 13)) > 0) {
//        pexchange((int*) &gMpKnobPosition, buf[0] & 0x1F);
//        pexchange((int*) &gMpAutothrottleState, (buf[1] >> 7) & 0x01);
//    }

//    gMpALT = (unsigned int) XPLMGetDataf(gApAltHoldRef);
//    tmp = (unsigned int) XPLMGetDataf(gApVsHoldRef);
//    gMpVS = (unsigned int) fabs(tmp);

//    if (tmp < 0.0)
//        gMpVSSign = 1;
//    else
//        gMpVSSign = 0;

//pexchange((int*) &gMpKnobPosition, buf[0] & 0x1F);
//    gMpIAS = (int) XPLMGetDataf(gApIasHoldRef);
//    gMpHDG = (int) XPLMGetDataf(gApCrsHoldRef);
//    gMpCRS = (int)XPLMGetDataf(gApCrsHoldRef);


//    switch () {
//        case :
//            break;
//        default:
//            break;
//    }

//    hid_send_feature_report(hid, mp_hid_blank_panel, sizeof(mp_hid_blank_panel));
}

/**
 *
 */
void sp_init(hid_device* hid) {

    DPRINTF("Saitek ProPanels Plugin: sp_init\n");
}

/**
 *
 */
void FromPanelThread::execute() {

    switch(product) {
    case RP_PROD_ID:
        proc_msg = &FromPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        proc_msg = &FromPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        proc_msg = &FromPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        break;
    }

    while (threads_run) {
        state->wait();

        // check if there's a panel to process
        if (!hid) {
            psleep(100); // what's a good timeout (milliseconds)?
            continue;
        }

        tmp = 0;
        if ((res = hid_read((hid_device*)hid, (uint8_t*)&tmp, sizeof(uint32_t))) <= 0) {
            if (res == HID_ERROR) {
                // TODO: log error
            }
//            if (res == HID_DISCONNECTED)
//                psleep(100); // what's a good timeout (milliseconds)?
            continue;
        }

        tmp = (this->*proc_msg)(tmp);

        // if (tmp) {
        //     uint32_t* pbuf2 = (uint32_t*) calloc(1, sizeof(uint32_t));
        //     *pbuf2 = 0;
        //     hid_get_feature_report((hid_device*)hid, (uint8_t*)pbuf2, sizeof(uint32_t));

        //     uint32_t* pbuf = (uint32_t*) calloc(1, sizeof(uint32_t));

        // if (!pbuf) {
        //     DPRINTF("Saitek ProPanels Plugin: FromPanelThread null pbuf allocation!\n");
        //     continue;
        // }

        // *pbuf = tmp;
        // ijq->post(new myjob(pbuf));
        // ijq->post(new myjob(pbuf2));

// can send a message to the ToPanelThread
//    ojq->post(new myjob(x));
    }

    DPRINTF("Saitek ProPanels Plugin: FromPanelThread goodbye\n");
}


/**
 *
 */
uint32_t FromPanelThread::rp_processing(uint32_t msg) {

    return msg;
}


/**
 *
 */
uint32_t FromPanelThread::mp_processing(uint32_t msg) {

// Knob chnaging
// button pressed
// tuning
// auto throttle toggle
// flaps
// trim

// parse data, update saved state, send msg to x-plane, send feature report

    return msg;
}

/**
 *
 */
uint32_t FromPanelThread::sp_processing(uint32_t msg) {

    return msg;
}


/**
 *
 */
void ToPanelThread::execute() {

    message* msg;
    uint32_t x;

    switch(product) {
    case RP_PROD_ID:
        proc_msg = &ToPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        proc_msg = &ToPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        proc_msg = &ToPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        break;
    }

    memset(buf, 0, OUT_BUF_CNT);

    while (threads_run) {
        state->wait();

        msg = ojq->getmessage(MSG_WAIT);

        x = *((myjob*) msg)->buf;

        if (x == EXITING_THREAD_LOOP) {
            delete msg;
            break;
        }
// TODO: processing?
        (this->*proc_msg)(x);

        delete msg;

//        toggle_bit(&buf[BTNS_BYTE_INDEX], AP_BIT_POS);

//        if (hid) {
//            res = hid_send_feature_report((hid_device*)hid, buf, OUT_BUF_CNT);

//            if (res == HID_DISCONNECTED)
//                psleep(100);
//        }

//        if (msg) {
//        u8_in_buf   = ((myjob*) msg)->buf;
//            hid_send_feature_report(hid, outBuf, OUT_BUF_CNT);

//            delete msg;
//        }
    }

    DPRINTF("Saitek ProPanels Plugin: ToPanelThread goodbye\n");
}


/**
 *
 */
void ToPanelThread::rp_processing(uint32_t msg) {

}


/**
 *
 */
void ToPanelThread::mp_processing(uint32_t msg) {

    switch(msg) {
    case MP_PANEL_BLANK:
        hid_send_feature_report((hid_device*)hid, mp_hid_blank_panel, sizeof(mp_hid_blank_panel));
        break;
    // case :
    //     break;
    // case :
    //     break;
    // case :
    //     break;
    // case :
    //     break;
    default:
        break;
    }

}

/**
 *
 */
void ToPanelThread::sp_processing(uint32_t msg) {

}


/**
 *
 */
void PanelsCheckThread::execute() {

    pexchange((int*)&pc_run, true);
#ifndef NO_PANEL_CHECK
    void* p;
#endif

// TODO: flush the queues during a pend
    while (pc_run) {

        gPcTrigger.wait();

        if (!pc_run)
            break;

#ifndef NO_PANEL_CHECK
        if (!gRpHandle) {
            if (hid_check(VENDOR_ID, RP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, RP_PROD_ID, NULL);

                if (p) {
                    pexchange((void**)&gRpHandle, p);
                    hid_send_feature_report((hid_device*)gRpHandle, hid_open_msg, sizeof(hid_open_msg));
                    gRpTrigger.post();
                }
            }
        }

        if (!gMpHandle) {
            //XPLMSpeakString("one");
            if (hid_check(VENDOR_ID, MP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, MP_PROD_ID, NULL);
                if (p) {
                    //XPLMSpeakString("two");
                    pexchange((void**)&gMpHandle, p);
                    hid_send_feature_report((hid_device*)gMpHandle, hid_open_msg, sizeof(hid_open_msg));
                    gMpTrigger.post();
                }
            }
        }

        if (!gSpHandle) {
            if (hid_check(VENDOR_ID, SP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, SP_PROD_ID, NULL);

                if (p) {
                    pexchange((void**)&gSpHandle, p);
                    hid_send_feature_report((hid_device*)gSpHandle, hid_open_msg, sizeof(hid_open_msg));
                    gSpTrigger.post();
                }
            }
        }
#endif
    }

    DPRINTF("Saitek ProPanels Plugin: PanelsCheckThread goodbye\n");
}

