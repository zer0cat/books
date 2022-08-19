//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: RETURN.C
//==================================
#include <windows.h>
#include <malloc.h>
#include "perthred.h"
#include "return.h"
#include "log.h"

void AsmCommonReturnPoint(void);

DWORD TlsIndex = 0xFFFFFFFF;

BOOL InitThreadReturnStack(void)
{
    PPER_THREAD_DATA pPerThreadData;
    
    static BOOL firstTime = TRUE;
    
    if ( firstTime )
    {
        TlsIndex = TlsAlloc();
        firstTime = FALSE;
    }

    if ( TlsIndex == 0xFFFFFFFF )
        return FALSE;
    
    pPerThreadData = malloc( sizeof(PER_THREAD_DATA) );
    if ( !pPerThreadData )
        return FALSE;
    
    pPerThreadData->FunctionStackPtr = 0;
    
    TlsSetValue(TlsIndex, pPerThreadData);
    
    return TRUE;
}

BOOL ShutdownThreadReturnStack(void)
{
    PPER_THREAD_DATA pPerThreadData;
    
    if ( TlsIndex == 0xFFFFFFFF )
        return FALSE;
    
    pPerThreadData = TlsGetValue( TlsIndex );
    if ( pPerThreadData )
        free( pPerThreadData );
    
    return TRUE;
}

BOOL InterceptFunctionReturn(PSTR pszName, PDWORD pFrame)
{
    PPER_THREAD_DATA pStack;
    DWORD i;
    
    pStack = (PPER_THREAD_DATA)TlsGetValue(TlsIndex);
    if ( !pStack )
        return FALSE;

    if ( pStack->FunctionStackPtr >= (MAX_HOOKED_FUNCTIONS-1) )
        return FALSE;
    
    i = pStack->FunctionStackPtr;
    
    pStack->FunctionStack[i].pfnReturnAddress = (PVOID)pFrame[0];
    pStack->FunctionStack[i].pszName = pszName;
    pStack->FunctionStackPtr++;
    
    pFrame[0] = (DWORD)AsmCommonReturnPoint;
    
    return TRUE;
}

// return_address <- pFrame[8]
// EAX            <- pFrame[7]
// ECX            <- pFrame[6]
// EDX            <- pFrame[5]
// EBX            <- pFrame[4]
// ESP            <- pFrame[3]
// EBP            <- pFrame[2]
// ESI            <- pFrame[1]
// EDI            <- pFrame[0]

//
// Common return point for all functions that we've intercepted.
// Called by _AsmCommonReturnPoint in ASMRETRN.ASM
// pFrame is a pointer to the stack frame set up by the PUSHAD
// (see above comment for the layout of this frame)
//
void CCommonReturnPoint( PDWORD pFrame )
{
    PPER_THREAD_DATA pStack;
    DWORD i;

    // Get the function stack for the current thread
    pStack = (PPER_THREAD_DATA)TlsGetValue(TlsIndex);
    if ( !pStack )
        return;

    i = --pStack->FunctionStackPtr;

    // Emit the information about the function return value to the logging
    // mechanism.
    LogReturn(pStack->FunctionStack[i].pszName, pFrame[7], i);

    // Patch the return address back to what it was when the function
    // was originally called.
    pFrame[8] = (DWORD)pStack->FunctionStack[i].pfnReturnAddress;
}
