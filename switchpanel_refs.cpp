// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

    /*  SwitchPanel command and data refs file.
     *
     *  The code in this file is included in SaitekProPanels.cpp via the
     *  preprocessor, therefore, it isn't compilable and doesn't need to be
     *  included in the build file list.
     */

    /*----- SwitchPanel Command Ref Assignment -----*/
    // 0 = OFF, 1 = ON
    // magnetos
    gSpMagnetosOffCmdRef  = XPLMFindCommand(sMAGNETOS_OFF_CR);
    gSpMagnetosRightCmdRef  = XPLMFindCommand(sMAGNETOS_RIGHT_CR);
    gSpMagnetosLeftCmdRef  = XPLMFindCommand(sMAGNETOS_LEFT_CR);
    gSpMagnetosBothCmdRef  = XPLMFindCommand(sMAGNETOS_BOTH_CR);
    gSpMagnetosStartCmdRef  = XPLMFindCommand(sMAGNETOS_START_CR);

    // battery
    gSpMasterBatteryOnCmdRef  = XPLMFindCommand(sMASTER_BATTERY_ON_CR);
    gSpMasterBatteryOffCmdRef  = XPLMFindCommand(sMASTER_BATTERY_OFF_CR);
    gSpMasterAltBatteryOnCmdRef  = XPLMFindCommand(sMASTER_ALT_BATTERY_ON_CR);
    gSpMasterAltBatteryOffCmdRef  = XPLMFindCommand(sMASTER_ALT_BATTERY_OFF_CR);

    // various
    gSpMasterAvionicsOnCmdRef  = XPLMFindCommand(sMASTER_AVIONICS_ON_CR);
    gSpMasterAvionicsOffCmdRef  = XPLMFindCommand(sMASTER_AVIONICS_OFF_CR);
    gSpFuelPumpOnCmdRef  = XPLMFindCommand(sFUEL_PUMP_ON_CR);
    gSpFuelPumpOffCmdRef  = XPLMFindCommand(sFUEL_PUMP_OFF_CR);
    gSpDeIceOnCmdRef  = XPLMFindCommand(sDE_ICE_ON_CR);
    gSpDeIceOffCmdRef  = XPLMFindCommand(sDE_ICE_OFF_CR);
    gSpPitotHeatOnCmdRef  = XPLMFindCommand(sPITOT_HEAT_ON_CR);
    gSpPitotHeatOffCmdRef  = XPLMFindCommand(sPITOT_HEAT_OFF_CR);

    gSpCowlClosedCmdRef  = XPLMFindCommand(sCOWL_CLOSED_CR);
    gSpCowlOpenCmdRef  = XPLMFindCommand(sCOWL_OPEN_CR);

    // lights
    gSpLightsPanelOnCmdRef  = XPLMFindCommand(sLIGHTS_PANEL_ON_CR);
    gSpLightsPanelOffCmdRef  = XPLMFindCommand(sLIGHTS_PANEL_OFF_CR);
    gSpLightsBeaconOnCmdRef  = XPLMFindCommand(sLIGHTS_BEACON_ON_CR);
    gSpLightsBeaconOffCmdRef  = XPLMFindCommand(sLIGHTS_BEACON_OFF_CR);
    gSpLightsNavOnCmdRef  = XPLMFindCommand(sLIGHTS_NAV_ON_CR);
    gSpLightsNavOffCmdRef  = XPLMFindCommand(sLIGHTS_NAV_OFF_CR);
    gSpLightsStrobeOnCmdRef  = XPLMFindCommand(sLIGHTS_STROBE_ON_CR);
    gSpLightsStrobeOffCmdRef  = XPLMFindCommand(sLIGHTS_STROBE_OFF_CR);
    gSpLightsTaxiOnCmdRef  = XPLMFindCommand(sLIGHTS_TAXI_ON_CR);
    gSpLightsTaxiOffCmdRef  = XPLMFindCommand(sLIGHTS_TAXI_OFF_CR);
    gSpLightsLandingOnCmdRef  = XPLMFindCommand(sLIGHTS_LANDING_ON_CR);
    gSpLightsLandingOffCmdRef  = XPLMFindCommand(sLIGHTS_LANDING_OFF_CR);

    // landing gear
    gSpLandingGearUpCmdRef  = XPLMFindCommand(sLANDING_GEAR_UP_CR);
    gSpLandingGearDownCmdRef  = XPLMFindCommand(sLANDING_GEAR_DOWN_CR);

    /*----- SwitchPanel Data Ref Assignment -----*/
    /* magnetos */
    cmd_ref = XPLMCreateCommand(sMAGNETOS_OFF_CR, "Magnetos off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MAGNETOS_OFF);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_RIGHT_CR, "Magnetos right for engine #1.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MAGNETOS_RIGHT);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_LEFT_CR, "Magnetos left for engine #1.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MAGNETOS_LEFT);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_BOTH_CR, "Magnetos both.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MAGNETOS_BOTH);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_START_CR, "Engage starter and fuel.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MAGNETOS_START);

    /* battery */
    cmd_ref = XPLMCreateCommand(sMASTER_BATTERY_ON_CR, "Battery 1 on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MASTER_BATTERY_ON);

    cmd_ref = XPLMCreateCommand(sMASTER_BATTERY_OFF_CR, "Battery 2 off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MASTER_BATTERY_OFF);

    cmd_ref = XPLMCreateCommand(sMASTER_ALT_BATTERY_ON_CR, "Battery 1 on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MASTER_ALT_BATTERY_ON);

    cmd_ref = XPLMCreateCommand(sMASTER_ALT_BATTERY_OFF_CR, "Battery 2 off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MASTER_ALT_BATTERY_OFF);

    /* various */
    cmd_ref = XPLMCreateCommand(sMASTER_AVIONICS_ON_CR, "Avionics on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MASTER_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand(sMASTER_AVIONICS_OFF_CR, "Avionics off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_MASTER_AVIONICS_OFF);

    cmd_ref = XPLMCreateCommand(sFUEL_PUMP_ON_CR, "Fuel pump for engine #1 on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_FUEL_PUMP_ON);

    cmd_ref = XPLMCreateCommand(sFUEL_PUMP_OFF_CR, "Fuel pump for engine #1 off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_FUEL_PUMP_OFF);

    cmd_ref = XPLMCreateCommand(sDE_ICE_ON_CR, "Anti-ice: ice-detection on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_DEICE_ON);

    cmd_ref = XPLMCreateCommand(sDE_ICE_OFF_CR, "Anti-ice: ice-detection off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_DEICE_OFF);

    cmd_ref = XPLMCreateCommand(sPITOT_HEAT_ON_CR, "Anti-ice: pitot heat on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_PITOT_HEAT_ON);

    cmd_ref = XPLMCreateCommand(sPITOT_HEAT_OFF_CR, "Anti-ice: pitot heat off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_PITOT_HEAT_OFF);

    cmd_ref = XPLMCreateCommand(sCOWL_CLOSED_CR, "Cowl flaps closed.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_COWL_CLOSED);

    cmd_ref = XPLMCreateCommand(sCOWL_OPEN_CR, " Cowl flaps open.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_COWL_OPEN);

    /* lights */
    cmd_ref = XPLMCreateCommand(sLIGHTS_PANEL_ON_CR, "Panel brightness down a bit.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_PANEL_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_PANEL_OFF_CR, "Panel brightness up a bit.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_PANEL_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_BEACON_ON_CR, "Beacon lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_BEACON_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_BEACON_OFF_CR, "Beacon lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_BEACON_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_NAV_ON_CR, "Nav lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_NAV_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_NAV_OFF_CR, "Nav lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_NAV_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_STROBE_ON_CR, "Strobe lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_STROBE_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_STROBE_OFF_CR, "Strobe lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_STROBE_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_TAXI_ON_CR, "Taxi lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_TAXI_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_TAXI_ON_CR, "Taxi lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_TAXI_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_LANDING_ON_CR, "Landing lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_LANDING_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_LANDING_OFF_CR, "Landing lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_LIGHTS_LANDING_OFF);


    /* landing gear */
    cmd_ref = XPLMCreateCommand(sLANDING_GEAR_UP_CR, "Landing gear up.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_GEAR_UP);

    cmd_ref = XPLMCreateCommand(sLANDING_GEAR_DOWN_CR, "Landing gear down.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)SP_CMD_GEAR_DOWN);


    /*----- SwitchPanel Command Handlers -----*/
