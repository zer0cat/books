//==================================
// APISPY32, SIMONSEZ - Matt Pietrek 1995
// FILE: W32SSUPP.C
//==================================
#include <windows.h>
#include "w32ssupp.h"

typedef DWORD (__stdcall *XPROC)(DWORD);

DWORD GetModuleBaseFromWin32sHMod(HMODULE hMod)
{
    XPROC ImteFromHModule, BaseAddrFromImte;
    HMODULE hModule;
    DWORD imte;
    
    hModule = GetModuleHandle("W32SKRNL.DLL");
    if( !hModule )
        return 0;
    
    ImteFromHModule = (XPROC)GetProcAddress(hModule, "_ImteFromHModule@4");
    if ( !ImteFromHModule )
        return 0;
    
    BaseAddrFromImte = (XPROC)GetProcAddress(hModule, "_BaseAddrFromImte@4");
    if ( !BaseAddrFromImte )
        return 0;

    imte = ImteFromHModule( (DWORD)hMod);
    if ( !imte )
        return 0;
    
    return BaseAddrFromImte(imte);
}
