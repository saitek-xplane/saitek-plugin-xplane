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

// Flightloop callback message queue processing count defaults.
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
//#ifdef __DO_LOGFILE__
//    gLogFile = new logfile("/Users/SaitekProPanels.log\0", false);
//    gLogFile->putf("Saitek ProPanels Plugin: XPluginStart\n");
//#endif

    LPRINTF("Saitek ProPanels Plugin: XPluginStart\n");

    strcpy(outName, "SaitekProPanels");
    strcpy(outSig , "jdp.panels.saitek");
    strcpy(outDesc, "Saitek Pro Panels Plugin.");

    uint32_t* x = new uint32_t;
    *x = MP_BLANK_SCRN;
    gMp_ojq.post(new myjob(x));

    gAvPwrOnDataRef = XPLMFindDataRef("sim/cockpit2/switches/avionics_power_on");
    // XXX: no switches data ref for battery?!
    gBatPwrOnDataRef = XPLMFindDataRef("sim/cockpit/electrical/battery_on");

    // check if power is on, default is off
    if (XPLMGetDatai(gAvPwrOnDataRef)) {
        x = new uint32_t;
        *x = gAvPwrOn;
        gMp_ojq.post(new myjob(x));
        pexchange((int*)&gAvPwrOn, true);
    }

    // check if the battery is on, default is off
    if (XPLMGetDatai(gBatPwrOnDataRef)) {
        x = new uint32_t;
        *x = gBat1On;
        gMp_ojq.post(new myjob(x));
        pexchange((int*)&gBat1On, true);
    }

    // A questionable way to use the (hideous) preprocessor
    // but it makes it easier to work with this file.
    #include "multipanel_refs.cpp"
    #include "switchpanel_refs.cpp"
    #include "radiopanel_refs.cpp"

//    gLogFile->putf("Saitek ProPanels Plugin: commands initialized\n");
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

//    gLogFile->putf("Saitek ProPanels Plugin: Panel threads running\n");
    LPRINTF("Saitek ProPanels Plugin: Panel threads running\n");

    XPLMRegisterFlightLoopCallback(RadioPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(MultiPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);
    XPLMRegisterFlightLoopCallback(SwitchPanelFlightLoopCallback, FL_CB_INTERVAL, NULL);

//    gLogFile->putf("Saitek ProPanels Plugin: startup completed\n");
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
int SwitchPanelCommandHandler(XPLMCommandRef    inCommand,
                             XPLMCommandPhase  inPhase,
                             void*             inRefcon) {
    int status = CMD_PASS_EVENT;

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
}
