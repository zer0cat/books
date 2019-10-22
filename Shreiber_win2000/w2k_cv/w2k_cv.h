
// w2k_cv.h
// 08-27-2000 Sven B. Schreiber
// sbs@orgon.com

//#define UNICODE

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

#define MAIN_MODULE             w2k_cv
#define MAIN_NAME               SBS Windows 2000 CodeView Decompiler
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

#include <w2k_img.h>

// =================================================================
// CONSTANTS
// =================================================================

#define OPTION_DEFAULT          0x00000000
#define OPTION_SECTIONS         0x00000001
#define OPTION_EXPORTS          0x00000002
#define OPTION_DIRECTORIES      0x00000004
#define OPTION_CODEVIEW         0x00000008
#define OPTION_PUBSYM           0x00000010
#define OPTION_PUBSYM_EX        0x00000020
#define OPTION_FPO              0x00000040
#define OPTION_MISC             0x00000080
#define OPTION_SOURCE           0x00000100
#define OPTION_TARGET           0x00000200
#define OPTION_ALL              0xFFFFFFFF

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
