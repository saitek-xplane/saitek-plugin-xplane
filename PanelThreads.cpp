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


USING_PTYPES

const int MSG_MYJOB = MSG_USER + 1;

int pc_pend = false;
int rp_pend = false;
int mp_pend = false;
int sp_pend = false;

int pc_run = false;
int rp_run = false;
int mp_run = false;
int sp_run = false;

int rp_errors = 0;
int mp_errors = 0;
int sp_errors = 0;




/**
 *
 */
void PanelsCheckThread::execute() {

    unsigned int cnt = 0;

    while (pc_run) {
DPRINTF_VA("PanelsCheckThread: %d \n", cnt++);
        if (pc_pend) {
            state->wait();
            pexchange(&pc_pend, false);
        }

        if (!rpHandle || rp_errors == RP_ERROR_THRESH) {
            if (rpHandle) {
                hid_close(rpHandle);
            }

            rp_hid_init();
            pexchange(&rp_errors, 0);
        }

        if (!mpHandle || mp_errors == MP_ERROR_THRESH) {
            if (mpHandle) {
                hid_close(mpHandle);
            }

            mp_hid_init();
            pexchange(&mp_errors, 0);
        }

        if (!spHandle || sp_errors == SP_ERROR_THRESH) {
            if (spHandle) {
                hid_close(spHandle);
            }

            sp_hid_init();
            pexchange(&sp_errors, 0);
        }

        psleep(PANEL_CHECK_INTERVAL * 1000);
    }
DPRINTF("Goodbye from PanelsCheckThread \n");
}

void PanelsCheckThread::cleanup() {
}

/**
 *
 */
void RadioPanelThread::execute() {

    unsigned int cnt = 0;
	hid_set_nonblocking(rpHandle, 1);

    while (rp_run) {
DPRINTF_VA("RadioPanelThread: %d \n", cnt++);

        if (rp_pend) {
            state->wait();
            pexchange(&rp_pend, false);
        }

        if (hid_read(rpHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
            pincrement(&rp_errors);
        }

// todo: res processing
//        rp_ijq->post(new myjob(udpRcv_buf));

        message* msg = rp_ojq->getmessage(MSG_NOWAIT);

        if (msg) {
//        u8_in_buf   = ((myjob*) msg)->buf;
            hid_send_feature_report(rpHandle, outBuf, OUT_BUF_CNT);

            delete msg;
        }
// todo: msg processing
psleep(500);
    }
DPRINTF("Goodbye from RadioPanelThread \n");
}

void RadioPanelThread::cleanup() {
}

void MultiPanelThread::execute() {

    unsigned int cnt = 0;
	hid_set_nonblocking(mpHandle, 1);

    while (mp_run) {
DPRINTF_VA("MultiPanelThread: %d \n", cnt++);
        if (mp_pend) {
            state->wait();
            pexchange(&mp_pend, false);
        }

        if (hid_read(mpHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
            pincrement(&mp_errors);
        }

// todo: res processing
//        mp_ijq->post(new myjob(outBuf));

        message* msg = mp_ojq->getmessage(MSG_NOWAIT);

        if (msg) {
//        u8_in_buf   = ((myjob*) msg)->buf;
            hid_send_feature_report(mpHandle, outBuf, OUT_BUF_CNT);

            delete msg;
        }
psleep(500);
    }
DPRINTF("Goodbye from MultiPanelThread \n");
}

void MultiPanelThread::cleanup() {
}

/**
 *
 *
 */
void SwitchPanelThread::execute() {

    unsigned int cnt = 0;
	hid_set_nonblocking(spHandle, 1);

    while (sp_run) {
DPRINTF_VA("SwitchPanelThread: %d \n", cnt++);
        if (sp_pend) {
            state->wait();
            pexchange(&sp_pend, false);
        }

        if (hid_read(spHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
            pincrement(&sp_errors);
        }

// todo: res processing
//        sp_ijq->post(new myjob(out_buf));

        message* msg = sp_ojq->getmessage(MSG_NOWAIT);

        if (msg) {
//        u8_in_buf   = ((myjob*) msg)->buf;
            hid_send_feature_report(spHandle, outBuf, OUT_BUF_CNT);

            delete msg;
        }
psleep(500);
    }
DPRINTF("Goodbye from SwitchPanelThread \n");
}

void SwitchPanelThread::cleanup() {
}
