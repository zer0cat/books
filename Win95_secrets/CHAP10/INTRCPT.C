//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: INTRCPT.C
//==================================
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#pragma hdrstop
#include "intrcpt.h"
#include "intrcpt2.h"
#include "log.h"

PAPIFunction BuildAPIStub(PSTR pszModule, PSTR pszFuncName, PBYTE params);

// MakePtr is a macro that allows you to easily add to values (including
// pointers) together without dealing with C's pointer arithmetic.  It
// essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr)+(DWORD)(addValue))

#define MAX_INTERCEPTED_APIS 2048
unsigned InterceptedAPICount = 0;
PAPIFunction InterceptedAPIArray[MAX_INTERCEPTED_APIS];

extern BOOL FChicago;
extern FILE * PLogFile;

BOOL AddAPIFunction
(
    PSTR pszModule,     // exporting DLL name
    PSTR pszFuncName,   // exported function name
    PBYTE params        // opcode encoded parameters of exported function
)
{
    PAPIFunction pNewFunction;

    if ( InterceptedAPICount >= MAX_INTERCEPTED_APIS )
        return FALSE;
    
    pNewFunction = BuildAPIStub(pszModule, pszFuncName, params);
    if ( !pNewFunction )
        return FALSE;
    
    InterceptedAPIArray[ InterceptedAPICount++ ] = pNewFunction;
    
    return TRUE;
}


PAPIFunction BuildAPIStub(PSTR pszModule, PSTR pszFuncName, PBYTE params)
{
    UINT allocSize;
    PAPIFunction pNewFunction;
    PVOID realProcAddress;
    UINT cbFuncName;
    HMODULE hModule;
    
    hModule = GetModuleHandle(pszModule);
    if ( !hModule )
        return 0;
    
    realProcAddress = GetProcAddress( hModule, pszFuncName );
    if ( !realProcAddress )
        return 0;
    
    cbFuncName = strlen(pszFuncName);
    allocSize = sizeof(APIFunction) + cbFuncName +1 + *params + 1;

    pNewFunction = malloc(allocSize);
    if ( !pNewFunction )
        return 0;

    pNewFunction->RealProcAddress = realProcAddress;
    pNewFunction->instr_pushad = 0x60;
    pNewFunction->instr_lea_eax_esp_plus_32 = 0x2024448D;
    pNewFunction->instr_push_eax = 0x50;
    pNewFunction->instr_push_offset_params = 0x68;
    pNewFunction->offset_params = (DWORD)(pNewFunction + 1) + cbFuncName + 1;
    pNewFunction->instr_push_offset_funcName = 0x68;
    pNewFunction->offset_funcName = (DWORD)(pNewFunction + 1);
    pNewFunction->instr_call_LogFunction = 0xE8;
    pNewFunction->offset_LogFunction
        =  (DWORD)LogCall - (DWORD)&pNewFunction->instr_popad;
    pNewFunction->instr_popad = 0x61;
    pNewFunction->instr_jmp_dword_ptr_RealProcAddress = 0x25FF;
    pNewFunction->offset_dword_ptr_RealProcAddrss = (DWORD)pNewFunction;

    strcpy( (PSTR)pNewFunction->offset_funcName, pszFuncName );
    memcpy( (PVOID)pNewFunction->offset_params, params, *params+1 );
    
    return pNewFunction;
}


PAPIFunction LookupInterceptedAPI( PVOID address )
{
    unsigned i;
    PVOID stubAddress;
    
    for ( i=0; i < InterceptedAPICount; i++ )
    {
        if ( InterceptedAPIArray[i]->RealProcAddress == address )
            return InterceptedAPIArray[i];
    }

    // If it's Chicago, and the app is being debugged (as this app is)
    // the loader doesn't fix up the calls to point directly at the
    // DLL's entry point.  Instead, the address in the .idata section
    // points to a PUSH xxxxxxxx / JMP yyyyyyyy stub.  The address in
    // xxxxxxxx points to another stub: PUSH aaaaaaaa / JMP bbbbbbbb.
    // The address in aaaaaaaa is the real address of the function in the
    // DLL.  This ugly code verifies we're looking at this stub setup,
    // and if so, grabs the real DLL entry point, and scans through
    // the InterceptedAPIArray list of addresses again.
    // ***WARNING*** ***WARNING*** ***WARNING*** ***WARNING*** 
    // This code is subject to change, and is current only as of 9/94.

    if ( FChicago )
    {
        
        if ( address < (PVOID)0x80000000 )  // Only shared, system DLLs
            return 0;                       // have stubs
        
        if ( IsBadReadPtr(address, 9) || (*(PBYTE)address != 0x68)
             || (*((PBYTE)address+5) != 0xE9) )
            return 0;

        stubAddress = (PVOID) *(PDWORD)((PBYTE)address+1);

        for ( i=0; i < InterceptedAPICount; i++ )
        {
            PVOID lunacy;
            
            if ( InterceptedAPIArray[i]->RealProcAddress == stubAddress )
                return InterceptedAPIArray[i];
            
            lunacy = InterceptedAPIArray[i]->RealProcAddress;
            
            if ( !IsBadReadPtr(lunacy, 9) && (*(PBYTE)lunacy == 0x68)
                && (*((PBYTE)lunacy+5) == 0xE9) )
            {
                lunacy = (PVOID)*(PDWORD)((PBYTE)lunacy+1);
                if ( lunacy == stubAddress )
                    return InterceptedAPIArray[i];
            }
        }
    }

    return 0;
}

BOOL InterceptFunctionsInModule(PVOID baseAddress)
{
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)baseAddress;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
    
    if ( IsBadReadPtr(baseAddress, sizeof(PIMAGE_NT_HEADERS)) )
        return FALSE;
    
    if ( pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE )
        return FALSE;

    pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
    if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        return FALSE;
    
    pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, baseAddress, 
                            pNTHeader->OptionalHeader.
                            DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
                            VirtualAddress);
                        
    // Bail out if the RVA of the imports section is 0 (it doesn't exist)
    if ( pImportDesc == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader )
        return FALSE;

    while ( pImportDesc->Name )
    {
        PIMAGE_THUNK_DATA pThunk;
        
        pThunk = MakePtr(PIMAGE_THUNK_DATA,
                         baseAddress, pImportDesc->FirstThunk);

        while ( pThunk->u1.Function )
        {
            PAPIFunction pInterceptedFunction;
            
            pInterceptedFunction = LookupInterceptedAPI(pThunk->u1.Function);

            if ( pInterceptedFunction )
            {
                DWORD cBytesMoved;
                DWORD src = (DWORD)&pInterceptedFunction->instr_pushad;
                
                // Bash the import thunk.  We have to use WriteProcessMemory,
                // since the import table may be in a code section (courtesy
                // of the NT 3.51 team!)
                
                WriteProcessMemory( GetCurrentProcess(),
                                &pThunk->u1.Function,
                                &src, sizeof(DWORD), &cBytesMoved );
            }
            
            pThunk++;
        }
        
        pImportDesc++;
    }
    
    return TRUE;
}

