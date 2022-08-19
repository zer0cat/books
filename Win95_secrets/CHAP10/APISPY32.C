//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: APISPY32.C
//==================================
#include <windows.h>
#pragma hdrstop
#include "intrcpt.h"
#include "loadapis.h"
#include "log.h"
#include "return.h"
#include "w32ssupp.h"

BOOL InitializeAPISpy32(void);
BOOL ShutDownAPISpy32(void);

HINSTANCE HInstance;
BOOL FChicago = FALSE;

#if defined(__BORLANDC__)
#define DllMain DllEntryPoint
#endif

INT WINAPI DllMain
(
    HANDLE hInst,
    ULONG ul_reason_being_called,
    LPVOID lpReserved
)
{
    // OutputDebugString("In APISPY32.C\r\n");
    
    switch (ul_reason_being_called)
    {
        case DLL_PROCESS_ATTACH:
            HInstance = hInst;
            FChicago = (BOOL)((GetVersion() & 0xC0000000) == 0xC0000000);

            if ( InitializeAPISpy32() == FALSE )
                return 0;
            if ( InitThreadReturnStack() == FALSE )
                return 0;
            break;

        case DLL_THREAD_ATTACH:
            if ( InitThreadReturnStack() == FALSE )
                return 0;
            break;

        case DLL_THREAD_DETACH:
            if ( ShutdownThreadReturnStack() == FALSE )
                return 0;
            break;

        case DLL_PROCESS_DETACH:
            ShutDownAPISpy32();
            
            if ( ShutdownThreadReturnStack() == FALSE )
                return 0;
            break;
    }

    return 1;
}

BOOL InitializeAPISpy32(void)
{
    HMODULE hModExe;
    DWORD moduleBase;

    if ( LoadAPIConfigFile() == FALSE )
        return FALSE;

    if ( OpenLogFile() == FALSE )
        return FALSE;
    
    hModExe = GetModuleHandle(0);
    if ( !hModExe )
        return FALSE;
    
    if ( (GetVersion() & 0xC0000000) == 0x80000000 )    // Win32s???
        moduleBase = GetModuleBaseFromWin32sHMod(hModExe);
    else
        moduleBase = (DWORD)hModExe;
            
    if ( !moduleBase )
        return FALSE;
    
    return InterceptFunctionsInModule( (HMODULE)moduleBase );
}

BOOL ShutDownAPISpy32(void)
{
    CloseLogFile();
    
    return TRUE;
}
