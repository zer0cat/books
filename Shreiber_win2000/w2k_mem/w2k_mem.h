
// w2k_mem.h
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

#define MAIN_MODULE             w2k_mem
#define MAIN_NAME               SBS Windows 2000 Memory Spy
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
// MACROS
// =================================================================

#define CHAR_UPPER(_a) ((WORD) CharUpperW ((PWORD) (_a)))
#define CHAR_LOWER(_a) ((WORD) CharLowerW ((PWORD) (_a)))

// =================================================================
// COMMAND OPTIONS
// =================================================================

#define COMMAND_OPTION_ON           '+'
#define COMMAND_OPTION_OFF          '-'
#define COMMAND_OPTION_NUMBER       '#'
#define COMMAND_OPTION_LOAD         '/'

#define COMMAND_OPTION_NONE         0x00000000

#define COMMAND_OPTION_BASE         0x000000FF
#define COMMAND_OPTION_TEB          0x00000001
#define COMMAND_OPTION_FS           0x00000002
#define COMMAND_OPTION_USER         0x00000004
#define COMMAND_OPTION_KERNEL       0x00000008
#define COMMAND_OPTION_HANDLE       0x00000010
#define COMMAND_OPTION_ADD          0x00000020
#define COMMAND_OPTION_SUBTRACT     0x00000040
#define COMMAND_OPTION_POINTER      0x00000080

#define COMMAND_OPTION_ADDRESS      0x00000F00
#define COMMAND_OPTION_ZERO         0x00000100
#define COMMAND_OPTION_RAM          0x00000200

#define COMMAND_OPTION_MODE         0x000FF000
#define COMMAND_OPTION_WORD         0x00001000
#define COMMAND_OPTION_DWORD        0x00002000
#define COMMAND_OPTION_QWORD        0x00004000

#define COMMAND_OPTION_INFO         0x0FF00000
#define COMMAND_OPTION_OS           0x00100000
#define COMMAND_OPTION_CPU          0x00200000
#define COMMAND_OPTION_GDT          0x00400000
#define COMMAND_OPTION_IDT          0x00800000
#define COMMAND_OPTION_BLOCKS       0x01000000

#define COMMAND_OPTION_OTHER        0xF0000000
#define COMMAND_OPTION_EXECUTE      0x10000000

#define DISPLAY_SIZE_DEFAULT        256

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
