#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <direct.h>

typedef struct
{
    unsigned address;
    char * pszName;
} _32BIT_SYMBOL, * P32BIT_SYMBOL;

char SzInputVarFile[ MAX_PATH ];
char SzExeFile[ MAX_PATH ];
char SzMapFile[ MAX_PATH ];

#define SYMBOL_REALLOC_SIZE 100

unsigned CSymbols = 0;
P32BIT_SYMBOL P32BitSymbols = 0;

int _32BitSymbolSort( const void *a, const void *b )
{
    return (int) (((P32BIT_SYMBOL)a)->address - ((P32BIT_SYMBOL)b)->address);
}

// When called, the .VAR file should be positioned to the start of the symbols
BOOL ReadAndSort32BitSymbols( FILE *varFile )
{
    char szInputBuffer[512];
    char szSymbolName[256];
    unsigned symAddress;

    while ( !feof(varFile) && fgets(szInputBuffer, sizeof(szInputBuffer), varFile) )
    {
        if ( sscanf( szInputBuffer, "%s = %x", szSymbolName, &symAddress) != 2 )
        {
            printf( "unhandled line: %s\n", szInputBuffer );
            continue;
        }
        
        if ( (CSymbols % SYMBOL_REALLOC_SIZE) == 0 )
        {
            P32BitSymbols = realloc( P32BitSymbols,
                                    (CSymbols + SYMBOL_REALLOC_SIZE) * sizeof(_32BIT_SYMBOL) );
            if ( !P32BitSymbols )
            {
                printf("unable to allocate memory for 32 bit symbols\n");
                return FALSE;
            }
        }
        
        P32BitSymbols[ CSymbols ].address = symAddress;
        P32BitSymbols[ CSymbols ].pszName = strdup( szSymbolName );
        if ( !P32BitSymbols[ CSymbols ].pszName )
        {
            printf("unable to allocate memory for 32 bit symbol names\n");
            return FALSE;
        }
        CSymbols++;
    }
    
    qsort( P32BitSymbols, CSymbols, sizeof(_32BIT_SYMBOL), _32BitSymbolSort );
    
    return TRUE;
}

BOOL ProcessVarFile( void )
{
    FILE * varFile = 0, *exeFile = 0, *mapFile = 0;
    char szInputBuffer[512];
    char szAnotherBuffer[256];
    BOOL _32BitFile = FALSE;
    IMAGE_DOS_HEADER dosHeader;
    IMAGE_NT_HEADERS ntHeaders;
    PIMAGE_SECTION_HEADER pSections = 0;
    unsigned i, sectionNumber, currentSectionBase, nextSectionBase;
    PSTR p;
    
    __try
    {

    varFile = fopen( SzInputVarFile, "rt" );
    
    if ( !varFile )
    {
        printf( "Unable to open %s\n", SzInputVarFile );
        return FALSE;
    }
    
    if ( !fgets(szInputBuffer, sizeof(szInputBuffer), varFile) )
    {
        printf( "Unable to read first line of input file (FILE=)\n");
        return FALSE;
    }
    
    if ( sscanf(szInputBuffer, "FILE = %s", SzExeFile) != 1 )
    {
        printf( "FILE = line not found\n" );
        return FALSE;
    }

    _32BitFile = TRUE;

    exeFile = fopen( SzExeFile, "rb" );
    if ( !exeFile )
    {
        printf( "unable to open %s\n", SzExeFile );
        return FALSE;
    }

    if ( !fread( &dosHeader, sizeof(dosHeader), 1, exeFile) )
    {
        printf( "unable to read DOS header of %s\n", SzExeFile );
        return FALSE;
    }
    
    if ( dosHeader.e_magic != IMAGE_DOS_SIGNATURE )
    {
        printf( "MZ header not found in %s\n", SzExeFile );
        return FALSE;
    }

    ntHeaders.Signature = 0;
    fseek(exeFile, dosHeader.e_lfanew, SEEK_SET);
    fread( &ntHeaders, sizeof(ntHeaders), 1, exeFile );
    if ( ntHeaders.Signature != IMAGE_NT_SIGNATURE )
    {
        printf( "%s is not a valid Win32 PE file\n", SzExeFile );
        return FALSE;
    }

    pSections = malloc( ntHeaders.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER );
    if ( !pSections )
    {
        printf( "unable to allocate memory for sections\n" );
        return FALSE;       
    }

    if ( !fread(pSections, ntHeaders.FileHeader.NumberOfSections,
                IMAGE_SIZEOF_SECTION_HEADER, exeFile) )
    {
        printf( "unable to read sections\n" );
        return FALSE;               
    }
    
    strcpy( SzMapFile, SzExeFile );
    p = strrchr(SzMapFile, '.');
    if ( p )
        strcpy( p+1, "MAP" );
    else
        strcat( SzMapFile, ".MAP");

    mapFile = fopen( SzMapFile, "wt" );
    if ( !exeFile )
    {
        printf( "unable to open %s\n", SzMapFile );
        return FALSE;
    }

    if ( !ReadAndSort32BitSymbols( varFile ) )
        return FALSE;

    fprintf( mapFile, " Start         Length     Name                   Class\n" );

    for ( i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++ )
    {
        fprintf( mapFile,
                " %04X:00000000 0%08X %-23.8s %s 32-bit\n",
                i+1,
                pSections[i].Misc.VirtualSize, 
                pSections[i].Name,
                pSections[i].Characteristics & IMAGE_SCN_MEM_EXECUTE ? "CODE" : "DATA" );
    }

    fprintf(mapFile, "\n  Address         Publics by Value\n\n");

    sectionNumber = 1;
    
    for ( i = 0; i < CSymbols; i++ )
    {
calculateSection:
        if ( sectionNumber > ntHeaders.FileHeader.NumberOfSections )
        {
            printf("%s is above a valid address in this module\n", P32BitSymbols[ i ].pszName);
            break;
        }

        currentSectionBase = ntHeaders.OptionalHeader.ImageBase + pSections[sectionNumber-1].VirtualAddress;
        nextSectionBase = currentSectionBase + pSections[sectionNumber-1].Misc.VirtualSize;

        if ( P32BitSymbols[ i ].address >= nextSectionBase )
        {
            sectionNumber++;
            goto calculateSection;
        }
            
        if ( currentSectionBase > P32BitSymbols[ i ].address )
            printf("%s is below a valid address in this module\n", P32BitSymbols[ i ].pszName);
        
        fprintf( mapFile, " %04X:%08X       %s\n",
            sectionNumber,
            P32BitSymbols[ i ].address - currentSectionBase,
            P32BitSymbols[ i ].pszName );
        
        free( P32BitSymbols[ i ].pszName );
    }
    
    }   // End of __try block
    __finally
    {
    if ( varFile )
        fclose( varFile );
    
    if ( exeFile )
        fclose( exeFile );
    
    if ( mapFile )
        fclose( mapFile );
    
    if ( P32BitSymbols )
        free( P32BitSymbols );
    }
    
    return TRUE;
}

// Returns TRUE if program should continue, FALSE otherwise
BOOL ParseCommandLine(int argc, char *argv[])
{
    if ( argc != 2 )
        return FALSE;
    
    strcpy( SzInputVarFile, argv[1] );

    return TRUE;
}

int main( int argc, char *argv[] )
{
    printf( "VAR2MAP - Matt Pietrek 1995\n" );

    if ( !ParseCommandLine(argc, argv) )
    {
        printf( "Syntax: VAR2MAP filename\n" );
        return 1;
    }
    
    if ( !ProcessVarFile() )
        return 1;

    return 0;
}
