//==================================
// PEDUMP - Matt Pietrek 1995
// FILE: DBGDUMP.H
//==================================

#ifndef RC_INVOKED
#pragma pack (1)
#endif // !RC_INVOKED

#define IMAGE_DBG_SIGNATURE     0x00004944

typedef struct _IMAGE_DBG_HEADER
{
    DWORD   Signature;
    WORD    Machine;
    WORD    Characteristics;
    DWORD   TimeDateStamp;
    DWORD   Checksum;
    DWORD   SizeOfImage;
    DWORD   unknown1;
    WORD    NumberOfSections;
    WORD    unknown2;
    DWORD   OffsetOfDebugDirectory;     // Relative to end of section table
    DWORD   SizeOfDebugDirectory;       // In bytes
    DWORD   unknown5;
    DWORD   unknown6;
    DWORD   unknown7;
} IMAGE_DBG_HEADER, *PIMAGE_DBG_HEADER;

void DumpDbgFile( PIMAGE_DBG_HEADER pImageDbgHeader );

#ifndef RC_INVOKED
#pragma pack ()
#endif  // !RC_INVOKED
