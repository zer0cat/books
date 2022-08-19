//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: LOG.C
//==================================
#include <windows.h>
#include <stdio.h>
#include "parmtype.h"
#include "return.h"
#include "perthred.h"

// Helper function prototypes
void MakeIndentString(PSTR buffer, UINT level);
void DecodeParamsToString(PBYTE pParams, PDWORD pFrame, PSTR pszParams);
BOOL GetLPSTR( PSTR ptr, PSTR buffer );

FILE *PLogFile = 0;
extern DWORD TlsIndex;          // defined in RETURN.C

BOOL OpenLogFile(void)
{
    char szFilename[MAX_PATH];
    PSTR pszExtension;
    
    GetModuleFileName( GetModuleHandle(0), szFilename, sizeof(szFilename) );
    
    pszExtension = strrchr(szFilename, '.');
    if ( !pszExtension )
        return FALSE;
    
    strcpy(pszExtension, ".out");
        
    PLogFile = fopen(szFilename, "wt");

    return (BOOL)PLogFile;
}


BOOL CloseLogFile(void)
{
    if ( PLogFile )
        fclose( PLogFile );
    return TRUE;
}


void __stdcall LogCall(PSTR pszName, PBYTE pParams, PDWORD pFrame)
{
    char szParams[512];
    char szIndent[128];
    PPER_THREAD_DATA pStack;
    
    if ( !PLogFile )
        return;
    
    DecodeParamsToString(pParams, pFrame, szParams);
    
    pStack = (PPER_THREAD_DATA)TlsGetValue(TlsIndex);
    if ( !pStack )
        return;

    MakeIndentString(szIndent, pStack->FunctionStackPtr);

    fprintf(PLogFile, "%s%s(%s)\n", szIndent, pszName, szParams);
    fflush(PLogFile);
    
    // Patch the return address of this function so that returns to us
    InterceptFunctionReturn(pszName, pFrame);
}

void DecodeParamsToString(PBYTE pParams, PDWORD pFrame, PSTR pszParams)
{
    unsigned i;
    unsigned paramCount;
    unsigned paramShowSize;
    PSTR pszParamName;
    
    pszParams[0] = 0;   // Null out string in case there's no parameters

    paramCount = *pParams++;    // Get number of parameters and advance
                                // to first encoded param
    pFrame++;                   // Bump past the DWORD return address
    
    for ( i=0; i < paramCount; i++ )
    {
        switch ( *pParams )
        {
            case PARAM_DWORD:
                pszParamName = "DWORD"; paramShowSize = 4; break;
            case PARAM_WORD:
                pszParamName = "WORD"; paramShowSize = 2; break;
            case PARAM_BYTE:
                pszParamName = "BYTE"; paramShowSize = 1; break;
            case PARAM_LPSTR:
                pszParamName = "LPSTR"; paramShowSize = 4; break;
            case PARAM_LPWSTR:
                pszParamName = "LPWSTR"; paramShowSize = 4; break;
            case PARAM_LPDATA:
                pszParamName = "LPDATA"; paramShowSize = 4; break;
            case PARAM_HANDLE:
                pszParamName = "HANDLE"; paramShowSize = 4; break;
            case PARAM_HWND:
                pszParamName = "HWND"; paramShowSize = 4; break;
            case PARAM_BOOL:
                pszParamName = "BOOL"; paramShowSize = 4; break;
            case PARAM_LPCODE:
                pszParamName = "LPCODE"; paramShowSize = 4; break;
            default:
                pszParamName = "<unknown>"; paramShowSize = 0;
        }

        pszParams += wsprintf(pszParams, "%s:", pszParamName);
        
        switch ( paramShowSize )
        {
            case 4: pszParamName = "%08X"; break;
            case 2: pszParamName = "%04X"; break;
            case 1: pszParamName = "%02X"; break;
        }

        pszParams += wsprintf(pszParams, pszParamName, *pFrame) ;

        // Tack on the string literal value if it's a PARAM_LPSTR
        if ( *pParams == PARAM_LPSTR )
        {
            char buffer[30];
            
            if ( GetLPSTR( (PSTR)*pFrame, buffer ) )
            {
                strcpy(pszParams, buffer);
                pszParams += strlen(buffer);
            }
        }
        
        if ( (paramCount - i) != 1 )    // Tack on a comma if not last
            *pszParams++ = ',';         // parameter
        
        pFrame++;   // Bump frame up to the next DWORD value
        pParams++;  // advance to next encoded parameter
    }               // End of for() statement
}


BOOL GetLPSTR( PSTR ptr, PSTR buffer )
{
    PSTR p = buffer;
    int i;
    
    *p++ = ':';     // Write out initial -> :" <-
    *p++ = '\"';
        
    for ( i=0; i < 10; i++ )
    {
        if ( !IsBadReadPtr( ptr, 1 ) && *ptr )
        {
            *p = *ptr++;
            if ( *p == '\r' ) { *p++ = '\\'; *p = 'r'; }
            else if ( *p == '\n' ) { *p++ = '\\'; *p = 'n'; }
            else if ( *p == '\t' ) { *p++ = '\\'; *p = 't'; }
            
            p++;
        }
        else
            break;
    }

    if ( i == 0 )   // Not a valid string
        return FALSE;
    
    *p++ = '\"';    // Valid string ptr - end quote and null
    *p++ = 0;       // terminate the string
    
    return TRUE;
}

void LogReturn(PSTR pszName, DWORD returnValue, DWORD level)
{
    char szIndent[128];

    if ( !PLogFile )
        return;
    
    MakeIndentString(szIndent, level);
    fprintf(PLogFile, "%s%s returns: %X\n", szIndent, pszName, returnValue);
    fflush(PLogFile);
}


void MakeIndentString(PSTR buffer, UINT level)
{
    DWORD cBytes = level * 2;
    memset(buffer, ' ', cBytes);
    buffer[cBytes] = 0;
}


