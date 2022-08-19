//==================================
// PHYS - Matt Pietrek 1995
// FILE: PHYS32.C
//==================================
#include <windows.h>

BOOL _stdcall phystk_ThunkConnect32(PSTR, PSTR, HINSTANCE, DWORD);

INT WINAPI DllMain
(
    HANDLE	hInst,
    ULONG	dwReason,
    LPVOID	lpReserved
)
{
	if (!(phystk_ThunkConnect32("PHYS16.DLL",   // Name of 16-bit DLL       
								"PHYS32.DLL",   // Name of 32-bit DLL
								hInst,
								dwReason)))
	{
		MessageBox(0, "phystk_ThunkConnect32 failed", 0, MB_OK);
		return FALSE;
	}
    return 1;
}
