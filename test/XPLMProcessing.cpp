#include <ctime>
#include <cstdlib>
#include <cassert>

#include "pport.h"
#include "ptypes.h"
#include "pstreams.h"

#include "XPLMProcessing.h"


USING_PTYPES


ThreadFactory* threads[THREADCNT] = { NULL, NULL, NULL, NULL, NULL, NULL };

void
ThreadFactory::cleanup() {

}

void
ThreadFactory::execute() {

    assert(cb && "thread execute: invalid function pointer");

    unsigned int cnt = 0;

    sleep = (unsigned int) (interval * 1000);

    while (run) {
        psleep(sleep);

        cb(0, 0, cnt++, NULL);
    }
pout.putf("Goodbye from ThreadFactory \n");
}

XPLM_API void
XPLMRegisterFlightLoopCallback(XPLMFlightLoop_f inFlightLoop, float inInterval, void* inRefcon) {

    ThreadFactory* f = new ThreadFactory(inFlightLoop, inInterval, inRefcon);

    for (int i = 0; i < THREADCNT; i++) {
        if (!threads[i]) {
            threads[i] = f;
            f->start();
            return;
        }
    }

    assert(false && "thread callback overflow");
}

XPLM_API void
XPLMUnregisterFlightLoopCallback(XPLMFlightLoop_f inFlightLoop, void* inRefcon) {

}


