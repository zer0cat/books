/******************************************************************************
Module:  RegNotify.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.
#include <WindowsX.h>
#include <Process.h>                   // For _beginthreadex
#include "Resource.h"

#define UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"      // See Appendix B.


///////////////////////////////////////////////////////////////////////////////


CUILayout g_UILayout;   // Repositions controls when dialog box size changes.


///////////////////////////////////////////////////////////////////////////////


// We are using this function in place of an event object to let the thread
// know that it is time to exit its primary loop.
void WINAPI DoNothingAPC(ULONG_PTR dwParam) {
}


///////////////////////////////////////////////////////////////////////////////


DWORD WINAPI RegSubkeyWatcher(PVOID pv) {

   HWND hwnd = (HWND) pv;

   // Create our event for notification
   HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   TCHAR szSubkey[200];

   // Get the Reg Key that we are going to create or open for watching
   GetDlgItemText(hwnd, IDC_REGKEY, szSubkey, chDIMOF(szSubkey));

   // Get our registry key to watch.
   HKEY hkey = NULL;
   RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubkey, 0, NULL, 
      REG_OPTION_NON_VOLATILE, KEY_NOTIFY | KEY_QUERY_VALUE, 
      NULL, &hkey, NULL);

   do {
      DWORD dwIndex = 0, cbValName;
      BYTE bData[1024];
      TCHAR szValName[100];

      cbValName = chDIMOF(szValName);
      DWORD dwType, cbData = chDIMOF(bData);
      TCHAR szRegVals[20 * 1024] = { 0 };

      // RegEnumValue to enumerate the values in the key
      while (ERROR_SUCCESS == RegEnumValue(hkey, 
         dwIndex++, szValName, (cbValName = chDIMOF(szValName), &cbValName), 
         NULL, &dwType, bData, (cbData = chDIMOF(bData), &cbData))) {

         PTSTR p = szRegVals + lstrlen(szRegVals);
         wsprintf(p, TEXT("\r\n%s\t"), szValName);
         p = szRegVals + lstrlen(szRegVals);

         // Handle the different types
         switch (dwType) {
         case REG_DWORD:
            wsprintf(p, TEXT("0x%08x"), * (PDWORD) bData); 
            break;

         case REG_EXPAND_SZ:
         case REG_LINK:
         case REG_MULTI_SZ:
         case REG_RESOURCE_LIST:
         case REG_SZ:
            wsprintf(p, TEXT("%s"), bData); 
            break;

         case REG_NONE:
         case REG_DWORD_BIG_ENDIAN:
         default:
            wsprintf(p, TEXT("Unknown type")); 
            break;

         case REG_BINARY:
            for (DWORD x = 0; x < cbData; x++) 
               wsprintf(p + lstrlen(p), TEXT("%02X "), bData[x]);
            break;
         }
      }

      // Set the display
      SetDlgItemText(hwnd, IDC_REGVALUES, &szRegVals[2]);   // skip "\r\n"

      // Set up the notification... notice we have to do this each time.
      // It is also important that the thread that makes the call to 
      // RegNotifyChangeKeyValue be the one that waits on the event.
      RegNotifyChangeKeyValue(hkey, FALSE, 
         REG_NOTIFY_CHANGE_NAME     | REG_NOTIFY_CHANGE_ATTRIBUTES | 
         REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_SECURITY,
         hEvent, TRUE); 

      // Wait forever for the event or the APC
   } while (WaitForSingleObjectEx(hEvent, INFINITE, TRUE) == WAIT_OBJECT_0);

   // We are done with the event and key
   CloseHandle(hEvent);
   RegCloseKey(hkey);
   return(0);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_REGNOTIFY);

   // Set up the resizeing of the controls
   g_UILayout.Initialize(hwnd);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_BOTTOMRIGHT, 
      IDC_REGVALUES, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPRIGHT, 
      IDC_REGKEY, FALSE);

   SetDlgItemText(hwnd, IDC_REGKEY, TEXT("SOFTWARE\\RegNotify"));
   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   static HANDLE s_hThread = NULL;

   switch (id) {
   case IDCANCEL:
      if (s_hThread != NULL) {
         // Que the useless APC function to signal the exit of our other thread
         QueueUserAPC(DoNothingAPC, s_hThread, 0); 
         // Wait for the thread to exit
         WaitForSingleObject(s_hThread, INFINITE);
         CloseHandle(s_hThread);
         s_hThread = NULL;
      }
      EndDialog(hwnd, id);
      break;

   case IDOK:
      // Disable our button
      EnableWindow(hwndCtl, FALSE);
      // Start the notification thread
      s_hThread = chBEGINTHREADEX(NULL, 0, RegSubkeyWatcher, hwnd, 0, NULL);
      break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnSize(HWND hwnd, UINT state, int cx, int cy) {

   // Reposition the child controls
   g_UILayout.AdjustControls(cx, cy);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnGetMinMaxInfo(HWND hwnd, PMINMAXINFO pMinMaxInfo) {

   // Return minimum size of dialog box
   g_UILayout.HandleMinMax(pMinMaxInfo);
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
   chHANDLE_DLGMSG(hwnd, WM_INITDIALOG,    Dlg_OnInitDialog);
   chHANDLE_DLGMSG(hwnd, WM_COMMAND,       Dlg_OnCommand);
   chHANDLE_DLGMSG(hwnd, WM_SIZE,          Dlg_OnSize);
   chHANDLE_DLGMSG(hwnd, WM_GETMINMAXINFO, Dlg_OnGetMinMaxInfo);
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, LPTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_REGNOTIFY), NULL, Dlg_Proc);
   return(0);
}


//////////////////////////////// End of File //////////////////////////////////
