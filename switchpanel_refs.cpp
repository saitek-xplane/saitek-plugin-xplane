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
    /* lights */
    gSpLightsLandingOnCmdRef  = XPLMFindCommand(sLIGHTS_LANDING_ON);

    /*----- SwitchPanel Data Ref Assignment -----*/
    /* lights */
    cmd_ref = XPLMCreateCommand(sLIGHTS_LANDING_ON, "Landing Lights On");
    XPLMRegisterCommandHandler(cmd_ref, SwitchPanelCommandHandler, true, (void*)CMD_LIGHTS_LANDING_ON);

    /*----- SwitchPanel Command Handlers -----*/
