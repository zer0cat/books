//==================================
// PEDUMP - Matt Pietrek 1995
// FILE: DBGDUMP.C
//==================================

#include <windows.h>
#include <stdio.h>
#include "common.h"
#include "extrnvar.h"
#include "dbgdump.h"

void DumpImageDbgHeader(PIMAGE_DBG_HEADER pImageDbgHeader)
{
    UINT headerFieldWidth = 30;

    printf("  %-*s%04X\n", headerFieldWidth, "Machine:",
                pImageDbgHeader->Machine);
    printf("  %-*s%04X\n", headerFieldWidth, "Characteristics:",
                pImageDbgHeader->Characteristics);
    printf("  %-*s%08X\n", headerFieldWidth, "TimeDateStamp:",
                pImageDbgHeader->TimeDateStamp);
    printf("  %-*s%08X\n", headerFieldWidth, "Checksum:",
                pImageDbgHeader->Checksum);
    printf("  %-*s%08X\n", headerFieldWidth, "Size of Image:",
                pImageDbgHeader->SizeOfImage);
    printf("  %-*s%08X\n", headerFieldWidth, "unknown1:",
                pImageDbgHeader->unknown1);
    printf("  %-*s%04X\n", headerFieldWidth, "Number of Sections:",
                pImageDbgHeader->NumberOfSections);
    printf("  %-*s%04X\n", headerFieldWidth, "unknown2:",
                pImageDbgHeader->unknown2);
    printf("  %-*s%08X\n", headerFieldWidth, "DebugDirOffset:",
                pImageDbgHeader->OffsetOfDebugDirectory);
    printf("  %-*s%08X\n", headerFieldWidth, "DebugDirSize:",
                pImageDbgHeader->SizeOfDebugDirectory);
    printf("  %-*s%08X\n", headerFieldWidth, "unknown5:",
                pImageDbgHeader->unknown5);
    printf("  %-*s%08X\n", headerFieldWidth, "unknown6:",
                pImageDbgHeader->unknown6);
    printf("  %-*s%08X\n", headerFieldWidth, "unknown7:",
                pImageDbgHeader->unknown7);
}

void DumpDbgFile( PIMAGE_DBG_HEADER pImageDbgHeader )
{
    DumpImageDbgHeader(pImageDbgHeader);
    printf("\n");
    
    DumpSectionTable( (PIMAGE_SECTION_HEADER)(pImageDbgHeader+1),
                        pImageDbgHeader->NumberOfSections, TRUE);
                    
    DumpDebugDirectory(
        MakePtr(PIMAGE_DEBUG_DIRECTORY,
        pImageDbgHeader, sizeof(IMAGE_DBG_HEADER) +
        (pImageDbgHeader->NumberOfSections * sizeof(IMAGE_SECTION_HEADER))
        + pImageDbgHeader->OffsetOfDebugDirectory),
        pImageDbgHeader->SizeOfDebugDirectory,
        (DWORD)pImageDbgHeader);
    
    printf("\n");
    
    if ( PCOFFDebugInfo )
        DumpCOFFHeader( PCOFFDebugInfo );
    
    printf("\n");

    DumpSymbolTable( MakePtr(PIMAGE_SYMBOL, PCOFFDebugInfo,
                                PCOFFDebugInfo->LvaToFirstSymbol),
                    PCOFFDebugInfo->NumberOfSymbols );
}