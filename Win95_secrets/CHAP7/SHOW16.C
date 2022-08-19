//==================================
// SHOW16 - Matt Pietrek 1995
// FILE: SHOW16.C
//==================================
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <toolhelp.h>
#include <assert.h>
#include <malloc.h>
#pragma hdrstop
#include "show16.h"
#include "tdb.h"
#include "hmodule.h"

// GetModuleName is KERNEL.27, and undocumented, so prototype it
BOOL WINAPI GetModuleName(HMODULE hModule, LPSTR lpszName, UINT cb);

// Prototype the functions for this
void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
void Handle_WM_INITDIALOG(HWND hWndDlg);
BOOL CALLBACK _export Show16DlgProc(HWND, UINT, WPARAM, LPARAM);
void RecordListboxLineTypeAndValue(HWND hWnd, WORD type, WORD value);
BOOL RetrieveListboxLineTypeAndValue(HWND hWnd, WORD *type, WORD *value);
void UpdateTaskList(void);
void UpdateModuleList(void);
void ShowTaskDetails(HTASK hTask);
void ShowModuleDetails(HMODULE hModule);
void ShowSegmentTableDetails(HMODULE hModule);
void ShowEntryTableDetails(HMODULE hModule);
void ShowResourceDetails(HMODULE hModule);
void ShowResidentNamesDetails(HMODULE hModule);
void ShowNonResidentNamesDetails(HMODULE hModule);
void lbprintf(HWND hWnd, char * format, ...);
LPSTR GetResourceTypeName(WORD id);
BOOL IsModule(HMODULE hModule);
BOOL IsA32BitHMODULE(WORD selector);
LPSTR GetTaskModuleName(HTASK hTask);

// HWNDs of the commonly used dialog controls
HWND HWndMainList;
HWND HWndDetails;
HWND HWndDetailsDescription;


int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow )
{
    DialogBox(hInstance, "Show16Dlg", 0, (DLGPROC)Show16DlgProc);
    return 0;
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
    if ( wParam == IDOK )
    {
        HWND hWndFocus = GetFocus();
        if (hWndFocus == HWndDetails )
        {
            wParam = IDC_LB_DETAILS; lParam = MAKELONG(0,LBN_DBLCLK);
        }
    }
    
    switch ( wParam )
    {
        case IDC_BUTTON_EXIT:
            EndDialog(hWndDlg, 0);
            break;
            
        case IDC_RB_TASKS:
            UpdateTaskList();
            // Set focus to main listbox
            //SendMessage( hWndDlg, WM_NEXTDLGCTL, FALSE, 0 );
            break;
            
        case IDC_RB_MODULES:
            UpdateModuleList();
            // Set focus to main listbox
            //SendMessage( hWndDlg, WM_NEXTDLGCTL, FALSE, 0 );
            break;
            
        case IDC_LB_MAIN_LIST:
            if ( HIWORD(lParam) == LBN_SELCHANGE )
            {
                WORD handle;
                WORD lbSelectedIndex;
                
                lbSelectedIndex =
                    (WORD)SendMessage( HWndMainList, LB_GETCURSEL, 0, 0 );
                handle =
                    (WORD)SendMessage(HWndMainList, LB_GETITEMDATA,
                                        lbSelectedIndex, 0 );
                                    
                if ( IsDlgButtonChecked(hWndDlg, IDC_RB_TASKS) )
                    ShowTaskDetails( (HTASK)handle );
                else
                    ShowModuleDetails( (HMODULE)handle );
            }
            break;
            
        case IDC_LB_DETAILS:
            if ( HIWORD(lParam) == LBN_DBLCLK )
            {
                WORD type, value;
                
                if ( !RetrieveListboxLineTypeAndValue(HWndDetails,
                                                        &type, &value) )
                    break;
                
                switch ( type )
                {
                    case LB_ITEM_HMODULE:
                        ShowModuleDetails( (HMODULE)value ); break;
                    case LB_ITEM_TASK:
                        ShowTaskDetails( (HTASK)value ); break;
                    case LB_ITEM_SEGMENTS:
                        ShowSegmentTableDetails( (HMODULE)value ); break;
                    case LB_ITEM_ENTRY_TABLE:
                        ShowEntryTableDetails( (HMODULE)value ); break;
                    case LB_ITEM_RESOURCES:
                        ShowResourceDetails( (HMODULE)value ); break;
                    case LB_ITEM_RESIDENT_NAMES:
                        ShowResidentNamesDetails( (HMODULE)value ); break;
                    case LB_ITEM_NONRESIDENT_NAMES:
                        ShowNonResidentNamesDetails( (HMODULE)value ); break;
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
    
    CheckDlgButton(hWndDlg, IDC_RB_TASKS, 1);
    
    if ( IsDlgButtonChecked(hWndDlg, IDC_RB_TASKS) )
        UpdateTaskList();   
}

//
// Dialog proc for the main dialog
//
BOOL CALLBACK _export Show16DlgProc(HWND hWndDlg, UINT msg,
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
    }
    return FALSE;
}

void UpdateTaskList(void)
{
    LPTDB lpTDB = 0;
    HTASK headHTask;
    char szModName[10];
    char szBuffer[64];
    unsigned lbIndex = 0;
    
    SendMessage(HWndMainList, LB_RESETCONTENT, 0, 0);
    
    GetCurrentTask();
    __asm mov [headHTask], DX

    lpTDB = MAKELP( headHTask, 0 );

    while ( lpTDB )
    {
        memcpy( szModName, lpTDB->TDB_ModName, 8 );
        szModName[8] = 0;

        wsprintf( szBuffer, "%s %s", szModName,
            lpTDB->TDB_flags & TDB_FLAGS_WIN32 ? "(Win32)" : "" );

        SendMessage( HWndMainList, LB_ADDSTRING, 0, (LPARAM)szBuffer );
        SendMessage( HWndMainList, LB_SETITEMDATA,
                        lbIndex, (LPARAM)SELECTOROF(lpTDB) );
        lbIndex++;
        
        lpTDB = MAKELP( lpTDB->TDB_next, 0 );
    }
    
    // Set selection to first task in list, and show its details
    SendMessage( HWndMainList, LB_SETCURSEL, 0, 0 );
    PostMessage( GetParent(HWndMainList), WM_COMMAND, IDC_LB_MAIN_LIST,
                    MAKELPARAM(HWndMainList, LBN_SELCHANGE) );
}

void UpdateModuleList(void)
{
    LPMODULE lpMODULE = 0;
    HMODULE headHModule;
    char szBuffer[32];
    LPSTR lpszModuleName;
    unsigned lbIndex = 0;
    WORD selector;
    
    SendMessage(HWndMainList, LB_RESETCONTENT, 0, 0);

    //
    // First we're going to walk the regular 16 bit module list.
    //
    GetModuleHandle("KERNEL");   // There will always be a "KERNEL" module
    __asm mov [headHModule], DX

    lpMODULE = MAKELP( headHModule, 0 );
    while ( lpMODULE )
    {
        lpszModuleName = MAKELP(SELECTOROF(lpMODULE),
                                lpMODULE->ne_resNamesTab );

        strncpy( szBuffer, lpszModuleName + 1, *lpszModuleName );
        szBuffer[ *lpszModuleName ] = 0;
        
        SendMessage( HWndMainList, LB_ADDSTRING, 0, (LPARAM)szBuffer );
        SendMessage( HWndMainList, LB_SETITEMDATA,
                        lbIndex, (LPARAM)SELECTOROF(lpMODULE) );
        lbIndex++;

        lpMODULE = MAKELP( lpMODULE->ne_npNextExe, 0 );
    }

    //
    // Now, let's go find the pseudo 16-bit modules created for Win32
    // EXEs and DLLs.
    for ( selector = 7; ; selector += 8 )
    {
        if ( IsA32BitHMODULE(selector) )
        {
            GetModuleName( selector, szBuffer, sizeof(szBuffer) );
            lbprintf( HWndMainList, "%s (Win32)", szBuffer );
            SendMessage( HWndMainList, LB_SETITEMDATA, lbIndex,
                        (LPARAM)selector );
                lbIndex++;
        }
        if ( selector == 0xFFFF )
            break;
    }

    // Set selection to first module in list, and show its details
    SendMessage( HWndMainList, LB_SETCURSEL, 0, 0 );
    PostMessage( GetParent(HWndMainList), WM_COMMAND, IDC_LB_MAIN_LIST,
                    MAKELPARAM(HWndMainList, LBN_SELCHANGE) );
}

BYTE HookedInts[7] = {0, 2, 4, 6, 7, 0x3E, 0x75};

WORD_FLAGS ErrorModeFlags[] = 
{
{ SEM_FAILCRITICALERRORS, "FailCriticalErrors" },
{ SEM_NOGPFAULTERRORBOX, "NoGPFaultErrorBox" },
{ SEM_NOOPENFILEERRORBOX, "NoOpenFileErrorBox" },
};

void ShowTaskDetails(HTASK hTask)
{
    LPTDB lpTDB = MAKELP( hTask, 0 );
    char szBuffer[260];
    unsigned i;
    
    if ( !IsTask(hTask) )
        return;
    
    wsprintf(szBuffer, "Task: %04X (%s)", hTask, GetTaskModuleName(hTask) );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    lbprintf( HWndDetails, "ModuleName: %.8s", lpTDB->TDB_ModName );
        
    GetModuleFileName( lpTDB->TDB_HMODULE, szBuffer, sizeof(szBuffer) );
    lbprintf( HWndDetails, "+ HMODULE: %04Xh %s",
                lpTDB->TDB_HMODULE, szBuffer ); 
    RecordListboxLineTypeAndValue(  HWndDetails, LB_ITEM_HMODULE,
                                    lpTDB->TDB_HMODULE );
    lbprintf( HWndDetails, "HINSTANCE: %04X", lpTDB->TDB_HInstance );

    lbprintf( HWndDetails, "CurrentDir: %c:%s",
                'A' + lpTDB->TDB_Drive - 0x80,
                lpTDB->TDB_current_directory );

    lbprintf( HWndDetails, "ExpWinVer: %u.%02u",
            HIBYTE(lpTDB->TDB_ExpWinVer), LOBYTE(lpTDB->TDB_ExpWinVer) );

    lbprintf( HWndDetails, "PSP: %04X", lpTDB->TDB_PSP );

    lbprintf( HWndDetails, "DOS DTA: %04X:%04X",
                    SELECTOROF(lpTDB->TDB_DTA),
                    OFFSETOF(lpTDB->TDB_DTA) );

    lbprintf( HWndDetails, "Queue: %04X", lpTDB->TDB_Queue );
    lbprintf( HWndDetails, "+ Parent: %04X %s",
        lpTDB->TDB_Parent, GetTaskModuleName(lpTDB->TDB_Parent) );
    RecordListboxLineTypeAndValue(  HWndDetails, LB_ITEM_TASK,
                                    lpTDB->TDB_Parent );
    
    lbprintf( HWndDetails, "TaskFlags: %04X", lpTDB->TDB_flags );
    if ( lpTDB->TDB_flags & TDB_FLAGS_WIN32 )
        lbprintf( HWndDetails, "    Win32 task" );
    if ( lpTDB->TDB_flags & TDB_FLAGS_WINOLDAP )
        lbprintf( HWndDetails, "    Winoldap task (DOS)" );
        
    lbprintf( HWndDetails, "ErrMode: %04X", lpTDB->TDB_ErrMode );
    for ( i=0; i < sizeof(ErrorModeFlags)/sizeof(WORD_FLAGS); i++ )
        if ( ErrorModeFlags[i].value & lpTDB->TDB_ErrMode )
            lbprintf( HWndDetails, "    %s", ErrorModeFlags[i].name );

    lbprintf( HWndDetails, "CompatFlags: %04X", lpTDB->TDB_CompatFlags );
    lbprintf( HWndDetails, "FS Selector: %04X", lpTDB->TDB_FS_selector );
    lbprintf( HWndDetails, "Ring 3 thread database: %08lX",
                            lpTDB->TDB_ring3_thread_db );
    lbprintf( HWndDetails, "Thunk SS: %04X", lpTDB->TDB_thunk_stack_ss );

    lbprintf( HWndDetails, "MakeProcInstance selector: %04X",
                lpTDB->TDB_MPI_Sel );
                
    lbprintf( HWndDetails, "SS:SP: %04X:%04X",
                lpTDB->TDB_taskSS, lpTDB->TDB_taskSP );

    lbprintf( HWndDetails, "FP CtrlWord: %04X", lpTDB->TDB_FCW );
    lbprintf( HWndDetails, "SigAction: %04X", lpTDB->TDB_SigAction );
    lbprintf( HWndDetails, "USER SignalProc: %04X:%04X",
                    SELECTOROF(lpTDB->TDB_USignalProc),
                    OFFSETOF(lpTDB->TDB_USignalProc) );
    lbprintf( HWndDetails, "GlobalNotify Proc: %04X:%04X",
                    SELECTOROF(lpTDB->TDB_GNotifyProc),
                    OFFSETOF(lpTDB->TDB_GNotifyProc) );
                
    for ( i=0; i < 7; i++ )
        lbprintf( HWndDetails, "INT %Xh handler: %04X:%04X",
                    HookedInts[i],
                    SELECTOROF(lpTDB->TDB_INTVECS[i]),
                    OFFSETOF(lpTDB->TDB_INTVECS[i]) );

    lbprintf( HWndDetails, "Events: %u", lpTDB->TDB_nEvents );
    lbprintf( HWndDetails, "Priority: %u", lpTDB->TDB_priority );

    // All these fields should be 0.  If they aren't, we want to know
    // about them PRONTO!  There may be useful information in them.
    assert( !lpTDB->TDB_thread_ordinal );
    assert( !lpTDB->TDB_thread_next );
    assert( !lpTDB->TDB_thread_tdb );
    assert( !lpTDB->TDB_thread_list );
    assert( !lpTDB->TDB_thread_free );
    assert( !lpTDB->TDB_thread_count );
    assert( !lpTDB->TDB_ASignalProc );
    assert( !lpTDB->TDB_filler[0] );
    assert( !lpTDB->TDB_filler[1] );
    assert( !lpTDB->TDB_filler[2] );
    assert( !lpTDB->TDB_Directory[0] );
    assert( !lpTDB->TDB_unused1 );
    assert( !lpTDB->TDB_unused2 );
}

WORD_FLAGS ModuleFlags[] = 
{
{ MODFLAGS_DLL,         "DLL" },
{ MODFLAGS_CALL_WEP,    "CallWep" },
{ 0x1000,               "0x1000" },
{ MODFLAGS_SELF_LOADING,"SelfLoading" },
{ 0x0400,               "0x0400" },
{ 0x0080,               "0x0080" },
{ 0x0040,               "ImplicitLoad" },
{ 0x0020,               "0x0020" },
{ MODFLAGS_WIN32,       "Win32" },
{ 0x0008,               "0x0008" },
{ 0x0004,               "0x0004" },
{ MODFLAGS_AUTODATA,    "PerProcessData" },
{ MODFLAGS_SINGLEDATA,  "SharedData" } 
};

void ShowModuleDetails(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    char szBuffer[260];
    OFSTRUCT_EXT FAR * lpofstruct;
    unsigned i;
    
    if ( !IsModule(hModule) )
        return;

    wsprintf(szBuffer, "Module: %04X", hModule);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    lpofstruct = MAKELP( hModule, lpMODULE->ne_npFileInfo );
    lbprintf( HWndDetails, "Filename: %s", lpofstruct->szPathName );
    lbprintf( HWndDetails, "+ Segments: %Xh", lpMODULE->ne_cseg );
    RecordListboxLineTypeAndValue(  HWndDetails, LB_ITEM_SEGMENTS, hModule );

    lbprintf(HWndDetails, "+ Resources: %04Xh", lpMODULE->ne_rsrcTab);
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_RESOURCES, hModule );

    lbprintf(HWndDetails, "+ ResidentNames: %04Xh", lpMODULE->ne_resNamesTab);
    RecordListboxLineTypeAndValue(HWndDetails, LB_ITEM_RESIDENT_NAMES,
                                    hModule );

    lbprintf(HWndDetails, "+ Non-ResidentNames Offs: %lXh  size: %04X",
            lpMODULE->ne_nonResNamesTab, lpMODULE->ne_cbNonResNamesTab );
    RecordListboxLineTypeAndValue(HWndDetails, LB_ITEM_NONRESIDENT_NAMES,
                                    hModule );

    lbprintf( HWndDetails, "+ EntryTableOffs: %04Xh",
                lpMODULE->ne_npEntryTable );
    RecordListboxLineTypeAndValue(HWndDetails, LB_ITEM_ENTRY_TABLE, hModule );

    lbprintf( HWndDetails, "Usage: %Xh", lpMODULE->ne_usage );
    lbprintf( HWndDetails, "Flags: %02Xh", lpMODULE->ne_flags );
    for ( i=0; i < sizeof(ModuleFlags)/sizeof(WORD_FLAGS); i++ )
        if ( ModuleFlags[i].value & lpMODULE->ne_flags )
            lbprintf( HWndDetails, "    %s", ModuleFlags[i].name );
        
    lbprintf( HWndDetails, "ExpWinVer: %u.%02u",
            HIBYTE(lpMODULE->ne_expver), LOBYTE(lpMODULE->ne_expver) );
    lbprintf( HWndDetails, "DGROUP index: %Xh", lpMODULE->ne_autodata );
    lbprintf( HWndDetails, "Local heap size: %Xh bytes", lpMODULE->ne_heap );
    lbprintf( HWndDetails, "Stack size: %Xh bytes", lpMODULE->ne_stack );
    lbprintf( HWndDetails, "Entry point: %04X:%04X",
                    SELECTOROF(lpMODULE->ne_csip),
                    OFFSETOF(lpMODULE->ne_csip) );
    lbprintf( HWndDetails, "Initial SS:SP: %04X:%04X",
                    SELECTOROF(lpMODULE->ne_sssp),
                    OFFSETOF(lpMODULE->ne_sssp) );
                
    if ( lpMODULE->ne_cModules )
    {
        LPWORD lpModuleList = MAKELP(hModule, lpMODULE->ne_modRefTab);
        
        lbprintf( HWndDetails, "imported modules:");
        for ( i=0; i <  lpMODULE->ne_cModules; i++ )
        {
            GetModuleName( lpModuleList[i], szBuffer, sizeof(szBuffer) );
            lbprintf( HWndDetails, "    + %s", szBuffer );
            RecordListboxLineTypeAndValue(  HWndDetails, LB_ITEM_HMODULE,
                                            lpModuleList[i] );
        }
    }
    
    // The fields in this "if" statement are only present for the
    // pseudo Win32 modules
    if ( lpMODULE->ne_flags & MODFLAGS_WIN32 )
    {
        lbprintf( HWndDetails, "Win32 Base Addr: %08lX",
                    lpMODULE->ne_Win32BaseAddr1 );
        assert ( lpMODULE->ne_Win32BaseAddr1 == lpMODULE->ne_Win32BaseAddr2 );

        lbprintf( HWndDetails, "Win32 Resource Addr: %08lX",
                    lpMODULE->ne_Win32ResourceAddr );       
    }
    
    lbprintf( HWndDetails, "alignment: %u bytes", 1 << lpMODULE->ne_align );
    lbprintf( HWndDetails, "OtherFlags: %02X", lpMODULE->ne_flagsother );
    lbprintf( HWndDetails, "ImportedNames2: %04X",
            lpMODULE->ne_importedNamesTab2 );
    lbprintf( HWndDetails, "ImportedNames3: %04X",
            lpMODULE->ne_importedNamesTab3 );

    lbprintf( HWndDetails, "SwapArea: %lXh bytes",
                lpMODULE->ne_swaparea * 16L );
            
    // These fields should be 0.  If they aren't, we want to know
    // about them PRONTO!  There may be useful information in them.
    assert ( !lpMODULE->ne_cres );
}

void ShowSegmentTableDetails(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    LPSEGMENT_RECORD pSeg;
    char szBuffer[260];
    char szModuleName[32];
    unsigned i;
    
    if ( !IsModule(hModule) )
        return;

    if ( lpMODULE->ne_cseg == 0 )   // Make sure there's something to see...
        return;
    
    GetModuleName( hModule, szModuleName, sizeof(szModuleName) );
    wsprintf(szBuffer, "Segments for module %04X (%s)",
                hModule, szModuleName);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    // The array of segment records starts right after the 40h NE header.
    pSeg = (LPSEGMENT_RECORD)MAKELP( hModule, lpMODULE->ne_segtab );
    
    lbprintf( HWndDetails, "SEG HNDL TYPE SIZE" );
        
    for ( i=0; i < lpMODULE->ne_cseg; i++ )
    {
        lbprintf( HWndDetails, "%02X:   %04X %s %04X bytes",
            i+1, pSeg->handle, pSeg->flags & 1 ? "DATA" : "CODE",
            pSeg->alloc_size );
        pSeg++;
    }
}

void ShowEntryTableDetails(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    LPENTRY_BUNDLE_HEADER lpBundleHdr;
    LPENTRY lpEntry;
    char szBuffer[260];
    char szModuleName[32];
    unsigned i;
    
    if ( !IsModule(hModule) )
        return;

    if ( lpMODULE->ne_npEntryTable == 0 )
        return;

    lpBundleHdr = MAKELP( hModule, lpMODULE->ne_npEntryTable );
    
    // Are there any entries to show???
    if ( lpBundleHdr->lastEntry == lpBundleHdr->firstEntry )
        return;
    
    GetModuleName( hModule, szModuleName, sizeof(szModuleName) );
    wsprintf(szBuffer, "Entry table for module %04X (%s)",
                hModule, szModuleName);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    while ( TRUE )
    {
        unsigned cEntries = lpBundleHdr->lastEntry - lpBundleHdr->firstEntry;

        lpEntry = (LPENTRY)(lpBundleHdr+1); // Entries start after bundle hdr
        
        for ( i=0; i < cEntries; i++ )
        {
            lbprintf( HWndDetails, "%3u: %02X:%04X  %s %s",
                        lpBundleHdr->firstEntry + 1 + i,
                        lpEntry->segNumber, lpEntry->offset,
                        lpEntry->segType == 0xFF ? "MOVEABLE" : "FIXED",
                        lpEntry->flags & 1 ? "EXPORTED" : "" );

            lpEntry++;
        }

        if ( lpBundleHdr->nextBundle == 0 )
            break;      
        lpBundleHdr = MAKELP( hModule, lpBundleHdr->nextBundle );
    }
}

void ShowResourceDetails(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    LPRESOURCE_TYPE lpRsrcType, lpRsrcEnd;
    LPRESOURCE_INFO lpRsrcInfo;
    LPBYTE lpName;
    char szBuffer[260];
    char szModuleName[32];
    unsigned i;
    
    if ( !IsModule(hModule) )
        return;

    if ( lpMODULE->ne_rsrcTab == lpMODULE->ne_resNamesTab )
        return;

    GetModuleName( hModule, szModuleName, sizeof(szModuleName) );
    wsprintf(szBuffer, "Resources for module %04X (%s)",
                hModule, szModuleName);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    // Get a pointer to the first resource type header.  The "+2" skips
    // past the alignment WORD at the beginning
    lpRsrcType = MAKELP( hModule, lpMODULE->ne_rsrcTab + 2);
    lpRsrcEnd = MAKELP( hModule, lpMODULE->ne_resNamesTab );
    
    while ( lpRsrcType < lpRsrcEnd )
    {
        if ( lpRsrcType->ID == 0 )
            break;

        if ( lpRsrcType->ID & 0x8000 )      // A predefined standard resource
        {
            lbprintf( HWndDetails, "%s - Handler: %04X:%04X",
                        GetResourceTypeName( lpRsrcType->ID ),
                        HIWORD(lpRsrcType->function),
                        LOWORD(lpRsrcType->function) );
        }
        else    // A named resource
        {
            lpName = MAKELP(hModule, lpMODULE->ne_rsrcTab + lpRsrcType->ID );
            memcpy( szBuffer, lpName+1, *lpName );
            szBuffer[*lpName] = 0;
            lbprintf( HWndDetails, "NamedResource %s - Handler: %04X:%04X",
                        szBuffer,
                        HIWORD(lpRsrcType->function),
                        LOWORD(lpRsrcType->function) );
        }
        
        lpRsrcInfo = (LPRESOURCE_INFO)(lpRsrcType+1);

        for ( i = 0; i < lpRsrcType->count; i++ )
        {
            // If it's a named resource, so retrieve the string.  The
            // ID value is an offset within the resource table.
            if ( !(lpRsrcInfo->ID & 0x8000) )
            {
                lpName = MAKELP(hModule, lpMODULE->ne_rsrcTab
                                        + lpRsrcInfo->ID );
                memcpy( szBuffer, lpName+1, *lpName );
                szBuffer[*lpName] = 0;
            }
            else    // An integer ID resource
                wsprintf(szBuffer, "%u", lpRsrcInfo->ID);
            
            lbprintf( HWndDetails,
                "  offs: %04X  len: %04Xh  ID: %s  hndl: %04X",
                lpRsrcInfo->offset, lpRsrcInfo->length,
                szBuffer, lpRsrcInfo->handle );

            lpRsrcInfo++;   // Point at next resource of this type
        }
        
        lpRsrcType = (LPRESOURCE_TYPE)  // Point at next resource type
                    ((LPBYTE)lpRsrcType + sizeof(RESOURCE_TYPE)
                    + (lpRsrcType->count * sizeof(RESOURCE_INFO)));
    }
}

void ShowResidentNamesDetails(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    LPBYTE lpNameEntry;
    char szBuffer[260];
    char szModuleName[32];

    if ( !IsModule(hModule) )
        return;

    GetModuleName( hModule, szModuleName, sizeof(szModuleName) );
    wsprintf(szBuffer, "Resident names for module %04X (%s)",
                hModule, szModuleName);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    lpNameEntry = MAKELP( hModule, lpMODULE->ne_resNamesTab );

    //  Each entry is as follows:
    //  BYTE    - length of string that follows
    //  char[?] - the name.  Size is given by previous field
    //  WORD    - the export ordinal for this function
    
    while ( TRUE )
    {
        if ( *lpNameEntry == 0 )    // A 0-length terminates the table
            break;

        memcpy(szBuffer, lpNameEntry+1, *lpNameEntry);
        szBuffer[ *lpNameEntry ] = 0;
        lbprintf( HWndDetails, "%03u %s",
                    *(LPWORD)(lpNameEntry + *lpNameEntry + 1), szBuffer );
        lpNameEntry += (*lpNameEntry + 3);
    }
}

void ShowNonResidentNamesDetails(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    LPBYTE lpNameEntry, lpNRNTable;
    char szBuffer[260];
    char szModuleName[32];
    HFILE hFile = HFILE_ERROR;
    
    if ( !IsModule(hModule) )
        return;

    // Open up the file, seek to the non-resident names, allocate
    // memory for the buffer, and read the table in.  Does error checking.
    if ( !GetModuleFileName(hModule, szBuffer, sizeof(szBuffer)) )
        return;
    hFile = _lopen( szBuffer, READ );
    if ( HFILE_ERROR == hFile )
        return;
    if ( HFILE_ERROR==_llseek(hFile, lpMODULE->ne_nonResNamesTab, SEEK_SET))
        goto done;
    lpNRNTable = malloc( lpMODULE->ne_cbNonResNamesTab );
    if ( !lpNRNTable )
        goto done;
    if ( _lread(hFile, lpNRNTable, lpMODULE->ne_cbNonResNamesTab)
            != lpMODULE->ne_cbNonResNamesTab )
        goto done;

    lpNameEntry = lpNRNTable;
    
    GetModuleName( hModule, szModuleName, sizeof(szModuleName) );
    wsprintf(szBuffer, "Non-resident names for module %04X (%s)",
                hModule, szModuleName);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    //  Each entry has the same format as the resident names table

    while ( TRUE )
    {
        if ( *lpNameEntry == 0 )    // A 0-length terminates the table
            break;

        memcpy(szBuffer, lpNameEntry+1, *lpNameEntry);
        szBuffer[ *lpNameEntry ] = 0;
        lbprintf( HWndDetails, "%03u %s",
                    *(LPWORD)(lpNameEntry + *lpNameEntry + 1), szBuffer );
        lpNameEntry += (*lpNameEntry + 3);
    }
    
done:
    // Clean up things we created
    if ( HFILE_ERROR != hFile )
        _lclose( hFile );
    if ( lpNRNTable )
        free( lpNRNTable );
}

void lbprintf(HWND hWnd, char * format, ...)
{
    char szBuffer[512];
    va_list argptr;
          
    va_start(argptr, format);
    vsprintf(szBuffer, format, argptr);
    va_end(argptr);

    SendMessage( hWnd, LB_ADDSTRING, 0, (LPARAM)szBuffer );
}

// Records the type (module, task, etc...) of the line that was just
// added to the specified listbox window, along with the value.
void RecordListboxLineTypeAndValue(HWND hWnd, WORD type, WORD value)
{
    unsigned lastIndex = (WORD)SendMessage( hWnd, LB_GETCOUNT, 0, 0 );
    if ( !lastIndex )
        return;
    
    lastIndex--;    // Index is 0 based
        
    SendMessage( hWnd, LB_SETITEMDATA, lastIndex, MAKELONG(value, type) );
}

BOOL RetrieveListboxLineTypeAndValue(HWND hWnd, WORD *type, WORD *value)
{
    DWORD itemData;
    unsigned index = (WORD)SendMessage( hWnd, LB_GETCURSEL, 0, 0 );

    itemData = SendMessage( hWnd, LB_GETITEMDATA, index, 0 );
    if ( !itemData && itemData == (DWORD)LB_ERR )
        return FALSE;

    *type = HIWORD( itemData );
    *value = LOWORD( itemData );
    
    return TRUE;
}

char *ResourceTypes[] =
{
    "",
    "Cursor",           // 1
    "Bitmap",           // 2
    "Icon",             // 3
    "Menu",             // 4
    "Dialog",           // 5
    "String Table",     // 6
    "Font Directory",   // 7
    "Font",             // 8
    "Accelerator",      // 9
    "RC Data",          //10
    "Error Table",      //11
    "Group Cursor",     //12
    "Unknown",          //13
    "Group Icon",       //14
    "Name Table",       //15
    "Version info"      //16
};

LPSTR GetResourceTypeName(WORD id)
{
    static char szBuffer[64];
    
    if ( (id & 0x8000) == 0 )
        return "NamedResource";
    
    id &= 0x7FFF;   // Turn off high bit
    
    if ( (id >= 1) && (id <= 16) )
        return ResourceTypes[id];
    
    if ( id == 0xCC )
        return "TrueType Font";
    
    wsprintf( szBuffer, "Unknown_%u", id );
    return szBuffer;
}

BOOL IsModule(HMODULE hModule)
{
    LPMODULE lpMODULE = MAKELP( hModule, 0 );
    
    if ( IsBadReadPtr( lpMODULE, 2) )
        return FALSE;
    
    return lpMODULE->ne_signature == 0x454E;    // Is 'NE' signature there?
}

BOOL IsA32BitHMODULE(WORD selector)
{
    LPMODULE lpMODULE;

    __asm {
        lar     AX, [selector]
        jc      false
        cmp     ah, 0F3h // Present, ring 3, writeable, accessed data segment
        jne     false;
    }
    
    lpMODULE = MAKELP( selector, 0 );
    if ( IsBadReadPtr(lpMODULE, sizeof(MODULE)) )   // Is the seg big enough?
        return FALSE;

    if ( lpMODULE->ne_signature != 0x454E ) // look for NE signature
        return FALSE;

    if ( GetSelectorBase(selector) < 0x80000000LU ) // IMTE is in system
        return FALSE;                               // memory > 2GB

    if ( lpMODULE->ne_align != 0 )          // alignment is always 1 in 32 bit
        return FALSE;                       // modules

    if ( lpMODULE->ne_cseg != 0 )           // No segments in 32 bit modules
        return FALSE;

    if ( lpMODULE->ne_usage != 1 )  // Usage will always be 1
        return TRUE;
    
    if ( lpMODULE->ne_flags == 0x8010 ) // Are DLL and Win32 flags set?
        return TRUE;

false:
    return FALSE;
}

LPSTR GetTaskModuleName(HTASK hTask)
{
    LPTDB lpTDB = MAKELP( hTask, 0 );
    static char szBuffer[10];
    
    if ( !IsTask(hTask) )
        return "<unknown TDB>";
    memcpy( szBuffer, lpTDB->TDB_ModName, 8 );
    szBuffer[8] = 0;
    return szBuffer;
}

