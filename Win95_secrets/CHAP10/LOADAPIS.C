//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: LOADAPIS.C
//==================================
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "intrcpt.h"
#include "parmtype.h"

BOOL IsNewAPILine(PSTR pszInputLine);
BOOL ParseNewAPILine(PSTR pszInput, PSTR pszDLLName, PSTR pszAPIName);
PARAMTYPE GetParameterEncoding(PSTR pszParam);
PSTR SkipWhitespace(PSTR pszInputLine);

extern HINSTANCE HInstance;

BOOL LoadAPIConfigFile(void)
{
    FILE *pFile;
    char szInput[256];
    BYTE params[33];
    BOOL fBuilding = FALSE;
    char szAPIFunctionFile[MAX_PATH];
    PSTR p;

    // Create a string with the path to the API function file.  This
    // file will be in the same directory as this DLL
    GetModuleFileName(HInstance, szAPIFunctionFile, sizeof(szAPIFunctionFile));
    p = strrchr(szAPIFunctionFile, '\\')+1;
    strcpy(p, "APISPY32.API");

    pFile = fopen(szAPIFunctionFile, "rt");
    if ( !pFile )
        return FALSE;
    
    //
    // Format of a line is moduleName:APIName
    // (e.g., "KERNEL32.DLL:LoadLibraryA")
    //
    while ( fgets(szInput, sizeof(szInput), pFile) )
    {
        PSTR pszNewline, pszInput;
        char szAPIName[128], szDLLName[128];

        pszInput = SkipWhitespace(szInput);
        
        if ( *pszInput == '\n' )    // Go to next line if this line is blank
            continue;
        
        pszNewline = strrchr(pszInput, '\n');   // Look for the newline
        if ( pszNewline )
            *pszNewline = 0;                    // Hack off the newline

        if ( IsNewAPILine(pszInput) )
        {
            // Dispense with the old one we've been building
            if ( fBuilding )
                AddAPIFunction(szDLLName, szAPIName, params);

            if ( ParseNewAPILine(pszInput, szDLLName, szAPIName) ) 
                fBuilding = TRUE;
            else
                fBuilding = FALSE;
            
            params[0] = 0;  // New set of parameters
        }
        else    // A parameter line
        {
            BYTE param = (BYTE)GetParameterEncoding(pszInput);
            if ( param != PARAM_NONE )
            {
                params[ params[0] +1 ] = param; // Add param to end of list
                params[0]++;                    // Update the param count
            }
            else
            {
                if ( (*pszInput != 0) && (stricmp(pszInput, "VOID") != 0) )
                {
                    char errBuff[256];
                    wsprintf(errBuff, "Unknown param %s in %s\r\n",
                            pszInput, szAPIName);
                    OutputDebugString(errBuff);
                }
            }
        }
    }

    fclose( pFile );
    
    return TRUE;
}


// Returns TRUE if this line is the start of an API definition.  It assumes
// that any whitespace has already been skipped over.
BOOL IsNewAPILine(PSTR pszInputLine)
{
    return 0 == strnicmp(pszInputLine, "API:", 4);
}

// Break apart a function definition line into a module name and a function
// name.  Returns those strings in the passed PSTR buffers.
BOOL ParseNewAPILine(PSTR pszInput, PSTR pszDLLName, PSTR pszAPIName)
{
    PSTR pszColonSeparator;

    pszDLLName[0] = pszAPIName[0] = 0;
    
    pszInput += 4;  // Skip over "API:"

    pszColonSeparator = strchr(pszInput, ':');
    if ( !pszColonSeparator )
        return FALSE;
    
    *pszColonSeparator++ = 0;   // Null terminate module name, bump up
                                // pointer to API name

    strcpy(pszDLLName, pszInput);
    strcpy(pszAPIName, pszColonSeparator);
    
    return TRUE;
}


typedef struct tagPARAM_ENCODING
{
    PSTR        pszName;    // Parameter name as it appears in APISPY32.API
    PARAMTYPE   value;      // Associated PARAM_xxx enum from PARMTYPE.H
} PARAM_ENCODING, * PPARAM_ENCODING;

PARAM_ENCODING ParamEncodings[] = 
{
{"DWORD", PARAM_DWORD},
{"WORD", PARAM_WORD},
{"BYTE", PARAM_BYTE},
{"LPSTR", PARAM_LPSTR},
{"LPWSTR", PARAM_LPWSTR},
{"LPDATA", PARAM_LPDATA},
{"HANDLE", PARAM_HANDLE},
{"HWND", PARAM_HWND},
{"BOOL", PARAM_BOOL},
{"LPCODE", PARAM_LPCODE},
};

// Given a line that's possibly a parameter line, returns the PARAM_xxx
// encoding for that parameter type.  Lines that don't match any of the
// strings in the ParamEncodings cause the function to return PARAM_NONE.
PARAMTYPE GetParameterEncoding(PSTR pszParam)
{
    unsigned i;
    PPARAM_ENCODING pParamEncoding = ParamEncodings;
        
    for ( i=0; i < (sizeof(ParamEncodings)/sizeof(PARAM_ENCODING)); i++ )
    {
        if ( stricmp(pParamEncoding->pszName, pszParam) == 0 )
            return pParamEncoding->value;
        
        pParamEncoding++;
    }

    return PARAM_NONE;
}

// Given a pointer to an ASCIIZ string, return a pointer to the first
// non-whitespace character in the line.
PSTR SkipWhitespace(PSTR pszInputLine)
{
    while ( *pszInputLine && isspace(*pszInputLine) )
        pszInputLine++;
    return pszInputLine;
}

