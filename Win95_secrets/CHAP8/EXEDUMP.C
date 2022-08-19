//==================================
// PEDUMP - Matt Pietrek 1995
// FILE: EXEDUMP.C
//==================================

#include <windows.h>
#include <stdio.h>
#pragma hdrstop
#include "common.h"
#include "extrnvar.h"

void DumpExeDebugDirectory(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_DEBUG_DIRECTORY debugDir;
    PIMAGE_SECTION_HEADER header;
    DWORD offsetInto_rdata;
    DWORD va_debug_dir;
    DWORD size;
    
    // This line was so long that we had to break it up
    va_debug_dir = pNTHeader->OptionalHeader.
                        DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].
                        VirtualAddress;
    if ( va_debug_dir == 0 )
        return;

    // If we found a .debug section, and the debug directory is at the
    // beginning of this section, it looks like a Borland file
    header = GetSectionHeader(".debug", pNTHeader);
    if ( header && (header->VirtualAddress == va_debug_dir) )
    {
        debugDir = (PIMAGE_DEBUG_DIRECTORY)(header->PointerToRawData+base);
        size = pNTHeader->OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size *
                sizeof(IMAGE_DEBUG_DIRECTORY);
    }
    else    // Look for microsoft debug directory in the .rdata section
    {
        header = GetSectionHeader(".rdata", pNTHeader);
        if ( !header )
            return;

        size = pNTHeader->OptionalHeader.
                        DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    
        offsetInto_rdata = va_debug_dir - header->VirtualAddress;
        debugDir = MakePtr(PIMAGE_DEBUG_DIRECTORY, base,
                            header->PointerToRawData + offsetInto_rdata);
    }

    DumpDebugDirectory( debugDir, size, base );
}

// Function prototype (necessary because two functions recurse)
void DumpResourceDirectory
(
    PIMAGE_RESOURCE_DIRECTORY resDir, DWORD resourceBase,
    DWORD level, DWORD resourceType
);

// The predefined resource types
char *SzResourceTypes[] = {
"???_0", "CURSOR", "BITMAP", "ICON", "MENU", "DIALOG", "STRING", "FONTDIR",
"FONT", "ACCELERATORS", "RCDATA", "MESSAGETABLE", "GROUP_CURSOR",
"???_13", "GROUP_ICON", "???_15", "VERSION"
};

// Get an ASCII string representing a resource type
void GetResourceTypeName(DWORD type, PSTR buffer, UINT cBytes)
{
    if ( type <= 16 )
        strncpy(buffer, SzResourceTypes[type], cBytes);
    else
        sprintf(buffer, "%X", type);
}

//
// If a resource entry has a string name (rather than an ID), go find
// the string and convert it from unicode to ascii.
//
void GetResourceNameFromId
(
    DWORD id, DWORD resourceBase, PSTR buffer, UINT cBytes
)
{
    PIMAGE_RESOURCE_DIR_STRING_U prdsu;

    // If it's a regular ID, just format it.
    if ( !(id & IMAGE_RESOURCE_NAME_IS_STRING) )
    {
        sprintf(buffer, "%X", id);
        return;
    }
    
    id &= 0x7FFFFFFF;
    prdsu = (PIMAGE_RESOURCE_DIR_STRING_U)(resourceBase + id);

    // prdsu->Length is the number of unicode characters
    WideCharToMultiByte(CP_ACP, 0, prdsu->NameString, prdsu->Length,
                        buffer, cBytes, 0, 0);
    buffer[ min(cBytes-1,prdsu->Length) ] = 0;  // Null terminate it!!!
}

//
// Dump the information about one resource directory entry.  If the
// entry is for a subdirectory, call the directory dumping routine
// instead of printing information in this routine.
//
void DumpResourceEntry
(
    PIMAGE_RESOURCE_DIRECTORY_ENTRY resDirEntry,
    DWORD resourceBase,
    DWORD level
)
{
    UINT i;
    char nameBuffer[128];
    PIMAGE_RESOURCE_DATA_ENTRY pResDataEntry;
    
    if ( resDirEntry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY )
    {
        DumpResourceDirectory( (PIMAGE_RESOURCE_DIRECTORY)
            ((resDirEntry->OffsetToData & 0x7FFFFFFF) + resourceBase),
            resourceBase, level, resDirEntry->Name);
        return;
    }

    // Spit out the spacing for the level indentation
    for ( i=0; i < level; i++ )
        printf("    ");

    if ( resDirEntry->Name & IMAGE_RESOURCE_NAME_IS_STRING )
    {
        GetResourceNameFromId(resDirEntry->Name, resourceBase, nameBuffer,
                              sizeof(nameBuffer));
        printf("Name: %s  DataEntryOffs: %08X\n",
            nameBuffer, resDirEntry->OffsetToData);
    }
    else
    {
        printf("ID: %08X  DataEntryOffs: %08X\n",
                resDirEntry->Name, resDirEntry->OffsetToData);
    }
    
    // the resDirEntry->OffsetToData is a pointer to an
    // IMAGE_RESOURCE_DATA_ENTRY.  Go dump out that information.  First,
    // spit out the proper indentation
    for ( i=0; i < level; i++ )
        printf("    ");
    
    pResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                    (resourceBase + resDirEntry->OffsetToData);
    printf("Offset: %05X  Size: %05X  CodePage: %X\n",
            pResDataEntry->OffsetToData, pResDataEntry->Size,
            pResDataEntry->CodePage);
}

//
// Dump the information about one resource directory.
//
void DumpResourceDirectory
(
    PIMAGE_RESOURCE_DIRECTORY resDir,
    DWORD resourceBase,
    DWORD level,
    DWORD resourceType
)
{
    PIMAGE_RESOURCE_DIRECTORY_ENTRY resDirEntry;
    char szType[64];
    UINT i;

    // Spit out the spacing for the level indentation
    for ( i=0; i < level; i++ )
        printf("    ");

    // Level 1 resources are the resource types
    if ( level == 1 && !(resourceType & IMAGE_RESOURCE_NAME_IS_STRING) )
    {
        GetResourceTypeName( resourceType, szType, sizeof(szType) );
    }
    else    // Just print out the regular id or name
    {
        GetResourceNameFromId( resourceType, resourceBase, szType,
                               sizeof(szType) );
    }
    
    printf(
        "ResDir (%s) Named:%02X ID:%02X TimeDate:%08X Vers:%u.%02u Char:%X\n",
        szType, resDir->NumberOfNamedEntries, resDir->NumberOfIdEntries,
        resDir->TimeDateStamp, resDir->MajorVersion,
        resDir->MinorVersion,resDir->Characteristics);

    resDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(resDir+1);
    
    for ( i=0; i < resDir->NumberOfNamedEntries; i++, resDirEntry++ )
        DumpResourceEntry(resDirEntry, resourceBase, level+1);

    for ( i=0; i < resDir->NumberOfIdEntries; i++, resDirEntry++ )
        DumpResourceEntry(resDirEntry, resourceBase, level+1);
}

//
// Top level routine called to dump out the entire resource hierarchy
//
void DumpResourceSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_RESOURCE_DIRECTORY resDir;
    
    resDir = GetSectionPtr(".rsrc", pNTHeader, (DWORD)base);
    if ( !resDir )
        return;
    
    printf("Resources\n");
    DumpResourceDirectory(resDir, (DWORD)resDir, 0, 0);
}

//
// Dump the imports table (the .idata section) of a PE file
//
void DumpImportsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_IMPORT_DESCRIPTOR importDesc;
    PIMAGE_SECTION_HEADER pSection;
    PIMAGE_THUNK_DATA thunk, thunkIAT=0;
    PIMAGE_IMPORT_BY_NAME pOrdinalName;
    DWORD importsStartRVA;
    INT delta = -1;

    // Look up where the imports section is (normally in the .idata section)
    // but not necessarily so.  Therefore, grab the RVA from the data dir.
    importsStartRVA = pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if ( !importsStartRVA )
        return;

    // Get the IMAGE_SECTION_HEADER that contains the imports.  This is
    // usually the .idata section, but doesn't have to be.
    pSection = GetEnclosingSectionHeader( importsStartRVA, pNTHeader );
    if ( !pSection )
        return;

    delta = (INT)(pSection->VirtualAddress-pSection->PointerToRawData);
    
    importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (importsStartRVA - delta + base);
            
    printf("Imports Table:\n");
    
    while ( 1 )
    {
        // See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
        if ( (importDesc->TimeDateStamp==0 ) && (importDesc->Name==0) )
            break;
        
        printf("  %s\n", (PBYTE)(importDesc->Name) - delta + base);

        printf("  Hint/Name Table: %08X\n", importDesc->Characteristics);
        printf("  TimeDateStamp:   %08X\n", importDesc->TimeDateStamp);
        printf("  ForwarderChain:  %08X\n", importDesc->ForwarderChain);
        printf("  First thunk RVA: %08X\n", importDesc->FirstThunk);
    
        thunk = (PIMAGE_THUNK_DATA)importDesc->Characteristics;
        thunkIAT = (PIMAGE_THUNK_DATA)importDesc->FirstThunk;

        if ( thunk == 0 )   // No Characteristics field?
        {
            // Yes! Gotta have a non-zero FirstThunk field then.
            thunk = thunkIAT;
            
            if ( thunk == 0 )   // No FirstThunk field?  Ooops!!!
                return;
        }
        
        // Adjust the pointer to point where the tables are in the
        // mem mapped file.
        thunk = (PIMAGE_THUNK_DATA)( (PBYTE)thunk - delta + base);
        thunkIAT = (PIMAGE_THUNK_DATA)( (PBYTE)thunkIAT - delta + base);
    
        printf("  Ordn  Name\n");
        
        while ( 1 ) // Loop forever (or until we break out)
        {
            if ( thunk->u1.AddressOfData == 0 )
                break;

            if ( thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG )
            {
                printf( "  %4u", IMAGE_ORDINAL(thunk->u1.Ordinal) );
            }
            else
            {
                pOrdinalName = thunk->u1.AddressOfData;
                pOrdinalName = (PIMAGE_IMPORT_BY_NAME)
                                ((PBYTE)pOrdinalName - delta + base);
                    
                printf("  %4u  %s", pOrdinalName->Hint, pOrdinalName->Name);
            }
            

            if ( fShowIATentries )
                printf( " (IAT: %08X)", thunkIAT->u1.Function );

            printf( "\n" );

            thunk++;            // Advance to next thunk
            thunkIAT++;         // advance to next thunk
        }

        importDesc++;   // advance to next IMAGE_IMPORT_DESCRIPTOR
        printf("\n");
    }
}

//
// Dump the exports table (usually the .edata section) of a PE file
//
void DumpExportsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_EXPORT_DIRECTORY exportDir;
    PIMAGE_SECTION_HEADER header;
    INT delta; 
    PSTR filename;
    DWORD i;
    PDWORD functions;
    PWORD ordinals;
    PSTR *name;
    DWORD exportsStartRVA, exportsEndRVA;
    
    exportsStartRVA = pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    exportsEndRVA = exportsStartRVA + pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

    // Get the IMAGE_SECTION_HEADER that contains the exports.  This is
    // usually the .edata section, but doesn't have to be.
    header = GetEnclosingSectionHeader( exportsStartRVA, pNTHeader );
    if ( !header )
        return;

    delta = (INT)(header->VirtualAddress - header->PointerToRawData);
        
    exportDir = MakePtr(PIMAGE_EXPORT_DIRECTORY, base,
                         exportsStartRVA - delta);
        
    filename = (PSTR)(exportDir->Name - delta + base);
        
    printf("exports table:\n\n");
    printf("  Name:            %s\n", filename);
    printf("  Characteristics: %08X\n", exportDir->Characteristics);
    printf("  TimeDateStamp:   %08X\n", exportDir->TimeDateStamp);
    printf("  Version:         %u.%02u\n", exportDir->MajorVersion,
            exportDir->MinorVersion);
    printf("  Ordinal base:    %08X\n", exportDir->Base);
    printf("  # of functions:  %08X\n", exportDir->NumberOfFunctions);
    printf("  # of Names:      %08X\n", exportDir->NumberOfNames);
    
    functions = (PDWORD)((DWORD)exportDir->AddressOfFunctions - delta + base);
    ordinals = (PWORD)((DWORD)exportDir->AddressOfNameOrdinals - delta + base);
    name = (PSTR *)((DWORD)exportDir->AddressOfNames - delta + base);

    printf("\n  Entry Pt  Ordn  Name\n");
    for ( i=0; i < exportDir->NumberOfFunctions; i++ )
    {
        DWORD entryPointRVA = functions[i];
        DWORD j;

        if ( entryPointRVA == 0 )   // Skip over gaps in exported function
            continue;               // ordinals (the entrypoint is 0 for
                                    // these functions).

        printf("  %08X  %4u", entryPointRVA, i + exportDir->Base );

        // See if this function has an associated name exported for it.
        for ( j=0; j < exportDir->NumberOfNames; j++ )
            if ( ordinals[j] == i )
                printf("  %s", name[j] - delta + base);

        // Is it a forwarder?  If so, the entry point RVA is inside the
        // .edata section, and is an RVA to the DllName.EntryPointName
        if ( (entryPointRVA >= exportsStartRVA)
             && (entryPointRVA <= exportsEndRVA) )
        {
            printf(" (forwarder -> %s)", entryPointRVA - delta + base );
        }
        
        printf("\n");
    }
}

// The names of the available base relocations
char *SzRelocTypes[] = {
"ABSOLUTE","HIGH","LOW","HIGHLOW","HIGHADJ","MIPS_JMPADDR",
"I860_BRADDR","I860_SPLIT" };

//
// Dump the base relocation table of a PE file
//
void DumpBaseRelocationsSection(DWORD base, PIMAGE_NT_HEADERS pNTHeader)
{
    PIMAGE_BASE_RELOCATION baseReloc;
    
    baseReloc = GetSectionPtr(".reloc", pNTHeader, base);
    if ( !baseReloc )
        return;

    printf("base relocations:\n\n");

    while ( baseReloc->SizeOfBlock != 0 )
    {
        unsigned i,cEntries;
        PWORD pEntry;
        char *szRelocType;
        WORD relocType;
        
        cEntries = (baseReloc->SizeOfBlock-sizeof(*baseReloc))/sizeof(WORD);
        pEntry = MakePtr( PWORD, baseReloc, sizeof(*baseReloc) );
        
        printf("Virtual Address: %08X  size: %08X\n",
                baseReloc->VirtualAddress, baseReloc->SizeOfBlock);
            
        for ( i=0; i < cEntries; i++ )
        {
            // Extract the top 4 bits of the relocation entry.  Turn those 4
            // bits into an appropriate descriptive string (szRelocType)
            relocType = (*pEntry & 0xF000) >> 12;
            szRelocType = relocType < 8 ? SzRelocTypes[relocType] : "unknown";
            
            printf("  %08X %s\n",
                    (*pEntry & 0x0FFF) + baseReloc->VirtualAddress,
                    szRelocType);
            pEntry++;   // Advance to next relocation entry
        }
        
        baseReloc = MakePtr( PIMAGE_BASE_RELOCATION, baseReloc,
                             baseReloc->SizeOfBlock);
    }
}

//
// Dump out the new IMAGE_BOUND_IMPORT_DESCRIPTOR that NT 3.51 added
//
void DumpBoundImportDescriptors( DWORD base, PIMAGE_NT_HEADERS pNTHeader )
{
    DWORD bidRVA;   // Bound import descriptors RVA
    PIMAGE_BOUND_IMPORT_DESCRIPTOR pibid;

    bidRVA = pNTHeader->OptionalHeader.DataDirectory
                        [IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;
    if ( !bidRVA )
        return;
    
    pibid = MakePtr( PIMAGE_BOUND_IMPORT_DESCRIPTOR, base, bidRVA );
    
    printf( "Bound import descriptors:\n\n" );
    printf( "  Module        TimeDate\n" );
    printf( "  ------------  --------\n" );
    
    while ( pibid->TimeDateStamp )
    {
        unsigned i;
        PIMAGE_BOUND_FORWARDER_REF pibfr;
        
        printf( "  %-12s  %08X\n", base + bidRVA + pibid->OffsetModuleName,
                                  pibid->TimeDateStamp );
                            
        pibfr = MakePtr(PIMAGE_BOUND_FORWARDER_REF, pibid,
                            sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR));

        for ( i=0; i < pibid->NumberOfModuleForwarderRefs; i++ )
        {
            printf("    forwarder:  %-12s  %08X\n", 
                            base + bidRVA + pibfr->OffsetModuleName,
                            pibfr->TimeDateStamp );
            pibfr++;    // advance to next forwarder ref
                
            // Keep the outer loop pointer up to date too!
            pibid = MakePtr( PIMAGE_BOUND_IMPORT_DESCRIPTOR, pibid,
                             sizeof( IMAGE_BOUND_FORWARDER_REF ) );
        }

        pibid++;    // Advance to next pibid;
    }
}

//
// top level routine called from PEDUMP.C to dump the components of a PE file
//
void DumpExeFile( PIMAGE_DOS_HEADER dosHeader )
{
    PIMAGE_NT_HEADERS pNTHeader;
    DWORD base = (DWORD)dosHeader;
    
    pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader,
                                dosHeader->e_lfanew );

    // First, verify that the e_lfanew field gave us a reasonable
    // pointer, then verify the PE signature.
    __try
    {
        if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        {
            printf("Not a Portable Executable (PE) EXE\n");
            return;
        }
    }
    __except( TRUE )    // Should only get here if pNTHeader (above) is bogus
    {
        printf( "invalid .EXE\n");
        return;
    }
    
    DumpHeader((PIMAGE_FILE_HEADER)&pNTHeader->FileHeader);
    printf("\n");

    DumpOptionalHeader((PIMAGE_OPTIONAL_HEADER)&pNTHeader->OptionalHeader);
    printf("\n");

    DumpSectionTable( IMAGE_FIRST_SECTION(pNTHeader), 
                        pNTHeader->FileHeader.NumberOfSections, TRUE);
    printf("\n");

    DumpExeDebugDirectory(base, pNTHeader);
    if ( pNTHeader->FileHeader.PointerToSymbolTable == 0 )
        PCOFFDebugInfo = 0; // Doesn't really exist!
    printf("\n");

    DumpResourceSection(base, pNTHeader);
    printf("\n");

    DumpImportsSection(base, pNTHeader);
    printf("\n");
    
    if ( pNTHeader->OptionalHeader.DataDirectory
                            [IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT]
                            .VirtualAddress )
    {
        DumpBoundImportDescriptors( base, pNTHeader );
        printf( "\n" );
    }
    
    DumpExportsSection(base, pNTHeader);
    printf("\n");

    if ( fShowRelocations )
    {
        DumpBaseRelocationsSection(base, pNTHeader);
        printf("\n");
    } 

    //
    // Initialize these vars here since we'll need them in DumpLineNumbers
    //
    PCOFFSymbolTable = MakePtr(PIMAGE_SYMBOL, base,
                        pNTHeader->FileHeader.PointerToSymbolTable);
    COFFSymbolCount = pNTHeader->FileHeader.NumberOfSymbols;

    if ( fShowSymbolTable && PCOFFDebugInfo )
    {
        DumpCOFFHeader( PCOFFDebugInfo );
        printf("\n");
    }
    
    if ( fShowLineNumbers && PCOFFDebugInfo )
    {
        DumpLineNumbers( MakePtr(PIMAGE_LINENUMBER, PCOFFDebugInfo,
                            PCOFFDebugInfo->LvaToFirstLinenumber),
                            PCOFFDebugInfo->NumberOfLinenumbers);
        printf("\n");
    }

    if ( fShowSymbolTable )
    {
        if ( pNTHeader->FileHeader.NumberOfSymbols 
            && pNTHeader->FileHeader.PointerToSymbolTable)
        {
            DumpSymbolTable(PCOFFSymbolTable, COFFSymbolCount);
            printf("\n");
        }
    }
    
    if ( fShowRawSectionData )
    {
        DumpRawSectionData( (PIMAGE_SECTION_HEADER)(pNTHeader+1),
                            dosHeader,
                            pNTHeader->FileHeader.NumberOfSections);
    }
}
