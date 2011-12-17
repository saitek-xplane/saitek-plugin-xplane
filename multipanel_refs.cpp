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
    gMpAltDnCmdRef           = XPLMFindCommand("sim/autopilot/altitude_down");
    gMpAltUpCmdRef           = XPLMFindCommand("sim/autopilot/altitude_up");
    gMpVrtclSpdDnCmdRef      = XPLMFindCommand("sim/autopilot/vertical_speed_down");
    gMpVrtclSpdUpCmdRef      = XPLMFindCommand("sim/autopilot/vertical_speed_up");
    gMpAsDnCmdRef            = XPLMFindCommand("sim/autopilot/airspeed_down");
    gMpAsUpCmdRef            = XPLMFindCommand("sim/autopilot/airspeed_up");
    gMpHdgDnCmdRef           = XPLMFindCommand("sim/autopilot/heading_down");
    gMpHdgUpCmdRef           = XPLMFindCommand("sim/autopilot/heading_up");
    gMpObsHsiDnCmdRef        = XPLMFindCommand("sim/radios/obs_HSI_down");
    gMpObsHsiUpCmdRef        = XPLMFindCommand("sim/radios/obs_HSI_up");

    /* buttons */
    gMpApArmedCmdRef         = XPLMFindCommand("sim/autopilot/flight_dir_on_only");
    gMpApOnCmdRef            = XPLMFindCommand("sim/autopilot/servos_and_flight_dir_on ");
    gMpApOffCmdRef           = XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off");
    gMpHdgCmdRef             = XPLMFindCommand("sim/autopilot/heading");
    gMpNavCmdRef             = XPLMFindCommand("sim/autopilot/NAV");
    gMpLvlChngCmdRef         = XPLMFindCommand("sim/autopilot/level_change");
//    gMpAltHoldCmdRef         = XPLMFindCommand("sim/autopilot/altitude_hold");
    gMpHdgCmdRef             = XPLMFindCommand("sim/autopilot/altitude_arm");
    gMpVrtclSpdCmdRef        = XPLMFindCommand("sim/autopilot/vertical_speed");
    gMpAppCmdRef             = XPLMFindCommand("sim/autopilot/approach");
    gMpBkCrsCmdRef           = XPLMFindCommand("sim/autopilot/back_course");

    /* auto throttle switch */
    gMpAtThrrtlOnCmdRef      = XPLMFindCommand("sim/autopilot/autothrottle_on");
    gMpAtThrrtlOffCmdRef     = XPLMFindCommand("sim/autopilot/autothrottle_off");
    gMpAtThrrtlTgglCmdRef    = XPLMFindCommand("sim/autopilot/autothrottle_toggle");

    /* flap handle */
    gMpFlpsDnCmdRef          = XPLMFindCommand("sim/flight_controls/flaps_down");
    gMpFlpsUpCmdRef          = XPLMFindCommand("sim/flight_controls/flaps_up");

    /* pitch trim wheel */
    gMpPtchTrmDnCmdRef       = XPLMFindCommand("sim/flight_controls/pitch_trim_down");
    gMpPtchTrmUpCmdRef       = XPLMFindCommand("sim/flight_controls/pitch_trim_up");
    gMpPtchTrmTkOffCmdRef    = XPLMFindCommand("sim/flight_controls/pitch_trim_takeoff");

    /*----- MultiPanel Data Ref assignment -----*/
    // 0: off, 1: on, 2: autopilot engaged
    gMpFlghtDirModeDataRef    = XPLMFindDataRef("sim/cockpit2/autopilot/flight_director_mode");

    gMpAltDataRef            = XPLMFindDataRef("sim/cockpit2/autopilot/altitude_dial_ft");
//    gMpAltDataRef             = XPLMFindDataRef("sim/cockpit2/autopilot/altitude_hold_ft");
    gMpVrtVelDataRef          = XPLMFindDataRef("sim/cockpit2/autopilot/vvi_dial_fpm");
    gMpArspdDataRef           = XPLMFindDataRef("sim/cockpit2/autopilot/airspeed_dial_kts_mach");
    gMpHdgMagDataRef          = XPLMFindDataRef("sim/cockpit2/autopilot/heading_dial_deg_mag_pilot");
    gMpHsiObsDegMagPltDataRef = XPLMFindDataRef("sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot");

    // gMpArspdDataRef           = XPLMFindDataRef("sim/cockpit/autopilot/airspeed");
    // gMpVrtVelDataRef          = XPLMFindDataRef("sim/cockpit/autopilot/vertical_velocity");
    // gMpAltDataRef             = XPLMFindDataRef("sim/cockpit/autopilot/altitude");
    // gMpHdgMagDataRef          = XPLMFindDataRef("sim/cockpit/autopilot/heading_mag");

    // 0 = off, 1 = armed, 2 = captured
    gMpAltHoldStatDataRef     = XPLMFindDataRef("sim/cockpit2/autopilot/altitude_hold_status");
    gMpApprchStatDataRef      = XPLMFindDataRef("sim/cockpit2/autopilot/approach_status");
    gMpApStateDataRef         = XPLMFindDataRef("sim/cockpit/autopilot/autopilot_state");
    gMpBckCrsStatDataRef      = XPLMFindDataRef("sim/cockpit2/autopilot/backcourse_status");
    gMpHdgStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/heading_status");
    gMpNavStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/nav_status");
    gMpSpdStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/speed_status");
    gMpVviStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/vvi_status");

#if 0
    /*----- MultiPanel Command Handlers -----*/
    cmd_ref = XPLMCreateCommand((const char*)gAvPwrOnCmdRef, "Avionics On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_SYS_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand((const char*)gAvPwrOffCmdRef, "Avionics Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_SYS_AVIONICS_OFF);

    cmd_ref = XPLMCreateCommand((const char*)gBatPwrOnCmdRef, "Battery 1 On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_ELEC_BATTERY1_ON);

    cmd_ref = XPLMCreateCommand((const char*)gBatPwrOffCmdRef, "Battery 1 Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_ELEC_BATTERY1_OFF);

    /*- MultiPanel */
    /* auto throttle */
    cmd_ref = XPLMCreateCommand((const char*)gMpAtThrrtlOnCmdRef, "Auto Throttle On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_AUTOTHROTTLE_ON);

    cmd_ref = XPLMCreateCommand((const char*)gMpAtThrrtlOffCmdRef, "Auto Throttle Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_AUTOTHROTTLE_OFF);

    /* readouts */
    cmd_ref = XPLMCreateCommand((const char*)gMpAltDnCmdRef, "AutoPilot ALT Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_ALT_DN);

    cmd_ref = XPLMCreateCommand((const char*)gMpAltUpCmdRef, "AutoPilot ALT Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_ALT_UP);

    cmd_ref = XPLMCreateCommand((const char*)gMpVrtclSpdDnCmdRef, "AutoPilot VS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_VS_DN);

    cmd_ref = XPLMCreateCommand((const char*)gMpVrtclSpdUpCmdRef, "AutoPilot VS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_VS_UP);

    cmd_ref = XPLMCreateCommand((const char*)gMpAsDnCmdRef, "AutoPilot IAS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_IAS_DN);

    cmd_ref = XPLMCreateCommand((const char*)gMpAsUpCmdRef, "AutoPilot IAS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_IAS_UP);

    cmd_ref = XPLMCreateCommand((const char*)gMpHdgDnCmdRef, "AutoPilot HDG Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_HDG_DN);

    cmd_ref = XPLMCreateCommand((const char*)gMpHdgUpCmdRef, "AutoPilot HDG Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_HDG_UP);

    cmd_ref = XPLMCreateCommand((const char*)gMpObsHsiDnCmdRef, "AutoPilot CRS Down");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_CRS_DN);

    cmd_ref = XPLMCreateCommand((const char*)gMpObsHsiUpCmdRef, "AutoPilot CRS Up");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_CRS_UP);

    /* buttons */
    cmd_ref = XPLMCreateCommand((const char*)gMpApArmedCmdRef, "AutoPilot Armed");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_ARMED);

    cmd_ref = XPLMCreateCommand((const char*)gMpApOnCmdRef, "AutoPilot On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_ON);

    cmd_ref = XPLMCreateCommand((const char*)gMpApOffCmdRef, "AutoPilot Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_OFF);

    cmd_ref = XPLMCreateCommand((const char*)gMpHdgCmdRef, "AutoPilot HDG");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_HDG_BTN);

    cmd_ref = XPLMCreateCommand((const char*)gMpNavCmdRef, "AutoPilot NAV");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_NAV_BTN);

    cmd_ref = XPLMCreateCommand((const char*)gMpLvlChngCmdRef, "AutoPilot IAS");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_IAS_BTN);

    cmd_ref = XPLMCreateCommand((const char*)gMpAltHoldCmdRef, "AutoPilot ALT");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_ALT_BTN);

    cmd_ref = XPLMCreateCommand((const char*)gMpVrtclSpdCmdRef, "AutoPilot VS");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_VS_BTN);

    cmd_ref = XPLMCreateCommand((const char*)gMpAppCmdRef, "AutoPilot APR");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_APR_BTN);

    cmd_ref = XPLMCreateCommand((const char*)gMpBkCrsCmdRef, "AutoPilot REV");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_REV_BTN);
#endif
