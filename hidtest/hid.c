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


#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <pthread.h>

#include "hidapi.h"

#define BUF_LEN (256)
#define HID_READ_TIMEOUT       (0.050)

int gRemoved = 0;

extern bool HID_API_EXPORT HID_API_CALL hid_check(unsigned short vendor_id, unsigned short product_id);

/* Linked List of input reports received from the device. */
struct input_report {
	uint8_t *data;
	size_t len;
	struct input_report *next;
};

struct hid_device_ {
	IOHIDDeviceRef device_handle;
	int blocking;
    int disconnected;
    func_cb fcb;
    unsigned short product_id;
	int uses_numbered_reports;
	CFStringRef run_loop_mode;
	uint8_t *input_report_buf;
	struct input_report *input_reports;
	pthread_mutex_t mutex;
    CFRunLoopRef rl;
};

void hid_inserted_cb(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
printf("hid inserted\n");
}

// XXX: added hid_check and hid_removed_cb
void hid_removed_cb(void* dev, IOReturn ret, void* ref) {
printf("hid removed\n");
    CFRunLoopStop(((hid_device*)dev)->rl);

    ((hid_device*)dev)->disconnected = true;

    if (((hid_device*)dev)->fcb)
        ((hid_device*)dev)->fcb((hid_device*)dev);
}

static hid_device *new_hid_device(void)
{
	hid_device *dev = (hid_device*)calloc(1, sizeof(hid_device));
	dev->device_handle = NULL;
	dev->blocking = 1;
	dev->uses_numbered_reports = 0;
	dev->input_reports = NULL;
	
	return dev;
}

static 	IOHIDManagerRef hid_mgr = 0x0;

#if 0
static void register_error(hid_device *device, const char *op)
{

}
#endif


static long get_long_property(IOHIDDeviceRef device, CFStringRef key)
{
	CFTypeRef ref;
	long value;
	
	ref = IOHIDDeviceGetProperty(device, key);
	if (ref) {
		if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
			CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
			return value;
		}
	}
	return 0;
}

static unsigned short get_vendor_id(IOHIDDeviceRef device)
{
	return get_long_property(device, CFSTR(kIOHIDVendorIDKey));
}

static unsigned short get_product_id(IOHIDDeviceRef device)
{
	return get_long_property(device, CFSTR(kIOHIDProductIDKey));
}

// XXX: added hid_check and hid_removed_cb
// not very elegant, need to add a device inserted callback
bool HID_API_EXPORT hid_check(unsigned short vendor_id, unsigned short product_id)
{
	IOHIDManagerRef _mgr;
    unsigned short dev_vid;
    unsigned short dev_pid;
    bool status = false;
	int i;
	
	_mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	IOHIDManagerSetDeviceMatching(_mgr, NULL);
	IOHIDManagerOpen(_mgr, kIOHIDOptionsTypeNone);
	
	CFSetRef device_set = IOHIDManagerCopyDevices(_mgr);
	
	CFIndex num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef* device_array = (IOHIDDeviceRef*)calloc((int)num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);
	
	setlocale(LC_ALL, "");
	
	for (i = 0; i < num_devices; i++) {
		IOHIDDeviceRef dev = device_array[i];
		dev_vid = get_vendor_id(dev);
		dev_pid = get_product_id(dev);

		/* Check the VID/PID against the arguments */
		if (vendor_id == dev_vid && product_id == dev_pid) {
            status = true;
            break;
		}
	}

	free(device_array);
	CFRelease(device_set);
	
	return status;
}

static long get_max_report_length(IOHIDDeviceRef device)
{
	return get_long_property(device, CFSTR(kIOHIDMaxInputReportSizeKey));
}

static int get_string_property(IOHIDDeviceRef device, CFStringRef prop, wchar_t *buf, size_t len)
{
	CFStringRef str = (CFStringRef)IOHIDDeviceGetProperty(device, prop);

	buf[0] = 0x0000;

	if (str) {
		CFRange range;
		range.location = 0;
		range.length = len;
		CFIndex used_buf_len;
		CFStringGetBytes(str,
			range,
			kCFStringEncodingUTF32LE,
			(char)'?',
			FALSE,
			(UInt8*)buf,
			len,
			&used_buf_len);
		return used_buf_len;
	}
	else
		return 0;
		
}

static int get_string_property_utf8(IOHIDDeviceRef device, CFStringRef prop, char *buf, size_t len)
{
	CFStringRef str = (CFStringRef)IOHIDDeviceGetProperty(device, prop);

	buf[0] = 0x0000;

	if (str) {
		CFRange range;
		range.location = 0;
		range.length = len;
		CFIndex used_buf_len;
		CFStringGetBytes(str,
			range,
			kCFStringEncodingUTF8,
			(char)'?',
			FALSE,
			(UInt8*)buf,
			len,
			&used_buf_len);
		buf[len-1] = 0x00000000;
		return used_buf_len;
	}
	else
		return 0;
		
}


static int get_serial_number(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDSerialNumberKey), buf, len);
}

static int get_manufacturer_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDManufacturerKey), buf, len);
}

static int get_product_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDProductKey), buf, len);
}


/* Implementation of wcsdup() for Mac. */
static wchar_t *dup_wcs(const wchar_t *s)
{
	size_t len = wcslen(s);
	wchar_t *ret = (wchar_t*)malloc((len+1)*sizeof(wchar_t));
	wcscpy(ret, s);

	return ret;
}


static int make_path(IOHIDDeviceRef device, char *buf, size_t len)
{
	int res;
	unsigned short vid, pid;
	char transport[32];

	buf[0] = '\0';

	res = get_string_property_utf8(
		device, CFSTR(kIOHIDTransportKey),
		transport, sizeof(transport));
	
	if (!res)
		return -1;

	vid = get_vendor_id(device);
	pid = get_product_id(device);

	res = snprintf(buf, len, "%s_%04hx_%04hx_%p",
	                   transport, vid, pid, device);
	
	
	buf[len-1] = '\0';
	return res+1;
}

static void init_hid_manager(void)
{
	/* Initialize all the HID Manager Objects */
	hid_mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	IOHIDManagerSetDeviceMatching(hid_mgr, NULL);
//    IOHIDManagerRegisterDeviceMatchingCallback(hid_mgr, hid_inserted_cb, NULL);
	IOHIDManagerScheduleWithRunLoop(hid_mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDManagerOpen(hid_mgr, kIOHIDOptionsTypeNone);
}


struct hid_device_info  HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	struct hid_device_info *root = NULL; // return object
	struct hid_device_info *cur_dev = NULL;
	CFIndex num_devices;
	int i;
	
	setlocale(LC_ALL,"");

	/* Set up the HID Manager if it hasn't been done */
	if (!hid_mgr)
		init_hid_manager();

	/* Get a list of the Devices */
	CFSetRef device_set = IOHIDManagerCopyDevices(hid_mgr);

	/* Convert the list into a C array so we can iterate easily. */	
	num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = (IOHIDDeviceRef*)calloc((int)num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);

	/* Iterate over each device, making an entry for it. */	
	for (i = 0; i < num_devices; i++) {
		unsigned short dev_vid;
		unsigned short dev_pid;

		wchar_t buf[BUF_LEN];
		char cbuf[BUF_LEN];

		IOHIDDeviceRef dev = device_array[i];

		dev_vid = get_vendor_id(dev);
		dev_pid = get_product_id(dev);

		/* Check the VID/PID against the arguments */
		if ((vendor_id == 0x0 && product_id == 0x0) || (vendor_id == dev_vid && product_id == dev_pid)) {
			struct hid_device_info *tmp;
			size_t len;

		    /* VID/PID match. Create the record. */
			tmp = (struct hid_device_info*)malloc(sizeof(struct hid_device_info));
			if (cur_dev) {
				cur_dev->next = tmp;
			}
			else {
				root = tmp;
			}
			cur_dev = tmp;
			
			/* Fill out the record */
			cur_dev->next = NULL;
			len = make_path(dev, cbuf, sizeof(cbuf));
			cur_dev->path = strdup(cbuf);

			/* Serial Number */
			get_serial_number(dev, buf, BUF_LEN);
			cur_dev->serial_number = dup_wcs(buf);

			/* Manufacturer and Product strings */
			get_manufacturer_string(dev, buf, BUF_LEN);
			cur_dev->manufacturer_string = dup_wcs(buf);
			get_product_string(dev, buf, BUF_LEN);
			cur_dev->product_string = dup_wcs(buf);
			
			/* VID/PID */
			cur_dev->vendor_id = dev_vid;
			cur_dev->product_id = dev_pid;
		}
	}
	
	free(device_array);
	CFRelease(device_set);
	
	return root;
}

void HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
	/* This function is identical to the Linux version. Platform independent. */
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

hid_device* HID_API_EXPORT hid_open(func_cb fcb,
                                    unsigned short vendor_id,
                                    unsigned short product_id,
                                    wchar_t *serial_number)
{
	/* This function is identical to the Linux version. Platform independent. */
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device * handle = NULL;
	
	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id && cur_dev->product_id == product_id) {
			if (serial_number) {
				if (wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
printf("path_to_open\n");
		handle = hid_open_path(path_to_open, fcb, product_id);
	}

	hid_free_enumeration(devs);
	
	return handle;
}

/* The Run Loop calls this function for each input report received.
   This function puts the data into a linked list to be picked up by
   hid_read(). */
static void hid_report_callback(void *context, IOReturn result, void *sender,
                         IOHIDReportType report_type, uint32_t report_id,
                         uint8_t *report, CFIndex report_length)
{
	struct input_report *rpt;
	hid_device *dev = (hid_device*)context;
	
	/* Make a new Input Report object */
	rpt = (struct input_report*)calloc(1, sizeof(struct input_report));
	rpt->data = (uint8_t*)calloc(1, report_length);
	memcpy(rpt->data, report, report_length);
	rpt->len = report_length;
	rpt->next = NULL;
	
	/* Attach the new report object to the end of the list. */
	if (dev->input_reports == NULL) {
		/* The list is empty. Put it at the root. */
		dev->input_reports = rpt;
	}
	else {
		/* Find the end of the list and attach. */
		struct input_report *cur = dev->input_reports;
		while (cur->next != NULL)
			cur = cur->next;
		cur->next = rpt;
	}
	
	/* Stop the Run Loop. This is mostly used for when blocking is
	   enabled, but it doesn't hurt for non-blocking as well.  */
	CFRunLoopStop(CFRunLoopGetCurrent());
}

hid_device* HID_API_EXPORT hid_open_path(const char *path, func_cb fcb, unsigned short product_id)
{
  	int i;
	hid_device *dev = NULL;
	CFIndex num_devices;
	
	dev = new_hid_device();

	/* Set up the HID Manager if it hasn't been done */
	if (!hid_mgr)
		init_hid_manager();

	CFSetRef device_set = IOHIDManagerCopyDevices(hid_mgr);

	num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = (IOHIDDeviceRef*)calloc((int)num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);	
	for (i = 0; i < num_devices; i++) {
		char cbuf[BUF_LEN];
		size_t len;
		IOHIDDeviceRef os_dev = device_array[i];

		len = make_path(os_dev, cbuf, sizeof(cbuf));
		if (!strcmp(cbuf, path)) {
			// Matched Paths. Open this Device.
			IOReturn ret = IOHIDDeviceOpen(os_dev, kIOHIDOptionsTypeNone);
			if (ret == kIOReturnSuccess) {
				char str[32];
				CFIndex max_input_report_len;

				free(device_array);
				CFRelease(device_set);
				dev->device_handle = os_dev;
				
				/* Create the buffers for receiving data */
				max_input_report_len = (CFIndex) get_max_report_length(os_dev);
				dev->input_report_buf = (uint8_t*)calloc((int)max_input_report_len, sizeof(uint8_t));

				/* Create the Run Loop Mode for this device. printing the reference seems to work. */
                /* From Mac library ref: mode can be any arbitrary CFString */
				sprintf(str, "%p", os_dev);
				dev->run_loop_mode = CFStringCreateWithCString(NULL, str, kCFStringEncodingASCII);

                // XXX: device removal callback
                dev->disconnected = false;
                dev->product_id = product_id;

                // setting the run-loop ref here requires hid_open
                // to be called by the thread doing the reading
                dev->rl = CFRunLoopGetMain();

                if (fcb) {
                    dev->fcb = fcb;
                    IOHIDDeviceRegisterRemovalCallback(os_dev, hid_removed_cb, (void*) dev);
                }
                //--------

				IOHIDDeviceScheduleWithRunLoop(os_dev, CFRunLoopGetCurrent(), dev->run_loop_mode);

				/* Attach the device to a Run Loop */
				IOHIDDeviceRegisterInputReportCallback(os_dev, dev->input_report_buf,
                                                        max_input_report_len,
                                                        &hid_report_callback, dev);

				 pthread_mutex_init(&dev->mutex, NULL);
				
				return dev;
			}
			else {
printf("failed\n");
				goto return_error;
			}
		}
	}

return_error:

	free(device_array);
	CFRelease(device_set);
	free(dev);
	return NULL;
}

static int set_report(hid_device *dev, IOHIDReportType type, const unsigned char *data, size_t length)
{
	const unsigned char *data_to_send;
	size_t length_to_send;
	IOReturn res;

// JDP: added 11-11-10
    if (!dev)
        return -1;

	if (data[0] == 0x0) {
		/* Not using numbered Reports.
		   Don't send the report number. */
		data_to_send = data+1;
		length_to_send = length-1;
	}
	else {
		/* Using numbered Reports.
		   Send the Report Number */
		data_to_send = data;
		length_to_send = length;
	}
	
	res = IOHIDDeviceSetReport(dev->device_handle,
	                           type,
	                           data[0], /* Report ID*/
	                           data_to_send, length_to_send);
	
	if (res == kIOReturnSuccess) {
		return length;
	}
	else
		return -1;

}

int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	return set_report(dev, kIOHIDReportTypeOutput, data, length);
}

/* Helper function, so that this isn't duplicated in hid_read(). */
static int return_data(hid_device *dev, unsigned char *data, size_t length)
{
	/* Copy the data out of the linked list item (rpt) into the
	   return buffer (data), and delete the liked list item. */
	struct input_report *rpt = dev->input_reports;
    if (!rpt) {
        return 0;
    } else if (!rpt->data) {
        free(rpt);
        return 0;
    }
	size_t len = (length < rpt->len) ? length : rpt->len;
	memcpy(data, rpt->data, len);
	dev->input_reports = rpt->next;
	free(rpt->data);
	free(rpt);
	return len;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
    SInt32 code;
	int ret_val = -1;

// JDP: added 11-11-10
    if (!dev)
        return ret_val;

    if (dev->disconnected)
        return HID_DISCONNECTED;

	/* Lock this function */
	pthread_mutex_lock(&dev->mutex);
	
	/* There's an input report queued up. Return it. */
	if (dev->input_reports) {
		/* Return the first one */
		ret_val = return_data(dev, data, length);
		goto ret;
	}
	
	/* There are no input reports queued up.
	   Need to get some from the OS. */

	/* Move the device's run loop to this thread. */
	IOHIDDeviceScheduleWithRunLoop(dev->device_handle, CFRunLoopGetCurrent(), dev->run_loop_mode);
	
	if (dev->blocking) {
		/* Run the Run Loop until it stops timing out. In other
		   words, until something happens. This is necessary because
		   there is no INFINITE timeout value. */
		while (1) {
            if (dev->disconnected) {
                ret_val = HID_DISCONNECTED;
                goto ret;
            }

// XXX: 2010-11-15 experimenting with the timeout value
            // if returnAfterSourceHandled is
            //      true : run loop should exit after processing one source
            //      false: the run loop continues processing events until seconds has passed
            code = CFRunLoopRunInMode(dev->run_loop_mode, HID_READ_TIMEOUT, true);

			/* Return if some data showed up. */
			if (dev->input_reports) {
                ret_val = return_data(dev, data, length);
                break;
            }

            if (code == kCFRunLoopRunTimedOut || code == kCFRunLoopRunHandledSource) {
                continue;
            }

            if (kCFRunLoopRunStopped)
                printf("kCFRunLoopRunStopped\n");
            /* Break if kCFRunLoopRunFinished or kCFRunLoopRunStopped */

            ret_val = -1;
            break;
		}
	} else {
		/* Non-blocking. See if the OS has any reports to give. */
        if (dev->disconnected) {
            ret_val = HID_DISCONNECTED;
        } else {
            code = CFRunLoopRunInMode(dev->run_loop_mode, 0, true);

            if (code == kCFRunLoopRunTimedOut|| code == kCFRunLoopRunHandledSource) {
                ret_val = -1; /* An error occured. */
            } else {
                if (dev->input_reports) {
                    /* Return the first one */
                    ret_val = return_data(dev, data, length);
                } else {
                    ret_val = 0; /* No data*/
                }
            }
		}
	}

ret:
	/* Unlock */
	pthread_mutex_unlock(&dev->mutex);
	return ret_val;
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
// JDP: added 11-11-10
    if (!dev)
        return -1;

	/* All Nonblocking operation is handled by the library. */
	dev->blocking = !nonblock;
	
	return 0;
}

int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
    if (!dev)
        return -1;

    if (dev->disconnected)
        return HID_DISCONNECTED;

	return set_report(dev, kIOHIDReportTypeFeature, data, length);
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	CFIndex len = length;
	IOReturn res;
//    int e;

// JDP: added 11-11-10
    if (!dev)
        return -1;

    if (dev->disconnected)
        return HID_DISCONNECTED;

    res = IOHIDDeviceGetReport(dev->device_handle,
                               kIOHIDReportTypeFeature,
                               data[0], /* Report ID */
                               data, &len);

	if (res == kIOReturnSuccess)
		return len;
	else
		return -1;
}

void HID_API_EXPORT hid_close(hid_device *dev)
{
	if (!dev)
		return;

// XXX:
    dev->disconnected = true;
    IOHIDDeviceRegisterRemovalCallback(dev->device_handle, NULL, NULL);
//-----

    /* Close the OS handle to the device. */
    IOReturn x = IOHIDDeviceClose(dev->device_handle, kIOHIDOptionsTypeNone);

    switch (x) {
        case kIOReturnSuccess:
                printf("success\n");
            break;
        case kIOReturnBadArgument:
                printf("bad arg\n");
            break;
        default:
            break;
    }

	/* Delete any input reports still left over. */
	struct input_report *rpt = dev->input_reports;
	while (rpt) {
		struct input_report *next = rpt->next;
		free(rpt->data);
		free(rpt);
		rpt = next;
	}

	/* Free the string and the report buffer. */
	CFRelease(dev->run_loop_mode);
	free(dev->input_report_buf);
	pthread_mutex_destroy(&dev->mutex);

	free(dev);
}

int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_manufacturer_string(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
// JDP: added 11-11-10
    if (!dev)
        return -1;

    if (dev->disconnected)
        return HID_DISCONNECTED;

	return get_product_string(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_serial_number(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	// TODO:

	return 0;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	// TODO:

	return NULL;
}


#if 1

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


hid_device *volatile gHandle;

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
//printf("closing \n");
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

    if (gHandle && !gHandle->disconnected) {
        hid_close((hid_device*)gHandle);
    }

    // pass a successful exit so our atexit handler is called
    exit(EXIT_SUCCESS);
}

void close_hid(hid_device* dev) {
gRemoved = 1;
//    hid_close((hid_device*)gHandle);
//    gHandle = 0;
}

void toggle_bit(unsigned char* c, long pos) {
    *c ^= (0x01 << pos);
}

unsigned int bcd2dec(unsigned int num, int n) {
    int i;
    unsigned int val = 0;

    for (i = 0; i < n; i++) {
        val += (((num >> (i * 4)) & 0x0F) * (i ? (i * 10):1));
    }

    return val;
}

unsigned int dec2bcd(unsigned int num, int n) {
    int i;
    unsigned int q, r;
    unsigned int val = 0;

    for (i = 0; i < n; i++) {
        q = num / 10;
        r = num % 10;
        val |= (r << (i * 4));
        num = q;
    }

    return val;
}

unsigned char bufIn[4];
unsigned char bufOut[13];
unsigned char bufGet[256];

#define MAX_STR 255
wchar_t wstr[MAX_STR];

int main(int argc, char* argv[])
{
	int res;
    unsigned int cnt = 0;



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

start:

	gHandle = (hid_device *volatile) hid_open(&close_hid, VENDOR_ID, MP_PROD_ID, NULL);
	if (!gHandle) {
		printf("unable to open device\n");
 		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string((hid_device*)gHandle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string((hid_device*)gHandle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string((hid_device*)gHandle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string((hid_device*)gHandle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);


    cnt = 0;
    int status;

    unsigned int tmp = 0;

    while (1) {
        cnt++;

//        if (hid_check(VENDOR_ID, MP_PROD_ID)) {
printf("--- hid exists---\n");
            if (!gHandle) {
                printf("--- waiting ---\n");
//                goto start;
                sleep(2);
            }

            if (gRemoved) {
                res = hid_get_manufacturer_string((hid_device*)gHandle, wstr, MAX_STR);
                if (res < 0)
                    printf("Unable to read manufacturer string\n");
            }

            memset(bufOut,0x00,sizeof(bufIn));
printf(" reading \n");
            if ((status = hid_read((hid_device*)gHandle, bufIn, 4)) <= 0) {
                usleep(500);
                continue;
            }
tmp = *((unsigned int*)(&bufIn[0]));
printf(" bifIn: 0x%.8X \n", tmp);

            res = hid_get_feature_report((hid_device*)gHandle, bufGet, sizeof(bufGet));
            if (res) {
printf("data received: %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X, %.2X\n",
bufIn[12], bufIn[11], bufIn[10], bufIn[9], bufIn[8] ,bufIn[7], bufIn[6], bufIn[5], bufIn[4], bufIn[3], bufIn[2], bufIn[1], bufIn[0]);
            }
//printf("data received: %.2X, %.2X, %.2X, %.2X\n", bufIn[3], bufIn[2], bufIn[1], bufIn[0]);
            memset(bufOut,0x00,sizeof(bufOut));

            tmp = dec2bcd(cnt, 5);
printf(" tmp: 0x%X\n", tmp);
            bufOut[1]  = (unsigned char) (tmp >> 16);
            bufOut[2]  = (unsigned char) (tmp >> 12);
            bufOut[3]  = (unsigned char) (tmp >> 8);
            bufOut[4]  = (unsigned char) (tmp >> 4);
            bufOut[5]  = (unsigned char) tmp;


            bufOut[7]  = (unsigned char) (tmp >> 12);
            bufOut[8]  = (unsigned char) (tmp >> 8);
            bufOut[9]  = (unsigned char) (tmp >> 4);
            bufOut[10] = (unsigned char) tmp;

//            toggle_bit(&bufIn[0], 1);
//            bufOut[11] = bufIn[0];

            memset(bufOut,0x00,sizeof(bufOut));
            status = hid_send_feature_report((hid_device*)gHandle, bufOut, 13);


//            usleep(1);
//            bufOut[11] = 0x01;
//            status = hid_send_feature_report((hid_device*)gHandle, bufOut, 13);
//            usleep(500);
            printf(" data sent: %d\n", cnt++);
            continue;
//        } else {
//            printf(" **** no hid: %d\n", cnt);
//            sleep(2);
//        }

        usleep(500);
    }

    if (gHandle)
         hid_close((hid_device*)gHandle);

	return 1;
}

// knobs
#define RP_KNOB_ALT      (0x00000001)
#define RP_KNOB_VS       (0x00000002)
#define RP_KNOB_IAS      (0x00000004)
#define RP_KNOB_HDG      (0x00000008)
#define RP_KNOB_CRS      (0x00000010)

// tuning knob
#define RP_TUNE_CW       (0x00000020)
#define RP_TUNE_CCW      (0x00000040)

// buttons
#define RP_BTN_AP        (0x00000080)
#define RP_BTN_HDG       (0x00000100)
#define RP_BTN_NAV       (0x00000200)
#define RP_BTN_IAS       (0x00000400)
#define RP_BTN_ALT       (0x00000800)
#define RP_BTN_VS        (0x00001000)
#define RP_BTN_APR       (0x00002000)
#define RP_BTN_REV       (0x00004000)

// autothrottle
#define RP_AT_ON         (0x00008000)
#define RP_AT_OFF        (0x00000000)

// flaps
#define RP_FLAPS_ON      (0x00010000)
#define RP_FLAPS_OFF     (0x00020000)

// trim
#define RP_TRIM_UP       (0x00040000)
#define RP_TRIM_DOWN     (0x00080000)


#endif
