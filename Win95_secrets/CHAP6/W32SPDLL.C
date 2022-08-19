//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SPDLL.C
//==================================
#include <windows.h>
#pragma hdrstop
#include "w32spdll.h"

__declspec(dllimport) int WINAPI VxDCall0(void);

BYTE    lpfnOriginalVxDCall[6];
PBYTE   ppfnOriginalVxDCall;

#define MAX_SAVE_CALLS 16384
WIN32SERVICECALLINFO VxDCalls[MAX_SAVE_CALLS];
DWORD   cLoggedCalls = 0;

BOOL    FHooked = FALSE;

void NewVxDCall_entry(void);

void InitWin32ServiceSpyDLL(void)
{   
    FARPROC pfnVxDCall0;

    if ( (DWORD)NewVxDCall_entry < 0x80000000 )
    {
        MessageBox( 0, "Error! DLL not loaded in shared memory region",
                    0, MB_OK );
        return;
    }
        
    pfnVxDCall0 = VxDCall0;
    cLoggedCalls = 0;

    ppfnOriginalVxDCall = (PBYTE)*(PDWORD)((DWORD)pfnVxDCall0 + 0xA);
    
    __asm {
            // Copy the original FWORD pointer into lpfnOriginalVxDCall
            cli
            mov     esi, [ppfnOriginalVxDCall]
            lea     edi, lpfnOriginalVxDCall
            mov     ECX, 6
            cld
            rep     movsb
            
            // Blow over ppfnOriginalVxDCall with the 16:32 address of 
            // NewVxDCall_entry
            lea     eax, NewVxDCall_entry
            mov     edi, [ppfnOriginalVxDCall]
            stosd
            mov     ax, cs
            stosw
            sti
    }
    
    FHooked = TRUE;
}

void StopWin32ServiceSpy(void)
{
    if ( !FHooked )
        return;

    __asm {
            // Copy the original FWORD pointer into lpfnOriginalVxDCall
            cli
            lea     esi, lpfnOriginalVxDCall
            mov     edi, [ppfnOriginalVxDCall]
            mov     ECX, 6
            cld
            rep     movsb
            sti
    }
}

BOOL GetWin32ServiceLogInfo( PDWORD cCalls, PWIN32SERVICECALLINFO * pCallArray)
{
    *cCalls = cLoggedCalls;
    *pCallArray = VxDCalls;
    
    return TRUE;
}

void GetProcessName( PSTR buffer )
{
    buffer[0] = 0;

    __asm   {
        push    es
        mov     eax, FS:[0Ch]
        mov     es, ax
        cld
        mov     edi, [buffer]
        mov     eax, es:[0f2h]
        mov     [edi], eax      // STOSD requires ES to be setup
        mov     eax, es:[0f6h]
        mov     [edi+4], eax    // STOSD requires ES to be setup
        pop     es
    }
}

void _stdcall LogVxDCall( PDWORD pStackFrame, DWORD serviceId )
{
    if ( cLoggedCalls < MAX_SAVE_CALLS )
    {
        VxDCalls[ cLoggedCalls ].serviceId = serviceId;
        VxDCalls[ cLoggedCalls ].processId = GetCurrentProcessId();
        VxDCalls[ cLoggedCalls ].threadId = GetCurrentThreadId();
        VxDCalls[ cLoggedCalls ].param1 = pStackFrame[3];
        GetProcessName( (PSTR)&VxDCalls[ cLoggedCalls ].szName );
    }
        
    cLoggedCalls++;
}

INT WINAPI DllMain
(
    HANDLE  hInst,
    ULONG   dwReason,
    LPVOID  lpReserved
)
{
    return 1;
}
