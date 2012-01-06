// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <stdio.h>
#include <stdint.h>

#include "ptypes.h"
#include "pasync.h"

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"

#include "defs.h"
#include "utils.h"
#include "hidapi.h"
#include "PanelThreads.h"
#include "radiopanel.h"

USING_PTYPES


/**
 *
 */
void rp_init(hid_device* hid) {
    LPRINTF("Saitek ProPanels Plugin: rp_init\n");
    uint8_t buf[4];

    hid_set_nonblocking(hid, (int)true);
    hid_read(hid, buf, sizeof(buf));
    hid_send_feature_report(hid, rp_blank_panel, sizeof(rp_blank_panel));
    hid_set_nonblocking(hid, (int)false);
}

