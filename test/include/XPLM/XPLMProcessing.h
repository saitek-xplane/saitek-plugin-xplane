#ifndef _XPLMProcessing_h_
#define _XPLMProcessing_h_

#include "pasync.h"
#include "XPLMDefs.h"

#define THREADCNT (6)

typedef float (* XPLMFlightLoop_f)(
                                   float                inElapsedSinceLastCall,
                                   float                inElapsedTimeSinceLastFlightLoop,
                                   int                  inCounter,
                                   void *               inRefcon);

class ThreadFactory : public pt::thread {
    protected:
        XPLMFlightLoop_f    cb;
        float               interval;
        unsigned int        sleep;
        void*               refcon;

    public:
        int run;

        ThreadFactory(XPLMFlightLoop_f a, float b, void* c)
                : thread(true), cb(a), interval(b), refcon(c) { run = true; }
        ~ThreadFactory() {}
        void cleanup();
        void execute();
};



#ifdef __cplusplus
extern "C" {
#endif

extern ThreadFactory* threads[THREADCNT];

XPLM_API void XPLMRegisterFlightLoopCallback(
                                   XPLMFlightLoop_f     inFlightLoop,
                                   float                inInterval,
                                   void *               inRefcon);

XPLM_API void XPLMUnregisterFlightLoopCallback(
                                   XPLMFlightLoop_f     inFlightLoop,
                                   void *               inRefcon);




#ifdef __cplusplus
}
#endif

#endif
