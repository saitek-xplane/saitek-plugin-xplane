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
    /* magnetos */
    gSpMagnetosOffCmdRef  = XPLMFindCommand(sMAGNETOS_OFF);
    gSpMagnetosRightCmdRef  = XPLMFindCommand(sMAGNETOS_RIGHT);
    gSpMagnetosLeftCmdRef  = XPLMFindCommand(sMAGNETOS_LEFT);
    gSpMagnetosBothCmdRef  = XPLMFindCommand(sMAGNETOS_BOTH);
    gSpMagnetosStartCmdRef  = XPLMFindCommand(sMAGNETOS_START);

    /* battery */
    gSpMasterBatteryOnCmdRef  = XPLMFindCommand(sMASTER_BATTERY_ON);
    gSpMasterBatteryOffCmdRef  = XPLMFindCommand(sMASTER_BATTERY_OFF);
    gSpMasterAltBatteryOnCmdRef  = XPLMFindCommand(sMASTER_ALT_BATTERY_ON);
    gSpMasterAltBatteryOffCmdRef  = XPLMFindCommand(sMASTER_ALT_BATTERY_OFF);

    /* various */
    gSpMasterAvionicsOnCmdRef  = XPLMFindCommand(sMASTER_AVIONICS_ON);
    gSpMasterAvionicsOffCmdRef  = XPLMFindCommand(sMASTER_AVIONICS_OFF);
    gSpFuelPumpOnCmdRef  = XPLMFindCommand(sFUEL_PUMP_ON);
    gSpFuelPumpOffCmdRef  = XPLMFindCommand(sFUEL_PUMP_OFF);
    gSpDeIceOnCmdRef  = XPLMFindCommand(sDE_ICE_ON);
    gSpDeIceOffCmdRef  = XPLMFindCommand(sDE_ICE_OFF);
    gSpPitotHeatOnCmdRef  = XPLMFindCommand(sPITOT_HEAT_ON);
    gSpPitotHeatOffCmdRef  = XPLMFindCommand(sPITOT_HEAT_OFF);

    gSpCowlClosedCmdRef  = XPLMFindCommand(sCOWL_CLOSED);
    gSpCowlOpenCmdRef  = XPLMFindCommand(sCOWL_OPEN);

    /* lights */
    gSpLightsPanelOnCmdRef  = XPLMFindCommand(sLIGHTS_PANEL_ON);
    gSpLightsPanelOffCmdRef  = XPLMFindCommand(sLIGHTS_PANEL_OFF);
    gSpLightsBeaconOnCmdRef  = XPLMFindCommand(sLIGHTS_BEACON_ON);
    gSpLightsBeaconOffCmdRef  = XPLMFindCommand(sLIGHTS_BEACON_OFF);
    gSpLightsNavOnCmdRef  = XPLMFindCommand(sLIGHTS_NAV_ON);
    gSpLightsNavOffCmdRef  = XPLMFindCommand(sLIGHTS_NAV_OFF);
    gSpLightsStrobeOnCmdRef  = XPLMFindCommand(sLIGHTS_STROBE_ON);
    gSpLightsStrobeOffCmdRef  = XPLMFindCommand(sLIGHTS_STROBE_OFF);
    gSpLightsTaxiOnCmdRef  = XPLMFindCommand(sLIGHTS_TAXI_ON);
    gSpLightsTaxiOffCmdRef  = XPLMFindCommand(sLIGHTS_TAXI_OFF);
    gSpLightsLandingOnCmdRef  = XPLMFindCommand(sLIGHTS_LANDING_ON);
    gSpLightsLandingOffCmdRef  = XPLMFindCommand(sLIGHTS_LANDING_OFF);

    /* landing gear */
    gSpLandingGearUpCmdRef  = XPLMFindCommand(sLANDING_GEAR_UP);
    gSpLandingGearDownCmdRef  = XPLMFindCommand(sLANDING_GEAR_DOWN);

    /*----- SwitchPanel Data Ref Assignment -----*/
    /* magnetos */
    cmd_ref = XPLMCreateCommand(sMAGNETOS_OFF, "Magnetos off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MAGNETOS_OFF);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_RIGHT, "Magnetos right for engine #1.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MAGNETOS_RIGHT);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_LEFT, "Magnetos left for engine #1.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MAGNETOS_LEFT);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_BOTH, "Magnetos both.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MAGNETOS_BOTH);

    cmd_ref = XPLMCreateCommand(sMAGNETOS_START, "Engage starter and fuel.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MAGNETOS_START);

    /* battery */
    cmd_ref = XPLMCreateCommand(sMASTER_BATTERY_ON, "Battery 1 on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MASTER_BATTERY_ON);

    cmd_ref = XPLMCreateCommand(sMASTER_BATTERY_OFF, "Battery 2 off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MASTER_BATTERY_OFF);

    cmd_ref = XPLMCreateCommand(sMASTER_ALT_BATTERY_ON, "Battery 1 on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MASTER_ALT_BATTERY_ON);

    cmd_ref = XPLMCreateCommand(sMASTER_ALT_BATTERY_OFF, "Battery 2 off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MASTER_ALT_BATTERY_OFF);

    /* various */
    cmd_ref = XPLMCreateCommand(sMASTER_AVIONICS_ON, "Avionics on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MASTER_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand(sMASTER_AVIONICS_OFF, "Avionics off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_MASTER_AVIONICS_OFF);

    cmd_ref = XPLMCreateCommand(sFUEL_PUMP_ON, "Fuel pump for engine #1 on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_FUEL_PUMP_ON);

    cmd_ref = XPLMCreateCommand(sFUEL_PUMP_OFF, "Fuel pump for engine #1 off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_FUEL_PUMP_OFF);

    cmd_ref = XPLMCreateCommand(sDE_ICE_ON, "Anti-ice: ice-detection on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_DEICE_ON);

    cmd_ref = XPLMCreateCommand(sDE_ICE_OFF, "Anti-ice: ice-detection off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_DEICE_OFF);

    cmd_ref = XPLMCreateCommand(sPITOT_HEAT_ON, "Anti-ice: pitot heat on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_PITOT_HEAT_ON);

    cmd_ref = XPLMCreateCommand(sPITOT_HEAT_OFF, "Anti-ice: pitot heat off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_PITOT_HEAT_OFF);

    cmd_ref = XPLMCreateCommand(sCOWL_CLOSED, "Cowl flaps closed.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_COWL_CLOSED);

    cmd_ref = XPLMCreateCommand(sCOWL_OPEN, " Cowl flaps open.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_COWL_OPEN);

    /* lights */
    cmd_ref = XPLMCreateCommand(sLIGHTS_PANEL_ON, "Panel brightness down a bit.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_PANEL_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_PANEL_OFF, "Panel brightness up a bit.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_PANEL_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_BEACON_ON, "Beacon lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_BEACON_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_BEACON_OFF, "Beacon lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_BEACON_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_NAV_ON, "Nav lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_NAV_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_NAV_OFF, "Nav lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_NAV_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_STROBE_ON, "Strobe lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_STROBE_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_STROBE_OFF, "Strobe lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_STROBE_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_TAXI_ON, "Taxi lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_TAXI_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_TAXI_ON, "Taxi lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_TAXI_OFF);

    cmd_ref = XPLMCreateCommand(sLIGHTS_LANDING_ON, "Landing lights on.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_LANDING_ON);

    cmd_ref = XPLMCreateCommand(sLIGHTS_LANDING_OFF, "Landing lights off.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_LIGHTS_LANDING_OFF);


    /* landing gear */
    cmd_ref = XPLMCreateCommand(sLANDING_GEAR_UP, "Landing gear up.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_GEAR_UP);

    cmd_ref = XPLMCreateCommand(sLANDING_GEAR_DOWN, "Landing gear down.");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, CMD_HNDLR_PROLOG, (void*)CMD_GEAR_DOWN);


    /*----- SwitchPanel Command Handlers -----*/
