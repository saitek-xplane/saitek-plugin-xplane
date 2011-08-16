// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef MULTIPANEL_H
#define MULTIPANEL_H

#include "defs.h"

/*
----
Read
    4 bytes, byte four gives ID, which is always 0x00
    Knob position and autothrottle state are always returned in the read operation
----
    item                            byte        bit pos     value
    ----                            ----        -------     -----
Knob Mode:
    alt knob                        0           0           1       0x00 00 00 01
    vs knob                         0           1           1       0x00 00 00 02
    ias knob                        0           2           1       0x00 00 00 04
    hdg knob                        0           3           1       0x00 00 00 08
    crs knob                        0           4           1       0x00 00 00 10

Tuning Knob:
    tune knob clockwise             0           5           1       0x00 00 00 20
    tune knob counter-clockwise     0           6           1       0x00 00 00 40

Push Buttons:
    ap button                       0           7           1       0x00 00 00 80
    hdg button                      1           0           1       0x00 00 01 00
    nav button                      1           1           1       0x00 00 02 00
    ias button                      1           2           1       0x00 00 04 00
    alt button                      1           3           1       0x00 00 08 00
    vs button                       1           4           1       0x00 00 10 00
    apr button                      1           5           1       0x00 00 20 00
    rev button                      1           6           1       0x00 00 40 00

Autothrottle switch:
    autothrottle arm                1           7           1       0x00 00 80 00
    autothrottle off                1           7           0       0x00 00 00 00

Flaps status:
    flaps up                        2           0           1       0x00 01 00 00
    flaps disengaged                                                      0
    flaps down                      2           1           1       0x00 02 00 00

Trim status:
    trim down                       2           2           1       0x00 04 00 00
    trim disengaged                                                       0
    trim up                         2           3           1       0x00 08 00 00

-----------
Get Feature
-----------
                                   |                  |
                -------------------|------------------|--------|-------|-----|------
Byte position:  13  12  11  10  9  |  8  7  6  5  4   |    3   |   2   |  1  |  0 (always zero)
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

                   Alt LED value   |   VS LED value   | Button | ID
                -------------------|------------------|--------|----|
Byte position:  0  1  2  3  4  5   | 6  7  8  9  10   |   11   | 12 |
-----------------------------------|------------------|--------|----|
* each byte is BCD 0 thru 9,                          |   01   |    |AP
  0x0E is a negative sign,                            |   02   |    |HDG
  any other value blanks the LED                      |   04   |    |NAV
                                                      |   08   |    |IAS
                                                      |   10   |    |ALT
                                                      |   20   |    |VS
                                                      |   40   |    |APR
                                                      |   80   |    |REV

*/

#define MP_READ_LED_MODE_MASK     (0x0000001F)
#define MP_READ_TUNING_MASK       (0x00000060)
#define MP_READ_BTNS_MASK         (0x00007F10)
#define MP_READ_THROTTLE_MASK     (0x00008000)
#define MP_READ_FLAPS_MASK        (0x00030000)
#define MP_READ_TRIM_STATUS       (0x000C0000)

#define MP_READ_THROTTLE_OFF      (0x00000000)
#define MP_READ_THROTTLE_ON       (0x00008000)
#define MP_READ_FLAPS_UP          (0x00010000)
#define MP_READ_FLAPS_DOWN        (0x00020000)
#define MP_READ_TRIM_UP           (0x00080000)
#define MP_READ_TRIM_DOWN         (0x00040000)
#define MP_READ_TUNING_RIGHT      (0x00000020)
#define MP_READ_TUNING_LEFT       (0x00000040)
#define MP_READ_AP_BTN_ON         (0x00000080)
#define MP_READ_HDG_BTN_ON        (0x00000100)
#define MP_READ_NAV_BTN_ON        (0x00000200)
#define MP_READ_IAS_BTN_ON        (0x00000400)
#define MP_READ_ALT_BTN_ON        (0x00000800)
#define MP_READ_VS_BTN_ON         (0x00001000)
#define MP_READ_APR_BTN_ON        (0x00002000)
#define MP_READ_REV_BTN_ON        (0x00004000)

// commands
enum {
    CMD_AP_BTN_PRESS = 0,       // 0
    CMD_HDG_BTN_PRESS,
    CMD_NAV_BTN_PRESS,
    CMD_IAS_BTN_PRESS,
    CMD_ALT_BTN_PRESS,
    CMD_VS_BTN_PRESS,           // 5
    CMD_APR_BTN_PRESS,
    CMD_REV_BTN_PRESS,
    CMD_FLAPS_UP,
    CMD_FLAPS_DOWN,
    CMD_TRIM_UP,                // 10
    CMD_TRIM_DOWN,
    CMD_AUTOTHROTTLE_ARM,
    CMD_AUTOTHROTTLE_OFF,
    CMD_AUTOTHROTTLE_TOGGLE,
    CMD_ALT_INCREMENT,          // 15
    CMD_ALT_DECREMENT,
    CMD_VS_INCREMENT,
    CMD_VS_DECREMENT,
    CMD_IAS_INCREMENT,
    CMD_IAS_DECREMENT,          // 20
    CMD_HDG_INCREMENT,
    CMD_HDG_DECREMENT,
    CMD_CRS_INCREMENT,
    CMD_CRS_DECREMENT           // 24
};

// read

// get feature

// set feature


#ifdef __cplusplus
extern "C" {
#endif

    extern unsigned int gMpALT;
    extern unsigned int gMpVS;
    extern unsigned int gMpIAS;
    extern unsigned int gMpHDG;
    extern unsigned int gMpCRS;
    extern unsigned int gMpVSSign;

    extern int gMpKnobPosition;
    extern int gMpAutothrottleState;
    extern void mpProcOutData(uint32_t data);


#ifdef __cplusplus
}
#endif

#endif  /* MULTIPANEL_H */

