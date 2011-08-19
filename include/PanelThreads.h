// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef PANELTHREADS_H
#define PANELTHREADS_H

/**
 * @class FromPanelThread
 *
 * @param
 * @param
 */
class FromPanelThread : public pt::thread {
    protected:
        hid_device *volatile   &hid;
        pt::jobqueue*           ijq;
        pt::jobqueue*           ojq;
        pt::trigger*            mState;

        uint32_t                mTmp;
        int                     mRes;
        unsigned short          mProduct;
        uint32_t                AvAndBatOn;

        void (FromPanelThread::*proc_msg)(uint32_t msg);

        void rp_processing(uint32_t msg);
        void mp_processing(uint32_t msg);
        void sp_processing(uint32_t msg);

        virtual void execute();
        virtual void cleanup() {}

    public:
        FromPanelThread(hid_device *volatile &ihid, pt::jobqueue* iiq, pt::jobqueue* ioq,
                        pt::trigger* itrigger, unsigned short iproduct)
                       : thread(true), hid(ihid), ijq(iiq), ojq(ioq), mState(itrigger),
                         mProduct(iproduct), AvAndBatOn(0) {}
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
        pt::jobqueue*           ojq;    // messages coming out from x-plane to the panel
        pt::trigger*            mState;

        uint8_t                 mBuf[OUT_BUF_CNT];
        int                     mRes;
        unsigned short          mProduct;
        bool                    mAvionicsOn;
        bool                    mBat1On;

        void (ToPanelThread::*proc_msg)(uint32_t msg);

        void rp_processing(uint32_t msg);
        void mp_processing(uint32_t msg);
        void sp_processing(uint32_t msg);

        virtual void execute();
        virtual void cleanup() {}

    public:
        ToPanelThread(hid_device *volatile &ihid, pt::jobqueue* ioq,
                      pt::trigger* itrigger, unsigned short iproduct)
                      : thread(true), hid(ihid), ojq(ioq), mState(itrigger),
            mProduct(iproduct), mAvionicsOn(false), mBat1On(false) {}
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
 * dynamically allocated data buffer.
 *
 * @param unsigned char* data_buf - Pointer.
 * @param int u8_amt - Byte count.
 */
class myjob : public pt::message {
    public:
        uint32_t* buf;

        myjob(uint32_t* pbuf) : message(pt::MSG_USER + 1), buf(pbuf) {}
        ~myjob()  { if (buf) delete buf; }
};


#ifdef __cplusplus
extern "C" {
#endif

    extern pt::trigger gPcTrigger;
    extern pt::trigger gRpTrigger;
    extern pt::trigger gMpTrigger;
    extern pt::trigger gSpTrigger;

    extern pt::jobqueue gRp_ijq;
    extern pt::jobqueue gRp_ojq;
    extern pt::jobqueue gMp_ijq;
    extern pt::jobqueue gMp_ojq;
    extern pt::jobqueue gSp_ijq;
    extern pt::jobqueue gSp_ojq;

    extern int volatile pc_run;
    extern int volatile threads_run;

    extern hid_device *volatile gRpHandle;
    extern hid_device *volatile gMpHandle;
    extern hid_device *volatile gSpHandle;

    extern const unsigned char rp_hid_blank_panel[13];
    extern const unsigned char mp_hid_blank_panel[13];
    extern const unsigned char sp_hid_blank_panel[13];

    extern void close_hid(hid_device* dev);
    extern bool init_hid(hid_device* volatile* dev, unsigned short prod_id);

#ifdef __cplusplus
}
#endif

#endif /* PANELTHREADS_H */
