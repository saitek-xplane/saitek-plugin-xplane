#include <ctime>
#include <cstdlib>
#include <cassert>
#include <stdio.h>
#include <signal.h>

#include "pport.h"
#include "ptypes.h"
#include "pasync.h"
#include "ptime.h"
#include "pstreams.h"

#include "XPLMProcessing.h"
#include "Saitek.h"
#include "hidapi.h"
#include "nedmalloc.h"
#include "overloaded.h"
#include "main.h"

USING_PTYPES
using namespace std;

static char outName[256];
static char outSig[256];
static char outDesc[256];

static bool gCleaned = false;

void cleanup() {
    if (gCleaned) {
        return;
    }

    gCleaned = true;

    XPluginStop();

    for (int i = 0; i < THREADCNT; i++) {
        ThreadFactory* t = threads[i];

        if (t && !t->get_finished()) {
            pexchange(&(t->run), false);

//            while (true) {
//                if (!t->get_running()) {
//                    continue;
//                }
//            }
        }
    }

    psleep(1000);
}

void signal_handler(int signal)
{
//    struct tm*  ptr = 0;
//    time_t      tm  = time(0);
//    ptr             = localtime(&tm);

    switch(signal)
    {
        case SIGFPE:
            perror("---------------- \n");
//            perror(asctime(ptr));
            perror("A floating point exception occured.\n");
            break;
        case SIGILL:
            perror("---------------- \n");
//            perror(asctime(ptr));
            perror("An illegal instruction occured.\n");
            break;
        case SIGINT:
            // the user hit CTRL-C
            if (!gCleaned) {
                cleanup();
            }
            break;
        case SIGSEGV:
            perror("---------------- \n");
//            perror(asctime(ptr));
            perror("A segmentation violation occured.\n");
            break;
        default:
            perror("---------------- \n");
//            perror(asctime(ptr));
            perror("An unknown signal was caught.\n");
            break;
    }

    // pass a successful exit so our atexit handler is called
    exit(EXIT_SUCCESS);
}

int main() {
pout.putf("-------- Main --------\n");
    atexit(cleanup);

    // siganl handlers
    if(signal(SIGFPE, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGFPE signal handler.\n");

    if(signal(SIGILL, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGILL signal handler.\n");

    if(signal(SIGINT, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGINT signal handler.\n");

    if(signal(SIGSEGV, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGSEGV signal handler.\n");

    XPluginStart(outName, outSig, outDesc);

    pout.putf("XPluginStart - Name: %s, Sig: %s, Desc: %s \n", outName, outSig, outDesc);

    unsigned int cnt = 0;

    while (true) {
        psleep(1000 * 10);
 pout.putf("-------- Calling XPluginDisable --------\n");
        XPluginDisable();

        psleep(1000 * 10);
 pout.putf("-------- Calling XPluginEnable --------\n");
        XPluginEnable();

        cnt++;
    }
}

