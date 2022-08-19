//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SVFLT.C
//==================================
#include <windows.h>
#include <stdio.h>
#include <string.h>
#pragma hdrstop
#include "w32srvdb.h"
#include "w32svspy.h"
#include "w32svflt.h"

void UpdateFilterDialogServiceList( HWND hWndDlg, unsigned index );
void HandleWin32ServiceListboxDblClk( HWND hWndDlg );

void Handle_FilterDlg_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    switch ( LOWORD(wParam) )
    {
        case IDOK:

            // Fall through to IDCANCEL
        case IDCANCEL:
            EndDialog(hWndDlg, 0); return;
            
        case IDC_FILTER_LB_PROVIDERS:
            if ( HIWORD(wParam) == LBN_SELCHANGE )
            {
                DWORD currSel = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
                if ( currSel != LB_ERR )
                    UpdateFilterDialogServiceList( hWndDlg, currSel );
            }
            break;
            
        case IDC_FILTER_LB_SERVICES:
            if ( HIWORD(wParam) == LBN_DBLCLK )
                HandleWin32ServiceListboxDblClk( hWndDlg );
            break;
    }
}

void Handle_FilterDlg_WM_INITDIALOG(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    unsigned i;
    HWND hWndLb;
    
    hWndLb = GetDlgItem(hWndDlg,IDC_FILTER_LB_PROVIDERS);
    
    for ( i=0; i < CWin32ServiceVxDs; i++ )
    {
        SendMessage( hWndLb,
                    LB_ADDSTRING,
                    (WPARAM)0,
                    (LPARAM)W32ServiceTable[i].pszVxDName );
    }

    // Force the first VXD's services to be shown in the service list
    SendMessage( hWndLb, LB_SETCURSEL, 0, 0 );
    PostMessage( hWndDlg,
                WM_COMMAND,
                MAKEWPARAM(IDC_FILTER_LB_PROVIDERS, LBN_SELCHANGE),
                (LPARAM)hWndLb );
}

void UpdateFilterDialogServiceList( HWND hWndDlg, unsigned index )
{
    unsigned i;
    HWND hWndLb;
    
    hWndLb = GetDlgItem( hWndDlg, IDC_FILTER_LB_SERVICES );
    
    SendMessage( hWndLb, LB_RESETCONTENT, 0, 0 );
    SendMessage( hWndLb, WM_SETREDRAW, FALSE, 0 );

    for ( i=0; i < W32ServiceTable[index].cServiceCalls; i++ )
    {
        char szBuffer[512];
        
        wsprintf(szBuffer, "%c %08X %s",
                    W32ServiceTable[index].pServiceCalls[i].fIgnore ? '-': '+',
                    W32ServiceTable[index].vxdID + i,
                    W32ServiceTable[index].pServiceCalls[i].pszServiceName);

        SendMessage(hWndLb, LB_ADDSTRING, 0, (LPARAM)szBuffer );
    }
        
    SendMessage( hWndLb, WM_SETREDRAW, TRUE, 0 );
}

void HandleWin32ServiceListboxDblClk( HWND hWndDlg )
{
    DWORD currVxD;
    DWORD currServ;
    char szBuffer[512];
    
    currVxD = SendDlgItemMessage(hWndDlg, IDC_FILTER_LB_PROVIDERS,
                                    LB_GETCURSEL, 0, 0);
                            
    if ( currVxD == LB_ERR )
        return;

    currServ = SendDlgItemMessage( hWndDlg, IDC_FILTER_LB_SERVICES,
                                    LB_GETCURSEL, 0, 0 );
                        
    if ( currServ == LB_ERR )
        return;

    // Toggle the state of the fIgnore flag
    W32ServiceTable[currVxD].pServiceCalls[currServ].fIgnore ^= 1;

    SendDlgItemMessage( hWndDlg, IDC_FILTER_LB_SERVICES, LB_GETTEXT, 
                        currServ, (LPARAM)szBuffer );
                    
    szBuffer[0] = (szBuffer[0] == '+') ? '-' : '+';

    SendDlgItemMessage( hWndDlg, IDC_FILTER_LB_SERVICES, LB_DELETESTRING, 
                        (WPARAM)currServ, 0 );
    SendDlgItemMessage( hWndDlg, IDC_FILTER_LB_SERVICES, LB_INSERTSTRING, 
                        (WPARAM)currServ, (LPARAM)szBuffer );
    
} 

BOOL CALLBACK W32SpyFilterDlgProc(HWND hWndDlg, UINT msg,
                                    WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_COMMAND:
            Handle_FilterDlg_WM_COMMAND( hWndDlg, wParam, lParam );
            break;

        case WM_INITDIALOG:
            Handle_FilterDlg_WM_INITDIALOG( hWndDlg, wParam, lParam );
            break;

        case WM_CLOSE:
            EndDialog(hWndDlg, 0); return FALSE;
    }
    return FALSE;
}

static void GetSavedFiltersFilename( PSTR pszBuffer )
{
    PSTR p;
    
    GetModuleFileName( 0, pszBuffer, MAX_PATH );
    
    p = strrchr( pszBuffer, '\\' );
    strcpy( p+1, "w32svspy.flt" );
}

BOOL SaveFilterValues(void)
{
    FILE * pFile;
    char szSaveFileName[MAX_PATH];
    unsigned i, j;
    
    GetSavedFiltersFilename( szSaveFileName );
    pFile = fopen( szSaveFileName, "wb" );
    if ( !pFile )
    {
        MessageBox(0, "Unable to open saved filters file!", 0, MB_OK);
        return FALSE;
    }
    
    for ( i=0; i < CWin32ServiceVxDs; i++ )
    {
        for ( j=0; j < W32ServiceTable[i].cServiceCalls; j++ )
        {
            if ( W32ServiceTable[i].pServiceCalls[j].fIgnore )
            {
                DWORD serviceID;
                
                serviceID = W32ServiceTable[i].vxdID + j;
                fwrite( &serviceID, sizeof(serviceID), 1, pFile );
            }
        }
    }
    
    fclose( pFile );

    return TRUE;
}

BOOL LoadSavedFilterValues(void)
{
    FILE * pFile;
    char szSaveFileName[MAX_PATH];
    DWORD serviceID;
    PWIN32_SERVICE_CALL pWin32ServiceCall;
        
    GetSavedFiltersFilename( szSaveFileName );
    pFile = fopen( szSaveFileName, "rb" );
    if ( !pFile )
    {
        // MessageBox(0, "Unable to open saved filters file!", 0, MB_OK);
        return FALSE;
    }
    
    while ( !feof(pFile) && fread(&serviceID, sizeof(serviceID), 1, pFile) )
    {
        pWin32ServiceCall = LookupWin32ServiceCall( serviceID );
        if ( pWin32ServiceCall )
            pWin32ServiceCall->fIgnore = TRUE;
    }
    
    fclose( pFile );
    
    return TRUE;
}
