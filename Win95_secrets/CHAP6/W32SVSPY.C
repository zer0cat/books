//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SVSPY.C
//==================================
#include <windows.h>
#include <stdio.h>
#include <commdlg.h>
#pragma hdrstop
#include "w32spdll.h"
#include "w32svspy.h"
#include "w32info.h"
#include "w32srvdb.h"
#include "w32svflt.h"

// Prototype the functions for this
void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam);
void Handle_WM_INITDIALOG(HWND hWndDlg);
BOOL CALLBACK W32SpyDlgProc(HWND, UINT, WPARAM, LPARAM);
void StartUp( HWND hWndDlg );
void CleanUp( HWND hWndDlg );
void ShowResults(void);
void SetButtonStates( HWND hWndDlg );
void SaveToFile( HWND hWndDlg );
BOOL FormatReportLine(  unsigned eventIndex,
                        PWIN32SERVICECALLINFO pCallArray,
                        PSTR pszBuffer,
                        unsigned cbBuffer );
BOOL IsFilteredCall( unsigned eventIndex, PWIN32SERVICECALLINFO pCallArray );


//====================== Global Vars ======================================
HINSTANCE HInstance;

HWND HWndMainList;
HWND HWndStatus;

BOOL FIntercepted = FALSE;

int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow )
{
    HInstance = hInstance;

    LoadSavedFilterValues();
    
    DialogBox(hInstance, "W32SPY_DLG", 0, (DLGPROC)W32SpyDlgProc);
    
    SaveFilterValues();
    
    return 0;
}

void StartUp( HWND hWndDlg )
{
    if ( !FIntercepted )
    {
        SendMessage( HWndMainList, LB_RESETCONTENT, 0, 0 );
        InitWin32ServiceSpyDLL();
        FIntercepted = TRUE;
        SetWindowText( HWndStatus, "Spying..." );
        SetButtonStates( hWndDlg );
    }
}

void CleanUp( HWND hWndDlg )
{
    if ( FIntercepted )
    {
        StopWin32ServiceSpy();
        FIntercepted = FALSE;
        SetWindowText( HWndStatus, "disabled..." );
        SetButtonStates( hWndDlg );
    }
}

void ShowResults(void)
{
    DWORD cCalls;
    PWIN32SERVICECALLINFO pCallArray;
    DWORD i;
    char szBuffer[512];
        
    if ( !GetWin32ServiceLogInfo(&cCalls, &pCallArray) )
        return;
    
    wsprintf( szBuffer, "%u calls logged", cCalls );
    SetWindowText( HWndStatus, szBuffer );
    
    SendMessage( HWndMainList, LB_RESETCONTENT, 0, 0 );
    SendMessage( HWndMainList, WM_SETREDRAW, FALSE, 0 );

    for ( i = 0; i < cCalls; i++ )
    {
        char szBuffer[512];
        
        if ( !IsFilteredCall( i, pCallArray ) )
        {
            FormatReportLine( i, pCallArray, szBuffer, sizeof(szBuffer) );
            SendMessage( HWndMainList, LB_ADDSTRING, 0, (LPARAM)szBuffer );
        }
    }   

    SendMessage( HWndMainList, WM_SETREDRAW, TRUE, 0 );
}

//
// Dialog proc for the main dialog
//
BOOL CALLBACK W32SpyDlgProc(HWND hWndDlg, UINT msg,
                              WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_COMMAND:
            Handle_WM_COMMAND(hWndDlg, wParam, lParam); return TRUE;
        case WM_INITDIALOG:
            Handle_WM_INITDIALOG(hWndDlg); return TRUE;
        case WM_CLOSE:
            CleanUp( hWndDlg );
            EndDialog(hWndDlg, 0); return FALSE;
    }
    return FALSE;
}

//
// Handle the dialog's WM_COMMAND messages
//
void Handle_WM_COMMAND(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{
    switch ( LOWORD(wParam) )
    {
        case IDC_BUTTON_START:
            StartUp( hWndDlg );
            break;
            
        case IDC_BUTTON_STOP:
            CleanUp( hWndDlg );
            ShowResults();
            break;
            
        case IDC_BUTTON_EXIT:
            CleanUp( hWndDlg );
            EndDialog(hWndDlg, 0);
            break;

        case IDC_BUTTON_FILTER:
            DialogBox(HInstance, "Win32ServicesFilterDialog", hWndDlg,
                        (DLGPROC)W32SpyFilterDlgProc);
            ShowResults();
            break;

        case IDC_BUTTON_SAVE:
            SaveToFile( hWndDlg );
            break;
    }

    return;
}

void Handle_WM_INITDIALOG(HWND hWndDlg)
{
    HWndMainList = GetDlgItem(hWndDlg, IDC_LISTBOX_CALLS);
    HWndStatus = GetDlgItem(hWndDlg, IDC_TEXT_STATUS);

    SetButtonStates( hWndDlg );
    
    SendMessage( HWndMainList, WM_SETFONT,
                 (WPARAM)GetStockObject(OEM_FIXED_FONT), 0 );
}

void SetButtonStates( HWND hWndDlg )
{
    EnableWindow( GetDlgItem(hWndDlg, IDC_BUTTON_START),
                    FIntercepted ? FALSE : TRUE );
    EnableWindow( GetDlgItem(hWndDlg, IDC_BUTTON_STOP),
                    FIntercepted ? TRUE : FALSE );  
    EnableWindow( GetDlgItem(hWndDlg, IDC_BUTTON_SAVE),
                    FIntercepted ? FALSE : TRUE );  
}

void SaveToFile( HWND hWndDlg )
{
    DWORD cCalls;
    PWIN32SERVICECALLINFO pCallArray;
    OPENFILENAME ofn;
    char szFile[MAX_PATH];
    FILE *pOutFile;
    unsigned i;

    if ( !GetWin32ServiceLogInfo(&cCalls, &pCallArray) )
        return;

    if ( cCalls == 0 )
        return;

    // use COMMDLG.DLL to browse for the name to save as
    memset(&ofn, 0, sizeof(OPENFILENAME));
    szFile[0] = '\0';
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWndDlg;
    ofn.lpstrFile= szFile;
    ofn.nMaxFile = sizeof(szFile);
    if ( !GetSaveFileName(&ofn) )
        return;

    pOutFile = fopen((LPSTR)ofn.lpstrFile, "wt");
    if ( !pOutFile )
    {
        MessageBox(hWndDlg,"Couldn't open specified report file", 0, MB_OK);
        return;
    }

    fprintf( pOutFile, "%u calls\n", cCalls );
    for ( i = 0; i < cCalls; i++ )
    {
        char szBuffer[1024];
        
        if ( !IsFilteredCall( i, pCallArray ) )
        {
            FormatReportLine( i, pCallArray, szBuffer, sizeof(szBuffer) );
            fputs( szBuffer, pOutFile );
            fputs( "\n", pOutFile );
        }
    }   

    fclose(pOutFile);   
}

BOOL
FormatReportLine(
            unsigned eventIndex,
            PWIN32SERVICECALLINFO pCallArray,
            PSTR pszBuffer,
            unsigned cbBuffer )
{
    char szBuffer2[512];
    
    GetWin32ServiceName( pCallArray[eventIndex].serviceId,
                        pCallArray[eventIndex].param1,
                        szBuffer2,
                        sizeof(szBuffer2) );

    if ( pCallArray[eventIndex].szName[0] == 0 )
    {
        char szTemp[10];
        wsprintf(szTemp, "%08X", pCallArray[eventIndex].processId);
        memcpy( pCallArray[eventIndex].szName, szTemp, 8 );
    }
    
    #if 0
    wsprintf( pszBuffer, "%08X (p1=%08X) %s",
                pCallArray[eventIndex].serviceId,
                pCallArray[eventIndex].param1,
                szBuffer2 );
    #else
    wsprintf( pszBuffer, "%-8.8s %s(%08X)",
                pCallArray[eventIndex].szName,
                szBuffer2,
                pCallArray[eventIndex].param1 );
    #endif
        
    return TRUE;
}

BOOL IsFilteredCall( unsigned eventIndex, PWIN32SERVICECALLINFO pCallArray )
{
    PWIN32_SERVICE_CALL pWin32ServiceCall;

    pWin32ServiceCall =
        LookupWin32ServiceCall( pCallArray[eventIndex].serviceId );
    
    if ( !pWin32ServiceCall )
        return FALSE;       // If we don't know about it, don't filter it
    
    return pWin32ServiceCall->fIgnore;
}
