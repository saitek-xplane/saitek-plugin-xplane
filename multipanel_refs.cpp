// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

    /*  MultiPanel command and data refs file.
     *
     *  The code in this file is included in SaitekProPanels.cpp via the
     *  preprocessor, therefore, it isn't compilable and doesn't need to be
     *  included in the build file list.
     */

    /*----- MultiPanel Command Ref assignment -----*/

    /* readouts */
    gMpAltDnCmdRef           = XPLMFindCommand(sALTITUDE_DOWN);
    gMpAltUpCmdRef           = XPLMFindCommand(sALTITUDE_UP);
    gMpVrtclSpdDnCmdRef      = XPLMFindCommand(sVERTICAL_SPEED_DOWN);
    gMpVrtclSpdUpCmdRef      = XPLMFindCommand(sVERTICAL_SPEED_UP);
    gMpAsDnCmdRef            = XPLMFindCommand(sAIRSPEED_DOWN);
    gMpAsUpCmdRef            = XPLMFindCommand(sAIRSPEED_UP);
    gMpHdgDnCmdRef           = XPLMFindCommand(sHEADING_DOWN);
    gMpHdgUpCmdRef           = XPLMFindCommand(sHEADING_UP);
    gMpObsHsiDnCmdRef        = XPLMFindCommand(sOBS_HSI_DOWN);
    gMpObsHsiUpCmdRef        = XPLMFindCommand(sOBS_HSI_UP);

    /* buttons */
    gMpApArmedCmdRef         = XPLMFindCommand(sFLIGHT_DIR_ON_ONLY);
    gMpApOnCmdRef            = XPLMFindCommand(sSERVOS_AND_FLIGHT_DIR_ON);
    gMpApOffCmdRef           = XPLMFindCommand(sSERVOS_AND_FLIGHT_DIR_OFF);
    gMpHdgCmdRef             = XPLMFindCommand(sHEADING);
    gMpNavCmdRef             = XPLMFindCommand(sNAV);
    gMpLvlChngCmdRef         = XPLMFindCommand(sLEVEL_CHANGE);
    gMpAltHoldCmdRef         = XPLMFindCommand(sALTITUDE_HOLD);
    gMpHdgCmdRef             = XPLMFindCommand(sALTITUDE_ARM);
    gMpVrtclSpdCmdRef        = XPLMFindCommand(sVERTICAL_SPEED);
    gMpAppCmdRef             = XPLMFindCommand(sAPPROACH);
    gMpBkCrsCmdRef           = XPLMFindCommand(sBACK_COURSE);

    /* auto throttle switch */
    gMpAtThrrtlOnCmdRef      = XPLMFindCommand(sAUTOTHROTTLE_ON);
    gMpAtThrrtlOffCmdRef     = XPLMFindCommand(sAUTOTHROTTLE_OFF);
    gMpAtThrrtlTgglCmdRef    = XPLMFindCommand(sAUTOTHROTTLE_TOGGLE);

    /* flap handle */
    gMpFlpsDnCmdRef          = XPLMFindCommand(sFLAPS_DOWN);
    gMpFlpsUpCmdRef          = XPLMFindCommand(sFLAPS_UP);

    /* pitch trim wheel */
    gMpPtchTrmDnCmdRef       = XPLMFindCommand(sPITCH_TRIM_DOWN);
    gMpPtchTrmUpCmdRef       = XPLMFindCommand(sPITCH_TRIM_UP);
    gMpPtchTrmTkOffCmdRef    = XPLMFindCommand(sPITCH_TRIM_TAKEOFF);

    /*----- MultiPanel Data Ref assignment -----*/

    // 0: off, 1: on, 2: autopilot engaged
    gMpFlghtDirModeDataRef    = XPLMFindDataRef(sFLIGHT_DIRECTOR_MODE);
    gMpAltDataRef            = XPLMFindDataRef(sALTITUDE_DIAL_FT);
//    gMpAltDataRef             = XPLMFindDataRef(sALTITUDE_HOLD_FT);
    gMpVrtVelDataRef          = XPLMFindDataRef(sVVI_DIAL_FPM);
    gMpArspdDataRef           = XPLMFindDataRef(sAIRSPEED_DIAL_KTS_MACH);
    gMpHdgMagDataRef          = XPLMFindDataRef(sHEADING_DIAL_DEG_MAG_PILOT);
    gMpHsiObsDegMagPltDataRef = XPLMFindDataRef(sHSI_OBS_DEG_MAG_PILOT);
    // gMpArspdDataRef           = XPLMFindDataRef(sAIRSPEED);
    // gMpVrtVelDataRef          = XPLMFindDataRef(sVERTICAL_VELOCITY);
    // gMpAltDataRef             = XPLMFindDataRef(sALTITUDE);
    // gMpHdgMagDataRef          = XPLMFindDataRef(sHEADING_MAG);

    // 0 = off, 1 = armed, 2 = captured
    gMpAltHoldStatDataRef     = XPLMFindDataRef(sALTITUDE_HOLD_STATUS);
    gMpApprchStatDataRef      = XPLMFindDataRef(sAPPROACH_STATUS);
    gMpApStateDataRef         = XPLMFindDataRef(sAUTOPILOT_STATE);
    gMpBckCrsStatDataRef      = XPLMFindDataRef(sBACKCOURSE_STATUS);
    gMpHdgStatDataRef         = XPLMFindDataRef(sHEADING_STATUS);
    gMpNavStatDataRef         = XPLMFindDataRef(sNAV_STATUS);
    gMpSpdStatDataRef         = XPLMFindDataRef(sSPEED_STATUS);
    gMpVviStatDataRef         = XPLMFindDataRef(sVVI_STATUS);

#if 0
    /*----- MultiPanel Command Handlers -----*/
    cmd_ref = XPLMCreateCommand(sAVIONICS_POWER_ON, "Avionics Power On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_SYS_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand(, "Avionics Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_SYS_AVIONICS_OFF);

    cmd_ref = XPLMCreateCommand(sBATTERY_ON, "Battery 1 On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_ELEC_BATTERY1_ON);

    cmd_ref = XPLMCreateCommand(, "Battery 1 Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_ELEC_BATTERY1_OFF);
#endif

    /*- MultiPanel */
    /* auto throttle */
    cmd_ref = XPLMCreateCommand(sAUTOTHROTTLE_ON, "Auto Throttle On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_AUTOTHROTTLE_ON);

    cmd_ref = XPLMCreateCommand(sAUTOTHROTTLE_OFF, "Auto Throttle Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_AUTOTHROTTLE_OFF);

    /* readouts */
    cmd_ref = XPLMCreateCommand(sALTITUDE_DOWN, "AutoPilot ALT Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_ALT_DN);

    cmd_ref = XPLMCreateCommand(sALTITUDE_UP, "AutoPilot ALT Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_ALT_UP);

    cmd_ref = XPLMCreateCommand(sVERTICAL_SPEED_DOWN, "AutoPilot VS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_VS_DN);

    cmd_ref = XPLMCreateCommand(sVERTICAL_SPEED_UP, "AutoPilot VS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_VS_UP);

    cmd_ref = XPLMCreateCommand(sAIRSPEED_DOWN, "AutoPilot IAS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_IAS_DN);

    cmd_ref = XPLMCreateCommand(sAIRSPEED_UP, "AutoPilot IAS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_IAS_UP);

    cmd_ref = XPLMCreateCommand(sHEADING_DOWN, "AutoPilot HDG Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_HDG_DN);

    cmd_ref = XPLMCreateCommand(sHEADING_UP, "AutoPilot HDG Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_HDG_UP);

    cmd_ref = XPLMCreateCommand(sOBS_HSI_DOWN, "AutoPilot CRS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_CRS_DN);

    cmd_ref = XPLMCreateCommand(sOBS_HSI_UP, "AutoPilot CRS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_CRS_UP);

    /* buttons */
    cmd_ref = XPLMCreateCommand(sFLIGHT_DIR_ON_ONLY, "AutoPilot Armed");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_ARMED);

    cmd_ref = XPLMCreateCommand(sSERVOS_AND_FLIGHT_DIR_ON, "AutoPilot On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_ON);

    cmd_ref = XPLMCreateCommand(sSERVOS_AND_FLIGHT_DIR_OFF, "AutoPilot Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_OFF);

    cmd_ref = XPLMCreateCommand(sALTITUDE_ARM, "AutoPilot HDG");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_HDG_BTN);

    cmd_ref = XPLMCreateCommand(sNAV, "AutoPilot NAV");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_NAV_BTN);

    cmd_ref = XPLMCreateCommand(sLEVEL_CHANGE, "AutoPilot IAS");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_IAS_BTN);

    cmd_ref = XPLMCreateCommand(sALTITUDE_HOLD, "AutoPilot ALT");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_ALT_BTN);

    cmd_ref = XPLMCreateCommand(sVERTICAL_SPEED, "AutoPilot VS");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_VS_BTN);

    cmd_ref = XPLMCreateCommand(sAPPROACH, "AutoPilot APR");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_APR_BTN);

    cmd_ref = XPLMCreateCommand(sBACK_COURSE, "AutoPilot REV");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_OTTO_REV_BTN);

