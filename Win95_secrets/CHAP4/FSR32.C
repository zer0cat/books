//==================================
// FSR32 - Matt Pietrek 1995
// FILE: FSR32.C
//==================================
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#pragma hdrstop

typedef int (CALLBACK *GFSR_PROC)(int);

// Steal some #define's from the 16 bit WINDOWS.H
#define GFSR_GDIRESOURCES      0x0001
#define GFSR_USERRESOURCES     0x0002

// Prototype some undocumented KERNEL32 functions
HINSTANCE WINAPI LoadLibrary16( PSTR );
void WINAPI FreeLibrary16( HINSTANCE );
FARPROC WINAPI GetProcAddress16( HINSTANCE, PSTR );
void __cdecl QT_Thunk(void);

GFSR_PROC pfnFreeSystemResources = 0;   // We don't want these as locals in
HINSTANCE hInstUser16;                  // main(), since QT_THUNK could
WORD user_fsr, gdi_fsr;                 // trash them...

int main()
{
    char buffer[0x40];

    buffer[0] = 0;  // Make sure to use the local variable so that the
                    // compiler sets up an EBP frame
        
    hInstUser16 = LoadLibrary16("USER.EXE");
    if ( hInstUser16 < (HINSTANCE)32 )
    {
        printf( "LoadLibrary16() failed!\n" );
        return 1;
    }

    FreeLibrary16( hInstUser16 );   // Decrement the reference count

    pfnFreeSystemResources =
        (GFSR_PROC) GetProcAddress16(hInstUser16, "GetFreeSystemResources");
    if ( !pfnFreeSystemResources )
    {
        printf( "GetProcAddress16() failed!\n" );
        return 1;
    }
    
    __asm {
        push    GFSR_USERRESOURCES
        mov     edx, [pfnFreeSystemResources]
        call    QT_Thunk
        mov     [user_fsr], ax

        push    GFSR_GDIRESOURCES
        mov     edx, [pfnFreeSystemResources]
        call    QT_Thunk
        mov     [gdi_fsr], ax
    }

    printf( "USER FSR: %u%%  GDI FSR: %u%%\n", user_fsr, gdi_fsr );

    return 0;
}
