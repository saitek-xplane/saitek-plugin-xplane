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
#include "pstreams.h"

#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "nedmalloc.h"
#include "defs.h"
#include "PanelThreads.h"
#include "overloaded.h"
#include "Saitek.h"

float FlightLoopCallback(float inElapsedSinceLastCall,
                         float inElapsedTimeSinceLastFlightLoop,
                         int inCounter,
                         void* inRefcon);

int CommandHandler(XPLMCommandRef inCommand,
                   XPLMCommandPhase inPhase,
                   void* inRefcon);

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

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel
static const float RP_CB_INTERVAL = -1.0;
static const float MP_CB_INTERVAL = 10.0;
static const float SP_CB_INTERVAL = 10.0;

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

// panel threads
hid_device volatile*    gRpHandle = NULL;
hid_device volatile*    gMpHandle = NULL;
hid_device volatile*    gSpHandle = NULL;

const unsigned char hid_open_msg[] = {};
const unsigned char hid_close_msg[] = {0x00, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00};

USING_PTYPES

void rp_hid_init() {
//    if (gRpHandle) {
//        hid_device* tmp = (hid_device*)gRpHandle;
//        pexchange((void**)(&gRpHandle), NULL);
//        hid_delete_report(tmp);
//    }

    pexchange((void**)(&gRpHandle), (void*)hid_open(VENDOR_ID, RP_PROD_ID, NULL));

    if (gRpHandle) {
        hid_set_nonblocking((hid_device*)gRpHandle, HID_BLOCKING);
        hid_send_feature_report((hid_device*)gRpHandle, hid_open_msg, OUT_BUF_CNT);
    }
}

void mp_hid_init() {
    pexchange((void**)(&gMpHandle), (void*)hid_open(VENDOR_ID, MP_PROD_ID, NULL));

    if (gMpHandle) {
        hid_set_nonblocking((hid_device*)gMpHandle, HID_BLOCKING);
        hid_send_feature_report((hid_device*)gMpHandle, hid_open_msg, OUT_BUF_CNT);
    }
}

void sp_hid_init() {
    pexchange((void**)(&gSpHandle), (void*)hid_open(VENDOR_ID, SP_PROD_ID, NULL));

    if (gSpHandle) {
        hid_set_nonblocking((hid_device*)gSpHandle, HID_BLOCKING);
        hid_send_feature_report((hid_device*)gSpHandle, hid_open_msg, OUT_BUF_CNT);
    }
}

void rp_hid_close() {
    if (gRpHandle) {
        hid_device* tmp = (hid_device*)gRpHandle;
        pexchange((void**)(&gRpHandle), NULL);
        hid_close((hid_device*)tmp);
    }
}

void mp_hid_close() {
    if (gMpHandle) {
        hid_device* tmp = (hid_device*)gMpHandle;
        pexchange((void**)(&gMpHandle), NULL);
        hid_close(tmp);
    }
}

void sp_hid_close() {
    if (gSpHandle) {
        hid_device* tmp = (hid_device*)gSpHandle;
        pexchange((void**)(&gSpHandle), NULL);
        hid_close(tmp);
    }
}

/*
 * - register the plugin
 * - check for hid connected pro panels
 * - register callbacks
 * - start threads
 *
 */
PLUGIN_API int
XPluginStart(char* outName, char* outSig, char* outDesc) {
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

    XPLMRegisterCommandHandler(systems_avionics_on,                 CommandHandler, 0, (void*) CMD_SYS_AVIONICS_ON);
    XPLMRegisterCommandHandler(systems_avionics_off,                CommandHandler, 0, (void*) CMD_SYS_AVIONICS_OFF);
    XPLMRegisterCommandHandler(systems_avionics_toggle,             CommandHandler, 0, (void*) CMD_SYS_AVIONICS_TOGGLE);
    XPLMRegisterCommandHandler(flightcontrol_flaps_up,              CommandHandler, 0, (void*) CMD_FLTCTL_FLAPS_UP);
    XPLMRegisterCommandHandler(flightcontrol_flaps_down,            CommandHandler, 0, (void*) CMD_FLTCTL_FLAPS_DOWN);
    XPLMRegisterCommandHandler(flightcontrol_pitch_trim_up,         CommandHandler, 0, (void*) CMD_FLTCTL_PITCHTRIM_UP);
    XPLMRegisterCommandHandler(flightcontrol_pitch_trim_down,       CommandHandler, 0, (void*) CMD_FLTCTL_PITCHTRIM_DOWN);
    XPLMRegisterCommandHandler(flightcontrol_pitch_trim_takeoff,    CommandHandler, 0, (void*) CMD_FLTCTL_PITCHTRIM_TAKEOFF);
    XPLMRegisterCommandHandler(autopilot_autothrottle_on,           CommandHandler, 0, (void*) CMD_OTTO_AUTOTHROTTLE_ON);
    XPLMRegisterCommandHandler(autopilot_autothrottle_off,          CommandHandler, 0, (void*) CMD_OTTO_AUTOTHROTTLE_OFF);
    XPLMRegisterCommandHandler(autopilot_autothrottle_toggle,       CommandHandler, 0, (void*) CMD_OTTO_AUTOTHROTTLE_TOGGLE);
    XPLMRegisterCommandHandler(autopilot_heading,                   CommandHandler, 0, (void*) CMD_OTTO_HEADING);
    XPLMRegisterCommandHandler(autopilot_NAV,                       CommandHandler, 0, (void*) CMD_OTTO_NAV);
    XPLMRegisterCommandHandler(autopilot_pitch_sync,                CommandHandler, 0, (void*) CMD_OTTO_PITCHSYNC);
    XPLMRegisterCommandHandler(autopilot_back_course,               CommandHandler, 0, (void*) CMD_OTTO_BACK_COURSE);
    XPLMRegisterCommandHandler(autopilot_approach,                  CommandHandler, 0, (void*) CMD_OTTO_APPROACH);
    XPLMRegisterCommandHandler(autopilot_airspeed_up,               CommandHandler, 0, (void*) CMD_OTTO_AIRSPEED_UP);
    XPLMRegisterCommandHandler(autopilot_airspeed_down,             CommandHandler, 0, (void*) CMD_OTTO_AIRSPEED_DOWN);
    XPLMRegisterCommandHandler(autopilot_airspeed_sync,             CommandHandler, 0, (void*) CMD_OTTO_AIRSPEED_SYNC);
    XPLMRegisterCommandHandler(autopilot_vertical_speed,            CommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED);
    XPLMRegisterCommandHandler(autopilot_verical_airspeed_up,       CommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED_UP);
    XPLMRegisterCommandHandler(autopilot_verical_airspeed_down,     CommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED_DOWN);
    XPLMRegisterCommandHandler(autopilot_verical_airspeed_sync,     CommandHandler, 0, (void*) CMD_OTTO_VERTICALSPEED_SYNC);
    XPLMRegisterCommandHandler(autopilot_altitude_hold,             CommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_HOLD);
    XPLMRegisterCommandHandler(autopilot_altitude_arm,              CommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_ARM);
    XPLMRegisterCommandHandler(autopilot_altitude_up,               CommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_UP);
    XPLMRegisterCommandHandler(autopilot_altitude_down,             CommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_DOWN);
    XPLMRegisterCommandHandler(autopilot_altitude_sync,             CommandHandler, 0, (void*) CMD_OTTO_ALTITUDE_SYNC);

    DPRINTF("Saitek ProPanels Plugin: commands initialized\n");

    rp_hid_init();
    mp_hid_init();
    sp_hid_init();

    DPRINTF("Saitek ProPanels Plugin: hid init completed\n");

    pexchange((int*)&threads_run, true);

    ToPanelThread*     tp;
    FromPanelThread*   fp;

    // radio panel
    tp = new ToPanelThread(gRpHandle, &gRp_ijq, &gRpTrigger, RP_PROD_ID);
    fp = new FromPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gRpTrigger, RP_PROD_ID);

    tp->start();
    fp->start();

    // multi panel
    tp = new ToPanelThread(gMpHandle, &gMp_ijq, &gMpTrigger, MP_PROD_ID);
    fp = new FromPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gMpTrigger, MP_PROD_ID);

    tp->start();
    fp->start();

    // switch panel
    tp = new ToPanelThread(gSpHandle, &gSp_ijq, &gSpTrigger, SP_PROD_ID);
    fp = new FromPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gSpTrigger, SP_PROD_ID);

    tp->start();
    fp->start();

    PanelsCheckThread* pc = new PanelsCheckThread();
    pc->start();

    if (gRpHandle) { gRpTrigger.post(); }
    if (gMpHandle) { gMpTrigger.post(); }
    if (gSpHandle) { gSpTrigger.post(); }

    DPRINTF("Saitek ProPanels Plugin: PanelsCheckThread running\n");

    XPLMRegisterFlightLoopCallback(FlightLoopCallback, RP_CB_INTERVAL, NULL);

    DPRINTF("Saitek ProPanels Plugin: startup completed\n");

    return 1;
}

int CommandHandler(XPLMCommandRef    inCommand,
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
float FlightLoopCallback(float   inElapsedSinceLastCall,
                float   inElapsedTimeSinceLastFlightLoop,
                int     inCounter,
                void*   inRefcon) {

 //   unsigned char x = 0;
//DPRINTF_VA("Hello from rpSendMsg callback: %d\n", inCounter);

//#ifdef __XPTESTING__
//    char str[50];
//    strcpy(str, "RP send\n");
//    XPLMSpeakString(str);
//#endif

    // get data from xplane and pass it on
//    gRp_ojq.post(new myjob(alloc_buf));

    // get data from xplane and pass it on
//    gMp_ojq.post(new myjob(alloc_buf));
    // get data from xplane and pass it on
//    gSp_ojq.post(new myjob(alloc_buf));
//    message* msg = gRp_ijq.getmessage(MSG_NOWAIT);
//    message* msg = gMp_ijq.getmessage(MSG_NOWAIT);
//    message* msg = gSp_ijq.getmessage(MSG_NOWAIT);

    // get message from panel and set xplane data
//    if (msg) {
//        XPLMSpeakString("message received\n");
//        free(((myjob*) msg)->buf);
//        delete msg;
//    }

//    if (test_flag1) {
//        XPLMSpeakString("one\n");
//        pexchange((int*)&test_flag1, 0);
//    }

//    if (test_flag2) {
//        XPLMSpeakString("two\n");
//        pexchange((int*)&test_flag2, 0);

//        if (test_flag2) {
//            XPLMSpeakString("again\n");
//            pexchange((int*)&test_flag2, 0);
//        }
//    }

//    if (test_flag3) {
//        XPLMSpeakString("three\n");
//        pexchange((int*)&test_flag3, 0);
//    }

    return 1.0;
}

/*
 *
 */
PLUGIN_API void
XPluginStop(void) {
    unsigned char* x;

    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 0xff;
    gRp_ijq.post(new myjob(x));

    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 0xff;
    gMp_ijq.post(new myjob(x));

    x = (unsigned char*) malloc(sizeof(unsigned char));
    *x = 0xff;
    gSp_ijq.post(new myjob(x));

    // die gracefully
    pexchange((int*)&pc_run, false);
    pexchange((int*)&threads_run, false);

    // innocuos if already running
    gPcTrigger.post();
    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    psleep(2000);
    XPLMUnregisterFlightLoopCallback(FlightLoopCallback, NULL);

    rp_hid_close();
    mp_hid_close();
    sp_hid_close();
}

/*
 *
 */
PLUGIN_API void
XPluginDisable(void) {
    gPcTrigger.reset();
    gRpTrigger.reset();
    gMpTrigger.reset();
    gSpTrigger.reset();
}

/*
 *
 */
PLUGIN_API int
XPluginEnable(void) {
    if (gPowerUp) {
        gPowerUp = false;
        return 1;
    }

    gPcTrigger.post();
    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    return 1;
}

PLUGIN_API void
XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void* inParam) {
    //
}
