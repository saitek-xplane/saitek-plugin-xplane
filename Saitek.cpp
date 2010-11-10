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

#include "Saitek.h"
#include "PanelThreads.h"
#include "overloaded.h"
#include "nedmalloc.h"

/*
             callback      [plugin]       out msg              set panel
    x-plane ------------> out (get) data ------------> thread ------------> panel

             callback      [plugin]        in msg              read panel
    x-plane ------------> in (set) data <------------- thread <------------ panel
*/


/*
            u8_cnt = ((myjob*) msg)->u8_amt;
            u8_buf = ((myjob*) msg)->data_buf;

datarefs
-------
    float   lat = XPLMGetDataf(gPlaneLat);
XPLMCommandKeyStroke((XPLMCommandKeyID) inItemRef);


sim/cockpit/autopilot/autopilot_mode    int y   enum    The autopilot master mode (off=0, flight director=1, on=2)
sim/cockpit/autopilot/airspeed_mode int n   enum    Airspeed mode for the autopilot. DEPRECATED
sim/cockpit/autopilot/heading_mode  int n   enum    Lateral navigation mode (GPS, heading, L-Nav approach). DEPRECATED
sim/cockpit/autopilot/altitude_mode int n   enum    Vertical navigation (alt hold, VVI hold, V-Nav approach). DEPRECATED
sim/cockpit/autopilot/backcourse_on int y   boolean Back course selection
sim/cockpit/autopilot/altitude  float   y   ftmsl   Altitude dialed into the AP
sim/cockpit/autopilot/current_altitude  float   y   ftmsl   Currently held altitude (remembered until you hit flchg)
sim/cockpit/autopilot/vertical_velocity float   y   fpm Vertical speed to hold
sim/cockpit/autopilot/airspeed  float   y   knots_mach  Airspeed to hold, this changes from knots to a mach number
sim/cockpit/autopilot/heading   float   y   degt    The heading to fly (true, legacy)
sim/cockpit/autopilot/heading_mag   float   y   degm    The heading to fly (magnetic, preferred) pilot
sim/cockpit/autopilot/heading_mag2  float   y   degm    The heading to fly (magnetic, preferred) copilot
sim/cockpit/autopilot/airspeed_is_mach  int y   boolean Is our airspeed a mach number (this is writable if the panel has the button, otherwise sim controls)
sim/cockpit/autopilot/flight_director_pitch float   y   degrees The recommended pitch from the Flight Director.  Use override_flightdir
sim/cockpit/autopilot/flight_director_roll  float   y   degrees The recommended roll from the Flight Director.  Use override_flightdir
sim/cockpit/autopilot/autopilot_state   int y   flags   Various autopilot engage modes, etc.  See docs for flags
sim/cockpit/autopilot/heading_roll_mode int y   enum    Bank limit - 0 = auto, 1-6 = 5-30 degrees of bank
sim/cockpit/autopilot/mode_hnav int y   enum    Localizer mode (off, armed, enaged). DEPRECATED
sim/cockpit/autopilot/mode_gls  int y   enum    Glide-slope mode (off, armed, engaged). DEPRECATED
sim/cockpit/autopilot/syn_hold_deg  float   y   degrees The pitch held when in pitch-hold mode.
sim/cockpit/autopilot/nav_steer_deg_mag float   y   degrees Heading to fly in nav mode - write this when override_nav_heading is set.  Useful for making a custom GPS that flies arcs.

commands
---------
sim/systems/avionics_on                            Avionics on.
sim/systems/avionics_off                           Avionics off.
sim/systems/avionics_toggle                        Avionics toggle.

sim/autopilot/hsi_select_nav_1                     HSI shows nav 1.
sim/autopilot/hsi_select_nav_2                     HSI shows nav 2.
sim/autopilot/hsi_select_gps                       HSI shows GPS.
sim/autopilot/flight_dir_on_only                   Flight directory only.
sim/autopilot/servos_and_flight_dir_on             Servos on.
sim/autopilot/servos_and_flight_dir_off            Disco servos, flt dir.
sim/autopilot/servos_fdir_yawd_off                 Disco servos, flt dir, yawdamp.
sim/autopilot/servos_fdir_yawd_trim_off            Disco servos, flt dir, yawdamp, trim.
sim/autopilot/fdir_servos_down_one                 Autopilot Mode down (on->fdir->off).
sim/autopilot/fdir_servos_up_one                   Autopilot Mode up (off->fdir->on).
sim/autopilot/fdir_servos_toggle                   Toggle between on and off.
sim/autopilot/control_wheel_steer                  Control-wheel steer.
sim/autopilot/autothrottle_on                      Autopilot auto-throttle on.
sim/autopilot/autothrottle_off                     Autopilot auto-throttle off.
sim/autopilot/autothrottle_toggle                  Autopilot auto-throttle toggle.
sim/autopilot/wing_leveler                         Autopilot wing-level.
sim/autopilot/heading                              Autopilot heading-hold.
sim/autopilot/NAV                                  Autopilot VOR/LOC arm.
sim/autopilot/pitch_sync                           Autopilot pitch-sync.
sim/autopilot/level_change                         Autopilot level change.
sim/autopilot/vertical_speed                       Autopilot vertical speed.
sim/autopilot/altitude_hold                        Autopilot altitude select or hold.
sim/autopilot/altitude_arm                         Autopilot altitude-hold ARM.
sim/autopilot/vnav                                 Autopilot VNAV.
sim/autopilot/FMS                                  Autopilot FMS.
sim/autopilot/glide_slope                          Autopilot glideslope.
sim/autopilot/back_course                          Autopilot back-course.
sim/autopilot/approach                             Autopilot approach.
sim/autopilot/take_off_go_around                   Autopilot take-off go-around.
sim/autopilot/reentry                              Autopilot re-entry.
sim/autopilot/terrain_following                    Autopilot terrain-mode following.
sim/autopilot/hdg_alt_spd_on                       Autopilot maintain heading-alt-speed on.
sim/autopilot/hdg_alt_spd_off                      Autopilot maintain heading-alt-speed off.
sim/autopilot/airspeed_down                        Autopilot airspeed down.
sim/autopilot/airspeed_up                          Autopilot airspeed up.
sim/autopilot/airspeed_sync                        Autopilot airspeed sync.
sim/autopilot/heading_down                         Autopilot heading down.
sim/autopilot/heading_up                           Autopilot heading up.
sim/autopilot/heading_sync                         Autopilot heading sync.
sim/autopilot/vertical_speed_down                  Autopilot VVI down.
sim/autopilot/vertical_speed_up                    Autopilot VVI up.
sim/autopilot/vertical_speed_sync                  Autopilot VVI sync.
sim/autopilot/altitude_down                        Autopilot altitude down.
sim/autopilot/altitude_up                          Autopilot altitude up.
sim/autopilot/altitude_sync                        Autopilot altitude sync.
sim/autopilot/nose_down                            Autopilot nose down.
sim/autopilot/nose_up                              Autopilot nose up.
sim/autopilot/nose_down_pitch_mode                 Autopilot nose down, go into pitch mode.
sim/autopilot/nose_up_pitch_mode                   Autopilot nose up, go into pitch mode
sim/autopilot/override_left                        Autopilot override left: Go to ROL mode.
sim/autopilot/override_right                       Autopilot override right: Go to ROL mode.
sim/autopilot/override_up                          Autopilot override up: Go to SYN mode.
sim/autopilot/override_down                        Autopilot override down: Go to SYN mode.

    {"AP Disconnect", xplm_key_otto_dis},
    {"AP Heading", xplm_key_otto_hdg},
    {"AP NAV1", xplm_key_otto_nav1},
    {"AP Air Speed", xplm_key_otto_asi},
    {"AP Altitude", xplm_key_otto_alt},
    {"AP Vertical Speed", xplm_key_otto_vvi},
    {"AP Auto Throttle", xplm_key_otto_atr},

    {"Flaps Up", xplm_key_flapsup},
    {"Flaps Down", xplm_key_flapsdn},

    {"Elevator Trim Down", xplm_key_elvtrimD},
    {"Elevator Trim Up", xplm_key_elvtrimU},

XPLMDataRef battery_array  = NULL;
XPLMDataRef generator      = NULL;
XPLMDataRef avionics       = NULL;
XPLMDataRef fuel_pump      = NULL;
XPLMDataRef anti_ice       = NULL;
XPLMDataRef pitot_heat     = NULL;
XPLMDataRef cockpit_lights = NULL;
XPLMDataRef beacon_lights  = NULL;
XPLMDataRef nav_lights     = NULL;
XPLMDataRef strobe_lights  = NULL;
XPLMDataRef taxi_light     = NULL;
XPLMDataRef landing_lights = NULL;
XPLMDataRef ignition       = NULL;
XPLMDataRef igniters       = NULL;

battery_array  = XPLMFindDataRef("sim/cockpit/electrical/battery_array_on");
generator      = XPLMFindDataRef("sim/cockpit/electrical/generator_on");
avionics       = XPLMFindDataRef("sim/cockpit/electrical/avionics_on");
fuel_pump      = XPLMFindDataRef("sim/cockpit/engine/fuel_pump_on");
anti_ice       = XPLMFindDataRef("sim/cockpit/switches/anti_ice_on");
pitot_heat     = XPLMFindDataRef("sim/cockpit/switches/pitot_heat_on");
cockpit_lights = XPLMFindDataRef("sim/cockpit/electrical/cockpit_lights_on");
beacon_lights  = XPLMFindDataRef("sim/cockpit/electrical/beacon_lights_on");
nav_lights     = XPLMFindDataRef("sim/cockpit/electrical/nav_lights_on");
strobe_lights  = XPLMFindDataRef("sim/cockpit/electrical/strobe_lights_on");
taxi_light     = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
landing_lights = XPLMFindDataRef("sim/cockpit/electrical/landing_lights_on");
ignition       = XPLMFindDataRef("sim/cockpit/engine/ignition_on");
igniters       = XPLMFindDataRef("sim/cockpit/engine/igniters_on");

 XPLMCommandButtonRelease(xplm_joy_gear_down);

    alt_knob
    vs_knob
    ias_knob
    hdg_knob
    crs_knob

    ap_btn
    hdg_btn
    nav_btn
    ias_btn
    alt_btn
    vs_btn
    apr_btn
    rev_btn

    tuning_knob
    auto_throt
    flaps
    pitch_trim
*/


USING_PTYPES
using namespace std;

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel

enum {
    VENDOR_ID   = 0x060A,
    RP_PROD_ID  = 0x0D05,
    MP_PROD_ID  = 0x0D06,
    SP_PROD_ID  = 0x0D07
};

static const float RP_CB_INTERVAL = 0.5;
static const float MP_CB_INTERVAL = 0.5;
static const float SP_CB_INTERVAL = 0.5;

// XPLMSetDataf(gControlDataRef[Item], FloatValue[Item]);
// FloatValue[Item] = XPLMGetDataf(gControlDataRef[Item]);
// gInputDataRef[Item] = XPLMFindDataRef(InputDataRefDescriptions[Item]);
// int NumberOfEngines = XPLMGetDatai(gInputDataRef[1]);
/*
XPLMRegisterCommandHandler
XPLMSpeakString
void log(char *logString)
{
stringstream ss;
ss << ">>> " << logString << " tick " << t++ << endl;
XPLMDebugString(ss.str().c_str());
}

    {"AP Disconnect", xplm_key_otto_dis},
    {"AP Heading", xplm_key_otto_hdg},
    {"AP NAV1", xplm_key_otto_nav1},
    {"AP Air Speed", xplm_key_otto_asi},
    {"AP Altitude", xplm_key_otto_alt},
    {"AP Vertical Speed", xplm_key_otto_vvi},
    {"AP Auto Throttle", xplm_key_otto_atr},

    {"Flaps Up", xplm_key_flapsup},
    {"Flaps Down", xplm_key_flapsdn},

    {"Elevator Trim Down", xplm_key_elvtrimD},
    {"Elevator Trim Up", xplm_key_elvtrimU},


    {"AP GPS", xplm_key_otto_gps},
    {"AP Wing Leveler", xplm_key_otto_lev},
    {"AP HNAV", xplm_key_otto_hnav},
    {"AP VNAV", xplm_key_otto_vnav},
    {"AP NAV2", xplm_key_otto_nav2},
    {"TARG Down", xplm_key_targ_dn},
    {"TARG Up", xplm_key_targ_up},
    {"Heading Down", xplm_key_hdgdn},
    {"Heading Up", xplm_key_hdgup},
    {"Barometer Down", xplm_key_barodn},
    {"Barometer Up", xplm_key_baroup},
    {"OBS1 Down", xplm_key_obs1dn},
    {"OBS1 Up", xplm_key_obs1up},
    {"OBS2 Down", xplm_key_obs2dn},
    {"OBS2 Up", xplm_key_obs2up},
    {"COM1 1", xplm_key_com1_1},
    {"COM1 2", xplm_key_com1_2},
    {"COM1 3", xplm_key_com1_3},
    {"COM1 4", xplm_key_com1_4},
    {"NAV1 1", xplm_key_nav1_1},
    {"NAV1 2", xplm_key_nav1_2},
    {"NAV1 3", xplm_key_nav1_3},
    {"NAV1 4", xplm_key_nav1_4},
    {"COM2 1", xplm_key_com2_1},
    {"COM2 2", xplm_key_com2_2},
    {"COM2 3", xplm_key_com2_3},
    {"COM2 4", xplm_key_com2_4},
    {"NAV2 1", xplm_key_nav2_1},
    {"NAV2 2", xplm_key_nav2_2},
    {"NAV2 3", xplm_key_nav2_3},
    {"NAV2 4", xplm_key_nav2_4},
    {"ADF 1", xplm_key_adf_1},
    {"ADF 2", xplm_key_adf_2},
    {"ADF 3", xplm_key_adf_3},
    {"ADF 4", xplm_key_adf_4},
    {"ADF 5", xplm_key_adf_5},
    {"ADF 6", xplm_key_adf_6},
    {"XPDR 1", xplm_key_transpon_1},
    {"XPDR 2", xplm_key_transpon_2},
    {"XPDR 3", xplm_key_transpon_3},
    {"XPDR 4", xplm_key_transpon_4},
    {"XPDR 5", xplm_key_transpon_5},
    {"XPDR 6", xplm_key_transpon_6},
    {"XPDR 7", xplm_key_transpon_7},
    {"XPDR 8", xplm_key_transpon_8},
    {"Carb Heat Off", xplm_key_cheatoff},
    {"Carb Heat On", xplm_key_cheaton},
    {"Speed Brake Off", xplm_key_sbrkoff},
    {"Speed Brake On", xplm_key_sbrkon},
    {"Aileron Trim Left", xplm_key_ailtrimL},
    {"Aileron Trim Right", xplm_key_ailtrimR},
    {"Rudder Trim Left", xplm_key_rudtrimL},
    {"Rudder Trim Right", xplm_key_rudtrimR},

typedef struct _MESSAGE_STRUCT
{
    char MessageText[80];
    int MessageEnum;
} MESSAGE_STRUCT;

MESSAGE_STRUCT AutopilotStates[MAX_AP_STATES] =
{
    {"Auto Throttle", 1},
    {"Heading Hold", 2},
    {"Wing Leveler On", 4},
    {"Airspeed Hold", 8},
    {"VVI Climb", 16},
    {"Altitude Hold", 32},
    {"Level Change", 64},
    {"Auto Sync", 128},
    {"HNAV Armed", 256},
    {"HNAV Engaged", 512},
    {"VNAV Armed", 1024},
    {"VNAV Engaged", 2048}
};
    XPLMSetDatai(gAutopilotState, AutopilotStates[AutopilotStateLinePosition].MessageEnum);
    gAutopilotState = XPLMFindDataRef("sim/cockpit/autopilot/autopilot_state");

    XPLMCommandKeyStroke(KeyCommands[KeyCommandLinePosition].MessageEnum);
*/

// panel threads
RadioPanelThread*   gRp_thread = 0;
MultiPanelThread*   gMp_thread = 0;
SwitchPanelThread*  gSp_thread = 0;

// Radio Panel resources
hid_device*         gRpHandle;
jobqueue            gRp_ojq;
jobqueue            gRp_ijq;

// Multi Panel resources
hid_device*         gMpHandle;
jobqueue            gMp_ijq;
jobqueue            gMp_ojq;

// Switch Panel resources
hid_device*         gSpHandle;
jobqueue            gSp_ijq;
jobqueue            gSp_ojq;

// broadcast message trigger
trigger             gState(false, false);

/*
 * - register the plugin
 * - check for hid connected pro panels
 * - register callbacks
 * - start threads
 *
 */
PLUGIN_API int
XPluginStart(char* outName, char* outSig, char* outDesc) {

    strcpy(outName, "SaitekProPanelsPlugin");
    strcpy(outSig , "0xe2.0x9a.0x9b");
    strcpy(outDesc, "Saitek Pro Panels Plugin.");

    gRpHandle = hid_open(VENDOR_ID, RP_PROD_ID, NULL);
    gMpHandle = hid_open(VENDOR_ID, MP_PROD_ID, NULL);
    gSpHandle = hid_open(VENDOR_ID, SP_PROD_ID, NULL);

    if (gRpHandle) {
        XPLMRegisterFlightLoopCallback(rpSendMsg, RP_CB_INTERVAL, NULL);
        XPLMRegisterFlightLoopCallback(rpReceiveMsg, RP_CB_INTERVAL, NULL);
        gRp_thread = new RadioPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gState);

        gRp_thread->start();
    }

    if (gMpHandle) {
        XPLMRegisterFlightLoopCallback(mpSendMsg, MP_CB_INTERVAL, NULL);
        XPLMRegisterFlightLoopCallback(mpReceiveMsg, MP_CB_INTERVAL, NULL);
        gMp_thread = new MultiPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gState);

        gMp_thread->start();
    }

    if (gSpHandle) {
        XPLMRegisterFlightLoopCallback(spSendMsg, SP_CB_INTERVAL, NULL);
        XPLMRegisterFlightLoopCallback(spReceiveMsg, SP_CB_INTERVAL, NULL);
        gSp_thread = new SwitchPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gState);

        gSp_thread->start();
    }

    return 1;
}

/*
 *
 *
 */
float rpSendMsg(float inElapsedSinceLastCall,
                float inElapsedTimeSinceLastFlightLoop,
                int inCounter,
                void* inRefcon) {
pout.putf("Hello from rpSendMsg callback: %d\n", inCounter);

    // get data from xplane and pass it on


//    gRp_ojq.post(new myjob(alloc_buf));

    return 1.0;
}

/*
 *
 */
float rpReceiveMsg(float inElapsedSinceLastCall,
                   float inElapsedTimeSinceLastFlightLoop,
                   int inCounter,
                   void* inRefcon) {
pout.putf("Hello from rpReceiveMsg callback: %d\n", inCounter);

    message* msg = gRp_ijq.getmessage(MSG_NOWAIT);

    // get message from panel and set xplane data
    if (msg) {

//        u8_in_buf   = ((myjob*) msg)->buf;
        delete msg;
    }

    return 1.0;
}

/*
 *
 */
float mpSendMsg(float inElapsedSinceLastCall,
                float inElapsedTimeSinceLastFlightLoop,
                int inCounter,
                void* inRefcon) {
pout.putf("Hello from mpSendMsg callback: %d\n", inCounter);

    // get data from xplane and pass it on

//    gMp_ojq.post(new myjob(alloc_buf));

    return 1.0;
}

/*
 *
 */
float mpReceiveMsg(float inElapsedSinceLastCall,
                   float inElapsedTimeSinceLastFlightLoop,
                   int inCounter,
                   void* inRefcon) {
pout.putf("Hello from mpReceiveMsg callback: %d\n", inCounter);

    message* msg = gMp_ijq.getmessage(MSG_NOWAIT);

    // get message from panel and set xplane data
    if (msg) {

//        u8_in_buf   = ((myjob*) msg)->buf;

        delete msg;
    }

    return 1.0;
}

/*
 *
 */
float spSendMsg(float inElapsedSinceLastCall,
                float inElapsedTimeSinceLastFlightLoop,
                int inCounter,
                void* inRefcon) {
pout.putf("Hello from spSendMsg callback: %d\n", inCounter);

    // get data from xplane and pass it on

//    gSp_ojq.post(new myjob(alloc_buf));

    return 1.0;
}

/*
 *
 */
float spReceiveMsg(float inElapsedSinceLastCall,
                   float inElapsedTimeSinceLastFlightLoop,
                   int inCounter,
                   void* inRefcon) {
pout.putf("Hello from spReceiveMsg callback: %d\n", inCounter);

    message* msg = gSp_ijq.getmessage(MSG_NOWAIT);

    // get message from panel and set xplane data
    if (msg) {

//        u8_in_buf   = ((myjob*) msg)->buf;

        delete msg;
    }

    return 1.0;
}

/*
 *
 */
PLUGIN_API void
XPluginStop(void) {

    if (gRp_thread && gRp_thread->get_running()) {
        pexchange(&(gRp_thread->run), false);
        gRp_thread = 0;
    }

    if (gMp_thread && gMp_thread->get_running()) {
        pexchange(&(gMp_thread->run), false);
        gMp_thread = 0;
    }

    if (gSp_thread && gSp_thread->get_running()) {
        pexchange(&(gSp_thread->run), false);
        gSp_thread = 0;
    }

    if (gRpHandle) {
        XPLMUnregisterFlightLoopCallback(rpSendMsg, NULL);
        XPLMUnregisterFlightLoopCallback(rpReceiveMsg, NULL);
        hid_close(gRpHandle);
        gRpHandle = 0;
    }

    if (gMpHandle) {
        hid_close(gMpHandle);
        XPLMUnregisterFlightLoopCallback(mpSendMsg, NULL);
        XPLMUnregisterFlightLoopCallback(mpReceiveMsg, NULL);
        gMpHandle = 0;
    }

    if (gSpHandle) {
        hid_close(gSpHandle);
        XPLMUnregisterFlightLoopCallback(spSendMsg, NULL);
        XPLMUnregisterFlightLoopCallback(spReceiveMsg, NULL);
        gSpHandle = 0;
    }
}

/*
 *
 */
PLUGIN_API void
XPluginDisable(void) {

    gState.reset();

    if (gRp_thread) {
        pexchange(&(gRp_thread->pend), true);
    }

    if (gMp_thread) {
        pexchange(&(gMp_thread->pend), true);
    }

    if (gSp_thread) {
        pexchange(&(gSp_thread->pend), true);
    }
}

/*
 *
 */
PLUGIN_API int
XPluginEnable(void) {

    gState.signal();

    return 1;
}

PLUGIN_API void
XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void* inParam) {
#if 0
    switch(inMsg)
    {
        case xplm_key_pause:

            break;
        case xplm_key_revthrust:

            break;
        case xplm_key_jettison:

            break;
        case xplm_key_brakesreg:

            break;
        case xplm_key_brakesmax:

            break;
        case xplm_key_gear:

            break;
        case xplm_key_timedn:

            break;
        case xplm_key_timeup:

            break;
        default:

            break;
    }
#endif
}
