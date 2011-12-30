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

#define IN_BUF_CNT  (4)
#define OUT_BUF_CNT (13)
#define MSG_NOWAIT  (0)
#define MSG_WAIT    (-1)

typedef void (*pHidInit) ();
typedef unsigned char* (*pProcOutData) (unsigned int);

#define READ_KNOB_MODE_MASK    (0x0000001F)
#define READ_BTNS_MASK         (0x00007F80)
#define READ_FLAPS_MASK        (0x00030000)
#define READ_TRIM_MASK         (0x000C0000)
#define READ_TUNING_MASK       (0x00000060)
#define READ_THROTTLE_MASK     (0x00008000)

#define READ_KNOB_ALT          (0x00000001)
#define READ_KNOB_VS           (0x00000002)
#define READ_KNOB_IAS          (0x00000004)
#define READ_KNOB_HDG          (0x00000008)
#define READ_KNOB_CRS          (0x00000010)

#define READ_TUNING_RIGHT      (0x00000020)
#define READ_TUNING_LEFT       (0x00000040)
#define READ_AP_BTN            (0x00000080)
#define READ_HDG_BTN           (0x00000100)
#define READ_NAV_BTN           (0x00000200)
#define READ_IAS_BTN           (0x00000400)
#define READ_ALT_BTN           (0x00000800)
#define READ_VS_BTN            (0x00001000)
#define READ_APR_BTN           (0x00002000)
#define READ_REV_BTN           (0x00004000)

#define READ_THROTTLE_ON       (0x00008000)
#define READ_THROTTLE_OFF      (0x00000000)

#define READ_FLAPS_UP          (0x00010000)
#define READ_FLAPS_DN          (0x00020000)

#define READ_TRIM_DOWN         (0x00040000)
#define READ_TRIM_UP           (0x00080000)
#define READ_NOMSG             (0xFFFFFFFF)

#define READ_SP_MASTER_BAT_MASK       (0x000001)
#define READ_SP_MASTER_ALT_MASK       (0x000002)
#define READ_SP_AVIONICS_MASTER_MASK  (0x000004)
#define READ_SP_FUEL_PUMP_MASK        (0x000008)
#define READ_SP_DE_ICE_MASK           (0x000010)
#define READ_SP_PITOT_HEAT_MASK       (0x000020)
#define READ_SP_COWL_MASK             (0x000040)
#define READ_SP_LIGHTS_PANEL_MASK     (0x000080)
#define READ_SP_LIGHTS_BEACON_MASK    (0x000100)
#define READ_SP_LIGHTS_NAV_MASK       (0x000200)
#define READ_SP_LIGHTS_STROBE_MASK    (0x000400)
#define READ_SP_LIGHTS_TAXI_MASK      (0x000800)
#define READ_SP_LIGHTS_LANDING_MASK   (0x001000)
#define READ_SP_ENGINES_KNOB_MASK     (0x03E000)
#define READ_SP_GEARLEVER_DOWN_MASK   (0x040000)
#define READ_SP_GEARLEVER_UP_MASK     (0x080000)

enum {
    MPM_CNT                 = 3,
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

    // global message ids
    // pt::MSG_USER = 0 and pt::message id = MSG_USER + 1
    AVIONICS_ON = 2,
    AVIONICS_OFF,
    BAT1_ON,
    BAT1_OFF,
    PITCHTRIM_UP,
    PITCHTRIM_DN,
    FLAPS_UP,
    FLAPS_DN,

    ALT_VAL,
    ALT_UP,
    ALT_DN,
    VS_VAL_POS,
    VS_VAL_NEG,
    VS_UP,
    VS_DN,
    IAS_VAL,
    IAS_UP,
    IAS_DN,
    HDG_VAL,
    HDG_UP,
    HDG_DN,
    CRS_VAL,
    CRS_UP,
    CRS_DN,

    BTN_AP_OFF,
    BTN_AP_ARMED,
    BTN_AP_ON,
    BTN_AP_TOGGLE,

    BTN_HDG_OFF,
    BTN_HDG_ARMED,
    BTN_HDG_CAPT,
    BTN_HDG_TOGGLE,

    BTN_NAV_OFF,
    BTN_NAV_ARMED,
    BTN_NAV_CAPT,
    BTN_NAV_TOGGLE,

    BTN_IAS_OFF,
    BTN_IAS_ARMED,
    BTN_IAS_CAPT,
    BTN_IAS_TOGGLE,

    BTN_ALT_OFF,
    BTN_ALT_ARMED,
    BTN_ALT_CAPT,
    BTN_ALT_TOGGLE,

    BTN_VS_OFF,
    BTN_VS_ARMED,
    BTN_VS_CAPT,
    BTN_VS_TOGGLE,

    BTN_APR_OFF,
    BTN_APR_ARMED,
    BTN_APR_CAPT,
    BTN_APR_TOGGLE,

    BTN_REV_OFF,
    BTN_REV_ARMED,
    BTN_REV_CAPT,
    BTN_REV_TOGGLE,

    KNOB_ALT_POS,
    KNOB_VS_POS,
    KNOB_IAS_POS,
    KNOB_HDG_POS,
    KNOB_CRS_POS,
    AUTOTHROTTLE_OFF,
    AUTOTHROTTLE_ON,
    TUNING_RIGHT,
    TUNING_LEFT,

    MP_BLANK_SCRN,
    MP_ZERO_SCRN,

    SP_MAGNETOS_OFF,
    SP_MAGNETOS_RIGHT,
    SP_MAGNETOS_LEFT,
    SP_MAGNETOS_BOTH,
    SP_MAGNETOS_START,
    SP_MASTER_BATTERY_ON,
    SP_MASTER_BATTERY_OFF,
    SP_MASTER_ALT_BATTERY_ON,
    SP_MASTER_ALT_BATTERY_OFF,
    SP_MASTER_AVIONICS_ON,
    SP_MASTER_AVIONICS_OFF,
    SP_FUEL_PUMP_ON,
    SP_FUEL_PUMP_OFF,
    SP_DEICE_ON,
    SP_DEICE_OFF,
    SP_PITOT_HEAT_ON,
    SP_PITOT_HEAT_OFF,
    SP_COWL_CLOSED,
    SP_COWL_OPEN,
    SP_LIGHTS_PANEL_ON,
    SP_LIGHTS_PANEL_OFF,
    SP_LIGHTS_BEACON_ON,
    SP_LIGHTS_BEACON_OFF,
    SP_LIGHTS_NAV_ON,
    SP_LIGHTS_NAV_OFF,
    SP_LIGHTS_STROBE_ON,
    SP_LIGHTS_STROBE_OFF,
    SP_LIGHTS_TAXI_ON,
    SP_LIGHTS_TAXI_OFF,
    SP_LIGHTS_LANDING_ON,
    SP_LIGHTS_LANDING_OFF,
    SP_LANDING_GEAR_UP,
    SP_LANDING_GEAR_DOWN,

    SP_BLANK_SCRN,
    SP_ALL_GREEN_SCRN,

    MPM                     = 0x0FFFFFFF, // multi-part message
    EXITING_THREAD_LOOP     = 0xFFFFFFFF,
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif  /* DEFS_H */

