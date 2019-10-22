
// w2k_hook.h
// 08-27-2000 Sven B. Schreiber
// sbs@orgon.com

#define UNICODE     // ANSI not supported by this application
#define _USER_MODE_ // ntdef.h and ntddk.h not available

// =================================================================
// PROGRAM IDENTIFICATION
// =================================================================

#define MAIN_BUILD              1
#define MAIN_VERSION_HIGH       1
#define MAIN_VERSION_LOW        0

// -----------------------------------------------------------------

#define MAIN_DAY                27
#define MAIN_MONTH              08
#define MAIN_YEAR               2000

// -----------------------------------------------------------------

#define MAIN_MODULE             w2k_hook
#define MAIN_NAME               SBS Windows 2000 API Hook Viewer
#define MAIN_COMPANY            Sven B. Schreiber
#define MAIN_AUTHOR             Sven B. Schreiber
#define MAIN_EMAIL              sbs@orgon.com
#define MAIN_PREFIX             SBS
#define MAIN_EXE

// =================================================================
// HEADER FILES
// =================================================================

#include <w32start.h>

////////////////////////////////////////////////////////////////////
#ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// MORE HEADER FILES
// =================================================================

#include <w2k_lib.h>
#include <w2k_spy.h>

// =================================================================
// CONSTANTS
// =================================================================

#define HOOK_MAX_DATA           4096 // bytes
#define HOOK_IOCTL_DELAY          10 // msec

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
