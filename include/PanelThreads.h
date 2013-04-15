// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef PANELTHREADS_H
#define PANELTHREADS_H

struct MpBtnStates {
    bool ap;  bool hdg; bool nav; bool ias;
    bool alt; bool vs; bool apr; bool rev;

    MpBtnStates() : ap(0), hdg(0), nav(0), ias(0),
                  alt(0), vs(0), apr(0), rev(0) {}
};

struct MpModeVals {
    uint32_t alt; uint32_t vs;
    uint32_t ias; uint32_t hdg;
    uint32_t crs; uint32_t vs_sign;

    MpModeVals() : alt(0), vs(0), ias(0), hdg(0), crs(0), vs_sign(0x11) {}
};

struct RpModeVals {
    uint32_t com1; uint32_t com2;
    uint32_t nav1; uint32_t nav2;
    uint32_t com1Stdby; uint32_t com2Stdby;
    uint32_t nav1Stdby; uint32_t nav2Stdby;
    uint32_t adf; uint32_t dme;
    uint32_t xpdr;

    RpModeVals() : com1(0), com2(0), nav1(0), nav2(0),
        com1Stdby(0), com2Stdby(0), nav1Stdby(0), nav2Stdby(0), adf(0), dme(0), xpdr(0) {}
};

/**
 * @class FromPanelThread
 *
 * @param
 * @param
 */
class FromPanelThread : public pt::thread {
    protected:
        hid_device *volatile   &mHid;
        pt::jobqueue*           ijq;
        pt::jobqueue*           ojq;
        pt::trigger*            mState;

        unsigned short          mProduct;
        uint32_t                mTmp;
        int                     mRes;
        bool                    mDoInit;

        void (FromPanelThread::*proc_msg)(uint32_t msg);
        void (FromPanelThread::*init)();

        void rp_processing(uint32_t msg);
        void mp_processing(uint32_t msg);
        void sp_processing(uint32_t msg);

        void mp_init();
        void sp_init();
        void rp_init();

        virtual void execute();
        virtual void cleanup() {}

    public:
        FromPanelThread(hid_device *volatile &ihid, pt::jobqueue* iiq, pt::jobqueue* ioq,
                        pt::trigger* itrigger, unsigned short iproduct)
                       : thread(true), mHid(ihid), ijq(iiq), ojq(ioq), mState(itrigger),
                         mProduct(iproduct), mDoInit(true) {}
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
        hid_device *volatile   &mHid;
        pt::jobqueue*           ojq;
        pt::trigger*            mState;

        unsigned short          mProduct;
        uint32_t                mKnobPos;
        bool                    mAvionicsOn;
        bool                    mBat1On;
        bool                    mAthlOn;

        uint8_t                 mPanelReport[OUT_BUF_CNT];
        MpModeVals              mModeVals;
        MpBtnStates             mBtns;
        int                     mRes;
        bool                    mDoInit;

        uint8_t                 mRpReport[RP_OUT_BUF_CNT];
        RpModeVals              mRpModeVals;
        uint32_t                mRpUpperKnobPos;
        uint32_t                mRpLowerKnobPos;


        void (ToPanelThread::*proc_msg)(uint32_t msg, uint32_t u32data);
        void (ToPanelThread::*init)();

        inline void mp_led_update(uint32_t x, uint32_t y, uint32_t s, uint8_t m[]);
        inline void rp_upper_led_update(uint32_t x, uint32_t y, uint8_t m[]);
        inline void rp_lower_led_update(uint32_t x, uint32_t y, uint8_t m[]);
        void rp_processing(uint32_t msg, uint32_t data);
        void mp_processing(uint32_t msg, uint32_t data);
        void sp_processing(uint32_t msg, uint32_t data);

        void mp_init();
        void sp_init();
        void rp_init();

        virtual void execute();
        virtual void cleanup() {}

    public:
        ToPanelThread(hid_device *volatile &ihid, pt::jobqueue* ioq,
                      pt::trigger* itrigger, unsigned short iproduct)
         : thread(true), mHid(ihid), ojq(ioq), mState(itrigger), mProduct(iproduct),
            mKnobPos(0), mAvionicsOn(false), mBat1On(false), mAthlOn(false), mDoInit(true) {}
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

    extern int volatile gPcRun;
    extern int volatile gThreadsRun;
    extern int volatile gPluginEnabled;

    extern hid_device *volatile gRpHidHandle;
    extern hid_device *volatile gMpHidHandle;
    extern hid_device *volatile gSpHidHandle;

    extern const unsigned char rp_blank_panel[23];
    extern const unsigned char mp_blank_panel[13];
    extern const unsigned char sp_blank_panel[2];

    extern void close_hid(hid_device* dev);
    extern bool init_hid(hid_device* volatile* dev, unsigned short prod_id);

#ifdef __cplusplus
}
#endif

#endif /* PANELTHREADS_H */
