// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include "ptypes.h"
#include "pasync.h"

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

#include "overloaded.h"
#include "nedmalloc.h"
#include "multipanel.h"


/* Command Refs */
XPLMCommandRef gMpAsDnCmdRef = NULL;
XPLMCommandRef gMpAsUpCmdRef = NULL;
XPLMCommandRef gMpAltDnCmdRef = NULL;
XPLMCommandRef gMpAltUpCmdRef = NULL;
XPLMCommandRef gMpAltHoldCmdRef = NULL;
XPLMCommandRef gMpAppCmdRef = NULL;
XPLMCommandRef gMpAtThrrtlTgglCmdRef = NULL;
XPLMCommandRef gMpBkCrsCmdRef = NULL;
XPLMCommandRef gMpFdirSrvUp1CmdRef = NULL;
XPLMCommandRef gMpFdirSrvDn1CmdRef = NULL;
XPLMCommandRef gMpFlpsDnCmdRef = NULL;
XPLMCommandRef gMpFlpsUpCmdRef = NULL;
XPLMCommandRef gMpHdgCmdRef = NULL;
XPLMCommandRef gMpHdgDnCmdRef = NULL;
XPLMCommandRef gMpHdgUpCmdRef = NULL;
XPLMCommandRef gMpLvlChngCmdRef = NULL;
XPLMCommandRef gMpNavCmdRef = NULL;
XPLMCommandRef gMpObsHsiDnCmdRef = NULL;
XPLMCommandRef gMpObsHsiUpCmdRef = NULL;
XPLMCommandRef gMpPtchTrmDnCmdRef = NULL;
XPLMCommandRef gMpPtchTrmUpCmdRef = NULL;
XPLMCommandRef gMpPtchTrmTkOffCmdRef = NULL;
XPLMCommandRef gMpSrvsFlghtDirOffCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdDnCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdUpCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdCmdRef = NULL;

/* Data Refs */
XPLMDataRef gMpArspdDataRef = NULL;
XPLMDataRef gMpAltDataRef = NULL;
XPLMDataRef gMpAltHoldStatDataRef = NULL;
XPLMDataRef gMpApprchStatDataRef = NULL;
XPLMDataRef gMpApStateDataRef = NULL;
XPLMDataRef gMpAvncsOnDataRef = NULL;
XPLMDataRef gMpBckCrsStatDataRef = NULL;
XPLMDataRef gMpBttryOnDataRef = NULL;
XPLMDataRef gMpFlghtDirModeDataRef = NULL;
XPLMDataRef gMpHdgMagDataRef = NULL;
XPLMDataRef gMpHdgStatDataRef = NULL;
XPLMDataRef gMpHsiObsDegMagPltDataRef = NULL;
XPLMDataRef gMpNavStatDataRef = NULL;
XPLMDataRef gMpSpdStatDataRef = NULL;
XPLMDataRef gMpVrtVelDataRef = NULL;
XPLMDataRef gMpVviStatDataRef = NULL;

//        sprintf(data, "%x%x%x%x", buf[0], buf[1], buf[2], buf[3]);
//        a = buf[0];
//        b = buf[1];
//        c = buf[2];
//        d = buf[3];
//        y =  a << 24 || b << 16 || c << 8 || d;

/*
#define MP_READ_THROTTLE_OFF
#define MP_READ_THROTTLE_ON
#define MP_READ_FLAPS_UP
#define MP_READ_FLAPS_DOWN
#define MP_READ_TRIM_UP
#define MP_READ_TRIM_DOWN
#define MP_READ_TUNING_RIGHT
#define MP_READ_TUNING_LEFT
#define MP_READ_AP_BTN_ON
#define MP_READ_HDG_BTN_ON
#define MP_READ_NAV_BTN_ON
#define MP_READ_IAS_BTN_ON
#define MP_READ_ALT_BTN_ON
#define MP_READ_VS_BTN_ON
#define MP_READ_APR_BTN_ON
#define MP_READ_REV_BTN_ON
*/
/*
 * Not re-entrant
 */
void mp_proc_data(uint32_t data) {

    static uint32_t led_mode;
    static uint32_t tuning_status;
    static uint32_t btns_status;
    static uint32_t throttle_status;
    static uint32_t flaps_status;
    static uint32_t trim_status;

    led_mode = data & MP_READ_LED_MODE_MASK;
    tuning_status = data &  MP_READ_TUNING_MASK;
    btns_status = data & MP_READ_BTNS_MASK;
    throttle_status =  data & MP_READ_THROTTLE_MASK;
    flaps_status = data &  MP_READ_FLAPS_MASK;
    trim_status = data &  MP_READ_TRIM_STATUS;

//    if (trimup_cnt) {
//        if (data & MP_READ_PITCHTRIM_UP) {
//            trimdown_cnt += 1;
//        } else {
//            msg = (unsigned char*) malloc(sizeof(unsigned int));
//            *((unsigned char*)msg) = MP_READ_PITCHTRIM_UP;
//            trimup_cnt = false;
//        }
//    } else if (trimdown_cnt) {
//        if (data & MP_READ_PITCHTRIM_DOWN) {
//            trimdown_cnt += 1;
//        } else {
//            msg = (unsigned char*) malloc(sizeof(unsigned int));
//            *((unsigned char*)msg) = MP_READ_PITCHTRIM_DOWN;
//            trimdown_cnt = false;
//        }
//    } else {

/*
        if ((data & MP_READ_AUTOTHROTTLE_ON) && !gMpAutothrottleState) {
            gMpAutothrottleState = true;
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = MP_READ_AUTOTHROTTLE_ON;
        } else if (!(data & MP_READ_AUTOTHROTTLE_ON) && gMpAutothrottleState) {
            gMpAutothrottleState = false;
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = MP_READ_AUTOTHROTTLE_OFF;
        } else if (data & MP_READ_FLAPSUP) {
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = MP_READ_FLAPSUP;
        } else if (data & MP_READ_FLAPSDOWN) {
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = MP_READ_FLAPSDOWN;
        } else if (data & MP_READ_PITCHTRIM_DOWN) {
            if (trimdown) {
                msg = (unsigned int*) malloc(sizeof(unsigned int));
                *msg = MP_READ_PITCHTRIM_DOWN;
                trimdown = false;
            } else {
                trimdown = true;
            }
        } else if (data & MP_READ_PITCHTRIM_UP) {
            if (trimup) {
                msg = (unsigned int*) malloc(sizeof(unsigned int));
                *msg = MP_READ_PITCHTRIM_UP;
                trimup = false;
            } else {
                trimup = true;
            }
        } else {  // temporary
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = 0xFFFFFFFF;
        }


            switch (cmd) {
                case MP_READ_AUTOTHROTTLE_OFF:
                    XPLMSetDataf(gApAutoThrottleRef, false);
                    break;
                case MP_READ_AUTOTHROTTLE_ON:
                    XPLMSetDataf(gApAutoThrottleRef, true);
                    break;
                case MP_READ_PITCHTRIM_UP:
    // TODO; add range and sanity checks
    //                if (XPLMGetDataf(gApMaxElevTrimRef) != 0.0) {
    //XPLMSpeakString("pitch up");
                        x = XPLMGetDataf(gApElevTrimRef) + 0.01;
    //                    if (x <= 1.0) {
                            XPLMSetDataf(gApElevTrimRef, x);
    //                        XPLMSetDatai(gApElevTrimUpAnnuncRef, true);
    //                    }
    //                }
    //                XPLMCommandKeyStroke(xplm_key_elvtrimU);
                    break;
                case MP_READ_PITCHTRIM_DOWN:
    // TODO; add range and sanity checks
    //                if (XPLMGetDataf(gApMaxElevTrimRef) != 0.0) {
    //XPLMSpeakString("pitch down");
                        x = XPLMGetDataf(gApElevTrimRef) - 0.01;
    //                    if (x >= 0.0) {
                            XPLMSetDataf(gApElevTrimRef, x);
    //                        XPLMSetDatai(gApElevTrimDownAnnuncRef, true);
    //                    }
    //                }
    //                XPLMCommandKeyStroke(xplm_key_elvtrimD);
                    break;
                case MP_READ_FLAPSDOWN:
    //XPLMSpeakString("flaps down");
                    XPLMCommandKeyStroke(xplm_key_flapsdn);
                    break;
                case MP_READ_FLAPSUP:
    //XPLMSpeakString("flaps up");
                    XPLMCommandKeyStroke(xplm_key_flapsup);
                    break;
                default:
                    break;
            }


*/
//    }


//        switch (y) {
//            case CLOCKWISE:
//                XPLMSpeakString("clockwise \n");
//                break;
//            case COUNTERCLOCKWISE:
//                XPLMSpeakString("counter clockwise \n");
//                break;
//            case HIDREAD_PITCHUP:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
    //        case PITCHNEUTRAL:
    //            XPLMSpeakString("pitch neutral \n");
    //            break;
//            case HIDREAD_PITCHDOWN:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
    //        case ARMSWTH:
    //            XPLMSpeakString("autothrottle arm \n");
    //            break;
    //        case OFFSWTH:
    //            XPLMSpeakString("autothrottle off \n");
    //            break;
//            case MP_READ_FLAPSUP:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
    //        case FLAPSNEUTRAL:
    //            XPLMSpeakString("flaps neutral \n");
    //            break;
//            case MP_READ_FLAPSDOWN:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
//            case HDGKNB:
//                XPLMSpeakString("heading switch \n");
//                break;
//            case IASKNB:
//                XPLMSpeakString("inicated airspeed switch \n");
//                break;
//            case VSKNB:
//                XPLMSpeakString("vertical speed  switch \n");
//                break;
//            case ALTKNB:
//                XPLMSpeakString("altitude switch \n");
//                break;
//            case CRSKNB:
//                XPLMSpeakString("course switch \n");
//                break;
    //push buttons
//            case AP:
//                XPLMSpeakString("approach \n");
//                break;
//            case HDG:
//                XPLMSpeakString("heading \n");
//                break;
//            case NAV:
//                XPLMSpeakString("navigation \n");
//                break;
//            case IAS:
//                XPLMSpeakString("indicated ait speed \n");
//                break;
//            case ALT:
//                XPLMSpeakString("altitude \n");
//                break;
//            case VS:
//                XPLMSpeakString("vertical speed \n");
//                break;
//            case APR:
//                XPLMSpeakString("approach \n");
//                break;
//            case REV:
//                XPLMSpeakString("reverse \n");
//                break;
//            default:
//                XPLMSpeakString(data);
//                break;
//        }
}
