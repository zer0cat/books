//==================================
// SHOWSEH - Matt Pietrek 1995
// FILE: SHOWSEH.C
//==================================
#include <windows.h>
#include <stdio.h>
#pragma hdrstop
#include "tib.h"

void _except_handler3(void);

void DisplaySEHChain(void)
{
    PTIB ptib;
    PSEH_record psehrec;
    
    __asm {
            mov     ax, fs
            mov     es, ax
            mov     eax, 18h
            mov     ebx, es:[eax]
            mov     [ptib], ebx
    }
    
    psehrec = ptib->pvExcept;

    printf("next rec  handler\n"
           "========  ========\n" );

    while ( psehrec != (PSEH_record)0xFFFFFFFF )
    {
        printf("%08X  %08X\n", psehrec->pNext, psehrec->pfnHandler);
        psehrec = psehrec->pNext;
    }

    printf("\n");
}

void c(void)
{
    DWORD   _esp;
    
    __asm   mov     [_esp], esp
    __try
    {
        DisplaySEHChain();
    }
    __except( 1 )
    {
    }
    
    printf("in    c(), ESP = %08X\n", _esp);
}

void b(void)
{
    DWORD   _esp;
    
    __asm   mov     [_esp], esp

    __try
    {
        c();
    }
    __except( 1 )
    {
    }

    printf("in    b(), ESP = %08X\n", _esp);
}

void a(void)
{
    DWORD   _esp;
    
    __asm   mov     [_esp], esp

    __try
    {
        b();
    }
    __except( 1 )
    {
    }
    printf("in    a(), ESP = %08X\n", _esp);
}

int main()
{
    DWORD   _esp;
    
    printf("offset of __except_handler3: %08X\n\n", _except_handler3);

    __asm   mov     [_esp], esp
    
    __try
    {
        a();
        printf("in main(), ESP = %08X\n", _esp);
    }
    __except( 1 )
    {
    }
    
    return 0;
}