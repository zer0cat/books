
// w2k_sym2.h
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

#define MAIN_MODULE             w2k_sym2
#define MAIN_NAME               SBS Windows 2000 Symbol Browser
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
// DISPLAY OPTIONS
// =================================================================

#define DISPLAY_OPTION_DEFAULT      0x00000000

#define DISPLAY_OPTION_ADDRESS      0x00000001
#define DISPLAY_OPTION_NAME_IC      0x00000002
#define DISPLAY_OPTION_NAME         0x00000003
#define DISPLAY_OPTION_SORT         0x0000000F

#define DISPLAY_OPTION_IGNORECASE   0x00010000
#define DISPLAY_OPTION_PREFERRED    0x00020000
#define DISPLAY_OPTION_RELATIVE     0x00040000
#define DISPLAY_OPTION_DECORATED    0x00080000
#define DISPLAY_OPTION_EXPORTED     0x00100000
#define DISPLAY_OPTION_UNDEFINED    0x00200000
#define DISPLAY_OPTION_SPECIAL      0x00400000
#define DISPLAY_OPTION_ZERO         0x00800000

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
