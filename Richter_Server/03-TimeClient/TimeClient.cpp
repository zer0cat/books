/******************************************************************************
Module:  TimeClient.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                    // See Appendix A.
#include <WindowsX.h>
#include "..\ClassLib\EnsureCleanup.h"    // See Appendix B.
#include "Resource.h"


//////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_TIMECLIENT);

   // Assume that the server is on the same machine as the client
   SetDlgItemText(hwnd, IDC_SERVER, TEXT("."));
   return(TRUE);
}


//////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   switch (id) {
   case IDCANCEL:
     EndDialog(hwnd, id); 
     break;

   case IDOK:
      // Construct the pathname of the pipe
      TCHAR sz[500];
      sz[0] = sz[1] = TEXT('\\');
      GetDlgItemText(hwnd, IDC_SERVER, &sz[2], chDIMOF(sz) - 2);
      lstrcat(sz, TEXT("\\pipe\\TimeService"));

      // Attempt to connect to the pipe 
      // Get a handle to use to talk to the pipe
      CEnsureCloseFile hpipe = 
         CreateFile(sz, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

      if (hpipe.IsValid()) {
         // Valid handle, read time from pipe
         SYSTEMTIME st;
         DWORD cbRead = 0;
         ReadFile(hpipe, &st, sizeof(st), &cbRead, NULL);

         // Convert UTC time to client machine's local time and display it
         SystemTimeToTzSpecificLocalTime(NULL, &st, &st);

         GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, 
            &st, NULL, sz, chDIMOF(sz));
         SetDlgItemText(hwnd, IDC_DATE, sz);

         GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, 
            &st, NULL, sz, chDIMOF(sz));
         SetDlgItemText(hwnd, IDC_TIME, sz);

      } else {
         // Invalid handle, report an error
         SetDlgItemText(hwnd, IDC_DATE, TEXT("Error"));
         SetDlgItemText(hwnd, IDC_TIME, TEXT("Error"));

         // Get the error code's textual description
         HLOCAL hlocal = NULL;   // Buffer that gets the error message string
         FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(), 
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (PSTR) &hlocal, 
            0, NULL);
         if (hlocal != NULL) {
            chMB((PCSTR) LocalLock(hlocal));
            LocalFree(hlocal);
         }
      }
      break;
   }
}


//////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
   chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
   chHANDLE_DLGMSG(hwnd, WM_COMMAND,    Dlg_OnCommand);
   }
   return(FALSE);
}


//////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_TIMECLIENT), NULL, Dlg_Proc);
   return(0);
}


//////////////////////////////// End Of File /////////////////////////////////
