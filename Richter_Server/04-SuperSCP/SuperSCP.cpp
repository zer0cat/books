/******************************************************************************
Module:  SuperSCP.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.
#include <Windowsx.h>
#include "Resource.h"

#define AUTOBUF_IMPL
#include "..\ClassLib\AutoBuf.h"       // See Appendix B.

#define SCMCTRL_IMPL
#include "SCMCtrl.h"

#define SERVICECTRL_IMPL
#include "ServiceCtrl.h"


///////////////////////////////////////////////////////////////////////////////


CSCMCtrl g_scm;


///////////////////////////////////////////////////////////////////////////////


void FixDependencyString(PTSTR pszDependencies, TCHAR chFrom, TCHAR chTo) {

   PTSTR p = pszDependencies;
   PTSTR q = NULL;
   while (p[0] != 0) {
      q = _tcschr(p, chFrom);
      if (q == NULL) break;
      *q = chTo;
      p = q + 1;
   }
   if (q != NULL) *q = chFrom;      // Undo the last change
}


///////////////////////////////////////////////////////////////////////////////


void ShowSCMLockStatus(PCTSTR pszMsgHeading) {

   #ifndef UNICODE
   PCSTR pszFmt = "%s";
   #else
   PCSTR pszFmt = "%S";
   #endif

   DWORD dwLockDuration = 0;
   PCTSTR pszLockOwner = g_scm.QueryLockOwner(&dwLockDuration);
   if (pszLockOwner != NULL) {
      #ifndef UNICODE
      pszFmt = "%s\nThe SCM was locked %d seconds ago by \"%s\".";
      #else
      pszFmt = "%S\nThe SCM was locked %d seconds ago by \"%S\".";
      #endif
   }
   char sz[300];
   wsprintfA(sz, pszFmt, pszMsgHeading, dwLockDuration, pszLockOwner);
   chMB(sz);
}

            
///////////////////////////////////////////////////////////////////////////////

            
void Dlg_SetControlButtons(HWND hwnd) {

   BOOL fOk = g_scm.OpenOK();
   SERVICE_STATUS_PROCESS ssp = { 0 };

   if (fOk) {
      TCHAR szInternalName[300];
      GetDlgItemText(hwnd, IDC_INTERNALNAME, 
         szInternalName, chDIMOF(szInternalName));

      CServiceCtrl sc(g_scm, TRUE, szInternalName, SERVICE_QUERY_STATUS);
      fOk = (sc.OpenOK() && sc.QueryStatus(&ssp));
   }

   EnableWindow(GetDlgItem(hwnd, IDC_STARTUPPARAMS), fOk && 
      (ssp.dwCurrentState == SERVICE_STOPPED));

   EnableWindow(GetDlgItem(hwnd, IDC_START), fOk && 
      (ssp.dwCurrentState == SERVICE_STOPPED));

   EnableWindow(GetDlgItem(hwnd, IDC_STOP), fOk &&  
      (ssp.dwCurrentState != SERVICE_STOPPED) && 
      ((ssp.dwControlsAccepted & SERVICE_ACCEPT_STOP) != 0));

   EnableWindow(GetDlgItem(hwnd, IDC_PAUSE), fOk &&  
      (ssp.dwCurrentState == SERVICE_RUNNING) && 
      ((ssp.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) != 0));
   
   EnableWindow(GetDlgItem(hwnd, IDC_CONTINUE), fOk &&  
      (ssp.dwCurrentState == SERVICE_PAUSED) && 
      ((ssp.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) != 0));
   
   EnableWindow(GetDlgItem(hwnd, IDC_INTERROGATE), fOk);

   EnableWindow(GetDlgItem(hwnd, IDC_PARAMCHANGE), fOk &&
      (ssp.dwCurrentState != SERVICE_STOPPED));

   EnableWindow(GetDlgItem(hwnd, IDC_USERCODE), fOk &&  
      (ssp.dwCurrentState != SERVICE_STOPPED));
   
   EnableWindow(GetDlgItem(hwnd, IDC_CODEVALUE), fOk &&  
      (ssp.dwCurrentState != SERVICE_STOPPED));
}


///////////////////////////////////////////////////////////////////////////////


const int g_nCtlIdFailAction[] = { 
   IDC_FIRSTFAILACTION, 
   IDC_SECONDFAILACTION, 
   IDC_SUBSEQUENTFAILACTION 
};


const int g_nCtlIdFailDelay[]  = { 
   IDC_FIRSTFAILDELAY, 
   IDC_SECONDFAILDELAY, 
   IDC_SUBSEQUENTFAILDELAY 
};


///////////////////////////////////////////////////////////////////////////////


void Dlg_ShowServiceData(HWND hwnd) {

   TCHAR szInternalName[300];
   GetDlgItemText(hwnd, IDC_INTERNALNAME, 
      szInternalName, chDIMOF(szInternalName));

   CServiceCtrl sc(g_scm, TRUE, szInternalName, SERVICE_QUERY_CONFIG);
   BOOL fOk = sc.OpenOK();
   if (fOk) {

      // Display service standard configuration.parameters
      const QUERY_SERVICE_CONFIG* pConfig = sc.QueryConfig();
      chASSERT(pConfig != NULL);
      SetDlgItemText(hwnd, IDC_DISPLAYNAME, pConfig->lpDisplayName);
      SetDlgItemText(hwnd, IDC_DESCRIPTION, sc.QueryDescription());
      SetDlgItemText(hwnd, IDC_PATHNAME, pConfig->lpBinaryPathName);

      DWORD dw = 0;
      switch (pConfig->dwServiceType & ~SERVICE_INTERACTIVE_PROCESS) {
      case SERVICE_WIN32_OWN_PROCESS:   dw = 0; break;
      case SERVICE_WIN32_SHARE_PROCESS: dw = 1; break;
      }
      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SERVICETYPE), dw);
      Button_SetCheck(GetDlgItem(hwnd, IDC_INTERACTWITHDESKTOP),
         ((pConfig->dwServiceType & SERVICE_INTERACTIVE_PROCESS) != 0) 
         ? BST_CHECKED : BST_UNCHECKED);

      switch (pConfig->dwStartType) {
      case SERVICE_AUTO_START:   dw = 0; break;
      case SERVICE_DEMAND_START: dw = 1; break;
      case SERVICE_DISABLED:     dw = 2; break;
      }
      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_STARTTYPE), dw);

      switch (pConfig->dwErrorControl) {
      case SERVICE_ERROR_IGNORE:   dw = 0; break;
      case SERVICE_ERROR_NORMAL:   dw = 1; break;
      case SERVICE_ERROR_SEVERE:   dw = 2; break;
      case SERVICE_ERROR_CRITICAL: dw = 3; break;

      }
      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_ERRORCONTROL), dw);

      int n = ComboBox_SelectString(GetDlgItem(hwnd, IDC_GROUP), 
         -1, pConfig->lpLoadOrderGroup);
      if (n == CB_ERR) 
         ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_GROUP), 0);

      FixDependencyString(pConfig->lpDependencies, 0, TEXT(','));
      SetDlgItemText(hwnd, IDC_DEPENDENCIES, pConfig->lpDependencies);
      SetDlgItemText(hwnd, IDC_USERNAME, pConfig->lpServiceStartName);
      SetDlgItemText(hwnd, IDC_USERPSWD, TEXT(""));

      // Display service failure actions configuration.parameters
      const SERVICE_FAILURE_ACTIONS* pFailureActions = 
         sc.QueryFailureActions();
      chASSERT(pFailureActions != NULL);
      // Clear the actions
      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_FIRSTFAILACTION), 0);
      SetDlgItemText(hwnd, IDC_FIRSTFAILDELAY, TEXT(""));

      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SECONDFAILACTION), 0);
      SetDlgItemText(hwnd, IDC_SECONDFAILDELAY, TEXT(""));
   
      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SUBSEQUENTFAILACTION), 0);
      SetDlgItemText(hwnd, IDC_SUBSEQUENTFAILDELAY, TEXT(""));

      if (pFailureActions != NULL) {
         SetDlgItemInt(hwnd, IDC_RESETPERIOD, 
            pFailureActions->dwResetPeriod, FALSE);
         SetDlgItemText(hwnd, IDC_REBOOTMSG, pFailureActions->lpRebootMsg);
         SetDlgItemText(hwnd, IDC_RUNCMD, pFailureActions->lpCommand);

         dw = min(chDIMOF(g_nCtlIdFailAction), pFailureActions->cActions);
         for (UINT n = 0; n < dw; n++) {
            ComboBox_SetCurSel(GetDlgItem(hwnd, g_nCtlIdFailAction[n]), 
               pFailureActions->lpsaActions[n].Type);
            SetDlgItemInt(hwnd, g_nCtlIdFailDelay[n], 
               pFailureActions->lpsaActions[n].Delay, FALSE);
         }
      }
   } else {
      // The service name is not in the SCM's database
   }
   ListBox_ResetContent(GetDlgItem(hwnd, IDC_STATUSLOG));
   EnableWindow(GetDlgItem(hwnd, IDC_CREATE),      !fOk);
   EnableWindow(GetDlgItem(hwnd, IDC_RECONFIGURE),  fOk);
   EnableWindow(GetDlgItem(hwnd, IDC_REMOVE),       fOk);
   EnableWindow(GetDlgItem(hwnd, IDC_SECURITY),     fOk);

   Dlg_SetControlButtons(hwnd);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_CreateOrConfigureService(HWND hwnd, BOOL fCreate) {

   TCHAR szInternalName[300];
   TCHAR szDisplayName[300];
   TCHAR szDescription[1024];
   TCHAR szPathname[_MAX_PATH];
   TCHAR szGroup[300], szDependencies[300] = { 0 };
   TCHAR szUserName[100], szUserPswd[100];
   DWORD dwServiceType = 0, dwStartType = 0, dwErrorControl = 0;

   GetDlgItemText(hwnd, IDC_INTERNALNAME, 
      szInternalName, chDIMOF(szInternalName));
   GetDlgItemText(hwnd, IDC_DISPLAYNAME, 
      szDisplayName, chDIMOF(szDisplayName));
   GetDlgItemText(hwnd, IDC_DESCRIPTION, 
      szDescription, chDIMOF(szDescription));
   GetDlgItemText(hwnd, IDC_PATHNAME, szPathname, chDIMOF(szPathname));
   GetDlgItemText(hwnd, IDC_GROUP, szGroup, chDIMOF(szGroup));
   if (lstrcmpi(szGroup, TEXT("(none)")) == 0) szGroup[0] = 0;
   GetDlgItemText(hwnd, IDC_DEPENDENCIES, 
      szDependencies, chDIMOF(szDependencies));
   FixDependencyString(szDependencies, TEXT(','), 0);

   GetDlgItemText(hwnd, IDC_USERNAME, szUserName, chDIMOF(szUserName));
   GetDlgItemText(hwnd, IDC_USERPSWD, szUserPswd, chDIMOF(szUserPswd));
   switch (ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SERVICETYPE))) {
   case 0: dwServiceType = SERVICE_WIN32_OWN_PROCESS;   break;
   case 1: dwServiceType = SERVICE_WIN32_SHARE_PROCESS; break;
   }
   if (Button_GetCheck(GetDlgItem(hwnd, IDC_INTERACTWITHDESKTOP)) 
      == BST_CHECKED) 
      dwServiceType |= SERVICE_INTERACTIVE_PROCESS;

   switch (ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_STARTTYPE))) {
   case 0: dwStartType = SERVICE_AUTO_START; break;
   case 1: dwStartType = SERVICE_DEMAND_START; break;
   case 2: dwStartType = SERVICE_DISABLED; break;
   }

   switch (ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_ERRORCONTROL))) {
   case 0: dwErrorControl = SERVICE_ERROR_IGNORE;   break;
   case 1: dwErrorControl = SERVICE_ERROR_NORMAL;   break;
   case 2: dwErrorControl = SERVICE_ERROR_SEVERE;   break;
   case 3: dwErrorControl = SERVICE_ERROR_CRITICAL; break;
   }

   BOOL fOk = FALSE;
   CServiceCtrl sc;
   if (fCreate) {
      fOk = sc.InstallAndOpen(g_scm, szInternalName, szDisplayName, 
         szDescription, dwServiceType, dwStartType, dwErrorControl, 
         szPathname, szGroup, szDependencies, szUserName, szUserPswd,
         SERVICE_START);
   } else {
      sc.Open(g_scm, TRUE, szInternalName, 
         SERVICE_CHANGE_CONFIG | SERVICE_START);
      fOk = sc.ChangeConfig(szDisplayName, szDescription,
         dwServiceType, dwStartType, dwErrorControl, szPathname, 
         szGroup, szDependencies, szUserName, szUserPswd);
   }

   if (fOk) {
      TCHAR szRebootMsg[200], szRunCmd[300];
      GetDlgItemText(hwnd, IDC_REBOOTMSG, szRebootMsg, chDIMOF(szRebootMsg));
      GetDlgItemText(hwnd, IDC_RUNCMD, szRunCmd, chDIMOF(szRunCmd));
      fOk = fOk && sc.ChangeFailureActions(
         GetDlgItemInt(hwnd, IDC_RESETPERIOD, NULL, FALSE),
         szRebootMsg, szRunCmd, 
         (SC_ACTION_TYPE) ComboBox_GetCurSel(
            GetDlgItem(hwnd, IDC_FIRSTFAILACTION)), 
         GetDlgItemInt(hwnd, IDC_FIRSTFAILDELAY, NULL, FALSE),
         (SC_ACTION_TYPE) ComboBox_GetCurSel(
            GetDlgItem(hwnd, IDC_SECONDFAILACTION)), 
         GetDlgItemInt(hwnd, IDC_SECONDFAILDELAY, NULL, FALSE),
         (SC_ACTION_TYPE) ComboBox_GetCurSel(
            GetDlgItem(hwnd, IDC_SUBSEQUENTFAILACTION)), 
         GetDlgItemInt(hwnd, IDC_SUBSEQUENTFAILDELAY, NULL, FALSE),
         (SC_ACTION_TYPE) -1);
   }

   if (fCreate) {
      chMB(fOk ? "Service creation sucessful." : "Service creation failed.");
   } else {
      if (fOk) {
         chMB("Service reconfiguration sucessful.");
      } else {
         ShowSCMLockStatus(TEXT("Service reconfiguration failed."));
      }
   }
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_RefreshServiceData(HWND hwnd) {

   TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
   GetDlgItemText(hwnd, IDC_MACHINE, szComputerName, chDIMOF(szComputerName));
   CheckDlgButton(hwnd, IDC_LOCKSCM, BST_UNCHECKED);   

   g_scm.Open(SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | 
      SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_LOCK | 
      SC_MANAGER_QUERY_LOCK_STATUS, szComputerName);
   if (!g_scm.OpenOK()) {
      chMB("Open SCM on specified machine failed.");

      // Force selection of local machine
      DWORD cb = chDIMOF(szComputerName);
      GetComputerName(szComputerName, &cb);
      SetDlgItemText(hwnd, IDC_MACHINE, szComputerName);
      g_scm.Open(SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | 
         SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_LOCK | 
         SC_MANAGER_QUERY_LOCK_STATUS, NULL);
   }

   HWND hwndCB = GetDlgItem(hwnd, IDC_INTERNALNAME);
   TCHAR szInternalName[300];
   ComboBox_GetText(hwndCB, szInternalName, chDIMOF(szInternalName));

   ComboBox_ResetContent(hwndCB);
   g_scm.CreateStatusSnapshot(SERVICE_STATE_ALL);
   for (int nIndex = 0; nIndex < g_scm.GetStatusSnapshotNum(); nIndex++) {
      const ENUM_SERVICE_STATUS* pService = 
         g_scm.GetStatusSnapshotEntry(nIndex);
      ComboBox_AddString(hwndCB, pService->lpServiceName);
   }
   if (CB_ERR == ComboBox_SelectString(hwndCB, -1, szInternalName)) 
      ComboBox_SetCurSel(hwndCB, 0);
   Dlg_ShowServiceData(hwnd);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_SUPERSCP);

   // Default to local machine's SCM
   TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
   DWORD cb = chDIMOF(szComputerName);
   GetComputerName(szComputerName, &cb);
   SetDlgItemText(hwnd, IDC_MACHINE, szComputerName);

   // Populate the controls of the dialog box
   HWND hwndCB = GetDlgItem(hwnd, IDC_SERVICETYPE);
   ComboBox_AddString(hwndCB, TEXT("Own process"));
   ComboBox_AddString(hwndCB, TEXT("Shares process"));

   hwndCB = GetDlgItem(hwnd, IDC_STARTTYPE);
   ComboBox_AddString(hwndCB, TEXT("Automatic"));
   ComboBox_AddString(hwndCB, TEXT("Demand"));
   ComboBox_AddString(hwndCB, TEXT("Disabled"));

   hwndCB = GetDlgItem(hwnd, IDC_ERRORCONTROL);
   ComboBox_AddString(hwndCB, TEXT("Ignore"));
   ComboBox_AddString(hwndCB, TEXT("Normal"));
   ComboBox_AddString(hwndCB, TEXT("Severe"));
   ComboBox_AddString(hwndCB, TEXT("Critical"));

   for (int n = 0; n < chDIMOF(g_nCtlIdFailAction); n++) {
      hwndCB = GetDlgItem(hwnd, g_nCtlIdFailAction[n]);
      int x;
      x = ComboBox_AddString(hwndCB, TEXT("None"));
      chASSERT(x == SC_ACTION_NONE);
      x = ComboBox_AddString(hwndCB, TEXT("Restart"));
      chASSERT(x == SC_ACTION_RESTART);
      x = ComboBox_AddString(hwndCB, TEXT("Reboot"));
      chASSERT(x == SC_ACTION_REBOOT);
      x = ComboBox_AddString(hwndCB, TEXT("Run Cmd"));
      chASSERT(x == SC_ACTION_RUN_COMMAND);
   }

   // Populate the Dependency Group combo box
   hwndCB = GetDlgItem(hwnd, IDC_GROUP);
   HKEY hkey;
   RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
      TEXT("SYSTEM\\CurrentControlSet\\Control\\ServiceGroupOrder"),
      0, KEY_QUERY_VALUE, &hkey);

   CAutoBuf<TCHAR, sizeof(TCHAR)> p;
   // Give buffer a size > 0 so that RegQueryValueEx returns 
   // ERROR_MORE_DATA instead of ERROR_SUCCESS.
   p = 1;  
   while (RegQueryValueEx(hkey, TEXT("List"), NULL, NULL, p, p) 
      == ERROR_MORE_DATA) ;
   RegCloseKey(hkey);

   ComboBox_AddString(hwndCB, TEXT("(none)"));
   for (PCTSTR q = p; q[0] != 0; q += lstrlen(q) + 1) {
      ComboBox_AddString(hwndCB, q);
   }

   // Default user-defined code
   SetDlgItemInt(hwnd, IDC_CODEVALUE, 128, FALSE);

   // Select the first service
   Dlg_RefreshServiceData(hwnd);
   FORWARD_WM_COMMAND(hwnd, IDC_INTERNALNAME, 
      GetDlgItem(hwnd, IDC_INTERNALNAME), CBN_EDITCHANGE, PostMessage);

   chVERIFY(SetTimer(hwnd, 1, 1000, NULL) == 1);
   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnTimer(HWND hwnd, UINT id) {

   HWND hwndLB = GetDlgItem(hwnd, IDC_STATUSLOG);
   BOOL fOk = g_scm.OpenOK();
   if (fOk) {
      TCHAR szInternalName[1024];
      ComboBox_GetText(GetDlgItem(hwnd, IDC_INTERNALNAME), 
         szInternalName, chDIMOF(szInternalName));

      CServiceCtrl sc(g_scm, TRUE, szInternalName, SERVICE_QUERY_STATUS);
      SERVICE_STATUS ss = { 0 };
      fOk = (sc.OpenOK() && sc.QueryStatus(&ss));
      if (fOk) {

         TCHAR sz[512];
         PCTSTR pszState = TEXT("Unknown");
         switch (ss.dwCurrentState) {
         case SERVICE_STOPPED:          
            pszState = TEXT("Stopped"); 
            break;

         case SERVICE_START_PENDING:    
            pszState = TEXT("Start pending"); 
            break;

         case SERVICE_STOP_PENDING:     
            pszState = TEXT("Stop pending"); 
            break;

         case SERVICE_RUNNING:          
            pszState = TEXT("Running"); 
            break;

         case SERVICE_CONTINUE_PENDING: 
            pszState = TEXT("Continue pending"); 
            break;

         case SERVICE_PAUSE_PENDING:    
            pszState = TEXT("Pause pending"); 
            break;

         case SERVICE_PAUSED:           
            pszState = TEXT("Paused"); 
            break;
         }
         wsprintf(sz, TEXT("%d: %s (ChkPt=%u, WtHnt=%u), WErr=%u, SErr=%u"),
            ListBox_GetCount(hwndLB), pszState, ss.dwCheckPoint, ss.dwWaitHint, 
            ss.dwWin32ExitCode, ss.dwServiceSpecificExitCode); 
         ListBox_SetCurSel(hwndLB, ListBox_AddString(hwndLB, sz));
      }
   } 
   
   if (!fOk) 
      ListBox_ResetContent(hwndLB);

   Dlg_SetControlButtons(hwnd);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   CServiceCtrl sc; 
   TCHAR szInternalName[300];
   ComboBox_GetText(GetDlgItem(hwnd, IDC_INTERNALNAME), 
      szInternalName, chDIMOF(szInternalName));

   switch (id) {
   case IDC_REFRESH:
      Dlg_RefreshServiceData(hwnd);
      break;

   case IDC_INTERNALNAME:
      switch (codeNotify) {
      case CBN_SELCHANGE:
         // Post the edit change to resolve a timing issue
         FORWARD_WM_COMMAND(hwnd, id, hwndCtl, CBN_EDITCHANGE, PostMessage);
         break;

      case CBN_EDITCHANGE:
         Dlg_ShowServiceData(hwnd);
         break;
      }
      break;

   case IDC_CREATE:
      Dlg_CreateOrConfigureService(hwnd, TRUE);
      Dlg_RefreshServiceData(hwnd);
      break;

   case IDC_BROWSE:
      {
      TCHAR szPathname[_MAX_PATH];
      OPENFILENAME ofn;
      ZeroMemory(&ofn, sizeof(ofn));
      ofn.lStructSize = sizeof(ofn); 
      ofn.hwndOwner = hwnd; 
      ofn.lpstrFilter = TEXT("Executable files\0*.exe\0"); 
      szPathname[0] = 0;
      ofn.lpstrFile = szPathname;
      ofn.nMaxFile = chDIMOF(szPathname);         
      ofn.lpstrTitle = TEXT("Select Service");
      ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST; 
      if (GetOpenFileName(&ofn))
         Edit_SetText(GetDlgItem(hwnd, IDC_PATHNAME), szPathname);
      }
      break;

   case IDC_RECONFIGURE:
      Dlg_CreateOrConfigureService(hwnd, FALSE);
      Dlg_RefreshServiceData(hwnd);
      break;

   case IDC_REMOVE:
      {
      sc.Open(g_scm, TRUE, szInternalName, DELETE);
      BOOL fOk = sc.Delete(); // Mark the service for deletion
      sc.ForceClose();  // Allow the SCM to actually delete the service
      chMB(fOk ? "Service was removed successfully." 
         : "Service could not be removed.");
      Dlg_RefreshServiceData(hwnd);
      }
      break;

   case IDC_SECURITY:
      sc.Open(g_scm, TRUE, szInternalName, 
         READ_CONTROL | WRITE_DAC | SERVICE_QUERY_CONFIG);
      sc.EditSecurity(hwnd);
      break;

   case IDC_LOCKSCM:
      {
      BOOL fLock = (IsDlgButtonChecked(hwnd, IDC_LOCKSCM) == BST_CHECKED);
      if (!g_scm.LockDatabase(fLock)) {
         ShowSCMLockStatus(TEXT("Locking SCM data base failed."));
         CheckDlgButton(hwnd, IDC_LOCKSCM, BST_UNCHECKED);
      }
      }
      break;

   case IDC_START:
      {
      TCHAR szStartupParams[200];
      GetDlgItemText(hwnd, IDC_STARTUPPARAMS, szStartupParams, 
         chDIMOF(szStartupParams));
      sc.Open(g_scm, TRUE, szInternalName, SERVICE_START);
      sc.Start(szStartupParams);
      }
      break;

   case IDC_STOP:
      {
      sc.Open(g_scm, TRUE, szInternalName, 
         SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS);
      BOOL fOk = sc.Control(SERVICE_CONTROL_STOP);
      if (!fOk && (GetLastError() == ERROR_DEPENDENT_SERVICES_RUNNING)) {
         // Tell the user why we can't stop the service
         sc.CreateDependencySnapshot(SERVICE_ACTIVE);
         TCHAR sz[2048] = TEXT("The service can't be stopped because ")
            TEXT("these services depend on it:\n");
         int nIndex = 0;
         for (; nIndex < sc.GetDependencySnapshotNum(); nIndex++) {
            lstrcat(sz, sc.GetDependencySnapshotEntry(nIndex)->lpDisplayName);
            lstrcat(sz, TEXT("\n"));
         }
         MessageBox(hwnd, sz, TEXT("Super SCP"), MB_OK);
      }
      }
      break;
   
   case IDC_PAUSE:
      sc.Open(g_scm, TRUE, szInternalName, SERVICE_PAUSE_CONTINUE);
      sc.Control(SERVICE_CONTROL_PAUSE);
      break;
   
   case IDC_CONTINUE:
      sc.Open(g_scm, TRUE, szInternalName, SERVICE_PAUSE_CONTINUE);
      sc.Control(SERVICE_CONTROL_CONTINUE);
      break;
   
   case IDC_INTERROGATE:
      sc.Open(g_scm, TRUE, szInternalName, SERVICE_INTERROGATE);
      sc.Control(SERVICE_CONTROL_INTERROGATE);
      break;

   case IDC_PARAMCHANGE:
      sc.Open(g_scm, TRUE, szInternalName, SERVICE_PAUSE_CONTINUE);
      sc.Control(SERVICE_CONTROL_PARAMCHANGE);
      break;

   case IDC_USERCODE:
      sc.Open(g_scm, TRUE, szInternalName, SERVICE_USER_DEFINED_CONTROL);
      sc.Control(GetDlgItemInt(hwnd, IDC_CODEVALUE, NULL, FALSE));
      break;

   case IDCANCEL:
      KillTimer(hwnd, 1);
      EndDialog(hwnd, id);
      break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnDropFiles(HWND hwnd, HDROP hdrop) {

   TCHAR szPathname[_MAX_PATH];
   DragQueryFile(hdrop, 0, szPathname, chDIMOF(szPathname));
   SetDlgItemText(hwnd, IDC_PATHNAME, szPathname);
   DragFinish(hdrop);
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
   chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
   chHANDLE_DLGMSG(hwnd, WM_COMMAND,    Dlg_OnCommand);
   chHANDLE_DLGMSG(hwnd, WM_DROPFILES,  Dlg_OnDropFiles);
   chHANDLE_DLGMSG(hwnd, WM_TIMER,      Dlg_OnTimer);
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_SUPERSCP), NULL, Dlg_Proc);
   return(0);
}


///////////////////////////////// End of File /////////////////////////////////
