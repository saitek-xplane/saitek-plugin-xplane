// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef PANELTHREADS_H
#define PANELTHREADS_H

struct BtnStates {
    bool ap;  bool hdg; bool nav; bool ias;
    bool alt; bool vs; bool apr; bool rev;

    BtnStates() : ap(0), hdg(0), nav(0), ias(0),
                  alt(0), vs(0), apr(0), rev(0) {}
};

struct ModeVals {
    uint32_t alt; uint32_t vs;
    uint32_t ias; uint32_t hdg;
    uint32_t crs; uint32_t vs_sign;

ModeVals() : alt(0), vs(0), ias(0), hdg(0), crs(0), vs_sign(0x11) {}
};

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

        unsigned short          mProduct;
        uint32_t                mTmp;
        int                     mRes;

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
                         mProduct(iproduct) {}
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
        pt::jobqueue*           ojq;
        pt::trigger*            mState;

        unsigned short          mProduct;
        uint32_t                mKnobPos;
        bool                    mAvionicsOn;
        bool                    mBat1On;
        bool                    mAthlOn;

        uint8_t                 mReport[OUT_BUF_CNT];
        ModeVals                mModeVals;
        BtnStates               mBtns;
        int                     mRes;

        void (ToPanelThread::*proc_msg)(uint32_t msg, uint32_t u32data);

        inline void led_update(uint32_t x, uint32_t y, uint32_t s, uint8_t m[]);
        void rp_processing(uint32_t msg, uint32_t data);
        void mp_processing(uint32_t msg, uint32_t data);
        void sp_processing(uint32_t msg, uint32_t data);

        virtual void execute();
        virtual void cleanup() {}

    public:
        ToPanelThread(hid_device *volatile &ihid, pt::jobqueue* ioq,
                      pt::trigger* itrigger, unsigned short iproduct)
         : thread(true), hid(ihid), ojq(ioq), mState(itrigger), mProduct(iproduct),
            mKnobPos(0), mAvionicsOn(false), mBat1On(false), mAthlOn(false) {}
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

    extern const unsigned char rp_blank_panel[13];
    extern const unsigned char mp_blank_panel[13];
    extern const unsigned char sp_blank_panel[1];

    extern void close_hid(hid_device* dev);
    extern bool init_hid(hid_device* volatile* dev, unsigned short prod_id);

#ifdef __cplusplus
}
#endif

#endif /* PANELTHREADS_H */
