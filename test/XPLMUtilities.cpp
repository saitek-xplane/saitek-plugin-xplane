#include <ctime>
#include <cstdlib>
#include <cassert>

#include "pport.h"
#include "ptypes.h"
#include "pstreams.h"

#include "nedmalloc.h"
#include "overloaded.h"

#include "XPLMUtilities.h"


USING_PTYPES


XPLM_API void
XPLMSimulateKeyPress(int inKeyType, int inKey) {

}

XPLM_API void
XPLMSpeakString(const char* inString) {

}

XPLM_API void
XPLMCommandKeyStroke(XPLMCommandKeyID inKey) {

}

XPLM_API void
XPLMCommandButtonPress(XPLMCommandButtonID inButton) {

}

XPLM_API void
XPLMCommandButtonRelease(XPLMCommandButtonID inButton) {

}

//XPLM_API const char*
//XPLMGetVirtualKeyDescription(char inVirtualKey) {

//}

XPLM_API void
XPLMReloadScenery(void) {

}

XPLM_API void
XPLMGetSystemPath(char* outSystemPath) {

}

XPLM_API void
XPLMGetPrefsPath(char* outPrefsPath) {

}

//XPLM_API const char * XPLMGetDirectorySeparator(void) {

//}

//XPLM_API char * XPLMExtractFileAndPath(char* inFullPath) {

//}

//XPLM_API int
//XPLMGetDirectoryContents(const char*         inDirectoryPath,
//                                   long                 inFirstReturn,
//                                   char*               outFileNames,
//                                   long                 inFileNameBufSize,
//                                   char**              outIndices,    /* Can be NULL */
//                                   long                 inIndexCount,
//                                   long*               outTotalFiles,    /* Can be NULL */
//                                   long*               outReturnedFiles) {    /* Can be NULL */
//}

XPLM_API int
XPLMInitialized(void) {

}

XPLM_API void
XPLMGetVersions(int* outXPlaneVersion,
                int* outXPLMVersion,
                XPLMHostApplicationID * outHostID) {

}

//XPLM_API XPLMLanguageCode XPLMGetLanguage(void) {

//}

XPLM_API void
XPLMDebugString(const char* inString) {

}

XPLM_API void
XPLMSetErrorCallback(XPLMError_f inCallback) {

}

//XPLM_API void *
//XPLMFindSymbol(const char* inString) {

//}

//XPLM_API int
//XPLMLoadDataFile(XPLMDataFileType inFileType,
//                 const char* inFilePath) {   /* Can be NULL */
//}

//XPLM_API int XPLMSaveDataFile(XPLMDataFileType inFileType,
//                             const char* inFilePath) {

//}

//XPLM_API XPLMCommandRef
//XPLMFindCommand(const char* inName) {

//}

XPLM_API void
XPLMCommandBegin(XPLMCommandRef inCommand) {

}

XPLM_API void
XPLMCommandEnd(XPLMCommandRef inCommand) {

}

XPLM_API void
XPLMCommandOnce(XPLMCommandRef inCommand) {

}

XPLM_API XPLMCommandRef
XPLMCreateCommand(const char* inName,
                  const char* inDescription) {

}

XPLM_API void
XPLMRegisterCommandHandler(XPLMCommandRef inComand,
                           XPLMCommandCallback_f inHandler,
                            int inBefore,
                           void* inRefcon) {

}

XPLM_API void
XPLMUnregisterCommandHandler(XPLMCommandRef inComand,
                             XPLMCommandCallback_f inHandler,
                             int inBefore,
                             void* inRefcon) {

}




