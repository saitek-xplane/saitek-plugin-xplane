// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#if defined(__WINDOWS__)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <stdio.h>

#include "pport.h"
#include "ptypes.h"
#include "pasync.h"
#include "ptime.h"

#include "XPLMDefs.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
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

//logfile* gLogFile;
//char gLogFilePath[512] = {};

/*
sim/flightmodel2/controls/flap_handle_deploy_ratio
sim/aircraft/specialcontrols/acf_ail1flaps
*/

USING_PTYPES

bool gPowerUp = true;
bool gEnabled = false;
unsigned int gFlCbCnt = 0;

// rp = Rp = RP = Radio Panel
// mp = Mp = MP = Milti Panel
// sp = Sp = SP = Switch Panel
static const float FL_CB_INTERVAL = -1.0;

/*
 * - register the plugin
 * - check for hid connected pro panels
 * - register callbacks
 * - start threads
 *
 */
PLUGIN_API int
XPluginStart(char* outName, char* outSig, char* outDesc) {

//    int tmp;
//#ifdef __XPTESTING__
//    gLogFile = new logfile("/Users/SaitekProPanels.log\0", false);
//    gLogFile->putf("Saitek ProPanels Plugin: XPluginStart\n");
//#endif

    DPRINTF("Saitek ProPanels Plugin: XPluginStart\n");

    strcpy(outName, "SaitekProPanels");
    strcpy(outSig , "jdp.panels.saitek");
    strcpy(outDesc, "Saitek Pro Panels Plugin.");

    /*----- MultiPanel Command Ref assignment -----*/
    gMpAsDnCmdRef            = XPLMFindCommand("sim/autopilot/airspeed_down");
    gMpAsUpCmdRef            = XPLMFindCommand("sim/autopilot/airspeed_up");
    gMpAltDnCmdRef           = XPLMFindCommand("sim/autopilot/altitude_down");
    gMpAltUpCmdRef           = XPLMFindCommand("sim/autopilot/altitude_up");
    gMpAltHoldCmdRef         = XPLMFindCommand("sim/autopilot/altitude_hold");
    gMpAppCmdRef             = XPLMFindCommand("sim/autopilot/approach");
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
//    fp = new FromPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gRpTrigger, RP_PROD_ID, &rpProcOutData);
    fp = new FromPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gRpTrigger, RP_PROD_ID);

    tp->start();
    fp->start();

    // multi panel
    tp = new ToPanelThread(gMpHandle, &gMp_ojq, &gMpTrigger, MP_PROD_ID);
//    fp = new FromPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gMpTrigger, MP_PROD_ID, &mpProcOutData);
    fp = new FromPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gMpTrigger, MP_PROD_ID);

    tp->start();
    fp->start();

    // switch panel
    tp = new ToPanelThread(gSpHandle, &gSp_ojq, &gSpTrigger, SP_PROD_ID);
//    fp = new FromPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gSpTrigger, SP_PROD_ID, &spProcOutData);
    fp = new FromPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gSpTrigger, SP_PROD_ID);

    tp->start();
    fp->start();

#ifndef NO_PANEL_CHECK
    pexchange((int*)&pc_run, true);
    PanelsCheckThread* pc = new PanelsCheckThread();
    pc->start();
#endif

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
#ifndef NDEBUG
    static char tmp[100];
#endif
//    float x;
//    unsigned int cmd;
// TODO: what's a good count, get rid of the magic number
//    unsigned int msg_cnt = 50;

//    if ((gFlCbCnt % PANEL_CHECK_INTERVAL) == 0) {
//        if (gEnabled) {
//            gPcTrigger.post();
//        }
//    }

//    while (msg_cnt-- > 0) {
        message* msg = gMp_ijq.getmessage(MSG_NOWAIT);

        if (msg) {
            sprintf(tmp, "Saitek ProPanels Plugin: msg received - 0x%0.8X \n", *(uint32_t*)((myjob*) msg)->buf);
// DPRINTF("Saitek ProPanels Plugin: msg received -------\n");
 DPRINTF(tmp);
            mp_proc_data(*(uint32_t*)((myjob*) msg)->buf);
            free((uint32_t*)((myjob*) msg)->buf);
            delete msg;
        }

/*
            free((unsigned int*)((myjob*) msg)->buf);
            delete msg;
        } else {
            break;
        }
*/
//    }

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

    x = (uint32_t*) malloc(sizeof(uint32_t));
    *x = EXITING_THREAD_LOOP;
    gRp_ojq.post(new myjob(x));

    x = (uint32_t*) malloc(sizeof(uint32_t));
    *x = EXITING_THREAD_LOOP;
    gMp_ojq.post(new myjob(x));

    x = (uint32_t*) malloc(sizeof(uint32_t));
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

/*
 *
 */
PLUGIN_API void
XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void* inParam) {

    //
}
