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
//	hid_set_nonblocking(rpHandle, 1);
pout.putf("Hello from RadioPanelThread \n");
    while (run) {
pout.putf("RadioPanelThread: %d \n", cnt++);

        if (pend) {
pout.putf("RadioPanelThread pend\n");
            state->wait();
            pend = 0;
pout.putf("RadioPanelThread awake\n");
        }

//        message* msg    = rp_ijq->getmessage(WAIT_FOREVER);
//        res = hid_read(rpHandle, buf, 4);

//        rp_ojq->post(new myjob(u8_rcv_cnt, udpRcv_buf));
psleep(500);
    }
pout.putf("Goodbye from RadioPanelThread \n");
}

void RadioPanelThread::cleanup() {

}

void MultiPanelThread::execute() {
    unsigned int cnt = 0;
//	hid_set_nonblocking(mpHandle, 1);
pout.putf("Hello from MultiPanelThread \n");
    while (run) {

        if (pend) {
pout.putf("MultiPanelThread pend\n");
            state->wait();
            pend = 0;
pout.putf("MultiPanelThread awake\n");
        }

pout.putf("MultiPanelThread: %d \n", cnt++);
//        message* msg    = mp_ijq->getmessage(WAIT_FOREVER);
//        res = hid_read(mpHandle, buf, 4);


//        u8_in_cnt   = ((myjob*) msg)->u8_amt;
//        u8_in_buf   = ((myjob*) msg)->data_buf;


//        mp_ojq->post(new myjob(u8_out_cnt, u8_out_buf));

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
//	hid_set_nonblocking(spHandle, 1);
pout.putf("Hello from SwitchPanelThread \n");
    while (run) {

        if (pend) {
pout.putf("SwitchPanelThread pend\n");
            state->wait();
            pend = 0;
pout.putf("SwitchPanelThread awake\n");
        }

pout.putf("SwitchPanelThread: %d \n", cnt++);
//        message* msg = sp_ijq->getmessage(WAIT_FOREVER);

//        res = hid_read(spHandle, buf, 4);

//        u8_snd_cnt      = ((myjob*) msg)->u8_amt;
//        udpSnd_buf      = ((myjob*) msg)->data_buf;

//        sp_ojq->post(new myjob(u8_out_cnt, u8_out_buf));

//        delete msg;
psleep(500);
    }
pout.putf("Goodbye from SwitchPanelThread \n");
}

void SwitchPanelThread::cleanup() {

}
