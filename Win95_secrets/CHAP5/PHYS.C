//==================================
// PHYS - Matt Pietrek 1995
// FILE: PHYS.C
//==================================

#include <windows.h>
#include <stdio.h>
#include <conio.h>

DWORD GetPhysicalAddrFromLinear(DWORD linear);  // Prototype helper funcs
WORD __stdcall GetRing0Callgate( DWORD addr, unsigned cParams );
BOOL __stdcall FreeRing0Callgate( WORD callgate );
DWORD _GetPhysicalAddrFromLinear(DWORD linear);     // ASM version
DWORD _GetPageAttributes(DWORD linear);     // ASM version

HMODULE WINAPI LoadLibrary16( LPCSTR lpLibFileName );
BOOL WINAPI FreeLibrary16( HINSTANCE hLibModule );

void ShowPhysicalPages(void);
void CreateSharedMemoryRegion(void);
void DeleteSharedMemoryRegion(void);
void ModifyCodePage(void);
DWORD Get_KRNL386_DGROUP_LinearAddress(void);
PSTR GetPageAttributesAsString(DWORD linear);

BOOL FirstInstance = TRUE;
WORD callgate1 = 0;
WORD callgate2 = 0;
PBYTE PMemMapFileRegion;

#pragma data_seg("SHAREDAT")        // Declare a variable in a shared
int MySharedSectionVariable = 0;    // Section.  The variable must be
#pragma data_seg()                  // initialized for the linker to put it
                                    // in the specified section

int main()
{
    CreateSharedMemoryRegion();

    if ( FirstInstance )
        printf("***** FIRST INSTANCE *****\n");
    else
        printf("***** SECONDARY INSTANCE *****\n");

    ShowPhysicalPages();

    printf("Press any key...\n");
    getch();

    if ( FirstInstance )
    {
        printf("\nNow modifying the code page\n");
        ModifyCodePage();
        ShowPhysicalPages();
    }
    
    FreeRing0Callgate( callgate1 );
    FreeRing0Callgate( callgate2 );

    DeleteSharedMemoryRegion();

    return 0;
}

void ShowPhysicalPages(void)
{
    DWORD linearAddr;
    MEMORY_BASIC_INFORMATION mbi;

    //
    // Get the address of a 16 bit DLL that's below 1MB (KRNL386's DGROUP)
    //
    linearAddr = Get_KRNL386_DGROUP_LinearAddress();
    printf( "KRNL386 DGROUP      - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Get the starting address of the code area.  We'll pass VirtualQuery
    // the address of a routine within the code area.
    //
    VirtualQuery( ShowPhysicalPages, &mbi, sizeof(mbi) );
    linearAddr = (DWORD)mbi.BaseAddress;
    printf( "First code page     - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Get the starting address of the data area.  We'll pass VirtualQuery
    // the address of a global variable within the data area.
    //
    VirtualQuery( &callgate1, &mbi, sizeof(mbi) );
    linearAddr = (DWORD)mbi.BaseAddress;
    printf( "First data page     - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Get the address of a data section with the SHARED attribute
    //
    MySharedSectionVariable = 1;    // Touch it to force it present
    linearAddr = (DWORD)&MySharedSectionVariable;
    printf( "Shared section      - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Get the address of a resource within the module
    //
    linearAddr = (DWORD)
            FindResource(GetModuleHandle(0), MAKEINTATOM(1), RT_STRING);
    printf( "Resources           - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );
    
    //
    // Get the starting address of the process heap area.
    //
    linearAddr = (DWORD)GetProcessHeap();
    printf( "Process Heap        - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Get the starting address of the process environment area.
    //
    VirtualQuery( GetEnvironmentStrings(), &mbi, sizeof(mbi) );
    linearAddr = (DWORD)mbi.BaseAddress;
    printf( "Environment area    - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Get the starting address of the stack area.  We'll pass
    // the address of a stack variable to VirtualQuery
    //
    VirtualQuery( &linearAddr, &mbi, sizeof(mbi) );
    linearAddr = (DWORD)mbi.BaseAddress;
    printf( "Current Stack page  - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Show the address of a memory mapped file
    //
    linearAddr = (DWORD)PMemMapFileRegion;
    printf( "Memory Mapped file  - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );

    //
    // Show the address of a routine in KERNEL32.DLL
    //
    linearAddr = (DWORD)
        GetProcAddress( GetModuleHandle("KERNEL32.DLL"), "VirtualQuery" );
    printf( "KERNEL32.DLL        - Linear:%08X  Physical:%08X  %s\n",
            linearAddr,
            GetPhysicalAddrFromLinear(linearAddr),
            GetPageAttributesAsString(linearAddr) );
}

HANDLE HFileMapping;

void CreateSharedMemoryRegion(void)
{
    BYTE myByte;
    
    HFileMapping = CreateFileMapping( (HANDLE)0xFFFFFFFF,   // File handle
                                    0,                      // security
                                    PAGE_READWRITE,         // protection
                                    0, 0x1000,              // size
                                    "MyFileMapping" );

    // In the above call, we can pass PAGE_WRITECOPY instead of
    // PAGE_READWRITE, but then the subsequent MapViewOfFile will fail

    if ( !HFileMapping )
    {
        printf("Couldn't create file mapping!\n");
        return;
    }

    if ( GetLastError() == ERROR_ALREADY_EXISTS )
        FirstInstance = FALSE;

    PMemMapFileRegion = MapViewOfFile( HFileMapping,            // hMapObject
                                        FILE_MAP_ALL_ACCESS,    // access
                                        0, 0,                   // offset
                                        0 );                    // size
    if ( !PMemMapFileRegion )
    {
        printf("Couldn't map view of file!\n");
        return;
    }
    
    myByte = *PMemMapFileRegion;    // Touch the memory to force it present
}

void DeleteSharedMemoryRegion(void)
{
    if ( PMemMapFileRegion )
        UnmapViewOfFile( PMemMapFileRegion );
    
    if ( HFileMapping )
        CloseHandle( HFileMapping );
}

void Dummy(void)    // Exists solely for us to bash
{
}

void ModifyCodePage(void)
{
    BYTE srcByte = 0xCC;
    DWORD cbWritten;
    
    if ( !WriteProcessMemory(GetCurrentProcess(), Dummy, &srcByte,
                                1, &cbWritten) || (cbWritten != 1) )
        printf("Couldn't write to code page!\n");
}

DWORD Get_KRNL386_DGROUP_LinearAddress(void)
{
    HMODULE hModKRNL386;
    LDT_ENTRY selectorEntry;
    
    hModKRNL386 = LoadLibrary16("KRNL386.EXE");
    if ( hModKRNL386 < (HMODULE)0x20 )
    {
        printf("Couldn't get selector of KRNL386 DGROUP");
        return 0;
    }

    if ( !GetThreadSelectorEntry( GetCurrentThread(), (DWORD)hModKRNL386, 
                                    &selectorEntry ) )
    {
        printf("GetThreadSelectorEntry failed!");
        return 0;
    }

    FreeLibrary16( hModKRNL386 );

    return  (selectorEntry.HighWord.Bytes.BaseHi << 24)
            + (selectorEntry.HighWord.Bytes.BaseMid << 16)
            + selectorEntry.BaseLow;
}

DWORD GetPhysicalAddrFromLinear(DWORD linear)
{
    if ( !callgate1 )
        callgate1 = GetRing0Callgate( (DWORD)_GetPhysicalAddrFromLinear, 1 );
    
    if ( callgate1 )
    {
        WORD myFwordPtr[3];
        
        myFwordPtr[2] = callgate1;
        __asm   push    [linear]
        __asm   cli
        __asm   call    fword ptr [myFwordPtr]
        __asm   sti

        // The return value is in EAX.  The compiler will complain, but...
    }
    else
        return 0xFFFFFFFF;
}

DWORD GetPageAttributes(DWORD linear)
{
    if ( !callgate2 )
        callgate2 = GetRing0Callgate( (DWORD)_GetPageAttributes, 1 );
    
    if ( callgate2 )
    {
        WORD myFwordPtr[3];
        
        myFwordPtr[2] = callgate2;
        __asm   push    [linear]
        __asm   cli
        __asm   call    fword ptr [myFwordPtr]
        __asm   sti

        // The return value is in EAX.  The compiler will complain, but...
    }
    else
        return 0xFFFFFFFF;
}

PSTR GetPageAttributesAsString(DWORD linear)
{
    DWORD attr;
    static char szRetBuffer[128];
    
    attr = GetPageAttributes(linear);
    if ( (attr & 1) == 0  )
        return "not present";
    
    strcpy( szRetBuffer, attr & 2 ? "Read/Write" : "ReadOnly" );
    strcat( szRetBuffer, " " );
    strcat( szRetBuffer, attr & 4 ? "USER" : "SPRVSR" );
    
    return szRetBuffer;
}

