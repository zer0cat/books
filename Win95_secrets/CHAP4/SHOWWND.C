//==================================
// SHOWWND - Matt Pietrek 1995
// FILE: SHOWWND.C
//==================================
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <stddef.h>
#pragma hdrstop
#include "mbassert.h"
#include "hwnd32.h"
#include "wndclass.h"
#include "msgqueue.h"
#include "ShowWnd.h"

// ========================== Function Prototypes =========================
// Undocumented KERNEL32 functions we need
HMODULE WINAPI LoadLibrary16( LPCSTR lpLibFileName );
BOOL WINAPI FreeLibrary16( HINSTANCE hLibModule );
void WINAPI EnterSysLevel( PVOID pMutex );
void WINAPI LeaveSysLevel( PVOID pMutex );
void WINAPI GetpWin16Lock( PVOID * pWin16Mutex ); 


// Helper functions
void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
void Handle_WM_INITDIALOG(HWND hWndDlg);
void Handle_WM_DELETEITEM(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ShowWndDlgProc(HWND, UINT, WPARAM, LPARAM);
void RecordListboxLineTypeAndValue(HWND hWnd, DWORD type, DWORD value);
BOOL RetrieveListboxLineTypeAndValue(HWND hWnd, DWORD *type, DWORD *value);
void UpdateWndList(void);
void ShowWndDetails( PWND32 pWnd );
void ShowClassDetails( PUSER_DGROUP_WNDCLASS pWndClass );
void ShowQueueDetails( WORD hQueue );
void lbprintf(HWND hWnd, char * format, ...);
void WalkAtLevel( PWND32 pWnd, unsigned level );
DWORD GetUserHeapBase(void);
PWND32 HWndToPtr( HWND hWnd );
PWND32 UserPtrToFlatPtr( PWND32 );
void GetProcessNameFromHQueue( WORD hQueue, PSTR szBuffer );
BOOL IsWin32Task( WORD hQueue );
PVOID ConvertFar16PtrToFlat( DWORD );
BOOL GetClassNameFromAtom( WORD atom, PSTR pszBuffer, unsigned cbLen );

// ====================== Global Variables =============================
DWORD UserHeapBase = 0;

// HWNDs of the commonly used dialog controls
HWND HWndMainList;
HWND HWndDetails;
HWND HWndDetailsDescription;

// ======================= Start of code ===============================

int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow )
{
    DialogBox(hInstance, "ShowWndDlg", 0, (DLGPROC)ShowWndDlgProc);
    return 0;
}

void UpdateWndList(void)
{
    PWND32 pWnd;
    PVOID pWin16Mutex;
    
    pWnd = HWndToPtr( GetDesktopWindow() );
    if ( !pWnd )
        return;
    
    SendMessage( HWndMainList, LB_RESETCONTENT, 0, 0 );
    SendMessage( HWndMainList, WM_SETREDRAW, FALSE, 0 );    // Turn off redraws

    // Grab the Win16Mutex so that we don't get interrupted during our
    // traversal of the tree.
    GetpWin16Lock( &pWin16Mutex );
    EnterSysLevel( pWin16Mutex );
    
    WalkAtLevel( pWnd, 0 );                                 // Walk wnd list

    // Release the Win16Mutex, since we're done walking
    LeaveSysLevel( pWin16Mutex );

    SendMessage( HWndMainList, WM_SETREDRAW, TRUE, 0 );     // Turn on redraws
    
    // Set selection to first window in list, and show its details
    SendMessage( HWndMainList, LB_SETCURSEL, 0, 0 );
    PostMessage( GetParent(HWndMainList), WM_COMMAND,
                    MAKEWPARAM(IDC_LB_MAIN_LIST, LBN_SELCHANGE),
                    (LPARAM)HWndMainList );

}

void WalkAtLevel( PWND32 pWnd, unsigned level )
{
    while ( pWnd )
    {
        char szOutBuff[384];
        char szTaskName[12];
        unsigned i;
        
        szOutBuff[0] = 0;   // Null out the output buffer

        // Get the name of the window's associated task
        GetProcessNameFromHQueue(  pWnd->hQueue, szTaskName );
            
        for ( i=0; i < level; i++ )
            strcat( szOutBuff, "\t" );
        
        wsprintf( szOutBuff + lstrlen(szOutBuff), "%04X %s (%u)",
                    pWnd->hWnd16, szTaskName,
                    IsWin32Task(pWnd->hQueue) ? 32 : 16 );

        lbprintf( HWndMainList, szOutBuff );
        RecordListboxLineTypeAndValue( HWndMainList, LB_ITEM_WND, (DWORD)pWnd );
        
        if ( pWnd->hWndChild )
            WalkAtLevel( UserPtrToFlatPtr(pWnd->hWndChild), level+1 );

        pWnd = UserPtrToFlatPtr( pWnd->hWndNext );
    }
}

DWORD_FLAGS WndStyles[]=
{
{ 0x80000000L, "WS_POPUP" },
{ 0x40000000L, "WS_CHILD" },
{ 0x20000000L, "WS_MINIMIZE" },
{ 0x10000000L, "WS_VISIBLE" },
{ 0x08000000L, "WS_DISABLED" },
{ 0x04000000L, "WS_CLIPSIBLINGS" },
{ 0x02000000L, "WS_CLIPCHILDREN" },
{ 0x01000000L, "WS_MAXIMIZE" },
{ 0x00800000L, "WS_BORDER" },
{ 0x00400000L, "WS_DLGFRAME" },
{ 0x00200000L, "WS_VSCROLL" },
{ 0x00100000L, "WS_HSCROLL" },
{ 0x00080000L, "WS_SYSMENU" },
{ 0x00040000L, "WS_THICKFRAME" },
{ 0x00020000L, "WS_MINIMIZEBOX" },
{ 0x00010000L, "WS_MAXIMIZEBOX" },
};

DWORD_FLAGS WndExStyles[]=
{
{ 0x00000001L, "WS_EX_DLGMODALFRAME" },
{ 0x00000004L, "WS_EX_NOPARENTNOTIFY" },
{ 0x00000008L, "WS_EX_TOPMOST" },
{ 0x00000010L, "WS_EX_ACCEPTFILES" },
{ 0x00000020L, "WS_EX_TRANSPARENT" },
{ 0x00000040L, "WS_EX_MDICHILD" },
{ 0x00000080L, "WS_EX_TOOLWINDOW" },
{ 0x00000100L, "WS_EX_WINDOWEDGE" },
{ 0x00000200L, "WS_EX_CLIENTEDGE" },
{ 0x00000400L, "WS_EX_CONTEXTHELP" },
{ 0x00001000L, "WS_EX_RIGHT" },
{ 0x00002000L, "WS_EX_RTLREADING" },
{ 0x00004000L, "WS_EX_LEFTSCROLLBAR" },
{ 0x00010000L, "WS_EX_CONTROLPARENT" },
{ 0x00020000L, "WS_EX_STATICEDGE" },
{ 0x00040000L, "WS_EX_APPWINDOW" },
};

void ShowWndDetails( PWND32 pWnd )
{
    char szBuffer[512];
    char szBuffer2[384];
    PBYTE pThunk;
    unsigned i;
    
    if ( IsBadReadPtr( pWnd, offsetof(WND32, hWnd16) + 2 ) )
        return;
    
    if ( !IsWindow( (HWND)pWnd->hWnd16 ) )
        return;

    wsprintf(szBuffer, "PWND: %08X", (DWORD)pWnd - UserHeapBase );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );

    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    SendMessage( HWndDetails, WM_SETREDRAW, FALSE, 0 ); // Turn off redraws

    lbprintf( HWndDetails, "+hWndNext: %08X", pWnd->hWndNext );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_WND,
                                    (DWORD)UserPtrToFlatPtr(pWnd->hWndNext) );

    lbprintf( HWndDetails, "+hWndChild: %08X", pWnd->hWndChild );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_WND,
                                    (DWORD)UserPtrToFlatPtr(pWnd->hWndChild) );
                                
    lbprintf( HWndDetails, "+hWndParent: %08X", pWnd->hWndParent );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_WND,
                                    (DWORD)UserPtrToFlatPtr(pWnd->hWndParent) );
    
    lbprintf( HWndDetails, "+hWndOwner: %08X", pWnd->hWndOwner );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_WND,
                                    (DWORD)UserPtrToFlatPtr(pWnd->hWndOwner) );
    
    lbprintf( HWndDetails, "Rect: (%u,%u)-(%u,%u)",
            pWnd->rectWindow.left, pWnd->rectWindow.top,
            pWnd->rectWindow.right, pWnd->rectWindow.bottom  );

    lbprintf( HWndDetails, "Client: (%u,%u)-(%u,%u)",
            pWnd->rectClient.left, pWnd->rectClient.top,
            pWnd->rectClient.right, pWnd->rectClient.bottom  );

    lbprintf( HWndDetails, "+hQueue: %04X", pWnd->hQueue );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_QUEUE,
                                    pWnd->hQueue );

    GetClassName( (HWND)pWnd->hWnd16, szBuffer2, sizeof(szBuffer2) );
    lbprintf( HWndDetails, "+wndClass: %04X (%s)", pWnd->wndClass,
                szBuffer2 );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_CLASS,
                                    (DWORD)(pWnd->wndClass + UserHeapBase) );
    lbprintf( HWndDetails, "hInstance: %04X", pWnd->hInstance );

    // See if this is a Win16 thunk for a Win32 WNDPROC
    pThunk = ConvertFar16PtrToFlat( (DWORD)pWnd->lpfnWndProc );
    if ( pThunk
        && (pThunk[0] == 0x66) && (pThunk[1] == 0x68)
        && (pThunk[6] == 0x66) && (pThunk[7] == 0x68)
        && (pThunk[0xC] == 0xEA) )
    {
        wsprintf( szBuffer2, " (thunk: %08X)", *(PDWORD)(pThunk+2) );
    }
    else                        // Not a thunk
        szBuffer2[0] = 0;
    
    lbprintf( HWndDetails, "WndProc: %04X:%04X%s", 
                HIWORD(pWnd->lpfnWndProc), LOWORD(pWnd->lpfnWndProc),
                szBuffer2 );
    
    lbprintf( HWndDetails, "hWnd16: %04X", pWnd->hWnd16 );
    lbprintf( HWndDetails, "dwStyleFlags: %08X", pWnd->dwStyleFlags );
    for ( i=0; i < (sizeof(WndStyles)/sizeof(DWORD_FLAGS)); i++ )
        if ( pWnd->dwStyleFlags & WndStyles[i].value )
            lbprintf( HWndDetails, "    %s", WndStyles[i].name );

    lbprintf( HWndDetails, "dwExStyleFlags: %08X", pWnd->dwExStyleFlags );
    for ( i=0; i < (sizeof(WndExStyles)/sizeof(DWORD_FLAGS)); i++ )
        if ( pWnd->dwExStyleFlags & WndExStyles[i].value )
            lbprintf( HWndDetails, "    %s", WndExStyles[i].name );
    
    lbprintf( HWndDetails, "hrgnUpdate: %04X", pWnd->hrgnUpdate );
    lbprintf( HWndDetails, "dwFlags: %08X", pWnd->dwFlags );
    lbprintf( HWndDetails, "moreFlags: %08X", pWnd->moreFlags );
    lbprintf( HWndDetails, "ctrlID: %08X", pWnd->ctrlID );
    lbprintf( HWndDetails, "windowTextOffset: %04X", pWnd->windowTextOffset );
    lbprintf( HWndDetails, "scrollBar: %04X", pWnd->scrollBar );
    lbprintf( HWndDetails, "properties: %04X", pWnd->properties );
    lbprintf( HWndDetails, "lastActive: %08X", pWnd->lastActive );
    lbprintf( HWndDetails, "hMenuSystem: %08X", pWnd->hMenuSystem );
    lbprintf( HWndDetails, "un1: %08X", pWnd->un1 );
    lbprintf( HWndDetails, "un2: %08X", pWnd->un2 );
    lbprintf( HWndDetails, "classAtom: %04X", pWnd->classAtom );

    MBassert( IsDivisibleBy4((DWORD)pWnd->hWndParent) );
    MBassert( Is16BitGlobalHandle( pWnd->hQueue ) );
    MBassert( IsDivisibleBy4(pWnd->wndClass) );
    MBassert( Is16BitGlobalHandle( pWnd->hInstance ) );
    MBassert( IsSelector( HIWORD(pWnd->lpfnWndProc)) );
    MBassert( IsDivisibleBy4(pWnd->hWnd16) );
    MBassert( IsDivisibleBy4((DWORD)pWnd->lastActive) );
        
    SendMessage( HWndDetails, WM_SETREDRAW, TRUE, 0 );  // Turn on redraws
}

DWORD_FLAGS ClassStyles[]=
{
{ 0x0001, "CS_VREDRAW" },
{ 0x0002, "CS_HREDRAW" },
{ 0x0004, "CS_KEYCVTWINDOW" },
{ 0x0008, "CS_DBLCLKS" },
{ 0x0020, "CS_OWNDC" },
{ 0x0040, "CS_CLASSDC" },
{ 0x0080, "CS_PARENTDC" },
{ 0x0100, "CS_NOKEYCVT" },
{ 0x0200, "CS_NOCLOSE" },
{ 0x0800, "CS_SAVEBITS" },
{ 0x1000, "CS_BYTEALIGNCLIENT" },
{ 0x2000, "CS_BYTEALIGNWINDOW" },
{ 0x4000, "CS_GLOBALCLASS" },
{ 0x00010000, "CS_IME" },
};

void ShowClassDetails( PUSER_DGROUP_WNDCLASS pWndClass )
{
    char szBuffer[512];
    unsigned i;
    PINTWNDCLASS pIntWndClass;
        
    if ( (DWORD)pWndClass == UserHeapBase )
    {
        MessageBox( 0, "Not a valid class", 0, MB_OK );
        return;
    }
    
    wsprintf(szBuffer, "class: %04X", (DWORD)pWndClass - UserHeapBase);
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );

    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);

    lbprintf( HWndDetails, "pIntWndClass: %04X:%04X",
                HIWORD(pWndClass->lpIntWndClass),
                LOWORD(pWndClass->lpIntWndClass) );

    lbprintf( HWndDetails, "classNameAtom: %04X", pWndClass->classNameAtom);
        
    lbprintf( HWndDetails, "+hcNext: %04X", pWndClass->hcNext );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_CLASS,
                                    (DWORD)(pWndClass->hcNext + UserHeapBase) );

    lbprintf( HWndDetails, "style: %08X", pWndClass->style );
    for ( i=0; i < (sizeof(ClassStyles)/sizeof(DWORD_FLAGS)); i++ )
        if ( pWndClass->style & ClassStyles[i].value )
            lbprintf( HWndDetails, "    %s", ClassStyles[i].name );

    pIntWndClass = ConvertFar16PtrToFlat( pWndClass->lpIntWndClass );
        
    lbprintf( HWndDetails, "cClsWnds: %08X", pIntWndClass->cClsWnds );
    lbprintf( HWndDetails, "lpfnWndProc: %04X:%04X",
                HIWORD(pIntWndClass->lpfnWndProc),LOWORD(pIntWndClass->lpfnWndProc));
    lbprintf( HWndDetails, "cbClsExtra: %02X", pIntWndClass->cbClsExtra );
    lbprintf( HWndDetails, "hModule: %04X", pIntWndClass->hModule );
    lbprintf( HWndDetails, "hIcon: %04X", pIntWndClass->hIcon );
    lbprintf( HWndDetails, "hCursor: %04X", pIntWndClass->hCursor );
    lbprintf( HWndDetails, "hBrBackground: %04X", pIntWndClass->hBrBackground );

    lbprintf( HWndDetails, "lpszMenuName: %04X:%04X",
                HIWORD(pIntWndClass->lpszMenuName),
                LOWORD(pIntWndClass->lpszMenuName));
    if ( pIntWndClass->lpszMenuName )
    {
        PSTR pszMenuName = ConvertFar16PtrToFlat(pIntWndClass->lpszMenuName);
        if ( !IsBadStringPtr( pszMenuName, 255 ) )
            lbprintf( HWndDetails, "  Menu: %s", pszMenuName );
    }

    lbprintf( HWndDetails, "hIconSm: %04X", pIntWndClass->hIconSm );
    lbprintf( HWndDetails, "cbWndExtra: %04X", pIntWndClass->cbWndExtra );
    
    MBassert( IsDivisibleBy4(pWndClass->hcNext) );
    MBassert( IsSelector( HIWORD(pIntWndClass->lpfnWndProc)) );
    MBassert( Is16BitGlobalHandle(pIntWndClass->hModule) );
    MBassert( Is16BitGlobalHandle(pIntWndClass->hCursor) || !pIntWndClass->hCursor );
    MBassert( Is16BitGlobalHandle(pIntWndClass->hIcon) || !pIntWndClass->hIcon );
    MBassert( Is16BitGlobalHandle(pIntWndClass->hIconSm) || !pIntWndClass->hIconSm );
}

void ShowQueueDetails( WORD hQueue )
{
    PMSGQUEUE pQ;
    char szBuffer[256];
    char szTaskName[12];

    pQ = ConvertFar16PtrToFlat( MAKELONG(0, hQueue) );
    if ( IsBadReadPtr( pQ, 0xA0 ) )
    {
error:
        MessageBox( 0, "Not a valid queue", 0, MB_OK );
        return;
    }
    
    if ( pQ->sig2 != 0x5148 )   // 0x5148 == "HQ"
        goto error;
    
    SendMessage(HWndDetails, LB_RESETCONTENT, 0, 0);
    
    wsprintf(szBuffer, "Queue: %04X", hQueue );
    SendMessage( HWndDetailsDescription, WM_SETTEXT, 0, (LPARAM)szBuffer );
    
    lbprintf( HWndDetails, "+nextQueue: %04X", pQ->nextQueue );
    RecordListboxLineTypeAndValue( HWndDetails, LB_ITEM_QUEUE, pQ->nextQueue);

    // Get the name of the window's associated task
    GetProcessNameFromHQueue(  hQueue, szTaskName );
    lbprintf( HWndDetails, "hTask: %04X (%s)", pQ->hTask, szTaskName );
    
    lbprintf( HWndDetails, "headMsg: %04X", pQ->headMsg );
    lbprintf( HWndDetails, "tailMsg: %04X", pQ->tailMsg );
    lbprintf( HWndDetails, "lastMsg: %04X", pQ->lastMsg );
    lbprintf( HWndDetails, "cMsgs: %04X", pQ->cMsgs );
    lbprintf( HWndDetails, "un1: %02X", pQ->un1 );
    lbprintf( HWndDetails, "sig: %-3.3s", pQ->sig );
    lbprintf( HWndDetails, "npPerQueueData: %04X", pQ->npPerQueue );
    lbprintf( HWndDetails, "un2_5: %04X", pQ->un2_5 );
    lbprintf( HWndDetails, "npQueueProcessData: %04X", pQ->npProcess );
    lbprintf( HWndDetails, "un3[0]: %08X", pQ->un3[0] );
    lbprintf( HWndDetails, "messageTime: %08X", pQ->messageTime);
    lbprintf( HWndDetails, "messagePos: %08X", pQ->messagePos);
    lbprintf( HWndDetails, "lastMsg2: %04X", pQ->lastMsg2 );
    lbprintf( HWndDetails, "extraInfo: %08X", pQ->extraInfo );
    lbprintf( HWndDetails, "threadId: %08X", pQ->threadId );
    lbprintf( HWndDetails, "un6: %04X", pQ->un6 );
    lbprintf( HWndDetails, "expWinVer: %04X", pQ->expWinVer );
    lbprintf( HWndDetails, "un7: %08X", pQ->un7 );
    lbprintf( HWndDetails, "ChangeBits: %04X", pQ->ChangeBits );
    lbprintf( HWndDetails, "WakeBits: %04X", pQ->WakeBits );
    lbprintf( HWndDetails, "WakeMask: %04X", pQ->WakeMask );
    lbprintf( HWndDetails, "hQueueSend: %04X", pQ->hQueueSend );
    lbprintf( HWndDetails, "sig2: %04X", pQ->sig2 );

    MBassert( !pQ->un2 );
    MBassert( !pQ->un3[1] );
    MBassert( !pQ->un3[2] );
    MBassert( !pQ->un4 );
    MBassert( !pQ->un5[0] );
    MBassert( !pQ->un5[1] );
    MBassert( !pQ->un8 );
    MBassert( !pQ->un9 );
    
    MBassert( (IsSelector(pQ->nextQueue) || !pQ->nextQueue) );
    MBassert( Is16BitGlobalHandle(pQ->hTask) );
    MBassert( (IsDivisibleBy4(pQ->headMsg) || !pQ->headMsg) );
    MBassert( (IsDivisibleBy4(pQ->tailMsg) || !pQ->tailMsg) );
    MBassert( IsDivisibleBy4(pQ->npPerQueue) );
    MBassert( IsDivisibleBy4(pQ->npProcess) );
    MBassert( pQ->sig2 == 0x5148 );
}

DWORD GetUserHeapBase(void)
{
    HMODULE hModUser;
    
    hModUser = LoadLibrary16( "USER.EXE" );
    if ( !hModUser )
        return 0;
    
    FreeLibrary16( hModUser );
    
    return (DWORD)ConvertFar16PtrToFlat( MAKELONG(0, hModUser) );
} 

PWND32 HWndToPtr( HWND hWnd )
{
    if ( !IsWindow( hWnd ) )
        return 0;
    
    return (PWND32)(UserHeapBase + 
                    *(PDWORD)(UserHeapBase + 0x10000 + (DWORD)hWnd) );
}

PWND32 UserPtrToFlatPtr( PWND32 pWnd32 )
{
    if ( pWnd32 == 0 )
        return 0;

    return (PWND32)( UserHeapBase + (DWORD)pWnd32 );
}

#if 0
BOOL GetClassNameFromAtom( WORD atom, PSTR pszBuffer, unsigned cbLen )
{
    WORD x;
    PBYTE pszString;
    
    // if ( atom < 0xC000 )
    // {
        wsprintf(pszBuffer, "#%X", atom );
        switch ( atom )
        {
            case 32768: strcat(pszBuffer, " PopupMenu"); break;
            case 32769: strcat(pszBuffer, " Desktop"); break;
            case 32770: strcat(pszBuffer, " Dialog"); break;
            case 32771: strcat(pszBuffer, " WinSwitch"); break;
            case 32772: strcat(pszBuffer, " IconTitle"); break;
        }
        return TRUE;
    // }

    // GlobalGetAtomName( atom, pszBuffer, cbLen );
    
    return TRUE;
}
#endif

//
// This routine is just modification of the GetProcessNameFromHTask
// function in the WIN32WLK.C program.
//
void GetProcessNameFromHQueue( WORD hQueue, PSTR pszBuffer )
{
    pszBuffer[0] = 0;
    
    __try
    {
        __asm
        {
            push ds
            push ds
            pop  es
            mov ds,word ptr [hQueue]    ;; Retrieve the hTask value from
            mov ax,ds:[0002h]           ;; the hQueue
            mov ds, ax
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

BOOL IsWin32Task( WORD hQueue )
{
    BOOL retValue = 0;
    
    __try
    {
        __asm
        {
            push    ds
            mov     ds,word ptr [hQueue]    ;; Retrieve the hTask value from
            mov     ax,ds:[0002h]           ;; the hQueue
            mov     ds, ax
            mov     ax, ds:[16h]
            test    ax, 10h
            setnz   byte ptr [retValue]
            pop     ds
        }
    }
    __except( 1 ){}
    
    return retValue;
}

PVOID ConvertFar16PtrToFlat( DWORD far16ptr )
{
    LDT_ENTRY selectorEntry;
    
    if ( !GetThreadSelectorEntry(GetCurrentThread(), HIWORD(far16ptr),
                                &selectorEntry) )
    {
        return 0;
    }
    
    return  (PVOID)(
            (selectorEntry.HighWord.Bytes.BaseHi << 24) + 
            (selectorEntry.HighWord.Bytes.BaseMid << 16) +
            selectorEntry.BaseLow +
            LOWORD(far16ptr));
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

//
// Dialog proc for the main dialog
//
BOOL CALLBACK ShowWndDlgProc(HWND hWndDlg, UINT msg,
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
        case IDC_BUTTON_EXIT:
            EndDialog(hWndDlg, 0);
            break;
            
        case IDC_BUTTON_REFRESH:
            UpdateWndList();
            break;
            
        case IDC_LB_MAIN_LIST:
            if ( HIWORD(wParam) == LBN_SELCHANGE )
            {
                DWORD handle, type;
                DWORD lbSelectedIndex;
                
                lbSelectedIndex = SendMessage(HWndMainList,LB_GETCURSEL, 0, 0);
                RetrieveListboxLineTypeAndValue(HWndMainList, &type, &handle);
                                    
                ShowWndDetails( (PWND32)handle );
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
                    case LB_ITEM_WND:
                        ShowWndDetails( (PWND32)value ); break;
                    case LB_ITEM_CLASS:
                        ShowClassDetails( (PUSER_DGROUP_WNDCLASS)value );break;
                    case LB_ITEM_QUEUE:
                        ShowQueueDetails( (WORD)value ); break;
                }
            }
            break;
    }
    return;
}

void Handle_WM_INITDIALOG(HWND hWndDlg)
{
    DWORD tabStops=16;
    
    HWndMainList = GetDlgItem(hWndDlg, IDC_LB_MAIN_LIST);
    HWndDetails = GetDlgItem(hWndDlg, IDC_LB_DETAILS);
    HWndDetailsDescription = GetDlgItem(hWndDlg, IDC_DETAILS_TYPE );

    SendMessage( HWndMainList, LB_SETTABSTOPS, 1, (LPARAM)&tabStops );
    
    UserHeapBase = GetUserHeapBase();
    
    UpdateWndList();
}

void Handle_WM_DELETEITEM(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    if ( (wParam != IDC_LB_MAIN_LIST) && (wParam != IDC_LB_DETAILS) )
        return;
    
    // Free the pointer stored in the item data
    free( (PVOID)((LPDELETEITEMSTRUCT)lParam)->itemData );
}

// Our own custom assert for GUI programs
void __cdecl _MBassert(void *pszExp, void *pszFile, unsigned lineNum)
{
    char buffer[512];
    
    wsprintf(buffer, "assert: %s (%s line %u)", pszExp, pszFile, lineNum);
    MessageBox( 0, buffer, 0, MB_OK );
}
