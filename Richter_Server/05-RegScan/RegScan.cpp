/******************************************************************************
Module:  RegScan.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.
#include <WindowsX.h>
#include <stdio.h>
#include "Resource.h"

#define UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"      // See Appendix B.

#define PRINTBUF_IMPL
#include "..\ClassLib\PrintBuf.h"      // See Appendix B.

#define AUTOBUF_IMPL
#include "..\ClassLib\AutoBuf.h"       // See Appendix B.

#define REGWALK_IMPL
#include "RegWalk.h"


///////////////////////////////////////////////////////////////////////////////


class CRegScan : private CRegWalk {
public:
   CRegScan() : m_pb(256 * 1024) { }
   BOOL Go(PCTSTR pszMachine, HKEY hkeyRoot, PCTSTR pszSubkey, 
      PCTSTR pszString, BOOL fSearchKeyNames, BOOL fSearchValueNames, 
      BOOL fSearchValueData, BOOL fCaseSensitive);
   PCTSTR Result() { return(m_pb); }

   void ForceSearchStop() { m_fStopSearch = TRUE; }
   BOOL WasSearchStopped() { return(m_fStopSearch); }
   
private:
   PCTSTR m_pszString;           // String to search for
   BOOL   m_fSearchKeyNames;     // Search key names?
   BOOL   m_fSearchValueNames;   // Search values names?
   BOOL   m_fSearchValueData;    // Search value string data?
   BOOL   m_fShownThisSubkey;    // Any matches from the current subkey?
   BOOL   m_fStopSearch;         // Prematurely stop the search?
   CPrintBuf m_pb;               // Growable results buffer

   typedef PTSTR (WINAPI* PFNSTRCMP)(PCTSTR pszFirst, PCTSTR pszSearch);
   PFNSTRCMP m_pfnStrCmp;        // String comparison function

protected:
   REGWALKSTATUS onSubkey(PCTSTR pszSubkey, int nDepth, 
      BOOL fRecurseRequested);
   REGWALKSTATUS onValue(HKEY hkey, PCTSTR pszValue, int nDepth);
   void ProcessUI();
};


///////////////////////////////////////////////////////////////////////////////


BOOL CRegScan::Go(PCTSTR pszMachine, HKEY hkeyRoot, PCTSTR pszSubkey, 
   PCTSTR pszString, BOOL fSearchKeyNames, BOOL fSearchValueNames, 
   BOOL fSearchValueData, BOOL fCaseSensitive) {

   m_pszString         = pszString;
   m_fSearchKeyNames   = fSearchKeyNames;
   m_fSearchValueNames = fSearchValueNames;
   m_fSearchValueData  = fSearchValueData;
   m_pfnStrCmp         = fCaseSensitive ? StrStr : StrStrI;
   m_fShownThisSubkey  = FALSE;
   m_fStopSearch       = FALSE;
   m_pb.Clear();
   BOOL fOk = TRUE;
   if (!m_fSearchKeyNames && !m_fSearchValueNames && !m_fSearchValueData) {
      chMB("You must at least select one field to search.");
   } else fOk = CRegWalk::Go(pszMachine, hkeyRoot, pszSubkey, TRUE);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


void CRegScan::ProcessUI() {
   MSG msg;
   while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      // There are UI messages, process them.      
      if (!IsDialogMessage(GetActiveWindow(), &msg)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);  
      }   
   }
}


///////////////////////////////////////////////////////////////////////////////


CRegWalk::REGWALKSTATUS CRegScan::onSubkey(PCTSTR pszSubkey, int nDepth, 
   BOOL fRecurseRequested) {

   REGWALKSTATUS rws = RWS_FULLSTOP;
   if (fRecurseRequested || (nDepth == 0)) rws = RWS_RECURSE;

   // Get this subkey's name without the full path
   PCTSTR pszSubkeyName = PathFindFileName(pszSubkey);
   if (m_fSearchKeyNames) 
      m_fShownThisSubkey = (m_pfnStrCmp(pszSubkeyName, m_pszString) != NULL);
   else m_fShownThisSubkey = FALSE;
   
   if (m_fShownThisSubkey) {
      m_pb.Print(TEXT("%s\r\n"), pszSubkey);
   }

   ProcessUI();
   return(WasSearchStopped() ? RWS_FULLSTOP : rws);
}


///////////////////////////////////////////////////////////////////////////////


CRegWalk::REGWALKSTATUS CRegScan::onValue(
   HKEY hkey, PCTSTR pszValue, int nDepth) {
   
   if (m_fSearchValueNames && (m_pfnStrCmp(pszValue, m_pszString) != NULL)) {
      if (!m_fShownThisSubkey) {
         m_pb.Print(TEXT("%s\r\n"), m_szSubkeyPath);
         m_fShownThisSubkey = TRUE;
      }
      m_pb.Print(TEXT("\t%s\r\n"), pszValue);
   }

   if (m_fSearchValueData) {
      // Check the value's data
      DWORD dwType;
      RegQueryValueEx(hkey, pszValue, NULL, &dwType, NULL, NULL);

      if ((dwType == REG_EXPAND_SZ) || (dwType == REG_SZ)) {
         CAutoBuf<TCHAR, sizeof(TCHAR)> szData;
         // Give buffer a size > 0 so that RegQueryValueEx returns 
         // ERROR_MORE_DATA instead of ERROR_SUCCESS.
         szData = 1;
         while (RegQueryValueEx(hkey, pszValue, NULL, NULL, szData, szData)
            == ERROR_MORE_DATA) ;

         // szData is NULL is there is no value data
         if (((PCTSTR) szData != NULL) && 
            (m_pfnStrCmp(szData, m_pszString) != NULL)) {
            
            if (!m_fShownThisSubkey) {
               m_pb.Print(TEXT("%s\r\n"), m_szSubkeyPath);
               m_fShownThisSubkey = TRUE;
            }
            m_pb.Print(TEXT("\t%s (%s)\r\n"), 
             ((pszValue[0] == 0) ? TEXT("(default)") : pszValue), 
             (PCTSTR) szData);
         }
      }
   }
   ProcessUI();
   return(WasSearchStopped() ? RWS_FULLSTOP : RWS_CONTINUE);
}


///////////////////////////////////////////////////////////////////////////////


CUILayout g_UILayout;   // Repositions controls when dialog box size changes.


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_REGSCAN);

   HWND hwndRootKey = GetDlgItem(hwnd, IDC_ROOTKEY);
   int n = 0;
   n = ComboBox_AddString(hwndRootKey, TEXT("HKEY_LOCAL_MACHINE"));
   ComboBox_SetItemData(hwndRootKey, n, HKEY_LOCAL_MACHINE);
   ComboBox_SetCurSel(hwndRootKey, n);   // HKLM is default

   n = ComboBox_AddString(hwndRootKey, TEXT("HKEY_CURRENT_CONFIG"));
   ComboBox_SetItemData(hwndRootKey, n, HKEY_CURRENT_CONFIG);
   
   n = ComboBox_AddString(hwndRootKey, TEXT("HKEY_CLASSES_ROOT"));
   ComboBox_SetItemData(hwndRootKey, n, HKEY_CLASSES_ROOT);

   n = ComboBox_AddString(hwndRootKey, TEXT("HKEY_USERS"));
   ComboBox_SetItemData(hwndRootKey, n, HKEY_USERS);

   n = ComboBox_AddString(hwndRootKey, TEXT("HKEY_CURRENT_USER"));
   ComboBox_SetItemData(hwndRootKey, n, HKEY_CURRENT_USER);

   // Set up the resizeing of the controls
   g_UILayout.Initialize(hwnd); 
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPRIGHT, 
      IDC_MACHINE, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPRIGHT, 
      IDC_ROOTKEY, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPRIGHT, 
      IDC_SUBKEY, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPRIGHT, 
      IDC_STRING, FALSE);

   g_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, 
      IDC_SEARCHKEYNAMES, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, 
      IDC_SEARCHVALUENAMES, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, 
      IDC_SEARCHVALUEDATA, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, 
      IDC_CASESENSITIVE, FALSE); 
   g_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, 
      IDC_SEARCHSTART, FALSE); 
   g_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, 
      IDC_SEARCHSTOP, FALSE); 

   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_BOTTOMRIGHT, 
      IDC_SEARCHRESULTS, FALSE);
   
   CheckDlgButton(hwnd, IDC_SEARCHKEYNAMES,   TRUE);
   CheckDlgButton(hwnd, IDC_SEARCHVALUENAMES, TRUE);
   CheckDlgButton(hwnd, IDC_SEARCHVALUEDATA,  TRUE);
   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void EnableControls(HWND hwnd, BOOL fEnable) {

   EnableWindow(GetDlgItem(hwnd, IDC_MACHINE),          fEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_ROOTKEY),          fEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SUBKEY),           fEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_STRING),           fEnable);

   EnableWindow(GetDlgItem(hwnd, IDC_SEARCHKEYNAMES),   fEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SEARCHVALUENAMES), fEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SEARCHVALUEDATA),  fEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_CASESENSITIVE),    fEnable);

   ShowWindow(GetDlgItem(hwnd, IDC_SEARCHSTART), fEnable ? SW_SHOW : SW_HIDE);
   ShowWindow(GetDlgItem(hwnd, IDC_SEARCHSTOP),  fEnable ? SW_HIDE : SW_SHOW);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   static CRegScan x;
   
   switch (id) {
   case IDCANCEL:
      EndDialog(hwnd, id);
      break;

   case IDC_SEARCHSTOP:
      x.ForceSearchStop();
      break;

   case IDC_SEARCHSTART:
      SetDlgItemText(hwnd, IDC_SEARCHRESULTS, TEXT("Scanning Registry..."));
      EnableControls(hwnd, FALSE);

      TCHAR szString[1000];
      GetDlgItemText(hwnd, IDC_STRING, szString, chDIMOF(szString));

      TCHAR szMachine[100], szSubkey[1000];
      GetDlgItemText(hwnd, IDC_MACHINE, szMachine, chDIMOF(szMachine));
      GetDlgItemText(hwnd, IDC_SUBKEY, szSubkey, chDIMOF(szSubkey));
      HWND hwndRootKey = GetDlgItem(hwnd, IDC_ROOTKEY);
      int nIndex = ComboBox_GetCurSel(hwndRootKey);
      HKEY hkeyRoot = (HKEY) ComboBox_GetItemData(hwndRootKey, nIndex);

      if (!x.Go(
         (szMachine[0] == 0) ? NULL : szMachine, hkeyRoot, szSubkey, 
         szString,
         IsDlgButtonChecked(hwnd, IDC_SEARCHKEYNAMES), 
         IsDlgButtonChecked(hwnd, IDC_SEARCHVALUENAMES), 
         IsDlgButtonChecked(hwnd, IDC_SEARCHVALUEDATA), 
         IsDlgButtonChecked(hwnd, IDC_CASESENSITIVE))) {
         chMB("Couldn't access the registry");
      }

      SetDlgItemText(hwnd, IDC_SEARCHRESULTS, 
         x.WasSearchStopped() ? TEXT("Scan Canceled") : 
            ((x.Result()[0] == 0) ? TEXT("No entries found") : x.Result()));
      EnableControls(hwnd, TRUE);         
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


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_REGSCAN), NULL, Dlg_Proc);
   return(0);
}


//////////////////////////////// End of File //////////////////////////////////
