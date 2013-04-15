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

#define MP_AP_OFF           (0)
#define MP_AP_ON            (1)
#define MP_AP_ARMED         (2)

#define MP_BTN_OFF          (0)
#define MP_BTN_ARMED        (1)
#define MP_BTN_CAPT         (2)

//static void toggle_bit(unsigned char* c, int pos);

const int MSG_MYJOB         = MSG_USER + 1;
int volatile gPcRun         = false;
int volatile gThreadsRun    = false;
int volatile gPluginEnabled = false;

// panel threads
hid_device *volatile gRpHidHandle = NULL;
hid_device *volatile gMpHidHandle = NULL;
hid_device *volatile gSpHidHandle = NULL;

// index[0] - report ID, which is always zero
const unsigned char rp_blank_panel[23] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char rp_zero_panel[23] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFF, 0xFF};

const unsigned char mp_blank_panel[13] = {0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                                        0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00};

const unsigned char mp_zero_panel[13] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// TODO: switch panel message
const unsigned char sp_blank_panel[2] = {0x00, 0x00};
const unsigned char sp_green_panel[2] = {0x00, 0x07};
const unsigned char sp_red_panel[2] = {0x00, 0x38};
const unsigned char sp_orange_panel[2] = {0x00, 0x3F};

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

    while (gThreadsRun) {
        mState->wait();

        if (mDoInit) {
            (this->*init)();
        }

        mTmp = 0;

        if (!mHid) {
            psleep(100); // what's a good timeout?
            continue;
        }

        if(!gPluginEnabled) {
            mRes = hid_read_timeout((hid_device*)mHid, (uint8_t*)&mTmp, sizeof(uint32_t), 100);
        } else {
            if ((mRes = hid_read((hid_device*)mHid, (uint8_t*)&mTmp, sizeof(uint32_t))) <= 0) {
                if (mRes == HID_ERROR) {
                    // TODO: log error
                }
                continue;
            }
#if DO_LPRINTFS
sprintf(gTmp1, "Saitek ProPanels Plugin: FromPanelThread::execute 0x%X \n", mTmp);
LPRINTF(gTmp1);
#endif
            if (gPluginEnabled) {
                (this->*proc_msg)(mTmp);
            }
        }
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
	mDoInit = false;
}


void FromPanelThread::rp_init() {
    // check hid before making calls
    // set mDoInit
	//mDoInit = false;
}


uint32_t rp_process_coarse_right(uint32_t knobSelection) {
	uint32_t returnMsg = 0;
	switch (knobSelection) {
	case 0x000001:
	case 0x000080:
		returnMsg = RP_COM1_COARSE_UP_CMD_MSG;
		break;
	case 0x000002:
	case 0x000100:
		returnMsg = RP_COM2_COARSE_UP_CMD_MSG;
		break;
	case 0x000004:
	case 0x000200:
		break;
	case 0x000008:
	case 0x000400:
		break;
	case 0x000010:
	case 0x000800:
		break;
	case 0x000020:
	case 0x001000:
		break;
	case 0x000040:
	case 0x002000:
		break;
	default:
		break;
	}
	return returnMsg;
}

uint32_t rp_process_coarse_left(uint32_t knobSelection) {
	uint32_t returnMsg = 0;
	switch (knobSelection) {
	case 0x000001:
	case 0x000080:
		returnMsg = RP_COM1_COARSE_DOWN_CMD_MSG;
		break;
	case 0x000002:
	case 0x000100:
		returnMsg = RP_COM2_COARSE_DOWN_CMD_MSG;
		break;
	case 0x000004:
	case 0x000200:
		break;
	case 0x000008:
	case 0x000400:
		break;
	case 0x000010:
	case 0x000800:
		break;
	case 0x000020:
	case 0x001000:
		break;
	case 0x000040:
	case 0x002000:
		break;
	default:
		break;
	}
	return returnMsg;
}

uint32_t rp_process_fine_right(uint32_t knobSelection) {
	uint32_t returnMsg = 0;
	switch (knobSelection) {
	case 0x000001:
	case 0x000080:
		returnMsg = RP_COM1_FINE_UP_CMD_MSG;
		break;
	case 0x000002:
	case 0x000100:
		returnMsg = RP_COM2_FINE_UP_CMD_MSG;
		break;
	case 0x000004:
	case 0x000200:
		break;
	case 0x000008:
	case 0x000400:
		break;
	case 0x000010:
	case 0x000800:
		break;
	case 0x000020:
	case 0x001000:
		break;
	case 0x000040:
	case 0x002000:
		break;
	default:
		break;
	}
	return returnMsg;
}

uint32_t rp_process_fine_left(uint32_t knobSelection) {
	uint32_t returnMsg = 0;
	switch (knobSelection) {
	case 0x000001:
	case 0x000080:
		returnMsg = RP_COM1_FINE_DOWN_CMD_MSG;
		break;
	case 0x000002:
	case 0x000100:
		returnMsg = RP_COM2_FINE_DOWN_CMD_MSG;
		break;
	case 0x000004:
	case 0x000200:
		break;
	case 0x000008:
	case 0x000400:
		break;
	case 0x000010:
	case 0x000800:
		break;
	case 0x000020:
	case 0x001000:
		break;
	case 0x000040:
	case 0x002000:
		break;
	default:
		break;
	}
	return returnMsg;
}

uint32_t rp_process_switch(uint32_t knobSelection) {
	uint32_t returnMsg = 0;
	switch (knobSelection) {
	case 0x000001:
	case 0x000080:
		returnMsg = RP_COM1_FLIP_CMD_MSG;
		break;
	case 0x000002:
	case 0x000100:
		returnMsg = RP_COM2_FLIP_CMD_MSG;
		break;
	case 0x000004:
	case 0x000200:
		break;
	case 0x000008:
	case 0x000400:
		break;
	case 0x000010:
	case 0x000800:
		break;
	case 0x000020:
	case 0x001000:
		break;
	case 0x000040:
	case 0x002000:
		break;
	default:
		break;
	}
	return returnMsg;
}


/**
 *
 */
void FromPanelThread::rp_processing(uint32_t msg) {
    uint32_t upperKnob = msg & RP_READ_UPPER_KNOB_MODE_MASK;
    uint32_t lowerKnob = msg & RP_READ_LOWER_KNOB_MODE_MASK;
    uint32_t upperFineTuning = msg & RP_READ_UPPER_FINE_TUNING_MASK;
    uint32_t upperCoarseTuning = msg & RP_READ_UPPER_COARSE_TUNING_MASK;
    uint32_t lowerFineTuning = msg & RP_READ_LOWER_FINE_TUNING_MASK;
    uint32_t lowerCoarseTuning = msg & RP_READ_LOWER_COARSE_TUNING_MASK;
    uint32_t upperStby = msg & RP_READ_UPPER_ACT_STBY;
    uint32_t lowerStby = msg & RP_READ_LOWER_ACT_STBY;

//    static char tmp[100];
//    sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%.8X \n", msg);
//    DPRINTF(tmp);

    uint32_t msg2= 0;
    uint32_t* x;
    msg = 0;

    if (upperCoarseTuning || upperFineTuning) {
		if (upperCoarseTuning == RP_READ_UPPER_COARSE_RIGHT) {
			msg = msg2 = rp_process_coarse_right(upperKnob);
		} else if (upperCoarseTuning == RP_READ_UPPER_COARSE_LEFT) {
			msg = msg2 = rp_process_coarse_left(upperKnob);
		} else if (upperFineTuning == RP_READ_UPPER_FINE_RIGHT) {
			msg = msg2 = rp_process_fine_right(upperKnob);
		} else if (upperFineTuning == RP_READ_UPPER_FINE_LEFT) {
			msg = msg2 = rp_process_fine_left(upperKnob);
		}
    }
    if (upperStby) {
    	msg = msg2 = rp_process_switch(upperKnob);
    }
    if (lowerCoarseTuning || lowerFineTuning) {
		if (lowerCoarseTuning == RP_READ_LOWER_COARSE_RIGHT) {
			msg = msg2 = rp_process_coarse_right(lowerKnob);
		} else if (lowerCoarseTuning == RP_READ_LOWER_COARSE_LEFT) {
			msg = msg2 = rp_process_coarse_left(lowerKnob);
		} else if (lowerFineTuning == RP_READ_LOWER_FINE_RIGHT) {
			msg = msg2 = rp_process_fine_right(lowerKnob);
		} else if (lowerFineTuning == RP_READ_LOWER_FINE_LEFT) {
			msg = msg2 = rp_process_fine_left(lowerKnob);
		}
    }
    if (lowerStby) {
    	msg = msg2 = rp_process_switch(lowerKnob);
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

    if (upperKnob) {
        switch(upperKnob) {
        case RP_READ_UPPER_KNOB_COM1:
            msg = RP_UP_KNOB_COM1_POS_MSG;
            break;
        case RP_READ_UPPER_KNOB_COM2:
            msg = RP_UP_KNOB_COM2_POS_MSG;
            break;
        case RP_READ_UPPER_KNOB_NAV1:
            msg = RP_UP_KNOB_NAV1_POS_MSG;
            break;
        case RP_READ_UPPER_KNOB_NAV2:
            msg = RP_UP_KNOB_NAV2_POS_MSG;
            break;
        case RP_READ_UPPER_KNOB_ADF:
            msg = RP_UP_KNOB_ADF_POS_MSG;
            break;
        case RP_READ_UPPER_KNOB_DME:
            msg = RP_UP_KNOB_DME_POS_MSG;
            break;
        case RP_READ_UPPER_KNOB_TRANSPNDR:
            msg = RP_UP_KNOB_XPDR_POS_MSG;
            break;
        default:
            // TODO: log error
            break;
        }
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ojq->post(new myjob(x));
    }

    msg = 0;
    if (lowerKnob) {
        switch(lowerKnob) {
        case RP_READ_LOWER_KNOB_COM1:
            msg = RP_LO_KNOB_COM1_POS_MSG;
            break;
        case RP_READ_LOWER_KNOB_COM2:
            msg = RP_LO_KNOB_COM2_POS_MSG;
            break;
        case RP_READ_LOWER_KNOB_NAV1:
            msg = RP_LO_KNOB_NAV1_POS_MSG;
            break;
        case RP_READ_LOWER_KNOB_NAV2:
            msg = RP_LO_KNOB_NAV2_POS_MSG;
            break;
        case RP_READ_LOWER_KNOB_ADF:
            msg = RP_LO_KNOB_ADF_POS_MSG;
            break;
        case RP_READ_LOWER_KNOB_DME:
            msg = RP_LO_KNOB_DME_POS_MSG;
            break;
        case RP_READ_LOWER_KNOB_TRANSPNDR:
            msg = RP_LO_KNOB_XPDR_POS_MSG;
            break;
        default:
            // TODO: log error
            break;
        }
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ojq->post(new myjob(x));
    }
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

#if 0
        // loopback to the panel
        if (msg2) {
            x = new uint32_t;
            *x = msg2;
            ojq->post(new myjob(x));
        }
#endif
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

//    static char tmp[100];
//    sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%.8X \n", msg);
//    DPRINTF(tmp);

    msg = 0;

    if (enginesknob == 0x002000) {
    	msg = SP_MAGNETOS_OFF_CMD_MSG;
    } else if (enginesknob == 0x004000) {
    	msg = SP_MAGNETOS_RIGHT_CMD_MSG;
    } else if (enginesknob == 0x008000) {
    	msg = SP_MAGNETOS_LEFT_CMD_MSG;
    } else if (enginesknob == 0x010000) {
    	msg = SP_MAGNETOS_BOTH_CMD_MSG;
    } else if (enginesknob == 0x020000) {
    	msg = SP_ENGINE_START_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (masterbat) {
        msg = SP_MASTER_BATTERY_ON_CMD_MSG;
    } else {
        msg = SP_MASTER_BATTERY_OFF_CMD_MSG;
    }

    if (masteralt) {
        msg = SP_MASTER_ALT_BATTERY_ON_CMD_MSG;
    } else {
        msg = SP_MASTER_ALT_BATTERY_OFF_CMD_MSG;
    }

    if (avionicsmaster) {
        msg = SP_MASTER_AVIONICS_ON_CMD_MSG;
    } else {
        msg = SP_MASTER_AVIONICS_OFF_CMD_MSG;
    }

    if (fuelpump) {
        msg = SP_FUEL_PUMP_ON_CMD_MSG;
    } else {
        msg = SP_FUEL_PUMP_OFF_CMD_MSG;
    }

    if (deice) {
        msg = SP_DEICE_ON_CMD_MSG;
    } else {
        msg = SP_DEICE_OFF_CMD_MSG;
    }

    if (pitotheat) {
        msg = SP_PITOT_HEAT_ON_CMD_MSG;
    } else {
        msg = SP_PITOT_HEAT_OFF_CMD_MSG;
    }

    if (cowl) {
        msg = SP_COWL_CLOSED_CMD_MSG;
    } else {
        msg = SP_COWL_OPEN_CMD_MSG;
    }

    if (lightspanel) {
        msg = SP_LIGHTS_PANEL_ON_CMD_MSG;
    } else {
        msg = SP_LIGHTS_PANEL_OFF_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightsbeacon) {
        msg = SP_LIGHTS_BEACON_ON_CMD_MSG;
    } else {
        msg = SP_LIGHTS_BEACON_OFF_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightsnav) {
        msg = SP_LIGHTS_NAV_ON_CMD_MSG;
    } else {
        msg = SP_LIGHTS_NAV_OFF_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightsstrobe) {
        msg = SP_LIGHTS_STROBE_ON_CMD_MSG;
    } else {
        msg = SP_LIGHTS_STROBE_OFF_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightstaxi) {
        msg = SP_LIGHTS_TAXI_ON_CMD_MSG;
    } else {
        msg = SP_LIGHTS_TAXI_OFF_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (lightslanding) {
        msg = SP_LIGHTS_LANDING_ON_CMD_MSG;
    } else {
        msg = SP_LIGHTS_LANDING_OFF_CMD_MSG;
    }

    if (msg) {
        x = new uint32_t;
        *x = msg;
        ijq->post(new myjob(x));

        msg = 0;
    }

    if (gearleverup) {
        msg = SP_LANDING_GEAR_UP_CMD_MSG;

        x = new uint32_t;
        *x = SP_ALL_GREEN_SCRN_MSG;
        ojq->post(new myjob(x));
    }

    if (gearleverdown) {
        msg = SP_LANDING_GEAR_DOWN_CMD_MSG;

        x = new uint32_t;
        *x = SP_ALL_RED_SCRN_MSG;
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
    memset((void*)mPanelReport, 0, sizeof(mPanelReport));

    switch(mProduct) {
    case RP_PROD_ID:
        //LPRINTF("Saitek ProPanels Plugin: ToPanelThread::execute RP_PROD_ID.\n");
        init = &ToPanelThread::rp_init;
        proc_msg = &ToPanelThread::rp_processing;
        break;
    case MP_PROD_ID:
        //LPRINTF("Saitek ProPanels Plugin: ToPanelThread::execute MP_PROD_ID.\n");
        init = &ToPanelThread::mp_init;
        proc_msg = &ToPanelThread::mp_processing;
        break;
    case SP_PROD_ID:
        //LPRINTF("Saitek ProPanels Plugin: ToPanelThread::execute SP_PROD_ID.\n");
        init = &ToPanelThread::sp_init;
        proc_msg = &ToPanelThread::sp_processing;
        break;
    default:
        // TODO: log error
        LPRINTF("Saitek ProPanels Plugin: ToPanelThread::execute error.\n");
        break;
    }

    while (gThreadsRun) {
        mState->wait();

        if (mDoInit) {
            (this->*init)();
        }

        // TODO: figure out the best sleep time!
        // 100Hz -> 10ms sleep
        // TODO: remove this line (set to prevent crash on startup).
        //psleep(10);

        // message from the xplane side or looped back
        // from FromPanelThread::mp_processing
        msg = ojq->getmessage(MSG_WAIT);

        if (msg) {
            p = ((myjob*) msg)->buf;
            // default to single message
            d1 = p[0];
            d2 = 0;

            // check for a multi-part message
            if ((d1 == MP_MPM) || (d1 == RP_MPM)) {
                d1 = p[1];
                d2 = p[2];
            }
#if DO_LPRINTFS
sprintf(gTmp2, "Saitek ProPanels Plugin: ToPanelThread::execute %d:%d \n", d1, d2);
LPRINTF(gTmp2);
#endif

            if (mHid) {
                (this->*proc_msg)(d1, d2);
            }
        }
        delete msg;
    }

    LPRINTF("Saitek ProPanels Plugin: ToPanelThread goodbye\n");
}


void ToPanelThread::mp_init() {
#if DO_LPRINTFS
LPRINTF("Saitek ProPanels Plugin: ToPanelThread::mp_init\n");
#endif
    mDoInit = false;
    if (mHid) {
        (this->*proc_msg)(MP_BLANK_SCRN_MSG, 0);
    }
}


void ToPanelThread::sp_init() {
#if DO_LPRINTFS
LPRINTF("Saitek ProPanels Plugin: ToPanelThread::sp_init\n");
#endif
    mDoInit = false;
    if (mHid) {
        (this->*proc_msg)(SP_BLANK_SCRN_MSG, 0);
    }
}


void ToPanelThread::rp_init() {
#if DO_LPRINTFS
LPRINTF("Saitek ProPanels Plugin: ToPanelThread::rp_init\n");
#endif
    mDoInit = false;
    if (mHid) {
        (this->*proc_msg)(RP_BLANK_SCRN_MSG, 0);
    }
}


inline void ToPanelThread::rp_upper_led_update(uint32_t x, uint32_t y, uint8_t m[]) {
    m[0] = 0x00;
    m[1] = ((x >> 16) & 0xFF);
    m[2] = ((x >> 12) & 0xFF);
    m[3] = ((x >>  8) & 0xFF);
    m[4] = ((x >>  4) & 0xFF);
    m[5] = ((x >>  0) & 0xFF);
    m[6] = ((y >> 16) & 0xFF);
    m[7] = ((y >> 12) & 0xFF);
    m[8] = ((y >>  8) & 0xFF);
    m[9] = ((y >>  4) & 0xFF);
    m[10] = ((y >>  0) & 0xFF);
    m[21] = 0x00;
    m[22] = 0x00;
}

inline void ToPanelThread::rp_lower_led_update(uint32_t x, uint32_t y, uint8_t m[]) {
    m[0] = 0x00;
    m[11] = ((x >> 16) & 0xFF);
    m[12] = ((x >> 12) & 0xFF);
    m[13] = ((x >>  8) & 0xFF);
    m[14] = ((x >>  4) & 0xFF);
    m[15] = ((x >>  0) & 0xFF);
    m[16] = ((y >> 16) & 0xFF);
    m[17] = ((y >> 12) & 0xFF);
    m[18] = ((y >>  8) & 0xFF);
    m[19] = ((y >>  4) & 0xFF);
    m[20] = ((y >>  0) & 0xFF);
    m[21] = 0x00;
    m[22] = 0x00;
}

/**
 *
 */
void ToPanelThread::rp_processing(uint32_t msg, uint32_t u32data) {
    bool send = true;
    uint32_t tmp1 = 0;
    uint32_t tmp2 = 0;

    switch(msg) {
    case RP_BLANK_SCRN_MSG:
        hid_send_feature_report((hid_device*)mHid, rp_blank_panel, sizeof(rp_blank_panel));
        return;
    case RP_ZERO_SCRN_MSG:
        hid_send_feature_report((hid_device*)mHid, rp_zero_panel, sizeof(rp_zero_panel));
        return;
    case RP_UP_KNOB_COM1_POS_MSG:
        if (mRpUpperKnobPos != 1) {
            mRpUpperKnobPos = 1;
            tmp1 = dec2bcd(mRpModeVals.com1, 5);
            tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
            rp_upper_led_update(tmp1, tmp2, mRpReport);
        } else {
            send = false;
        }
        break;
    case RP_UP_KNOB_COM2_POS_MSG:
        if (mRpUpperKnobPos != 2) {
            mRpUpperKnobPos = 2;
            tmp1 = dec2bcd(mRpModeVals.com2, 5);
            tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
            rp_upper_led_update(tmp1, tmp2, mRpReport);
        } else {
            send = false;
        }
        break;
    case RP_LO_KNOB_COM1_POS_MSG:
        if (mRpLowerKnobPos != 1) {
            mRpLowerKnobPos = 1;
            tmp1 = dec2bcd(mRpModeVals.com1, 5);
            tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
            rp_lower_led_update(tmp1, tmp2, mRpReport);
        } else {
            send = false;
        }
        break;
    case RP_LO_KNOB_COM2_POS_MSG:
        if (mRpLowerKnobPos != 2) {
            mRpLowerKnobPos = 2;
            tmp1 = dec2bcd(mRpModeVals.com2, 5);
            tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
            rp_lower_led_update(tmp1, tmp2, mRpReport);
        } else {
            send = false;
        }
        break;
    case RP_COM1_VAL_MSG:
        send = false;
        if (mRpModeVals.com1 != u32data) {
            mRpModeVals.com1 = u32data;
            if (mRpUpperKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            if (mRpLowerKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_lower_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
        }
        break;
    case RP_COM1_STDBY_VAL_MSG:
        send = false;
        if (mRpModeVals.com1Stdby != u32data) {
            mRpModeVals.com1Stdby = u32data;
            if (mRpUpperKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            if (mRpLowerKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_lower_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
        }
        break;
    case RP_COM2_VAL_MSG:
       send = false;
        if (mRpModeVals.com2 != u32data) {
            mRpModeVals.com2 = u32data;
            if (mRpUpperKnobPos == 2) {
                tmp1 = dec2bcd(mRpModeVals.com2, 5);
                tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            if (mRpLowerKnobPos == 2) {
                tmp1 = dec2bcd(mRpModeVals.com2, 5);
                tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
                rp_lower_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
        }
    case RP_COM2_STDBY_VAL_MSG:
        send = false;
        if (mRpModeVals.com2Stdby != u32data) {
            mRpModeVals.com2Stdby = u32data;
            if (mRpUpperKnobPos == 2) {
                tmp1 = dec2bcd(mRpModeVals.com2, 5);
                tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            if (mRpLowerKnobPos == 2) {
                tmp1 = dec2bcd(mRpModeVals.com2, 5);
                tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
                rp_lower_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
        }
        break;
    case RP_COM1_COARSE_UP_CMD_MSG:
        send = false;
        gRpUpperFineTuneUpCnt += 1;
        if (gRpUpperFineTuneUpCnt >= gRpTuningThresh) {
            mRpModeVals.com1Stdby += 1;
            if (mRpUpperKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            gRpUpperCoarseTuneUpCnt = 0;
        }
        gRpUpperCoarseTuneDownCnt = 0;
        break;
    case RP_COM1_COARSE_DOWN_CMD_MSG:
        send = false;
        gRpUpperCoarseTuneDownCnt += 1;
        if (gRpUpperCoarseTuneDownCnt >= gRpTuningThresh) {
            mRpModeVals.com1Stdby -= 1;
            if (mRpUpperKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            gRpUpperCoarseTuneDownCnt = 0;
        }
        gRpUpperCoarseTuneUpCnt = 0;
        break;
    case RP_COM1_FINE_UP_CMD_MSG:
        send = false;
        gRpUpperFineTuneUpCnt += 1;
        if (gRpUpperFineTuneUpCnt >= gRpTuningThresh) {
            mRpModeVals.com1Stdby += 1;
            if (mRpUpperKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            gRpUpperFineTuneUpCnt = 0;
        }
        gRpUpperFineTuneDownCnt = 0;
        break;
    case RP_COM1_FINE_DOWN_CMD_MSG:
        send = false;
        gRpUpperFineTuneDownCnt += 1;
        if (gRpUpperFineTuneDownCnt >= gRpTuningThresh) {
            mRpModeVals.com1Stdby -= 1;
            if (mRpUpperKnobPos == 1) {
                tmp1 = dec2bcd(mRpModeVals.com1, 5);
                tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
                rp_upper_led_update(tmp1, tmp2, mRpReport);
                send = true;
            }
            gRpUpperFineTuneDownCnt = 0;
        }
        gRpUpperFineTuneUpCnt = 0;
        break;
    case RP_COM2_COARSE_UP_CMD_MSG:
    case RP_COM2_COARSE_DOWN_CMD_MSG:
    case RP_COM2_FINE_UP_CMD_MSG:
    case RP_COM2_FINE_DOWN_CMD_MSG:
         send = false;
        if (mRpUpperKnobPos == 2) {
            tmp1 = dec2bcd(mRpModeVals.com2, 5);
            tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
            rp_upper_led_update(tmp1, tmp2, mRpReport);
            send = true;
        }
        if (mRpLowerKnobPos == 2) {
            tmp1 = dec2bcd(mRpModeVals.com2, 5);
            tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
            rp_lower_led_update(tmp1, tmp2, mRpReport);
            send = true;
        }
        break;
    case RP_COM1_FLIP_CMD_MSG:
         send = false;
         if (mRpUpperKnobPos == 1) {
             tmp1 = dec2bcd(mRpModeVals.com1, 5);
             tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
             rp_upper_led_update(tmp1, tmp2, mRpReport);
             send = true;
         }
         if (mRpLowerKnobPos == 1) {
             tmp1 = dec2bcd(mRpModeVals.com1, 5);
             tmp2 = dec2bcd(mRpModeVals.com1Stdby, 5);
             rp_lower_led_update(tmp1, tmp2, mRpReport);
             send = true;
         }
        break;
    case RP_COM2_FLIP_CMD_MSG:
         send = false;
         if (mRpUpperKnobPos == 2) {
             tmp1 = dec2bcd(mRpModeVals.com2, 5);
             tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
             rp_upper_led_update(tmp1, tmp2, mRpReport);
             send = true;
         }
         if (mRpLowerKnobPos == 2) {
             tmp1 = dec2bcd(mRpModeVals.com2, 5);
             tmp2 = dec2bcd(mRpModeVals.com2Stdby, 5);
             rp_lower_led_update(tmp1, tmp2, mRpReport);
             send = true;
         }
        break;

    default:
        send = false;
        break;
    }
    if (send) {
        hid_send_feature_report((hid_device*)mHid, mRpReport, sizeof(mRpReport));
    }
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

    bool send;

    switch(msg) {
    case SYS_TIC_MSG:
        if (mBtns.ap == MP_AP_OFF) {
            hid_send_feature_report((hid_device*)mHid, mp_blank_panel, sizeof(mp_blank_panel));
        } else {
            send = false;
            if (mBtns.ap == MP_AP_ON) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_APBTN_BITPOS);
            }
            if (mBtns.hdg == MP_BTN_ARMED) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_HDGBTN_BITPOS);
            }
            if (mBtns.nav == MP_BTN_ARMED) {
                toggle_bit(&mPanelReport[11], MP_NAVBTN_BITPOS);
            }
            if (mBtns.ias == MP_BTN_ARMED) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_IASBTN_BITPOS);
            }
            if (mBtns.alt == MP_BTN_ARMED) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            }
            if (mBtns.vs == MP_BTN_ARMED) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            }
            if (mBtns.apr == MP_BTN_ARMED) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_APRBTN_BITPOS);
            }
            if (mBtns.rev == MP_BTN_ARMED) {
                send = true;
                toggle_bit(&mPanelReport[11], MP_REVBTN_BITPOS);
            }
            if (send) {
                hid_send_feature_report((hid_device*)mHid, mPanelReport, sizeof(mPanelReport));
            }
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

    uint32_t tmp1 = 0;
    uint32_t tmp2 = 0x0A0A0A0A;
    send = true;
    if (mAvionicsOn && mBat1On) {
        switch(msg) {
// TODO: handle the proper states and flash when in armed mode
// XXX: refactor this code
        case MP_BTN_AP_ARMED_MSG:
            mBtns.ap = MP_AP_ARMED;
            set_bit(&mPanelReport[11], MP_APBTN_BITPOS);
            break;
        case MP_BTN_AP_ON_MSG:
            mBtns.ap = MP_AP_ON;
            set_bit(&mPanelReport[11], MP_APBTN_BITPOS);
            break;
        case MP_BTN_AP_OFF_MSG:
            mBtns.ap = MP_AP_OFF;
            memset((void*)mPanelReport, 0, sizeof(mPanelReport));
//            clear_bit(&mPanelReport[11], MP_APBTN_BITPOS);
            break;
        case MP_BTN_HDG_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.hdg = 2;
            clear_bit(&mPanelReport[11], MP_HDGBTN_BITPOS);
            break;
        case MP_BTN_HDG_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.hdg = 0;
            clear_bit(&mPanelReport[11], MP_HDGBTN_BITPOS);
            break;
        case MP_BTN_HDG_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.hdg = 1;
            set_bit(&mPanelReport[11], MP_HDGBTN_BITPOS);
            break;
        case MP_BTN_NAV_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.nav = 2;
            set_bit(&mPanelReport[11], MP_NAVBTN_BITPOS);
            break;
        case MP_BTN_NAV_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.nav = 0;
            clear_bit(&mPanelReport[11], MP_NAVBTN_BITPOS);
            break;
        case MP_BTN_NAV_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.nav = 1;
            set_bit(&mPanelReport[11], MP_NAVBTN_BITPOS);
            break;
        case MP_BTN_IAS_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.ias = 2;
            set_bit(&mPanelReport[11], MP_IASBTN_BITPOS);
            break;
        case MP_BTN_IAS_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.ias = 0;
            clear_bit(&mPanelReport[11], MP_IASBTN_BITPOS);
            break;
        case MP_BTN_IAS_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.ias = 1;
            set_bit(&mPanelReport[11], MP_IASBTN_BITPOS);
            break;
        case MP_BTN_ALT_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.alt = 2;
            set_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_ALT_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.alt = 0;
            clear_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_ALT_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.alt = 1;
            set_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_VS_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.vs = 2;
            set_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_VS_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.vs = 0;
            clear_bit(&mPanelReport[11], MP_VSBTN_BITPOS);
            break;
        case MP_BTN_VS_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.vs = 1;
            set_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            break;
        case MP_BTN_APR_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.apr = 2;
            toggle_bit(&mPanelReport[11], MP_APRBTN_BITPOS);
            break;
        case MP_BTN_APR_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.apr = 0;
            clear_bit(&mPanelReport[11], MP_APRBTN_BITPOS);
            break;
        case MP_BTN_APR_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.apr = 1;
            toggle_bit(&mPanelReport[11], MP_APRBTN_BITPOS);
            break;
        case MP_BTN_REV_CAPT_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.rev = 2;
            toggle_bit(&mPanelReport[11], MP_REVBTN_BITPOS);
            break;
        case MP_BTN_REV_OFF_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.rev = 0;
            clear_bit(&mPanelReport[11], MP_REVBTN_BITPOS);
            break;
        case MP_BTN_REV_ARMED_MSG:
            if (mBtns.ap == MP_AP_OFF) { send = false; break; }
            mBtns.rev = 1;
            toggle_bit(&mPanelReport[11], MP_REVBTN_BITPOS);
            break;
        case MP_KNOB_ALT_POS_MSG:
            mKnobPos = 1;
            tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
            tmp2 = dec2bcd((uint32_t)(abs((int)mModeVals.vs)), 4) | 0xAAAA0000;
            mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
            break;
        case MP_KNOB_VS_POS_MSG:
            mKnobPos = 2;
            tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
            tmp2 = dec2bcd((uint32_t)(abs((int)mModeVals.vs)), 4) | 0xAAAA0000;
            mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
            break;
        case MP_KNOB_IAS_POS_MSG:
            mKnobPos = 3;
            tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
            mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
            break;
        case MP_KNOB_HDG_POS_MSG:
            mKnobPos = 4;
            tmp1 = dec2bcd(mModeVals.hdg, 3) | 0xAAAAA000;
            mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
            break;
        case MP_KNOB_CRS_POS_MSG:
            mKnobPos = 5;
            tmp1 = dec2bcd(mModeVals.crs, 3) | 0xAAAAA000;
            mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
            break;
        case MP_ALT_VAL_MSG:
            send = false;
            mModeVals.alt = u32data;
            if (mKnobPos == 1) {
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
                send = true;
            }
            break;
        case MP_VS_VAL_POS_MSG:
           send = false;
            mModeVals.vs = u32data;
            mModeVals.vs_sign = MP_LED_PLUS_SIGN;
            if (mKnobPos == 2) {
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                send = true;
            }
            break;
        case MP_VS_VAL_NEG_MSG:
           send = false;
            mModeVals.vs = u32data;
            mModeVals.vs_sign = MP_LED_MINUS_SIGN;
            if (mKnobPos == 2) {
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, MP_LED_MINUS_SIGN, mPanelReport);
                send = true;
            }
            break;
        case MP_IAS_VAL_MSG:
           send = false;
            mModeVals.ias = u32data;
            if (mKnobPos == 3) {
                tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                send = true;
            }
            break;
        case MP_HDG_VAL_MSG:
           send = false;
            mModeVals.hdg = u32data;
            if (mKnobPos == 4) {
                tmp1 = dec2bcd(mModeVals.hdg, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                send = true;
            }
            break;
        case MP_CRS_VAL_MSG:
            send = false;
            mModeVals.crs = u32data;
            if (mKnobPos == 5) {
                tmp1 = dec2bcd(mModeVals.crs, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                send = true;
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
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
                break;
            case 2:
                tmp1 = dec2bcd(mModeVals.alt, 5) | 0xAAA00000;
                tmp2 = dec2bcd((uint32_t)abs((int)mModeVals.vs), 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
                break;
            case 3:
                tmp1 = dec2bcd(mModeVals.ias, 4) | 0xAAAA0000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                break;
            case 4:
                tmp1 = dec2bcd(mModeVals.hdg, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                break;
            case 5:
                tmp1 = dec2bcd(mModeVals.crs, 3) | 0xAAAAA000;
                mp_led_update(tmp1, tmp2, MP_LED_PLUS_SIGN, mPanelReport);
                break;
            }
            if (mBtns.ap == 1) {
                set_bit(&mPanelReport[11], MP_APBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_APBTN_BITPOS);
            }
            if (mBtns.hdg == 0) {
                set_bit(&mPanelReport[11], MP_HDGBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_HDGBTN_BITPOS);
            }
            if (mBtns.nav == 1) {
                set_bit(&mPanelReport[11], MP_NAVBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_NAVBTN_BITPOS);
            }
            if (mBtns.ias == 1) {
                set_bit(&mPanelReport[11], MP_IASBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_IASBTN_BITPOS);
            }
            if (mBtns.alt == 1) {
                set_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_ALTBTN_BITPOS);
            }
            if (mBtns.vs == 1) {
                set_bit(&mPanelReport[11], MP_VSBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_VSBTN_BITPOS);
            }
            if (mBtns.apr == 1) {
                set_bit(&mPanelReport[11], MP_APRBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_APRBTN_BITPOS);
            }
            if (mBtns.rev == 1) {
                set_bit(&mPanelReport[11], MP_REVBTN_BITPOS);
            } else {
                clear_bit(&mPanelReport[11], MP_REVBTN_BITPOS);
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
                    mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
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
//                    mp_led_update(tmp1, tmp2, mModeVals.vs_sign, mPanelReport);
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
            hid_send_feature_report((hid_device*)mHid, mPanelReport, sizeof(mPanelReport));
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

	bool data = true;
    if (!u32data) {
        data = false;
    }

// TODO: state information?
    switch(msg) {
    case SP_ALL_GREEN_SCRN_MSG:
        hid_send_feature_report((hid_device*)mHid, sp_green_panel, sizeof(sp_green_panel));
        return;
    case SP_BLANK_SCRN_MSG:
    	hid_send_feature_report((hid_device*)mHid, sp_blank_panel, sizeof(sp_blank_panel));
        return;
    case SP_ALL_RED_SCRN_MSG:
    	hid_send_feature_report((hid_device*)mHid, sp_red_panel, sizeof(sp_red_panel));
        return;
    case SP_ALL_ORANGE_SCRN_MSG:
    	hid_send_feature_report((hid_device*)mHid, sp_orange_panel, sizeof(sp_orange_panel));
        return;
    default:
        break;
    }

}


/**
 *
 */
void PanelsCheckThread::execute() {
    pexchange((int*)&gPcRun, true);
#ifdef DO_USBPANEL_CHECK
    void* p;
#endif

// TODO: flush the queues during a pend
    while (gPcRun) {
        gPcTrigger.wait();

        if (!gPcRun) {
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

