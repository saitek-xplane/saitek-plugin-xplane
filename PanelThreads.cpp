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

const unsigned char hid_open_msg[13] = {0x00, 0x0a, 0x0a, 0x0a, 0x0a,
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



/*
    tuning:
        clockwise           28 00 00
        counter clockwise   48 00 00
    pitch
        up                  08 80 04
        neutral             08 80 00
        down                08 80 08
    auto throttle
        arm                 08 80 00
        off                 08 00 00
    flaps
        up                  08 80 01
        neutral             08 80 00
        down                08 80 02
    IAS knob
        hdg                 08 00 00
        ias                 04 00 00
        vs                  02 00 00
        alt                 01 00 00
        crs                 10 00 00
    button
        ap                  10 01 00
        hdg                 10 02 00/10 80 00
        nav                 10 04 00
        ias                 10 08 00
        alt                 10 10 00
        vs                  10 20 00
        apr                 10 40 00
        rev                 10 80 00
*/

bool init_hid(hid_device* volatile* dev, unsigned short prod_id) {
    pexchange((void**)(dev),
                (void*)hid_open(&close_hid, VENDOR_ID, prod_id, NULL));

    if (*dev) {
        hid_send_feature_report((hid_device*)*dev, hid_open_msg, OUT_BUF_CNT);

        return true;
    }

    return false;
}

/**
 *
 */
void FromPanelThread::execute() {
//    memset(outBuf, 0, OUT_BUF_CNT);
    unsigned char* x;

    while (threads_run) {
        state->wait();

        if (!hid) {
            psleep(100); // what's a good (millisecond) timeout time?
            continue;
        }

        if ((res = hid_read((hid_device*)hid, buf, HID_READ_CNT)) <= 0) {
            if (res == HID_DISCONNECTED)
//XPLMSpeakString("disconnected");
                psleep(100); // what's a good (millisecond) timeout time?
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

