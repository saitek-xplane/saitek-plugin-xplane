// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef DEFS_H
#define DEFS_H

#ifdef USE_NED
  #define free nedfree
#endif

#ifdef NDEBUG
 #ifndef _STATIC_ASSERT_
  #define static_assert(e) \
    do { \
      enum {static_assert__ = 1/(e)}; \
    } while (0)
 #endif

 #ifndef  _STATIC_ASSERT_MSG_
  #define static_assert_msg(e, msg) \
    do { \
      enum { static_assert_msg__ ## msg = 1/(e)}; \
    } while (0)
 #endif
#endif

// Standard printf
#ifdef NPRINTF
    #define PRINTF(fmt)
    #define PRINTF_VA(fmt, ...)
#else
# ifdef __TESTING__
    #define PRINTF(fmt)                 pout.putf(fmt)
    #define PRINTF_VA(fmt, ...)         pout.putf(fmt, __VA_ARGS__)
# else
    #define PRINTF(fmt)                 XPLMDebugString(fmt)
    #define PRINTF_VA(fmt, ...)         XPLMDebugString(fmt, __VA_ARGS__)
# endif
#endif

// Debug printf
#ifdef NDEBUG
    #define DPRINTF(fmt)
    #define DPRINTF_VA(fmt, ...)
#else
# ifdef __TESTING__
    #define DPRINTF(fmt)                pout.putf(fmt)
    #define DPRINTF_VA(fmt, ...)        pout.putf(fmt, __VA_ARGS__)
# else
    #define DPRINTF(fmt)                XPLMDebugString(fmt)
    #define DPRINTF_VA(fmt, ...)
# endif
#endif

#ifdef LOGPRINTF
    #define LPRINTF(fmt)                XPLMDebugString(fmt)
    #define LPRINTF_VA(fmt, ...)
#else
    #define LPRINTF(fmt)
    #define LPRINTF_VA(fmt, ...)
#endif

// Error printf
#ifdef NEPRINTF
    #define EPRINTF(fmt)
    #define EPRINTF_VA(fmt, ...)
#else
# ifdef __TESTING__
    #define EPRINTF(fmt)                pout.putf(fmt)
    #define EPRINTF_VA(fmt, ...)        pout.putf(fmt, __VA_ARGS__)
# else
    #define EPRINTF(fmt)                XPLMDebugString(fmt)
    #define EPRINTF_VA(fmt, ...)        XPLMDebugString(fmt, __VA_ARGS__)
# endif
#endif

// CommandHandler pre-event and post-event designators
#define CMD_HNDLR_PROLOG (1)
#define CMD_HNDLR_EPILOG (0)

#define IN_BUF_CNT  (4)
#define OUT_BUF_CNT (13)
#define RP_OUT_BUF_CNT (23)
#define MSG_NOWAIT  (0)
#define MSG_WAIT    (-1)

typedef void (*pHidInit) ();
typedef unsigned char* (*pProcOutData) (unsigned int);


#define MP_APBTN_BITPOS             (0)
#define MP_HDGBTN_BITPOS            (1)
#define MP_NAVBTN_BITPOS            (2)
#define MP_IASBTN_BITPOS            (3)
#define MP_ALTBTN_BITPOS            (4)
#define MP_VSBTN_BITPOS             (5)
#define MP_APRBTN_BITPOS            (6)
#define MP_REVBTN_BITPOS            (7)
#define MP_READ_KNOB_MODE_MASK      (0x0000001F)
#define MP_READ_BTNS_MASK           (0x00007F80)
#define MP_READ_FLAPS_MASK          (0x00030000)
#define MP_READ_TRIM_MASK           (0x000C0000)
#define MP_READ_TUNING_MASK         (0x00000060)
#define MP_READ_THROTTLE_MASK       (0x00008000)
#define MP_READ_KNOB_ALT            (0x00000001)
#define MP_READ_KNOB_VS             (0x00000002)
#define MP_READ_KNOB_IAS            (0x00000004)
#define MP_READ_KNOB_HDG            (0x00000008)
#define MP_READ_KNOB_CRS            (0x00000010)
#define MP_READ_TUNING_RIGHT        (0x00000020)
#define MP_READ_TUNING_LEFT         (0x00000040)
#define MP_READ_AP_BTN              (0x00000080)
#define MP_READ_HDG_BTN             (0x00000100)
#define MP_READ_NAV_BTN             (0x00000200)
#define MP_READ_IAS_BTN             (0x00000400)
#define MP_READ_ALT_BTN             (0x00000800)
#define MP_READ_VS_BTN              (0x00001000)
#define MP_READ_APR_BTN             (0x00002000)
#define MP_READ_REV_BTN             (0x00004000)
#define MP_READ_THROTTLE_ON         (0x00008000)
#define MP_READ_THROTTLE_OFF        (0x00000000)
#define MP_READ_FLAPS_UP            (0x00010000)
#define MP_READ_FLAPS_DN            (0x00020000)
#define MP_READ_TRIM_DOWN           (0x00040000)
#define MP_READ_TRIM_UP             (0x00080000)
#define MP_READ_NOMSG               (0xFFFFFFFF)

#define SP_READ_MASTER_BAT_MASK       (0x000001)
#define SP_READ_MASTER_ALT_MASK       (0x000002)
#define SP_READ_AVIONICS_MASTER_MASK  (0x000004)
#define SP_READ_FUEL_PUMP_MASK        (0x000008)
#define SP_READ_DE_ICE_MASK           (0x000010)
#define SP_READ_PITOT_HEAT_MASK       (0x000020)
#define SP_READ_COWL_MASK             (0x000040)
#define SP_READ_LIGHTS_PANEL_MASK     (0x000080)
#define SP_READ_LIGHTS_BEACON_MASK    (0x000100)
#define SP_READ_LIGHTS_NAV_MASK       (0x000200)
#define SP_READ_LIGHTS_STROBE_MASK    (0x000400)
#define SP_READ_LIGHTS_TAXI_MASK      (0x000800)
#define SP_READ_LIGHTS_LANDING_MASK   (0x001000)
#define SP_READ_ENGINES_KNOB_MASK     (0x03E000)
#define SP_READ_GEARLEVER_DOWN_MASK   (0x040000)
#define SP_READ_GEARLEVER_UP_MASK     (0x080000)

#define RP_READ_UPPER_KNOB_MODE_MASK (0x00007F)
#define RP_READ_UPPER_KNOB_COM1      (0x000001)
#define RP_READ_UPPER_KNOB_COM2      (0x000002)
#define RP_READ_UPPER_KNOB_NAV1      (0x000004)
#define RP_READ_UPPER_KNOB_NAV2      (0x000008)
#define RP_READ_UPPER_KNOB_ADF       (0x000010)
#define RP_READ_UPPER_KNOB_DME       (0x000020)
#define RP_READ_UPPER_KNOB_TRANSPNDR (0x000040)
#define RP_READ_LOWER_KNOB_MODE_MASK (0x003F80)
#define RP_READ_LOWER_KNOB_COM1      (0x000080)
#define RP_READ_LOWER_KNOB_COM2      (0x000100)
#define RP_READ_LOWER_KNOB_NAV1      (0x000200)
#define RP_READ_LOWER_KNOB_NAV2      (0x000400)
#define RP_READ_LOWER_KNOB_ADF       (0x000800)
#define RP_READ_LOWER_KNOB_DME       (0x001000)
#define RP_READ_LOWER_KNOB_TRANSPNDR (0x002000)
#define RP_READ_UPPER_ACT_STBY       (0x004000)
#define RP_READ_LOWER_ACT_STBY       (0x008000)
#define RP_READ_UPPER_FINE_TUNING_MASK (0x030000)
#define RP_READ_UPPER_COARSE_TUNING_MASK (0x0C0000)
#define RP_READ_UPPER_FINE_RIGHT     (0x010000)
#define RP_READ_UPPER_FINE_LEFT      (0x020000)
#define RP_READ_UPPER_COARSE_RIGHT   (0x040000)
#define RP_READ_UPPER_COARSE_LEFT    (0x080000)
#define RP_READ_LOWER_FINE_TUNING_MASK (0x300000)
#define RP_READ_LOWER_COARSE_TUNING_MASK (0xC00000)
#define RP_READ_LOWER_FINE_RIGHT     (0x100000)
#define RP_READ_LOWER_FINE_LEFT      (0x200000)
#define RP_READ_LOWER_COARSE_RIGHT   (0x400000)
#define RP_READ_LOWER_COARSE_LEFT    (0x800000)


enum {
    MP_MPM                  = 0x0FFFFFFF, // multi-part message
    MP_MPM_CNT              = 3, // MP multi-part message
    RP_MPM                  = 0x0FFFFFFF, // multi-part message
    RP_MPM_CNT              = 3, // MP multi-part message
//    MP_MPM_ID_NDX           = 0,
//    MP_MPM_MSG_ID_NDX       = ,
//    MP_MPM_MSG_VAL_NDX
    HID_READ_CNT            = 4,
    HID_ERROR               = -1,
    VENDOR_ID               = 0x06A3,
    RP_PROD_ID              = 0x0D05,
    MP_PROD_ID              = 0x0D06,
    SP_PROD_ID              = 0x0D67,
    RP_ERROR_THRESH         = 40,
    MP_ERROR_THRESH         = 40,
    SP_ERROR_THRESH         = 40,
    PANEL_CHECK_INTERVAL    = 3,


    AVIONICS_ON_MSG = 2,
    AVIONICS_OFF_MSG,
    BAT1_ON_MSG,
    BAT1_OFF_MSG,
    MP_BLANK_SCRN_MSG,
    MP_ZERO_SCRN_MSG,
    MP_PLANE_CRASH_MSG,
    MP_BTN_AP_OFF_MSG,
    MP_BTN_AP_ON_MSG,  // 10
    MP_BTN_AP_ARMED_MSG,
    MP_BTN_HDG_OFF_MSG,
    MP_BTN_HDG_ARMED_MSG,
    MP_BTN_HDG_CAPT_MSG,
    MP_BTN_NAV_OFF_MSG,
    MP_BTN_NAV_ARMED_MSG,
    MP_BTN_NAV_CAPT_MSG,
    MP_BTN_IAS_OFF_MSG,
    MP_BTN_IAS_ARMED_MSG,
    MP_BTN_IAS_CAPT_MSG, // 20
    MP_BTN_ALT_OFF_MSG,
    MP_BTN_ALT_ARMED_MSG,
    MP_BTN_ALT_CAPT_MSG,
    MP_BTN_VS_OFF_MSG,
    MP_BTN_VS_ARMED_MSG,
    MP_BTN_VS_CAPT_MSG,
    MP_BTN_APR_OFF_MSG,
    MP_BTN_APR_ARMED_MSG,
    MP_BTN_APR_CAPT_MSG,
    MP_BTN_REV_OFF_MSG, // 30
    MP_BTN_REV_ARMED_MSG,
    MP_BTN_REV_CAPT_MSG,
    MP_KNOB_ALT_POS_MSG,
    MP_KNOB_VS_POS_MSG,
    MP_KNOB_IAS_POS_MSG,
    MP_KNOB_HDG_POS_MSG,
    MP_KNOB_CRS_POS_MSG,
    MP_ALT_VAL_MSG,
    MP_VS_VAL_POS_MSG,
    MP_VS_VAL_NEG_MSG, // 40
    MP_IAS_VAL_MSG,
    MP_HDG_VAL_MSG,
    MP_CRS_VAL_MSG,
    MP_UPDATE_LEDS,

    // multi panel command messages
    MP_PITCHTRIM_UP_CMD_MSG,
    MP_PITCHTRIM_DN_CMD_MSG,
    MP_FLAPS_UP_CMD_MSG,
    MP_FLAPS_DN_CMD_MSG,
    MP_AUTOTHROTTLE_ON_CMD_MSG,
    MP_AUTOTHROTTLE_OFF_CMD_MSG,
    MP_BTN_AP_TOGGLE_CMD_MSG,
    MP_BTN_FD_UP_ONE_CMD_MSG,
    MP_BTN_HDG_TOGGLE_CMD_MSG,
    MP_BTN_NAV_TOGGLE_CMD_MSG,
    MP_BTN_IAS_TOGGLE_CMD_MSG,
    MP_BTN_ALT_TOGGLE_CMD_MSG,
    MP_BTN_VS_TOGGLE_CMD_MSG,
    MP_BTN_APR_TOGGLE_CMD_MSG,
    MP_BTN_REV_TOGGLE_CMD_MSG,
    MP_ALT_UP_CMD_MSG,
    MP_ALT_DN_CMD_MSG = MP_ALT_UP_CMD_MSG + 1,
    MP_VS_UP_CMD_MSG,
    MP_VS_DN_CMD_MSG = MP_VS_UP_CMD_MSG + 1,
    MP_IAS_UP_CMD_MSG,
    MP_IAS_DN_CMD_MSG = MP_IAS_UP_CMD_MSG + 1,
    MP_HDG_UP_CMD_MSG,
    MP_HDG_DN_CMD_MSG = MP_HDG_UP_CMD_MSG + 1,
    MP_CRS_UP_CMD_MSG,
    MP_CRS_DN_CMD_MSG = MP_CRS_UP_CMD_MSG + 1,

    SP_MAGNETOS_OFF_CMD_MSG,
    SP_MAGNETOS_RIGHT_CMD_MSG,
    SP_MAGNETOS_LEFT_CMD_MSG,
    SP_MAGNETOS_BOTH_CMD_MSG,
    SP_ENGINE_START_CMD_MSG,
    SP_MASTER_BATTERY_ON_CMD_MSG,
    SP_MASTER_BATTERY_OFF_CMD_MSG,
    SP_MASTER_ALT_BATTERY_ON_CMD_MSG,
    SP_MASTER_ALT_BATTERY_OFF_CMD_MSG,
    SP_MASTER_AVIONICS_ON_CMD_MSG,
    SP_MASTER_AVIONICS_OFF_CMD_MSG,
    SP_FUEL_PUMP_ON_CMD_MSG,
    SP_FUEL_PUMP_OFF_CMD_MSG,
    SP_DEICE_ON_CMD_MSG,
    SP_DEICE_OFF_CMD_MSG,
    SP_PITOT_HEAT_ON_CMD_MSG,
    SP_PITOT_HEAT_OFF_CMD_MSG,
    SP_COWL_CLOSED_CMD_MSG,
    SP_COWL_OPEN_CMD_MSG,
    SP_LIGHTS_PANEL_ON_CMD_MSG,
    SP_LIGHTS_PANEL_OFF_CMD_MSG,
    SP_LIGHTS_BEACON_ON_CMD_MSG,
    SP_LIGHTS_BEACON_OFF_CMD_MSG,
    SP_LIGHTS_NAV_ON_CMD_MSG,
    SP_LIGHTS_NAV_OFF_CMD_MSG,
    SP_LIGHTS_STROBE_ON_CMD_MSG,
    SP_LIGHTS_STROBE_OFF_CMD_MSG,
    SP_LIGHTS_TAXI_ON_CMD_MSG,
    SP_LIGHTS_TAXI_OFF_CMD_MSG,
    SP_LIGHTS_LANDING_ON_CMD_MSG,
    SP_LIGHTS_LANDING_OFF_CMD_MSG,
    SP_LANDING_GEAR_UP_CMD_MSG,
    SP_LANDING_GEAR_DOWN_CMD_MSG,
    SP_LANDING_GEAR_RETRACT_CMD_MSG,

    SP_BLANK_SCRN_MSG,
    SP_ALL_GREEN_SCRN_MSG,
    SP_ALL_RED_SCRN_MSG,
    SP_ALL_ORANGE_SCRN_MSG,

    RP_BLANK_SCRN_MSG,
    RP_ZERO_SCRN_MSG,

    RP_UP_KNOB_COM1_POS_MSG,
    RP_UP_KNOB_COM2_POS_MSG,
    RP_UP_KNOB_NAV1_POS_MSG,
    RP_UP_KNOB_NAV2_POS_MSG,
    RP_UP_KNOB_ADF_POS_MSG,
    RP_UP_KNOB_DME_POS_MSG,
    RP_UP_KNOB_XPDR_POS_MSG,
    RP_LO_KNOB_COM1_POS_MSG,
    RP_LO_KNOB_COM2_POS_MSG,
    RP_LO_KNOB_NAV1_POS_MSG,
    RP_LO_KNOB_NAV2_POS_MSG,
    RP_LO_KNOB_ADF_POS_MSG,
    RP_LO_KNOB_DME_POS_MSG,
    RP_LO_KNOB_XPDR_POS_MSG,

    RP_COM1_VAL_MSG,
    RP_COM1_STDBY_VAL_MSG,
    RP_COM2_VAL_MSG,
    RP_COM2_STDBY_VAL_MSG,

    // Radio Panel Command Messages
    RP_COM1_COARSE_UP_CMD_MSG,
    RP_COM1_COARSE_DOWN_CMD_MSG,
    RP_COM1_FINE_UP_CMD_MSG,
    RP_COM1_FINE_DOWN_CMD_MSG,
    RP_COM2_COARSE_UP_CMD_MSG,
    RP_COM2_COARSE_DOWN_CMD_MSG,
    RP_COM2_FINE_UP_CMD_MSG,
    RP_COM2_FINE_DOWN_CMD_MSG,
    RP_NAV1_COARSE_UP_CMD_MSG,
    RP_NAV1_COARSE_DOWN_CMD_MSG,
    RP_NAV1_FINE_UP_CMD_MSG,
    RP_NAV1_FINE_DOWN_CMD_MSG,
    RP_NAV2_COARSE_UP_CMD_MSG,
    RP_NAV2_COARSE_DOWN_CMD_MSG,
    RP_NAV2_FINE_UP_CMD_MSG,
    RP_NAV2_FINE_DOWN_CMD_MSG,
    RP_ADF_COARSE_UP_CMD_MSG,
    RP_ADF_COARSE_DOWN_CMD_MSG,
    RP_ADF_FINE_UP_CMD_MSG,
    RP_ADF_FINE_DOWN_CMD_MSG,
    RP_DME_COARSE_UP_CMD_MSG,
    RP_DME_COARSE_DOWN_CMD_MSG,
    RP_DME_FINE_UP_CMD_MSG,
    RP_DME_FINE_DOWN_CMD_MSG,
    RP_XPDR_COARSE_UP_CMD_MSG,
    RP_XPDR_COARSE_DOWN_CMD_MSG,
    RP_XPDR_FINE_UP_CMD_MSG,
    RP_XPDR_FINE_DOWN_CMD_MSG,

    SYS_TIC_MSG,
    G_PLUGIN_ENABLED,
    G_PLUGIN_DISABLED,
    G_EXITING_THREAD_LOOP_MSG     = 0xFFFFFFFF,
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif  /* DEFS_H */

