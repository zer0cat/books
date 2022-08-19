//==================================
// WIN32WLK - Matt Pietrek 1995
// FILE: WIN32WLK.C
//==================================
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <tlhelp32.h>
#pragma hdrstop
#include "mbassert.h"
#include "win32wlk.h"
#include "module32.h"
#include "procdb.h"
#include "threaddb.h"
#include "k32objs.h"

// Prototype the functions for this
void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
void Handle_WM_INITDIALOG(HWND hWndDlg);
void Handle_WM_DELETEITEM(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Win32WlkDlgProc(HWND, UINT, WPARAM, LPARAM);
void RecordListboxLineTypeAndValue(HWND hWnd, DWORD type, DWORD value);
BOOL RetrieveListboxLineTypeAndValue(HWND hWnd, DWORD *type, DWORD *value);
void UpdateProcessList(void);
void UpdateThreadList(void);
void UpdateModuleList(void);
void ShowProcessDetails( DWORD processID );
void ShowHandleTableDetails( PHANDLE_TABLE pHndTbl );
void ShowThreadDetails( DWORD threadID );
void ShowTIBDetails( PTIB ptib );
void ShowModuleDetails( PIMTE pimte );
void ShowPEHeader( PIMAGE_NT_HEADERS pNTHdr );
void ShowMODREFListDetails( PMODREF pModRef );
void lbprintf(HWND hWnd, char * format, ...);
BOOL IsModule(PIMTE pimte);
BOOL IsProcessId( DWORD pid);
BOOL IsThreadId( DWORD tid);
BOOL IsMODREF( PMODREF pModRef );
PPROCESS_DATABASE PIDToPDB( DWORD pid );
PTHREAD_DATABASE TIDToTDB( DWORD tid );
void InitUnobsfucator(void);
void InitModuleTableBase(void);
void InitKernel32HeapHandle(void);
void GetProcessNameFromHTask( HTASK hTask, PSTR szBuffer );
void GetModuleNameFromIMTEIndex( unsigned short index, PSTR pszBuffer );
PSTR GetKernel32ObjectType( PVOID pObject );

// HWNDs of the commonly used dialog controls
HWND HWndMainList;
HWND HWndDetails;
HWND HWndDetailsDescription;

DWORD Unobsfucator = 0;
PIMTE *PModuleTable = 0;
HANDLE HKernel32Heap;
BOOL fDebugVersion;

int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow )
{
    DialogBox(hInstance, "Win32WlkDlg", 0, (DLGPROC)Win32WlkDlgProc);
    return 0;
}

void UpdateProcessList(void)
{
    HANDLE hSnapshot;
    
    SendMessage(HWndMainList, LB_RESETCONTENT, 0, 0);

    hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPALL, 0 );
    if ( hSnapshot )
    {
        PROCESSENTRY32 process;
        BOOL fMore;

        process.dwSize = sizeof(process);
        fMore = Process32First( hSnapshot, &process );
        
        while ( fMore )
        {
            PPROCESS_DATABASE ppdb;
            char szBuffer[20];
            
            ppdb = PIDToPDB( process.th32ProcessID );
            GetProcessNameFromHTask( ppdb->W16TDB, szBuffer );
                
            lbprintf(HWndMainList, "%08X %s", process.th32ProcessID, szBuffer);
            RecordListboxLineTypeAndValue( HWndMainList, LB_ITEM_PROCESS, 
                                            process.th32ProcessID );
            
            fMore = Process32Next( hSnapshot, &process );
        }
        
        CloseHandle( hSnapshot );
    }

    // Set selection to first process in list, and show its details
    SendMessage( HWndMainList, LB_SETCURSEL, 0, 0 );
    PostMessage( GetParent(HWndMainList), WM_COMMAND,
                    MAKEWPARAM(IDC_LB_MAIN_LIST, LBN_SELCHANGE),
                    (LPARAM)HWndMainList );
}

void UpdateThreadList(void)
{
    HANDLE hSnapshot;
    
    SendMessage(HWndMainList, LB_RESETCONTENT, 0, 0);

    hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPALL, 0 );
    if ( hSnapshot )
    {
        THREADENTRY32 thread;
        BOOL fMore;
        
        thread.dwSize = sizeof(thread);
        fMore = Thread32First( hSnapshot, &thread );
        
        while ( fMore )
        {
            PPROCESS_DATABASE ppdb;
            char szBuffer[20];
            
            ppdb = PIDToPDB( thread.th32OwnerProcessID );
            GetProcessNameFromHTask( ppdb->W16TDB, szBuffer );
                
            lbprintf( HWndMainList, "%08X %s", thread.th32ThreadID, szBuffer );
            RecordListboxLineTypeAndValue( HWndMainList, LB_ITEM_PROCESS, 
                                            thread.th32ThreadID );
            
            fMore = Thread32Next( hSnapshot, &thread );
        }
        
        CloseHandle( hSnapshot );
    }

    // Set selection to first thread in list, and show its details
    SendMessage( HWndMainList, LB_SETCURSEL, 0, 0 );
    PostMessage( GetParent(HWndMainList), WM_COMMAND,
                    MAKEWPARAM(IDC_LB_MAIN_LIST, LBN_SELCHANGE),
                    (LPARAM)HWndMainList );
}

void UpdateModuleList(void)
{
    unsigned i, cIMTEs;
    
    InitModuleTableBase();      // In case PModuleTableArray got reallocated
    
    SendMessage(HWndMainList, LB_RESETCONTENT, 0, 0);

    cIMTEs = HeapSize( HKernel32Heap, 0, (PVOID)PModuleTable ) / sizeof(PIMTE);
    
    for( i=0; i < cIMTEs; i++ )
    {
        if ( PModuleTable[i] )
        {
            lbprintf( HWndMainList, "%s", PModuleTable[i]->pszModName );
            RecordListboxLineTypeAndValue( HWndMainList, LB_ITEM_HMODULE, 
                                            (DWORD)PModuleTable[i] );
        }
    }
    
    // Set selection to first module in list, and show its details
    SendMessage( HWndMainList, LB_SETCURSEL, 0, 0 );
    PostMessage( GetParent(HWndMainList), WM_COMMAND,
                    MAKEWPARAM(IDC_LB_MAIN_LIST, LBN_SELCHANGE),
                    (LPARAM)HWndMainList );
}

DWORD_FLAGS ProcessFlagNames[] = 
{
{ 0x00000001, "fDebugSingle" },
{ 0x00000002, "fCreateProcessEvent" },
{ 0x00000004, "fExitProcessEvent" },
{ 0x00000008, "fWin16Process" },
{ 0x00000010, "fDosProcess" },
{ 0x00000020, "fConsoleProcess" },
{ 0x00000040, "fFileApisAreOem" },
{ 0x00000080, "fNukeProcess" },
{ 0x00000100, "fServiceProcess" },
{ 0x00000800, "fLoginScriptHack" },
{ 0x00200000, "fSendDLLNotifications" },
{ 0x00400000, "fDebugEventPending" },
{ 0x00800000, "fNearlyTerminating" },
{ 0x08000000, "fFaulted" },
{ 0x10000000, "fTerminating" },
{ 0x20000000, "fTerminated" },
{ 0x40000000, "fInitError" },
{ 0x80000000, "fSignaled" },
};

void ShowProcessDetails( DWORD pid )
{
    char szBuffer[512];
    char szBuffer2[384];
    PPROCESS_DATABASE ppdb;
    PENVIRONMENT_DATABASE pedb;
    unsigned i;
    
    if ( !IsProcessId(pid) )
    {
        MessageBox( 0, "Not a valid process", 0, MB_OK );
        return;
    }
    
    ppdb = PIDToPDB(pid);
    pedb = ppdb->pEDB;
    MBassert( IsK32HeapHandle(ppdb->pEDB) || !ppdb->pEDB);

    GetProcessNameFromHTask( (HTASK)ppdb->W16TDB, szBuffer2 );

    InitModuleTableBase();      // In case PModuleTableArray got reallocated
    
    wsprintf(szBuffer, "Process: %08X (%08X) %s", pid, ppdb, szBuffer2 );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    SendMessage( HWndDetails, WM_SETREDRAW, FALSE, 0 ); // Turn off redraws

    lbprintf( HWndDetails, "Type: %08X", ppdb->Type );
    lbprintf( HWndDetails, "cReference: %08X", ppdb->cReference );
    MBassert( !ppdb->un1 );

    lbprintf( HWndDetails, "someEvent: %08X", ppdb->someEvent );
    MBassert( IsK32HeapHandle(ppdb->someEvent) || !ppdb->someEvent );

    lbprintf( HWndDetails, "TerminationStatus: %08X", ppdb->TerminationStatus );
    MBassert( !ppdb->un2 );

    lbprintf( HWndDetails, "DefaultHeap: %08X", ppdb->DefaultHeap );
    MBassert( IsHeapStart(ppdb->DefaultHeap) ) ;
    lbprintf( HWndDetails, "MemoryContext: %08X", ppdb->MemoryContext );

    MBassert( IsRing0HeapHandle(ppdb->MemoryContext) );

    wsprintf(szBuffer, "flags: %08X ", ppdb->flags );
    for ( i=0; i < (sizeof(ProcessFlagNames)/sizeof(DWORD_FLAGS)); i++ )
        if ( ppdb->flags & ProcessFlagNames[i].value )
            wsprintf(szBuffer + lstrlen(szBuffer), "%s ",
                     ProcessFlagNames[i].name);
    lbprintf( HWndDetails, szBuffer );

    lbprintf( HWndDetails, "pPSP: %08X", ppdb->pPSP );
    lbprintf( HWndDetails, "PSPSelector: %04X", ppdb->PSPSelector );
    MBassert( IsSelector( ppdb->PSPSelector ) );

    lbprintf( HWndDetails, "+MTE Index: %04X", ppdb->MTEIndex );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_HMODULE,
                                    (DWORD)PModuleTable[ppdb->MTEIndex] );
    lbprintf( HWndDetails, "cThreads: %04X", ppdb->cThreads );
    MBassert( ppdb->cThreads );

    lbprintf( HWndDetails, "cNotTermThreads: %04X", ppdb->cNotTermThreads );
    MBassert( !ppdb->un3 );

    lbprintf( HWndDetails, "cRing0Threads: %08X", ppdb->cRing0Threads );
    MBassert( ppdb->cRing0Threads >= ppdb->cThreads );

    lbprintf( HWndDetails, "HeapHandle: %08X", ppdb->HeapHandle );
    MBassert( IsHeapStart(ppdb->HeapHandle) ) ;
    lbprintf( HWndDetails, "W16TDB: %08X", ppdb->W16TDB );

    MBassert( Is16BitGlobalHandle(ppdb->W16TDB) );

    lbprintf( HWndDetails, "MemMapFiles: %08X", ppdb->MemMapFiles );
    MBassert( IsK32HeapHandle(ppdb->MemMapFiles) || !ppdb->MemMapFiles );

    lbprintf( HWndDetails, "pEDB: %08X", ppdb->pEDB );
    lbprintf( HWndDetails, "+pHandleTable: %08X", ppdb->pHandleTable );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_HANDLE_TABLE,
                                    (DWORD)ppdb->pHandleTable );
    MBassert( IsK32HeapHandle(ppdb->pHandleTable) );

    lbprintf( HWndDetails, "+Parent process: %08X", ppdb->ParentPDB );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_PROCESS,
                                    (DWORD)PIDToPDB((DWORD)ppdb->ParentPDB));
    MBassert( IsK32HeapHandle( ppdb->ParentPDB ) || !ppdb->ParentPDB );

    lbprintf( HWndDetails, "+MODREFlist: %08X", ppdb->MODREFlist );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_MODREF_LIST,
                                    (DWORD)ppdb->MODREFlist );
    MBassert( IsK32HeapHandle( ppdb->MODREFlist ) );

    lbprintf( HWndDetails, "ThreadList: %08X", ppdb->ThreadList );
    MBassert( IsK32HeapHandle(ppdb->ThreadList) );

    lbprintf( HWndDetails, "DebuggeeCB: %08X", ppdb->DebuggeeCB );
    lbprintf( HWndDetails, "LocalHeapFreeHead: %08X", ppdb->LocalHeapFreeHead );
    MBassert( IsDivisibleBy4(ppdb->LocalHeapFreeHead)
              || !ppdb->LocalHeapFreeHead );

    lbprintf( HWndDetails, "InitialRing0ID: %08X", ppdb->InitialRing0ID );

    MBassert( !ppdb->un4[0] );
    MBassert( !ppdb->un4[1] );
    MBassert( !ppdb->un4[2] );

    if ( !fDebugVersion )
        ppdb = (PPROCESS_DATABASE)( (PBYTE)ppdb - 4 );
            
    lbprintf( HWndDetails, "pConsole: %08X", ppdb->pConsole );
    MBassert( IsK32HeapHandle(ppdb->pConsole) || !ppdb->pConsole );

    lbprintf( HWndDetails, "tlsInUseBits1: %08X", ppdb->tlsInUseBits1 );
    lbprintf( HWndDetails, "tlsInUseBits2: %08X", ppdb->tlsInUseBits2 );
    lbprintf( HWndDetails, "ProcessDWORD: %08X", ppdb->ProcessDWORD );
    lbprintf( HWndDetails, "+ProcessGroup: %08X", ppdb->ProcessGroup );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_PROCESS,
                                    (DWORD)PIDToPDB((DWORD)ppdb->ProcessGroup));
    MBassert( IsK32HeapHandle( ppdb->ProcessGroup ) || !ppdb->ProcessGroup );

    lbprintf( HWndDetails, "pExeMODREF: %08X", ppdb->pExeMODREF );  
    MBassert( IsK32HeapHandle( ppdb->pExeMODREF ) );

    lbprintf( HWndDetails, "TopExcFilter: %08X", ppdb->TopExcFilter );
    lbprintf( HWndDetails, "BasePriority: %08X", ppdb->BasePriority );
    MBassert( (ppdb->BasePriority <= 31) );

    lbprintf( HWndDetails, "HeapOwnList: %08X", ppdb->HeapOwnList );
    MBassert( IsHeapStart(ppdb->HeapOwnList) );

    lbprintf( HWndDetails, "HeapHandleBlockList: %08X", ppdb->HeapHandleBlockList );
    MBassert( IsDivisibleBy4(ppdb->HeapHandleBlockList)
              || !ppdb->HeapHandleBlockList );

    lbprintf( HWndDetails, "pSomeHeapPtr: %08X", ppdb->pSomeHeapPtr );
    MBassert( IsK32HeapHandle(ppdb->pSomeHeapPtr) || !ppdb->pSomeHeapPtr );

    lbprintf( HWndDetails, "pConsoleProvider: %08X", ppdb->pConsoleProvider );
    MBassert( IsK32HeapHandle(ppdb->pConsoleProvider) ||
              !ppdb->pConsoleProvider );

    lbprintf( HWndDetails, "EnvironSelector: %04X", ppdb->EnvironSelector );
    MBassert( IsSelector( ppdb->EnvironSelector) || !ppdb->EnvironSelector );

    lbprintf( HWndDetails, "ErrorMode: %04X", ppdb->ErrorMode );
    lbprintf( HWndDetails, "pevtLoadFinished: %08X", ppdb->pevtLoadFinished );
    MBassert( IsK32HeapHandle(ppdb->pevtLoadFinished) );

    lbprintf( HWndDetails, "UTState: %04X", ppdb->UTState );

    SendMessage( HWndDetails, WM_SETREDRAW, TRUE, 0 );  // Turn on redraws
        
    if ( IsBadReadPtr(pedb, sizeof(ENVIRONMENT_DATABASE)) )
        lbprintf( HWndDetails, "Environment Database ptr invalid" );
    else
    {
    lbprintf( HWndDetails, "Environment Database:" );
    __try
    {
        lbprintf(HWndDetails, "  pszEnvironment: %08X", pedb->pszEnvironment);
        lbprintf(HWndDetails, "  pszCmdLine: %s", pedb->pszCmdLine);
    }
    __except( 1 ){}
    lbprintf( HWndDetails, "  pszCurrDirectory: %s", pedb->pszCurrDirectory );
    lbprintf( HWndDetails, "  pStartupInfo: %08X", pedb->pStartupInfo );
    lbprintf( HWndDetails, "  hStdIn: %08X", pedb->hStdIn );
    lbprintf( HWndDetails, "  hStdOut: %08X", pedb->hStdOut );
    lbprintf( HWndDetails, "  hStdErr: %08X", pedb->hStdErr );
    lbprintf( HWndDetails, "  un2: %08X", pedb->un2 );
    lbprintf( HWndDetails, "  InheritConsole: %08X", pedb->InheritConsole );
    lbprintf( HWndDetails, "  BreakType: %08X", pedb->BreakType );
    lbprintf( HWndDetails, "  BreakSem: %08X", pedb->BreakSem );
    lbprintf( HWndDetails, "  BreakEvent: %08X", pedb->BreakEvent );
    lbprintf( HWndDetails, "  BreakThreadID: %08X", pedb->BreakThreadID );
    lbprintf( HWndDetails, "  BreakHandlers: %08X", pedb->BreakHandlers );
    }

    __try
    {
        MBassert( !pedb->un1 );
    }
    __except( 1 ){}
}

void ShowHandleTableDetails( PHANDLE_TABLE pHndTbl )
{
    char szBuffer[384];
    unsigned i;

    if ( IsBadReadPtr(pHndTbl, sizeof(HANDLE_TABLE)) || !pHndTbl->cEntries )
    {
        MessageBox( 0, "Not a valid handle table", 0, MB_OK );
        return;
    }

    wsprintf(szBuffer, "Handle Table: %08X", pHndTbl);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    SendMessage( HWndDetails, WM_SETREDRAW, FALSE, 0 ); // Turn off redraws
    for ( i=0; i < pHndTbl->cEntries; i++ )
    {
        if ( !pHndTbl->array[i].pObject )
            continue;
        
        lbprintf( HWndDetails, "%02X %08X %s", i, pHndTbl->array[i].pObject,
                    GetKernel32ObjectType(pHndTbl->array[i].pObject) );
    }

    SendMessage( HWndDetails, WM_SETREDRAW, TRUE, 0 );  // Turn on redraws
}

DWORD_FLAGS TIBFlagNames[] = 
{
{ 0x00000001, "TIBF_WIN32" },
{ 0x00000002, "TIBF_TRAP" },
};

DWORD_FLAGS TDBFlagNames[] =
{
{ 0x00000001, "fCreateThreadEvent" },
{ 0x00000002, "fCancelExceptionAbort" },
{ 0x00000004, "fOnTempStack" },
{ 0x00000008, "fGrowableStack" },
{ 0x00000010, "fDelaySingleStep" },
{ 0x00000020, "fOpenExeAsImmovableFile" },
{ 0x00000040, "fCreateSuspended" },
{ 0x00000080, "fStackOverflow" },
{ 0x00000100, "fNestedCleanAPCs" },
{ 0x00000200, "fWasOemNowAnsi" },
{ 0x00000400, "fOKToSetThreadOem" },
};

void ShowThreadDetails( DWORD tid )
{
    char szBuffer[512];
    char szBuffer2[20];
    PTHREAD_DATABASE ptdb;
    unsigned i;
    
    if ( !IsThreadId(tid) )
    {
        MessageBox( 0, "Not a valid thread", 0, MB_OK );
        return;
    }
    
    ptdb = TIDToTDB(tid);
    
    GetProcessNameFromHTask( (HTASK)ptdb->W16TDB, szBuffer2 );
    
    wsprintf(szBuffer, "Thread: %08X (%08X) %s", tid, ptdb, szBuffer2 );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    SendMessage( HWndDetails, WM_SETREDRAW, FALSE, 0 ); // Turn off redraws

    lbprintf(HWndDetails, "+pProcess: %08X", ptdb->pProcess);
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_PROCESS, 
                                    (DWORD)PIDToPDB((DWORD)ptdb->pProcess) );
    lbprintf(HWndDetails, "+pProcess2: %08X", ptdb->pProcess2);
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_PROCESS, 
                                    (DWORD)PIDToPDB((DWORD)ptdb->pProcess2) );
    lbprintf(HWndDetails, "pCurrentPriority: %08X (%X)",
                ptdb->pCurrentPriority, *(ptdb->pCurrentPriority) );
    lbprintf(HWndDetails, "DeltaPriority: %08X", ptdb->DeltaPriority);
    lbprintf(HWndDetails, "TopOfStack: %08X  (size:%X)",
        ptdb->TopOfStack,
        ptdb->TopOfStack -(ptdb->StackBase ? ptdb->StackBase:ptdb->TopOfStack));
    lbprintf(HWndDetails, "StackLow: %08X (used:%X)", ptdb->StackLow,
        ptdb->TopOfStack - ptdb->StackLow);
    lbprintf(HWndDetails, "StackBase: %08X", ptdb->StackBase);
    lbprintf(HWndDetails, "StackSelector16: %04X", ptdb->StackSelector16);
    lbprintf(HWndDetails, "ThunkSS16: %04X", ptdb->ThunkSS16);
    lbprintf(HWndDetails, "CurrentSS: %08X", ptdb->CurrentSS);
    lbprintf(HWndDetails, "NegStackBase: %08X", ptdb->NegStackBase);
    lbprintf(HWndDetails, "SSTable: %08X", ptdb->SSTable);
    lbprintf(HWndDetails, "+pTIB: %08X", ptdb->pTIB);
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_TIB,(DWORD)ptdb->pTIB);
    lbprintf(HWndDetails, "TIBSelector: %04X", ptdb->TIBSelector);

    wsprintf(szBuffer, "TIBFlags: %08X ", ptdb->TIBFlags );
    for ( i=0; i < (sizeof(TIBFlagNames)/sizeof(DWORD_FLAGS)); i++ )
        if ( ptdb->TIBFlags & TIBFlagNames[i].value )
            wsprintf(szBuffer + lstrlen(szBuffer), "%s ",
                     TIBFlagNames[i].name);
    lbprintf( HWndDetails, szBuffer );

    lbprintf(HWndDetails, "W16TDB: %04X", ptdb->W16TDB);
    lbprintf(HWndDetails, "MessageQueue: %04X", ptdb->MessageQueue);
    lbprintf(HWndDetails, "GetLastErrorCode: %08X", ptdb->GetLastErrorCode);
    lbprintf(HWndDetails, "Win16MutexCount: %04X", ptdb->Win16MutexCount);
    lbprintf(HWndDetails, "pvExcept: %08X", ptdb->pvExcept);

    wsprintf(szBuffer, "Flags: %08X ", ptdb->Flags );
    for ( i=0; i < (sizeof(TDBFlagNames)/sizeof(DWORD_FLAGS)); i++ )
        if ( ptdb->Flags & TDBFlagNames[i].value )
            wsprintf(szBuffer + lstrlen(szBuffer), "%s ",
                     TDBFlagNames[i].name);
    lbprintf( HWndDetails, szBuffer );

    lbprintf(HWndDetails, "WaitNodeList: %08X", ptdb->WaitNodeList);
    lbprintf(HWndDetails, "Ring0Thread: %08X", ptdb->Ring0Thread);
    lbprintf(HWndDetails, "pTDBX: %08X", ptdb->pTDBX);
    lbprintf(HWndDetails, "TerminationStatus: %08X", ptdb->TerminationStatus);
    lbprintf(HWndDetails, "TerminationStack: %08X", ptdb->TerminationStack);
    lbprintf(HWndDetails, "ThreadContext: %08X", ptdb->ThreadContext);
    lbprintf(HWndDetails, "DebugContext: %08X", ptdb->DebugContext);
    lbprintf(HWndDetails, "DebuggerCB: %08X", ptdb->DebuggerCB);
    lbprintf(HWndDetails, "DebuggerThread: %08X", ptdb->DebuggerThread);

    // Relatively uninteresting fields
    lbprintf(HWndDetails, "Type: %08X", ptdb->Type);
    lbprintf(HWndDetails, "cReference: %08X", ptdb->cReference);
    lbprintf(HWndDetails, "someEvent: %08X", ptdb->someEvent);
    lbprintf(HWndDetails, "pTLSArray: %08X", ptdb->pTLSArray);
    MBassert( ((DWORD)ptdb + offsetof(THREAD_DATABASE, TLSArray))
                == ptdb->pTLSArray);

    lbprintf(HWndDetails, "EmulatorSelector: %04X", ptdb->EmulatorSelector);
    lbprintf(HWndDetails, "EmulatorData: %08X", ptdb->EmulatorData);
    lbprintf(HWndDetails, "SelmanList: %08X", ptdb->SelmanList);
    lbprintf(HWndDetails, "un4: %08X", ptdb->un4);

    MBassert( !ptdb->UserPointer );
    MBassert( !ptdb->cHandles );
    MBassert( !ptdb->Except16List );
    MBassert( !ptdb->ThunkConnect );

    if ( fDebugVersion )
    {
        MBassert( !ptdb->un5[0] );
        MBassert( !ptdb->un5[1] );
        MBassert( !ptdb->un5[2] );
        MBassert( !ptdb->un5[3] );
        MBassert( !ptdb->un5[4] );
        MBassert( !ptdb->un5[5] );
        MBassert( !ptdb->un5[6] );
        MBassert( !ptdb->pCreateData16 );
        lbprintf(HWndDetails, "APISuspendCount: %08X", ptdb->APISuspendCount);
        MBassert( !ptdb->un6 );
        lbprintf(HWndDetails, "WOWChain: %08X", ptdb->WOWChain);
        lbprintf(HWndDetails, "wSSBig: %08X", ptdb->wSSBig);
        MBassert( !ptdb->un7 );
        MBassert( !ptdb->lp16SwitchRec );
        MBassert( !ptdb->un8[0] );
        MBassert( !ptdb->un8[1] );
        MBassert( !ptdb->un8[2] );
        MBassert( !ptdb->un8[3] );
        MBassert( !ptdb->un8[4] );
        MBassert( !ptdb->un8[5] );
        lbprintf(HWndDetails, "pSomeCritSect1: %08X", ptdb->pSomeCritSect1);
        lbprintf(HWndDetails, "pWin16Mutex: %08X", ptdb->pWin16Mutex);
        lbprintf(HWndDetails, "pWin32Mutex: %08X", ptdb->pWin32Mutex);
        lbprintf(HWndDetails, "pSomeCritSect2: %08X", ptdb->pSomeCritSect2);
        MBassert( !ptdb->un9 );
        lbprintf(HWndDetails, "ripString: %08X", ptdb->ripString);
    }

    SendMessage( HWndDetails, WM_SETREDRAW, TRUE, 0 );  // Turn on redraws
}

void ShowTIBDetails( PTIB ptib )
{
    char szBuffer[384];

    #if 0   // Need a validation routine
    if (.....
    {
        MessageBox( 0, "Not a valid TIB", 0, MB_OK );
        return;
    }
    #endif
        
    wsprintf(szBuffer, "TIB: %08X", ptib );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    lbprintf(HWndDetails, "pvExcept: %08X", ptib->pvExcept);
    lbprintf(HWndDetails, "pvStackUserTop: %08X", ptib->pvStackUserTop);
    lbprintf(HWndDetails, "pvStackUserBase: %08X", ptib->pvStackUserBase);
    lbprintf(HWndDetails, "pvTDB: %04X", ptib->pvTDB);
    lbprintf(HWndDetails, "pvThunksSS: %04X", ptib->pvThunksSS);
    lbprintf(HWndDetails, "SelmanList: %08X", ptib->SelmanList);
    lbprintf(HWndDetails, "pvArbitrary: %08X", ptib->pvArbitrary);
    lbprintf(HWndDetails, "ptibSelf: %08X", ptib->ptibSelf);
    lbprintf(HWndDetails, "TIBFlags: %04X", ptib->TIBFlags);
    lbprintf(HWndDetails, "Win16MutexCount: %04X", ptib->Win16MutexCount);
    lbprintf(HWndDetails, "DebugContext: %08X", ptib->DebugContext);
    lbprintf(HWndDetails, "pCurrentPriority: %08X", ptib->pCurrentPriority);
    lbprintf(HWndDetails, "pvQueue: %04X", ptib->pvQueue);
    lbprintf(HWndDetails, "pvTLSArray: %08X", ptib->pvTLSArray);
}

void ShowModuleDetails( PIMTE pimte )
{
    char szBuffer[384];

    if ( !IsModule(pimte) )
    {
        MessageBox( 0, "Not a valid module", 0, MB_OK );
        return;
    }

    wsprintf(szBuffer, "Module: %08X", pimte);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    lbprintf( HWndDetails, "FileName: (%08X) %s", pimte->pszFileName, pimte->pszFileName );
    lbprintf( HWndDetails, "FileName2: (%08X) %s", pimte->pszFileName2, pimte->pszFileName2 );
    lbprintf( HWndDetails, "ModName: %s", pimte->pszModName );
    lbprintf( HWndDetails, "ModName2: %s", pimte->pszModName2 );
    lbprintf( HWndDetails, "cUsage: %04X", pimte->cUsage );
    lbprintf( HWndDetails, "baseAddress: %08X", pimte->baseAddress );
    lbprintf( HWndDetails, "cSections: %08X", pimte->cSections );
    lbprintf( HWndDetails, "IMAGE_NT_HEADER: %08X", pimte->pNTHdr );
    ShowPEHeader( pimte->pNTHdr );
    
    lbprintf( HWndDetails, "hModule16: %04X", pimte->hModule16 );
    lbprintf( HWndDetails, "un1: %08X", pimte->un1 );
    lbprintf( HWndDetails, "un3: %08X", pimte->un3 );
    lbprintf( HWndDetails, "un5: %08X", pimte->un5 );
    lbprintf( HWndDetails, "un7: %08X", pimte->un7 );

    // Uninteresting fields
    // lbprintf( HWndDetails, "cbFileName: %04X", pimte->cbFileName );
    // lbprintf( HWndDetails, "cbModName: %04X", pimte->cbModName );
    // lbprintf( HWndDetails, "cbFileName: %04X", pimte->cbFileName );
    
    MBassert( pimte->un2 == -1 );

}

void ShowPEHeader( PIMAGE_NT_HEADERS pNTHdr )
{
    unsigned i;
    PIMAGE_SECTION_HEADER pSection;
    
    pSection = IMAGE_FIRST_SECTION( pNTHdr );
    
    for ( i=1; i <= pNTHdr->FileHeader.NumberOfSections; i++ )
    {
        lbprintf(HWndDetails, "  %02X  %-8.8s  va:%08X  size:%08X",
                i, pSection->Name,
                pSection->VirtualAddress + pNTHdr->OptionalHeader.ImageBase,
                pSection->Misc.VirtualSize);
                    
        pSection++;
    }
}

void ShowMODREFListDetails( PMODREF pModRef )
{
    char szBuffer[384];

    if ( !IsMODREF(pModRef) )
    {
        MessageBox( 0, "Not a valid MODREF", 0, MB_OK );
        return;
    }

    InitModuleTableBase();      // In case PModuleTableArray got reallocated

    wsprintf( szBuffer, "MODREF list: %08X", pModRef );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    while ( pModRef )
    {
        GetModuleNameFromIMTEIndex( pModRef->mteIndex, szBuffer );
        lbprintf( HWndDetails, "+%s", szBuffer );
        RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_HMODULE, 
                                      (DWORD)PModuleTable[pModRef->mteIndex] );
        pModRef = pModRef->pNextModRef;
    }
}

void lbprintf(HWND hWnd, char * format, ...)
{
    char szBuffer[512];
    va_list argptr;
          
    va_start(argptr, format);
    wvsprintf(szBuffer, format, argptr);
    va_end(argptr);

    SendMessage( hWnd, LB_ADDSTRING, 0, (LPARAM)szBuffer );
}

typedef struct
{
    DWORD   type;
    DWORD   value;
} LBITEMDATA, *PLBITEMDATA;

// Records the type (module, process, etc...) of the line that was just
// added to the specified listbox window, along with the value.
void RecordListboxLineTypeAndValue(HWND hWnd, DWORD type, DWORD value)
{
    unsigned lastIndex;
    PLBITEMDATA plbdata;
    
    lastIndex = SendMessage( hWnd, LB_GETCOUNT, 0, 0 );
    if ( !lastIndex )
        return;
    
    lastIndex--;    // Index is 0 based

    plbdata = malloc( sizeof(LBITEMDATA) );     // These will be freed in
    if ( plbdata )                              // out WM_DELETEITEM handler
    {                                           // in the dlg proc
        plbdata->type = type;
        plbdata->value = value;
    }
    
    SendMessage( hWnd, LB_SETITEMDATA, lastIndex, (LPARAM)plbdata );
}

BOOL RetrieveListboxLineTypeAndValue(HWND hWnd, DWORD *type, DWORD *value)
{
    PLBITEMDATA plbdata;
    unsigned index = SendMessage( hWnd, LB_GETCURSEL, 0, 0 );

    plbdata = (PLBITEMDATA)SendMessage( hWnd, LB_GETITEMDATA, index, 0 );
    if ( !plbdata || ((DWORD)plbdata == LB_ERR) )
        return FALSE;

    *type = plbdata->type;
    *value = plbdata->value;
    
    return TRUE;
}

BOOL IsProcessId(DWORD pid)
{
    PPROCESS_DATABASE ppdb = PIDToPDB( pid );

    if ( (DWORD)ppdb < 0x80000000 )
        return FALSE;
    
    if ( IsBadReadPtr((PVOID)ppdb, sizeof(DWORD)) )
        return FALSE;

    if ( *(PDWORD)ppdb != 5 )
        return FALSE;

    // There are additional sanity checks that can be made here

    return TRUE;
}

BOOL IsThreadId(DWORD tid)
{
    PTHREAD_DATABASE ptdb = TIDToTDB( tid );
        
    if ( (DWORD)ptdb < 0x80000000 )
        return FALSE;
    
    if ( IsBadReadPtr((PVOID)ptdb, sizeof(DWORD)) )
        return FALSE;

    if ( *(PDWORD)ptdb != 6 )
        return FALSE;

    // There are additional sanity checks that can be made here

    return TRUE;
}

BOOL IsModule( PIMTE pimte )
{
    __try
    {
        // Verify there's a pointer to an IMAGE_NT_HEADER
        if ( pimte->pNTHdr->Signature != IMAGE_NT_SIGNATURE )
            return FALSE;

        // Verify that the number of sections that are stored in two
        // places match up.
        if ( pimte->pNTHdr->FileHeader.NumberOfSections != pimte->cSections )
            return FALSE;

        // Verify the pointers to the EXE/module names
        if ( IsBadReadPtr( pimte->pszFileName, 1) )
            return FALSE;
        if ( IsBadReadPtr( pimte->pszModName, 1) )
            return FALSE;
        if ( pimte->pszFileName > pimte->pszModName )
            return FALSE;
    }
    __except( 1 )
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL IsMODREF( PMODREF pModRef )
{
    if ( (DWORD)pModRef < 0x80000000 )
        return FALSE;
    
    if ( IsBadReadPtr( pModRef, sizeof(MODREF) ) )
        return FALSE;
            
    if ( pModRef->pNextModRef != 0 )
    {
        if ( (DWORD)pModRef->pNextModRef < 0x80000000 )
            return FALSE;

        if ( IsBadReadPtr(pModRef->pNextModRef, sizeof(MODREF)) )
            return FALSE;
    }

    return TRUE;
}

PPROCESS_DATABASE PIDToPDB( DWORD pid )
{
    return (PPROCESS_DATABASE)(pid ^ Unobsfucator);
}

PTHREAD_DATABASE TIDToTDB( DWORD tid )
{
    return (PTHREAD_DATABASE)(tid ^ Unobsfucator);
}

void GetProcessNameFromHTask( HTASK hTask, PSTR pszBuffer )
{
    pszBuffer[0] = 0;
    
    __try
    {
        __asm
        {
            push ds
            push ds
            pop  es
            mov ds, word ptr [hTask]
            mov esi, 0F2h
            mov edi, [pszBuffer]
            mov ecx, 2
            cld
            rep movsd
            mov byte ptr es:[edi], 0
            pop ds
        }
    }
    __except( 1 ){}
    
}

void InitUnobsfucator(void)
{
    DWORD   tid;
    
    tid = GetCurrentThreadId();
    
    __asm {
            mov     ax, fs
            mov     es, ax
            mov     eax, 18h
            mov     eax, es:[eax]
            sub     eax, 10h
            xor     eax,[tid]
            mov     [Unobsfucator], eax
    }
}

void WINAPI GDIReallyCares( HINSTANCE );

void InitModuleTableBase(void)
{
    // Yes, this is really disgusting!
    GDIReallyCares( GetModuleHandle(0) );
    __asm   mov     [PModuleTable], ecx
}

void InitKernel32HeapHandle(void)
{
    PPROCESS_DATABASE ppdb;

    ppdb = PIDToPDB( GetCurrentProcessId() );
    
    HKernel32Heap = ppdb->HeapHandle;
}

//
// Dialog proc for the main dialog
//
BOOL CALLBACK Win32WlkDlgProc(HWND hWndDlg, UINT msg,
                              WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_COMMAND:
            Handle_WM_COMMAND(hWndDlg, wParam, lParam); return TRUE;
        case WM_INITDIALOG:
            Handle_WM_INITDIALOG(hWndDlg); return TRUE;
        case WM_CLOSE:
            EndDialog(hWndDlg, 0); return FALSE;
        case WM_DELETEITEM:
            Handle_WM_DELETEITEM( hWndDlg, wParam, lParam ); return TRUE;
    }
    return FALSE;
}

//
// Handle the dialog's WM_COMMAND messages
//
void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    //
    // If user hit <enter> see which listbox has the focus, and
    // change wParam and lParam to look as if the user performed
    // the equivalent dbl-click action.
    //
    if ( LOWORD(wParam) == IDOK )
    {
        HWND hWndFocus = GetFocus();
        if (hWndFocus == HWndDetails )
        {
            wParam = IDC_LB_DETAILS; lParam = MAKELONG(0,LBN_DBLCLK);
        }
    }
    
    switch ( LOWORD(wParam) )
    {
        case IDC_RB_PROCESSES:
            UpdateProcessList();
            break;
            
        case IDC_RB_THREADS:
            UpdateThreadList();
            break;
            
        case IDC_RB_MODULES:
            UpdateModuleList();
            break;
            
        case IDC_LB_MAIN_LIST:
            if ( HIWORD(wParam) == LBN_SELCHANGE )
            {
                DWORD handle, type;
                DWORD lbSelectedIndex;
                
                lbSelectedIndex = SendMessage(HWndMainList,LB_GETCURSEL, 0, 0);
                RetrieveListboxLineTypeAndValue(HWndMainList, &type, &handle);
                                    
                if ( IsDlgButtonChecked(hWndDlg, IDC_RB_PROCESSES) )
                    ShowProcessDetails( handle );
                else if ( IsDlgButtonChecked(hWndDlg, IDC_RB_THREADS) )
                    ShowThreadDetails( handle );
                else
                    ShowModuleDetails( (HMODULE)handle );
            }
            break;
            
        case IDC_LB_DETAILS:
            if ( HIWORD(wParam) == LBN_DBLCLK )
            {
                DWORD type, value;
                
                if ( !RetrieveListboxLineTypeAndValue(HWndDetails,
                                                        &type, &value) )
                    break;
                
                switch ( type )
                {
                    case LB_ITEM_HMODULE:
                        ShowModuleDetails( (PIMTE)value ); break;
                    case LB_ITEM_PROCESS:
                        ShowProcessDetails( value ); break;
                    case LB_ITEM_MODREF_LIST:
                        ShowMODREFListDetails( (PMODREF)value ); break;
                    case LB_ITEM_HANDLE_TABLE:
                        ShowHandleTableDetails( (PHANDLE_TABLE)value ); break;
                    case LB_ITEM_TIB:
                        ShowTIBDetails( (PTIB)value ); break;
                }
            }
            break;
    }
    return;
}

void Handle_WM_INITDIALOG(HWND hWndDlg)
{
    HWndMainList = GetDlgItem(hWndDlg, IDC_LB_MAIN_LIST);
    HWndDetails = GetDlgItem(hWndDlg, IDC_LB_DETAILS);
    HWndDetailsDescription = GetDlgItem(hWndDlg, IDC_DETAILS_TYPE );

    fDebugVersion = (BOOL)GetSystemMetrics( SM_DEBUG );

    InitUnobsfucator();
    InitModuleTableBase();
    InitKernel32HeapHandle();
    
    CheckDlgButton(hWndDlg, IDC_RB_PROCESSES, 1);
    
    if ( IsDlgButtonChecked(hWndDlg, IDC_RB_PROCESSES) )
        UpdateProcessList();   
}

void Handle_WM_DELETEITEM(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    if ( wParam != IDC_LB_DETAILS )
        return;
    
    // Free the pointer stored in the item data
    free( (PVOID)((LPDELETEITEMSTRUCT)lParam)->itemData );
}

void GetModuleNameFromIMTEIndex( unsigned short index, PSTR pszBuffer )
{
    lstrcpy( pszBuffer, PModuleTable[index]->pszModName );
}

PSTR GetKernel32ObjectType( PVOID pObject )
{
    if ( IsBadReadPtr(pObject, 4) )
        return "<???>";
    
    switch( *(PDWORD)pObject )
    {
        case K32OBJ_SEMAPHORE: return "SEMAPHORE";
        case K32OBJ_EVENT: return "EVENT";
        case K32OBJ_MUTEX: return "MUTEX";
        case K32OBJ_CRITICAL_SECTION: return "CRITICAL_SECTION";
        case K32OBJ_PROCESS: return "PROCESS";
        case K32OBJ_THREAD: return "THREAD";
        case K32OBJ_FILE: return "FILE";
        case K32OBJ_CHANGE: return "CHANGE";
        case K32OBJ_CONSOLE: return "CONSOLE";
        case K32OBJ_SCREEN_BUFFER: return "SCREEN_BUFFER";
        case K32OBJ_MEM_MAPPED_FILE: return "MEM_MAPPED_FILE";
        case K32OBJ_SERIAL: return "SERIAL";
        case K32OBJ_DEVICE_IOCTL: return "DEVICE_IOCTL";
        case K32OBJ_PIPE: return "PIPE";
        case K32OBJ_MAILSLOT: return "MAILSLOT";
        case K32OBJ_TOOLHELP_SNAPSHOT: return "TOOLHELP_SNAPSHOT";
        case K32OBJ_SOCKET: return "SOCKET";
        default: return "<unknown>";
    }
}

// Our own custom assert for GUI programs
void __cdecl _MBassert(void *pszExp, void *pszFile, unsigned lineNum)
{
    char buffer[512];
    
    wsprintf(buffer, "assert: %s (%s line %u)", pszExp, pszFile, lineNum);
    MessageBox( 0, buffer, 0, MB_OK );
}
