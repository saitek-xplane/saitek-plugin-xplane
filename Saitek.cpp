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

#include "Saitek.h"
#include "PanelThreads.h"
#include "XPLMProcessing.h"

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
// XPLMRegisterFlightLoopCallback(InputOutputLoopCB, 1.0, NULL);
// XPLMUnregisterFlightLoopCallback(InputOutputLoopCB, NULL);
// int NumberOfEngines = XPLMGetDatai(gInputDataRef[1]);
/*
    {"AP Disconnect", xplm_key_otto_dis},
    {"AP Auto Throttle", xplm_key_otto_atr},
    {"AP Air Speed", xplm_key_otto_asi},
    {"AP Heading", xplm_key_otto_hdg},
    {"AP GPS", xplm_key_otto_gps},
    {"AP Wing Leveler", xplm_key_otto_lev},
    {"AP HNAV", xplm_key_otto_hnav},
    {"AP Altitude", xplm_key_otto_alt},
    {"AP Vertical Speed", xplm_key_otto_vvi},
    {"AP VNAV", xplm_key_otto_vnav},
    {"AP NAV1", xplm_key_otto_nav1},
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
	{"Flaps Up", xplm_key_flapsup},
	{"Flaps Down", xplm_key_flapsdn},
	{"Carb Heat Off", xplm_key_cheatoff},
	{"Carb Heat On", xplm_key_cheaton},
	{"Speed Brake Off", xplm_key_sbrkoff},
	{"Speed Brake On", xplm_key_sbrkon},
	{"Aileron Trim Left", xplm_key_ailtrimL},
	{"Aileron Trim Right", xplm_key_ailtrimR},
	{"Rudder Trim Left", xplm_key_rudtrimL},
	{"Rudder Trim Right", xplm_key_rudtrimR},
	{"Elevator Trim Down", xplm_key_elvtrimD},
	{"Elevator Trim Up", xplm_key_elvtrimU},
*/

RadioPanelThread* prp_thread = 0;
MultiPanelThread* pmp_thread = 0;
SwitchPanelThread* psp_thread = 0;

// Radio Panel
hid_device*         gRpHandle;
jobqueue            gRp_ojq;
jobqueue            gRp_ijq;

// Multi Panel
hid_device*         gMpHandle;
jobqueue            gMp_ijq;
jobqueue            gMp_ojq;

// Switch Panel
hid_device*         gSpHandle;
jobqueue            gSp_ijq;
jobqueue            gSp_ojq;

trigger             gState(false, false);

/*
 *
 */
PLUGIN_API int
XPluginStart(char* outName, char* outSig, char* outDesc) {

    strcpy(outName, "saitek");
    strcpy(outSig , "xpsdk.examples.Control");
    strcpy(outDesc, "Saitek Pro Control Panels Plugin.");

	gRpHandle = hid_open(VENDOR_ID, RP_PROD_ID, NULL);
	gMpHandle = hid_open(VENDOR_ID, MP_PROD_ID, NULL);
	gSpHandle = hid_open(VENDOR_ID, SP_PROD_ID, NULL);

    if (gRpHandle) {
        XPLMRegisterFlightLoopCallback(rpSendMsg, RP_CB_INTERVAL, NULL);
        XPLMRegisterFlightLoopCallback(rpReceiveMsg, RP_CB_INTERVAL, NULL);
        prp_thread = new RadioPanelThread(gRpHandle, &gRp_ijq, &gRp_ojq, &gState);

        prp_thread->start();
    }

    if (gMpHandle) {
        XPLMRegisterFlightLoopCallback(mpSendMsg, MP_CB_INTERVAL, NULL);
        XPLMRegisterFlightLoopCallback(mpReceiveMsg, MP_CB_INTERVAL, NULL);
        pmp_thread = new MultiPanelThread(gMpHandle, &gMp_ijq, &gMp_ojq, &gState);

        pmp_thread->start();
    }

    if (gSpHandle) {
        XPLMRegisterFlightLoopCallback(spSendMsg, SP_CB_INTERVAL, NULL);
        XPLMRegisterFlightLoopCallback(spReceiveMsg, SP_CB_INTERVAL, NULL);
        psp_thread = new SwitchPanelThread(gSpHandle, &gSp_ijq, &gSp_ojq, &gState);

        psp_thread->start();
    }

    return 1;
}

//            u8_cnt = ((myjob*) msg)->u8_amt;
//            u8_buf = ((myjob*) msg)->data_buf;

//float FlightLoopCB(
//                                   float                inElapsedSinceLastCall,
//                                   float                inElapsedTimeSinceLastFlightLoop,
//                                   int                  inCounter,
//                                   void *               inRefcon)

//typedef float (* XPLMFlightLoop_f)(
//                                   float                inElapsedSinceLastCall,
//                                   float                inElapsedTimeSinceLastFlightLoop,
//                                   int                  inCounter,
//                                   void *               inRefcon);

//XPLM_API void                 XPLMRegisterFlightLoopCallback(
//                                   XPLMFlightLoop_f     inFlightLoop,
//                                   float                inInterval,
//                                   void *               inRefcon);

//XPLM_API void                 XPLMUnregisterFlightLoopCallback(
//                                   XPLMFlightLoop_f     inFlightLoop,
//                                   void *               inRefcon);

/*
 *
 */
float rpSendMsg(float inElapsedSinceLastCall,
                float inElapsedTimeSinceLastFlightLoop,
                int inCounter,
                void* inRefcon) {
pout.putf("Hello from rpSendMsg callback: %d\n", inCounter);
    // get radio data from xplane and send message
//    gRp_ijq.post(new myjob(ui_rcv_cnt, alloc_buf));

    return 0;

}

/*
 *
 */
float rpReceiveMsg(float inElapsedSinceLastCall,
                   float inElapsedTimeSinceLastFlightLoop,
                   int inCounter,
                   void* inRefcon) {
pout.putf("Hello from rpReceiveMsg callback: %d\n", inCounter);
    // get radio panel message and set xplane radio data

    return 0;
}

/*
 *
 */
float mpSendMsg(float inElapsedSinceLastCall,
                float inElapsedTimeSinceLastFlightLoop,
                int inCounter,
                void* inRefcon) {
pout.putf("Hello from mpSendMsg callback: %d\n", inCounter);
    // get multi data from xplane and send message

    return 0;
}

/*
 *
 */
float mpReceiveMsg(float inElapsedSinceLastCall,
                   float inElapsedTimeSinceLastFlightLoop,
                   int inCounter,
                   void* inRefcon) {
pout.putf("Hello from mpReceiveMsg callback: %d\n", inCounter);
    // get multi panel message and set xplane multi data

    return 0;
}

/*
 *
 */
float spSendMsg(float inElapsedSinceLastCall,
                float inElapsedTimeSinceLastFlightLoop,
                int inCounter,
                void* inRefcon) {
pout.putf("Hello from spSendMsg callback: %d\n", inCounter);
    // get switch data from xplane and send message

    return 0;
}

/*
 *
 */
float spReceiveMsg(float inElapsedSinceLastCall,
                   float inElapsedTimeSinceLastFlightLoop,
                   int inCounter,
                   void* inRefcon) {
pout.putf("Hello from spReceiveMsg callback: %d\n", inCounter);
    // get switch panel message and set xplane switch data

    return 0;
}


PLUGIN_API void
XPluginStop(void) {

    if (prp_thread && prp_thread->get_running()) {
        pexchange(&(prp_thread->run), false);
        prp_thread = 0;
    }

    if (pmp_thread && pmp_thread->get_running()) {
        pexchange(&(pmp_thread->run), false);
        pmp_thread = 0;
    }

    if (psp_thread && psp_thread->get_running()) {
        pexchange(&(psp_thread->run), false);
        psp_thread = 0;
    }

    if (gRpHandle) {
        hid_close(gRpHandle);
        gRpHandle = 0;
    }

    if (gMpHandle) {
        hid_close(gMpHandle);
        gMpHandle = 0;
    }

    if (gSpHandle) {
        hid_close(gSpHandle);
        gSpHandle = 0;
    }
}

PLUGIN_API void
XPluginDisable(void) {

    gState.reset();

    if (prp_thread) {
        pexchange(&(prp_thread->pend), true);
    }

    if (pmp_thread) {
        pexchange(&(pmp_thread->pend), true);
    }

    if (psp_thread) {
        pexchange(&(psp_thread->pend), true);
    }
}

PLUGIN_API int
XPluginEnable(void) {

    gState.signal();

    return 1;
}


