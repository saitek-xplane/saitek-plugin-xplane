// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef MULTIPANEL_H
#define MULTIPANEL_H

/*

----
Read
    4 bytes, byte three gives ID, which is always 0x00
    The knob position and autothrottle state is always returned
    in the read operation.
----
    item                            byte        bit pos     value
    ----                            ----        -------     -----
    alt knob                        0           0           1
    vs knob                         0           1           1
    ias knob                        0           2           1
    hdg knob                        0           3           1
    crs knob                        0           4           1

    tune knob clockwise             0           5           1
    tune knob counter-clockwise     0           6           1

    ap button                       0           7           1
    hdg button                      1           0           1
    nav button                      1           1           1
    ias button                      1           2           1
    alt button                      1           3           1
    vs button                       1           4           1
    apr button                      1           5           1
    rev button                      1           6           1

    autothrottle arm                1           7           1
    autothrottle off                1           7           0

    flaps up                        2           0           1
    flaps disengaged
    flaps down                      2           1           1

    trim down                       2           2           1
    trim disengaged
    trim up                         2           3           1

-----------
Get Feature
-----------
                                   |                  |
                -------------------|------------------|--------|-------|-----|
Byte position:  13  12  11  10  9  |  8  7  6  5  4   |    3   |   2   |  1  |
-----------------------------------|------------------|--------|-------|-----|
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

                   Alt LED value   | VS LED value | Button | ID
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

//    flaps
#define        HIDREAD_FLAPSUP                  0x00010000
#define        HIDREAD_FLAPSDOWN                0x00020000
 //   pitch trim
#define        HIDREAD_PITCHTRIM_DOWN           0x00040000
#define        HIDREAD_PITCHTRIM_UP             0x00080000






#define        CLOCKWISE           0x05800050
#define        COUNTERCLOCKWISE     0x03080050

//    auto throttle
#define        ARMSWTH                 0x000100050
//#define        OFFSWTH                 0x

//     knob
#define        HDGKNB                 0x00880050
#define        IASKNB                 0x00480050
#define        VSKNB                  0x00280050
#define        ALTKNB                 0x00180050
#define        CRSKNB                 0x01080050
//    button
#define        AP                  0x08180050
#define        HDG                 0x00181050
#define        NAV                 0x00182050
#define        IAS                 0x00184050
#define        ALT                 0x00188050
#define        VS                  0x00190050
#define        APR                 0x001A0050
#define        REV                 0x00100050

// commands
enum {
    CMD_AP_BTN_PRESS = 0,
    CMD_HDG_BTN_PRESS,
    CMD_NAV_BTN_PRESS,
    CMD_IAS_BTN_PRESS,
    CMD_ALT_BTN_PRESS,
    CMD_VS_BTN_PRESS,
    CMD_APR_BTN_PRESS,
    CMD_REV_BTN_PRESS,
    CMD_FLAPS_UP,
    CMD_FLAPS_DOWN,
    CMD_TRIM_UP,
    CMD_TRIM_DOWN,
    CMD_AUTOTHROTTLE_ARM,
    CMD_AUTOTHROTTLE_OFF,
    CMD_AUTOTHROTTLE_TOGGLE,
    CMD_ALT_INCREMENT,
    CMD_ALT_DECREMENT,
    CMD_VS_INCREMENT,
    CMD_VS_DECREMENT,
    CMD_IAS_INCREMENT,
    CMD_IAS_DECREMENT,
    CMD_HDG_INCREMENT,
    CMD_HDG_DECREMENT,
    CMD_CRS_INCREMENT,
    CMD_CRS_DECREMENT
};

// read

// get feature

// set feature


#ifdef __cplusplus
extern "C" {
#endif

    extern unsigned char* mpProcOutData(unsigned int data);


#ifdef __cplusplus
}
#endif

#endif  // MULTIPANEL_H

/* end of file */
