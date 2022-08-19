//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32INFO.C
//==================================
#include <windows.h>
#pragma hdrstop
#include "w32srvdb.h"

typedef struct
{
    WORD    ah;
    PSTR    pszName;
} INT21_FUNCTION_ID, *PINT21_FUNCTION_ID;

INT21_FUNCTION_ID Int21Functions[] = {
{ 0x00, "terminate Program" },
{ 0x01, "stdin input with echo" },
{ 0x02, "stdout output" },
{ 0x03, "stdaux input" },
{ 0x04, "stdaux output" },
{ 0x05, "printer output" },
{ 0x06, "direct console I/O" },
{ 0x07, "direct console input without echo" },
{ 0x08, "char input without echo" },
{ 0x09, "write string to stdout" },
{ 0x0A, "buffered keyb input" },
{ 0x0B, "get stdin status" },
{ 0x0C, "flush buffer" },
{ 0x0D, "disk reset" },
{ 0x0E, "set default drive" },
{ 0x0F, "open file using FCB" },
{ 0x10, "close file using FCB" },
{ 0x11, "find first using FCB" },
{ 0x12, "find next using FCB" },
{ 0x13, "delete file using FCB" },
{ 0x14, "read file using FCB" },
{ 0x15, "write file using FCB" },
{ 0x16, "create file using FCB" },
{ 0x17, "rename file using FCB" },
{ 0x18, "null CPM function" },
{ 0x19, "get current drive" },
{ 0x1A, "Set DTA" },
{ 0x1B, "get allocation info for default drive" },
{ 0x1C, "get allocation info for specified drive" },
{ 0x1D, "null CPM function" },
{ 0x1E, "null CPM function" },
{ 0x1F, "get drive parameter block" },
{ 0x20, "null CPM function" },
{ 0x21, "read random using FCB" },
{ 0x22, "write random using FCB" },
{ 0x23, "file size using FCB" },
{ 0x24, "set record number using FCB" },
{ 0x25, "set INT vector" },
{ 0x26, "create new PSP" },
{ 0x27, "random read using FCB" },
{ 0x28, "random write using FCB" },
{ 0x29, "parse filename using FCB" },
{ 0x2A, "get system date" },
{ 0x2B, "set system date" },
{ 0x2C, "get system time" },
{ 0x2D, "set system time" },
{ 0x2E, "set verify flag" },
{ 0x2F, "get DTA" },
{ 0x30, "get DOS version" },
{ 0x31, "terminate and stay resident" },
{ 0x32, "get DOS drive paramter block" },
{ 0x33, "break state" },
{ 0x34, "get INDOS flag" },
{ 0x35, "get INT vector" },
{ 0x36, "get disk free space" },
{ 0x37, "get/set switch char" },
{ 0x38, "get country info" },
{ 0x39, "create directory" },
{ 0x3A, "remove directory" },
{ 0x3B, "change directory" },
{ 0x3C, "create file" },
{ 0x3D, "open existing file" },
{ 0x3E, "close file" },
{ 0x3F, "read file" },
{ 0x40, "write file" },
{ 0x41, "delete file" },
{ 0x42, "set file position" },
{ 0x43, "get/set file attributes" },
{ 0x44, "IOCTL" },
{ 0x45, "dup file handle" },
{ 0x46, "force dup file handle" },
{ 0x47, "get current directory" },
{ 0x48, "allocate memory" },
{ 0x49, "free memory" },
{ 0x4A, "resize memory block" },
{ 0x4B, "EXEC" },
{ 0x4C, "exit process" },
{ 0x4D, "get return code" },
{ 0x4E, "find first file" },
{ 0x4F, "find next file" },
{ 0x50, "set current PSP" },
{ 0x51, "get current PSP" },
{ 0x52, "get list of lists" },
{ 0x53, "translate BIOS parameter block" },
{ 0x54, "get verify flat" },
{ 0x55, "create child PSP" },
{ 0x56, "rename file" },
{ 0x57, "get file date/time" },
{ 0x58, "get/set memory allocation strategy" },
{ 0x59, "get extended error info" },
{ 0x5A, "create temporary file" },
{ 0x5B, "create new file" },
{ 0x5C, "file locking" },
{ 0x5D, "rat's nest" },
{ 0x5E, "network functions" },
{ 0x5F, "enable/disable drive" },
{ 0x60, "get canonical filename" },
{ 0x61, "unused" },
{ 0x62, "get current PSP" },
{ 0x63, "get lead byte table address/other stuff" },
{ 0x64, "set device driver lookahead" },
{ 0x65, "get extended country info" },
{ 0x66, "get/set global page table" },
{ 0x67, "set handle count" },
{ 0x68, "commit file" },
{ 0x69, "get/set disk serial number" },
{ 0x6a, "commit file" },
{ 0x6b, "IFS IOCTL" },
{ 0x6C, "Extended open/create" },
{ 0x6D, "find first ROM program" },
{ 0x6E, "find next ROM program" },
{ 0x6F, "get/set ROM scan start address" },
{ 0x70, "unknown" },
{ 0x71, "LFN"    },
};

#define NUM_DOS_FUNCTIONS (sizeof(Int21Functions)/sizeof(INT21_FUNCTION_ID))

BOOL
GetWin32ServiceName(
    DWORD id,
    DWORD param1,
    PSTR pszBuffer,
    unsigned cbBuffer
)
{
    PWIN32_SERVICE_CALL pW32Service;
    
    pW32Service = LookupWin32ServiceCall( id );
    if ( !pW32Service )
    {
        wsprintf( pszBuffer, "%08X", id );
        return TRUE;
    }
    
    strcpy( pszBuffer, pW32Service->pszServiceName );

    if ( HIWORD(id) == 0x002A )
    {
        if ( LOWORD(id) == 0x10 )
        {
            BYTE ah_code = HIBYTE(LOWORD(param1));
            BYTE al_code = LOBYTE(param1);

            if ( ah_code < NUM_DOS_FUNCTIONS )
            {
                wsprintf( pszBuffer + lstrlen(pszBuffer), " %s",
                            Int21Functions[ah_code].pszName );
            }

            if ( ah_code == 0x71 )
            {
                if ( al_code < NUM_DOS_FUNCTIONS )
                {
                    wsprintf( pszBuffer + lstrlen(pszBuffer), " %s",
                                Int21Functions[al_code].pszName );
                }
                else
                {
                    PSTR p;
                    
                    switch( al_code )
                    {
                        case 0xA0: p = "Get Volume Information"; break;
                        case 0xA1: p = "Find Close"; break;
                        case 0xA6: p = "Get File Info By Handle"; break;
                        case 0xA7: p = "File Time To DOS Time"; break;
                        default: p = 0;
                    }
                    
                    if ( p )
                        wsprintf( pszBuffer + lstrlen(pszBuffer), " %s", p );
                }
            }
        }
    }

    return TRUE;
}
