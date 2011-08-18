// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <stdio.h>
#include <stdint.h>

#include "ptypes.h"
#include "pasync.h"

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

#ifdef USE_NED
#include "nedmalloc.h"
#endif
//#include "overloaded.h"
#include "defs.h"
#include "utils.h"
#include "hidapi.h"
#include "PanelThreads.h"
#include "multipanel.h"


/*
----
Read
    - 4 bytes, byte four is the  ID and  is always 0x00
    - knob position and autothrottle state are always returned in the read data

    - Knob mode being turned generates 1 message
    - Auto Throttle switch toggle generates 1 message
    - Pushing a button generates 2 messages, 1) button state and 2) knob mode
    - Flap handle generates 2 messages, 1) flap state and 2) knob mode
    - Pitch Trim generates 2 messages, 1) pitch state and 2) knob mode
----
    item                            byte        bit pos     value
    ----                            ----        -------     -----
Knob Mode, 1 message  (X = Auto Throttle state):
    alt knob                        0           0           1       0x00 00 X0 01
    vs knob                         0           1           1       0x00 00 X0 02
    ias knob                        0           2           1       0x00 00 X0 04
    hdg knob                        0           3           1       0x00 00 X0 08
    crs knob                        0           4           1       0x00 00 X0 10

Tuning Knob, 2* messages (X = Auto Throttle state, YY = Mode knob status):
    tune knob clockwise             0           5           1       0x00 00 X0 2|YY
    tune knob counter-clockwise     0           6           1       0x00 00 X0 4|YY

Push Buttons, 2* messages (X = Auto Throttle state, YY = Mode knob status):
    ap button                       0           7           1       0x00 00 X0 8|YY
    hdg button                      1           0           1       0x00 00 X1 YY
    nav button                      1           1           1       0x00 00 X2 YY
    ias button                      1           2           1       0x00 00 X4 YY
    alt button                      1           3           1       0x00 00 X8 YY
    vs button                       1           4           1       0x00 00 X|10 YY
    apr button                      1           5           1       0x00 00 X|20 YY
    rev button                      1           6           1       0x00 00 X|40 YY

Autothrottle switch, 1 message (YY = Mode knob status):
    autothrottle arm                1           7           1       0x00 00 80 YY
    autothrottle off                1           7           0       0x00 00 00 YY

Flaps status, 2* messages (X = Auto Throttle state, YY = Mode knob status):
    flaps up                        2           0           1       0x00 01 X0 YY
    flaps disengaged
    flaps down                      2           1           1       0x00 02 X0 YY

Trim status, 2* messages (X = Auto Throttle state, YY = Mode knob status):
    trim down                       2           2           1       0x00 04 X0 YY
    trim disengaged
    trim up                         2           3           1       0x00 08 X0 YY

*knob mode status is the second message


LED Displays
------------
ALT & VS mode:
    ALT    XXXXX
     VS     XXXX

IAS mode:
    IAS      XXX

HDG mode:
    HDG      XXX

CRS mode:
    CRS      XXX

-----------
Get Feature
-----------
                -------------------|------------------|--------|-------|-----|------
Byte        13  12  11  10   9  8  |  7  6  5  4  3   |    2   |   1   |  0  |
-----------------------------------|------------------|--------|-------|-----|------
                                                      |        |       | 01  | alt knob
                                                      |        |       | 02  | vs knob
                                                      |        |       | 04  | ias knob
                                                      |        |       | 08  | hdg knob
                                                      |        |       | 10  | crs knob
                                                      |        |       | 20  | tune clockwise
                                                      |        |       | 40  | tune counter-clockwise
                                                      |        |       | 80  | ap button
                                                      |        |   01  |     | hdg button
                                                      |        |   02  |     | nav button
                                                      |        |   04  |     | ias button
                                                      |        |   08  |     | alt button
                                                      |        |   10  |     | vs button
                                                      |        |   20  |     | apr button
                                                      |        |   40  |     | rev button
                                                      |        |   80  |     | autothrottle on
                                                      |        |   00  |     | autothrottle off (bit position 7)
                                                      |   01   |       |     | flaps up
                                                      |   02   |       |     | flaps down
                                                      |   04   |       |     | trim up
                                                      |   08   |       |     | trim down

-----------
Set Feature
-----------
                   Alt LED value   |   VS LED value   |    Button | ID
                -------------------|------------------|-----------|----|
Byte position:  0  1  2  3  4  5   | 6  7  8  9  10   |     11    | 12 |
-----------------------------------|------------------|-----------|----|
* each byte is BCD 0 thru 9,  0x0E is a negative sign |   01 AP   |    |
 any other value blanks the LED                       |   02 HDG  |    |
                                                      |   04 NAV  |    |
                                                      |   08 IAS  |    |
                                                      |   10 ALT  |    |
                                                      |   20 VS   |    |
                                                      |   40 APR  |    |
                                                      |   80 REV  |    |

*/




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
XPLMDataRef gMpOttoOvrrde = NULL;
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
 * Not re-entrant
 */
void mp_proc_data(uint32_t data) {

/*

 - battery on and avionics on
     AP enabled
     Auto Throttle armed
     electirc flaps
 - handle pitch trim
 - manual flaps
 */

    static uint32_t led_mode;
    static uint32_t tuning_status;
    static uint32_t btns_status;
    static uint32_t throttle_status;
    static uint32_t flaps_status;
    static uint32_t trim_status;

//    x = new uint32_t;
//    *x = EXITING_THREAD_LOOP;
//    gRp_ojq.post(new myjob(x));

//    led_mode = data & READ_LED_MODE_MASK;
//    tuning_status = data & READ_TUNING_MASK;
//    btns_status = data & READ_BTNS_MASK;
//    throttle_status =  data & READ_THROTTLE_MASK;
//    flaps_status = data & READ_FLAPS_MASK;
//    trim_status = data & READ_TRIM_STATUS;

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
}
