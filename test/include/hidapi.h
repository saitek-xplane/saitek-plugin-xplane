/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.

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

#include <wchar.h>

#ifdef _WIN32
      #define HID_API_EXPORT __declspec(dllexport)
      #define HID_API_CALL
#else
      #define HID_API_EXPORT
      #define HID_API_CALL
#endif

#define HID_API_EXPORT_CALL HID_API_EXPORT HID_API_CALL

#ifdef __cplusplus
extern "C" {
#endif
		struct hid_device_;
		typedef struct hid_device_ hid_device;

		HID_API_EXPORT hid_device* HID_API_CALL hid_open(unsigned short vendor_id, unsigned short product_id, wchar_t* serial_number);
                int HID_API_EXPORT hid_read(hid_device* dev, unsigned char* data, size_t length);
		int  HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device* device, int nonblock);
		int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device* device, const unsigned char* data, size_t length);
		void HID_API_EXPORT HID_API_CALL hid_close(hid_device *device);

#ifdef __cplusplus
}
#endif

