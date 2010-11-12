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

#include "PanelThreads.h"
#include "nedmalloc.h"
#include "overloaded.h"
#include "hidapi.h"
#include "Saitek.h"


USING_PTYPES

const int MSG_MYJOB = MSG_USER + 1;

/**
 *
 */
void PanelsCheckThread::execute() {

    unsigned int cnt = 0;

    while (run) {
pout.putf("PanelsCheckThread: %d \n", cnt++);
        if (pend) {
            state->wait();
            pend = 0;
        }

        if (!rpHandle || rpThread->errors == RP_ERROR_THRESH) {
            if (rpHandle) {
                hid_close(rpHandle);
            }

            rp_hid_init();
        }

        if (!mpHandle || mpThread->errors == MP_ERROR_THRESH) {
            if (mpHandle) {
                hid_close(mpHandle);
            }

            mp_hid_init();
        }

        if (!spHandle || spThread->errors == SP_ERROR_THRESH) {
            if (spHandle) {
                hid_close(spHandle);
            }

            sp_hid_init();
        }

        psleep(PANEL_CHECK_INTERVAL * 1000);
    }
pout.putf("Goodbye from PanelsCheckThread \n");
}

void PanelsCheckThread::cleanup() {
}

/**
 *
 */
void RadioPanelThread::execute() {

    unsigned int cnt = 0;
	hid_set_nonblocking(rpHandle, 1);

    while (run) {
pout.putf("RadioPanelThread: %d \n", cnt++);

        if (pend) {
            state->wait();
            pend = 0;
        }

        if (hid_read(rpHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
            pincrement(&errors);
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
pout.putf("Goodbye from RadioPanelThread \n");
}

void RadioPanelThread::cleanup() {
}

void MultiPanelThread::execute() {

    unsigned int cnt = 0;
	hid_set_nonblocking(mpHandle, 1);

    while (run) {
pout.putf("MultiPanelThread: %d \n", cnt++);
        if (pend) {
            state->wait();
            pend = 0;
        }

        if (hid_read(mpHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
            pincrement(&errors);
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
pout.putf("Goodbye from MultiPanelThread \n");
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

    while (run) {
pout.putf("SwitchPanelThread: %d \n", cnt++);
        if (pend) {
            state->wait();
            pend = 0;
        }

        if (hid_read(spHandle, inBuf, IN_BUF_CNT) == HID_ERROR) {
            pincrement(&errors);
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
pout.putf("Goodbye from SwitchPanelThread \n");
}

void SwitchPanelThread::cleanup() {
}
