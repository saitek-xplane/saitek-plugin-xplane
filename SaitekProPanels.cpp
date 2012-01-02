// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#if defined(__WINDOWS__)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "pport.h"
#include "ptypes.h"
#include "pasync.h"
#include "ptime.h"
#include "pstreams.h"

#include "XPLMDefs.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

//#include "overloaded.h"
#include "defs.h"
#include "utils.h"
#include "hidapi.h"
#include "radiopanel.h"
#include "multipanel.h"
#include "switchpanel.h"
#include "PanelThreads.h"
#include "SaitekProPanels.h"


int RadioPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon);

int MultiPanelCommandHandler(XPLMCommandRef inCommand,
                             XPLMCommandPhase inPhase,
                             void* inRefcon);

int SwitchPanelCommandHandler(XPLMCommandRef    inCommand,
                              XPLMCommandPhase  inPhase,
                              void*             inRefcon);

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

//logfile* gLogFile;
//char gLogFilePath[512] = {};

/*
sim/flightmodel2/controls/flap_handle_deploy_ratio
sim/aircraft/specialcontrols/acf_ail1flaps
*/

USING_PTYPES

enum {
    CMD_EAT_EVENT,
    CMD_PASS_EVENT,
    CMD_OTTO_AUTOTHROTTLE_ON,
    CMD_OTTO_AUTOTHROTTLE_OFF,
    CMD_SYS_AVIONICS_ON,
    CMD_SYS_AVIONICS_OFF,
    CMD_ELEC_BATTERY1_ON,
    CMD_ELEC_BATTERY1_OFF,
    CMD_FLTCTL_FLAPS_UP,
    CMD_FLTCTL_FLAPS_DOWN,
    CMD_FLTCTL_PITCHTRIM_UP,
    CMD_FLTCTL_PITCHTRIM_DOWN,
    CMD_FLTCTL_PITCHTRIM_TAKEOFF,
    CMD_OTTO_ON,
    CMD_OTTO_OFF,
    CMD_OTTO_ARMED,
    CMD_OTTO_HDG_BTN,
    CMD_OTTO_NAV_BTN,
    CMD_OTTO_IAS_BTN,
    CMD_OTTO_ALT_BTN,
    CMD_OTTO_VS_BTN,
    CMD_OTTO_APR_BTN,
    CMD_OTTO_REV_BTN,
    CMD_OTTO_ALT_UP,
    CMD_OTTO_ALT_DN,
    CMD_OTTO_VS_UP,
    CMD_OTTO_VS_DN,
    CMD_OTTO_IAS_UP,
    CMD_OTTO_IAS_DN,
    CMD_OTTO_HDG_UP,
    CMD_OTTO_HDG_DN,
    CMD_OTTO_CRS_UP,
    CMD_OTTO_CRS_DN,
};

// Switch panel
enum {
    CMD_MAGNETOS_OFF,
    CMD_MAGNETOS_RIGHT,
    CMD_MAGNETOS_LEFT,
    CMD_MAGNETOS_BOTH,
    CMD_MAGNETOS_START,
    CMD_MASTER_BATTERY_ON,
    CMD_MASTER_BATTERY_OFF,
    CMD_MASTER_ALT_BATTERY_ON,
    CMD_MASTER_ALT_BATTERY_OFF,
    CMD_MASTER_AVIONICS_ON,
    CMD_MASTER_AVIONICS_OFF,
    CMD_FUEL_PUMP_ON,
    CMD_FUEL_PUMP_OFF,
    CMD_DEICE_ON,
    CMD_DEICE_OFF,
    CMD_PITOT_HEAT_ON,
    CMD_PITOT_HEAT_OFF,
    CMD_COWL_CLOSED,
    CMD_COWL_OPEN,
    CMD_LIGHTS_PANEL_ON,
    CMD_LIGHTS_PANEL_OFF,
    CMD_LIGHTS_BEACON_ON,
    CMD_LIGHTS_BEACON_OFF,
    CMD_LIGHTS_NAV_ON,
    CMD_LIGHTS_NAV_OFF,
    CMD_LIGHTS_STROBE_ON,
    CMD_LIGHTS_STROBE_OFF,
    CMD_LIGHTS_TAXI_ON,
    CMD_LIGHTS_TAXI_OFF,
    CMD_LIGHTS_LANDING_ON,
    CMD_LIGHTS_LANDING_OFF,
    CMD_GEAR_UP,
    CMD_GEAR_DOWN
};

// Flightloop callback message queue processing count defaults.
// TODO: create a small user menu with a user adjustment slider (range ?)
enum {
    RP_MSGPROC_CNT = 50,
    MP_MSGPROC_CNT = 50,
    SP_MSGPROC_CNT = 50
};

// Flightloop callback message queue processing globals.
// These should be adjustable by the user via a menu item.
// For X-Plane v9.x, and possibly v10, a higher count means
// we interrupt X-Plane proper and a lower count increases
// panel input/output latency.
int gRp_MsgProc_Cnt = RP_MSGPROC_CNT;
int gMp_MsgProc_Cnt = MP_MSGPROC_CNT;
int gSp_MsgProc_Cnt = SP_MSGPROC_CNT;

int gAvPwrOn = false;
int gBat1On = false;

bool gPowerUp = true;
bool gEnabled = false;
unsigned int gFlCbCnt = 0;

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel
// cb = Cb = CB = Callback
// Flightloop Callback INterval
static const float FL_CB_INTERVAL = -1.0;

uint32_t gMpBtnEvtPending = 0;

XPLMDataRef gAvPwrOnDataRef = NULL;
XPLMDataRef gBatPwrOnDataRef = NULL;

/* Command Refs */
XPLMCommandRef gAvPwrOnCmdRef = NULL;
XPLMCommandRef gAvPwrOffCmdRef = NULL;
XPLMCommandRef gBatPwrOnCmdRef = NULL;
XPLMCommandRef gBatPwrOffCmdRef = NULL;

XPLMCommandRef gMpAsDnCmdRef = NULL;
XPLMCommandRef gMpAsUpCmdRef = NULL;
XPLMCommandRef gMpAltDnCmdRef = NULL;
XPLMCommandRef gMpAltUpCmdRef = NULL;
XPLMCommandRef gMpAltHoldCmdRef = NULL;
XPLMCommandRef gMpAppCmdRef = NULL;
XPLMCommandRef gMpAtThrrtlOnCmdRef = NULL;
XPLMCommandRef gMpAtThrrtlOffCmdRef = NULL;
XPLMCommandRef gMpAtThrrtlTgglCmdRef = NULL;
XPLMCommandRef gMpBkCrsCmdRef = NULL;
XPLMCommandRef gMpFlpsDnCmdRef = NULL;
XPLMCommandRef gMpFlpsUpCmdRef = NULL;
XPLMCommandRef gMpHdgCmdRef = NULL;
XPLMCommandRef gMpHdgDnCmdRef = NULL;
XPLMCommandRef gMpHdgUpCmdRef = NULL;
XPLMCommandRef gMpLvlChngCmdRef = NULL;
XPLMCommandRef gMpNavCmdRef = NULL;
XPLMCommandRef gMpObsHsiDnCmdRef = NULL;
XPLMCommandRef gMpObsHsiUpCmdRef = NULL;
XPLMCommandRef gMpPtchTrmDnCmdRef = NULL;
XPLMCommandRef gMpPtchTrmUpCmdRef = NULL;
XPLMCommandRef gMpPtchTrmTkOffCmdRef = NULL;
XPLMCommandRef gMpApOnCmdRef = NULL;
XPLMCommandRef gMpApOffCmdRef = NULL;
XPLMCommandRef gMpApArmedCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdDnCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdUpCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdCmdRef = NULL;

/* Data Refs */
XPLMDataRef gMpOttoOvrrde = NULL;
XPLMDataRef gMpArspdDataRef = NULL;
XPLMDataRef gMpAltDataRef = NULL;
XPLMDataRef gMpAltHoldStatDataRef = NULL;
XPLMDataRef gMpApprchStatDataRef = NULL;
XPLMDataRef gMpApStateDataRef = NULL;
XPLMDataRef gMpBckCrsStatDataRef = NULL;
XPLMDataRef gMpFlghtDirModeDataRef = NULL;
XPLMDataRef gMpHdgMagDataRef = NULL;
XPLMDataRef gMpHdgStatDataRef = NULL;
XPLMDataRef gMpHsiObsDegMagPltDataRef = NULL;
XPLMDataRef gMpNavStatDataRef = NULL;
XPLMDataRef gMpSpdStatDataRef = NULL;
XPLMDataRef gMpVrtVelDataRef = NULL;
XPLMDataRef gMpVviStatDataRef = NULL;

/* SWITCH PANEL Command Refs */
XPLMCommandRef gSpMagnetosOffCmdRef = NULL;
XPLMCommandRef gSpMagnetosRightCmdRef = NULL;
XPLMCommandRef gSpMagnetosLeftCmdRef = NULL;
XPLMCommandRef gSpMagnetosBothCmdRef = NULL;
XPLMCommandRef gSpMagnetosStartCmdRef = NULL;
XPLMCommandRef gSpMasterBatteryOnCmdRef = NULL;
XPLMCommandRef gSpMasterBatteryOffCmdRef = NULL;
XPLMCommandRef gSpMasterAltBatteryOnCmdRef = NULL;
XPLMCommandRef gSpMasterAltBatteryOffCmdRef = NULL;
XPLMCommandRef gSpMasterAvionicsOnCmdRef = NULL;
XPLMCommandRef gSpMasterAvionicsOffCmdRef = NULL;
XPLMCommandRef gSpFuelPumpOnCmdRef = NULL;
XPLMCommandRef gSpFuelPumpOffCmdRef = NULL;
XPLMCommandRef gSpDeIceOnCmdRef = NULL;
XPLMCommandRef gSpDeIceOffCmdRef = NULL;
XPLMCommandRef gSpPitotHeatOnCmdRef = NULL;
XPLMCommandRef gSpPitotHeatOffCmdRef = NULL;
XPLMCommandRef gSpCowlClosedCmdRef = NULL;
XPLMCommandRef gSpCowlOpenCmdRef = NULL;
XPLMCommandRef gSpLightsPanelOnCmdRef = NULL;
XPLMCommandRef gSpLightsPanelOffCmdRef = NULL;
XPLMCommandRef gSpLightsBeaconOnCmdRef = NULL;
XPLMCommandRef gSpLightsBeaconOffCmdRef = NULL;
XPLMCommandRef gSpLightsNavOnCmdRef = NULL;
XPLMCommandRef gSpLightsNavOffCmdRef = NULL;
XPLMCommandRef gSpLightsStrobeOnCmdRef = NULL;
XPLMCommandRef gSpLightsStrobeOffCmdRef = NULL;
XPLMCommandRef gSpLightsTaxiOnCmdRef = NULL;
XPLMCommandRef gSpLightsTaxiOffCmdRef = NULL;
XPLMCommandRef gSpLightsLandingOnCmdRef = NULL;
XPLMCommandRef gSpLightsLandingOffCmdRef = NULL;
XPLMCommandRef gSpLandingGearUpCmdRef = NULL;
XPLMCommandRef gSpLandingGearDownCmdRef = NULL;

/* SWITCH PANEL Data Refs */
XPLMDataRef gSpMagnetosOffDataRef = NULL;
XPLMDataRef gSpMagnetosRightDataRef = NULL;
XPLMDataRef gSpMagnetosLeftDataRef = NULL;
XPLMDataRef gSpMagnetosBothDataRef = NULL;
XPLMDataRef gSpMagnetosStartDataRef = NULL;
XPLMDataRef gSpMasterBatteryOnDataRef = NULL;
XPLMDataRef gSpMasterBatteryOffDataRef = NULL;
XPLMDataRef gSpMasterAltBatteryOnDataRef = NULL;
XPLMDataRef gSpMasterAltBatteryOffDataRef = NULL;
XPLMDataRef gSpMasterAvionicsOnDataRef = NULL;
XPLMDataRef gSpMasterAvionicsOffDataRef = NULL;
XPLMDataRef gSpFuelPumpOnDataRef = NULL;
XPLMDataRef gSpFuelPumpOffDataRef = NULL;
XPLMDataRef gSpDeIceOnDataRef = NULL;
XPLMDataRef gSpDeIceOffDataRef = NULL;
XPLMDataRef gSpPitotHeatOnDataRef = NULL;
XPLMDataRef gSpPitotHeatOffDataRef = NULL;
XPLMDataRef gSpCowlClosedDataRef = NULL;
XPLMDataRef gSpCowlOpenDataRef = NULL;
XPLMDataRef gSpLightsBeaconOffDataRef = NULL;
XPLMDataRef gSpLightsPanelOffDataRef = NULL;
XPLMDataRef gSpLightsBeaconOnDataRef = NULL;
XPLMDataRef gSpLightsNavOnOnDataRef = NULL;
XPLMDataRef gSpLightsNavOffDataRef = NULL;
XPLMDataRef gSpLightsStrobeOnDataRef = NULL;
XPLMDataRef gSpLightsStrobeOffDataRef = NULL;
XPLMDataRef gSpLightsTaxiOnDataRef = NULL;
XPLMDataRef gSpLightsTaxiOffDataRef = NULL;
XPLMDataRef gSpLightsLandingOnDataRef = NULL;
XPLMDataRef gSpLightsLandingOffDataRef = NULL;
XPLMDataRef gSpLandingGearUpDataRef = NULL;
XPLMDataRef gSpLandingGearDownDataRef = NULL;



/*
sim/systems/avionics_on                            Avionics on.
sim/systems/avionics_off                           Avionics off.

sim/autopilot/hsi_select_nav_1                     HSI shows nav 1.
sim/autopilot/hsi_select_nav_2                     HSI shows nav 2.
sim/autopilot/hsi_select_gps                       HSI shows GPS.
sim/autopilot/flight_dir_on_only                   Flight directory only.
sim/autopilot/servos_and_flight_dir_on             Servos on.

sim/autopilot/fdir_servos_down_one                 Autopilot Mode down (on->fdir->off).
sim/autopilot/fdir_servos_up_one                   Autopilot Mode up (off->fdir->on).
sim/autopilot/fdir_servos_toggle                   Toggle between on and off.
sim/autopilot/control_wheel_steer                  Control-wheel steer.

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
sim/autopilot/hdg_alt_spd_on                       Autopilot maintain heading-alt-speed on.
sim/autopilot/hdg_alt_spd_off                      Autopilot maintain heading-alt-speed off.
sim/autopilot/airspeed_down                        Autopilot airspeed down.
sim/autopilot/airspeed_up                          Autopilot airspeed up.
sim/autopilot/airspeed_sync                        Autopilot airspeed sync.
sim/autopilot/heading_down                         Autopilot heading down.
sim/autopilot/heading_up                           Autopilot heading up.
sim/autopilot/vertical_speed_down                  Autopilot VVI down.
sim/autopilot/vertical_speed_up                    Autopilot VVI up.
sim/autopilot/altitude_down                        Autopilot altitude down.
sim/autopilot/altitude_up                          Autopilot altitude up.
sim/autopilot/altitude_sync                        Autopilot altitude sync.
*/

#define sAVIONICS_POWER_ON          "sim/cockpit2/switches/avionics_power_on"
#define sBATTERY_ON                 "sim/cockpit/electrical/battery_on"
#define sALTITUDE_DOWN              "sim/autopilot/altitude_down"
#define sALTITUDE_UP                "sim/autopilot/altitude_up"
#define sVERTICAL_SPEED_DOWN        "sim/autopilot/vertical_speed_down"
#define sVERTICAL_SPEED_UP          "sim/autopilot/vertical_speed_up"
#define sAIRSPEED_DOWN              "sim/autopilot/airspeed_down"
#define sAIRSPEED_UP                "sim/autopilot/airspeed_up"
#define sHEADING_DOWN               "sim/autopilot/heading_down"
#define sHEADING_UP                 "sim/autopilot/heading_up"
#define sOBS_HSI_DOWN               "sim/radios/obs_HSI_down"
#define sOBS_HSI_UP                 "sim/radios/obs_HSI_up"
#define sFLIGHT_DIR_ON_ONLY         "sim/autopilot/flight_dir_on_only"
#define sSERVOS_AND_FLIGHT_DIR_ON   "sim/autopilot/servos_and_flight_dir_on "
#define sSERVOS_AND_FLIGHT_DIR_OFF  "sim/autopilot/servos_and_flight_dir_off"
#define sHEADING                    "sim/autopilot/heading"
#define sNAV                        "sim/autopilot/NAV"
#define sLEVEL_CHANGE               "sim/autopilot/level_change"
#define sALTITUDE_HOLD              "sim/autopilot/altitude_hold"
#define sALTITUDE_ARM               "sim/autopilot/altitude_arm"
#define sVERTICAL_SPEED             "sim/autopilot/vertical_speed"
#define sAPPROACH                   "sim/autopilot/approach"
#define sBACK_COURSE                "sim/autopilot/back_course"
#define sAUTOTHROTTLE_ON            "sim/autopilot/autothrottle_on"
#define sAUTOTHROTTLE_OFF           "sim/autopilot/autothrottle_off"
#define sAUTOTHROTTLE_TOGGLE        "sim/autopilot/autothrottle_toggle"
#define sFLAPS_DOWN                 "sim/flight_controls/flaps_down"
#define sFLAPS_UP                   "sim/flight_controls/flaps_up"
#define sPITCH_TRIM_DOWN            "sim/flight_controls/pitch_trim_down"
#define sPITCH_TRIM_UP              "sim/flight_controls/pitch_trim_up"
#define sPITCH_TRIM_TAKEOFF         "sim/flight_controls/pitch_trim_takeoff"
#define sFLIGHT_DIRECTOR_MODE       "sim/cockpit2/autopilot/flight_director_mode"
#define sALTITUDE_DIAL_FT           "sim/cockpit2/autopilot/altitude_dial_ft"
#define sALTITUDE_HOLD_FT           "sim/cockpit2/autopilot/altitude_hold_ft"
#define sVVI_DIAL_FPM               "sim/cockpit2/autopilot/vvi_dial_fpm"
#define sAIRSPEED_DIAL_KTS_MACH     "sim/cockpit2/autopilot/airspeed_dial_kts_mach"
#define sHEADING_DIAL_DEG_MAG_PILOT "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot"
#define sHSI_OBS_DEG_MAG_PILOT      "sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot"
#define sAIRSPEED                   "sim/cockpit/autopilot/airspeed"
#define sVERTICAL_VELOCITY          "sim/cockpit/autopilot/vertical_velocity"
#define sALTITUDE                   "sim/cockpit/autopilot/altitude"
#define sHEADING_MAG                "sim/cockpit/autopilot/heading_mag"
#define sALTITUDE_HOLD_STATUS       "sim/cockpit2/autopilot/altitude_hold_status"
#define sAPPROACH_STATUS            "sim/cockpit2/autopilot/approach_status"
#define sAUTOPILOT_STATE            "sim/cockpit/autopilot/autopilot_state"
#define sBACKCOURSE_STATUS          "sim/cockpit2/autopilot/backcourse_status"
#define sHEADING_STATUS             "sim/cockpit2/autopilot/heading_status"
#define sNAV_STATUS                 "sim/cockpit2/autopilot/nav_status"
#define sSPEED_STATUS               "sim/cockpit2/autopilot/speed_status"
#define sVVI_STATUS                 "sim/cockpit2/autopilot/vvi_status"
/* SWITCH PANEL */
#define sMAGNETOS_OFF               "sim/magnetos/magnetos_off"
#define sMAGNETOS_RIGHT             "sim/magnetos/magnetos_right_1"
#define sMAGNETOS_LEFT              "sim/magnetos/magnetos_left_1"
#define sMAGNETOS_BOTH              "sim/magnetos/magnetos_both"
#define sMAGNETOS_START             "sim/starters/engage_start_run"

#define sMASTER_BATTERY_ON          "sim/electrical/battery_1_on"
#define sMASTER_BATTERY_OFF         "sim/electrical/battery_1_off"
#define sMASTER_ALT_BATTERY_ON      "sim/electrical/battery_2_on"
#define sMASTER_ALT_BATTERY_OFF     "sim/electrical/battery_2_off"

#define sMASTER_AVIONICS_ON         "sim/systems/avionics_on"
#define sMASTER_AVIONICS_OFF        "sim/systems/avionics_off"
#define sFUEL_PUMP_ON               "sim/fuel/fuel_pump_1_on"
#define sFUEL_PUMP_OFF              "sim/fuel/fuel_pump_1_off"
#define sDE_ICE_LW_ON               "sim/ice/wing_heat0_on"
#define sDE_ICE_LW_OFF              "sim/ice/wing_heat0_off"
#define sDE_ICE_RW_ON               "sim/ice/wing_heat1_on"
#define sDE_ICE_RW_OFF              "sim/ice/wing_heat1_off"
#define sPITOT_HEAT_ON              "sim/ice/pitot_heat_on"
#define sPITOT_HEAT_OFF             "sim/ice/pitot_heat_off"

#define sCOWL_CLOSED                "sim/flight_controls/cowl_flaps_closed"
#define sCOWL_OPEN                  "sim/flight_controls/cowl_flaps_open"

#define sLIGHTS_PANEL_ON            "sim/instruments/panel_bright_down"
#define sLIGHTS_PANEL_OFF           "sim/instruments/panel_bright_up"
#define sLIGHTS_BEACON_ON           "sim/lights/beacon_lights_on"
#define sLIGHTS_BEACON_OFF          "sim/lights/beacon_lights_off"
#define sLIGHTS_NAV_ON              "sim/lights/nav_lights_on"
#define sLIGHTS_NAV_OFF             "sim/lights/nav_lights_off"
#define sLIGHTS_STROBE_ON           "sim/lights/strobe_lights_on"
#define sLIGHTS_STROBE_OFF          "sim/lights/strobe_lights_off"
#define sLIGHTS_TAXI_ON             "sim/lights/taxi_lights_on"
#define sLIGHTS_TAXI_OFF            "sim/lights/taxi_lights_off"
#define sLIGHTS_LANDING_ON          "sim/lights/landing_lights_on"
#define sLIGHTS_LANDING_OFF         "sim/lights/landing_lights_off"

#define sLANDING_GEAR_UP            "sim/flight_controls/landing_gear_down"
#define sLANDING_GEAR_DOWN          "sim/flight_controls/landing_gear_up"


/*
 * - register the plugin
 * - check for hid connected panels
 * - register callbacks
 * - start threads
 *
 */
PLUGIN_API int
XPluginStart(char* outName, char* outSig, char* outDesc) {
    XPLMCommandRef cmd_ref;

    LPRINTF("Saitek ProPanels Plugin: XPluginStart\n");
    strcpy(outName, "SaitekProPanels");
    strcpy(outSig , "jdp.panels.saitek");
    strcpy(outDesc, "Saitek Pro Panels Plugin.");

    uint32_t* x = new uint32_t;
    *x = MP_BLANK_SCRN;
    gMp_ojq.post(new myjob(x));

    gAvPwrOnDataRef = XPLMFindDataRef(sAVIONICS_POWER_ON);
    // XXX: no switches data ref for battery?!
    gBatPwrOnDataRef = XPLMFindDataRef(sBATTERY_ON);

// TODO: figure out why gAvPwrOnDataRef & gBatPwrOnDataRef are always off
    // check if power is on, gAvPwrOn default is off
//    if (XPLMGetDatai(gAvPwrOnDataRef)) {
        gAvPwrOn = true;
        x = new uint32_t;
        *x = gAvPwrOn;
        gMp_ojq.post(new myjob(x));
        pexchange((int*)&gAvPwrOn, true);
//    }

    // check if the battery is on, gBat1On default is off
//    if (XPLMGetDatai(gBatPwrOnDataRef)) {
        gBat1On = true;
        x = new uint32_t;
        *x = gBat1On;
        gMp_ojq.post(new myjob(x));
        pexchange((int*)&gBat1On, true);
//    }

    // A questionable way to use the (hideous) preprocessor
    // but it makes it easier to work with this file.
    #include "multipanel_refs.cpp"
    #include "switchpanel_refs.cpp"
    #include "radiopanel_refs.cpp"

    LPRINTF("Saitek ProPanels Plugin: commands initialized\n");

    if (init_hid(&gRpHandle, RP_PROD_ID))
        rp_init(gRpHandle);

    if (init_hid(&gMpHandle, MP_PROD_ID))
        mp_init(gMpHandle);

    if (init_hid(&gSpHandle, SP_PROD_ID))
        sp_init(gSpHandle);

    pexchange((int*)&threads_run, true);

    ToPanelThread*     tp;
    FromPanelThread*   fp;

    // radio panel
    tp = new ToPanelThread(gRpHandle, &gRp_ojq, &gRpTrigger, RP_PROD_ID);
    fp = new FromPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gRpTrigger, RP_PROD_ID);

    tp->start();
    fp->start();

    // multi panel
    tp = new ToPanelThread(gMpHandle, &gMp_ojq, &gMpTrigger, MP_PROD_ID);
    fp = new FromPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gMpTrigger, MP_PROD_ID);

    tp->start();
    fp->start();

    // switch panel
    tp = new ToPanelThread(gSpHandle, &gSp_ojq, &gSpTrigger, SP_PROD_ID);
    fp = new FromPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gSpTrigger, SP_PROD_ID);

    tp->start();
    fp->start();

#ifdef DO_USBPANEL_CHECK
    pexchange((int*)&pc_run, true);
    PanelsCheckThread* pc = new PanelsCheckThread();
    pc->start();
#endif

    if (gRpHandle) { LPRINTF("Saitek ProPanels Plugin: gRpHandle\n"); gRpTrigger.post(); }
    if (gMpHandle) { /*XPLMSetDatai(gMpOttoOvrrde, true);*/ LPRINTF("Saitek ProPanels Plugin: gMpHandle\n"); gMpTrigger.post(); }
    if (gSpHandle) { LPRINTF("Saitek ProPanels Plugin: gSpHandle\n"); gSpTrigger.post(); }

    LPRINTF("Saitek ProPanels Plugin: Panel threads running\n");

    XPLMRegisterFlightLoopCallback(RadioPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(MultiPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(SwitchPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);

//XPLM_API void XPLMSetErrorCallback(XPLMError_f inCallback);
//XPLM_API void XPLMGetVersions(int* outXPlaneVersion, int* outXPLMVersion, XPLMHostApplicationID* outHostID);

    LPRINTF("Saitek ProPanels Plugin: startup completed\n");

    return 1;
}


/*
 *
 *
 */
int MultiPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    uint32_t* m;
    uint32_t x;
    float f;
    int status = CMD_PASS_EVENT;

// TODO: check/set item state for some events!?
    switch (reinterpret_cast<uint32_t>(inRefcon)) {
    case CMD_OTTO_AUTOTHROTTLE_ON:
        status = CMD_EAT_EVENT;
        break;
    case CMD_OTTO_AUTOTHROTTLE_OFF:
        status = CMD_EAT_EVENT;
        break;
    case CMD_SYS_AVIONICS_ON:
        pexchange((int*)&gAvPwrOn, true);
        m = new uint32_t;
        *m = AVIONICS_ON;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_SYS_AVIONICS_OFF:
        pexchange((int*)&gAvPwrOn, false);
        m = new uint32_t;
        *m = AVIONICS_OFF;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_ELEC_BATTERY1_ON:
        pexchange((int*)&gBat1On, true);
        m = new uint32_t;
        *m = BAT1_ON;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_ELEC_BATTERY1_OFF:
        pexchange((int*)&gBat1On, false);
        m = new uint32_t;
        *m = BAT1_OFF;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_ON:
        m = new uint32_t;
        *m = BTN_AP_ON;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_OFF:
        m = new uint32_t;
        *m = BTN_AP_OFF;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_ARMED:
        m = new uint32_t;
        *m = BTN_AP_ARMED;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_ALT_UP:
    case CMD_OTTO_ALT_DN:
        m = new uint32_t[MPM_CNT];
        m[0] = MPM;
        m[1] = ALT_VAL;
        m[2] = (uint32_t)XPLMGetDataf(gMpAltDataRef);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_VS_UP:
    case CMD_OTTO_VS_DN:
        m = new uint32_t[MPM_CNT];
        f = XPLMGetDataf(gMpVrtVelDataRef);
        m[1] = (f < 0) ? VS_VAL_NEG : VS_VAL_POS;
        m[0] = MPM;
        m[2] = (uint32_t) fabs(f);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_IAS_UP:
    case CMD_OTTO_IAS_DN:
        m = new uint32_t[MPM_CNT];
        m[0] = MPM; m[1] = IAS_VAL;
        m[2] = (uint32_t)XPLMGetDataf(gMpArspdDataRef);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_HDG_UP:
    case CMD_OTTO_HDG_DN:
        m = new uint32_t[MPM_CNT];
        m[0] = MPM;
        m[1] = HDG_VAL;
        m[2] = (uint32_t)XPLMGetDataf(gMpHdgMagDataRef);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_CRS_UP:
    case CMD_OTTO_CRS_DN:
        m = new uint32_t[MPM_CNT];
        m[0] = MPM;
        m[1] = CRS_VAL;
        m[2] = (uint32_t)XPLMGetDataf(gMpHsiObsDegMagPltDataRef);
        gMp_ojq.post(new myjob(m));
        break;
   case CMD_OTTO_HDG_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpHdgStatDataRef);
        *m = (x == 0) ? BTN_HDG_OFF : ((x == 2) ? BTN_HDG_CAPT : BTN_HDG_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_NAV_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpNavStatDataRef);
        *m = (x == 0) ? BTN_NAV_OFF : ((x == 2) ? BTN_NAV_CAPT : BTN_NAV_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_IAS_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpSpdStatDataRef);
        *m = (x == 0) ? BTN_IAS_OFF : ((x == 2) ? BTN_IAS_CAPT : BTN_IAS_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_ALT_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpAltHoldStatDataRef);
        *m = (x == 0) ? BTN_ALT_OFF : ((x == 2) ? BTN_ALT_CAPT : BTN_ALT_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_VS_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpVviStatDataRef);
        *m = (x == 0) ? BTN_VS_OFF : ((x == 2) ? BTN_VS_CAPT : BTN_VS_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_APR_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpApprchStatDataRef);
        *m = (x == 0) ? BTN_APR_OFF : ((x == 2) ? BTN_APR_CAPT : BTN_APR_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_OTTO_REV_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpBckCrsStatDataRef);
        *m = (x == 0) ? BTN_REV_OFF : ((x == 2) ? BTN_REV_CAPT : BTN_REV_ARMED);
        gMp_ojq.post(new myjob(m));
        break;
    default:
        break;
    }

    return status;
}


/*
 *
 *
 */
int RadioPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    int status = CMD_PASS_EVENT;

    return status;
}


/*
 *
 *
 */
int SwitchPanelCommandHandler(XPLMCommandRef   inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    uint32_t* m;
    uint32_t x;
    float f;
    int status = CMD_PASS_EVENT;
    LPRINTF("Saitek ProPanels Plugin: switch panel lights landing on\n");


    switch (reinterpret_cast<uint32_t>(inRefcon)) {
    case CMD_MAGNETOS_OFF:
        m = new uint32_t;
        *m = SP_MAGNETOS_OFF;
        //
        break;
    case CMD_MAGNETOS_RIGHT:
        m = new uint32_t;
        *m = SP_MAGNETOS_RIGHT;
        //
        break;
    case CMD_MAGNETOS_LEFT:
        m = new uint32_t;
        *m = SP_MAGNETOS_LEFT;
        //
        break;
    case CMD_MAGNETOS_BOTH:
        m = new uint32_t;
        *m = SP_MAGNETOS_BOTH;
        //
        break;
    case CMD_MAGNETOS_START:
        m = new uint32_t;
        *m = SP_MAGNETOS_START;
        //
        break;
    case CMD_MASTER_BATTERY_ON:
        m = new uint32_t;
        *m = SP_MASTER_BATTERY_ON;
        //
        break;
    case CMD_MASTER_BATTERY_OFF:
        m = new uint32_t;
        *m = SP_MASTER_BATTERY_OFF;
        //
        break;
    case CMD_MASTER_ALT_BATTERY_ON:
        m = new uint32_t;
        *m = SP_MASTER_ALT_BATTERY_ON;
        //
        break;
    case CMD_MASTER_ALT_BATTERY_OFF:
        m = new uint32_t;
        *m = SP_MASTER_ALT_BATTERY_OFF;
        //
        break;
    case CMD_MASTER_AVIONICS_ON:
        m = new uint32_t;
        *m = SP_MASTER_AVIONICS_ON;
        //
        break;
    case CMD_MASTER_AVIONICS_OFF:
        m = new uint32_t;
        *m = SP_MASTER_AVIONICS_OFF;
        //
        break;
    case CMD_FUEL_PUMP_ON:
        m = new uint32_t;
        *m = SP_FUEL_PUMP_ON;
        //
        break;
    case CMD_FUEL_PUMP_OFF:
        m = new uint32_t;
        *m = SP_FUEL_PUMP_OFF;
        //
        break;
    case CMD_DEICE_ON:
        m = new uint32_t;
        *m = SP_DEICE_LW_ON;
        //
        break;
    case CMD_DEICE_OFF:
        m = new uint32_t;
        *m = SP_DEICE_LW_OFF;
        //
        break;
    case CMD_PITOT_HEAT_ON:
        m = new uint32_t;
        *m = SP_PITOT_HEAT_ON;
        //
        break;
    case CMD_PITOT_HEAT_OFF:
        m = new uint32_t;
        *m = SP_PITOT_HEAT_OFF;
        //
        break;
    case CMD_COWL_CLOSED:
        m = new uint32_t;
        *m = SP_COWL_CLOSED;
        //
        break;
    case CMD_COWL_OPEN:
        m = new uint32_t;
        *m = SP_COWL_OPEN;
        //
        break;
    case CMD_LIGHTS_PANEL_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_PANEL_ON;
        //
        break;
    case CMD_LIGHTS_PANEL_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_PANEL_OFF;
        //
        break;
    case CMD_LIGHTS_BEACON_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_BEACON_ON;
        //
        break;
    case CMD_LIGHTS_BEACON_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_BEACON_OFF;
        //
        break;
    case CMD_LIGHTS_NAV_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_NAV_ON;
        //
        break;
    case CMD_LIGHTS_NAV_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_NAV_OFF;
        //
        break;
    case CMD_LIGHTS_STROBE_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_STROBE_ON;
        //
        break;
    case CMD_LIGHTS_STROBE_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_STROBE_OFF;
        //
        break;
    case CMD_LIGHTS_TAXI_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_TAXI_ON;
        //
        break;
    case CMD_LIGHTS_TAXI_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_TAXI_OFF;
        //
        break;
    case CMD_LIGHTS_LANDING_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_LANDING_ON;
        //
        break;
    case CMD_LIGHTS_LANDING_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_LANDING_OFF;
        //
        break;
    case CMD_GEAR_UP:
        m = new uint32_t;
        *m = SP_LANDING_GEAR_UP;
        //
        break;
    case CMD_GEAR_DOWN:
        m = new uint32_t;
        *m = SP_LANDING_GEAR_DOWN;
        //
        break;
    default:
        break;
    }

    return status;
}

/*
 *
 *
 */
float RadioPanelFlightLoopCallback(float   inElapsedSinceLastCall,
                                   float   inElapsedTimeSinceLastFlightLoop,
                                   int     inCounter,
                                   void*   inRefcon) {
// #ifndef NDEBUG
//     static char tmp[100];
// #endif

    uint32_t x;
    int msg_cnt = gRp_MsgProc_Cnt;

//    if ((gFlCbCnt % PANEL_CHECK_INTERVAL) == 0) {
//        if (gEnabled) {
//            gPcTrigger.post();
//        }
//    }

    while (msg_cnt--) {
        message* msg = gRp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {
        } // if (msg)

        delete msg;
    } // while

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
// #ifndef NDEBUG
//     static char tmp[100];
// #endif

    uint32_t x;
    int msg_cnt = gMp_MsgProc_Cnt;

//    if ((gFlCbCnt % PANEL_CHECK_INTERVAL) == 0) {
//        if (gEnabled) {
//            gPcTrigger.post();
//        }
//    }

    while (msg_cnt--) {
        message* msg = gMp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {
//sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%0.8X \n", *(uint32_t*)((myjob*) msg)->buf);
//LPRINTF(tmp);
            if (gAvPwrOn && gBat1On) {
                x = *((myjob*)msg)->buf;

                switch (x) {
                case PITCHTRIM_UP:
                    XPLMCommandOnce(gMpPtchTrmUpCmdRef);
                    break;
                case PITCHTRIM_DN:
                    XPLMCommandOnce(gMpPtchTrmDnCmdRef);
                    break;
                case FLAPS_UP:
                    XPLMCommandOnce(gMpFlpsUpCmdRef);
                    break;
                case FLAPS_DN:
                    XPLMCommandOnce(gMpFlpsDnCmdRef);
                    break;
/*
    gMpApArmedCmdRef         = XPLMFindCommand("sim/autopilot/flight_dir_on_only");
    gMpApOnCmdRef            = XPLMFindCommand("sim/autopilot/servos_and_flight_dir_on ");
    gMpApOffCmdRef           = XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off");
    gMpAltHoldCmdRef         = XPLMFindCommand("sim/autopilot/altitude_hold");
 */
                case BTN_AP_TOGGLE:
//                x = XPLMGetDatai(gMpFlghtDirModeDataRef)
//                XPLMSetDatai()
//                XPLMCommandOnce();
                    break;
                case BTN_HDG_TOGGLE:
                    XPLMCommandOnce(gMpHdgCmdRef);
                    break;
                case BTN_NAV_TOGGLE:
                    XPLMCommandOnce(gMpNavCmdRef);
                    break;
                case BTN_IAS_TOGGLE:
                    XPLMCommandOnce(gMpLvlChngCmdRef);
                    break;
                case BTN_ALT_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_VS_TOGGLE:
                    XPLMCommandOnce(gMpVrtclSpdCmdRef);
                    break;
                case BTN_APR_TOGGLE:
                    XPLMCommandOnce(gMpAppCmdRef);
                    break;
                case BTN_REV_TOGGLE:
                    XPLMCommandOnce(gMpBkCrsCmdRef);
                    break;
                case AUTOTHROTTLE_OFF:
                    XPLMCommandOnce(gMpAtThrrtlOffCmdRef);
                    break;
                case AUTOTHROTTLE_ON:
                    XPLMCommandOnce(gMpAtThrrtlOnCmdRef);
                    break;
                case ALT_UP:
                    XPLMCommandOnce(gMpAltUpCmdRef);
                    break;
                case ALT_DN:
                    XPLMCommandOnce(gMpAltDnCmdRef);
                    break;
                case VS_UP:
                    XPLMCommandOnce(gMpVrtclSpdUpCmdRef);
                    break;
                case VS_DN:
                    XPLMCommandOnce(gMpVrtclSpdDnCmdRef);
                    break;
                case IAS_UP:
                    XPLMCommandOnce(gMpAsUpCmdRef);
                    break;
                case IAS_DN:
                    XPLMCommandOnce(gMpAsDnCmdRef);
                    break;
                case HDG_UP:
                    XPLMCommandOnce(gMpHdgUpCmdRef);
                    break;
                case HDG_DN:
                    XPLMCommandOnce(gMpHdgDnCmdRef);
                    break;
                case CRS_UP:
                    XPLMCommandOnce(gMpObsHsiUpCmdRef);
                    break;
                case CRS_DN:
                    XPLMCommandOnce(gMpObsHsiDnCmdRef);
                    break;
                default:
// DPRINTF("Saitek ProPanels Plugin: UNKNOWN MSG -------\n");
                    // TODO: log error
                    break;
                }
            } // if (gAvPwrOn && gBat1On)

            delete msg;
        } // if (msg)
    } // while

// sprintf(tmp, "Saitek ProPanels Plugin: msg received - 0x%0.8X \n", *(uint32_t*)((myjob*) msg)->buf);
// DPRINTF("Saitek ProPanels Plugin: msg received -------\n");
// DPRINTF(tmp);

//    gFlCbCnt++;

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
// #ifndef NDEBUG
//     static char tmp[100];
// #endif
    uint32_t x;
    int msg_cnt = gSp_MsgProc_Cnt;

//    if ((gFlCbCnt % PANEL_CHECK_INTERVAL) == 0) {
//        if (gEnabled) {
//            gPcTrigger.post();
//        }
//    }

    while (msg_cnt--) {
        message* msg = gSp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {
            x = *((myjob*)msg)->buf;

            switch (x) {
            case SP_MAGNETOS_OFF:
                XPLMCommandOnce(gSpMagnetosOffCmdRef);
                break;
            case SP_MAGNETOS_RIGHT:
                XPLMCommandOnce(gSpMagnetosRightCmdRef);
                break;
            case SP_MAGNETOS_LEFT:
                XPLMCommandOnce(gSpMagnetosLeftCmdRef);
                break;
            case SP_MAGNETOS_BOTH:
                XPLMCommandOnce(gSpMagnetosBothCmdRef);
                break;
            case SP_MAGNETOS_START:
                XPLMCommandOnce(gSpMagnetosStartCmdRef);
                break;
            case SP_MASTER_BATTERY_ON:
                XPLMCommandOnce(gSpMasterBatteryOnCmdRef);
                break;
            case SP_MASTER_BATTERY_OFF:
                XPLMCommandOnce(gSpMasterBatteryOffCmdRef);
                break;
            case SP_MASTER_ALT_BATTERY_ON:
                XPLMCommandOnce(gSpMasterAltBatteryOnCmdRef);
                break;
            case SP_MASTER_ALT_BATTERY_OFF:
                XPLMCommandOnce(gSpMasterAltBatteryOffCmdRef);
                break;
            case SP_MASTER_AVIONICS_ON:
                XPLMCommandOnce(gSpMasterAvionicsOnCmdRef);
                break;
            case SP_MASTER_AVIONICS_OFF:
                XPLMCommandOnce(gSpMasterAvionicsOffCmdRef);
                break;
            case SP_FUEL_PUMP_ON:
                XPLMCommandOnce(gSpFuelPumpOnCmdRef);
                break;
            case SP_FUEL_PUMP_OFF:
                XPLMCommandOnce(gSpFuelPumpOffCmdRef);
                break;
            case SP_DEICE_LW_ON:
                XPLMCommandOnce(gSpDeIceOnCmdRef);
                break;
            case SP_DEICE_LW_OFF:
                XPLMCommandOnce(gSpDeIceOffCmdRef);
                break;
            case SP_PITOT_HEAT_ON:
                XPLMCommandOnce(gSpPitotHeatOnCmdRef);
                break;
            case SP_PITOT_HEAT_OFF:
                XPLMCommandOnce(gSpPitotHeatOffCmdRef);
                break;
            case SP_COWL_CLOSED:
                XPLMCommandOnce(gSpCowlClosedCmdRef);
                break;
            case SP_COWL_OPEN:
                XPLMCommandOnce(gSpCowlOpenCmdRef);
                break;
            case SP_LIGHTS_PANEL_ON:
                XPLMCommandOnce(gSpLightsPanelOnCmdRef);
                break;
            case SP_LIGHTS_PANEL_OFF:
                XPLMCommandOnce(gSpLightsPanelOffCmdRef);
                break;
            case SP_LIGHTS_BEACON_ON:
                XPLMCommandOnce(gSpLightsBeaconOnCmdRef);
                break;
            case SP_LIGHTS_BEACON_OFF:
                XPLMCommandOnce(gSpLightsBeaconOffCmdRef);
                break;
            case SP_LIGHTS_NAV_ON:
                XPLMCommandOnce(gSpLightsNavOnCmdRef);
                break;
            case SP_LIGHTS_NAV_OFF:
                XPLMCommandOnce(gSpLightsNavOffCmdRef);
                break;
            case SP_LIGHTS_STROBE_ON:
                XPLMCommandOnce(gSpLightsStrobeOnCmdRef);
                break;
            case SP_LIGHTS_STROBE_OFF:
                XPLMCommandOnce(gSpLightsStrobeOffCmdRef);
                break;
            case SP_LIGHTS_TAXI_ON:
                XPLMCommandOnce(gSpLightsTaxiOnCmdRef);
                break;
            case SP_LIGHTS_TAXI_OFF:
                XPLMCommandOnce(gSpLightsTaxiOffCmdRef);
                break;
            case SP_LIGHTS_LANDING_ON:
                XPLMCommandOnce(gSpLightsLandingOnCmdRef);
                break;
            case SP_LIGHTS_LANDING_OFF:
                XPLMCommandOnce(gSpLightsLandingOffCmdRef);
                break;
            case SP_LANDING_GEAR_UP:
                XPLMCommandOnce(gSpLandingGearUpCmdRef);
                break;
            case SP_LANDING_GEAR_DOWN:
                XPLMCommandOnce(gSpLandingGearDownCmdRef);
                break;
           default:
                break;
            }


        } // if (msg)

        delete msg;
    } // while

    return 1.0;
}


/*
 *
 */
PLUGIN_API void
XPluginStop(void) {
    LPRINTF("Saitek ProPanels Plugin: XPluginStop\n");

//    uint32_t* x = new uint32_t;
//    *x = MP_BLANK_SCRN;
//    gMp_ojq.post(new myjob(x));
//    psleep(500);
/*
    uint32_t* x;

    x = new uint32_t;
    *x = EXITING_THREAD_LOOP;
    gRp_ojq.post(new myjob(x));

    x =  new uint32_t;
    *x = EXITING_THREAD_LOOP;
    gMp_ojq.post(new myjob(x));

    x = new uint32_t;
    *x = EXITING_THREAD_LOOP;
    gSp_ojq.post(new myjob(x));
*/

#ifdef DO_USBPANEL_CHECK
    pexchange((int*)&pc_run, false);
    gPcTrigger.post();
#endif

    pexchange((int*)&threads_run, false);

    if (gRpHandle) {
        gRpTrigger.post();
        close_hid(gRpHandle);
    }

    if (gMpHandle) {
        gMpTrigger.post();
        close_hid(gMpHandle);
    }

    if (gSpHandle) {
        gSpTrigger.post();
        close_hid(gSpHandle);
    }

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
    LPRINTF("Saitek ProPanels Plugin: XPluginDisable\n");
    gEnabled = false;
    gRpTrigger.reset();
    gMpTrigger.reset();
    gSpTrigger.reset();
    XPLMSetDatai(gMpOttoOvrrde, false);
}


/*
 *
 */
PLUGIN_API int
XPluginEnable(void) {
    LPRINTF("Saitek ProPanels Plugin: XPluginEnable\n");
    gEnabled = true;

    if (gMpHandle) {
        XPLMSetDatai(gMpOttoOvrrde, true);
    }

    if (gPowerUp) {
        gPowerUp = false;
        return 1;
    }

    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    return 1;
}


/*
 *
 */
PLUGIN_API void
XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void* inParam) {
//    LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage\n");
}
