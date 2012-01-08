// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <cstring>
#include <cmath>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>

#include "pport.h"
#include "ptypes.h"
#include "pstreams.h"

#include "XPLMDefs.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

//#include "overloaded.h"
#include "defs.h"
#include "utils.h"
#include "hidapi.h"
#include "PanelThreads.h"
#include "multipanel.h"
#include "SaitekProPanels.h"

USING_PTYPES

#define toggle_bit(c, pos)  (*(c) ^= (0x01 << pos))
#define set_bit(c, pos)     (*(c) |= (0x01 << pos))
#define clear_bit(c, pos)   (*(c) &= ~(0x01 << pos))
#define get_bit(c, pos)     (*(c) &= (0x01 << pos))

#define MP_LED_PLUS_SIGN    (0x0A)
#define MP_LED_MINUS_SIGN   (0x0E)

//static void toggle_bit(unsigned char* c, int pos);

const int MSG_MYJOB         = MSG_USER + 1;
int volatile pc_run         = false;
int volatile threads_run    = false;

// panel threads
hid_device *volatile gRpHidHandle = NULL;
hid_device *volatile gMpHidHandle = NULL;
hid_device *volatile gSpHidHandle = NULL;

// index[0] - report ID, which is always zero
// TODO: radio panel message
const unsigned char rp_blank_panel[13] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char mp_blank_panel[13] = {0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                                        0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00};

const unsigned char mp_zero_panel[13] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// TODO: switch panel message
const unsigned char sp_blank_panel[2] = {0x00, 0x00};
const unsigned char sp_green_panel[2] = {0x00, 0x07};

trigger gPcTrigger(true, false);
trigger gRpTrigger(false, false);
trigger gMpTrigger(false, false);
trigger gSpTrigger(false, false);

/**
 *
 */
void close_hid(hid_device* dev) {
// TODO: flush the queues?
    if (dev) {
        if (dev == gRpHidHandle) {
            hid_send_feature_report(gRpHidHandle, rp_blank_panel, sizeof(rp_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gRpHidHandle), NULL);
            gRpTrigger.reset();
        } else if (dev == gMpHidHandle) {
            hid_send_feature_report(gMpHidHandle, mp_blank_panel, sizeof(mp_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gMpHidHandle), NULL);
            gMpTrigger.reset();
        } else if (dev == gSpHidHandle) {
            hid_send_feature_report(gSpHidHandle, sp_blank_panel, sizeof(sp_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gSpHidHandle), NULL);
            gSpTrigger.reset();
        }
    }
}


/**
 *
 */
bool init_hid(hid_device* volatile* dev, unsigned short prod_id) {
    pexchange((void**)dev, (void*)hid_open(VENDOR_ID, prod_id, NULL));
    return (*dev != 0) ? true : false;
}



#define DO_LPRINTFS 0
#if DO_LPRINTFS
char gTmp1[100];
#endif
/**
 *
 */
void FromPanelThread::execute() {
    switch(mProduct) {
    case RP_PROD_ID:
        init = &FromPanelThread::rp_init;
        proc_msg = &FromPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        init = &FromPanelThread::mp_init;
        proc_msg = &FromPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        init = &FromPanelThread::sp_init;
        proc_msg = &FromPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        break;
    }

    while (threads_run) {
        mState->wait();

        if (mDoInit) {
            (this->*init)();
        }

        mTmp = 0;

        if (!mHid) {
            psleep(100); // what's a good timeout?
            continue;
        }

        if ((mRes = hid_read((hid_device*)mHid, (uint8_t*)&mTmp, sizeof(uint32_t))) <= 0) {
            if (mRes == HID_ERROR) {
                // TODO: log error
            }
            continue;
        }
#if DO_LPRINTFS
sprintf(gTmp1, "Saitek ProPanels Plugin: FromPanelThread::execute ox%X \n", mTmp);
LPRINTF(gTmp1);
#endif
        (this->*proc_msg)(mTmp);
    }

    LPRINTF("Saitek ProPanels Plugin: FromPanelThread goodbye\n");
}


void FromPanelThread::mp_init() {
//    if (!mHid) {
//        return;
//    }
    mDoInit = false;
//    hid_set_nonblocking(mHid, (int)true);

//    hid_send_feature_report(mHid, mp_zero_panel, sizeof(mp_zero_panel));
//    if ((mRes = hid_read((hid_device*)mHid, (uint8_t*)&mTmp, sizeof(uint32_t))) <= 0) {
//        if (mRes == HID_ERROR) {
//            // TODO: log error
//        }
//    } else {
//        (this->*proc_msg)(mTmp);
//    }
//    hid_set_nonblocking(mHid, (int)false);
}


void FromPanelThread::sp_init() {
    // check hid before making calls
    // set mDoInit
}


void FromPanelThread::rp_init() {
    // check hid before making calls
    // set mDoInit
}

/**
 *
 */
void FromPanelThread::rp_processing(uint32_t msg) {
}


/**
 *
 */
void FromPanelThread::mp_processing(uint32_t msg) {
    //static char tmp[100];
    uint32_t btns = msg & MP_READ_BTNS_MASK;
    uint32_t flaps = msg & MP_READ_FLAPS_MASK;
    uint32_t trim = msg & MP_READ_TRIM_MASK;
    uint32_t tuning = msg & MP_READ_TUNING_MASK;

    // When not an exclusive event, the state
    // is included with the previously listed
    // events
    uint32_t knob = msg & MP_READ_KNOB_MODE_MASK;

    // When not an exclusive event, the state
    // is included with all other events
    uint32_t autothrottle = msg & MP_READ_THROTTLE_MASK;

    uint32_t msg2= 0;
    uint32_t* x;
    bool to_oqueue = true;

    //sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%.8X \n", msg);
    //DPRINTF(tmp);

    msg = 0;
    if (btns) {
        switch(btns) {
        case MP_READ_AP_BTN:
//            msg = MP_BTN_AP_TOGGLE_CMD_MSG;
            msg = MP_BTN_FD_UP_ONE_CMD_MSG;
            break;
        case MP_READ_HDG_BTN:
            msg = MP_BTN_HDG_TOGGLE_CMD_MSG;
            break;
        case MP_READ_NAV_BTN:
            msg = MP_BTN_NAV_TOGGLE_CMD_MSG;
            break;
        case MP_READ_IAS_BTN:
            msg = MP_BTN_IAS_TOGGLE_CMD_MSG;
            break;
        case MP_READ_ALT_BTN:
            msg = MP_BTN_ALT_TOGGLE_CMD_MSG;
            break;
        case MP_READ_VS_BTN:
            msg = MP_BTN_VS_TOGGLE_CMD_MSG;
            break;
       case MP_READ_APR_BTN:
            msg = MP_BTN_APR_TOGGLE_CMD_MSG;
            break;
        case MP_READ_REV_BTN:
            msg = MP_BTN_REV_TOGGLE_CMD_MSG;
            break;
        default:
            to_oqueue = false;
            // TODO: log error
            break;
        }
    } else if (flaps) {
        to_oqueue = false;
        if (flaps == MP_READ_FLAPS_UP) {
            msg = MP_FLAPS_UP_CMD_MSG;
        } else if (flaps == MP_READ_FLAPS_DN) {
            msg = MP_FLAPS_DN_CMD_MSG;
        } else {
            // TODO: log error
        }
    } else if (trim) {
// TODO: fine & coarse grained adjustment
        to_oqueue = false;
        if (trim == MP_READ_TRIM_UP) {
            msg = MP_PITCHTRIM_UP_CMD_MSG;
        } else if (trim == MP_READ_TRIM_DOWN) {
            msg = MP_PITCHTRIM_DN_CMD_MSG;
        } else {
            // TODO: log error
        }
    } else if (tuning) {
// TODO: fine & coarse grained adjustment
        switch(knob) {
        case MP_READ_KNOB_ALT:
            msg = msg2 = (tuning == MP_READ_TUNING_RIGHT) ? MP_ALT_UP_CMD_MSG : MP_ALT_DN_CMD_MSG;
            break;
        case MP_READ_KNOB_VS:
            msg = msg2 = (tuning == MP_READ_TUNING_RIGHT) ? MP_VS_UP_CMD_MSG : MP_VS_DN_CMD_MSG;
            break;
        case MP_READ_KNOB_IAS:
            msg = msg2 = (tuning == MP_READ_TUNING_RIGHT) ? MP_IAS_UP_CMD_MSG : MP_IAS_DN_CMD_MSG;
            break;
        case MP_READ_KNOB_HDG:
            msg = msg2 = (tuning == MP_READ_TUNING_RIGHT) ? MP_HDG_UP_CMD_MSG : MP_HDG_DN_CMD_MSG;
            break;
        case MP_READ_KNOB_CRS:
            msg = msg2 = (tuning == MP_READ_TUNING_RIGHT) ? MP_CRS_UP_CMD_MSG : MP_CRS_DN_CMD_MSG;
            break;
        default:
            // TODO: log error
            break;
        }
    }

    if (msg) {
        // to the xplane side
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        // loopback to the panel
        if (msg2) {
            x = new uint32_t;
            *x = msg2;
            ojq->post(new myjob(x));
        }

        msg = msg2 = 0;
    }

    if (knob) {
        switch(knob) {
        case MP_READ_KNOB_ALT:
            msg = MP_KNOB_ALT_POS_MSG;
            break;
        case MP_READ_KNOB_VS:
            msg = MP_KNOB_VS_POS_MSG;
            break;
        case MP_READ_KNOB_IAS:
            msg = MP_KNOB_IAS_POS_MSG;
            break;
        case MP_READ_KNOB_HDG:
            msg = MP_KNOB_HDG_POS_MSG;
            break;
        case MP_READ_KNOB_CRS:
            msg = MP_KNOB_CRS_POS_MSG;
            break;
        default:
            // TODO: log error
            break;
        }
    }

    if (msg) {
//        x = new uint32_t;
//        *x = msg;
//        ijq->post(new myjob(x));

        // loopback to the panel and no xplane
        x = new uint32_t;
        *x = msg;
        ojq->post(new myjob(x));
    }

    // message going to xplane and no loopback
    msg = (autothrottle > 0) ? MP_AUTOTHROTTLE_ON_CMD_MSG : MP_AUTOTHROTTLE_OFF_CMD_MSG;
    x = new uint32_t;
    *x = msg;
    ijq->post(new myjob(x));
}


/**
 *
 */
void FromPanelThread::sp_processing(uint32_t msg) {

    uint32_t masterbat = msg & SP_READ_MASTER_BAT_MASK;
    uint32_t masteralt = msg & SP_READ_MASTER_ALT_MASK;
    uint32_t avionicsmaster = msg & SP_READ_AVIONICS_MASTER_MASK;
    uint32_t fuelpump = msg & SP_READ_FUEL_PUMP_MASK;
    uint32_t deice = msg & SP_READ_DE_ICE_MASK;
    uint32_t pitotheat = msg & SP_READ_PITOT_HEAT_MASK;
    uint32_t cowl = msg & SP_READ_COWL_MASK;
    uint32_t lightspanel = msg & SP_READ_LIGHTS_PANEL_MASK;
    uint32_t lightsbeacon = msg & SP_READ_LIGHTS_BEACON_MASK;
    uint32_t lightsnav = msg & SP_READ_LIGHTS_NAV_MASK;
    uint32_t lightsstrobe = msg & SP_READ_LIGHTS_STROBE_MASK;
    uint32_t lightstaxi = msg & SP_READ_LIGHTS_TAXI_MASK;
    uint32_t lightslanding = msg & SP_READ_LIGHTS_LANDING_MASK;
    uint32_t enginesknob = msg & SP_READ_ENGINES_KNOB_MASK;
    uint32_t gearleverup = msg & SP_READ_GEARLEVER_UP_MASK;
    uint32_t gearleverdown = msg & SP_READ_GEARLEVER_DOWN_MASK;

    uint32_t* x;
    bool to_iqueue = true;

    static char tmp[100];
    sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%.8X \n", msg);
    DPRINTF(tmp);

    msg = 0;

    if (enginesknob == 0x002000) {
        to_iqueue = false;
    	msg = SP_MAGNETOS_OFF_MSG;
    } else if (enginesknob == 0x004000) {
        to_iqueue = false;
    	msg = SP_MAGNETOS_RIGHT_MSG;
    } else if (enginesknob == 0x008000) {
        to_iqueue = false;
    	msg = SP_MAGNETOS_LEFT_MSG;
    } else if (enginesknob == 0x010000) {
        to_iqueue = false;
    	msg = SP_MAGNETOS_BOTH_MSG;
    } else if (enginesknob == 0x020000) {
        to_iqueue = false;
    	msg = SP_MAGNETOS_START_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (masterbat) {
        to_iqueue = false;
        msg = SP_MASTER_BATTERY_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_MASTER_BATTERY_OFF_MSG;
    }

    if (masteralt) {
        to_iqueue = false;
        msg = SP_MASTER_ALT_BATTERY_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_MASTER_ALT_BATTERY_OFF_MSG;
    }

    if (avionicsmaster) {
        to_iqueue = false;
        msg = SP_MASTER_AVIONICS_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_MASTER_AVIONICS_OFF_MSG;
    }

    if (fuelpump) {
        to_iqueue = false;
        msg = SP_FUEL_PUMP_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_FUEL_PUMP_OFF_MSG;
    }

    if (deice) {
        to_iqueue = false;
        msg = SP_DEICE_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_DEICE_OFF_MSG;
    }

    if (pitotheat) {
        to_iqueue = false;
        msg = SP_PITOT_HEAT_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_PITOT_HEAT_OFF_MSG;
    }

    if (cowl) {
        to_iqueue = false;
        msg = SP_COWL_CLOSED_MSG;
    } else {
        to_iqueue = false;
        msg = SP_COWL_OPEN_MSG;
    }

    if (lightspanel) {
        to_iqueue = false;
        msg = SP_LIGHTS_PANEL_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_LIGHTS_PANEL_OFF_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightsbeacon) {
        to_iqueue = false;
        msg = SP_LIGHTS_BEACON_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_LIGHTS_BEACON_OFF_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightsnav) {
        to_iqueue = false;
        msg = SP_LIGHTS_NAV_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_LIGHTS_NAV_OFF_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightsstrobe) {
        to_iqueue = false;
        msg = SP_LIGHTS_STROBE_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_LIGHTS_STROBE_OFF_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightstaxi) {
        to_iqueue = false;
        msg = SP_LIGHTS_TAXI_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_LIGHTS_TAXI_OFF_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightslanding) {
        to_iqueue = false;
        msg = SP_LIGHTS_LANDING_ON_MSG;
    } else {
        to_iqueue = false;
        msg = SP_LIGHTS_LANDING_OFF_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (gearleverup) {
        to_iqueue = false;
        msg = SP_LANDING_GEAR_UP_MSG;
    }

    if (gearleverdown) {
        to_iqueue = false;
        msg = SP_LANDING_GEAR_DOWN_MSG;

        x = new uint32_t;
        *x = SP_ALL_GREEN_SCRN;
        ojq->post(new myjob(x));
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

}

#define DO_LPRINTFS 0
#if DO_LPRINTFS
char gTmp2[100];
#endif
/**
 *
 */
void ToPanelThread::execute() {
    message* msg;
    uint32_t* p;
    uint32_t d1;
    uint32_t d2;

    switch(mProduct) {
    case RP_PROD_ID:
        init = &ToPanelThread::rp_init;
        proc_msg = &ToPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        init = &ToPanelThread::mp_init;
        proc_msg = &ToPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        init = &ToPanelThread::sp_init;
        proc_msg = &ToPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        break;
    }

    while (threads_run) {
        mState->wait();

        if (mDoInit) {
            (this->*init)();
        }

        // TODO: figure out the best sleep time!
//        psleep(100);

        // message from the xplane side or looped back
        // from FromPanelThread::mp_processing
        msg = ojq->getmessage(MSG_WAIT);

        if (msg) {
            p = ((myjob*) msg)->buf;
            // default to single message
            d1 = p[0];
            d2 = 0;

            // check for a multi-part message
            if (d1 == MP_MPM) {
                d1 = p[1];
                d2 = p[2];
            }
#if DO_LPRINTFS
sprintf(gTmp2, "Saitek ProPanels Plugin: ToPanelThread::execute %d:%d \n", d1, d2);
LPRINTF(gTmp2);
#endif
            (this->*proc_msg)(d1, d2);
        }
        delete msg;
    }

    LPRINTF("Saitek ProPanels Plugin: ToPanelThread goodbye\n");
}


void ToPanelThread::mp_init() {
    mDoInit = false;
    (this->*proc_msg)(MP_ZERO_SCRN_MSG, 0);
}


void ToPanelThread::sp_init() {
//    mDoInit = false;
    // check hid before making calls
    // set mDoInit
}


void ToPanelThread::rp_init() {
//    mDoInit = false;
    // check hid before making calls
    // set mDoInit
}


/**
 *
 */
void ToPanelThread::rp_processing(uint32_t msg, uint32_t data) {
}


/**
 *
 */
inline void ToPanelThread::mp_led_update(uint32_t x, uint32_t y, uint32_t s, uint8_t m[]) {
    m[0] = 0x00;
    m[1] = ((x >> 16) & 0xFF);
    m[2] = ((x >> 12) & 0xFF);
    m[3] = ((x >>  8) & 0xFF);
    m[4] = ((x >>  4) & 0xFF);
    m[5] = ((x >>  0) & 0xFF);
    m[6] = s;
    m[7] = ((y >> 12) & 0xFF);
    m[8] = ((y >>  8) & 0xFF);
    m[9] = ((y >>  4) & 0xFF);
    m[10] = ((y >>  0) & 0xFF);
}


/**
 *
 */
void ToPanelThread::mp_processing(uint32_t msg, uint32_t u32data) {
    static uint32_t HdgTuneUpCnt = 0;
    static uint32_t HdgTuneDnCnt = 0;
    static uint32_t CrsTuneUpCnt = 0;
    static uint32_t CrsTuneDnCnt = 0;
    static uint32_t IasTuneUpCnt = 0;
    static uint32_t IasTuneDnCnt = 0;
    static uint32_t VsTuneUpCnt = 0;
    static uint32_t VsTuneDnCnt = 0;
    static uint32_t AltTuneUpCnt = 0;
    static uint32_t AltTuneDnCnt = 0;

    bool data = (u32data == 0) ? false : true;

    switch(msg) {
    case SYS_TIC_MSG:
        if (mBtns.ap) {
            // do button updates
        }
        return;
    case AVIONICS_ON_MSG:
        mAvionicsOn = true;
        return;
    case AVIONICS_OFF_MSG:
        mAvionicsOn = false;
        return;
    case BAT1_ON_MSG:
        mBat1On = true;
        return;
    case BAT1_OFF_MSG:
        mBat1On = false;
        return;
    case MP_BLANK_SCRN_MSG:
        hid_send_feature_report((hid_device*)mHid, mp_blank_panel, sizeof(mp_blank_panel));
        return;
    case MP_PLANE_CRASH_MSG:
        mDoInit = true;
        hid_send_feature_report((hid_device*)mHid, mp_blank_panel, sizeof(mp_blank_panel));
        return;
    case MP_ZERO_SCRN_MSG:
        hid_send_feature_report((hid_device*)mHid, mp_zero_panel, sizeof(mp_zero_panel));
        return;
    default:
        break;
    }

// TODO: check if the flight director is off
    bool send = true;
    uint32_t tmp1 = 0;
    uint32_t tmp2 = 0x0A0A0A0A;
    if (mAvionicsOn && mBat1On) {
        switch(msg) {
// TODO: handle the proper states and flash when in armed mode
// XXX: refactor this code
        case MP_BTN_AP_ARMED_MSG:
        case MP_BTN_AP_ON_MSG:
            if (mBtns.ap != 1) {
                mBtns.ap = 1;
                set_bit(&mReport[11], MP_APBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_AP_OFF_MSG:
            if (mBtns.ap != 0) {
                mBtns.ap = 0;
                clear_bit(&mReport[11], MP_APBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_HDG_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.hdg != 1) {
                mBtns.hdg = 1;
                set_bit(&mReport[11], MP_HDGBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_HDG_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.hdg != 0) {
                mBtns.hdg = 0;
                clear_bit(&mReport[11], MP_HDGBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_HDG_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_HDGBTN_BITPOS);
            mBtns.hdg = get_bit(&mReport[11], MP_HDGBTN_BITPOS);
            break;
        case MP_BTN_NAV_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.nav != 1) {
                mBtns.nav = 1;
                set_bit(&mReport[11], MP_NAVBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_NAV_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.nav != 0) {
                mBtns.nav = 0;
                clear_bit(&mReport[11], MP_NAVBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_NAV_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_NAVBTN_BITPOS);
            mBtns.nav = get_bit(&mReport[11], MP_NAVBTN_BITPOS);
            break;
        case MP_BTN_IAS_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.ias != 1) {
                mBtns.ias = 1;
                set_bit(&mReport[11], MP_IASBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_IAS_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.ias != 0) {
                mBtns.ias = 0;
                clear_bit(&mReport[11], MP_IASBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_IAS_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_IASBTN_BITPOS);
            mBtns.ias = get_bit(&mReport[11], MP_IASBTN_BITPOS);
            break;
        case MP_BTN_ALT_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.alt != 1) {
                mBtns.alt = 1;
                set_bit(&mReport[11], MP_ALTBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_ALT_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.alt != 0) {
                mBtns.alt = 0;
                clear_bit(&mReport[11], MP_ALTBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_ALT_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_ALTBTN_BITPOS);
            mBtns.alt = get_bit(&mReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_VS_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.vs != 1) {
                mBtns.vs = 1;
                set_bit(&mReport[11], MP_VSBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_VS_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.vs != 0) {
                mBtns.vs = 0;
                clear_bit(&mReport[11], MP_VSBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_VS_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_ALTBTN_BITPOS);
            mBtns.vs = get_bit(&mReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_APR_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.apr != 1) {
                mBtns.apr = 1;
                set_bit(&mReport[11], MP_APRBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_APR_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.apr != 0) {
                mBtns.apr = 0;
                clear_bit(&mReport[11], MP_APRBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_APR_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_APRBTN_BITPOS);
            mBtns.apr = get_bit(&mReport[11], MP_APRBTN_BITPOS);
            break;
        case MP_BTN_REV_CAPT_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.rev != 1) {
                mBtns.rev = 1;
                set_bit(&mReport[11], MP_REVBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_REV_OFF_MSG:
            if (!mBtns.ap) { send = false; break; }
            if (mBtns.rev != 0) {
                mBtns.rev = 0;
                clear_bit(&mReport[11], MP_REVBTN_BITPOS);
            } else {
                send = false;
            }
            break;
        case MP_BTN_REV_ARMED_MSG:
            if (!mBtns.ap) { send = false; break; }
            toggle_bit(&mReport[11], MP_REVBTN_BITPOS);
            mBtns.rev = get_bit(&mReport[11], MP_REVBTN_BITPOS);
            break;
        case MP_KNOB_ALT_POS_MSG:
            if (mKnobPos != 1) {
                mKnobPos = 1;
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)(abs((int)mModeVals.vs)), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
            } else {
                send = false;
            }
            break;
        case MP_KNOB_VS_POS_MSG:
            if (mKnobPos != 2) {
                mKnobPos = 2;
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)(abs((int)mModeVals.vs)), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
            } else {
                send = false;
            }
            break;
        case MP_KNOB_IAS_POS_MSG:
            if (mKnobPos != 3) {
                mKnobPos = 3;
                tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
            } else {
                send = false;
            }
            break;
        case MP_KNOB_HDG_POS_MSG:
            if (mKnobPos != 4) {
                mKnobPos = 4;
                tmp1 = dec2bcd(mModeVals.hdg, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
            } else {
                send = false;
            }
            break;
        case MP_KNOB_CRS_POS_MSG:
            if (mKnobPos != 5) {
                mKnobPos = 5;
                tmp1 = dec2bcd(mModeVals.crs, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
            } else {
                send = false;
            }
            break;
        case MP_ALT_VAL_MSG:
            send = false;
            if (mModeVals.alt != u32data) {
                mModeVals.alt = u32data;
                if (mKnobPos == 1) {
                    tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                    tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                    mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
                    send = true;
                }
            }
            break;
        case MP_VS_VAL_POS_MSG:
           send = false;
            if (mModeVals.vs != u32data) {
                mModeVals.vs = u32data;
                mModeVals.vs_sign = MP_LED_PLUS_SIGN;
                if (mKnobPos == 2) {
                    tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                    tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                    mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                    send = true;
                }
            }
            break;
        case MP_VS_VAL_NEG_MSG:
           send = false;
            if (mModeVals.vs != u32data) {
                mModeVals.vs = u32data;
                mModeVals.vs_sign = MP_LED_MINUS_SIGN;
                if (mKnobPos == 2) {
                    tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                    tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                    mp_led_update(tmp1, tmp2, MP_LED_MINUS_SIGN, mReport);
                    send = true;
                }
            }
            break;
        case MP_IAS_VAL_MSG:
           send = false;
            if (mModeVals.ias != u32data) {
                mModeVals.ias = u32data;
                if (mKnobPos == 3) {
                    tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                    mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                    send = true;
                }
            }
            break;
        case MP_HDG_VAL_MSG:
           send = false;
            if (mModeVals.hdg != u32data) {
                mModeVals.hdg = u32data;
                if (mKnobPos == 4) {
                    tmp1 = dec2bcd(mModeVals.hdg, 3) | 0xAAAAA000;
                    mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                    send = true;
                }
            }
            break;
        case MP_CRS_VAL_MSG:
           send = false;
            if (mModeVals.crs != u32data) {
                mModeVals.crs = u32data;
                if (mKnobPos == 5) {
                    tmp1 = dec2bcd(mModeVals.crs, 3) | 0xAAAAA000;
                    mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                    send = true;
                }
            }
            break;
        case MP_UPDATE_LEDS:
#if DO_LPRINTFS
sprintf(gTmp2, "Saitek ProPanels Plugin: ToPanelThread::mp_processing knobpos %d \n", mKnobPos);
LPRINTF(gTmp2);
#endif
            switch (mKnobPos) {
            case 1:
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)(abs((int)mModeVals.vs)), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
                break;
            case 2:
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
                break;
            case 3:
                tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                break;
            case 4:
                tmp1 = dec2bcd(mModeVals.hdg, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                break;
            case 5:
                tmp1 = dec2bcd(mModeVals.crs, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mReport);
                break;
            }
            if (mBtns.ap == 1) {
                set_bit(&mReport[11], MP_APBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_APBTN_BITPOS);
            }
            if (mBtns.hdg == 0) {
                set_bit(&mReport[11], MP_HDGBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_HDGBTN_BITPOS);
            }
            if (mBtns.nav == 1) {
                set_bit(&mReport[11], MP_NAVBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_NAVBTN_BITPOS);
            }
            if (mBtns.ias == 1) {
                set_bit(&mReport[11], MP_IASBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_IASBTN_BITPOS);
            }
            if (mBtns.alt == 1) {
                set_bit(&mReport[11], MP_ALTBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_ALTBTN_BITPOS);
            }
            if (mBtns.vs == 1) {
                set_bit(&mReport[11], MP_VSBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_VSBTN_BITPOS);
            }
            if (mBtns.apr == 1) {
                set_bit(&mReport[11], MP_APRBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_APRBTN_BITPOS);
            }
            if (mBtns.rev == 1) {
                set_bit(&mReport[11], MP_REVBTN_BITPOS);
            } else {
                clear_bit(&mReport[11], MP_REVBTN_BITPOS);
            }
//            case MP_AUTOTHROTTLE_ON_CMD_MSG:
//                send = false;
//                mAthlOn = true;
//                break;
//            case MP_AUTOTHROTTLE_OFF_CMD_MSG:
//                send = false;
//                mAthlOn = false;
//                break;
            default:
                // TODO: log error
                send = false;
                break;
        case MP_ALT_UP_CMD_MSG:
            send = false;
            AltTuneUpCnt += 1;
            if (AltTuneUpCnt >= gMpTuningThresh) {
                AltTuneUpCnt = 0;
                mModeVals.alt += 1;
                if (mKnobPos == 1) {
                    tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                    tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                    mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
                    send = true;
                }
            }
            AltTuneDnCnt = 0;
            VsTuneUpCnt = VsTuneDnCnt = 0;
            IasTuneUpCnt = IasTuneDnCnt = 0;
            HdgTuneUpCnt = HdgTuneDnCnt = 0;
            CrsTuneUpCnt = CrsTuneDnCnt = 0;
            break;
// TODO: if LED latency is too high implement these
        case MP_ALT_DN_CMD_MSG:
//            send = false;
//            AltTuneDnCnt += 1;
//            if (AltTuneDnCnt >= gMpTuningThresh) {
//                AltTuneDnCnt = 0;
//                mModeVals.alt += 1;
//                if (mKnobPos == 1) {
//                    tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
//                    tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
//                    mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mReport);
//                    send = true;
//                }
//            }
//            AltTuneUpCnt = 0;
//            VsTuneUpCnt = VsTuneDnCnt = 0;
//            IasTuneUpCnt = IasTuneDnCnt = 0;
//            HdgTuneUpCnt = HdgTuneDnCnt = 0;
//            CrsTuneUpCnt = CrsTuneDnCnt = 0;
            break;
        case MP_VS_UP_CMD_MSG:
            break;
        case MP_VS_DN_CMD_MSG:
            break;
        case MP_IAS_UP_CMD_MSG:
            break;
        case MP_IAS_DN_CMD_MSG:
            break;
        case MP_HDG_UP_CMD_MSG:
            break;
        case MP_HDG_DN_CMD_MSG:
            break;
        case MP_CRS_UP_CMD_MSG:
            break;
        case MP_CRS_DN_CMD_MSG:
            break;
        }
        if (send) {
            hid_send_feature_report((hid_device*)mHid, mReport, sizeof(mReport));
        }
    } else if (!mAvionicsOn || !mBat1On && !mDoInit) {
        hid_send_feature_report((hid_device*)mHid, mp_blank_panel, sizeof(mp_blank_panel));
        mModeVals.vs = 0;
        mModeVals.crs = 0;
        mModeVals.hdg = 0;
        mModeVals.alt = 0;
        mModeVals.ias = 0;
    }
}

/**
 *
 */
void ToPanelThread::sp_processing(uint32_t msg, uint32_t u32data) {
    LPRINTF("Saitek ProPanels Plugin: ToPanelThread::sp_processing\n");

	bool data = true;
    if (!u32data) {
        data = false;
    }

// TODO: state information?
    switch(msg) {
    case SP_ALL_GREEN_SCRN:
        hid_send_feature_report((hid_device*)mHid, sp_green_panel, sizeof(sp_green_panel));
        return;
    default:
        hid_send_feature_report((hid_device*)mHid, sp_blank_panel, sizeof(sp_blank_panel));
        break;
    }

}


/**
 *
 */
void PanelsCheckThread::execute() {
    pexchange((int*)&pc_run, true);
#ifdef DO_USBPANEL_CHECK
    void* p;
#endif

// TODO: flush the queues during a pend
    while (pc_run) {
        gPcTrigger.wait();

        if (!pc_run) {
            break;
        }

#ifdef DO_USBPANEL_CHECK
        if (!gRpHidHandle) {
            if (hid_check(VENDOR_ID, RP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, RP_PROD_ID, NULL);

                if (p) {
                    pexchange((void**)&gRpHidHandle, p);
                    hid_send_feature_report((hid_device*)gRpHidHandle, hid_open_msg, sizeof(hid_open_msg));
                    gRpTrigger.post();
                }
            }
        }

        if (!gMpHidHandle) {
            //XPLMSpeakString("one");
            if (hid_check(VENDOR_ID, MP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, MP_PROD_ID, NULL);

                if (p) {
                    //XPLMSpeakString("two");
                    pexchange((void**)&gMpHidHandle, p);
                    hid_send_feature_report((hid_device*)gMpHidHandle, hid_open_msg, sizeof(hid_open_msg));
                    gMpTrigger.post();
                }
            }
        }

        if (!gSpHidHandle) {
            if (hid_check(VENDOR_ID, SP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, SP_PROD_ID, NULL);

                if (p) {
                    pexchange((void**)&gSpHidHandle, p);
                    hid_send_feature_report((hid_device*)gSpHidHandle, hid_open_msg, sizeof(hid_open_msg));
                    gSpTrigger.post();
                }
            }
        }
#endif
    }

    LPRINTF("Saitek ProPanels Plugin: PanelsCheckThread goodbye\n");
}

