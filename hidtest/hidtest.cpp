/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.

 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.



********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

enum {
    HID_ERROR               = -1,
    VENDOR_ID               = 0x06A3,
    RP_PROD_ID              = 0x0D05,
    MP_PROD_ID              = 0x0D06,
    SP_PROD_ID              = 0x0D07,
    RP_ERROR_THRESH         = 40,
    MP_ERROR_THRESH         = 40,
    SP_ERROR_THRESH         = 40,
    PANEL_CHECK_INTERVAL    = 5 // seconds
};

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include "hidapi.h"

hid_device *handle;

void signal_handler(int32_t signal)
{
    struct tm*  ptr = 0;
    time_t      tm  = time(0);

    ptr             = localtime(&tm);

    switch(signal)
    {
        case SIGFPE:
            perror("---------------- \n");
            perror(asctime(ptr));
            perror("A floating point exception occured.\n");
            break;
        case SIGILL:
            perror("---------------- \n");
            perror(asctime(ptr));
            perror("An illegal instruction occured.\n");
            break;
        case SIGINT:
            // the user hit CTRL-C
//            perror("\n");
printf("closing \n");
            break;
        case SIGSEGV:
            perror("---------------- \n");
            perror(asctime(ptr));
            perror("A segmentation violation occured.\n");
            break;
        default:
            perror("---------------- \n");
            perror(asctime(ptr));
            perror("An unknown signal was caught.\n");
            break;
    }

    if (handle)
        hid_close(handle);

    // pass a successful exit so our atexit handler is called
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	int res;
    unsigned int cnt = 0;
	unsigned char buf[256];
	unsigned char buf2[4];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];

	int i;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;

    // siganl handlers
    if(signal(SIGFPE, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGFPE signal handler.\n");

    if(signal(SIGILL, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGILL signal handler.\n");

    if(signal(SIGINT, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGINT signal handler.\n");

    if(signal(SIGSEGV, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGSEGV signal handler.\n");


//	devs = hid_enumerate(0x0, 0x0);
//	cur_dev = devs;
//	while (cur_dev) {
//		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
//		printf("\n");
//		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
//		printf("  Product:      %ls\n", cur_dev->product_string);
//		printf("\n");
//		cur_dev = cur_dev->next;
//	}
//	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
//	buf[0] = 0x01;
//	buf[1] = 0x81;

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
//	handle = hid_open(0x4d8, 0x3f, NULL);

// saitek pro control panels
	handle = hid_open(NULL, VENDOR_ID, MP_PROD_ID, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

	// Set the hid_read() function to be non-blocking.
//	hid_set_nonblocking(handle, 1);

	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
//	res = hid_read(handle, buf, 4);

	// Send a Feature Report to the device
//	memset(buf,0x09,sizeof(buf));
//	buf[12] = 0x01;
//	buf[11] = 0x01;
//	buf[10] = 0x10;
//	buf[0] = 0x2;
//	buf[1] = 0xa0;
//	buf[2] = 0x0a;
//	buf[3] = 0x00;
//	buf[4] = 0x00;
//	res = hid_send_feature_report(handle, buf, 13);
//	if (res < 0) {
//		printf("Unable to send a feature report.\n");
//	}

//	buf[12] = 0x00;
//	buf[11] = 0x80;
//	buf[10] = 0x10;
//	res = hid_write(handle, buf, 13);
//	if (res < 0) {
//		printf("Unable to write()\n");
//		printf("Error: %ls\n", hid_error(handle));
//	}

//    memset(buf,0x00,sizeof(buf));
//                buf[12] = 0x00;

    memset(buf,0x00,sizeof(buf));
//    buf[0] = 0;
//    buf[1] = 1;
//    buf[13] = 0;
//hid_send_feature_report(handle, buf, 13);
//return 1;
    while (1) {
        if (hid_check(VENDOR_ID, MP_PROD_ID)) {
            if (handle) {
                buf[11] = 0x00;
                hid_send_feature_report(handle, buf, 13);
                printf("sleep: %d\n", cnt++);
                sleep(1);
                buf[11] = 0x01;
                hid_send_feature_report(handle, buf, 13);
                sleep(1);

                continue;
            }
        } else {
            printf("no hid\n");
        }
        if (handle) {
            hid_close(handle);
 printf("hid closed\n");
            handle = NULL;
        }
        handle = hid_open(NULL, VENDOR_ID, MP_PROD_ID, NULL);
        if (!handle) {
            printf("unable to open device\n");
        } else {
            printf("device open\n");
        }

        sleep(1);
    }



	// Read the Manufacturer String
//	wstr[0] = 0x0000;
//	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
//	if (res < 0)
//		printf("Unable to read manufacturer string\n");
//    }

//	res = 0;
//	while (1) {
//		res = hid_read(handle, buf2, sizeof(buf2));
//		if (res == 0)
//			printf("waiting...\n");
//		else if (res < 0)
//			printf("Unable to read()\n");
//        else {
//            for (i = 0; i < res; i++)
//                printf("%02hhx ", buf2[i]);
//            printf("\n");
//            memset(buf2,0x00,sizeof(buf2));
//        }
//		#ifdef WIN32
//		Sleep(500);
//		#else
//		usleep(500*1000);
//		#endif
//	}
#if 0
	memset(buf,0,sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}

	memset(buf,0,sizeof(buf));

	// Toggle LED (cmd 0x80). The first byte is the report number (0x1).
	buf[0] = 0x00;
	buf[1] = 0x81;
	buf[2] = 0x10;
	res = hid_write(handle, buf, 13);
	if (res < 0) {
		printf("Unable to write()\n");
		printf("Error: %ls\n", hid_error(handle));
	}


	// Request state (cmd 0x81). The first byte is the report number (0x1).
	buf[0] = 0x1;
	buf[1] = 0x81;
	hid_write(handle, buf, 13);
	if (res < 0)
		printf("Unable to write() (2)\n");

	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
	res = 0;
	while (res == 0) {
		res = hid_read(handle, buf, sizeof(buf));
		if (res == 0)
			printf("waiting...\n");
		if (res < 0)
			printf("Unable to read()\n");
		#ifdef WIN32
		Sleep(500);
		#else
		usleep(500*1000);
		#endif
	}

	printf("Data read:\n   ");
	// Print out the returned buffer.
	for (i = 0; i < res; i++)
		printf("%02hhx ", buf[i]);
	printf("\n");
#endif

#ifdef WIN32
	system("pause");
#endif

	return 1;
}
