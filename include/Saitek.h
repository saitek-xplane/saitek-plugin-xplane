// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __SAITEK_H
#define __SAITEK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __TESTING__
    #include "XPLMDefs.h"
    PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc);
    PLUGIN_API void XPluginStop(void);
    PLUGIN_API void XPluginDisable(void);
    PLUGIN_API int XPluginEnable(void);
#endif

    extern const unsigned char hid_open_msg[13];
    extern void close_hid(hid_device* dev);
    extern hid_device volatile* gRpHandle;
    extern hid_device volatile* gMpHandle;
    extern hid_device volatile* gSpHandle;
    extern bool hid_init(hid_device volatile** dev, unsigned short prod_id);

#ifdef __cplusplus
}
#endif

#endif
