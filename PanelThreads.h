// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __PANELTHREADS_H
#define __PANELTHREADS_H

#include "pasync.h"
#include "hidapi.h"

/**
 * @class RadioPanelThread
 *
 * @param
 * @param
 */
class RadioPanelThread : public pt::thread {
    protected:
        hid_device*     rpHandle;
        pt::jobqueue*   rp_ijq;
        pt::jobqueue*   rp_ojq;

        unsigned char   buf[32];
        int             res;

    public:
        int run;

        RadioPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c)
                : thread(true), rpHandle(a), rp_ijq(b), rp_ojq(c) { run = true; }
        ~RadioPanelThread() {  }
        void execute();
};

/**
 * @class MultiPanelThread
 *
 * @param
 * @param
 */
class MultiPanelThread : public pt::thread {
    protected:
        hid_device*     mpHandle;
        pt::jobqueue*   mp_ijq;
        pt::jobqueue*   mp_ojq;

        unsigned char   buf[32];
        int             res;

    public:
        int run;

        MultiPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c)
                : thread(true), mpHandle(a), mp_ijq(b), mp_ojq(c) { run = true; }
        ~MultiPanelThread() {  }
        void execute();
};

/**
 * @class SwitchPanelThread
 *
 * @param
 * @param
 */
class SwitchPanelThread : public pt::thread {
    protected:
        hid_device*     spHandle;
        pt::jobqueue*   sp_ijq;
        pt::jobqueue*   sp_ojq;

        unsigned char   buf[32];
        int             res;

    public:
        int run;

        SwitchPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c)
                : thread(true), spHandle(a), sp_ijq(b), sp_ojq(c) { run = true; }
        ~SwitchPanelThread() {  }
        void execute();
};

/**
 * @class myjob
 * Inherits from PTypes message class. A queue item consists of a pointer to a
 * dynamically allocated data buffer as well as the buffer's byte count.
 *
 * @param unsigned char* data_buf - Pointer.
 * @param int u8_amt - Byte count.
 */
class myjob : public pt::message {
    public:
        int         u8_amt;
        unsigned char*        data_buf;

        myjob(int amt, unsigned char* pbuf) :
                            message(pt::MSG_USER + 1), u8_amt(amt), data_buf(pbuf) {}
        ~myjob()  {}
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
