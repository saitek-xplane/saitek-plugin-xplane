// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#if defined(__WINDOWS__)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

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


int MultiPanelCommandHandler(XPLMCommandRef inCommand,
                             XPLMCommandPhase inPhase,
                             void* inRefcon);

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

    CMD_SYS_AVIONICS_TOGGLE,
    CMD_FLTCTL_FLAPS_UP,
    CMD_FLTCTL_FLAPS_DOWN,
    CMD_FLTCTL_PITCHTRIM_UP,
    CMD_FLTCTL_PITCHTRIM_DOWN,
    CMD_FLTCTL_PITCHTRIM_TAKEOFF,
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

int gAvPwrOn = false;
int gBat1On = false;

bool gPowerUp = true;
bool gEnabled = false;
unsigned int gFlCbCnt = 0;

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel
static const float FL_CB_INTERVAL = -1.0;


XPLMDataRef gAvPwrOnDataRef = NULL;
XPLMDataRef gBatPwrOnDataRef = NULL;

/* Command Refs */
XPLMCommandRef gAvPwrOnCmdRef = NULL;
XPLMCommandRef gAvPwrOffCmdRef = NULL;
XPLMCommandRef gBat1PwrOnCmdRef = NULL;
XPLMCommandRef gBat1PwrOffCmdRef = NULL;

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
XPLMCommandRef gMpFdirSrvUp1CmdRef = NULL;
XPLMCommandRef gMpFdirSrvDn1CmdRef = NULL;
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
XPLMCommandRef gMpSrvsFlghtDirOffCmdRef = NULL;
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
XPLMDataRef gMpAvncsOnDataRef = NULL;
XPLMDataRef gMpBckCrsStatDataRef = NULL;
XPLMDataRef gMpBttryOnDataRef = NULL;
XPLMDataRef gMpFlghtDirModeDataRef = NULL;
XPLMDataRef gMpHdgMagDataRef = NULL;
XPLMDataRef gMpHdgStatDataRef = NULL;
XPLMDataRef gMpHsiObsDegMagPltDataRef = NULL;
XPLMDataRef gMpNavStatDataRef = NULL;
XPLMDataRef gMpSpdStatDataRef = NULL;
XPLMDataRef gMpVrtVelDataRef = NULL;
XPLMDataRef gMpVviStatDataRef = NULL;
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
//    int tmp;
//#ifdef __XPTESTING__
//    gLogFile = new logfile("/Users/SaitekProPanels.log\0", false);
//    gLogFile->putf("Saitek ProPanels Plugin: XPluginStart\n");
//#endif

    DPRINTF("Saitek ProPanels Plugin: XPluginStart\n");

    strcpy(outName, "SaitekProPanels");
    strcpy(outSig , "jdp.panels.saitek");
    strcpy(outDesc, "Saitek Pro Panels Plugin.");

    if (XPLMGetDatai(gAvPwrOnDataRef))
        pexchange((int*)&gAvPwrOn, true);

    if (XPLMGetDatai(gBatPwrOnDataRef))
        pexchange((int*)&gBat1On, true);

    gAvPwrOnDataRef          = XPLMFindDataRef("sim/cockpit/electrical/avionics_on");
    gBatPwrOnDataRef         = XPLMFindDataRef("sim/cockpit/electrical/battery_on");

    gAvPwrOnCmdRef           = XPLMFindCommand("sim/systems/avionics_on");
    gAvPwrOffCmdRef          = XPLMFindCommand("sim/systems/avionics_off");
    gBat1PwrOnCmdRef         = XPLMFindCommand("sim/electrical/battery_1_on ");
    gBat1PwrOffCmdRef        = XPLMFindCommand("sim/electrical/battery_1_off ");

    /*----- MultiPanel Command Ref assignment -----*/
    gMpAsDnCmdRef            = XPLMFindCommand("sim/autopilot/airspeed_down");
    gMpAsUpCmdRef            = XPLMFindCommand("sim/autopilot/airspeed_up");
    gMpAltDnCmdRef           = XPLMFindCommand("sim/autopilot/altitude_down");
    gMpAltUpCmdRef           = XPLMFindCommand("sim/autopilot/altitude_up");
    gMpAltHoldCmdRef         = XPLMFindCommand("sim/autopilot/altitude_hold");
    gMpAppCmdRef             = XPLMFindCommand("sim/autopilot/approach");
    gMpAtThrrtlOnCmdRef      = XPLMFindCommand("sim/autopilot/autothrottle_on");
    gMpAtThrrtlOffCmdRef     = XPLMFindCommand("sim/autopilot/autothrottle_off");
    gMpAtThrrtlTgglCmdRef    = XPLMFindCommand("sim/autopilot/autothrottle_toggle");
    gMpBkCrsCmdRef           = XPLMFindCommand("sim/autopilot/back_course");
    gMpFdirSrvUp1CmdRef      = XPLMFindCommand("sim/autopilot/fdir_servos_up_one");
    gMpFdirSrvDn1CmdRef      = XPLMFindCommand("sim/autopilot/fdir_servos_down_one");
    gMpFlpsDnCmdRef          = XPLMFindCommand("sim/flight_controls/flaps_down");
    gMpFlpsUpCmdRef          = XPLMFindCommand("sim/flight_controls/flaps_up");
    gMpHdgCmdRef             = XPLMFindCommand("sim/autopilot/heading");
    gMpHdgDnCmdRef           = XPLMFindCommand("sim/autopilot/heading_down");
    gMpHdgUpCmdRef           = XPLMFindCommand("sim/autopilot/heading_up");
    gMpLvlChngCmdRef         = XPLMFindCommand("sim/autopilot/level_change");
    gMpNavCmdRef             = XPLMFindCommand("sim/autopilot/NAV");
    gMpObsHsiDnCmdRef        = XPLMFindCommand("sim/radios/obs_HSI_down");
    gMpObsHsiUpCmdRef        = XPLMFindCommand("sim/radios/obs_HSI_up");
    gMpPtchTrmDnCmdRef       = XPLMFindCommand("sim/flight_controls/pitch_trim_down");
    gMpPtchTrmUpCmdRef       = XPLMFindCommand("sim/flight_controls/pitch_trim_up");
    gMpPtchTrmTkOffCmdRef    = XPLMFindCommand("sim/flight_controls/pitch_trim_takeoff");
    gMpSrvsFlghtDirOffCmdRef = XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off");
    gMpVrtclSpdDnCmdRef      = XPLMFindCommand("sim/autopilot/vertical_speed_down");
    gMpVrtclSpdUpCmdRef      = XPLMFindCommand("sim/autopilot/vertical_speed_up");
    gMpVrtclSpdCmdRef        = XPLMFindCommand("sim/autopilot/vertical_speed");

    /*----- MultiPanel Data Ref assignment -----*/
    gMpOttoOvrrde             = XPLMFindDataRef("sim/operation/override/override_autopilot");
    gMpArspdDataRef           = XPLMFindDataRef("sim/cockpit/autopilot/airspeed");
    gMpAltDataRef             = XPLMFindDataRef("sim/cockpit/autopilot/altitude");
    gMpAltHoldStatDataRef     = XPLMFindDataRef("sim/cockpit2/autopilot/altitude_hold_status");
    gMpApprchStatDataRef      = XPLMFindDataRef("sim/cockpit2/autopilot/approach_status");
    gMpApStateDataRef         = XPLMFindDataRef("sim/cockpit/autopilot/autopilot_state");
    gMpAvncsOnDataRef         = XPLMFindDataRef("sim/cockpit/electrical/avionics_on");
    gMpBckCrsStatDataRef      = XPLMFindDataRef("sim/cockpit2/autopilot/backcourse_status");
    gMpBttryOnDataRef         = XPLMFindDataRef("sim/cockpit/electrical/battery_on");
    gMpFlghtDirModeDataRef    = XPLMFindDataRef("sim/cockpit2/autopilot/flight_director_mode");
    gMpHdgMagDataRef          = XPLMFindDataRef("sim/cockpit/autopilot/heading_mag");
    gMpHdgStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/heading_status");
    gMpHsiObsDegMagPltDataRef = XPLMFindDataRef("sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot");
    gMpNavStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/nav_status");
    gMpSpdStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/speed_status");
    gMpVrtVelDataRef          = XPLMFindDataRef("sim/cockpit/autopilot/vertical_velocity");
    gMpVviStatDataRef         = XPLMFindDataRef("sim/cockpit2/autopilot/vvi_status");

    /*----- MultiPanel Command Handlers -----*/
    cmd_ref = XPLMCreateCommand((const char*)gMpAtThrrtlOnCmdRef, "Auto Throttle On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_AUTOTHROTTLE_ON);

    cmd_ref = XPLMCreateCommand((const char*)gMpAtThrrtlOffCmdRef, "Auto Throttle Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_OTTO_AUTOTHROTTLE_OFF);

    cmd_ref = XPLMCreateCommand((const char*)gAvPwrOnCmdRef, "Avionics On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_SYS_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand((const char*)gAvPwrOffCmdRef, "Avionics Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_SYS_AVIONICS_ON);

    cmd_ref = XPLMCreateCommand((const char*)gBat1PwrOnCmdRef, "Battery 1 On");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_ELEC_BATTERY1_ON);

    cmd_ref = XPLMCreateCommand((const char*)gBat1PwrOffCmdRef, "Batter 1 Off");
    XPLMRegisterCommandHandler(cmd_ref, MultiPanelCommandHandler, true, (void*)CMD_ELEC_BATTERY1_OFF);

//    gLogFile->putf("Saitek ProPanels Plugin: commands initialized\n");
    DPRINTF("Saitek ProPanels Plugin: commands initialized\n");

    if (init_hid(&gRpHandle, RP_PROD_ID))
        rp_init(gRpHandle);

    if (init_hid(&gMpHandle, MP_PROD_ID))
        mp_init(gMpHandle);

    if (init_hid(&gSpHandle, SP_PROD_ID))
        sp_init(gSpHandle);

    pexchange((int*)&threads_run, true);

    ToPanelThread*     tp;
    FromPanelThread*   fp;

    // radio panel: queue to the panel
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

#ifndef NO_PANEL_CHECK
    pexchange((int*)&pc_run, true);
    PanelsCheckThread* pc = new PanelsCheckThread();
    pc->start();
#endif

    if (gRpHandle) { DPRINTF("Saitek ProPanels Plugin: gRpHandle\n"); gRpTrigger.post(); }
    if (gMpHandle) { XPLMSetDatai(gMpOttoOvrrde, true); DPRINTF("Saitek ProPanels Plugin: gMpHandle\n"); gMpTrigger.post(); }
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

/*
 *
 *
 */
int MultiPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {

    int status = CMD_PASS_EVENT;
    uint32_t* m;

// TODO: check/set item state for some events!?

    switch (reinterpret_cast<long>(inRefcon)) {
    case CMD_OTTO_AUTOTHROTTLE_ON:
        break;
    case CMD_OTTO_AUTOTHROTTLE_OFF:
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
    default:
        break;
    }

// return 0 to eat the event and 1 to let x-plane process it
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
// #ifndef NDEBUG
     static char tmp[100];
// #endif

     uint32_t x;
    // TODO: best count value?
    int msg_cnt = 50;

//    if ((gFlCbCnt % PANEL_CHECK_INTERVAL) == 0) {
//        if (gEnabled) {
//            gPcTrigger.post();
//        }
//    }

    while (msg_cnt--) {
        message* msg = gMp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {
// sprintf(tmp, "Saitek ProPanels Plugin: msg received  0x%0.8X \n", *(uint32_t*)((myjob*) msg)->buf);
// DPRINTF(tmp);

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
                case BTN_AP_TOGGLE:
//                x = XPLMGetDatai(gMpFlghtDirModeDataRef)
//                XPLMSetDatai()
//                XPLMCommandOnce();
                    break;
                case BTN_HDG_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_NAV_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_IAS_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_ALT_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_VS_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_APR_TOGGLE:
//                XPLMCommandOnce();
                    break;
                case BTN_REV_TOGGLE:
//                XPLMCommandOnce(gMpBkCrsCmdRef);
                    break;
                case KNOB_ALT_POS:
//                XPLMCommandOnce();
                    break;
                case KNOB_VS_POS:
//                XPLMCommandOnce();
                    break;
                case KNOB_IAS_POS:
//                XPLMCommandOnce();
                    break;
                case KNOB_HDG_POS:
//                XPLMCommandOnce();
                    break;
                case KNOB_CRS_POS:
//                XPLMCommandOnce();
                    break;
                case AUTOTHROTTLE_OFF:
                    XPLMCommandOnce(gMpAtThrrtlOffCmdRef);
                    break;
                case AUTOTHROTTLE_ON:
                    XPLMCommandOnce(gMpAtThrrtlOnCmdRef);
                    break;
                case TUNING_RIGHT:
//                XPLMCommandOnce();
                    break;
                case TUNING_LEFT:
//                XPLMCommandOnce();
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
 */
PLUGIN_API void
XPluginStop(void) {

    DPRINTF("Saitek ProPanels Plugin: XPluginStop\n");

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

#ifndef NO_PANEL_CHECK
    pexchange((int*)&pc_run, false);
#endif

    pexchange((int*)&threads_run, false);

    gPcTrigger.post();
    gRpTrigger.post();
    gMpTrigger.post();
    gSpTrigger.post();

    if (gRpHandle) {
        close_hid(gRpHandle);
    }

    if (gMpHandle) {
        close_hid(gMpHandle);
    }

    if (gSpHandle) {
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

    gEnabled = true;

    if (gMpHandle)
        XPLMSetDatai(gMpOttoOvrrde, true);

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

    //
}
