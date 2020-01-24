/******************************************************************************
Module:  EventMonitor.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.
#include <WindowsX.h>
#include <ShlWAPI.h>
#pragma comment(lib, "ShlWAPI")

#include <Process.h>
#include <Malloc.h>

#include "Resource.h"

#define AUTOBUF_IMPL
#include "..\ClassLib\AutoBuf.h"       // See Appendix B.

#define PRINTBUF_IMPL
#include "..\ClassLib\PrintBuf.h"      // See Appendix B.

#include "..\ClassLib\EnsureCleanup.h" // See Appendix B.

#define UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"      // See Appendix B.


///////////////////////////////////////////////////////////////////////////////


HANDLE g_hEventLog = NULL;             // Handle to the event log
HANDLE g_evtNewEventLogRecord = NULL;  // Signalled for new event log entries
HWND   g_hwnd = NULL;                  // Handle of main dialog box window
CUILayout g_UILayout;   // Repositions controls when dialog box size changes.


///////////////////////////////////////////////////////////////////////////////


// User-defined message, handler, and forwarder
#define WM_USERNOTIFYUPDATE    (WM_APP + 0)
#define HANDLE_WM_USERNOTIFYUPDATE(hwnd, wParam, lParam, fn) ((fn)(), 0L)
#define FORWARD_WM_USERNOTIFYUPDATE(hwnd, fn) \
    (void)(fn)((hwnd), WM_USERNOTIFYUPDATE, 0L, 0L)


///////////////////////////////////////////////////////////////////////////////


// Useful macros for reading EVENTLOGRECORD structures
#define SourceNameFromEventRecord(pelr) \
   (PTSTR) ((&pelr->DataOffset) + 1)

#define StringsFromEventRecord(pelr) \
   (PTSTR) (((PBYTE) pelr) + pelr->StringOffset)

#define ComputerNameFromEventRecord(pelr) \
   (SourceNameFromEventRecord(pelr) + \
   lstrlen(SourceNameFromEventRecord(pelr)) + 1)

#define SIDFromEventRecord(pelr) \
   (PSID) (((PBYTE) pelr) + pelr->UserSidOffset)


///////////////////////////////////////////////////////////////////////////////


// Useful functions for event log reading
#define GetEventCategory(pszLog, pelr) \
   FormatEventMessage(pszLog, pelr, pelr->EventCategory, \
   TEXT("CategoryMessageFile"))


///////////////////////////////////////////////////////////////////////////////


PTSTR FormatEventMessage(PCTSTR pszLog, PEVENTLOGRECORD pelr, 
   DWORD dwMessageID, PTSTR pszMsgFileType) {

   PTSTR pszMessage = NULL;
   try {{
      // Get the record's source name and replacable strings
      PTSTR pszSourceName = SourceNameFromEventRecord(pelr);

      // Open this log sources registry information.
      TCHAR szSubkey[MAX_PATH];
      wsprintf(szSubkey, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s"), pszLog, pszSourceName);
      HKEY hkeyTemp;
      if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubkey, 0, KEY_QUERY_VALUE, &hkeyTemp))
         goto leave;
      CEnsureRegCloseKey hkey = hkeyTemp;

      // Get the pathname(s) of the desired message file
      TCHAR szMsgFileMods[MAX_PATH];
      DWORD dwSize = sizeof(szMsgFileMods);   // Size in bytes (not characters)
      if (ERROR_SUCCESS != RegQueryValueEx(hkey, pszMsgFileType, NULL, NULL, (PBYTE) szMsgFileMods, &dwSize))
         goto leave;

      // Expand environment paths.
      chVERIFY(ExpandEnvironmentStrings(szMsgFileMods, szMsgFileMods, chDIMOF(szMsgFileMods)) != 0);

      // If replaceable string params exist, create an array of string pointers
      PTSTR pszStrings = StringsFromEventRecord(pelr);
      PCTSTR* ppszStrings = NULL;
      if (pelr->NumStrings > 0) {
         ppszStrings = (PCTSTR*) _alloca(pelr->NumStrings * sizeof(PCTSTR));
         for (DWORD i = 0; i < pelr->NumStrings; i++) {
            ppszStrings[i] = pszStrings;
            pszStrings += lstrlen(pszStrings) + 1;
         }
      }

      // Scan all the possible DLLs (in order) looking for the desired message ID
      PCTSTR pszMsgFileMod = szMsgFileMods;
      for (BOOL fStop = FALSE; !fStop; ) {
         PTSTR pszEndOfMod = StrChr(pszMsgFileMod, TEXT(';'));
         if (pszEndOfMod != NULL) *pszEndOfMod = 0;   // Truncate this module

         CEnsureFreeLibrary hmod = LoadLibraryEx(pszMsgFileMod, NULL, LOAD_LIBRARY_AS_DATAFILE);
         if (hmod.IsInvalid()) goto leave;

         // Conver the message ID to its string equivalent

         // Note: FormatMessage is in a try block because it may raise an access violation
         // if the number of replacable string parameters doesn't match what the string expect.
         try {
            DWORD dwError = 0;
            dwError = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
               FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_HMODULE,
               hmod, dwMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               (PTSTR) &pszMessage, 0, (va_list*) ppszStrings);
            fStop = (dwError > 0);  // The message was found in this module
         } 
         catch (...) {
            pszMessage = NULL;
         }
         // Advance to the next module
         if (pszEndOfMod == NULL) fStop = TRUE;
         else pszMsgFileMod = pszEndOfMod + 1;  // Point to start of next module
      }
   }
leave:;
   }
   catch (...) {
   }

   // If the last 2 characters are "\r\n", remove them.
   if ((pszMessage != NULL) && (lstrlen(pszMessage) >= 2)) {
      if (StrCmpI(&pszMessage[lstrlen(pszMessage) - 2], TEXT("\r\n")) == 0) {   // 13 followed by 10
         pszMessage[lstrlen(pszMessage) - 2] = 0; // Force early string termination
      }
   }

   return(pszMessage);
}


///////////////////////////////////////////////////////////////////////////////


// Read event log functions
PCTSTR GetEventMessage(PCTSTR pszLog, PEVENTLOGRECORD pelr) {

   PTSTR pszFinalMessage = NULL; // Points to return string

   try {{
      // Here we get the message all nicely formatted
      PTSTR pszMessage = FormatEventMessage(pszLog, pelr, pelr->EventID, TEXT("EventMessageFile"));
      if (pszMessage == NULL) goto leave;

      // If there are no replaceable strings, there is nothing else to do.
      if ((pelr->NumStrings == 0)) {
         pszFinalMessage = pszMessage;
         goto leave;
      }

      // Search for replaceable parameters (for example "%%255" in original string)
      
      for (int nParams = 0, nIndex = 0; pszMessage[nIndex] != 0; nIndex++){
         if ((pszMessage[nIndex] == TEXT('%')) && 
            chINRANGE(TEXT('0'), pszMessage[nIndex+1], TEXT('9'))) nParams++;
      }

      // If there are no replaceable parameters, there is nothing else to do.
      if (nParams == 0) {
         pszFinalMessage = pszMessage;
         goto leave;
      }

      // Insert the replaceable parameters
      CPrintBuf szMsgWithReplaceableParams;
      PTSTR pszMsgPart = pszMessage;
      for (PTSTR pszParam = pszMsgPart; 
         NULL != (pszParam = StrChr(pszMsgPart, TEXT('%'))); ) {
         // Append the first part of the message to the output buffer
         *pszParam = 0;  // Append part up to percent sign         
         szMsgWithReplaceableParams.Print(TEXT("%s"), pszMsgPart);
         while(!chINRANGE(TEXT('0'), *pszParam, TEXT('9')))
            pszParam++;     // Skip past the % sign
         
         if (chINRANGE(TEXT('0'), pszParam[0], TEXT('9'))) {
            // Character after the percent sign is a digit.
            // Convert number to replaceable string
            PCTSTR pszReplaceableParameterString = 
               FormatEventMessage(pszLog, pelr, _ttoi(pszParam), TEXT("ParameterMessageFile"));
            if (pszReplaceableParameterString != NULL) {
               szMsgWithReplaceableParams.Print(TEXT("%s"), pszReplaceableParameterString);
               LocalFree((HLOCAL) pszReplaceableParameterString);
               // Skip over the remaining digits of the number
               for (; chINRANGE(TEXT('0'), *pszParam, TEXT('9')); pszParam++) ;
            } else {
               // Number could not be converted, keep the number in the string.
               szMsgWithReplaceableParams.Print(TEXT("%%"));
            }
            // Advance to next part (after replaceable string number)
            pszMsgPart = pszParam;
         } else {
            // This is not a replaceable parameter, put the percent sign back
            szMsgWithReplaceableParams.Print(TEXT("%%"));
         }
      }
      // Append final message part to final buffer
      szMsgWithReplaceableParams.Print(TEXT("%s"), pszMsgPart);

      // Creating a buffer for the total size of the new string
      pszFinalMessage = (PTSTR) LocalAlloc(LPTR, chSIZEOFSTRING(szMsgWithReplaceableParams));
      if (pszFinalMessage != NULL)
         lstrcpy(pszFinalMessage, szMsgWithReplaceableParams);
   }
leave:;
   }
   catch (...) {
      pszFinalMessage = NULL;
   }
   return(pszFinalMessage);
}


///////////////////////////////////////////////////////////////////////////////


void EventTimeToLocalSystemTime(DWORD dwEventTime, SYSTEMTIME* pstTime) {

   SYSTEMTIME st1970;
   // Create a FILETIME for 00:00:00 January 1, 1970
   st1970.wYear         = 1970;
   st1970.wMonth        = 1;
   st1970.wDay          = 1;
   st1970.wHour         = 0;
   st1970.wMinute       = 0;
   st1970.wSecond       = 0;
   st1970.wMilliseconds = 0;

   union {
      FILETIME ft;
      LONGLONG ll;
   } u1970;
   SystemTimeToFileTime(&st1970, &u1970.ft);

   union {
      FILETIME ft;
      LONGLONG ll;
   } uUCT;
   // Scale from seconds to 100-nanoseconds
   uUCT.ll = 0;
   uUCT.ft.dwLowDateTime = dwEventTime;
   uUCT.ll *= 10000000;
   uUCT.ll += u1970.ll;

   FILETIME   ftLocal;
   FileTimeToLocalFileTime(&uUCT.ft, &ftLocal);
   FileTimeToSystemTime(&ftLocal, pstTime);
}


///////////////////////////////////////////////////////////////////////////////


void AddEventsToMonitor(HANDLE hEventLog, HWND hwnd) {

   // Add the event log entries to the listbox.
   for (BOOL fStop = FALSE; !fStop; ) {
      DWORD dwBytesRead = 0, dwBytesNeeded = 0;
      BYTE bDummy;
      ReadEventLog(hEventLog, 
         EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ, 
         0, &bDummy, 1, &dwBytesRead, &dwBytesNeeded);
      if (dwBytesNeeded == 0) {
         // We reached th end of the list
         fStop = TRUE;
         break;
      }

      PEVENTLOGRECORD pelr = (PEVENTLOGRECORD) malloc(dwBytesNeeded);
      chASSERT(pelr != NULL);
      ReadEventLog(hEventLog, 
         EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ, 
         0, pelr, dwBytesNeeded, &dwBytesRead, &dwBytesNeeded);

      // Allocate memory, copy elr to it, Add entry to LB, associate mem block w/LB entry
      CPrintBuf szLine(1024);

      SYSTEMTIME stTime;
      TCHAR szBuffer[256];
      EventTimeToLocalSystemTime(pelr->TimeGenerated, &stTime);
      GetDateFormat(LOCALE_USER_DEFAULT, 0, &stTime, TEXT("MM/dd/yy"), szBuffer, chDIMOF(szBuffer));
      szLine.Print(TEXT("%s  "), szBuffer);

      GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stTime, TEXT("HH:mm:ss"), szBuffer, chDIMOF(szBuffer));
      szLine.Print(TEXT("%s\t"), szBuffer);

      szLine.Print(TEXT("%s\t%08X\t%s"), SourceNameFromEventRecord(pelr), pelr->EventID,
         ComputerNameFromEventRecord(pelr));

      HWND hwndEvents = GetDlgItem(hwnd, IDC_EVENTS);
      DWORD dwIndex = ListBox_InsertString(hwndEvents, -1, szLine);
      ListBox_SetItemData(hwndEvents, dwIndex, pelr);
      ListBox_SetCurSel(hwndEvents, dwIndex);
   }
   FORWARD_WM_COMMAND(hwnd, IDC_EVENTS, GetDlgItem(hwnd, IDC_EVENTS), LBN_SELCHANGE, SendMessage);
}


///////////////////////////////////////////////////////////////////////////////


void InitEventMonitor(HWND hwnd) {

   TCHAR szComputer[256];
   GetDlgItemText(hwnd, IDC_COMPUTER, szComputer, chDIMOF(szComputer));

   TCHAR szLog[256];
   GetDlgItemText(hwnd, IDC_LOG, szLog, chDIMOF(szLog));

   chASSERT(g_hEventLog == NULL);
   g_hEventLog = OpenEventLog(szComputer, szLog);
   if (g_hEventLog == NULL) {
      CPrintBuf buf;
      buf.PrintError();
      MessageBox(hwnd, buf, NULL, MB_OK);;
   } else {
      NotifyChangeEventLog(g_hEventLog, g_evtNewEventLogRecord);
      AddEventsToMonitor(g_hEventLog, hwnd);
   }
}


///////////////////////////////////////////////////////////////////////////////


void UninitEventMonitor(HWND hwnd) {
   // Cleanup everything
   if (g_hEventLog != NULL) {
      chVERIFY(CloseEventLog(g_hEventLog));
      g_hEventLog = NULL;

      // Erase the entries in the Event list box
      HWND hwndEvents = GetDlgItem(hwnd, IDC_EVENTS);
      while (ListBox_GetCount(hwndEvents) > 0) {
         PEVENTLOGRECORD pelr = (PEVENTLOGRECORD) 
            ListBox_GetItemData(hwndEvents, 0);
         free(pelr);
         ListBox_DeleteString(hwndEvents, 0);
      }
   }
   SetDlgItemText(hwnd, IDC_DETAILS, TEXT(""));
}


///////////////////////////////////////////////////////////////////////////////


void ConstructEventDetailsString(HWND hwnd, PCTSTR pszLog, PEVENTLOGRECORD pelr) {

   CPrintBuf szDetails;

   PCTSTR pszType = TEXT("(unknown");
   switch (pelr->EventType) {
   case EVENTLOG_ERROR_TYPE:
      pszType = TEXT("Error");
      break;

   case EVENTLOG_WARNING_TYPE:
      pszType = TEXT("Warning");
      break;

   case EVENTLOG_INFORMATION_TYPE:
      pszType = TEXT("Information");
      break;

   case EVENTLOG_AUDIT_SUCCESS:
      pszType = TEXT("Audit Success");
      break;

   case EVENTLOG_AUDIT_FAILURE:
      pszType = TEXT("Audit Failure");
      break;
   }

   // Call our helper function to get the event category string.
   PCTSTR pszCategory = GetEventCategory(pszLog, pelr);
   PCTSTR pszMessage  = GetEventMessage(pszLog, pelr);
   szDetails.Print(
      TEXT("Type: %s\r\nCategory: %s\r\nMessage: %s"), pszType, pszCategory, pszMessage);
   SetDlgItemText(hwnd, IDC_DETAILS, szDetails);

   if (pszCategory != NULL) LocalFree((HLOCAL) pszCategory);
   if (pszMessage  != NULL) LocalFree((HLOCAL) pszMessage);
}


///////////////////////////////////////////////////////////////////////////////


// We are using this function in place of an event object to let the thread
// know that it is time to exit its primary loop.
void WINAPI DoNothingAPC(ULONG_PTR dwParam) {
}


///////////////////////////////////////////////////////////////////////////////


DWORD WINAPI EventNotifyThread(PVOID pvParam) {

   // Note that this is not the thread that called NotifyChangeEventLog.  
   // The main thread does this in response to the WM_INITDIALOG message.
   // If the main thread were to terminate before this thread, then 
   // notifications would cease to function.  However, since it will always 
   // terminate after this thread, then we can count on the notification event 
   // working properly.

   // Wait for an event notification or for APC altering this thread to termiante.
   while (WaitForSingleObjectEx(g_evtNewEventLogRecord, INFINITE, TRUE) != WAIT_IO_COMPLETION) {
      FORWARD_WM_USERNOTIFYUPDATE(g_hwnd, PostMessage);
   }

   // We got an APC, this thread should terminate
   return(0);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_EVENTMONITOR);
   g_hwnd = hwnd; // Save handle globally so other functions can talk to us

   // Set up the resizeing of the controls
   g_UILayout.Initialize(hwnd);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_MIDDLERIGHT, 
      IDC_EVENTS, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_MIDDLELEFT, CUILayout::AP_BOTTOMRIGHT, 
      IDC_DETAILS, FALSE);

   // Setup log popup
   HWND hwndCtrl = GetDlgItem(hwnd, IDC_LOG);
   ComboBox_AddString(hwndCtrl, TEXT("System"));
   ComboBox_AddString(hwndCtrl, TEXT("Security"));
   ComboBox_AddString(hwndCtrl, TEXT("Application"));

   // Default to the "Application" log
   ComboBox_SelectString(hwndCtrl, -1, TEXT("Application"));

   // Setup computer edit
   TCHAR szComputer[MAX_COMPUTERNAME_LENGTH + 1];
   DWORD dwSize = chDIMOF(szComputer);
   GetComputerName(szComputer, &dwSize);
   SetDlgItemText(hwnd, IDC_COMPUTER, szComputer);

   // Setup log
   int nEventTabs[] = { 85, 135, 175 };
   ListBox_SetTabStops(GetDlgItem(hwnd, IDC_EVENTS), 3, nEventTabs);
   InitEventMonitor(hwnd);

   SetFocus(GetDlgItem(hwnd, IDC_DETAILS));
   return(FALSE); // We explicitly set focus
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   switch (id) {
   case IDCANCEL:
      UninitEventMonitor(hwnd);
      EndDialog(hwnd, id);
      break;

   case IDC_MONITOR:
      UninitEventMonitor(hwnd);
      InitEventMonitor(hwnd);
      break;

   case IDC_EVENTS:
      if (codeNotify == LBN_SELCHANGE) {
         // Passes the the item data (or event log record pointer) to item detail.
         int nIndex = ListBox_GetCurSel(hwndCtl);
         if (nIndex != LB_ERR) {
            TCHAR szLog[100];
            GetDlgItemText(hwnd, IDC_LOG, szLog, chDIMOF(szLog));
            ConstructEventDetailsString(hwnd, szLog, 
               (PEVENTLOGRECORD) ListBox_GetItemData(hwndCtl, nIndex));
         }
      }
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
   case WM_USERNOTIFYUPDATE:
      // Posted from EventNotifyThread function
      AddEventsToMonitor(g_hEventLog, hwnd);
      break;
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   // Create our events for event logging notification as well as our 
   // event for ending the notification thread
   g_evtNewEventLogRecord = CreateEvent(NULL, TRUE, FALSE, NULL);
   HANDLE hThread = chBEGINTHREADEX(NULL, 0, EventNotifyThread, NULL, 0, NULL);

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_EVENTMONITOR), NULL, Dlg_Proc);

   // Clean up
   QueueUserAPC(DoNothingAPC, hThread, NULL);
   WaitForSingleObject(hThread, INFINITE);
   CloseHandle(g_evtNewEventLogRecord);
   CloseHandle(hThread);
   return(0);
}


///////////////////////////////// End Of File /////////////////////////////////