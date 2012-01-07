// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef SAITEKPROPANELS_H
#define SAITEKPROPANELS_H


//#include "defs.h"

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
    extern uint32_t         gMpTuningThresh;
    extern XPLMDataRef      gApStateRef;
    extern XPLMDataRef      gApAutoThrottleRef;
    extern XPLMDataRef      gApElevTrimRef;
//    extern XPLMDataRef      gApElevTrimUpAnnuncRef;
//    extern XPLMDataRef      gApElevTrimDownAnnuncRef;
    extern XPLMDataRef      gApMaxElevTrimRef;

    extern XPLMDataRef      gApAltHoldRef;
    extern XPLMDataRef      gApVsHoldRef;
    extern XPLMDataRef      gApIasHoldRef;
    extern XPLMDataRef      gApHdgHoldRef;
    extern XPLMDataRef      gApCrsHoldRef;

    extern pt::logfile*     gLogFile;

#ifdef __cplusplus
}
#endif

#endif /* SAITEKPROPANELS */
