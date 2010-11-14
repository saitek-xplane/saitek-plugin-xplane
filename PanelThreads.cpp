// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <cstring>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>

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

static void toggle_bit(unsigned char* c, int pos);

const int MSG_MYJOB     = MSG_USER + 1;

int volatile test_flag1 = 0;
int volatile test_flag2 = 0;
int volatile test_flag3 = 0;

int volatile pc_pend    = false;
int volatile rp_pend    = false;
int volatile mp_pend    = false;
int volatile sp_pend    = false;
int volatile pc_run     = false;
int volatile rp_run     = false;
int volatile mp_run     = false;
int volatile sp_run     = false;
int volatile rp_errors  = 0;
int volatile mp_errors  = 0;
int volatile sp_errors  = 0;

enum {
    LED1_BYTE_START = 1, // top
    LED1_BYTE_CNT   = 5, // bottom

    LED2_BYTE_START = 6,
    LED2_BYTE_CNT   = 5,

    BTNS_BYTE_START = 11,
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
void PanelsCheckThread::execute() {
    pexchange((int*)&pc_run, true);

    while (pc_run) {

        if (pc_pend) {
            state->wait();
            pexchange((int*)&pc_pend, false);
        }

// XXX: add triggers to start-up threads if they're in pend mode
        if (!rpHandle || rp_errors == RP_ERROR_THRESH) {
            if (rpHandle) {
                sp_hid_close();
            }

            rp_hid_init();
            pexchange((int*)&rp_errors, 0);
        }

        if (!mpHandle || mp_errors == MP_ERROR_THRESH) {
            if (mpHandle) {
                sp_hid_close();
            }

            mp_hid_init();
            pexchange((int*)&mp_errors, 0);
        }

        if (!spHandle || sp_errors == SP_ERROR_THRESH) {
            if (spHandle) {
                sp_hid_close();
            }

            sp_hid_init();
            pexchange((int*)&sp_errors, 0);
        }

        psleep(PANEL_CHECK_INTERVAL * 1000);
    }
}

void PanelsCheckThread::cleanup() {
}

/**
 *
 */
void RadioPanelThread::execute() {
    pexchange((int*)&rp_run, true);

	hid_set_nonblocking(rpHandle, 1);

    memset(outBuf, 0, OUT_BUF_CNT);

    while (rp_run) {

        if (rp_pend) {
            state->wait();
            pexchange((int*)&rp_pend, false);
        }

        outBuf[1] =  outBuf[1] ^ 0xFF;
        hid_send_feature_report(rpHandle, outBuf, OUT_BUF_CNT);

//        if (hid_read(rpHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
//            pincrement(&rp_errors);
//        }

// todo: res processing
//        unsigned char* x = (unsigned char*) malloc(sizeof(unsigned char));
//        *x = 1;
//        rp_ijq->post(new myjob(x));

//        message* msg = rp_ojq->getmessage(MSG_NOWAIT);

//        if (msg) {
//        u8_in_buf   = ((myjob*) msg)->buf;
//            hid_send_feature_report(rpHandle, outBuf, OUT_BUF_CNT);

//            delete msg;
//        }
// todo: msg processing
psleep(5000);
    }
}

void RadioPanelThread::cleanup() {
}

void MultiPanelThread::execute() {
    pexchange((int*)&mp_run, true);

	hid_set_nonblocking(mpHandle, 1);

    while (mp_run) {

        if (mp_pend) {
            state->wait();
            pexchange((int*)&mp_pend, false);
        }

//        if (hid_read(mpHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
//            pincrement(&mp_errors);
//        }

// todo: res processing
//        mp_ijq->post(new myjob(outBuf));

//        message* msg = mp_ojq->getmessage(MSG_NOWAIT);

//        if (msg) {
////        u8_in_buf   = ((myjob*) msg)->buf;
//            hid_send_feature_report(mpHandle, outBuf, OUT_BUF_CNT);

//            delete msg;
//        }
psleep(500);
    }
}

void MultiPanelThread::cleanup() {
}

/**
 *
 *
 */
void SwitchPanelThread::execute() {
    pexchange((int*)&sp_pend, true);

	hid_set_nonblocking(spHandle, 1);

    while (sp_run) {

        if (sp_pend) {
            state->wait();
            pexchange((int*)&sp_pend, false);
        }

//        if (hid_read(spHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
//            pincrement(&sp_errors);
//        }

// todo: res processing
//        sp_ijq->post(new myjob(out_buf));

//        message* msg = sp_ojq->getmessage(MSG_NOWAIT);

//        if (msg) {
////        u8_in_buf   = ((myjob*) msg)->buf;
//            hid_send_feature_report(spHandle, outBuf, OUT_BUF_CNT);

//            delete msg;
//        }
psleep(500);
    }
}

void SwitchPanelThread::cleanup() {
}
