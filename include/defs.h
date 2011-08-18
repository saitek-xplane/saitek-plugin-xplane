// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

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





enum {
    HID_READ_CNT            = 4,
    HID_ERROR               = -1,
    VENDOR_ID               = 0x06A3,
    RP_PROD_ID              = 0x0D05,
    MP_PROD_ID              = 0x0D06,
    SP_PROD_ID              = 0x0D07,
    RP_ERROR_THRESH         = 40,
    MP_ERROR_THRESH         = 40,
    SP_ERROR_THRESH         = 40,
    PANEL_CHECK_INTERVAL    = 3,

    // globsl queue message ids
    // note, pt::MSG_USER = 0 and pt::message id = MSG_USER + 1
    PANEL_ON = 2,
    PANEL_OFF,
    PITCHTRIM_UP,
    PITCHTRIM_DN, // 5
    FLAPS_UP,
    FLAPS_DN,
    BTN_AP_TOGGLE,
    BTN_HDG_TOGGLE,
    BTN_NAV_TOGGLE, // 10
    BTN_IAS_TOGGLE,
    BTN_ALT_TOGGLE,
    BTN_VS_TOGGLE,
    BTN_APR_TOGGLE,
    BTN_REV_TOGGLE, // 15
    KNOB_ALT_POS,
    KNOB_VS_POS,
    KNOB_IAS_POS,
    KNOB_HDG_POS,
    KNOB_CRS_POS, // 20
    AUTOTHROTTLE_OFF,
    AUTOTHROTTLE_ON,

    EXITING_THREAD_LOOP     = 0xFFFFFFFF,
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif  /* DEFS_H */

