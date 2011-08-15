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

#include "XPLMUtilities.h"

#include "defs.h"
#include "multipanel.h"
#include "utils.h"
#include "PanelThreads.h"
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

const unsigned char hid_init_msg[13] = {0x00, 0x0a, 0x0a, 0x0a, 0x0a,
                                        0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
                                        0x0a, 0x00, 0x00};

const unsigned char hid_close_msg[13] = {0x00, 0x0a, 0x0a, 0x0a, 0x0a,
                                         0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
                                         0x0a, 0x00, 0x00};

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

void close_hid(hid_device* dev) {

// TODO: queues flushed!
    if (dev) {
        hid_close(dev);

        if (dev == gRpHandle) {
            pexchange((void**)(&gRpHandle), NULL);
            gRpTrigger.reset();
        } else if (dev == gMpHandle) {
            pexchange((void**)(&gMpHandle), NULL);
            gMpTrigger.reset();
        } else if (dev == gSpHandle) {
            pexchange((void**)(&gSpHandle), NULL);
            gSpTrigger.reset();
        } else {
            // ???
        }
    }
}

bool init_hid(hid_device* volatile* dev, unsigned short prod_id) {

    pexchange((void**)dev, (void*)hid_open(&close_hid, VENDOR_ID, prod_id, NULL));

    if (*dev) {
        return true;
    }

    return false;
}

void rp_init(hid_device* hid, int state) {

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
void mp_init(hid_device* hid, int state) {

    unsigned char buf[13];
    float tmp;
//    int res;

    if((hid_get_feature_report(hid, buf, 13)) > 0) {
        pexchange((int*) &gMpKnobPosition, buf[0] & 0x1F);
        pexchange((int*) &gMpAutothrottleState, (buf[1] >> 7) & 0x01);
    }

    gMpALT = (unsigned int) XPLMGetDataf(gApAltHoldRef);
    tmp = (unsigned int) XPLMGetDataf(gApVsHoldRef);
//    gMpVS = (unsigned int) fabs(tmp);

    if (tmp < 0.0)
        gMpVSSign = 1;
    else
        gMpVSSign = 0;

pexchange((int*) &gMpKnobPosition, buf[0] & 0x1F);
    gMpIAS = (int) XPLMGetDataf(gApIasHoldRef);
    gMpHDG = (int) XPLMGetDataf(gApCrsHoldRef);
//    gMpCRS = (int)XPLMGetDataf(gApCrsHoldRef);


//    switch () {
//        case :
//            break;
//        default:
//            break;
//    }

    hid_send_feature_report(hid, hid_init_msg, OUT_BUF_CNT);
}

void wp_init(hid_device* hid, int state) {

}

/**
 *
 */
void FromPanelThread::execute() {

    unsigned char* x;

    while (threads_run) {
        state->wait();

        // check if there's a panel to process
        if (!hid) {
            psleep(100); // what's a good timeout (milliseconds)?
            continue;
        }

        memset(buf, 0, IN_BUF_CNT);

        // check for data to process
        if ((res = hid_read((hid_device*)hid, buf, HID_READ_CNT)) <= 0) {
            if (res == HID_DISCONNECTED)
                //XPLMSpeakString("disconnected");
                psleep(100); // what's a good timeout (milliseconds)?
            continue;
        }

        x = procData(*((unsigned int*)buf));

        if (x) {
            ojq->post(new myjob(x));
        }

//    ijq->post(new myjob(x));

//        message* msg = ojq->getmessage(MSG_NOWAIT);

//        if (msg) {
//        u8_in_buf   = ((myjob*) msg)->buf;
//            hid_send_feature_report(hid, outBuf, OUT_BUF_CNT);

//            delete msg;
//        }

//        psleep(5000);
    }

    DPRINTF("Saitek ProPanels Plugin: from panel thread goodbye\n");
}

/**
 *
 */
void ToPanelThread::execute() {

    message* msg;
    unsigned char* x;

    memset(buf, 0, OUT_BUF_CNT);

    while (threads_run) {
        state->wait();

// todo: res processing
//        unsigned char* x = (unsigned char*) malloc(sizeof(unsigned char));
//        *x = 1;
//        ijq->post(new myjob(x));

        msg = ijq->getmessage(MSG_WAIT);
//XPLMSpeakString("received\n");
        x = ((myjob*) msg)->buf;

// TODO: add a real message
        if (*x == 0xff)
            goto end;

        toggle_bit(&buf[BTNS_BYTE_INDEX], AP_BIT_POS);

        if (hid) {
            res = hid_send_feature_report((hid_device*)hid, buf, OUT_BUF_CNT);

            if (res == HID_DISCONNECTED)
                psleep(100);
        }
end:
        free(x);
        delete msg;
    }

    DPRINTF("Saitek ProPanels Plugin: to panel thread goodbye\n");
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

    DPRINTF("Saitek ProPanels Plugin: panel check thread goodbye\n");
}

