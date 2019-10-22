
// w2k_dump.h
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

#define MAIN_MODULE             w2k_dump
#define MAIN_NAME               SBS Windows 2000 Hex Dump Utility
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

#define DISPLAY_OPTION_STREAM_MASK  PDB_STREAM_MASK
#define DISPLAY_OPTION_BASE         (DISPLAY_OPTION_STREAM_MASK + 1)

#define DISPLAY_OPTION_STREAM       (DISPLAY_OPTION_BASE * 0x01)
#define DISPLAY_OPTION_PDB          (DISPLAY_OPTION_BASE * 0x02)

#define DISPLAY_OPTION_BYTE         (DISPLAY_OPTION_BASE * 0x10)
#define DISPLAY_OPTION_WORD         (DISPLAY_OPTION_BASE * 0x20)
#define DISPLAY_OPTION_DWORD        (DISPLAY_OPTION_BASE * 0x30)
#define DISPLAY_OPTION_QWORD        (DISPLAY_OPTION_BASE * 0x40)
#define DISPLAY_OPTION_FORMAT       (DISPLAY_OPTION_BASE * 0xF0)

// =================================================================
// SIMPLE TYPES
// =================================================================

typedef __int64 QWORD, *PQWORD;

// =================================================================
// STRUCTURES
// =================================================================

typedef struct _DATA_ROW
    {
    PVOID pArguments;
    PWORD pwAddress;
    PWORD apwHex    [16];
    PWORD apwText   [16];
    WORD  awAddress [8+1];
    WORD  awHex     [16*(2+1)];
    WORD  awText    [16*(1+1)];
    WORD  awBuffer  [1024];
    }
    DATA_ROW, *PDATA_ROW, **PPDATA_ROW;

#define DATA_ROW_ sizeof (DATA_ROW)

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
