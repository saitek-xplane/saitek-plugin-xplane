// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __PANELTHREADS_H
#define __PANELTHREADS_H

#include "pasync.h"
#include "hidapi.h"

#define IN_BUF_CNT  (4)
#define OUT_BUF_CNT  (13)

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
        pt::trigger*    state;

        unsigned char   inBuf[IN_BUF_CNT];
        unsigned char   outBuf[OUT_BUF_CNT];
        int             res;

    public:
        int     run;
        int     pend;

        RadioPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c, pt::trigger* d)
                : thread(true), rpHandle(a), rp_ijq(b), rp_ojq(c), state(d) { run = true; pend = false; }
        ~RadioPanelThread() {}
        void cleanup();
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
        pt::trigger*    state;

        unsigned char   inBuf[IN_BUF_CNT];
        unsigned char   outBuf[OUT_BUF_CNT];
        int             res;

    public:
        int     run;
        int     pend;

        MultiPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c, pt::trigger* d)
                : thread(true), mpHandle(a), mp_ijq(b), mp_ojq(c), state(d) { run = true; pend = false; }
        ~MultiPanelThread() {}
        void cleanup();
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
        pt::trigger*    state;

        unsigned char   inBuf[IN_BUF_CNT];
        unsigned char   outBuf[OUT_BUF_CNT];
        int             res;

    public:
        int     run;
        int     pend;

        SwitchPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c, pt::trigger* d)
                : thread(true), spHandle(a), sp_ijq(b), sp_ojq(c), state(d) { run = true; pend = false; }
        ~SwitchPanelThread() {}
        void cleanup();
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
