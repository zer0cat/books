
// w2k_obj.h
// 08-27-2000 Sven B. Schreiber
// sbs@orgon.com

#define UNICODE // ANSI not supported by this application

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

#define MAIN_MODULE             w2k_obj
#define MAIN_NAME               SBS Windows 2000 Object Browser
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

#include <w2k_call.h>

// =================================================================
// CONSTANTS
// =================================================================

#define OPTION_DEFAULT  0x00000000
#define OPTION_ADDRESS  0x00000001
#define OPTION_TYPE     0x00000002
#define OPTION_FLAGS    0x00000004

// =================================================================
// STRUCTURES
// =================================================================

typedef struct _DIR_LEVEL
    {
    PWORD pwName;
    BOOL  fLastEntry;
    }
    DIR_LEVEL, *PDIR_LEVEL, **PPDIR_LEVEL;

#define DIR_LEVEL_ sizeof (DIR_LEVEL)

// =================================================================
// FUNCTION PROTOTYPES
// =================================================================

DWORD WINAPI DisplayObject (PW2K_OBJECT pObject,
                            DWORD       dOptions,
                            PWORD       pwType,
                            DWORD       dLevel,
                            DWORD       dMaxLevel,
                            PDIR_LEVEL  pLevels,
                            BOOL        fDirectories);

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
