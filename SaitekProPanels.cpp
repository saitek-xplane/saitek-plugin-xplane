// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#if defined(__WINDOWS__)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include "pport.h"
#include "ptypes.h"
#include "pasync.h"
#include "ptime.h"


#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "nedmalloc.h"
#include "defs.h"
#include "radiopanel.h"
#include "multipanel.h"
#include "switchpanel.h"
#include "overloaded.h"
#include "PanelThreads.h"
#include "SaitekProPanels.h"

float RadioPanelFlightLoopCallback(float inElapsedSinceLastCall,
                                   float inElapsedTimeSinceLastFlightLoop,
                                   int inCounter,
                                   void* inRefcon);

float MultiPanelFlightLoopCallback(float inElapsedSinceLastCall,
                                   float inElapsedTimeSinceLastFlightLoop,
                                   int inCounter,
                                   void* inRefcon);

float SwitchPanelFlightLoopCallback(float inElapsedSinceLastCall,
                                    float inElapsedTimeSinceLastFlightLoop,
                                    int inCounter,
                                    void* inRefcon);

int RadioPanelCommandHandler(XPLMCommandRef inCommand,
                             XPLMCommandPhase inPhase,
                             void* inRefcon);

int MultiPanelCommandHandler(XPLMCommandRef inCommand,
                             XPLMCommandPhase inPhase,
                             void* inRefcon);

int SwitchPanelCommandHandler(XPLMCommandRef inCommand,
                              XPLMCommandPhase inPhase,
                              void* inRefcon);

USING_PTYPES

enum {
    CMD_SYS_AVIONICS_ON,
    CMD_SYS_AVIONICS_OFF,
    CMD_SYS_AVIONICS_TOGGLE,
    CMD_FLTCTL_FLAPS_UP,
    CMD_FLTCTL_FLAPS_DOWN,
    CMD_FLTCTL_PITCHTRIM_UP,
    CMD_FLTCTL_PITCHTRIM_DOWN,
    CMD_FLTCTL_PITCHTRIM_TAKEOFF,
    CMD_OTTO_AUTOTHROTTLE_ON,
    CMD_OTTO_AUTOTHROTTLE_OFF,
    CMD_OTTO_AUTOTHROTTLE_TOGGLE,
    CMD_OTTO_HEADING,
    CMD_OTTO_NAV,
    CMD_OTTO_PITCHSYNC,
    CMD_OTTO_BACK_COURSE,
    CMD_OTTO_APPROACH,
    CMD_OTTO_AIRSPEED_UP,
    CMD_OTTO_AIRSPEED_DOWN,
    CMD_OTTO_AIRSPEED_SYNC,
    CMD_OTTO_VERTICALSPEED,
    CMD_OTTO_VERTICALSPEED_UP,
    CMD_OTTO_VERTICALSPEED_DOWN,
    CMD_OTTO_VERTICALSPEED_SYNC,
    CMD_OTTO_ALTITUDE_HOLD,
    CMD_OTTO_ALTITUDE_ARM,
    CMD_OTTO_ALTITUDE_UP,
    CMD_OTTO_ALTITUDE_DOWN,
    CMD_OTTO_ALTITUDE_SYNC
};

bool gPowerUp = true;
bool gEnabled = false;
unsigned int gFlCbCnt = 0;

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel
static const float FL_CB_INTERVAL = -1.0;

XPLMCommandRef  systems_avionics_on;
XPLMCommandRef  systems_avionics_off;
XPLMCommandRef  systems_avionics_toggle;
XPLMCommandRef  flightcontrol_flaps_up;
XPLMCommandRef  flightcontrol_flaps_down;
XPLMCommandRef  flightcontrol_pitch_trim_up;
XPLMCommandRef  flightcontrol_pitch_trim_down;
XPLMCommandRef  flightcontrol_pitch_trim_takeoff;
XPLMCommandRef  autopilot_autothrottle_on;
XPLMCommandRef  autopilot_autothrottle_off;
XPLMCommandRef  autopilot_autothrottle_toggle;
XPLMCommandRef  autopilot_heading;
XPLMCommandRef  autopilot_NAV;
XPLMCommandRef  autopilot_pitch_sync;
XPLMCommandRef  autopilot_back_course;
XPLMCommandRef  autopilot_approach;
XPLMCommandRef  autopilot_airspeed_up;
XPLMCommandRef  autopilot_airspeed_down;
XPLMCommandRef  autopilot_airspeed_sync;
XPLMCommandRef  autopilot_vertical_speed;
XPLMCommandRef  autopilot_verical_airspeed_up;
XPLMCommandRef  autopilot_verical_airspeed_down;
XPLMCommandRef  autopilot_verical_airspeed_sync;
XPLMCommandRef  autopilot_altitude_hold;
XPLMCommandRef  autopilot_altitude_arm;
XPLMCommandRef  autopilot_altitude_up;
XPLMCommandRef  autopilot_altitude_down;
XPLMCommandRef  autopilot_altitude_sync;

logfile* gLogFile;
char gLogFilePath[512] = {};


/*
 * - register the plugin
 * - check for hid connected pro panels
 * - register callbacks
 * - start threads
 *
 */
PLUGIN_API int
XPluginStart(char* outName, char* outSig, char* outDesc) {
//#ifdef __XPTESTING__
//    gLogFile = new logfile("/Users/SaitekProPanels.log\0", false);
//    gLogFile->putf("Saitek ProPanels Plugin: XPluginStart\n");
//#endif

    DPRINTF("Saitek ProPanels Plugin: XPluginStart\n");

    strcpy(outName, "SaitekProPanels");
    strcpy(outSig , "jdp.panels.saitek");
    strcpy(outDesc, "Saitek Pro Panels Plugin.");

    systems_avionics_on                 = XPLMCreateCommand("sim/systems/avionics_on","Avionics on");
    systems_avionics_off                = XPLMCreateCommand("sim/systems/avionics_off","Avionics off");
    systems_avionics_toggle             = XPLMCreateCommand("sim/systems/avionics_toggle ","Avionics toggle");
    flightcontrol_flaps_up              = XPLMCreateCommand("sim/flight_controls/flaps_up ","Flaps up a notch");
    flightcontrol_flaps_down            = XPLMCreateCommand("sim/flight_controls/flaps_down ","Flaps down a notch");
    flightcontrol_pitch_trim_up         = XPLMCreateCommand("sim/flight_controls/pitch_trim_up","Pitch Trim up");
    flightcontrol_pitch_trim_down       = XPLMCreateCommand("sim/flight_controls/pitch_trim_down ","Pitch Trim down");
    flightcontrol_pitch_trim_takeoff    = XPLMCreateCommand("sim/flight_controls/pitch_trim_takeoff","Pitch Trim takeoff");
    autopilot_autothrottle_on           = XPLMCreateCommand("sim/autopilot/autothrottle_on","Autopilot Auto Throttle on");
    autopilot_autothrottle_off          = XPLMCreateCommand("sim/autopilot/autothrottle_off","Autopilot Auto Throttle off");
    autopilot_autothrottle_toggle       = XPLMCreateCommand("sim/autopilot/autothrottle_off","Autopilot Auto Throttle toggle");
    autopilot_heading                   = XPLMCreateCommand("sim/autopilot/heading","Autopilot Heading hold");
    autopilot_NAV                       = XPLMCreateCommand("sim/autopilot/NAV","Autopilot VOR/LOC arm");
    autopilot_pitch_sync                = XPLMCreateCommand("sim/autopilot/level_change","Autopilot pitch-sync");
    autopilot_back_course               = XPLMCreateCommand("sim/autopilot/back_course","Autopilot back-course");
    autopilot_approach                  = XPLMCreateCommand("sim/autopilot/approach","Autopilot approach");
    autopilot_airspeed_up               = XPLMCreateCommand("sim/autopilot/airspeed_up","Autopilot airspeed up");
    autopilot_airspeed_down             = XPLMCreateCommand("sim/autopilot/airspeed_up","Autopilot airspeed down");
    autopilot_airspeed_sync             = XPLMCreateCommand("sim/autopilot/vertical_speed","Autopilot airspeed sync");
    autopilot_vertical_speed            = XPLMCreateCommand("sim/autopilot/vertical_speed","Autopilot VVI arm");
    autopilot_verical_airspeed_up       = XPLMCreateCommand("sim/autopilot/vertical_speed_up","Autopilot VVI up");
    autopilot_verical_airspeed_down     = XPLMCreateCommand("sim/autopilot/vertical_speed_down","Autopilot VVI down");
    autopilot_verical_airspeed_sync     = XPLMCreateCommand("sim/autopilot/airspeed_up","Autopilot VVI dync");
    autopilot_altitude_hold             = XPLMCreateCommand("sim/autopilot/altitude_hold","Autopilot altitude select or hold");
    autopilot_altitude_arm              = XPLMCreateCommand("sim/autopilot/altitude_arm ","Autopilot altitude-hold arm");
    autopilot_altitude_up               = XPLMCreateCommand("sim/autopilot/altitude_up","Autopilot altitude up");
    autopilot_altitude_down             = XPLMCreateCommand("sim/autopilot/altitude_down","Autopilot altitude down");
    autopilot_altitude_sync             = XPLMCreateCommand("sim/autopilot/altitude_sync ","Autopilot altitude sync");

    XPLMRegisterCommandHandler(systems_avionics_on,                 MultiPanelCommandHandler, 0, (void*) CMD_SYS_AVIONICS_ON);
    XPLMRegisterCommandHandler(systems_avionics_off,                MultiPanelCommandHandler, 0, (void*) CMD_SYS_AVIONICS_OFF);
    XPLMRegisterCommandHandler(systems_avionics_toggle,             MultiPanelCommandHandler, 0, (void*) CMD_SYS_AVIONICS_TOGGLE);
    XPLMRegisterCommandHandler(flightcontrol_flaps_up,              MultiPanelCommandHandler, 0, (void*) CMD_FLTCTL_FLAPS_UP);
    XPLMRegisterCommandHandler(flightcontrol_flaps_down,            MultiPanelCommandHandler, 0, (void*) CMD_FLTCTL_FLAPS_DOWN);
    XPLMRegisterCommandHandler(flightcontrol_pitch_trim_up,         MultiPanelCommandHandler, 0, (void*) CMD_FLTCTL_PITCHTRIM_UP);
    XPLMRegisterCommandHandler(flightcontrol_pitch_trim_down,       MultiPanelCommandHandler, 0, (void*) CMD_FLTCTL_PITCHTRIM_DOWN);
    XPLMRegisterCommandHandler(flightcontrol_pitch_trim_takeoff,    MultiPanelCommandHandler, 0, (void*) CMD_FLTCTL_PITCHTRIM_TAKEOFF);
    XPLMRegisterCommandHandler(autopilot_autothrottle_on,           MultiPanelCommandHandler, 0, (void*) CMD_OTTO_AUTOTHROTTLE_ON);
    XPLMRegisterCommandHandler(autopilot_autothrottle_off,          MultiPanelCommandHandler, 0, (void*) CMD_OTTO_AUTOTHROTTLE_OFF);
    XPLMRegisterCommandHandler(autopilot_autothrottle_toggle,       MultiPanelCommandHandler, 0, (void*) CMD_OTTO_AUTOTHROTTLE_TOGGLE);
    XPLMRegisterCommandHandler(autopilot_heading,                   MultiPanelCommandHandler, 0, (void*) CMD_OTTO_HEADING);
    XPLMRegisterCommandHandler(autopilot_NAV,                       MultiPanelCommandHandler, 0, (void*) CMD_OTTO_NAV);
    XPLMRegisterCommandHandler(autopilot_pitch_sync,                MultiPanelCommandHandler, 0, (void*) CMD_OTTO_PITCHSYNC);
    XPLMRegisterCommandHandler(autopilot_back_course,               MultiPanelCommandHandler, 0, (void*) CMD_OTTO_BACK_COURSE);
    XPLMRegisterCommandHandler(autopilot_approach,                  MultiPanelCommandHandler, 0, (void*) CMD_OTTO_APPROACH);
    XPLMRegisterCommandHandler(autopilot_airspeed_up,               MultiPanelCommandHandler, 0, (void*) CMD_OTTO_AIRSPEED_UP);
    XPLMRegisterCommandHandler(autopilot_airspeed_down,             MultiPanelCommandHandler, 0, (void*) CMD_OTTO_AIRSPEED_DOWN);
    XPLMRegisterCommandHandler(autopilot_airspeed_sync,             MultiPanelCommandHandler, 0, (void*) CMD_OTTO_AIRSPEED_SYNC);
    XPLMRegisterCommandHandler(autopilot_vertical_speed,            MultiPanelCommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED);
    XPLMRegisterCommandHandler(autopilot_verical_airspeed_up,       MultiPanelCommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED_UP);
    XPLMRegisterCommandHandler(autopilot_verical_airspeed_down,     MultiPanelCommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED_DOWN);
    XPLMRegisterCommandHandler(autopilot_verical_airspeed_sync,     MultiPanelCommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED_SYNC);
    XPLMRegisterCommandHandler(autopilot_altitude_hold,             MultiPanelCommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_HOLD);
    XPLMRegisterCommandHandler(autopilot_altitude_arm,              MultiPanelCommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_ARM);
    XPLMRegisterCommandHandler(autopilot_altitude_up,               MultiPanelCommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_UP);
    XPLMRegisterCommandHandler(autopilot_altitude_down,             MultiPanelCommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_DOWN);
    XPLMRegisterCommandHandler(autopilot_altitude_sync,             MultiPanelCommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_SYNC);

//    gLogFile->putf("Saitek ProPanels Plugin: commands initialized\n");
    DPRINTF("Saitek ProPanels Plugin: commands initialized\n");

    init_hid(&gRpHandle, RP_PROD_ID);
    init_hid(&gMpHandle, MP_PROD_ID);
    init_hid(&gSpHandle, SP_PROD_ID);

    pexchange((int*)&threads_run, true);

    ToPanelThread*     tp;
    FromPanelThread*   fp;

    // radio panel
    tp = new ToPanelThread(gRpHandle, &gRp_ijq, &gRpTrigger, RP_PROD_ID);
    fp = new FromPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gRpTrigger, RP_PROD_ID, &rpProcOutData);

    tp->start();
    fp->start();

    // multi panel
    tp = new ToPanelThread(gMpHandle, &gMp_ijq, &gMpTrigger, MP_PROD_ID);
    fp = new FromPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gMpTrigger, MP_PROD_ID, &mpProcOutData);

    tp->start();
    fp->start();

    // switch panel
    tp = new ToPanelThread(gSpHandle, &gSp_ijq, &gSpTrigger, SP_PROD_ID);
    fp = new FromPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gSpTrigger, SP_PROD_ID, &spProcOutData);

    tp->start();
    fp->start();

    PanelsCheckThread* pc = new PanelsCheckThread();
    pc->start();

    if (gRpHandle) { DPRINTF("Saitek ProPanels Plugin: gRpHandle\n"); gRpTrigger.post(); }
    if (gMpHandle) { DPRINTF("Saitek ProPanels Plugin: gMpHandle\n"); gMpTrigger.post(); }
    if (gSpHandle) { DPRINTF("Saitek ProPanels Plugin: gSpHandle\n"); gSpTrigger.post(); }

 //   gLogFile->putf("Saitek ProPanels Plugin: Panel threads running\n");
    DPRINTF("Saitek ProPanels Plugin: Panel threads running\n");

    XPLMRegisterFlightLoopCallback(RadioPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(MultiPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(SwitchPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);

//    gLogFile->putf("Saitek ProPanels Plugin: startup completed\n");
    DPRINTF("Saitek ProPanels Plugin: startup completed\n");

    return 1;
}

int RadioPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    return 1;
}

int SwitchPanelCommandHandler(XPLMCommandRef    inCommand,
                              XPLMCommandPhase  inPhase,
                              void*             inRefcon) {
    return 1;
}

int MultiPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    char str[50];

    switch (reinterpret_cast<long>(inRefcon)) {
        case CMD_SYS_AVIONICS_ON:
            strcpy(str, "system avionics on\n");
            break;
        case CMD_SYS_AVIONICS_OFF:
            strcpy(str, "system avionics off\n");
            break;
        case CMD_SYS_AVIONICS_TOGGLE:
            strcpy(str, "system avionics toggle\n");
            break;
        case CMD_FLTCTL_FLAPS_UP:
            strcpy(str, "flaps up\n");
            break;
        case CMD_FLTCTL_FLAPS_DOWN:
            strcpy(str, "flaps down\n");
            break;
        case CMD_FLTCTL_PITCHTRIM_UP:
            strcpy(str, "pitch trim up\n");
            break;
        case CMD_FLTCTL_PITCHTRIM_DOWN:
            strcpy(str, "pitch trim down\n");
            break;
        case CMD_FLTCTL_PITCHTRIM_TAKEOFF:
            strcpy(str, "pitch trim takeoff\n");
            break;
        case CMD_OTTO_AUTOTHROTTLE_ON:
            strcpy(str, "auto throttle on\n");
            break;
        case CMD_OTTO_AUTOTHROTTLE_OFF:
            strcpy(str, "auto throttle off\n");
            break;
        case CMD_OTTO_AUTOTHROTTLE_TOGGLE:
            strcpy(str, "auto throttle toggle\n");
            break;
        case CMD_OTTO_HEADING:
            strcpy(str, "auto pilot heading\n");
            break;
        case CMD_OTTO_NAV:
            strcpy(str, "auto pilot navigation\n");
            break;
        case CMD_OTTO_PITCHSYNC:
            strcpy(str, "auto pilot pitch sync\n");
            break;
        case CMD_OTTO_BACK_COURSE:
            strcpy(str, "auto pilot back course\n");
            break;
        case CMD_OTTO_AIRSPEED_UP:
            strcpy(str, "auto pilot airspeed up\n");
            break;
        case CMD_OTTO_AIRSPEED_DOWN:
            strcpy(str, "auto pilot airspeed down\n");
            break;
        case CMD_OTTO_AIRSPEED_SYNC:
            strcpy(str, "auto pilot airspeed sync\n");
            break;
        case CMD_OTTO_VERTICALSPEED:
            strcpy(str, "auto pilot vertical speed\n");
            break;
        case CMD_OTTO_VERTICALSPEED_UP:
            strcpy(str, "auto pilot vertical up\n");
            break;
        case CMD_OTTO_VERTICALSPEED_DOWN:
            strcpy(str, "auto pilot vertical down\n");
            break;
        case CMD_OTTO_VERTICALSPEED_SYNC:
            strcpy(str, "auto pilot vertical sync\n");
            break;
        case CMD_OTTO_ALTITUDE_HOLD:
            strcpy(str, "auto pilot altitude hold\n");
            break;
        case CMD_OTTO_ALTITUDE_ARM:
            strcpy(str, "auto pilot altitude arm\n");
            break;
        case CMD_OTTO_ALTITUDE_UP:
            strcpy(str, "auto pilot altitude up\n");
            break;
        case CMD_OTTO_ALTITUDE_DOWN:
            strcpy(str, "auto pilot altitude down\n");
            break;
        case CMD_OTTO_ALTITUDE_SYNC:
            strcpy(str, "auto pilot altitude sync\n");
            break;
        default:
            strcpy(str, "UNKNOWN\n");
            break;
    }

    XPLMSpeakString(str);

    return 1;
}

/*
 *
 *
 */
float RadioPanelFlightLoopCallback(float   inElapsedSinceLastCall,
                         float   inElapsedTimeSinceLastFlightLoop,
                         int     inCounter,
                         void*   inRefcon) {
    return 1.0;
}

/*
 *
 *
 */
float SwitchPanelFlightLoopCallback(float   inElapsedSinceLastCall,
                         float   inElapsedTimeSinceLastFlightLoop,
                         int     inCounter,
                         void*   inRefcon) {
    return 1.0;
}

/*
 *
 *
 */
float MultiPanelFlightLoopCallback(float   inElapsedSinceLastCall,
                         float   inElapsedTimeSinceLastFlightLoop,
                         int     inCounter,
                         void*   inRefcon) {

    if ((gFlCbCnt % PANEL_CHECK_INTERVAL) == 0) {
        if (gEnabled) {
            gPcTrigger.post();
        }
    }

    message* msg = gMp_ojq.getmessage(MSG_NOWAIT);
    unsigned int cmd;

    if (msg) {
        cmd = *((unsigned int*)((myjob*) msg)->buf);

        switch (cmd) {
            case HIDREAD_PITCHTRIM_UP:
//XPLMSpeakString("pitch up");
                XPLMCommandKeyStroke(xplm_key_elvtrimU);
                break;
            case HIDREAD_PITCHTRIM_DOWN:
//XPLMSpeakString("pitch down");
                XPLMCommandKeyStroke(xplm_key_elvtrimD);
                break;
            case HIDREAD_FLAPSDOWN:
//XPLMSpeakString("flaps down");
                XPLMCommandKeyStroke(xplm_key_flapsdn);
                break;
            case HIDREAD_FLAPSUP:
//XPLMSpeakString("flaps up");
                XPLMCommandKeyStroke(xplm_key_flapsup);
                break;
            default:
                break;
        }

        free(((myjob*) msg)->buf);
        delete msg;
    }

    gFlCbCnt++;

    return 1.0;
}

/*
 *
 */
PLUGIN_API void
XPluginStop(void) {
    unsigned char* x;
// TODO: do the message and protocol
    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 0xff;
    gRp_ijq.post(new myjob(x));

    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 0xff;
    gMp_ijq.post(new myjob(x));

    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 0xff;
    gSp_ijq.post(new myjob(x));

    pexchange((int*)&pc_run, false);
    pexchange((int*)&threads_run, false);

    gPcTrigger.post();
    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    hid_close((hid_device*)gRpHandle);
    hid_close((hid_device*)gMpHandle);
    hid_close((hid_device*)gSpHandle);

    psleep(500);
    XPLMUnregisterFlightLoopCallback(RadioPanelFlightLoopCallback, NULL);
    XPLMUnregisterFlightLoopCallback(MultiPanelFlightLoopCallback, NULL);
    XPLMUnregisterFlightLoopCallback(SwitchPanelFlightLoopCallback, NULL);
}

/*
 *
 */
PLUGIN_API void
XPluginDisable(void) {
    gEnabled = false;
    gRpTrigger.reset();
    gMpTrigger.reset();
    gSpTrigger.reset();
}

/*
 *
 */
PLUGIN_API int
XPluginEnable(void) {
    gEnabled = true;

    if (gPowerUp) {
        gPowerUp = false;
        return 1;
    }

    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    return 1;
}

PLUGIN_API void
XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void* inParam) {
    //
}
