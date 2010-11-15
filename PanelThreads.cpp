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
#include "PanelThreads.h"
#include "nedmalloc.h"
#include "overloaded.h"
#include "hidapi.h"
#include "Saitek.h"

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

//static void toggle_bit(unsigned char* c, int pos);

const int MSG_MYJOB     = MSG_USER + 1;

int volatile pc_run     = false;
int volatile threads_run = false;


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

unsigned char gReport[64];

trigger     gPcTrigger(false, true);
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

//// mask_gen returns the bit field width representation of the value x.
//func maskWidth(x uint32) uint32 {
//	return ^(0xFFFFFFFF << x)
//}

//// bitsSet
//func bitsSet(x uint32) uint32 {
//	x = x - ((x >> 1) & 0x55555555)
//	x = (x & 0x33333333) + ((x >> 2) & 0x33333333)

//	return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24
//}

void toggle_bit(unsigned char* c, long pos) {
    *c ^= (0x01 << pos);
}

void to_bytes(unsigned char* c, unsigned long long v) {
    int shift = 32;

    for (int i = 0; i < 5; i++) {
        c[i] = (unsigned char) (v >> shift);
        shift += 8;
    }
}

/**
 *
 */
void FromPanelThread::execute() {
//    memset(outBuf, 0, OUT_BUF_CNT);
    unsigned char* x;

    while (threads_run) {
        state->wait();
psleep(300);
//if (product == RP_PROD_ID) {
////    gRpTrigger.wait();
//    XPLMSpeakString("radio\n");
//} else if (product == MP_PROD_ID) {
////    gMpTrigger.wait();
//    XPLMSpeakString("multi\n");
//} else if (product == SP_PROD_ID) {
////    gSpTrigger.wait();
//    XPLMSpeakString("switch\n");
//}

    if (hid && hid_check(VENDOR_ID, product)) {
        if ((res = hid_read((hid_device*)hid, buf, 4)) < 0) {
XPLMSpeakString("less than 0 a\n");
            continue;
        }
    } else {
        continue;
    }

    if (!res)
        continue;

    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 1;
//XPLMSpeakString("posting\n");
    ijq->post(new myjob(x));

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

        if (*x == 0xff)
            goto end;

        toggle_bit(&buf[BTNS_BYTE_INDEX], AP_BIT_POS);

        if (hid && hid_check(VENDOR_ID, product)) {
            hid_send_feature_report((hid_device*)hid, buf, OUT_BUF_CNT);
        }
end:
        free(x);
        delete msg;
    }
DPRINTF("Saitek ProPanels Plugin: to panel thread goodbye\n");
}

/**
 * XPLMSpeakString("init b\n");
 */
void PanelsCheckThread::execute() {
    pexchange((int*)&pc_run, true);
    hid_device* tmpHandle;

// XXX: flush the queues during a pend
    while (pc_run) {
        gPcTrigger.wait();

        if (gRpHandle) {
            if (hid_check(VENDOR_ID, RP_PROD_ID)) {
                goto continue_a;
            }

            gRpTrigger.reset();
            tmpHandle = (hid_device*)gRpHandle;
            pexchange((void**)(&gRpHandle), NULL);
            hid_delete_report(tmpHandle);
            rp_hid_init();

            if (gRpHandle) {
                gRpTrigger.post();
            }
        } else {
            gRpTrigger.reset();
            rp_hid_init();

            if (gRpHandle) {
                gRpTrigger.post();
            }
        }

continue_a:

        if (gMpHandle) {
            if (hid_check(VENDOR_ID, MP_PROD_ID)) {
                goto continue_b;
            }

            gMpTrigger.reset();
            tmpHandle = (hid_device*)gMpHandle;
            pexchange((void**)(&gMpHandle), NULL);
            hid_delete_report((hid_device*)gMpHandle);
            mp_hid_init();

            if (gMpHandle) {
                gMpTrigger.post();
XPLMSpeakString("okay a\n");
            }
        } else {
            gMpTrigger.reset();
            mp_hid_init();

            if (gMpHandle) {
XPLMSpeakString("okay b\n");
                gMpTrigger.post();
            }
        }

continue_b:

        if (gSpHandle) {
            if (hid_check(VENDOR_ID, SP_PROD_ID)) {
                goto continue_c;
            }

            gSpTrigger.reset();
            tmpHandle = (hid_device*)gSpHandle;
            pexchange((void**)(&gSpHandle), NULL);
            hid_delete_report((hid_device*)gSpHandle);
            sp_hid_init();

            if (gSpHandle) {
                gSpTrigger.post();
            }
        } else {
            gSpTrigger.reset();
            sp_hid_init();

            if (gSpHandle) {
                gSpTrigger.post();
            }
        }

continue_c:

        psleep(PANEL_CHECK_INTERVAL * 1000);
    }
DPRINTF("Saitek ProPanels Plugin: panel check thread goodbye\n");
}

