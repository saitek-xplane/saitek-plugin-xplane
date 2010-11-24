// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include "overloaded.h"
#include "nedmalloc.h"
#include "multipanel.h"

//        sprintf(data, "%x%x%x%x", buf[0], buf[1], buf[2], buf[3]);
//        a = buf[0];
//        b = buf[1];
//        c = buf[2];
//        d = buf[3];
//        y =  a << 24 || b << 16 || c << 8 || d;


unsigned char* mpProcOutData(unsigned int data) {
    static bool trimup = true;
    static bool trimdown = true;

    unsigned int* msg = NULL;

//    if (trimup_cnt) {
//        if (data & HIDREAD_PITCHTRIM_UP) {
//            trimdown_cnt += 1;
//        } else {
//            msg = (unsigned char*) malloc(sizeof(unsigned int));
//            *((unsigned char*)msg) = HIDREAD_PITCHTRIM_UP;
//            trimup_cnt = false;
//        }
//    } else if (trimdown_cnt) {
//        if (data & HIDREAD_PITCHTRIM_DOWN) {
//            trimdown_cnt += 1;
//        } else {
//            msg = (unsigned char*) malloc(sizeof(unsigned int));
//            *((unsigned char*)msg) = HIDREAD_PITCHTRIM_DOWN;
//            trimdown_cnt = false;
//        }
//    } else {

        if (data & HIDREAD_FLAPSUP) {
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = HIDREAD_FLAPSUP;
        } else if (data & HIDREAD_FLAPSDOWN) {
            msg = (unsigned int*) malloc(sizeof(unsigned int));
            *msg = HIDREAD_FLAPSDOWN;
        } else if (data & HIDREAD_PITCHTRIM_DOWN) {
            if (trimdown) {
                msg = (unsigned int*) malloc(sizeof(unsigned int));
                *msg = HIDREAD_PITCHTRIM_DOWN;
                trimdown = false;
            } else {
                trimdown = true;
            }
        } else if (data & HIDREAD_PITCHTRIM_UP) {
            if (trimup) {
                msg = (unsigned int*) malloc(sizeof(unsigned int));
                *msg = HIDREAD_PITCHTRIM_UP;
                trimup = false;
            } else {
                trimup = true;
            }
        }

//    }


//        switch (y) {
//            case CLOCKWISE:
//                XPLMSpeakString("clockwise \n");
//                break;
//            case COUNTERCLOCKWISE:
//                XPLMSpeakString("counter clockwise \n");
//                break;
//            case HIDREAD_PITCHUP:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
    //        case PITCHNEUTRAL:
    //            XPLMSpeakString("pitch neutral \n");
    //            break;
//            case HIDREAD_PITCHDOWN:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
    //        case ARMSWTH:
    //            XPLMSpeakString("autothrottle arm \n");
    //            break;
    //        case OFFSWTH:
    //            XPLMSpeakString("autothrottle off \n");
    //            break;
//            case HIDREAD_FLAPSUP:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
    //        case FLAPSNEUTRAL:
    //            XPLMSpeakString("flaps neutral \n");
    //            break;
//            case HIDREAD_FLAPSDOWN:
//                msg = (unsigned int*) malloc(sizeof(unsigned int));
//                *msg = y;
//                break;
//            case HDGKNB:
//                XPLMSpeakString("heading switch \n");
//                break;
//            case IASKNB:
//                XPLMSpeakString("inicated airspeed switch \n");
//                break;
//            case VSKNB:
//                XPLMSpeakString("vertical speed  switch \n");
//                break;
//            case ALTKNB:
//                XPLMSpeakString("altitude switch \n");
//                break;
//            case CRSKNB:
//                XPLMSpeakString("course switch \n");
//                break;
    //push buttons
//            case AP:
//                XPLMSpeakString("approach \n");
//                break;
//            case HDG:
//                XPLMSpeakString("heading \n");
//                break;
//            case NAV:
//                XPLMSpeakString("navigation \n");
//                break;
//            case IAS:
//                XPLMSpeakString("indicated ait speed \n");
//                break;
//            case ALT:
//                XPLMSpeakString("altitude \n");
//                break;
//            case VS:
//                XPLMSpeakString("vertical speed \n");
//                break;
//            case APR:
//                XPLMSpeakString("approach \n");
//                break;
//            case REV:
//                XPLMSpeakString("reverse \n");
//                break;
//            default:
//                XPLMSpeakString(data);
//                break;
//        }

    return (unsigned char*)msg;
}
