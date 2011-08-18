// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef MULTIPANEL_H
#define MULTIPANEL_H


#ifdef __cplusplus
extern "C" {
#endif

    /* Command Refs */
    extern XPLMCommandRef gMpAsDnCmdRef;
    extern XPLMCommandRef gMpAsUpCmdRef;
    extern XPLMCommandRef gMpAltDnCmdRef;
    extern XPLMCommandRef gMpAltUpCmdRef;
    extern XPLMCommandRef gMpAltHoldCmdRef;
    extern XPLMCommandRef gMpAppCmdRef;
    extern XPLMCommandRef gMpAtThrrtlTgglCmdRef;
    extern XPLMCommandRef gMpBkCrsCmdRef;
    extern XPLMCommandRef gMpFdirSrvUp1CmdRef;
    extern XPLMCommandRef gMpFdirSrvDn1CmdRef;
    extern XPLMCommandRef gMpFlpsDnCmdRef;
    extern XPLMCommandRef gMpFlpsUpCmdRef;
    extern XPLMCommandRef gMpHdgCmdRef;
    extern XPLMCommandRef gMpHdgDnCmdRef;
    extern XPLMCommandRef gMpHdgUpCmdRef;
    extern XPLMCommandRef gMpLvlChngCmdRef;
    extern XPLMCommandRef gMpNavCmdRef;
    extern XPLMCommandRef gMpObsHsiDnCmdRef;
    extern XPLMCommandRef gMpObsHsiUpCmdRef;
    extern XPLMCommandRef gMpPtchTrmDnCmdRef;
    extern XPLMCommandRef gMpPtchTrmUpCmdRef;
    extern XPLMCommandRef gMpPtchTrmTkOffCmdRef;
    extern XPLMCommandRef gMpSrvsFlghtDirOffCmdRef;
    extern XPLMCommandRef gMpVrtclSpdDnCmdRef;
    extern XPLMCommandRef gMpVrtclSpdUpCmdRef;
    extern XPLMCommandRef gMpVrtclSpdCmdRef;

    /* Data Refs */
    extern XPLMDataRef gMpOttoOvrrde;
    extern XPLMDataRef gMpArspdDataRef;
    extern XPLMDataRef gMpAltDataRef;
    extern XPLMDataRef gMpAltHoldStatDataRef;
    extern XPLMDataRef gMpApprchStatDataRef;
    extern XPLMDataRef gMpApStateDataRef;
    extern XPLMDataRef gMpAvncsOnDataRef;
    extern XPLMDataRef gMpBckCrsStatDataRef;
    extern XPLMDataRef gMpBttryOnDataRef;
    extern XPLMDataRef gMpFlghtDirModeDataRef;
    extern XPLMDataRef gMpHdgMagDataRef;
    extern XPLMDataRef gMpHdgStatDataRef;
    extern XPLMDataRef gMpHsiObsDegMagPltDataRef;
    extern XPLMDataRef gMpNavStatDataRef;
    extern XPLMDataRef gMpSpdStatDataRef;
    extern XPLMDataRef gMpVrtVelDataRef;
    extern XPLMDataRef gMpVviStatDataRef;

    extern void mp_proc_data(uint32_t data);

#ifdef __cplusplus
}
#endif

#endif  /* MULTIPANEL_H */

