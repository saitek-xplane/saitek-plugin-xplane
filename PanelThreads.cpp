// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <cstring>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>

#include "pport.h"
#include "ptypes.h"
#include "pstreams.h"

#include "XPLMDefs.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

//#include "overloaded.h"
#include "defs.h"
#include "utils.h"
#include "hidapi.h"
#include "PanelThreads.h"
#include "multipanel.h"
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
jobqueue    gRp_sjq;

// Multi Panel resources
jobqueue    gMp_ijq;
jobqueue    gMp_ojq;
jobqueue    gMp_sjq;

// Switch Panel resources
jobqueue    gSp_ijq;
jobqueue    gSp_ojq;
jobqueue    gSp_sjq;

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

    pexchange((void**)dev, (void*)hid_open(VENDOR_ID, prod_id, NULL));

    if (*dev) {
        return true;
    }

    return false;
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


/**
 *
 */
void FromPanelThread::execute() {

    switch(mProduct) {
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
        mState->wait();

        mTmp = 0;

        if (!hid) {
            psleep(100); // what's a good timeout?
            continue;
        }

        // TODO: use hid_read_timeout?
        if ((mRes = hid_read((hid_device*)hid, (uint8_t*)&mTmp, sizeof(uint32_t))) <= 0) {
            if (mRes == HID_ERROR) {
                // TODO: log error
            }
        }

        (this->*proc_msg)(mTmp);
    }

    DPRINTF("Saitek ProPanels Plugin: FromPanelThread goodbye\n");
}


/**
 *
 */
void FromPanelThread::rp_processing(uint32_t msg) {

}


/**
 *
 */
void FromPanelThread::mp_processing(uint32_t msg) {

    //static char tmp[100];

    uint32_t btns = msg & READ_BTNS_MASK;
    uint32_t flaps =  msg & READ_FLAPS_MASK;
    uint32_t trim =  msg & READ_TRIM_MASK;
    uint32_t tuning =  msg & READ_TUNING_MASK;

    // When not an exclusive event, the state
    // is included with the previous events
    uint32_t knob = msg & READ_KNOB_MODE_MASK;

    // When not an exclusive event, the state
    // is included with all other events
    uint32_t autothrottle =  msg & READ_THROTTLE_MASK;

    uint32_t* x;

    //sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%.8X \n", msg);
    //DPRINTF(tmp);

    msg = 0;

    if (btns) {
        switch(btns) {
        case READ_AP_BTN:
            msg = BTN_AP_TOGGLE;
            break;
        case READ_HDG_BTN:
            msg = BTN_HDG_TOGGLE;
            break;
        case READ_NAV_BTN:
            msg = BTN_NAV_TOGGLE;
            break;
        case READ_IAS_BTN:
            msg = BTN_IAS_TOGGLE;
            break;
        case READ_ALT_BTN:
            msg = BTN_ALT_TOGGLE;
            break;
        case READ_VS_BTN:
            msg = BTN_VS_TOGGLE;
            break;
       case READ_APR_BTN:
           msg = BTN_APR_TOGGLE;
            break;
        case READ_REV_BTN:
            msg = BTN_REV_TOGGLE;
            break;
        default:
            // TODO: log error
            break;
        }
    } else if (flaps) {
        if (flaps == READ_FLAPS_UP) {
            msg = FLAPS_UP;
        } else if (flaps == READ_FLAPS_DN) {
            msg = FLAPS_DN;
        } else {
            // TODO: log error
        }
    } else if (trim) {
// TODO: fine & coarse grained adjustment
        if (trim == READ_TRIM_UP) {
            msg = PITCHTRIM_UP;
        } else if (trim == READ_TRIM_DOWN) {
            msg = PITCHTRIM_DN;
        } else {
            // TODO: log error
        }
    } else if (tuning) {
// TODO: fine & coarse grained adjustment
        if (tuning == READ_TUNING_RIGHT) {
            msg = READ_TUNING_RIGHT;
        } else if (tuning == READ_TUNING_LEFT) {
            msg = READ_TUNING_LEFT;
        } else {
            // TODO: log error
        }
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));
        msg = 0;
    }

    if (knob) {
        switch(knob) {
        case READ_KNOB_ALT:
            msg = KNOB_ALT_POS;
            break;
        case READ_KNOB_VS:
            msg = KNOB_VS_POS;
            break;
        case READ_KNOB_IAS:
            msg = KNOB_IAS_POS;
            break;
        case READ_KNOB_HDG:
            msg = KNOB_HDG_POS;
            break;
        case READ_KNOB_CRS:
            msg = KNOB_CRS_POS;
            break;
        default:
            // TODO: log error
            break;
        }
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));
    }

    if (autothrottle) {
        msg = AUTOTHROTTLE_ON;
    } else {
        msg = AUTOTHROTTLE_OFF;
    }

    x = new uint32_t;
    *x = msg;
    ijq->post(new myjob(x));
}


/**
 *
 */
void FromPanelThread::sp_processing(uint32_t msg) {

}


/**
 *
 */
void ToPanelThread::execute() {

    message* msg;
    uint32_t x;

    switch(mProduct) {
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

//    memset(mBuf, 0, OUT_BUF_CNT);

    while (threads_run) {
        mState->wait();

        // TODO: figure out the best sleep time!
//        psleep(100);

        msg = ojq->getmessage(MSG_WAIT);

        if (msg) {
            x = *((myjob*) msg)->buf;

            if (x == EXITING_THREAD_LOOP) {
                delete msg;
                break;
            }

            (this->*proc_msg)(x);
            delete msg;
        }

#if 0
        msg = sjq->getmessage(MSG_NOWAIT);

        if (msg) {
            (this->*proc_msg)(*((myjob*) msg)->buf);
            delete msg;
        }
#endif
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

    size_t cnt = 0;
    unsigned char* p;

// TODO: state information?

    switch(msg) {
    case AVIONICS_ON:
        if (!mAvionicsOn) {
            mAvionicsOn = true;
            if (mBat1On) {
//              hid_send_feature_report((hid_device*)hid, , sizeof());
            }
        }
        return;
    case AVIONICS_OFF:
        if (mAvionicsOn) {
            mAvionicsOn = false;
            if (!mBat1On) {
                cnt = sizeof(mp_hid_blank_panel);
                p = (unsigned char*)mp_hid_blank_panel;
                hid_send_feature_report((hid_device*)hid, p, sizeof(mp_hid_blank_panel));
            }
        }
        return;
    case BAT1_ON:
        if (!mBat1On) {
            mBat1On = true;
            if (mAvionicsOn) {
//            hid_send_feature_report((hid_device*)hid, , sizeof());
            }
        }
        return;
    case BAT1_OFF:
        if (mBat1On) {
            mBat1On = false;
            if (!mAvionicsOn) {
                cnt = sizeof(mp_hid_blank_panel);
                p = (unsigned char*)mp_hid_blank_panel;
                hid_send_feature_report((hid_device*)hid, p, sizeof(mp_hid_blank_panel));
            }
        }
        return;
    default:
        break;
    }

    if (mAvionicsOn && mBat1On) {
        switch(msg) {
        case BTN_AP_TOGGLE:
            mBtns.ap = ~mBtns.ap;
            break;
        case BTN_HDG_TOGGLE:
            mBtns.ap = ~mBtns.ap;
            break;
        case BTN_NAV_TOGGLE:
            mBtns.ap = ~mBtns.ap;
            break;
        case BTN_IAS_TOGGLE:
            mBtns.ap = ~mBtns.ias;
            break;
        case BTN_ALT_TOGGLE:
            mBtns.ap = ~mBtns.alt;
            break;
        case BTN_VS_TOGGLE:
            mBtns.ap = ~mBtns.vs;
            break;
        case BTN_APR_TOGGLE:
            mBtns.ap = ~mBtns.apr;
            break;
        case BTN_REV_TOGGLE:
            mBtns.ap = ~mBtns.rev;
            break;
        default:
            // TODO: log error
            break;
        }

// TODO: button logic
    }

//    if (cnt && p)
//        hid_send_feature_report((hid_device*)hid, p, cnt);

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

