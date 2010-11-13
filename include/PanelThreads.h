// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __PANELTHREADS_H
#define __PANELTHREADS_H

#include "pasync.h"
#include "hidapi.h"
#include "defs.h"

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
        RadioPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c, pt::trigger* d)
                : thread(true), rpHandle(a), rp_ijq(b), rp_ojq(c), state(d) {}
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
        MultiPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c, pt::trigger* d)
                : thread(true), mpHandle(a), mp_ijq(b), mp_ojq(c), state(d) {}
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
        SwitchPanelThread(hid_device* a, pt::jobqueue* b, pt::jobqueue* c, pt::trigger* d)
                : thread(true), spHandle(a), sp_ijq(b), sp_ojq(c), state(d) {}
        ~SwitchPanelThread() {}
        void cleanup();
        void execute();
};

/**
 * @class PanelsCheckThread
 *
 * @param
 * @param
 */
class PanelsCheckThread : public pt::thread {
    protected:
        hid_device*         rpHandle;
        hid_device*         mpHandle;
        hid_device*         spHandle;
        RadioPanelThread*   rpThread;
        MultiPanelThread*   mpThread;
        SwitchPanelThread*  spThread;
        pt::trigger*        state;

    public:
        PanelsCheckThread(hid_device* a, hid_device* b, hid_device* c, pt::trigger* d)
                        : thread(true), rpHandle(a),  mpHandle(b), spHandle(c), state(d) {}
        ~PanelsCheckThread() {}
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
        unsigned char* buf;

        myjob(unsigned char* pbuf) : message(pt::MSG_USER + 1), buf(pbuf) {}
        ~myjob()  {}
};

#ifdef __cplusplus
extern "C" {
#endif

    extern int volatile test_flag1;
    extern int volatile test_flag2;
    extern int volatile test_flag3;

    extern int volatile pc_pend;
    extern int volatile rp_pend;
    extern int volatile mp_pend;
    extern int volatile sp_pend;

    extern int volatile pc_run;
    extern int volatile rp_run;
    extern int volatile mp_run;
    extern int volatile sp_run;

    extern int volatile rp_errors;
    extern int volatile mp_errors;
    extern int volatile sp_errors;

#ifdef __cplusplus
}
#endif

#endif
