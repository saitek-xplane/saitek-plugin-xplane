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
#include "XPLMPlugin.h"
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

void mp_do_init();

//logfile* gLogFile;
//char gLogFilePath[512] = {};

/*
sim/flightmodel2/controls/flap_handle_deploy_ratio
sim/aircraft/specialcontrols/acf_ail1flaps
*/

USING_PTYPES

// general & misc
enum {
    PLUGIN_PLANE_ID = 0
};

// Multi panel
enum {
    MP_CMD_EAT_EVENT = 0,
    MP_CMD_PASS_EVENT = 1,
    MP_CMD_OTTO_AUTOTHROTTLE_ON = 0,
    MP_CMD_OTTO_AUTOTHROTTLE_OFF,
    MP_CMD_OTTO_AUTOTHROTTLE_TOGGLE,
    MP_CMD_FLAPS_UP,
    MP_CMD_FLAPS_DOWN,
    MP_CMD_PITCHTRIM_UP,
    MP_CMD_PITCHTRIM_DOWN,
    MP_CMD_OTTO_ON,
    MP_CMD_OTTO_OFF,
    MP_CMD_OTTO_FD_UP_ONE,
    MP_CMD_OTTO_ARMED,
    MP_CMD_OTTO_AP_BTN,
    MP_CMD_OTTO_HDG_BTN,
    MP_CMD_OTTO_NAV_BTN,
    MP_CMD_OTTO_IAS_BTN,
    MP_CMD_OTTO_ALT_ARM_BTN,
    MP_CMD_OTTO_ALT_HOLD_BTN,
    MP_CMD_OTTO_VS_BTN,
    MP_CMD_OTTO_APR_BTN,
    MP_CMD_OTTO_REV_BTN,
    MP_CMD_OTTO_ALT_UP,
    MP_CMD_OTTO_ALT_DN,
    MP_CMD_OTTO_VS_UP,
    MP_CMD_OTTO_VS_DN,
    MP_CMD_OTTO_IAS_UP,
    MP_CMD_OTTO_IAS_DN,
    MP_CMD_OTTO_HDG_UP,
    MP_CMD_OTTO_HDG_DN,
    MP_CMD_OTTO_CRS_UP,
    MP_CMD_OTTO_CRS_DN,

    CMD_SYS_AVIONICS_ON,
    CMD_SYS_AVIONICS_OFF,
    CMD_ELEC_BATTERY1_ON,
    CMD_ELEC_BATTERY1_OFF,
};

// Switch panel
enum {
    SP_CMD_MAGNETOS_OFF,
    SP_CMD_MAGNETOS_RIGHT,
    SP_CMD_MAGNETOS_LEFT,
    SP_CMD_MAGNETOS_BOTH,
    SP_CMD_MAGNETOS_START,
    SP_CMD_MASTER_BATTERY_ON,
    SP_CMD_MASTER_BATTERY_OFF,
    SP_CMD_MASTER_ALT_BATTERY_ON,
    SP_CMD_MASTER_ALT_BATTERY_OFF,
    SP_CMD_MASTER_AVIONICS_ON,
    SP_CMD_MASTER_AVIONICS_OFF,
    SP_CMD_FUEL_PUMP_ON,
    SP_CMD_FUEL_PUMP_OFF,
    SP_CMD_DEICE_ON,
    SP_CMD_DEICE_OFF,
    SP_CMD_PITOT_HEAT_ON,
    SP_CMD_PITOT_HEAT_OFF,
    SP_CMD_COWL_CLOSED,
    SP_CMD_COWL_OPEN,
    SP_CMD_LIGHTS_PANEL_ON,
    SP_CMD_LIGHTS_PANEL_OFF,
    SP_CMD_LIGHTS_BEACON_ON,
    SP_CMD_LIGHTS_BEACON_OFF,
    SP_CMD_LIGHTS_NAV_ON,
    SP_CMD_LIGHTS_NAV_OFF,
    SP_CMD_LIGHTS_STROBE_ON,
    SP_CMD_LIGHTS_STROBE_OFF,
    SP_CMD_LIGHTS_TAXI_ON,
    SP_CMD_LIGHTS_TAXI_OFF,
    SP_CMD_LIGHTS_LANDING_ON,
    SP_CMD_LIGHTS_LANDING_OFF,
    SP_CMD_GEAR_UP,
    SP_CMD_GEAR_DOWN
};

// Flightloop callback message queue processing count defaults.
// TODO: create a small user menu with a user adjustment slider (range ?)
enum {
    RP_MSGPROC_CNT = 100,
    MP_MSGPROC_CNT = 100,
    SP_MSGPROC_CNT = 100
};

// Flightloop callback message queue processing globals.
// These should be adjustable by the user via a menu item.
// For X-Plane v9.x, and possibly v10, a higher count means
// we interrupt X-Plane proper and a lower count increases
// panel input/output latency.
int32_t gRp_MsgProc_Cnt = RP_MSGPROC_CNT;
int32_t gMp_MsgProc_Cnt = MP_MSGPROC_CNT;
int32_t gSp_MsgProc_Cnt = SP_MSGPROC_CNT;

int32_t gAvPwrOn = false;
int32_t gBat1On = false;
int32_t gPlaneLoaded = false;
int32_t gPluginEnabled = false;
uint32_t gFlCbCnt = 0;

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel
// cb = Cb = CB = Callback
// Flightloop Callback INterval
static const float FL_CB_INTERVAL = -1.0;

XPLMDataRef gAvPwrOnDataRef = NULL;
XPLMDataRef gBatPwrOnDataRef = NULL;

// Multi Panel Command Refs
XPLMCommandRef gAvPwrOnCmdRef = NULL;
XPLMCommandRef gAvPwrOffCmdRef = NULL;
XPLMCommandRef gBatPwrOnCmdRef = NULL;
XPLMCommandRef gBatPwrOffCmdRef = NULL;

XPLMCommandRef gMpAsDnCmdRef = NULL;
XPLMCommandRef gMpAsUpCmdRef = NULL;
XPLMCommandRef gMpAltDnCmdRef = NULL;
XPLMCommandRef gMpAltUpCmdRef = NULL;
XPLMCommandRef gMpAltHoldCmdRef = NULL;
XPLMCommandRef gMpAltArmCmdRef = NULL;
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
XPLMCommandRef gMpNavArmCmdRef = NULL;
XPLMCommandRef gMpObsHsiDnCmdRef = NULL;
XPLMCommandRef gMpObsHsiUpCmdRef = NULL;
XPLMCommandRef gMpPtchTrmDnCmdRef = NULL;
XPLMCommandRef gMpPtchTrmUpCmdRef = NULL;
XPLMCommandRef gMpPtchTrmTkOffCmdRef = NULL;
XPLMCommandRef gMpApOnCmdRef = NULL;
XPLMCommandRef gMpApOffCmdRef = NULL;
XPLMCommandRef gMpApArmedCmdRef = NULL;
XPLMCommandRef gMpApToggleCmdRef = NULL;
XPLMCommandRef gMpApFdUpOneCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdDnCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdUpCmdRef = NULL;
XPLMCommandRef gMpVrtclSpdCmdRef = NULL;

// Multi Panel Data Refs
XPLMDataRef gMpOttoOvrrde = NULL;
//XPLMDataRef gMpAltDialFtDataRef = NULL;
XPLMDataRef gMpAltHoldStatBtnDataRef = NULL;
//XPLMDataRef gMpAltArmStatDataRef = NULL;
XPLMDataRef gMpApprchStatBtnDataRef = NULL;

XPLMDataRef gMpApOnDataRef = NULL;
XPLMDataRef gMpArspdDataRef = NULL;

XPLMDataRef gMpBckCrsStatBtnDataRef = NULL;
XPLMDataRef gMpFlghtDirModeDataRef = NULL;
XPLMDataRef gMpHdgMagDataRef = NULL;
XPLMDataRef gMpHdgStatBtnDataRef = NULL;
XPLMDataRef gMpHsiObsDegMagPltDataRef = NULL;
XPLMDataRef gMpHsiSrcSelPltDataRef = NULL;
XPLMDataRef gMpNav1CrsDefMagPltDataRef = NULL;
XPLMDataRef gMpNav2CrsDefMagPltDataRef = NULL;
XPLMDataRef gMpGpsCourseDataRef = NULL;
XPLMDataRef gMpNavStatBtnDataRef = NULL;
XPLMDataRef gMpSpdStatBtnDataRef = NULL;
XPLMDataRef gMpVrtVelDataRef = NULL;
XPLMDataRef gMpVviStatBtnDataRef = NULL;
XPLMDataRef gMpAltDataRef = NULL;
XPLMDataRef gMpAltHoldFtDataRef = NULL;

// Multi Panel reference counters
int32_t gMpPitchTrimUpPending = 0;
int32_t gMpPitchTrimDnPending = 0;
int32_t gMpFlapsUpPending = 0;
int32_t gMpFlapsDnPending = 0;
int32_t gMpBtn_Ap_TogglePending = 0;
int32_t gMpBtn_Hdg_TogglePending = 0;
int32_t gMpBtn_Nav_TogglePending = 0;
int32_t gMpBtn_Ias_TogglePending = 0;
int32_t gMpBtn_Alt_TogglePending = 0;
int32_t gMpBtn_Vs_TogglePending = 0;
int32_t gMpBtn_Apr_TogglePending = 0;
int32_t gMpBtn_Rev_TogglePending = 0;
int32_t gMpAutothrottle_togglePending = 0;
int32_t gMpAutothrottle_offPending = 0;
int32_t gMpAutothrottle_onPending = 0;
int32_t gMpAlt_Pending = 0;
int32_t gMpVs_Pending = 0;
int32_t gMpIas_Pending = 0;
int32_t gMpHdg_Pending = 0;
int32_t gMpCrs_Pending = 0;

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

// Radio Panel resources
jobqueue    gRp_ojq;
jobqueue    gRp_ijq;
jobqueue    gRp_sjq;

// Multi Panel resources
jobqueue    gMp_ijq;
jobqueue    gMp_ojq;
jobqueue    gMp_sjq;

// Switch Panel resources
jobqueue    gSp_ijq;
jobqueue    gSp_ojq;
jobqueue    gSp_sjq;

uint32_t gMpTuningThresh = 4;
uint32_t gMpHdgTuneUpCnt = 0;
uint32_t gMpHdgTuneDnCnt = 0;
uint32_t gMpCrsTuneUpCnt = 0;
uint32_t gMpCrsTuneDnCnt = 0;
uint32_t gMpIasTuneUpCnt = 0;
uint32_t gMpIasTuneDnCnt = 0;
uint32_t gMpVsTuneUpCnt = 0;
uint32_t gMpVsTuneDnCnt = 0;
uint32_t gMpAltTuneUpCnt = 0;
uint32_t gMpAltTuneDnCnt = 0;

// Multi Panel command and data refs
// command refs
#define sMP_ALTITUDE_DOWN_CR                "sim/autopilot/altitude_down"
#define sMP_ALTITUDE_UP_CR                  "sim/autopilot/altitude_up"
#define sMP_VERTICAL_SPEED_DOWN_CR          "sim/autopilot/vertical_speed_down"
#define sMP_VERTICAL_SPEED_UP_CR            "sim/autopilot/vertical_speed_up"
#define sMP_AIRSPEED_DOWN_CR                "sim/autopilot/airspeed_down"
#define sMP_AIRSPEED_UP_CR                  "sim/autopilot/airspeed_up"
#define sMP_HEADING_DOWN_CR                 "sim/autopilot/heading_down"
#define sMP_HEADING_UP_CR                   "sim/autopilot/heading_up"
#define sMP_OBS_HSI_DOWN_CR                 "sim/radios/obs_HSI_down"
#define sMP_OBS_HSI_UP_CR                   "sim/radios/obs_HSI_up"
#define sMP_FLIGHT_DIR_ON_ONLY_CR           "sim/autopilot/flight_dir_on_only"
#define sMP_FDIR_SERVOS_TOGGLE_CR           "sim/autopilot/fdir_servos_toggle"
#define sMP_FDIR_SERVOS_UP_ONE_CR           "sim/autopilot/fdir_servos_up_one"
#define sMP_SERVOS_AND_FLIGHT_DIR_ON_CR     "sim/autopilot/servos_and_flight_dir_on"
#define sMP_SERVOS_AND_FLIGHT_DIR_OFF_CR    "sim/autopilot/servos_and_flight_dir_off"
#define sMP_HEADING_CR                      "sim/autopilot/heading"
#define sMP_NAV_ARM_CR                      "sim/autopilot/NAV"
#define sMP_LEVEL_CHANGE_CR                 "sim/autopilot/level_change"
#define sMP_ALTITUDE_HOLD_CR                "sim/autopilot/altitude_hold"
#define sMP_ALTITUDE_ARM_CR                 "sim/autopilot/altitude_arm"
#define sMP_VERTICAL_SPEED_CR               "sim/autopilot/vertical_speed"
#define sMP_APPROACH_CR                     "sim/autopilot/approach"
#define sMP_BACK_COURSE_CR                  "sim/autopilot/back_course"
#define sMP_AUTOTHROTTLE_ON_CR              "sim/autopilot/autothrottle_on"
#define sMP_AUTOTHROTTLE_OFF_CR             "sim/autopilot/autothrottle_off"
#define sMP_AUTOTHROTTLE_TOGGLE_CR          "sim/autopilot/autothrottle_toggle"
#define sMP_FLAPS_DOWN_CR                   "sim/flight_controls/flaps_down"
#define sMP_FLAPS_UP_CR                     "sim/flight_controls/flaps_up"
#define sMP_PITCH_TRIM_DOWN_CR              "sim/flight_controls/pitch_trim_down"
#define sMP_PITCH_TRIM_UP_CR                "sim/flight_controls/pitch_trim_up"
#define sMP_PITCH_TRIM_TAKEOFF_CR           "sim/flight_controls/pitch_trim_takeoff"

// data refs
#define sAVIONICS_POWER_ON_DR               "sim/cockpit2/switches/avionics_power_on"
#define sBATTERY_ON_DR                      "sim/cockpit/electrical/battery_on"
#define sMP_AUTOPILOT_ON_DR                 "sim/cockpit2/autopilot/autopilot_on"
#define sMP_FLIGHT_DIRECTOR_MODE_DR         "sim/cockpit2/autopilot/flight_director_mode"
#define sMP_ALTITUDE_DR                     "sim/cockpit/autopilot/altitude"
#define sMP_ALTITUDE_HOLD_FT_DR             "sim/cockpit2/autopilot/altitude_hold_ft"
#define sMP_VVI_DIAL_FPM_DR                 "sim/cockpit2/autopilot/vvi_dial_fpm"
#define sMP_AIRSPEED_DR                     "sim/cockpit/autopilot/airspeed"
#define sMP_HEADING_DIAL_DEG_MAG_PILOT_DR   "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot"

//#define sMP_HSI_BEARING_DEG_MAG_PILOT        "sim/cockpit2/radios/indicators/hsi_bearing_deg_mag_pilot"
#define sMP_HSI_OBS_DEG_MAG_PILOT_DR        "sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot"
#define sMP_HSI_SOURCE_SELECT_PILOT_DR      "sim/cockpit2/radios/actuators/HSI_source_select_pilot"
#define sMP_NAV1_COURSE_DEG_MAG_PILOT_DR    "sim/cockpit2/radios/actuators/nav1_course_deg_mag_pilot"
#define sMP_NAV2_COURSE_DEG_MAG_PILOT_DR    "sim/cockpit2/radios/actuators/nav2_course_deg_mag_pilot"
#define sMP_GPS_COURSE_DR                   "sim/cockpit/gps/course"

//#define sMP_ALTITUDE_DIAL_FT_DR            "sim/cockpit2/autopilot/altitude_dial_ft"
//#define sMP_AIRSPEED_DIAL_KTS_MACH_DR      "sim/cockpit2/autopilot/airspeed_dial_kts_mach"
//#define sMP_VERTICAL_VELOCITY_DR           "sim/cockpit/autopilot/vertical_velocity"
#define sMP_HEADING_MAG_DR                 "sim/cockpit/autopilot/heading_mag"
#define sMP_HEADING_STATUS_DR              "sim/cockpit2/autopilot/heading_status"
#define sMP_NAV_STATUS_DR                  "sim/cockpit2/autopilot/nav_status"
#define sMP_SPEED_STATUS_DR                "sim/cockpit2/autopilot/speed_status"
#define sMP_ALTITUDE_HOLD_STATUS_DR        "sim/cockpit2/autopilot/altitude_hold_status"
#define sMP_VVI_STATUS_DR                  "sim/cockpit2/autopilot/vvi_status"
#define sMP_APPROACH_STATUS_DR             "sim/cockpit2/autopilot/approach_status"
#define sMP_BACKCOURSE_STATUS_DR           "sim/cockpit2/autopilot/backcourse_status"
// SWITCH PANEL
#define sMAGNETOS_OFF_CR               "sim/magnetos/magnetos_off"
#define sMAGNETOS_RIGHT_CR             "sim/magnetos/magnetos_right_1"
#define sMAGNETOS_LEFT_CR              "sim/magnetos/magnetos_left_1"
#define sMAGNETOS_BOTH_CR              "sim/magnetos/magnetos_both"
#define sMAGNETOS_START_CR             "sim/starters/engage_start_run"

#define sMASTER_BATTERY_ON_CR          "sim/electrical/battery_1_on"
#define sMASTER_BATTERY_OFF_CR         "sim/electrical/battery_1_off"
#define sMASTER_ALT_BATTERY_ON_CR      "sim/electrical/battery_2_on"
#define sMASTER_ALT_BATTERY_OFF_CR     "sim/electrical/battery_2_off"

#define sMASTER_AVIONICS_ON_CR         "sim/systems/avionics_on"
#define sMASTER_AVIONICS_OFF_CR        "sim/systems/avionics_off"
#define sFUEL_PUMP_ON_CR               "sim/fuel/fuel_pump_1_on"
#define sFUEL_PUMP_OFF_CR              "sim/fuel/fuel_pump_1_off"
#define sDE_ICE_ON_CR                  "sim/ice/detect_on"
#define sDE_ICE_OFF_CR                 "sim/ice/detect_off"
#define sPITOT_HEAT_ON_CR              "sim/ice/pitot_heat_on"
#define sPITOT_HEAT_OFF_CR             "sim/ice/pitot_heat_off"

#define sCOWL_CLOSED_CR                "sim/flight_controls/cowl_flaps_closed"
#define sCOWL_OPEN_CR                  "sim/flight_controls/cowl_flaps_open"

#define sLIGHTS_PANEL_ON_CR            "sim/instruments/panel_bright_down"
#define sLIGHTS_PANEL_OFF_CR           "sim/instruments/panel_bright_up"
#define sLIGHTS_BEACON_ON_CR           "sim/lights/beacon_lights_on"
#define sLIGHTS_BEACON_OFF_CR          "sim/lights/beacon_lights_off"
#define sLIGHTS_NAV_ON_CR              "sim/lights/nav_lights_on"
#define sLIGHTS_NAV_OFF_CR             "sim/lights/nav_lights_off"
#define sLIGHTS_STROBE_ON_CR           "sim/lights/strobe_lights_on"
#define sLIGHTS_STROBE_OFF_CR          "sim/lights/strobe_lights_off"
#define sLIGHTS_TAXI_ON_CR             "sim/lights/taxi_lights_on"
#define sLIGHTS_TAXI_OFF_CR            "sim/lights/taxi_lights_off"
#define sLIGHTS_LANDING_ON_CR          "sim/lights/landing_lights_on"
#define sLIGHTS_LANDING_OFF_CR         "sim/lights/landing_lights_off"

#define sLANDING_GEAR_UP_CR            "sim/flight_controls/landing_gear_down"
#define sLANDING_GEAR_DOWN_CR          "sim/flight_controls/landing_gear_up"


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

    gAvPwrOnDataRef = XPLMFindDataRef(sAVIONICS_POWER_ON_DR);
    gBatPwrOnDataRef = XPLMFindDataRef(sBATTERY_ON_DR);

    // abuse of the (evil) preprocessor
    #include "multipanel_refs.cpp"
    #include "switchpanel_refs.cpp"
    #include "radiopanel_refs.cpp"

    LPRINTF("Saitek ProPanels Plugin: commands initialized\n");

    if (init_hid(&gRpHidHandle, RP_PROD_ID)) {
        rp_init(gRpHidHandle);
    }
    if (init_hid(&gMpHidHandle, MP_PROD_ID)) {
        mp_init(gMpHidHandle);
    }
    if (init_hid(&gSpHidHandle, SP_PROD_ID)) {
        sp_init(gSpHidHandle);
    }

    pexchange((int*)&threads_run, true);

    ToPanelThread*     tp;
    FromPanelThread*   fp;

    // radio panel
    tp = new ToPanelThread(gRpHidHandle, &gRp_ojq, &gRpTrigger, RP_PROD_ID);
    fp = new FromPanelThread(gRpHidHandle, &gRp_ijq, &gRp_ojq, &gRpTrigger, RP_PROD_ID);
    tp->start();
    fp->start();

    // multi panel
    tp = new ToPanelThread(gMpHidHandle, &gMp_ojq, &gMpTrigger, MP_PROD_ID);
    fp = new FromPanelThread(gMpHidHandle, &gMp_ijq, &gMp_ojq, &gMpTrigger, MP_PROD_ID);
    tp->start();
    fp->start();

    // switch panel
    tp = new ToPanelThread(gSpHidHandle, &gSp_ojq, &gSpTrigger, SP_PROD_ID);
    fp = new FromPanelThread(gSpHidHandle, &gSp_ijq, &gSp_ojq, &gSpTrigger, SP_PROD_ID);
    tp->start();
    fp->start();

#ifdef DO_USBPANEL_CHECK
    pexchange((int*)&pc_run, true);
    PanelsCheckThread* pc = new PanelsCheckThread();
    pc->start();
#endif

    if (gRpHidHandle) { LPRINTF("Saitek ProPanels Plugin: gRpHandle\n"); gRpTrigger.post(); }
    if (gMpHidHandle) { LPRINTF("Saitek ProPanels Plugin: gMpHandle\n"); gMpTrigger.post(); }
    if (gSpHidHandle) { LPRINTF("Saitek ProPanels Plugin: gSpHandle\n"); gSpTrigger.post(); }

    LPRINTF("Saitek ProPanels Plugin: Panel threads running\n");

    XPLMRegisterFlightLoopCallback(RadioPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(MultiPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(SwitchPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);

//XPLM_API void XPLMSetErrorCallback(XPLMError_f inCallback);
//XPLM_API void XPLMGetVersions(int* outXPlaneVersion, int* outXPLMVersion, XPLMHostApplicationID* outHostID);

    LPRINTF("Saitek ProPanels Plugin: startup completed\n");

    uint32_t* x = new uint32_t;
    *x = MP_BLANK_SCRN_MSG;
    gMp_ojq.post(new myjob(x));

    mp_do_init();

    return 1;
}


/*
 *
 *
 */
int MultiPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    int32_t t = 0;
    uint32_t x = 0;
    float f = 0.0;
    uint32_t* m = NULL;
    int status = MP_CMD_PASS_EVENT;

#define DO_LPRINTFS 0
#if DO_LPRINTFS
    static char tmp[100];
#endif

    switch (reinterpret_cast<uint32_t>(inRefcon)) {
    case MP_CMD_FLAPS_UP:
        pexchange((int*)&t, gMpFlapsUpPending);
        if (t > 0) {
            pdecrement(&gMpFlapsUpPending);
            status = MP_CMD_PASS_EVENT;
        } else {
            status = MP_CMD_EAT_EVENT;
        }
        break;
    case MP_CMD_FLAPS_DOWN:
        pexchange((int*)&t, gMpFlapsDnPending);
        if (t > 0) {
            pdecrement(&gMpFlapsDnPending);
            status = MP_CMD_PASS_EVENT;
        } else {
            status = MP_CMD_EAT_EVENT;
        }
        break;
    case MP_CMD_OTTO_AUTOTHROTTLE_ON:
        pexchange((int*)&t, gMpAutothrottle_onPending);
        if (t > 0) {
            pdecrement(&gMpAutothrottle_onPending);
            status = MP_CMD_PASS_EVENT;
        } else {
            status = MP_CMD_EAT_EVENT;
        }
        break;
    case MP_CMD_OTTO_AUTOTHROTTLE_OFF:
        pexchange((int*)&t, gMpAutothrottle_offPending);
        if (t > 0) {
            pdecrement(&gMpAutothrottle_offPending);
            status = MP_CMD_PASS_EVENT;
        } else {
            status = MP_CMD_EAT_EVENT;
        }
        break;
    case MP_CMD_OTTO_AUTOTHROTTLE_TOGGLE:
        pexchange((int*)&t, gMpAutothrottle_togglePending);
        if (t > 0) {
            pdecrement(&gMpAutothrottle_togglePending);
            status = MP_CMD_PASS_EVENT;
        } else {
            status = MP_CMD_EAT_EVENT;
        }
        break;
    case CMD_SYS_AVIONICS_ON:
        pexchange((int*)&gAvPwrOn, true);
        m = new uint32_t;
        *m = AVIONICS_ON_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_SYS_AVIONICS_OFF:
        pexchange((int*)&gAvPwrOn, false);
        m = new uint32_t;
        *m = AVIONICS_OFF_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_ELEC_BATTERY1_ON:
        pexchange((int*)&gBat1On, true);
        m = new uint32_t;
        *m = BAT1_ON_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case CMD_ELEC_BATTERY1_OFF:
        pexchange((int*)&gBat1On, false);
        m = new uint32_t;
        *m = BAT1_OFF_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_FD_UP_ONE:
        m = new uint32_t;
        // Flight director mode, 0 is off, 1 is on, 2 is on with autopilot servos
        x = (uint32_t)XPLMGetDatai(gMpFlghtDirModeDataRef);
        *m = (x == 0) ? MP_BTN_AP_OFF_MSG : ((x == 2) ? MP_BTN_AP_ARMED_MSG : MP_BTN_AP_ON_MSG);
        break;
    case MP_CMD_OTTO_ON:
        m = new uint32_t;
        *m = MP_BTN_AP_ON_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_OFF:
        m = new uint32_t;
        *m = MP_BTN_AP_OFF_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_ARMED:
        m = new uint32_t;
        *m = MP_BTN_AP_ARMED_MSG;
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_ALT_UP:
        m = new uint32_t[MP_MPM_CNT];
        m[0] = MP_MPM;
        m[1] = MP_ALT_VAL_MSG;
        m[2] = (static_cast<uint32_t>(floor(XPLMGetDataf(gMpAltDataRef)))) & 0xFFFFFFFC;
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_ALT_UP 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_ALT_DN:
        m = new uint32_t[MP_MPM_CNT];
        m[0] = MP_MPM;
        m[1] = MP_ALT_VAL_MSG;
        x = static_cast<uint32_t>(ceil(XPLMGetDataf(gMpAltDataRef)));
        if ((t = x % 100)) {
            x = x - t + 100;
        }
        m[2] = x;
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_ALT_DN 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_VS_UP:
        m = new uint32_t[MP_MPM_CNT];
        f = XPLMGetDataf(gMpVrtVelDataRef);
        m[0] = MP_MPM;
        m[1] = (f < 0) ? MP_VS_VAL_NEG_MSG : MP_VS_VAL_POS_MSG;
        m[2] = (static_cast<uint32_t>(fabs(f))) & 0xFFFFFFFC;
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_VS_UP 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_VS_DN:
        m = new uint32_t[MP_MPM_CNT];
        f = XPLMGetDataf(gMpVrtVelDataRef);
        m[0] = MP_MPM;
        m[1] = (f < 0) ? MP_VS_VAL_NEG_MSG : MP_VS_VAL_POS_MSG;
        x = static_cast<uint32_t>(fabs(f));
        if ((t = x % 100)) {
            x = x - t + 100;
        }
        m[2] = x;
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_VS_DN 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_IAS_UP:
    case MP_CMD_OTTO_IAS_DN:
        m = new uint32_t[MP_MPM_CNT];
        m[0] = MP_MPM;
        m[1] = MP_IAS_VAL_MSG;
        m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpArspdDataRef)));
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_IAS 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_HDG_UP:
    case MP_CMD_OTTO_HDG_DN:
        m = new uint32_t[MP_MPM_CNT];
        m[0] = MP_MPM;
        m[1] = MP_HDG_VAL_MSG;
        m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpHdgMagDataRef)));
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_HDG 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_CRS_UP:
    case MP_CMD_OTTO_CRS_DN:
        m = new uint32_t[MP_MPM_CNT];
        m[0] = MP_MPM;
        m[1] = MP_CRS_VAL_MSG;
        m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpHsiObsDegMagPltDataRef)));
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_CRS 0x%X:%d:%d \n", m[0], m[1], m[2]);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    //--- Buttons
    case MP_CMD_OTTO_ALT_HOLD_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpAltHoldStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_ALT_OFF_MSG : ((x == 2) ? MP_BTN_ALT_CAPT_MSG : MP_BTN_ALT_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_ALT_HOLD_BTN %d \n", x);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
//    case MP_CMD_OTTO_ALT_ARM_BTN:
//        pexchange((int*)&t, gMpBtn_Alt_TogglePending);
//        if (t > 0) {
//            pdecrement(&gMpBtn_Alt_TogglePending);
//        } else {
//            m = new uint32_t;
//            x = (uint32_t)XPLMGetDatai(gMpAltArmStatDataRef);
//            *m = (x == 0) ? MP_BTN_ALT_OFF_MSG : ((x == 2) ? MP_BTN_ALT_CAPT_MSG : MP_BTN_ALT_ARMED_MSG);
//            gMp_ojq.post(new myjob(m));
//        }
//        break;
    case MP_CMD_OTTO_APR_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpApprchStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_APR_OFF_MSG : ((x == 2) ? MP_BTN_APR_CAPT_MSG : MP_BTN_APR_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_APR_BTN %d \n", x);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_REV_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpBckCrsStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_REV_OFF_MSG : ((x == 2) ? MP_BTN_REV_CAPT_MSG : MP_BTN_REV_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_REV_BTN %d \n", x);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_HDG_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpHdgStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_HDG_OFF_MSG : ((x == 2) ? MP_BTN_HDG_CAPT_MSG : MP_BTN_HDG_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_HDG_BTN %d \n", x);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_NAV_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpNavStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_NAV_OFF_MSG : ((x == 2) ? MP_BTN_NAV_CAPT_MSG : MP_BTN_NAV_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_NAV_BTN %d \n", x);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_IAS_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpSpdStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_IAS_OFF_MSG : ((x == 2) ? MP_BTN_IAS_CAPT_MSG : MP_BTN_IAS_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_IAS_BTN %d \n", x);
        LPRINTF(tmp);
#endif
        gMp_ojq.post(new myjob(m));
        break;
    case MP_CMD_OTTO_VS_BTN:
        m = new uint32_t;
        x = (uint32_t)XPLMGetDatai(gMpVviStatBtnDataRef);
        *m = (x == 0) ? MP_BTN_VS_OFF_MSG : ((x == 2) ? MP_BTN_VS_CAPT_MSG : MP_BTN_VS_ARMED_MSG);
#if DO_LPRINTFS
        sprintf(tmp, "Saitek ProPanels Plugin: MP_CMD_OTTO_VS_BTN %d \n", x);
        LPRINTF(tmp);
#endif
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
    int status = MP_CMD_PASS_EVENT;

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
    int status = MP_CMD_PASS_EVENT;
    LPRINTF("Saitek ProPanels Plugin: switch panel lights landing on\n");

    switch (reinterpret_cast<uint32_t>(inRefcon)) {
    case SP_CMD_MAGNETOS_OFF:
        m = new uint32_t;
        *m = SP_MAGNETOS_OFF_MSG;
        //
        break;
    case SP_CMD_MAGNETOS_RIGHT:
        m = new uint32_t;
        *m = SP_MAGNETOS_RIGHT_MSG;
        //
        break;
    case SP_CMD_MAGNETOS_LEFT:
        m = new uint32_t;
        *m = SP_MAGNETOS_LEFT_MSG;
        //
        break;
    case SP_CMD_MAGNETOS_BOTH:
        m = new uint32_t;
        *m = SP_MAGNETOS_BOTH_MSG;
        //
        break;
    case SP_CMD_MAGNETOS_START:
        m = new uint32_t;
        *m = SP_MAGNETOS_START_MSG;
        //
        break;
    case SP_CMD_MASTER_BATTERY_ON:
        m = new uint32_t;
        *m = SP_MASTER_BATTERY_ON_MSG;
        //
        break;
    case SP_CMD_MASTER_BATTERY_OFF:
        m = new uint32_t;
        *m = SP_MASTER_BATTERY_OFF_MSG;
        //
        break;
    case SP_CMD_MASTER_ALT_BATTERY_ON:
        m = new uint32_t;
        *m = SP_MASTER_ALT_BATTERY_ON_MSG;
        //
        break;
    case SP_CMD_MASTER_ALT_BATTERY_OFF:
        m = new uint32_t;
        *m = SP_MASTER_ALT_BATTERY_OFF_MSG;
        //
        break;
    case SP_CMD_MASTER_AVIONICS_ON:
        m = new uint32_t;
        *m = SP_MASTER_AVIONICS_ON_MSG;
        //
        break;
    case SP_CMD_MASTER_AVIONICS_OFF:
        m = new uint32_t;
        *m = SP_MASTER_AVIONICS_OFF_MSG;
        //
        break;
    case SP_CMD_FUEL_PUMP_ON:
        m = new uint32_t;
        *m = SP_FUEL_PUMP_ON_MSG;
        //
        break;
    case SP_CMD_FUEL_PUMP_OFF:
        m = new uint32_t;
        *m = SP_FUEL_PUMP_OFF_MSG;
        //
        break;
    case SP_CMD_DEICE_ON:
        m = new uint32_t;
        *m = SP_DEICE_ON_MSG;
        //
        break;
    case SP_CMD_DEICE_OFF:
        m = new uint32_t;
        *m = SP_DEICE_OFF_MSG;
        //
        break;
    case SP_CMD_PITOT_HEAT_ON:
        m = new uint32_t;
        *m = SP_PITOT_HEAT_ON_MSG;
        //
        break;
    case SP_CMD_PITOT_HEAT_OFF:
        m = new uint32_t;
        *m = SP_PITOT_HEAT_OFF_MSG;
        //
        break;
    case SP_CMD_COWL_CLOSED:
        m = new uint32_t;
        *m = SP_COWL_CLOSED_MSG;
        //
        break;
    case SP_CMD_COWL_OPEN:
        m = new uint32_t;
        *m = SP_COWL_OPEN_MSG;
        //
        break;
    case SP_CMD_LIGHTS_PANEL_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_PANEL_ON_MSG;
        //
        break;
    case SP_CMD_LIGHTS_PANEL_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_PANEL_OFF_MSG;
        //
        break;
    case SP_CMD_LIGHTS_BEACON_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_BEACON_ON_MSG;
        //
        break;
    case SP_CMD_LIGHTS_BEACON_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_BEACON_OFF_MSG;
        //
        break;
    case SP_CMD_LIGHTS_NAV_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_NAV_ON_MSG;
        //
        break;
    case SP_CMD_LIGHTS_NAV_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_NAV_OFF_MSG;
        //
        break;
    case SP_CMD_LIGHTS_STROBE_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_STROBE_ON_MSG;
        //
        break;
    case SP_CMD_LIGHTS_STROBE_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_STROBE_OFF_MSG;
        //
        break;
    case SP_CMD_LIGHTS_TAXI_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_TAXI_ON_MSG;
        //
        break;
    case SP_CMD_LIGHTS_TAXI_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_TAXI_OFF_MSG;
        //
        break;
    case SP_CMD_LIGHTS_LANDING_ON:
        m = new uint32_t;
        *m = SP_LIGHTS_LANDING_ON_MSG;
        //
        break;
    case SP_CMD_LIGHTS_LANDING_OFF:
        m = new uint32_t;
        *m = SP_LIGHTS_LANDING_OFF_MSG;
        //
        break;
    case SP_CMD_GEAR_UP:
        m = new uint32_t;
        *m = SP_LANDING_GEAR_UP_MSG;
        //
        break;
    case SP_CMD_GEAR_DOWN:
        m = new uint32_t;
        *m = SP_LANDING_GEAR_DOWN_MSG;
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
//        if ((bool)gPluginEnabled) {
//            gPcTrigger.post();
//        }
//    }

    while (msg_cnt--) {
        message* msg = gRp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {

        }
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
//        if ((boolgPluginEnabled) {
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
                //--- pitch
                case MP_PITCHTRIM_UP_CMD_MSG:
                    XPLMCommandOnce(gMpPtchTrmUpCmdRef);
                    break;
                case MP_PITCHTRIM_DN_CMD_MSG:
                    XPLMCommandOnce(gMpPtchTrmDnCmdRef);
                    break;
                //--- flaps
                case MP_FLAPS_UP_CMD_MSG:
                    pincrement(&gMpFlapsUpPending);
                    XPLMCommandOnce(gMpFlpsUpCmdRef);
                    break;
                case MP_FLAPS_DN_CMD_MSG:
                    pincrement(&gMpFlapsDnPending);
                    XPLMCommandOnce(gMpFlpsDnCmdRef);
                    break;
                //--- autothrottle
                // gMpAutothrottle_togglePending ??
                case MP_AUTOTHROTTLE_ON_CMD_MSG:
                    pincrement(&gMpAutothrottle_onPending);
                    XPLMCommandOnce(gMpAtThrrtlOnCmdRef);
                    break;
                case MP_AUTOTHROTTLE_OFF_CMD_MSG:
                    pincrement(&gMpAutothrottle_offPending);
                    XPLMCommandOnce(gMpAtThrrtlOffCmdRef);
                    break;
                //-- buttons
                case MP_BTN_AP_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpApToggleCmdRef);
                    break;
                case MP_BTN_FD_UP_ONE_CMD_MSG:
                    XPLMCommandOnce(gMpApFdUpOneCmdRef);
                    break;
                case MP_BTN_HDG_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpHdgCmdRef);
                    break;
                case MP_BTN_NAV_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpNavArmCmdRef);
                    break;
                case MP_BTN_IAS_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpLvlChngCmdRef);
                    break;
                case MP_BTN_ALT_TOGGLE_CMD_MSG:
//                    XPLMCommandOnce(&gMpAltArmCmdRef);
                    XPLMCommandOnce(&gMpAltHoldCmdRef);
                    break;
                case MP_BTN_VS_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpVrtclSpdCmdRef);
                    break;
                case MP_BTN_APR_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpAppCmdRef);
                    break;
                case MP_BTN_REV_TOGGLE_CMD_MSG:
                    XPLMCommandOnce(gMpBkCrsCmdRef);
                    break;
                //--- tuning
                case MP_ALT_UP_CMD_MSG:
                    gMpAltTuneUpCnt += 1;
                    if (gMpAltTuneUpCnt >= gMpTuningThresh) {
                        gMpAltTuneUpCnt = 0;
                        XPLMCommandOnce(gMpAltUpCmdRef);
                    }
                    gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_ALT_DN_CMD_MSG:
                    gMpAltTuneDnCnt += 1;
                    if (gMpAltTuneDnCnt >= gMpTuningThresh) {
                        gMpAltTuneDnCnt = 0;
                        XPLMCommandOnce(gMpAltDnCmdRef);
                    }
                    gMpAltTuneUpCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_VS_UP_CMD_MSG:
                    gMpVsTuneUpCnt += 1;
                    if (gMpVsTuneUpCnt >= gMpTuningThresh) {
                        gMpVsTuneUpCnt = 0;
                        XPLMCommandOnce(gMpVrtclSpdUpCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_VS_DN_CMD_MSG:
                    gMpVsTuneDnCnt += 1;
                    if (gMpVsTuneDnCnt >= gMpTuningThresh) {
                        gMpVsTuneDnCnt = 0;
                        XPLMCommandOnce(gMpVrtclSpdDnCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_IAS_UP_CMD_MSG:
                    gMpIasTuneUpCnt += 1;
                    if (gMpIasTuneUpCnt >= gMpTuningThresh) {
                        gMpIasTuneUpCnt = 0;
                        XPLMCommandOnce(gMpAsUpCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_IAS_DN_CMD_MSG:
                    gMpIasTuneDnCnt += 1;
                    if (gMpIasTuneDnCnt >= gMpTuningThresh) {
                        gMpIasTuneDnCnt = 0;
                        XPLMCommandOnce(gMpAsDnCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_HDG_UP_CMD_MSG:
                    gMpHdgTuneUpCnt += 1;
                    if (gMpHdgTuneUpCnt >= gMpTuningThresh) {
                        gMpHdgTuneUpCnt = 0;
                        XPLMCommandOnce(gMpHdgUpCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_HDG_DN_CMD_MSG:
                    gMpHdgTuneDnCnt += 1;
                    if (gMpHdgTuneDnCnt >= gMpTuningThresh) {
                        gMpHdgTuneDnCnt = 0;
                        XPLMCommandOnce(gMpHdgDnCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = 0;
                    gMpCrsTuneUpCnt = gMpCrsTuneDnCnt = 0;
                    break;
                case MP_CRS_UP_CMD_MSG:
                    gMpCrsTuneUpCnt += 1;
                    if (gMpCrsTuneUpCnt >= gMpTuningThresh) {
                        gMpCrsTuneUpCnt = 0;
                        XPLMCommandOnce(gMpObsHsiUpCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneDnCnt = 0;
                    break;
                case MP_CRS_DN_CMD_MSG:
                    gMpCrsTuneDnCnt += 1;
                    if (gMpCrsTuneDnCnt >= gMpTuningThresh) {
                        gMpCrsTuneDnCnt = 0;
                        XPLMCommandOnce(gMpObsHsiDnCmdRef);
                    }
                    gMpAltTuneUpCnt = gMpAltTuneDnCnt = 0;
                    gMpVsTuneUpCnt = gMpVsTuneDnCnt = 0;
                    gMpIasTuneUpCnt = gMpIasTuneDnCnt = 0;
                    gMpHdgTuneUpCnt = gMpHdgTuneDnCnt = 0;
                    gMpCrsTuneUpCnt = 0;
                    break;
                default:
                    // DPRINTF("Saitek ProPanels Plugin: UNKNOWN MSG -------\n");
                    // TODO: log error
                    break;
                } // switch (x)
            } // if (gAvPwrOn && gBat1On)
        } // if (msg)
        delete msg;
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
//        if ((boolgPluginEnabled) {
//            gPcTrigger.post();
//        }
//    }

    while (msg_cnt--) {
        message* msg = gSp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {
            x = *((myjob*)msg)->buf;

            switch (x) {
            case SP_MAGNETOS_OFF_MSG:
                XPLMCommandOnce(gSpMagnetosOffCmdRef);
                break;
            case SP_MAGNETOS_RIGHT_MSG:
                XPLMCommandOnce(gSpMagnetosRightCmdRef);
                break;
            case SP_MAGNETOS_LEFT_MSG:
                XPLMCommandOnce(gSpMagnetosLeftCmdRef);
                break;
            case SP_MAGNETOS_BOTH_MSG:
                XPLMCommandOnce(gSpMagnetosBothCmdRef);
                break;
            case SP_MAGNETOS_START_MSG:
                XPLMCommandOnce(gSpMagnetosStartCmdRef);
                break;
            case SP_MASTER_BATTERY_ON_MSG:
                XPLMCommandOnce(gSpMasterBatteryOnCmdRef);
                break;
            case SP_MASTER_BATTERY_OFF_MSG:
                XPLMCommandOnce(gSpMasterBatteryOffCmdRef);
                break;
            case SP_MASTER_ALT_BATTERY_ON_MSG:
                XPLMCommandOnce(gSpMasterAltBatteryOnCmdRef);
                break;
            case SP_MASTER_ALT_BATTERY_OFF_MSG:
                XPLMCommandOnce(gSpMasterAltBatteryOffCmdRef);
                break;
            case SP_MASTER_AVIONICS_ON_MSG:
                XPLMCommandOnce(gSpMasterAvionicsOnCmdRef);
                break;
            case SP_MASTER_AVIONICS_OFF_MSG:
                XPLMCommandOnce(gSpMasterAvionicsOffCmdRef);
                break;
            case SP_FUEL_PUMP_ON_MSG:
                XPLMCommandOnce(gSpFuelPumpOnCmdRef);
                break;
            case SP_FUEL_PUMP_OFF_MSG:
                XPLMCommandOnce(gSpFuelPumpOffCmdRef);
                break;
            case SP_DEICE_ON_MSG:
                XPLMCommandOnce(gSpDeIceOnCmdRef);
                break;
            case SP_DEICE_OFF_MSG:
                XPLMCommandOnce(gSpDeIceOffCmdRef);
                break;
            case SP_PITOT_HEAT_ON_MSG:
                XPLMCommandOnce(gSpPitotHeatOnCmdRef);
                break;
            case SP_PITOT_HEAT_OFF_MSG:
                XPLMCommandOnce(gSpPitotHeatOffCmdRef);
                break;
            case SP_COWL_CLOSED_MSG:
                XPLMCommandOnce(gSpCowlClosedCmdRef);
                break;
            case SP_COWL_OPEN_MSG:
                XPLMCommandOnce(gSpCowlOpenCmdRef);
                break;
            case SP_LIGHTS_PANEL_ON_MSG:
                XPLMCommandOnce(gSpLightsPanelOnCmdRef);
                break;
            case SP_LIGHTS_PANEL_OFF_MSG:
                XPLMCommandOnce(gSpLightsPanelOffCmdRef);
                break;
            case SP_LIGHTS_BEACON_ON_MSG:
                XPLMCommandOnce(gSpLightsBeaconOnCmdRef);
                break;
            case SP_LIGHTS_BEACON_OFF_MSG:
                XPLMCommandOnce(gSpLightsBeaconOffCmdRef);
                break;
            case SP_LIGHTS_NAV_ON_MSG:
                XPLMCommandOnce(gSpLightsNavOnCmdRef);
                break;
            case SP_LIGHTS_NAV_OFF_MSG:
                XPLMCommandOnce(gSpLightsNavOffCmdRef);
                break;
            case SP_LIGHTS_STROBE_ON_MSG:
                XPLMCommandOnce(gSpLightsStrobeOnCmdRef);
                break;
            case SP_LIGHTS_STROBE_OFF_MSG:
                XPLMCommandOnce(gSpLightsStrobeOffCmdRef);
                break;
            case SP_LIGHTS_TAXI_ON_MSG:
                XPLMCommandOnce(gSpLightsTaxiOnCmdRef);
                break;
            case SP_LIGHTS_TAXI_OFF_MSG:
                XPLMCommandOnce(gSpLightsTaxiOffCmdRef);
                break;
            case SP_LIGHTS_LANDING_ON_MSG:
                XPLMCommandOnce(gSpLightsLandingOnCmdRef);
                break;
            case SP_LIGHTS_LANDING_OFF_MSG:
                XPLMCommandOnce(gSpLightsLandingOffCmdRef);
                break;
            case SP_LANDING_GEAR_UP_MSG:
                XPLMCommandOnce(gSpLandingGearUpCmdRef);
                break;
            case SP_LANDING_GEAR_DOWN_MSG:
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
//    *x = MP_BLANK_SCRN_MSG;
//    gMp_ojq.post(new myjob(x));
//    psleep(500);
/*
    uint32_t* x;

    x = new uint32_t;
    *x = G_EXITING_THREAD_LOOP_MSG;
    gRp_ojq.post(new myjob(x));

    x =  new uint32_t;
    *x = G_EXITING_THREAD_LOOP_MSG;
    gMp_ojq.post(new myjob(x));

    x = new uint32_t;
    *x = G_EXITING_THREAD_LOOP_MSG;
    gSp_ojq.post(new myjob(x));
*/

#ifdef DO_USBPANEL_CHECK
    pexchange((int*)&pc_run, false);
    gPcTrigger.post();
#endif

    pexchange((int*)&threads_run, false);

    if (gRpHidHandle) {
        gRpTrigger.post();
        close_hid(gRpHidHandle);
    }

    if (gMpHidHandle) {
        gMpTrigger.post();
        close_hid(gMpHidHandle);
    }

    if (gSpHidHandle) {
        gSpTrigger.post();
        close_hid(gSpHidHandle);
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

    pexchange((int*)&gPluginEnabled, false);
    gRpTrigger.reset();
    gMpTrigger.reset();
    gSpTrigger.reset();

    // set any panel specific globals here
    if (gMpHidHandle) {
        XPLMSetDatai(gMpOttoOvrrde, false);
    }
    if (gSpHidHandle) {
    }
    if (gRpHidHandle) {
    }
}


/*
 *
 */
PLUGIN_API int
XPluginEnable(void) {
    LPRINTF("Saitek ProPanels Plugin: XPluginEnable\n");

    pexchange((int*)&gPluginEnabled, false);

    // set any panel specific globals here
    if (gMpHidHandle) {
        XPLMSetDatai(gMpOttoOvrrde, true);
    }
    if (gSpHidHandle) {
    }
    if (gRpHidHandle) {
    }

    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    return 1;
}


void mp_do_init() {
    float f;
    int32_t t;
    uint32_t* m;
    uint32_t* x;

    pexchange((int*)&gPlaneLoaded, true); // always first
    x = new uint32_t;
//    if (XPLMGetDatai(gAvPwrOnDataRef)) {
        pexchange((int*)&gAvPwrOn, true);
        *x = AVIONICS_ON_MSG;
        gMp_ojq.post(new myjob(x));
//    } else {
//        pexchange((int*)&gAvPwrOn, false);
//        *x = AVIONICS_OFF_MSG;
//        gMp_ojq.post(new myjob(x));
//    }
    x = new uint32_t;
//    if (XPLMGetDatai(gBatPwrOnDataRef)) {
        pexchange((int*)&gBat1On, true);
        *x = BAT1_ON_MSG;
        gMp_ojq.post(new myjob(x));
//    } else {
//        pexchange((int*)&gBat1On, false);
//        *x = BAT1_OFF_MSG;
//        gMp_ojq.post(new myjob(x));
//    }
    // ALT val init
    m = new uint32_t[MP_MPM_CNT];
    m[0] = MP_MPM;
    m[1] = MP_ALT_VAL_MSG;
    m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpAltDataRef)));
    gMp_ojq.post(new myjob(m));
    // VS val init
    m = new uint32_t[MP_MPM_CNT];
    f = XPLMGetDataf(gMpVrtVelDataRef);
    m[0] = MP_MPM;
    m[1] = (f < 0) ? MP_VS_VAL_NEG_MSG : MP_VS_VAL_POS_MSG;
    m[2] = static_cast<uint32_t>(fabs(f));
    gMp_ojq.post(new myjob(m));
    // IAS val init
    m = new uint32_t[MP_MPM_CNT];
    m[0] = MP_MPM;
    m[1] = MP_IAS_VAL_MSG;
    m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpArspdDataRef)));
    gMp_ojq.post(new myjob(m));
    // HDG val init
    m = new uint32_t[MP_MPM_CNT];
    m[0] = MP_MPM;
    m[1] = MP_HDG_VAL_MSG;
    m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpHdgMagDataRef)));
    gMp_ojq.post(new myjob(m));
    // CRS val init
    x = new uint32_t;
    m[0] = MP_MPM;
    m[1] = MP_CRS_VAL_MSG;
    m[2] = static_cast<uint32_t>(floor(XPLMGetDataf(gMpHsiObsDegMagPltDataRef)));
    gMp_ojq.post(new myjob(m));
    //--- buttons
    // AP button
    x = new uint32_t;
    // Flight director mode, 0 is off, 1 is on, 2 is on with autopilot servos
    t = XPLMGetDatai(gMpFlghtDirModeDataRef);
    *x = (t == 0) ? MP_BTN_AP_OFF_MSG : ((t == 2) ? MP_BTN_AP_ON_MSG : MP_BTN_AP_ARMED_MSG);
//    t = XPLMGetDatai(gMpApOnDataRef);
//    *x = (t == 0) ? MP_BTN_AP_OFF_MSG : MP_BTN_AP_ON_MSG;
    gMp_ojq.post(new myjob(x));
    // ALT button
    x = new uint32_t;
    t = XPLMGetDatai(gMpAltHoldStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_ALT_OFF_MSG : ((t == 2) ? MP_BTN_ALT_CAPT_MSG : MP_BTN_ALT_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // APR button
    x = new uint32_t;
    t = XPLMGetDatai(gMpApprchStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_APR_OFF_MSG : ((t == 2) ? MP_BTN_APR_CAPT_MSG : MP_BTN_APR_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // REV button
    x = new uint32_t;
    t = XPLMGetDatai(gMpBckCrsStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_REV_OFF_MSG : ((t == 2) ? MP_BTN_REV_CAPT_MSG : MP_BTN_REV_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // HDG button
    x = new uint32_t;
    t = XPLMGetDatai(gMpHdgStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_HDG_OFF_MSG : ((t == 2) ? MP_BTN_HDG_CAPT_MSG : MP_BTN_HDG_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // NAV button
    x = new uint32_t;
    t = XPLMGetDatai(gMpNavStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_NAV_OFF_MSG : ((t == 2) ? MP_BTN_NAV_CAPT_MSG : MP_BTN_NAV_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // IAS button
    x = new uint32_t;
    t = XPLMGetDatai(gMpSpdStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_IAS_OFF_MSG : ((t == 2) ? MP_BTN_IAS_CAPT_MSG : MP_BTN_IAS_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // VS button
    x = new uint32_t;
    t = XPLMGetDatai(gMpVviStatBtnDataRef);
    *x = (t == 0) ? MP_BTN_VS_OFF_MSG : ((t == 2) ? MP_BTN_VS_CAPT_MSG : MP_BTN_VS_ARMED_MSG);
    gMp_ojq.post(new myjob(x));
    // LED update
    x = new uint32_t;
    *x = MP_UPDATE_LEDS;
    gMp_ojq.post(new myjob(x));
}


/*
 *
 */
PLUGIN_API void
XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void* inParam) {
//    LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage\n");

    uint32_t* x;
    if (inFrom == XPLM_PLUGIN_XPLANE) {
    int inparam = reinterpret_cast<int>(inParam);
        switch (inMsg) {
        case XPLM_MSG_PLANE_LOADED:
            if (inparam != PLUGIN_PLANE_ID || gPlaneLoaded) {
                break;
            }
            if (gSpHidHandle) {
            }
            if (gRpHidHandle) {
            }
            if (gMpHidHandle) {
                mp_do_init();
            }
            LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage XPLM_MSG_PLANE_LOADED\n");
            break;
        case XPLM_MSG_AIRPORT_LOADED:
            LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage XPLM_MSG_AIRPORT_LOADED\n");
            break;
        case XPLM_MSG_SCENERY_LOADED:
            LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage XPLM_MSG_SCENERY_LOADED\n");
            break;
        case XPLM_MSG_AIRPLANE_COUNT_CHANGED:
            LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage XPLM_MSG_AIRPLANE_COUNT_CHANGED\n");
            break;
// XXX: what's different between an unloaded and crashed plane
// as far as system state and procedure?
        case XPLM_MSG_PLANE_CRASHED:
            if ((int)inParam != PLUGIN_PLANE_ID) {
                break;
            }
            if (gSpHidHandle) {
            }
            if (gRpHidHandle) {
            }
            if (gMpHidHandle) {
                x = new uint32_t;
                *x = MP_PLANE_CRASH_MSG;
                gMp_ojq.post(new myjob(x));
            }
            LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage XPLM_MSG_PLANE_CRASHED\n");
            break;
        case XPLM_MSG_PLANE_UNLOADED:
            if ((int)inParam != PLUGIN_PLANE_ID) {
                break;
            }
            if (gSpHidHandle) {
            }
            if (gRpHidHandle) {
            }
            if (gMpHidHandle) {
                x = new uint32_t;
                if ((bool)XPLMGetDatai(gAvPwrOnDataRef)) {
                    pexchange((int*)&gAvPwrOn, false);
                    *x = AVIONICS_OFF_MSG;
                    gMp_ojq.post(new myjob(x));
                } else {
                    pexchange((int*)&gAvPwrOn, true);
                    *x = AVIONICS_ON_MSG;
                    gMp_ojq.post(new myjob(x));
                }
                x = new uint32_t;
                if ((bool)XPLMGetDatai(gBatPwrOnDataRef)) {
                    pexchange((int*)&gBat1On, false);
                    *x = BAT1_OFF_MSG;
                    gMp_ojq.post(new myjob(x));
                } else {
                    pexchange((int*)&gBat1On, true);
                    *x = BAT1_ON_MSG;
                    gMp_ojq.post(new myjob(x));
                }
            }
            pexchange((int*)&gPlaneLoaded, false); // always last
            LPRINTF("Saitek ProPanels Plugin: XPluginReceiveMessage XPLM_MSG_PLANE_UNLOADED\n");
            break;
        default:
            // unknown
            break;
        } // switch (inMsg)
    } // if (inFrom == XPLM_PLUGIN_XPLANE)
}
