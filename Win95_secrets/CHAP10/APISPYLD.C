//==================================
// APISPYLD - Matt Pietrek 1995
// FILE: APISPYLD.C
//==================================
#include <windows.h>
#include <stddef.h>
#pragma hdrstop
#include "apispyld.h"

//======================== Global Variables =================================
char SzINISection[] = "Options";
char SzINICmdLineKey[] = "CommandLine";
char SzINIFile[] = "APISPY32.INI";
char SzCmdLine[MAX_PATH];

BOOL FFirstBreakpointHit = FALSE, FSecondBreakpointHit = FALSE;

PROCESS_INFORMATION ProcessInformation;
CREATE_PROCESS_DEBUG_INFO ProcessDebugInfo;

CONTEXT OriginalThreadContext, FakeLoadLibraryContext;
PVOID PInjectionPage;

#define PAGE_SIZE 4096
BYTE OriginalCodePage[PAGE_SIZE];
BYTE NewCodePage[PAGE_SIZE];

//======================== Code =============================================

//
// Function prototypes
//
BOOL  CALLBACK APISPY32DlgProc(HWND, UINT, WPARAM, LPARAM);
void  Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
void  Handle_WM_INITDIALOG(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
BOOL  GetProgramName(HWND hWndOwner, PSTR szFile, unsigned nFileBuffSize);
BOOL  LoadProcessForSpying(PSTR SzCmdLine);
void  DebugLoop(void);
DWORD HandleDebugEvent( DEBUG_EVENT * event );
void  HandleException(LPDEBUG_EVENT lpEvent, PDWORD continueStatus);
void  EmptyMsgQueueOfUselessMessages(void);
BOOL  InjectSpyDll(void);
BOOL  ReplaceOriginalPagesAndContext(void);
PVOID FindUsablePage(HANDLE hProcess, PVOID PProcessBase);
BOOL  GetSpyDllName(PSTR buffer, UINT cBytes);


int APIENTRY WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                        LPSTR lpszCmdLine, int nCmdShow )
{
    // This dialog returns 0 if the user pressed cancel
    while ( 0 != DialogBox(hInstance, "APISPY32_LOAD_DLG", 0,
                            (DLGPROC)APISPY32DlgProc) )
    {
        if ( LoadProcessForSpying(SzCmdLine) )
        {
            DebugLoop();
            break;
        }

        MessageBox(0, "Unable to start program", 0, MB_OK);
    }
    
    return 0;
}

BOOL CALLBACK APISPY32DlgProc(HWND hWndDlg, UINT msg,
                              WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_COMMAND:
            Handle_WM_COMMAND(hWndDlg, wParam, lParam);
            return TRUE;
        case WM_INITDIALOG:
            Handle_WM_INITDIALOG(hWndDlg, wParam, lParam);
            return TRUE;
        case WM_CLOSE:
            EndDialog(hWndDlg, 0);
            return FALSE;
    }
    
    return FALSE;
}

void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    if ( wParam == IDC_RUN )
    {
        if ( GetWindowText( GetDlgItem(hWndDlg, IDC_CMDLINE),
                            SzCmdLine, sizeof(SzCmdLine)) )
        {
            WritePrivateProfileString(SzINISection, SzINICmdLineKey,
                                        SzCmdLine, SzINIFile);
            EndDialog(hWndDlg, 1);  // Return TRUE
        }
        else
        {
            MessageBox( hWndDlg, "No program selected", 0, MB_OK);
        }
    }
    else if ( wParam == IDC_FILE )
    {
        if ( GetProgramName(hWndDlg, SzCmdLine, sizeof(SzCmdLine)) )
            SetWindowText( GetDlgItem(hWndDlg, IDC_CMDLINE), SzCmdLine );
    }
    else if ( wParam == IDCANCEL )
    {
        EndDialog(hWndDlg, 0);
    }
}

void Handle_WM_INITDIALOG(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    GetPrivateProfileString(SzINISection, SzINICmdLineKey, "", SzCmdLine,
                            sizeof(SzCmdLine), SzINIFile);
    SetWindowText( GetDlgItem(hWndDlg, IDC_CMDLINE), SzCmdLine );
}

static char szFilter1[] = "Programs (*.EXE)\0*.EXE\0";

BOOL GetProgramName(HWND hWndOwner, PSTR szFile, unsigned nFileBuffSize)
{
    OPENFILENAME ofn;

    szFile[0] = 0;

    memset(&ofn, 0, sizeof(OPENFILENAME));
    
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWndOwner;
    ofn.lpstrFilter = szFilter1;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile= szFile;
    ofn.nMaxFile = nFileBuffSize;
    ofn.lpstrFileTitle = 0;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = 0;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    return GetOpenFileName(&ofn);
}

BOOL LoadProcessForSpying(PSTR SzCmdLine)
{
    STARTUPINFO startupInfo;
    
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
        
    return CreateProcess(
                0,                          // lpszImageName
                SzCmdLine,                  // lpszCommandLine
                0,                          // lpsaProcess
                0,                          // lpsaThread
                FALSE,                      // fInheritHandles
                DEBUG_ONLY_THIS_PROCESS,    // fdwCreate
                0,                          // lpvEnvironment
                0,                          // lpszCurDir
                &startupInfo,               // lpsiStartupInfo
                &ProcessInformation         // lppiProcInfo
                );
}

void DebugLoop(void)
{
    DEBUG_EVENT event;
    DWORD continueStatus;
    BOOL fWin32s;
    BOOL fWaitResult;
    
    fWin32s = (GetVersion() & 0xC0000000) == 0x80000000;
    
    while ( 1 )
    {
        fWaitResult = WaitForDebugEvent(&event, INFINITE);
            
        if ( (fWaitResult == FALSE) && fWin32s )
        {
            EmptyMsgQueueOfUselessMessages();
            continue;
        }
        
        continueStatus = HandleDebugEvent( &event );
        
        if ( event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT )
            return;
        
        ContinueDebugEvent( event.dwProcessId,
                            event.dwThreadId,
                            continueStatus );
    }
}

PSTR SzDebugEventTypes[] = 
{
"",
"EXCEPTION",
"CREATE_THREAD",
"CREATE_PROCESS",
"EXIT_THREAD",
"EXIT_PROCESS",
"LOAD_DLL",
"UNLOAD_DLL",
"OUTPUT_DEBUG_STRING",
"RIP",
};

DWORD HandleDebugEvent( DEBUG_EVENT * event )
{
    DWORD continueStatus = DBG_CONTINUE;
    // char buffer[1024];

    // wsprintf(buffer, "Event: %s\r\n",
    //          SzDebugEventTypes[event->dwDebugEventCode]);
    // OutputDebugString(buffer);


    if ( event->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT )
    {
        ProcessDebugInfo = event->u.CreateProcessInfo;
    }
    else if ( event->dwDebugEventCode == EXCEPTION_DEBUG_EVENT )
    {
        HandleException(event, &continueStatus);
    }
        
    return continueStatus;
}

void HandleException(LPDEBUG_EVENT lpEvent, PDWORD continueStatus)
{
    // char buffer[128];
    // wsprintf(buffer, "Exception code: %X  Addr: %08X\r\n",
    //          lpEvent->u.Exception.ExceptionRecord.ExceptionCode,
    //          lpEvent->u.Exception.ExceptionRecord.ExceptionAddress);
    // OutputDebugString(buffer);
            
                
    if ( lpEvent->u.Exception.ExceptionRecord.ExceptionCode
            == EXCEPTION_BREAKPOINT )
    {
        if ( FFirstBreakpointHit == FALSE )
        {
            InjectSpyDll();
            FFirstBreakpointHit = TRUE;
        }
        else if ( FSecondBreakpointHit == FALSE )
        {
            ReplaceOriginalPagesAndContext();
            FSecondBreakpointHit = TRUE;
        }
        
        *continueStatus = DBG_CONTINUE;
    }
    else
    {
        *continueStatus = DBG_EXCEPTION_NOT_HANDLED;
    }
}

void EmptyMsgQueueOfUselessMessages(void)
{
    MSG msg;        // See PeekMessage loop for explanation of idiocy

    // Win32s idiocy puts W32s_Debug_Msg message in our message queue
    // Dispose of them!  They're useless!
    while ( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
    {
        if ( msg.hwnd )
            DispatchMessage(&msg);
    }
}

#pragma pack ( 1 )
typedef struct
{
    WORD    instr_SUB;
    DWORD   operand_SUB_value;
    BYTE    instr_PUSH;
    DWORD   operand_PUSH_value;
    BYTE    instr_CALL;
    DWORD   operand_CALL_offset;
    BYTE    instr_INT_3;
    char    data_DllName[1];
} FAKE_LOADLIBRARY_CODE, * PFAKE_LOADLIBRARY_CODE;

BOOL InjectSpyDll(void)
{
    BOOL retCode;
    DWORD cBytesMoved;
    char szSpyDllName[MAX_PATH];
    FARPROC pfnLoadLibrary;
    PFAKE_LOADLIBRARY_CODE pNewCode;
    
    // =====================================================================
    // Phase 1 - Locating addresses of important things
    // =====================================================================
        
    pfnLoadLibrary = GetProcAddress( GetModuleHandle("KERNEL32.DLL"),
                                     "LoadLibraryA" );
    if ( !pfnLoadLibrary )
        return FALSE;
    
    PInjectionPage = FindUsablePage(ProcessInformation.hProcess,
                                        ProcessDebugInfo.lpBaseOfImage);
    if ( !PInjectionPage )
        return FALSE;
    
    if ( !GetSpyDllName(szSpyDllName, sizeof(szSpyDllName)) )
        return FALSE;

    OriginalThreadContext.ContextFlags = CONTEXT_CONTROL;
    if ( !GetThreadContext(ProcessInformation.hThread,&OriginalThreadContext))
        return FALSE;
    
    // =====================================================================
    // Phase 2 - Saving the original code page away
    // =====================================================================

    // Save off the original code page
    retCode = ReadProcessMemory(ProcessInformation.hProcess, PInjectionPage,
                                OriginalCodePage, sizeof(OriginalCodePage),
                                &cBytesMoved);
    if ( !retCode || (cBytesMoved != sizeof(OriginalCodePage)) )
        return FALSE;

    // =====================================================================
    // Phase 3 - Writing new code page and changing the thread context
    // =====================================================================

    pNewCode = (PFAKE_LOADLIBRARY_CODE)NewCodePage;

    pNewCode->instr_SUB = 0xEC81;
    pNewCode->operand_SUB_value = 0x1000;
        
    pNewCode->instr_PUSH = 0x68;
    pNewCode->operand_PUSH_value = (DWORD)PInjectionPage
                            + offsetof(FAKE_LOADLIBRARY_CODE, data_DllName);

    pNewCode->instr_CALL = 0xE8;
    pNewCode->operand_CALL_offset =
            (DWORD)pfnLoadLibrary - (DWORD)PInjectionPage
            - offsetof(FAKE_LOADLIBRARY_CODE,instr_CALL) - 5;

    pNewCode->instr_INT_3 = 0xCC;

    lstrcpy(pNewCode->data_DllName, szSpyDllName); // Copy DLL name
    
    // Write out the new code page
    retCode = WriteProcessMemory(ProcessInformation.hProcess, PInjectionPage,
                                &NewCodePage, sizeof(NewCodePage),
                                &cBytesMoved);
    if ( !retCode || (cBytesMoved != sizeof(NewCodePage)) )
        return FALSE;

    FakeLoadLibraryContext = OriginalThreadContext;
    FakeLoadLibraryContext.Eip = (DWORD)PInjectionPage;
    
    if ( !SetThreadContext(ProcessInformation.hThread,
                            &FakeLoadLibraryContext) )
        return FALSE;
    
    return TRUE;
}

BOOL ReplaceOriginalPagesAndContext(void)
{
    BOOL retCode;
    DWORD cBytesMoved;
    
    retCode = WriteProcessMemory(ProcessInformation.hProcess, PInjectionPage,
                                OriginalCodePage, sizeof(OriginalCodePage),
                                &cBytesMoved);
    if ( !retCode || (cBytesMoved != sizeof(OriginalCodePage)) )
        return FALSE;

    if ( !SetThreadContext(ProcessInformation.hThread,
                            &OriginalThreadContext) )
        return FALSE;
    
    return TRUE;
}

PVOID FindUsablePage(HANDLE hProcess, PVOID PProcessBase)
{
    DWORD peHdrOffset;
    DWORD cBytesMoved;
    IMAGE_NT_HEADERS ntHdr;
    PIMAGE_SECTION_HEADER pSection;
    unsigned i;
    
    // Read in the offset of the PE header within the debuggee
    if ( !ReadProcessMemory(ProcessInformation.hProcess,
                            (PBYTE)PProcessBase + 0x3C,
                            &peHdrOffset,
                            sizeof(peHdrOffset),
                            &cBytesMoved) )
        return FALSE;
        
    
    // Read in the IMAGE_NT_HEADERS.OptionalHeader.BaseOfCode field
    if ( !ReadProcessMemory(ProcessInformation.hProcess,
                            (PBYTE)PProcessBase + peHdrOffset,
                            &ntHdr, sizeof(ntHdr), &cBytesMoved) )
        return FALSE;

    pSection = (PIMAGE_SECTION_HEADER)
                ((PBYTE)PProcessBase + peHdrOffset + 4
                + sizeof(ntHdr.FileHeader)
                + ntHdr.FileHeader.SizeOfOptionalHeader);
        
    for ( i=0; i < ntHdr.FileHeader.NumberOfSections; i++ )
    {
        IMAGE_SECTION_HEADER section;
        
        if ( !ReadProcessMemory( ProcessInformation.hProcess,
                                 pSection, &section, sizeof(section),
                                 &cBytesMoved) )
            return FALSE;

        // OutputDebugString( "trying section: " );
        // OutputDebugString( section.Name );
        // OutputDebugString( "\r\n" );

        // If it's writeable, and not the .idata section, we'll go with it
        if ( (section.Characteristics & IMAGE_SCN_MEM_WRITE)
             && strncmp(section.Name, ".idata", 6) )
        {
            // OutputDebugString( "using section: " );
            // OutputDebugString( section.Name );
            // OutputDebugString( "\r\n" );
            
            return (PVOID) ((DWORD)PProcessBase + section.VirtualAddress);
        }

        pSection++; // Not this section.  Advance to next section.
    }

    return 0;
}

BOOL GetSpyDllName(PSTR buffer, UINT cBytes)
{
    char szBuffer[MAX_PATH];
    PSTR pszFilename;
    
    // Get the complete path to this EXE - The spy dll should be in the
    // same directory.
    GetModuleFileName(0, szBuffer, sizeof(szBuffer));

    pszFilename = strrchr(szBuffer, '\\');
    if ( !pszFilename )
        return FALSE;
    
    lstrcpy(pszFilename+1, "APISPY32.DLL");
    strncpy(buffer, szBuffer, cBytes);
    return TRUE;
}

