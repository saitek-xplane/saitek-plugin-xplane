// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __SAITEKPROPANELS_H
#define __SAITEKPROPANELS_H

#include "pstreams.h"

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

    extern pt::logfile* gLogFile;

#ifdef __cplusplus
}
#endif

#endif
