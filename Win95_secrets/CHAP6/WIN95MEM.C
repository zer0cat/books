//=================================
// WIN95MEM - Matt Pietrek 1995
// FILE: WIN95MEM.C
//=================================
#include <windows.h>
#include "win95mem.h"

// The DemandInfoStruc struct below is excerpted from the VMM.H file
// in the Windows 95 DDK

struct DemandInfoStruc {
    ULONG DILin_Total_Count;    /* # pages in linear address space */
    ULONG DIPhys_Count;         /* Count of phys pages */
    ULONG DIFree_Count;         /* Count of free phys pages */
    ULONG DIUnlock_Count;       /* Count of unlocked Phys Pages */
    ULONG DILinear_Base_Addr;   /* Base of pageable address space */
    ULONG DILin_Total_Free;     /* Total Count of free linear pages */

    /*
     *  The following 5 fields are all running totals, kept from the time
     *  the system was started
     */
    ULONG DIPage_Faults;        /* total page faults */
    ULONG DIPage_Ins;           /* calls to pagers to page in a page */
    ULONG DIPage_Outs;          /* calls to pagers to page out a page*/
    ULONG DIPage_Discards;      /* pages discarded w/o calling pager */
    ULONG DIInstance_Faults;    /* instance page faults */

    ULONG DIPagingFileMax;      /* maximum # of pages that could be in paging file */
    ULONG DIPagingFileInUse;    /* # of pages of paging file currently in use */

    ULONG DICommit_Count;       /* Total committed memory, in pages */

    ULONG DIReserved[2];        /* Reserved for expansion */
};

DWORD WINAPI VxDCall2( DWORD service_number, DWORD, DWORD );

void Handle_WM_TIMER(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    struct DemandInfoStruc dis;
    char szBuffer[256];

    // Demonstrate calling a Win32 VxD service (in this case, the
    // _GetDemandPageInfo service.
    VxDCall2( 0x0001001E, (DWORD)&dis, 0 );
    
    wsprintf(szBuffer, "Comm: %uK", dis.DICommit_Count * 4);
    SetDlgItemText( hWndDlg, IDC_TEXT_commited, szBuffer );

    wsprintf(szBuffer, "Phys: %uK", dis.DIPhys_Count * 4);
    SetDlgItemText( hWndDlg, IDC_TEXT_physical, szBuffer );

    wsprintf(szBuffer, "%u%%",
                (dis.DICommit_Count * 100) / dis.DIPhys_Count);
    SetDlgItemText( hWndDlg, IDC_TEXT_percentage, szBuffer );
}

BOOL CALLBACK Win95MemDlgProc(HWND hWndDlg, UINT msg,
                              WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_INITDIALOG:
            SetTimer( hWndDlg, 0, 1000, 0 ); return TRUE;
        case WM_TIMER:
            Handle_WM_TIMER(hWndDlg, wParam, lParam); return TRUE;
        case WM_CLOSE:
            KillTimer(hWndDlg, 0);
            EndDialog(hWndDlg, 0);
            return FALSE;
    }
    
    return FALSE;
}

int APIENTRY WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                        LPSTR lpszCmdLine, int nCmdShow )
{
    DialogBox(hInstance, "WIN95MEM_DLG", 0, (DLGPROC)Win95MemDlgProc );
    return 00;
}

