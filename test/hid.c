/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 2010-07-03

 Copyright 2010, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "hidapi.h"

struct hid_device_ {
    unsigned short vendor_id;
    unsigned short product_id;
};

hid_device* HID_API_EXPORT
hid_open(unsigned short vendor_id, unsigned short product_id, wchar_t* serial_number) {

    printf("-- hid_open - vendor id: 0x%X, product id: 0x%X\n", vendor_id, product_id);

    hid_device* dev = (hid_device*) calloc(1, sizeof(hid_device));
    dev->vendor_id = vendor_id;
    dev->product_id = product_id;

	return dev;
}

int HID_API_EXPORT
hid_read(hid_device* dev, unsigned char* data, size_t length) {
    size_t i;

    printf("-- hid_read - vendor id: 0x%X, product id: 0x%X\n", dev->vendor_id, dev->product_id);

    for (i = 0; i < length; i++) {
        data[i] = data[i] + 1;
    }

	return length;
}

int HID_API_EXPORT
hid_set_nonblocking(hid_device* dev, int nonblock) {	
    printf("-- hid_set_nonblocking - vendor id: 0x%X, product id: 0x%X\n", dev->vendor_id, dev->product_id);

	return 0;
}

int HID_API_EXPORT
hid_send_feature_report(hid_device* dev, const unsigned char* data, size_t length) {
    size_t i;

    printf("-- hid_send_feature_report - vendor id: 0x%X, product id: 0x%X\n", dev->vendor_id, dev->product_id);

    for (i = 0; i < length; i++) {
        printf("---- data[%d]: 0x%X\n", (int)i, data[i]);
    }


	return length;
}

void HID_API_EXPORT hid_close(hid_device* dev) {
    printf("-- hid_close - vendor id: 0x%X, product id: 0x%X\n", dev->vendor_id, dev->product_id);

    free(dev);
}

