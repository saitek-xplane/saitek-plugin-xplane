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


USING_PTYPES

const int MSG_MYJOB = MSG_USER + 1;

int volatile test_flag1      = 0;
int volatile test_flag2      = 0;
int volatile test_flag3      = 0;

int volatile pc_pend     = false;
int volatile rp_pend     = false;
int volatile mp_pend     = false;
int volatile sp_pend     = false;
int volatile pc_run      = false;
int volatile rp_run      = false;
int volatile mp_run      = false;
int volatile sp_run      = false;
int volatile rp_errors   = 0;
int volatile mp_errors   = 0;
int volatile sp_errors   = 0;

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

    while (rp_run) {

        if (rp_pend) {
            state->wait();
            pexchange((int*)&rp_pend, false);
        }

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
