//==================================
// PHYS - Matt Pietrek 1995
// FILE: PHYS16.C
//==================================

#include <windows.h>
#include "descript.h"

BOOL FAR PASCAL __export DllEntryPoint (DWORD dwReason,
                               WORD  hInst,
                               WORD  wDS,
                               WORD  wHeapSize,
                               DWORD dwReserved1,
                               WORD  wReserved2);

BOOL FAR PASCAL phystk_ThunkConnect16(	LPSTR pszDll16,
										LPSTR pszDll32,
										WORD  hInst,
										DWORD dwReason);

int FAR PASCAL LibMain (HANDLE hInstance,
                        WORD   wDataSeg,
                        WORD   wHeapSize,
                        LPSTR  lpszCmdLine)
{
    if (wHeapSize != 0)   // If DLL data seg is MOVEABLE
        UnlockData (0);

    return (1);
}

BOOL FAR PASCAL __export DllEntryPoint (DWORD dwReason,
                               WORD  hInst,
                               WORD  wDS,
                               WORD  wHeapSize,
                               DWORD dwReserved1,
                               WORD  wReserved2)
{
    OutputDebugString("In 16bit DllEntryPoint: Calling phystk_ThunkConnect16\r\n");
    if (!phystk_ThunkConnect16(	"PHYS16.DLL",
								"PHYS32.DLL",
								hInst,
								dwReason))
    {
        OutputDebugString("In 16bit DllEntryPoint: phystk_ThunkConnect16 ret FALSE\r\n");
        return FALSE;
    }

    OutputDebugString("In 16bit DllEntryPoint: phystk_ThunkConnect16 ret TRUE\r\n");
    return TRUE;
}

static char MS_DOS_STR[] = "MS-DOS";

//
// Return a far pointer to the LDT
//
unsigned short GetLDTAlias(void)
{
    unsigned short  LDT_alias;
    unsigned short  (far * dpmiproc)(void);

    //
    // Use INT 2Fh, fn. 168A to get the "DPMI extensions
    // entry point" function pointer
    //
    _asm     mov     si, offset MS_DOS_STR   // DS:SI = "MS-DOS"
    _asm     mov     ax, 168Ah
    _asm     int     2Fh
    _asm     cmp     al, 8Ah
    _asm     je      extensions_not_found

    //
    // The entry point is returned in ES:DI.  Save it
    //
    _asm     mov     word ptr [dpmiproc], di
    _asm     mov     word ptr [dpmiproc+2], es

    //
    // Call the extensions with AX == 0x100.  The LDT alias
    // selector is return in AX.  Carry flag is set on failure.
    //
    _asm     mov     ax, 100h
    LDT_alias = dpmiproc();
    _asm     jc      extensions_not_found;

    return  LDT_alias;

extensions_not_found:   // We get here if something failed
    return  0;
}

WORD FAR PASCAL __loadds GetRing0Callgate( DWORD func_address,
											unsigned cParams )
{
    CALLGATE_DESCRIPTOR far *callgate_desc;
    CODE_SEG_DESCRIPTOR far *ring0_desc;
    unsigned short ldt_alias;
    unsigned short ring_0_alias;
    unsigned short callgate_selector;

    if ( (ldt_alias = GetLDTAlias()) == 0 )
        return 0;

    //
    // Grab a selector from Windows to use as the CS at ring 0
    //
    if ( !(ring_0_alias = AllocSelector(0)) )
        return 0;

    //
	// Set up the fields in the descriptor to be a ring 0, flat model seg
    //
    ring0_desc = MAKELP( ldt_alias, ring_0_alias & 0xFFF8 );
	ring0_desc->limit_0_15 = 0xFFFF;
	ring0_desc->base_0_15 = 0;
	ring0_desc->base_16_23 = 0;
	ring0_desc->readable = 1;
	ring0_desc->conforming = 0;
	ring0_desc->code_data = 1;
	ring0_desc->app_system = 1;
    ring0_desc->dpl = 0;
    ring0_desc->present = 1;
	ring0_desc->limit_16_19 = 0xF;
	ring0_desc->always_0 = 0;
	ring0_desc->seg_16_32 = 1;
	ring0_desc->granularity = 1;
	ring0_desc->base_24_31 = 0;
	
    //
    // Allocate the selector that'll be used for the call gate
    //
    if ( (callgate_selector= AllocSelector(0)) == 0 )
    {
        FreeSelector( ring_0_alias );
        return 0;
    }

    //
    // Create a pointer to the call gate descriptor
    //
    callgate_desc = MAKELP( ldt_alias, callgate_selector & 0xFFF8 );

    //
    // Fill in the fields of the call gate descriptor with the
    // appropriate values for a 16 bit callgate.
    //
    callgate_desc->offset_0_15 = LOWORD( func_address );
    callgate_desc->selector = ring_0_alias;
    callgate_desc->param_count = cParams;
    callgate_desc->some_bits = 0;
    callgate_desc->type = 0xC;          // 386 call gate
    callgate_desc->app_system = 0;      // A system descriptor
    callgate_desc->dpl = 3;             // Ring 3 code can call
    callgate_desc->present = 1;
    callgate_desc->offset_16_31 = HIWORD(func_address);

    return callgate_selector;
}

BOOL FAR PASCAL __loadds FreeRing0Callgate( WORD callgate )
{
    CALLGATE_DESCRIPTOR far *callgate_desc;
    unsigned short ldt_alias;

    if ( (ldt_alias = GetLDTAlias()) == 0 )
        return FALSE;

    //
    // Create a pointer to the call gate descriptor
    //
    callgate_desc = MAKELP( ldt_alias, callgate & 0xFFF8 );

    //
    // First, free the ring 0 alias selector stored in the LDT
    // call gate descriptor, then free the call gate selector.
    //
    FreeSelector( callgate_desc->selector );
    FreeSelector( callgate );
}

