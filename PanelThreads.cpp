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

#include "PanelThreads.h"


USING_PTYPES
using namespace std;

const int MSG_MYJOB = MSG_USER + 1;

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

        res = hid_read(rpHandle, inBuf, IN_BUF_CNT);

// todo: res processing
//        rp_ojq->post(new myjob(u8_rcv_cnt, udpRcv_buf));

        message* msg = rp_ijq->getmessage(0);

        if (msg) {

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

        res = hid_read(mpHandle, inBuf, IN_BUF_CNT);

// todo: res processing
//        mp_ojq->post(new myjob(u8_out_cnt, u8_out_buf));

        message* msg = mp_ijq->getmessage(0);

        if (msg) {


            hid_send_feature_report(mpHandle, outBuf, OUT_BUF_CNT);
            delete msg;
        }

//        u8_in_cnt   = ((myjob*) msg)->u8_amt;
//        u8_in_buf   = ((myjob*) msg)->data_buf;




//        delete msg;
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

        res = hid_read(spHandle, inBuf, IN_BUF_CNT);

// todo: res processing
//        sp_ojq->post(new myjob(u8_out_cnt, u8_out_buf));

        message* msg = sp_ijq->getmessage(0);

        if (msg) {

            hid_send_feature_report(spHandle, outBuf, OUT_BUF_CNT);
            delete msg;
        }

//        u8_snd_cnt      = ((myjob*) msg)->u8_amt;
//        udpSnd_buf      = ((myjob*) msg)->data_buf;




psleep(500);
    }
pout.putf("Goodbye from SwitchPanelThread \n");
}

void SwitchPanelThread::cleanup() {

}
