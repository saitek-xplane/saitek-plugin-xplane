// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

    /*  MultiPanel command and data refs file.
     *
     *  The code in this file is included in SaitekProPanels.cpp via the
     *  preprocessor, therefore, it isn't compilable and doesn't need to be
     *  included in the build file list.
     */

    // ----- MultiPanel Command Ref assignment -----
    // readouts
    gMpAltDnCmdRef          = XPLMFindCommand(sMP_ALTITUDE_DOWN_CR);
    gMpAltUpCmdRef          = XPLMFindCommand(sMP_ALTITUDE_UP_CR);
    gMpVrtclSpdDnCmdRef     = XPLMFindCommand(sMP_VERTICAL_SPEED_DOWN_CR);
    gMpVrtclSpdUpCmdRef     = XPLMFindCommand(sMP_VERTICAL_SPEED_UP_CR);
    gMpAsDnCmdRef           = XPLMFindCommand(sMP_AIRSPEED_DOWN_CR);
    gMpAsUpCmdRef           = XPLMFindCommand(sMP_AIRSPEED_UP_CR);
    gMpHdgDnCmdRef          = XPLMFindCommand(sMP_HEADING_DOWN_CR);
    gMpHdgUpCmdRef          = XPLMFindCommand(sMP_HEADING_UP_CR);
    gMpObsHsiDnCmdRef       = XPLMFindCommand(sMP_OBS_HSI_DOWN_CR);
    gMpObsHsiUpCmdRef       = XPLMFindCommand(sMP_OBS_HSI_UP_CR);

    // buttons
    gMpApToggleCmdRef       = XPLMFindCommand(sMP_FDIR_SERVOS_TOGGLE_CR);
    gMpApFdUpOneCmdRef      = XPLMFindCommand(sMP_FDIR_SERVOS_UP_ONE_CR);
    gMpApArmedCmdRef        = XPLMFindCommand(sMP_FLIGHT_DIR_ON_ONLY_CR);
    gMpApOnCmdRef           = XPLMFindCommand(sMP_SERVOS_AND_FLIGHT_DIR_ON_CR);
    gMpApOffCmdRef          = XPLMFindCommand(sMP_SERVOS_AND_FLIGHT_DIR_OFF_CR);
    gMpHdgCmdRef            = XPLMFindCommand(sMP_HEADING_CR);
    gMpNavArmCmdRef         = XPLMFindCommand(sMP_NAV_ARM_CR);
    gMpLvlChngCmdRef        = XPLMFindCommand(sMP_LEVEL_CHANGE_CR);
    gMpAltHoldCmdRef        = XPLMFindCommand(sMP_ALTITUDE_HOLD_CR);
    gMpAltArmCmdRef         = XPLMFindCommand(sMP_ALTITUDE_ARM_CR);
    gMpVrtclSpdCmdRef       = XPLMFindCommand(sMP_VERTICAL_SPEED_CR);
    gMpAppCmdRef            = XPLMFindCommand(sMP_APPROACH_CR);
    gMpBkCrsCmdRef          = XPLMFindCommand(sMP_BACK_COURSE_CR);

    // auto throttle switch
    gMpAtThrrtlOnCmdRef     = XPLMFindCommand(sMP_AUTOTHROTTLE_ON_CR);
    gMpAtThrrtlOffCmdRef    = XPLMFindCommand(sMP_AUTOTHROTTLE_OFF_CR);
    gMpAtThrrtlTgglCmdRef   = XPLMFindCommand(sMP_AUTOTHROTTLE_TOGGLE_CR);

    // flap handle
    gMpFlpsDnCmdRef         = XPLMFindCommand(sMP_FLAPS_DOWN_CR);
    gMpFlpsUpCmdRef         = XPLMFindCommand(sMP_FLAPS_UP_CR);

    // pitch trim wheel
    gMpPtchTrmDnCmdRef      = XPLMFindCommand(sMP_PITCH_TRIM_DOWN_CR);
    gMpPtchTrmUpCmdRef      = XPLMFindCommand(sMP_PITCH_TRIM_UP_CR);
    gMpPtchTrmTkOffCmdRef   = XPLMFindCommand(sMP_PITCH_TRIM_TAKEOFF_CR);

    /*----- MultiPanel Data Ref assignment -----*/
    gMpAltDataRef               = XPLMFindDataRef(sMP_ALTITUDE_DR);
    gMpAltHoldFtDataRef         = XPLMFindDataRef(sMP_ALTITUDE_HOLD_FT_DR);
    gMpVrtVelDataRef            = XPLMFindDataRef(sMP_VVI_DIAL_FPM_DR);
    gMpArspdDataRef             = XPLMFindDataRef(sMP_AIRSPEED_DR);
    gMpHdgMagDataRef            = XPLMFindDataRef(sMP_HEADING_DIAL_DEG_MAG_PILOT_DR);
    gMpHsiObsDegMagPltDataRef   = XPLMFindDataRef(sMP_HSI_OBS_DEG_MAG_PILOT_DR);
    gMpHsiSrcSelPltDataRef      = XPLMFindDataRef(sMP_HSI_SOURCE_SELECT_PILOT_DR);
    gMpNav1CrsDefMagPltDataRef  = XPLMFindDataRef(sMP_NAV1_COURSE_DEG_MAG_PILOT_DR);
    gMpNav2CrsDefMagPltDataRef  = XPLMFindDataRef(sMP_NAV2_COURSE_DEG_MAG_PILOT_DR);
    gMpGpsCourseDataRef         = XPLMFindDataRef(sMP_GPS_COURSE_DR);
//    gMpAltDialFtDataRef      = XPLMFindDataRef(sMP_ALTITUDE_DIAL_FT_DR);
//    gMpArspdDataRef          = XPLMFindDataRef(sMP_AIRSPEED_DIAL_KTS_MACH_DR);
//    gMpVrtVelDataRef          = XPLMFindDataRef(sMP_VERTICAL_VELOCITY_DR);
//    gMpHdgMagDataRef          = XPLMFindDataRef(sMP_HEADING_MAG_DR);

    // 0 = off, 1 = armed, 2 = captured
    gMpFlghtDirModeDataRef      = XPLMFindDataRef(sMP_FLIGHT_DIRECTOR_MODE_DR);
    gMpApOnDataRef              = XPLMFindDataRef(sMP_AUTOPILOT_ON_DR);
    gMpAltHoldStatBtnDataRef    = XPLMFindDataRef(sMP_ALTITUDE_HOLD_STATUS_DR);
    gMpApprchStatBtnDataRef     = XPLMFindDataRef(sMP_APPROACH_STATUS_DR);
    gMpBckCrsStatBtnDataRef     = XPLMFindDataRef(sMP_BACKCOURSE_STATUS_DR);
    gMpHdgStatBtnDataRef        = XPLMFindDataRef(sMP_HEADING_STATUS_DR);
    gMpNavStatBtnDataRef        = XPLMFindDataRef(sMP_NAV_STATUS_DR);
    gMpSpdStatBtnDataRef        = XPLMFindDataRef(sMP_SPEED_STATUS_DR);
    gMpVviStatBtnDataRef        = XPLMFindDataRef(sMP_VVI_STATUS_DR);

#if 0
    /*----- MultiPanel Command Handlers -----*/
    cmd_ref = XPLMCreateCommand(, "Avionics Power On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_SYS_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand(, "Avionics Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_SYS_AVIONICS_OFF);

    cmd_ref = XPLMCreateCommand(, "Battery 1 On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_ELEC_BATTERY1_ON);

    cmd_ref = XPLMCreateCommand(, "Battery 1 Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_ELEC_BATTERY1_OFF);
#endif

    //--- MultiPanel
    // auto throttle
    cmd_ref = XPLMCreateCommand(sMP_AUTOTHROTTLE_ON_CR, "Auto Throttle On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_OTTO_AUTOTHROTTLE_ON);

    cmd_ref = XPLMCreateCommand(sMP_AUTOTHROTTLE_ON_CR, "Auto Throttle On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_OTTO_AUTOTHROTTLE_ON);

    cmd_ref = XPLMCreateCommand(sMP_AUTOTHROTTLE_TOGGLE_CR, "Auto Throttle Toggle");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_OTTO_AUTOTHROTTLE_TOGGLE);

    // flaps
    cmd_ref = XPLMCreateCommand(sMP_FLAPS_UP_CR, "Flaps Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_FLAPS_UP);

    cmd_ref = XPLMCreateCommand(sMP_FLAPS_DOWN_CR, "Flaps Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_FLAPS_DOWN);

    // pitch trim
//    cmd_ref = XPLMCreateCommand(sMP_PITCH_TRIM_UP_CR, "Pitch Trim Up");
//    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_PITCHTRIM_UP);

//    cmd_ref = XPLMCreateCommand(sMP_PITCH_TRIM_DOWN_CR, "Pitch Trim Down");
//    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)MP_CMD_PITCHTRIM_DOWN);

    // readouts
    cmd_ref = XPLMCreateCommand(sMP_ALTITUDE_DOWN_CR, "AutoPilot ALT Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_ALT_DN);

    cmd_ref = XPLMCreateCommand(sMP_ALTITUDE_UP_CR, "AutoPilot ALT Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_ALT_UP);

    cmd_ref = XPLMCreateCommand(sMP_VERTICAL_SPEED_DOWN_CR, "AutoPilot VS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_VS_DN);

    cmd_ref = XPLMCreateCommand(sMP_VERTICAL_SPEED_UP_CR, "AutoPilot VS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_VS_UP);

    cmd_ref = XPLMCreateCommand(sMP_AIRSPEED_DOWN_CR, "AutoPilot IAS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_IAS_DN);

    cmd_ref = XPLMCreateCommand(sMP_AIRSPEED_UP_CR, "AutoPilot IAS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_IAS_UP);

    cmd_ref = XPLMCreateCommand(sMP_HEADING_DOWN_CR, "AutoPilot HDG Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_HDG_DN);

    cmd_ref = XPLMCreateCommand(sMP_HEADING_UP_CR, "AutoPilot HDG Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_HDG_UP);

    cmd_ref = XPLMCreateCommand(sMP_OBS_HSI_DOWN_CR, "AutoPilot CRS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_CRS_DN);

    cmd_ref = XPLMCreateCommand(sMP_OBS_HSI_UP_CR, "AutoPilot CRS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_CRS_UP);

    // buttons

    cmd_ref = XPLMCreateCommand(sMP_FDIR_SERVOS_UP_ONE_CR, "Flight Director Up One");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_FD_UP_ONE);

    cmd_ref = XPLMCreateCommand(sMP_FLIGHT_DIR_ON_ONLY_CR, "AutoPilot Armed");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_ARMED);

    cmd_ref = XPLMCreateCommand(sMP_SERVOS_AND_FLIGHT_DIR_ON_CR, "AutoPilot On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_ON);

    cmd_ref = XPLMCreateCommand(sMP_SERVOS_AND_FLIGHT_DIR_OFF_CR, "AutoPilot Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_OFF);

    cmd_ref = XPLMCreateCommand(sMP_NAV_ARM_CR, "AutoPilot NAV");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_NAV_BTN);

    cmd_ref = XPLMCreateCommand(sMP_LEVEL_CHANGE_CR, "AutoPilot IAS");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_IAS_BTN);

//    cmd_ref = XPLMCreateCommand(sMP_ALTITUDE_ARM_CR, "AutoPilot ALT Arm");
//    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_ALT_ARM_BTN);

    cmd_ref = XPLMCreateCommand(sMP_ALTITUDE_HOLD_CR, "AutoPilot ALT Hold");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_ALT_HOLD_BTN);

    cmd_ref = XPLMCreateCommand(sMP_VERTICAL_SPEED_CR, "AutoPilot VS");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_VS_BTN);

    cmd_ref = XPLMCreateCommand(sMP_APPROACH_CR, "AutoPilot APR");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_APR_BTN);

    cmd_ref = XPLMCreateCommand(sMP_BACK_COURSE_CR, "AutoPilot REV");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, CMD_HNDLR_EPILOG, (void*)MP_CMD_OTTO_REV_BTN);

