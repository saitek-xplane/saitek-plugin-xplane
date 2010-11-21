// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __PANELTHREADS_H
#define __PANELTHREADS_H

#include "pasync.h"
#include "hidapi.h"
#include "defs.h"


/**
 * @class FromPanelThread
 *
 * @param
 * @param
 */
class FromPanelThread : public pt::thread {
    protected:
        hid_device *volatile   &hid;
        pt::jobqueue*           ijq;    // messages looped back around to ToPanelThread
        pt::jobqueue*           ojq;    // messages from the panel going to x-plane
        pt::trigger*            state;

        unsigned char           buf[IN_BUF_CNT];
        int                     res;
        unsigned short          product;

        virtual void execute();
        virtual void cleanup() {}

    public:
        FromPanelThread(hid_device *volatile &ia, pt::jobqueue* iiq, pt::jobqueue* ioq,
                        pt::trigger* id, unsigned short ip)
                : thread(true), hid(ia), ijq(iiq), ojq(ioq), state(id), product(ip) {}
        ~FromPanelThread() {}
};

/**
 * @class ToPanelThread
 *
 * @param
 * @param
 */
class ToPanelThread : public pt::thread {
    protected:
        hid_device *volatile   &hid;
        pt::jobqueue*           ijq;    // message from x-plane to the panel
        pt::trigger*            state;

        unsigned char           buf[OUT_BUF_CNT];
        int                     res;
        unsigned short          product;

        virtual void execute();
        virtual void cleanup() {}

    public:
        ToPanelThread(hid_device *volatile &ia, pt::jobqueue* iiq, pt::trigger* id, unsigned short ip)
                : thread(true), hid(ia), ijq(iiq), state(id), product(ip) {}
        ~ToPanelThread() {}
};

/**
 * @class PanelsCheckThread
 *
 * @param
 * @param
 */
class PanelsCheckThread : public pt::thread {
    protected:
        virtual void execute();
        virtual void cleanup() {}

    public:
        PanelsCheckThread() : thread(true) {}
        ~PanelsCheckThread() {}
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

    extern pt::trigger gPcTrigger;
    extern pt::trigger gRpTrigger;
    extern pt::trigger gMpTrigger;
    extern pt::trigger gSpTrigger;

    extern pt::jobqueue gRp_ojq;
    extern pt::jobqueue gRp_ijq;
    extern pt::jobqueue gMp_ijq;
    extern pt::jobqueue gMp_ojq;
    extern pt::jobqueue gSp_ijq;
    extern pt::jobqueue gSp_ojq;

    extern int volatile pc_run;
    extern int volatile threads_run;

    extern hid_device *volatile gRpHandle;
    extern hid_device *volatile gMpHandle;
    extern hid_device *volatile gSpHandle;

    extern void close_hid(hid_device* dev);
    extern bool init_hid(hid_device* volatile* dev, unsigned short prod_id);

#ifdef __cplusplus
}
#endif

#endif
