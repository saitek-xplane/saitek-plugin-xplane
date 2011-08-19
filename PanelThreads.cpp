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

/*
  index
  -----
    0: NA
    1-5 : top LEDs
    6-10: bottom LEDs
    11  : push buttons
                        bit positions
                        -------------
                7   6   5   4   3   2   1   0
               REV APR  VS ALT IAS NAV HDG  AP

      ALT
    0 - 5 | 6 - 10 | 11

Multi Panel messages:
 - 1 message when mode switch is turned
 - 1 message for autothrottle toggle switch
 - 2 messages for flap switch
 - 2 messages for a button press
*/

USING_PTYPES

#define toggle_bit(c, pos) (*(c) ^= (0x01 << pos))

enum {
    LED1_BYTE_START = 1, // top
    LED1_BYTE_CNT   = 5, // bottom

    LED2_BYTE_START = 6,
    LED2_BYTE_CNT   = 5,

    BTNS_BYTE_INDEX = 11,
    BTNS_BYTE_CNT   = 1,

    MINUS_SIGN      = 0x0E,
};

//static void toggle_bit(unsigned char* c, int pos);

const int MSG_MYJOB         = MSG_USER + 1;
int volatile pc_run         = false;
int volatile threads_run    = false;

// panel threads
hid_device *volatile gRpHandle = NULL;
hid_device *volatile gMpHandle = NULL;
hid_device *volatile gSpHandle = NULL;

// index[0] - report ID, which is always zero
// TODO: radio panel message
const unsigned char rp_blank_panel[13] = {};

const unsigned char mp_blank_panel[13] = {0x00, 0x0A, 0x0A, 0x0A, 0x0A,
                                          0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                                          0x0A, 0x00, 0x00};

const unsigned char mp_zero_panel[13] = {0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00};

// TODO: switch panel message
const unsigned char sp_blank_panel[13] = {};

trigger     gPcTrigger(true, false);
trigger     gRpTrigger(false, false);
trigger     gMpTrigger(false, false);
trigger     gSpTrigger(false, false);

// Radio Panel resources
jobqueue    gRp_ojq;
jobqueue    gRp_ijq;
jobqueue    gRp_sjq;

// Multi Panel resources
jobqueue    gMp_ijq;
jobqueue    gMp_ojq;
jobqueue    gMp_sjq;

// Switch Panel resources
jobqueue    gSp_ijq;
jobqueue    gSp_ojq;
jobqueue    gSp_sjq;

/**
 *
 */
void close_hid(hid_device* dev) {

// TODO: flush the queues?
    if (dev) {
        if (dev == gRpHandle) {
            hid_send_feature_report(gRpHandle, rp_blank_panel, sizeof(rp_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gRpHandle), NULL);
            gRpTrigger.reset();
        } else if (dev == gMpHandle) {
            hid_send_feature_report(gMpHandle, mp_blank_panel, sizeof(mp_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gMpHandle), NULL);
            gMpTrigger.reset();
        } else if (dev == gSpHandle) {
            hid_send_feature_report(gSpHandle, sp_blank_panel, sizeof(sp_blank_panel));
            hid_close(dev);
            pexchange((void**)(&gSpHandle), NULL);
            gSpTrigger.reset();
        }
    }
}

/**
 *
 */
bool init_hid(hid_device* volatile* dev, unsigned short prod_id) {

    pexchange((void**)dev, (void*)hid_open(VENDOR_ID, prod_id, NULL));

    if (*dev) {
        return true;
    }

    return false;
}


/**
 *
 */
void FromPanelThread::execute() {

    switch(mProduct) {
    case RP_PROD_ID:
        proc_msg = &FromPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        proc_msg = &FromPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        proc_msg = &FromPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        break;
    }

    while (threads_run) {
        mState->wait();

        mTmp = 0;

        if (!hid) {
            psleep(100); // what's a good timeout?
            continue;
        }

        // TODO: use hid_read_timeout?
        if ((mRes = hid_read((hid_device*)hid, (uint8_t*)&mTmp, sizeof(uint32_t))) <= 0) {
            if (mRes == HID_ERROR) {
                // TODO: log error
            }
        }

        (this->*proc_msg)(mTmp);
    }

    DPRINTF("Saitek ProPanels Plugin: FromPanelThread goodbye\n");
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

    uint32_t btns = msg & READ_BTNS_MASK;
    uint32_t flaps =  msg & READ_FLAPS_MASK;
    uint32_t trim =  msg & READ_TRIM_MASK;
    uint32_t tuning =  msg & READ_TUNING_MASK;

    // When not an exclusive event, the state
    // is included with the previous events
    uint32_t knob = msg & READ_KNOB_MODE_MASK;

    // When not an exclusive event, the state
    // is included with all other events
    uint32_t autothrottle =  msg & READ_THROTTLE_MASK;

    uint32_t* x;
    bool to_iqueue = true;

    //sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%.8X \n", msg);
    //DPRINTF(tmp);

    msg = 0;

    if (btns) {
        switch(btns) {
        case READ_AP_BTN:
            msg = BTN_AP_TOGGLE;
            break;
        case READ_HDG_BTN:
            msg = BTN_HDG_TOGGLE;
            break;
        case READ_NAV_BTN:
            msg = BTN_NAV_TOGGLE;
            break;
        case READ_IAS_BTN:
            msg = BTN_IAS_TOGGLE;
            break;
        case READ_ALT_BTN:
            msg = BTN_ALT_TOGGLE;
            break;
        case READ_VS_BTN:
            msg = BTN_VS_TOGGLE;
            break;
       case READ_APR_BTN:
           msg = BTN_APR_TOGGLE;
            break;
        case READ_REV_BTN:
            msg = BTN_REV_TOGGLE;
            break;
        default:
            to_iqueue = false;
            // TODO: log error
            break;
        }
    } else if (flaps) {
        to_iqueue = false;

        if (flaps == READ_FLAPS_UP) {
            msg = FLAPS_UP;
        } else if (flaps == READ_FLAPS_DN) {
            msg = FLAPS_DN;
        } else {
            // TODO: log error
        }
    } else if (trim) {
        to_iqueue = false;
// TODO: fine & coarse grained adjustment
        if (trim == READ_TRIM_UP) {
            msg = PITCHTRIM_UP;
        } else if (trim == READ_TRIM_DOWN) {
            msg = PITCHTRIM_DN;
        } else {
            // TODO: log error
        }
    } else if (tuning) {
// TODO: fine & coarse grained adjustment
        if (tuning == READ_TUNING_RIGHT) {
            msg = TUNING_RIGHT;
        } else if (tuning == READ_TUNING_LEFT) {
            msg = TUNING_LEFT;
        } else {
            to_iqueue = false;
            // TODO: log error
        }
    }

    if (msg) {
        x = new uint32_t; *x = msg;
        ijq->post(new myjob(x));

//        if (to_iqueue) {
//            x = new uint32_t; *x = msg;
//            ojq->post(new myjob(x));
//        }

        msg = 0;
    }

    if (knob) {
        switch(knob) {
        case READ_KNOB_ALT:
            msg = KNOB_ALT_POS;
            break;
        case READ_KNOB_VS:
            msg = KNOB_VS_POS;
            break;
        case READ_KNOB_IAS:
            msg = KNOB_IAS_POS;
            break;
        case READ_KNOB_HDG:
            msg = KNOB_HDG_POS;
            break;
        case READ_KNOB_CRS:
            msg = KNOB_CRS_POS;
            break;
        default:
            // TODO: log error
            break;
        }
    }

    if (msg) {
        // x-plane doesn't care about the panel knob
//        x = new uint32_t; *x = msg;
//        ijq->post(new myjob(x));

        x = new uint32_t; *x = msg;
        ojq->post(new myjob(x));
    }

    msg = (autothrottle > 0) ? AUTOTHROTTLE_ON : AUTOTHROTTLE_OFF;

    x = new uint32_t; *x = msg;
    ijq->post(new myjob(x));

// ToPanelThread doesn't care the auto throttle switch
//    x = new uint32_t; *x = msg;
//    ojq->post(new myjob(x));
}


/**
 *
 */
void FromPanelThread::sp_processing(uint32_t msg) {

}


/**
 *
 */
void ToPanelThread::execute() {

    message* msg;
    uint32_t* p;
    uint32_t d1 ;
    uint32_t d2;

    switch(mProduct) {
    case RP_PROD_ID:
        proc_msg = &ToPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        proc_msg = &ToPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        proc_msg = &ToPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        break;
    }

//    memset(mBuf, 0, OUT_BUF_CNT);

    while (threads_run) {
        mState->wait();

        // TODO: figure out the best sleep time!
//        psleep(100);

        msg = ojq->getmessage(MSG_WAIT);

        if (msg) {
            p = ((myjob*) msg)->buf;

            d1 = p[0];
            d2 = 0;
            if (d1 == MPM) {
                d1 = p[2];
                d2 = p[3];
            }

            (this->*proc_msg)(d1, d2);
            delete msg;
        }
    }

    DPRINTF("Saitek ProPanels Plugin: ToPanelThread goodbye\n");
}


/**
 *
 */
void ToPanelThread::rp_processing(uint32_t msg, uint32_t data) {

}

#define LED_UPDATE(x, y, s) {  \
             mReport[0] = 0; \
             mReport[1] = ((x >> 16) && 0xFF); mReport[2] = ((x >> 12) && 0xFF); \
             mReport[3] = ((x >> 8) && 0xFF); mReport[4] = ((x >> 4) && 0xFF); \
             mReport[5] = (x && 0xFF); \
             mReport[6] = s; \
             mReport[7] = ((y >> 12) && 0xFF); mReport[8] = ((y >> 8) && 0xFF); \
             mReport[9] = ((y >> 4) && 0xFF); mReport[10] = (y && 0xFF); \
            }

/**
 *
 */
void ToPanelThread::mp_processing(uint32_t msg, uint32_t data) {

// TODO: state information?

    switch(msg) {
    case AVIONICS_ON:
        if (!mAvionicsOn) {
            mAvionicsOn = true;
            if (mBat1On) {
                hid_send_feature_report((hid_device*)hid, mReport, sizeof(mReport));
            }
        }
        return;
    case AVIONICS_OFF:
        if (mAvionicsOn) {
            mAvionicsOn = false;
            if (!mBat1On) {
                hid_send_feature_report((hid_device*)hid, mp_blank_panel, sizeof(mp_blank_panel));
            }
        }
        return;
    case BAT1_ON:
        if (!mBat1On) {
            mBat1On = true;
            if (mAvionicsOn) {
                hid_send_feature_report((hid_device*)hid, mReport, sizeof(mReport));
            }
        }
        return;
    case BAT1_OFF:
        if (mBat1On) {
            mBat1On = false;
            if (!mAvionicsOn) {
                hid_send_feature_report((hid_device*)hid, mp_blank_panel, sizeof(mp_blank_panel));
            }
        }
        return;
    case MP_BLANK_SCRN:
        hid_send_feature_report((hid_device*)hid, mp_blank_panel, sizeof(mp_blank_panel));
        return;
    case MP_ZERO_SCRN:
        hid_send_feature_report((hid_device*)hid, mp_zero_panel, sizeof(mp_zero_panel));
        return;
    default:
        break;
    }

    bool send = true;
    uint32_t tmp1 = 0;
    uint32_t tmp2 = 0x0A0A0A0A;

// TODO: flash when in armed mode
    if (mAvionicsOn && mBat1On) {
        switch(msg) {
// TODO: handle tuning messages?
        // case TUNING_RIGHT:
        //     break;
        // case TUNING_LEFT:
        //     break;
        case BTN_AP_ON:
        case BTN_AP_OFF:
        case BTN_AP_ARMED:
            if (mBtns.ap != data) {
                mBtns.ap = data;
                toggle_bit(&mReport[11], 0);
            } else
                send = false;
            break;
        case BTN_HDG_ON:
        case BTN_HDG_OFF:
        case BTN_HDG_ARMED:
            if (mBtns.hdg != data) {
                mBtns.hdg = data;
                toggle_bit(&mReport[11], 1);
            } else
                send = false;
            break;
        case BTN_NAV_ON:
        case BTN_NAV_OFF:
        case BTN_NAV_ARMED:
            if (mBtns.nav != data) {
                mBtns.nav = data;
                toggle_bit(&mReport[11], 2);
            } else
                send = false;
            break;
        case BTN_IAS_ON:
        case BTN_IAS_OFF:
        case BTN_IAS_ARMED:
            if (mBtns.ias != data) {
                mBtns.ias = data;
                toggle_bit(&mReport[11], 3);
            } else
                send = false;
            break;
        case BTN_ALT_ON:
        case BTN_ALT_OFF:
        case BTN_ALT_ARMED:
            if (mBtns.alt != data) {
                mBtns.alt = data;
                toggle_bit(&mReport[11], 4);
            } else
                send = false;
            break;
        case BTN_VS_ON:
        case BTN_VS_OFF:
        case BTN_VS_ARMED:
            if (mBtns.vs != data) {
                mBtns.vs = data;
                toggle_bit(&mReport[11], 5);
            } else
                send = false;
            break;
        case BTN_APR_ON:
        case BTN_APR_OFF:
        case BTN_APR_ARMED:
            if (mBtns.apr != data) {
                mBtns.apr = data;
                toggle_bit(&mReport[11], 6);
            } else
                send = false;
            break;
        case BTN_REV_ON:
        case BTN_REV_OFF:
        case BTN_REV_TOGGLE:
            if (mBtns.rev != data) {
                mBtns.rev = data;
                toggle_bit(&mReport[11], 7);
            } else
                send = false;
            break;
        case KNOB_ALT_POS:
            if (mKnobPos != 1) {
                mKnobPos = 1;
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd(abs(mModeVals.vs), 4) | 0xAAAA0000;
                LED_UPDATE(tmp1, tmp2, mModeVals.vs_sign);
            } else
                send = false;
            break;
        case KNOB_VS_POS:
            if (mKnobPos != 1) {
                mKnobPos = 2;
                tmp1 = dec2bcd(mModeVals.vs, 4) | 0xAAAA0000;
                tmp2 = dec2bcd(mModeVals.alt, 5)| 0xAAA00000;
                LED_UPDATE(tmp1, tmp2, mModeVals.vs_sign);
            } else
                send = false;
            break;
        case KNOB_IAS_POS:
            if (mKnobPos != 2) {
                mKnobPos = 3;
                tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                LED_UPDATE(tmp1, tmp2, 0xA);
            } else
                send = false;
            break;
        case KNOB_HDG_POS:
            if (mKnobPos != 3) {
                mKnobPos = 4;
                tmp1 = dec2bcd((uint32_t)mModeVals.hdg, 3) | 0xAAAAA000;
                LED_UPDATE(tmp1, tmp2, 0xA);
            } else
                send = false;
            break;
        case KNOB_CRS_POS:
            if (mKnobPos != 5) {
                mKnobPos = 5;
                tmp1 = dec2bcd((uint32_t)mModeVals.crs, 3) | 0xAAAAA000;
                LED_UPDATE(tmp1, tmp2, 0xA);
            } else
                send = false;
            break;
        case ALT_VAL:
            send = false;
            if (mModeVals.alt != data) {
                mModeVals.alt = data;
                if (mKnobPos == 1) {
                    tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                    tmp2 = dec2bcd(abs(mModeVals.vs), 4) | 0xAAAA0000;
                    LED_UPDATE(tmp1, tmp2, mModeVals.vs_sign);
                    send = true;
                }
            }
            break;
        case VS_VAL_POS:
           send = false;
            if (mModeVals.vs != data) {
                mModeVals.vs = data;
                mModeVals.vs_sign = 0x0A;
                if (mKnobPos == 2) {
                    tmp1 = dec2bcd(mModeVals.vs, 4) | 0xAAAA0000;
                    tmp2 = dec2bcd(mModeVals.alt, 5)| 0xAAA00000;
                    LED_UPDATE(tmp1, tmp2, 0x0A);
                    send = true;
                }
            }
            break;
        case VS_VAL_NEG:
           send = false;
            if (mModeVals.vs != data) {
                mModeVals.vs = data;
                mModeVals.vs_sign = 0x0E;
                if (mKnobPos == 2) {
                    tmp1 = dec2bcd(mModeVals.vs, 4) | 0xAAAA0000;
                    tmp2 = dec2bcd(mModeVals.alt, 5)| 0xAAA00000;
                    LED_UPDATE(tmp1, tmp2, 0x0E);
                    send = true;
                }
            }
            break;
        case IAS_VAL:
           send = false;
            if (mModeVals.ias != data) {
                mModeVals.ias = data;
                if (mKnobPos == 3) {
                    tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                    LED_UPDATE(tmp1, tmp2, 0xA);
                    send = true;
                }
            }
            break;
        case HDG_VAL:
           send = false;
            if (mModeVals.hdg != data) {
                mModeVals.hdg = data;
                if (mKnobPos == 4) {
                    tmp1 = dec2bcd((uint32_t)mModeVals.hdg, 3) | 0xAAAAA000;
                    LED_UPDATE(tmp1, tmp2, 0xA);
                    send = true;
                }
            }
            break;
        case CRS_VAL:
           send = false;
            if (mModeVals.crs != data) {
                mModeVals.crs = data;
                if (mKnobPos == 4) {
                    tmp1 = dec2bcd((uint32_t)mModeVals.crs, 3) | 0xAAAAA000;
                    LED_UPDATE(tmp1, tmp2, 0xA);
                    send = true;
                }
            }
            break;
        case AUTOTHROTTLE_ON:
            mAthlOn = true;
            break;
        case AUTOTHROTTLE_OFF:
            mAthlOn = false;
            break;
        default:
            // TODO: log error
            send = false;
            break;
        }

        if (send) {
            hid_send_feature_report((hid_device*)hid, mReport, sizeof(mReport));
        }
    }
}

/**
 *
 */
void ToPanelThread::sp_processing(uint32_t msg, uint32_t data) {

}


/**
 *
 */
void PanelsCheckThread::execute() {

    pexchange((int*)&pc_run, true);
#ifndef NO_PANEL_CHECK
    void* p;
#endif

// TODO: flush the queues during a pend
    while (pc_run) {

        gPcTrigger.wait();

        if (!pc_run)
            break;

#ifndef NO_PANEL_CHECK
        if (!gRpHandle) {
            if (hid_check(VENDOR_ID, RP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, RP_PROD_ID, NULL);

                if (p) {
                    pexchange((void**)&gRpHandle, p);
                    hid_send_feature_report((hid_device*)gRpHandle, hid_open_msg, sizeof(hid_open_msg));
                    gRpTrigger.post();
                }
            }
        }

        if (!gMpHandle) {
            //XPLMSpeakString("one");
            if (hid_check(VENDOR_ID, MP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, MP_PROD_ID, NULL);
                if (p) {
                    //XPLMSpeakString("two");
                    pexchange((void**)&gMpHandle, p);
                    hid_send_feature_report((hid_device*)gMpHandle, hid_open_msg, sizeof(hid_open_msg));
                    gMpTrigger.post();
                }
            }
        }

        if (!gSpHandle) {
            if (hid_check(VENDOR_ID, SP_PROD_ID)) {
                p = hid_open(&close_hid, VENDOR_ID, SP_PROD_ID, NULL);

                if (p) {
                    pexchange((void**)&gSpHandle, p);
                    hid_send_feature_report((hid_device*)gSpHandle, hid_open_msg, sizeof(hid_open_msg));
                    gSpTrigger.post();
                }
            }
        }
#endif
    }

    DPRINTF("Saitek ProPanels Plugin: PanelsCheckThread goodbye\n");
}

