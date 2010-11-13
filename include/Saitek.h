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

    extern void rp_hid_init();
    extern void mp_hid_init();
    extern void sp_hid_init();
    extern void rp_hid_close();
    extern void mp_hid_close();
    extern void sp_hid_close();

#ifdef __cplusplus
}
#endif

#endif
