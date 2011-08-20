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


//        sprintf(data, "%x%x%x%x", buf[0], buf[1], buf[2], buf[3]);
//        a = buf[0];
//        b = buf[1];
//        c = buf[2];
//        d = buf[3];
//        y =  a << 24 || b << 16 || c << 8 || d;

/*
 *
 */
void mp_init(hid_device* hid) {

    DPRINTF("Saitek ProPanels Plugin: mp_init\n");

    uint8_t buf[4];

    hid_set_nonblocking(hid, 1);
    hid_read(hid, buf, sizeof(buf));
    hid_send_feature_report(hid, mp_hid_blank_panel, sizeof(mp_hid_blank_panel));
    hid_set_nonblocking(hid, 0);
}
