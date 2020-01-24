/******************************************************************************
Module:  AccessMaster.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"              /* See Appendix A. */
#include <WindowsX.h>
#include <ACLAPI.h>
#include <ACLUI.h>
#include "Resource.h"

// Force linking against the ACLUI library
#pragma comment(lib, "ACLUI.lib")   

#define PRINTBUF_IMPL
#include "..\ClassLib\PrintBuf.h"

#include "AccessData.h"


///////////////////////////////////////////////////////////////////////////////


#ifndef UNICODE
#error This module must be compiled natively using Unicode.
#endif


///////////////////////////////////////////////////////////////////////////////


void ReportError(PTSTR szFunction, ULONG lErr) {

   CPrintBuf prntBuf;
   prntBuf.Print(TEXT("The Function:  %s\r\n"), szFunction);
   prntBuf.Print(TEXT("Caused the following error - \r\n"));
   prntBuf.PrintError(lErr);
   MessageBox(NULL, prntBuf, TEXT("AccessMaster Error"), MB_OK);
}


///////////////////////////////////////////////////////////////////////////////


void UpdateObjDependentCtrls(HWND hwnd) {
   
   // Setup controls for selected object type
   HWND hwndCtrl = GetDlgItem(hwnd, IDC_TYPE);
   int nIndex = ComboBox_GetCurSel(hwndCtrl);

   SetDlgItemText(hwnd, IDE_USAGE, g_objMap[nIndex].m_pszUsageText);

   hwndCtrl = GetDlgItem(hwnd, IDE_NAME);
   EnableWindow(hwndCtrl, g_objMap[nIndex].m_fUseName);

   hwndCtrl = GetDlgItem(hwnd, IDE_HANDLE);
   EnableWindow(hwndCtrl, g_objMap[nIndex].m_fUseHandle);

   hwndCtrl = GetDlgItem(hwnd, IDE_PID);
   EnableWindow(hwndCtrl, g_objMap[nIndex].m_fUsePID);

   if (g_objMap[nIndex].m_fUsePID || g_objMap[nIndex].m_fUseHandle) {
      hwndCtrl = GetDlgItem(hwnd, IDR_HANDLE);
      EnableWindow(hwndCtrl, TRUE);
   } else {
      hwndCtrl = GetDlgItem(hwnd, IDR_HANDLE);
      EnableWindow(hwndCtrl, FALSE);
      CheckRadioButton(hwnd, IDR_NAME, IDR_HANDLE, IDR_NAME);
   }

   if (g_objMap[nIndex].m_fUseName) {
      hwndCtrl = GetDlgItem(hwnd, IDR_NAME);
      EnableWindow(hwndCtrl, TRUE);
   } else {
      hwndCtrl = GetDlgItem(hwnd, IDR_NAME);
      EnableWindow(hwndCtrl, FALSE);
      CheckRadioButton(hwnd, IDR_NAME, IDR_HANDLE, IDR_HANDLE);
   }
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_ACCESSMASTER);

   CheckDlgButton(hwnd, IDR_NAME, BST_CHECKED);

   TCHAR szTitle[1024];
   lstrcpy(szTitle, TEXT("AccessMaster is running as \""));
   ULONG lSize = chDIMOF(szTitle)-lstrlen(szTitle);
   GetUserName(szTitle+lstrlen(szTitle),&lSize);
   lstrcat(szTitle, TEXT("\""));
   SetWindowText(hwnd, szTitle);

   // Set-up the object type combo
   int nIndex = chDIMOF(g_objMap);
   HWND hwndCtrl = GetDlgItem(hwnd, IDC_TYPE);
   while (nIndex-- != 0) {
      ComboBox_InsertString(hwndCtrl, 0, g_objMap[nIndex].m_pszComboText);
   }

   ComboBox_SetCurSel(hwndCtrl, 0);
   UpdateObjDependentCtrls(hwnd);

   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void HandleType(HWND hwnd, UINT codeNotify, HWND hwndCtl) {

   if (codeNotify == CBN_SELCHANGE)
      UpdateObjDependentCtrls(hwnd);
}


///////////////////////////////////////////////////////////////////////////////


void HandleRadio(HWND hwnd, UINT codeNotify, UINT nCtrl) {
   
   if (codeNotify == EN_SETFOCUS)
      CheckRadioButton(hwnd, IDR_NAME, IDR_HANDLE, nCtrl);
}


///////////////////////////////////////////////////////////////////////////////


BOOL FillInfo(HWND hwnd, ObjInf* pInfo) {
   
   BOOL fReturn = FALSE;

   // Map object type to data block in the object map
   HWND hwndCtrl = GetDlgItem(hwnd, IDC_TYPE);
   int nIndex = ComboBox_GetCurSel(hwndCtrl);
   pInfo->m_pEntry = g_objMap + nIndex;

   // Copy the object's name into the info block for building the title text
   lstrcpy(pInfo->m_szObjectName, pInfo->m_pEntry->m_pszComboText);

   // Is it a named item?
   if (IsDlgButtonChecked(hwnd, IDR_NAME)) {
      switch (pInfo->m_pEntry->m_nSpecificType) {
         
         case AM_WINDOWSTATION:
            { // If windowstation, we must translate the name to a handle
               HWINSTA hwinsta = NULL;
               GetDlgItemText(hwnd, IDE_NAME, pInfo->m_szName, 
                     chDIMOF(pInfo->m_szName));
               // Get the maximum possible access
               hwinsta = OpenWindowStation(pInfo->m_szName, FALSE, 
                        MAXIMUM_ALLOWED);

               if (hwinsta == NULL) // Still failed?
                  ReportError(TEXT("OpenWindowStation"), GetLastError());
               else { // Otherwise finish title text
                  fReturn = TRUE;
                  pInfo->m_hHandle = (HANDLE) hwinsta;
                  lstrcat(pInfo->m_szObjectName, TEXT("-"));
                  lstrcat(pInfo->m_szObjectName, pInfo->m_szName); 
                  pInfo->m_szName[0] = 0;
               }
            }
            break;

         case AM_DESKTOP:
            { // If desktop, we must translate the name to a handle
               HWINSTA hwinstaOld;
               HWINSTA hwinstaTemp;
               HDESK hdesk=NULL;
               PTSTR pszWinSta;
               PTSTR pszDesktop;
               int nIndex;

               GetDlgItemText(hwnd, IDE_NAME, pInfo->m_szName, 
                     chDIMOF(pInfo->m_szName));
               pszWinSta = pInfo->m_szName;
               nIndex = lstrlen(pInfo->m_szName);
               
               // Parse the text for windowstation and desktop
               while (nIndex-- != 0) {

                  if (pInfo->m_szName[nIndex] == TEXT('\\') 
                        || pInfo->m_szName[nIndex] == TEXT('/')) {

                     pInfo->m_szName[nIndex] = 0;
                     break;
                  }
               }
            
               // Desktop string
               nIndex++;
               pszDesktop = pInfo->m_szName + nIndex;
               // Open the windowstation
               hwinstaTemp = OpenWindowStation(pszWinSta, FALSE, 
                     DESKTOP_ENUMERATE);
               if (hwinstaTemp != NULL) {
                  // Save the last one
                  hwinstaOld = GetProcessWindowStation();
                  SetProcessWindowStation(hwinstaTemp);
                  // Get maximum access to the desktop
                  hdesk = OpenDesktop(pszDesktop, 0, FALSE, 
                           MAXIMUM_ALLOWED);
                  if (hdesk == NULL)// failed?
                     ReportError(TEXT("OpenDesktop"), GetLastError());
                  else { // build title
                     fReturn = TRUE; 
                     pInfo->m_hHandle = (HANDLE) hdesk;
                     lstrcat(pInfo->m_szObjectName, TEXT("-"));
                     lstrcat(pInfo->m_szObjectName, pszDesktop);
                     pInfo->m_szName[0] = 0;
                  }
                  
                  // Close and reset window stations for the process
                  CloseWindowStation(hwinstaTemp);
                  SetProcessWindowStation(hwinstaOld);
            
               } else // Failed open winsta
                  ReportError(TEXT("OpenWindowStation"), GetLastError());
            }
            break;

         default: // The rest of named objects work with GetNamedSecurity...
            GetDlgItemText(hwnd, IDE_NAME, pInfo->m_szName, 
                  chDIMOF(pInfo->m_szName));
            lstrcat(pInfo->m_szObjectName, TEXT("-"));
            lstrcat(pInfo->m_szObjectName, pInfo->m_szName);
            fReturn = TRUE;
            break;
      }
   
   } else { // Is it a handle and or process id we are dealing with?

      BOOL fTrans;
      // Get the actual numbers
      ULONG lPid = GetDlgItemInt(hwnd, IDE_PID, &fTrans, FALSE);
      HANDLE hHandle = (HANDLE) GetDlgItemInt(hwnd, IDE_HANDLE, &fTrans, 
            FALSE);
      HANDLE hObj = NULL;

      switch (pInfo->m_pEntry->m_nSpecificType) {

         case AM_THREAD: // Maximum access to the thread
            hObj = OpenThread(MAXIMUM_ALLOWED, FALSE, lPid);
            if (hObj == NULL) // None == failure
               ReportError(TEXT("OpenThread"), GetLastError());
            break;

         case AM_PROCESS: // Get maximum access to the process
            hObj = OpenProcess(MAXIMUM_ALLOWED, FALSE, lPid);
            if (hObj == NULL) // None == failure
               ReportError(TEXT("OpenProcess"), GetLastError());
            break;

         default: // The rest work with duplicate handle
            {
               HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, lPid);
               if (hProcess != NULL) {
                  // Get as much access as possible
                  if (!DuplicateHandle(hProcess, hHandle, GetCurrentProcess(),
                     &hObj, MAXIMUM_ALLOWED, FALSE, 0))
                     ReportError(TEXT("DuplicateHandle"), GetLastError());
               } else
                  ReportError(TEXT("OpenProcess"), GetLastError());
            }
            break;
      }

      if (hObj != NULL) {
         pInfo->m_hHandle = hObj;
         fReturn = TRUE;
      }
   }

   // Test object availability 
   if (fReturn) {
      ULONG lErr;
      PSECURITY_DESCRIPTOR pSD = NULL;
      if (pInfo->m_szName[0] != 0) // Is it named
         lErr = GetNamedSecurityInfo(pInfo->m_szName, 
                pInfo->m_pEntry->m_objType, DACL_SECURITY_INFORMATION, 
                NULL, NULL, NULL, NULL, &pSD);
      else // Is it a handle case
         lErr = GetSecurityInfo(pInfo->m_hHandle, pInfo->m_pEntry->m_objType,
                DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &pSD);

      if ((lErr != ERROR_ACCESS_DENIED) && (lErr != ERROR_SUCCESS)){
         ReportError(TEXT("Get[Named]SecurityInfo"), lErr);
         fReturn = FALSE;
      }
      else {
         LocalFree(pSD);
      }
   }

   return(fReturn);
}


///////////////////////////////////////////////////////////////////////////////


SI_INHERIT_TYPE CSecurityInformation::m_siInheritType[] = {
   {&m_guidNULL, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
            TEXT("Child")},
   {&m_guidNULL, OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE, TEXT("Child")}
};


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::QueryInterface(REFIID riid, PVOID* ppvObj) {

   HRESULT hr = E_NOINTERFACE;
   if ((riid == IID_ISecurityInformation) || (riid == IID_IUnknown)) {
      *ppvObj = this;
      AddRef();
      hr = S_OK;
   }
   return(hr);
}


///////////////////////////////////////////////////////////////////////////////


ULONG CSecurityInformation::AddRef() {
   
   m_nRef++;
   return(m_nRef);
}


///////////////////////////////////////////////////////////////////////////////


ULONG CSecurityInformation::Release() {
   
   ULONG nRef = --m_nRef;
   if (m_nRef == 0)
      delete this;
   return(nRef);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetObjectInformation(
      PSI_OBJECT_INFO pObjectInfo) {

   // We are doing both normal and advanced editing
   pObjectInfo->dwFlags = SI_EDIT_ALL  | SI_ADVANCED;

   // Is it a container?
   if (m_pInfo->m_pEntry->m_fIsContainer) {
      pObjectInfo->dwFlags  |= SI_CONTAINER;
   }

   // Is it a child?
   if (!m_pInfo->m_pEntry->m_fIsChild) {
      pObjectInfo->dwFlags  |= SI_NO_ACL_PROTECT;
   }

   pObjectInfo->hInstance = GetModuleHandle(NULL);
   pObjectInfo->pszServerName = NULL;
   pObjectInfo->pszObjectName = m_pInfo->m_szObjectName;
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetSecurity(
      SECURITY_INFORMATION RequestedInformation, 
      PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) {

   HRESULT hr = 1;
   PSECURITY_DESCRIPTOR pSD;

   // Get security information
   ULONG lErr;
   if (m_pInfo->m_szName[0] != 0) // Is it named
      lErr = GetNamedSecurityInfo(m_pInfo->m_szName, 
             m_pInfo->m_pEntry->m_objType, RequestedInformation, NULL, NULL, 
             NULL, NULL, &pSD);
   else // Is it a handle case
      lErr = GetSecurityInfo(m_pInfo->m_hHandle, m_pInfo->m_pEntry->m_objType,
             RequestedInformation, NULL, NULL, NULL, NULL, &pSD);

   // No matter what we still display security information
   if (lErr != ERROR_SUCCESS){ // Failure produces an empty SD
      ReportError(TEXT("GetNamedSecurityInfo"), lErr);
      MessageBox(NULL, TEXT("An error occurred retrieving security ")
         TEXT("information for this object, \npossibly due to insufficient")
         TEXT(" access rights.  AccessMaster \nhas created an empty security")
         TEXT(" descriptor for editing."), TEXT("AccessMaster Notice"), MB_OK);
   }
   else {
      hr = S_OK;
      *ppSecurityDescriptor = pSD;
   }

   return(hr);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetAccessRights(const GUID* pguidObjectType,
      DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, 
      ULONG* piDefaultAccess) {

   // If the binary check box was set, we show only raw binary ACE information
   if (m_fBinary) {

      *pcAccesses = 32;
      *ppAccess = m_siAccessBinaryRights;
   
   } else { // Otherwise locate the appropriate block of specific rights
   
      // See AccessData.H header file
      *ppAccess = m_siAccessAllRights[m_pInfo->m_pEntry->m_nSpecificType];
      *pcAccesses = 0;
      while ((*ppAccess)[*pcAccesses].mask != 0)
         (*pcAccesses)++;
      *piDefaultAccess = 0;
   }
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::MapGeneric(const GUID* pguidObjectType, 
      UCHAR* pAceFlags, ACCESS_MASK* pMask) {
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes,
      ULONG* pcInheritTypes) {

   *pcInheritTypes = 1;
   // If it is a container pass static enherit information for containers
   if (m_pInfo->m_pEntry->m_fIsContainer) {
   
      *ppInheritTypes = &(m_siInheritType[0]);
   
   } 
   else { // If it is a child pass static enherit information for containers
      
      if (m_pInfo->m_pEntry->m_fIsChild) {
      
         *ppInheritTypes = &(m_siInheritType[1]);
      
      } 
      else { // If niether, no inheritance

         *ppInheritTypes = NULL;
         *pcInheritTypes = 0;
      }
   }
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::PropertySheetPageCallback(HWND hwnd, UINT uMsg, 
      SI_PAGE_TYPE uPage) {

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::SetSecurity(
      SECURITY_INFORMATION SecurityInformation,
      PSECURITY_DESCRIPTOR pSecurityDescriptor) {

   HRESULT hr = 1;
   
   // Get the Dacl
   PACL pDACL = NULL;
   BOOL fPresent, fDefaulted;
   GetSecurityDescriptorDacl(pSecurityDescriptor, &fPresent, &pDACL, 
         &fDefaulted);

   // Get the SACL
   PACL pSACL = NULL;
   GetSecurityDescriptorSacl(pSecurityDescriptor, &fPresent, &pSACL, 
         &fDefaulted);

   // Get the owner
   PSID psidOwner = NULL;
   GetSecurityDescriptorOwner(pSecurityDescriptor, &psidOwner, &fDefaulted);

   // Get the group
   PSID psidGroup = NULL;
   GetSecurityDescriptorOwner(pSecurityDescriptor, &psidGroup, &fDefaulted);

   // Find out if DACL and SACL inherit from parent objects
   SECURITY_DESCRIPTOR_CONTROL sdCtrl = NULL;
   ULONG ulRevision;
   GetSecurityDescriptorControl(pSecurityDescriptor, &sdCtrl, &ulRevision);
   if ((sdCtrl & SE_DACL_PROTECTED) != SE_DACL_PROTECTED)
      SecurityInformation  |= UNPROTECTED_DACL_SECURITY_INFORMATION;
   else
      SecurityInformation  |= PROTECTED_DACL_SECURITY_INFORMATION;

   if ((sdCtrl & SE_SACL_PROTECTED) != SE_SACL_PROTECTED)
      SecurityInformation  |= UNPROTECTED_SACL_SECURITY_INFORMATION;
   else
      SecurityInformation  |= PROTECTED_SACL_SECURITY_INFORMATION;

   // Set the security
   ULONG lErr;
   if (m_pInfo->m_szName[0] != 0) // Is it named
      lErr = SetNamedSecurityInfo(m_pInfo->m_szName, 
             m_pInfo->m_pEntry->m_objType, SecurityInformation, psidOwner, 
             psidGroup, pDACL, pSACL);
   else // Is it a handle case
      lErr = SetSecurityInfo(m_pInfo->m_hHandle, m_pInfo->m_pEntry->m_objType,
            SecurityInformation, psidOwner, psidGroup, pDACL, pSACL);
   
   // Report error
   if (lErr != ERROR_SUCCESS)
      ReportError(TEXT("GetNamedSecurityInfo"), lErr);
   else
      hr = S_OK;

   return(hr);
}


///////////////////////////////////////////////////////////////////////////////


void HandleEdit(HWND hwnd) {
   
   // Maintains information about the object whose security we are editing
   ObjInf info = { 0 };

   // Fill the info structure with info from the UI
   if (FillInfo(hwnd, &info)) {

      // Create instance of class derived from interface ISecurityInformation 
      CSecurityInformation* pSec = new CSecurityInformation(&info, 
            IsDlgButtonChecked(hwnd, IDC_BINARY) == BST_CHECKED);

      // Common dialog box for ACL editing
      EditSecurity(hwnd, pSec);
      if (pSec != NULL)
            pSec->Release();

      // Cleanup if we had opened a handle before
      if (info.m_szName[0] == 0) {

         switch (info.m_pEntry->m_nSpecificType) {

            case AM_FILE:
            case AM_PROCESS:
            case AM_THREAD:
            case AM_JOB:
            case AM_SEMAPHORE:
            case AM_EVENT:
            case AM_MUTEX:
            case AM_MAPPING:
            case AM_TIMER:
            case AM_TOKEN:
            case AM_NAMEDPIPE:
            case AM_ANONPIPE:
               CloseHandle(info.m_hHandle);
               break;
            
            case AM_REGISTRY:
               RegCloseKey((HKEY) info.m_hHandle);
               break;
            
            case AM_WINDOWSTATION:
               CloseWindowStation((HWINSTA) info.m_hHandle);
               break;
            
            case AM_DESKTOP:
               CloseDesktop((HDESK) info.m_hHandle);
               break;
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   switch (id) {

      case IDCANCEL:
         EndDialog(hwnd, 0);
         break;

      case IDC_TYPE:
         HandleType(hwnd, codeNotify, hwndCtl);
         break;

      case IDE_PID:
      case IDE_HANDLE:
         HandleRadio(hwnd, codeNotify, IDR_HANDLE);
         break;
      
      case IDE_NAME:
         HandleRadio(hwnd, codeNotify, IDR_NAME);
         break;
      
      case IDB_EDIT:
         HandleEdit(hwnd);
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {

      chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
      chHANDLE_DLGMSG(hwnd, WM_COMMAND, Dlg_OnCommand);
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_DIALOG), NULL, Dlg_Proc);
   return(0);
}


///////////////////////////////// End of File /////////////////////////////////