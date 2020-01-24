/******************************************************************************
Module:  TokenMaster.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"           /* See Appendix A. */

#include <WindowsX.h>
#include <AclAPI.h>
#include <AclUI.h>
#include <ObjBase.h>
#include <ObjSel.h>
#include <TlHelp32.h>

#include "Resource.h"

#define UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"      // See Appendix B.

#define AUTOBUF_IMPL
#include "..\ClassLib\AutoBuf.h"       // See Appendix B.

#define PRINTBUF_IMPL
#include "..\ClassLib\PrintBuf.h"      // See Appendix B.

#define SECINFO_IMPL
#include "..\ClassLib\SecInfo.h"      // See Appendix B.


HANDLE g_hSnapShot = NULL;
HANDLE g_hToken = NULL;

HWND g_hwndProcessCombo;
HWND g_hwndThreadCombo;
HWND g_hwndToken;
HWND g_hwndStatus;
HWND g_hwndEnablePrivileges;
HWND g_hwndEnableGroups;
HWND g_hwndDeletedPrivileges;
HWND g_hwndDisabledSids;
HWND g_hwndRestrictedSids;
HWND g_hwndLogonTypes;
HWND g_hwndLogonProviders;
HWND g_hwndImpersonationLevels;
HWND g_hwndTokenTypes;

HINSTANCE g_hInst;

CUILayout g_pResizer;


#define DIVIDERL  TEXT("****************************************") \
                  TEXT("****************************************\r\n")

#define DIVIDERS  TEXT("----------------------------------------") \
                  TEXT("----------------------------------------\r\n")


///////////////////////////////////////////////////////////////////////////////


class CSecurityPage: public CSecInfo {
public:
   CSecurityPage(PSECURITY_DESCRIPTOR pSD);
   ~CSecurityPage();

private:   
   PSECURITY_DESCRIPTOR m_pSD; 
   static SI_ACCESS m_siAccessAllRights[];

private:
   HRESULT WINAPI GetObjectInformation(PSI_OBJECT_INFO pObjectInfo);
   HRESULT WINAPI GetSecurity(SECURITY_INFORMATION RequestedInformation,
      PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault);
   STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
         PSECURITY_DESCRIPTOR pSecurityDescriptor);
   STDMETHOD(GetAccessRights) (const GUID* pguidObjectType, DWORD dwFlags,
         PSI_ACCESS* ppAccess, PULONG pcAccesses, PULONG piDefaultAccess);
   STDMETHOD(MapGeneric) (const GUID* pguidObjectType, PUCHAR pAceFlags,
         PACCESS_MASK pMask);
   HRESULT WINAPI GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, ULONG *pcInheritTypes);

   BOOL UseSDCopy(PSECURITY_DESCRIPTOR psd);
};


///////////////////////////////////////////////////////////////////////////////


BOOL EnablePrivilege(PTSTR szPriv, BOOL fEnabled) {

   HANDLE hToken   = NULL;
   BOOL   fSuccess = FALSE;

   try {{

      // First lookup the system unique luid for the privilege
      LUID luid;
      if (!LookupPrivilegeValue(NULL, szPriv, &luid)) {

         // If the name is bogus...
         goto leave;
      }

      // Then get the processes token
      if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
            &hToken)) {
         goto leave;
      }

      // Set up our token privileges "array" (in our case an array of one)
      TOKEN_PRIVILEGES tp;
      tp.PrivilegeCount             = 1;
      tp.Privileges[0].Luid         = luid;
      tp.Privileges[0].Attributes = fEnabled ? SE_PRIVILEGE_ENABLED : 0;

      // Adjust our token privileges by enabling or disabling this one
      if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
            NULL, NULL)) {
         goto leave;
      }

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   // Cleanup
   if (hToken != NULL)
      CloseHandle(hToken);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


HANDLE OpenSystemProcess() {

   HANDLE hSnapshot = NULL;
   HANDLE hProc     = NULL;

   try {{

      // Get a snapshot of the processes in the system
      hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
      if (hSnapshot == NULL)
         goto leave;

      PROCESSENTRY32 pe32;
      pe32.dwSize = sizeof(pe32);

      // Find the "System" process
      BOOL fProcess = Process32First(hSnapshot, &pe32);
      while (fProcess && (lstrcmpi(pe32.szExeFile, TEXT("SYSTEM")) != 0))
         fProcess = Process32Next(hSnapshot, &pe32);
      if (!fProcess)
         goto leave;    // Didn't find "System" process

      // Open the process with PROCESS_QUERY_INFORMATION access
      hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
            pe32.th32ProcessID);
      if (hProc == NULL)
         goto leave;

   } leave:;
   } catch(...) {}

   // Cleanup the snapshot
   if (hSnapshot != NULL)
      CloseHandle(hSnapshot);

   return(hProc);
}


///////////////////////////////////////////////////////////////////////////////


BOOL ModifySecurity(HANDLE hProc, DWORD dwAccess) {

   PACL pAcl        = NULL;
   PACL pNewAcl     = NULL;
   PACL pSacl       = NULL;
   PSID pSidOwner   = NULL;
   PSID pSidPrimary = NULL;
   BOOL fSuccess    = TRUE;

   PSECURITY_DESCRIPTOR pSD = NULL;

   try {{

      // Find the length of the security object for the kernel object
      DWORD dwSDLength;
      if (GetKernelObjectSecurity(hProc, DACL_SECURITY_INFORMATION, pSD, 0,
            &dwSDLength) || (GetLastError() !=
            ERROR_INSUFFICIENT_BUFFER))
         goto leave;

      // Allocate a buffer of that length
      pSD = LocalAlloc(LPTR, dwSDLength);
      if (pSD == NULL)
         goto leave;

      // Retrieve the kernel object
      if (!GetKernelObjectSecurity(hProc, DACL_SECURITY_INFORMATION, pSD,
            dwSDLength, &dwSDLength))
         goto leave;

      // Get a pointer to the DACL of the SD
      BOOL fDaclPresent;
      BOOL fDaclDefaulted;
      if (!GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pAcl,
            &fDaclDefaulted))
         goto leave;

      // Get the current user's name
      TCHAR szName[1024];
      DWORD dwLen = chDIMOF(szName);
      if (!GetUserName(szName, &dwLen))
         goto leave;

      // Build an EXPLICIT_ACCESS structure for the ace we wish to add.
      EXPLICIT_ACCESS ea;
      BuildExplicitAccessWithName(&ea, szName, dwAccess, GRANT_ACCESS, 0);
      ea.Trustee.TrusteeType = TRUSTEE_IS_USER;

      // We are allocating a new ACL with a new ace inserted.  The new
      // ACL must be LocalFree'd
      if (ERROR_SUCCESS != SetEntriesInAcl(1, &ea, pAcl,
            &pNewAcl)) {
         pNewAcl = NULL;
         goto leave;
      }

      // Find the buffer sizes we would need to make our SD absolute
      pAcl               = NULL;
      dwSDLength         = 0;
      DWORD dwAclSize    = 0;
      DWORD dwSaclSize   = 0;
      DWORD dwSidOwnLen  = 0;
      DWORD dwSidPrimLen = 0;
      PSECURITY_DESCRIPTOR pAbsSD = NULL;
      if (MakeAbsoluteSD(pSD, pAbsSD, &dwSDLength, pAcl, &dwAclSize, pSacl,
            &dwSaclSize, pSidOwner, &dwSidOwnLen, pSidPrimary, &dwSidPrimLen)
            || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
         goto leave;

      // Allocate the buffers
      pAcl = (PACL) LocalAlloc(LPTR, dwAclSize);
      pSacl = (PACL) LocalAlloc(LPTR, dwSaclSize);
      pSidOwner = (PSID) LocalAlloc(LPTR, dwSidOwnLen);
      pSidPrimary = (PSID) LocalAlloc(LPTR, dwSidPrimLen);
      pAbsSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, dwSDLength);
      if (!(pAcl && pSacl && pSidOwner && pSidPrimary && pAbsSD))
         goto leave;

      // And actually make our SD absolute
      if (!MakeAbsoluteSD(pSD, pAbsSD, &dwSDLength, pAcl, &dwAclSize, pSacl,
            &dwSaclSize, pSidOwner, &dwSidOwnLen, pSidPrimary, &dwSidPrimLen))
         goto leave;

      // Now set the security descriptor DACL
      if (!SetSecurityDescriptorDacl(pAbsSD, fDaclPresent, pNewAcl,
            fDaclDefaulted))
         goto leave;

      // And set the security for the object
      if (!SetKernelObjectSecurity(hProc, DACL_SECURITY_INFORMATION, pAbsSD))
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   // Cleanup
   if (pNewAcl == NULL)
      LocalFree(pNewAcl);

   if (pSD == NULL)
      LocalFree(pSD);

   if (pAcl == NULL)
      LocalFree(pAcl);

   if (pSacl == NULL)
      LocalFree(pSacl);

   if (pSidOwner == NULL)
      LocalFree(pSidOwner);

   if (pSidPrimary == NULL)
      LocalFree(pSidPrimary);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


HANDLE GetLSAToken() {

   HANDLE hProc  = NULL;
   HANDLE hToken = NULL;

   try {{

      // Enable the SE_DEBUG_NAME privilege in our process token
      if (!EnablePrivilege(SE_DEBUG_NAME, TRUE))
         goto leave;

      // Retrieve a handle to the "System" process
      hProc = OpenSystemProcess();
      if (hProc == NULL)
         goto leave;

      // Open the process token with READ_CONTROL and WRITE_DAC access.  We
      // will use this access to modify the security of the token so that we
      // retrieve it again with a more complete set of rights.
      BOOL fResult = OpenProcessToken(hProc, READ_CONTROL | WRITE_DAC,
            &hToken);
      if (FALSE == fResult)  {
         hToken = NULL;
         goto leave;
      }

      // Add an ace for the current user for the token.  This ace will add
      // TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY rights.
      if (!ModifySecurity(hToken, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY
            | TOKEN_QUERY)) {
         CloseHandle(hToken);
         hToken = NULL;
         goto leave;
      }

      // Reopen the process token now that we have added the rights to
      // query the token, duplicate it, and assign it.
      fResult = OpenProcessToken(hProc, TOKEN_QUERY | TOKEN_DUPLICATE
            | TOKEN_ASSIGN_PRIMARY, &hToken);
      if (FALSE == fResult)  {
         hToken = NULL;
         goto leave;
      }

   } leave:;
   } catch(...) {}

   // Close the System process handle
   if (hProc != NULL)
      CloseHandle(hProc);

   return(hToken);
}


///////////////////////////////////////////////////////////////////////////////


BOOL RunAsUser(PTSTR pszEXE, PTSTR pszUserName, PTSTR pszPassword,
      PTSTR pszDesktop) {

   HANDLE hToken   = NULL;
   BOOL   fProcess = FALSE;
   BOOL   fSuccess = FALSE;

   PROCESS_INFORMATION pi = {NULL, NULL, 0, 0};

   try {{

      if (pszUserName == NULL) {

         hToken = GetLSAToken();
         if (hToken == NULL)
            goto leave;

      } else {

         if (!LogonUser(pszUserName, NULL, pszPassword,
               LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken))
            goto leave;
      }

      STARTUPINFO si;
      si.cb          = sizeof(si);
      si.lpDesktop   = pszDesktop;
      si.lpTitle     = NULL;
      si.dwFlags     = 0;
      si.cbReserved2 = 0;
      si.lpReserved  = NULL;
      si.lpReserved2 = NULL;

      fProcess = CreateProcessAsUser(hToken, NULL, pszEXE, NULL, NULL, FALSE,
            0, NULL, NULL, &si, &pi);
      if (!fProcess)
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   if (hToken != NULL)
      CloseHandle(hToken);

   if (fProcess) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
   }

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL TryRelaunch() {

   BOOL fSuccess = FALSE;

   try {{

      TCHAR szFilename[MAX_PATH + 50];
      if (!GetModuleFileName(NULL, szFilename, chDIMOF(szFilename)))
         goto leave;

      fSuccess = RunAsUser(szFilename, NULL, NULL, TEXT("Winsta0\\Default"));

   } leave:;
   } catch(...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL GetUserSID(PSID psid, BOOL fAllowImpersonate, PDWORD pdwSize) {

   BOOL   fSuccess = FALSE;
   HANDLE hToken   = NULL;

   try {{

      if (fAllowImpersonate)
         if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
            hToken = NULL;

      if (hToken == NULL)
         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            goto leave;

      DWORD dwSize;
      if (!GetTokenInformation(hToken, TokenUser, psid, *pdwSize, &dwSize))
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   if (hToken != NULL)
      CloseHandle(hToken);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


void SetDlgDefaultButton(HWND hwndDlg, UINT nDefault) {

   // Get that last default control
   UINT nOld = SendMessage(hwndDlg, DM_GETDEFID, 0, 0);

   // Reset the current default push button to a regular button.
   if (HIWORD(nOld) == DC_HASDEFID)
      SendDlgItemMessage(hwndDlg, LOWORD(nOld), BM_SETSTYLE, BS_PUSHBUTTON,
            (LPARAM) TRUE);

   // Update the default push button's control ID.
   SendMessage(hwndDlg, DM_SETDEFID, nDefault, 0L);

   // Set the new style.
   SendDlgItemMessage(hwndDlg, nDefault, BM_SETSTYLE, BS_DEFPUSHBUTTON,
         (LPARAM) TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void SmartDefaultButton(HWND hwndDlg, int id) {

   int nCtrlMap[][2] = {
      { IDC_PROCESSES,          IDB_DUMPTOKEN },
      { IDC_THREADS,            IDB_DUMPTOKEN },
      { IDC_IMPERSONATIONLEVEL, IDB_DUPLICATE },
      { IDC_TOKENTYPE,          IDB_DUPLICATE },
      { IDE_USERNAME,           IDB_LOGONUSER },
      { IDE_PASSWORD,           IDB_LOGONUSER },
      { IDC_LOGONTYPE,          IDB_LOGONUSER },
      { IDC_LOGONPROVIDER,      IDB_LOGONUSER },
      { IDE_FILENAME,           IDB_CREATEPROCESS },
      { -1,                     -1 }
   };

   int nIndex = 0;
   while (nCtrlMap[nIndex][0] != nCtrlMap[nIndex][1]) {
      if (nCtrlMap[nIndex][0] == id) {
         SetDlgDefaultButton(hwndDlg, nCtrlMap[nIndex][1]);
         break;
      }
      nIndex++;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Status(PTSTR szStatus, DWORD dwLastError) {

   CPrintBuf* pbufStatus = NULL;

   try {{

      if (szStatus == NULL)
         goto leave;

      // Clear the text
      SetWindowText(g_hwndStatus, TEXT(""));

      // Using the CPrintBuf class to buffer printing to the window
      pbufStatus = new CPrintBuf;
      if (pbufStatus == NULL)
         goto leave;

      pbufStatus->Print(TEXT("Token Master, Status - "));
      pbufStatus->Print(szStatus);

      if (dwLastError != 0)   {
         pbufStatus->Print(TEXT(": "));
         pbufStatus->PrintError(dwLastError);
      }

   } leave:;
   } catch(...) {}

   if (pbufStatus != NULL) {
      SetWindowText(g_hwndStatus, *pbufStatus);
      delete pbufStatus;
   }
}


///////////////////////////////////////////////////////////////////////////////


PVOID AllocateTokenInfo(HANDLE hToken, TOKEN_INFORMATION_CLASS tokenClass) {

   PVOID pvBuffer  = NULL;
   PTSTR pszStatus = NULL;
   DWORD dwStatus;

   try {{

      // Initial buffer size
      DWORD dwSize   = 0;
      BOOL  fSuccess = FALSE;
      do {

         // Do we have a size now?
         if (dwSize != 0) {

            // If we already have a buffer, free it
            if (pvBuffer != NULL)
               LocalFree(pvBuffer);

            // Allocate a new buffer
            pvBuffer = LocalAlloc(LPTR, dwSize);
            if (pvBuffer == NULL) {
               pszStatus = TEXT("LocalAlloc");
               goto leave;
            }
         }

         // Try again
         fSuccess = GetTokenInformation(hToken, tokenClass, pvBuffer, dwSize,
               &dwSize);
      // while it is failing on ERROR_INSUFFICIENT_BUFFER
      } while (!fSuccess && (GetLastError() == ERROR_INSUFFICIENT_BUFFER));

      // If we failed for some other reason then back out
      if (!fSuccess) {
         dwStatus = GetLastError();
         pszStatus = TEXT("GetTokenInformation");
         if (pvBuffer != NULL) {
            LocalFree(pvBuffer);
            pvBuffer = NULL;
         }
         SetLastError(dwStatus);
         goto leave;
      }

      SetLastError(0);

   } leave:;
   } catch(...) {}

   dwStatus = GetLastError();
   Status(pszStatus, dwStatus);
   SetLastError(dwStatus);

   // Return locally allocated buffer
   return(pvBuffer);
}


///////////////////////////////////////////////////////////////////////////////


BOOL GetTextualSid(PSID pSid, PTSTR TextualSid, PDWORD pdwBufferLen) {

   BOOL fSuccess = FALSE;

   try {{

      // Test if Sid passed in is valid
      if (!IsValidSid(pSid))
         goto leave;

      // Obtain Sid identifier authority
      PSID_IDENTIFIER_AUTHORITY psia = GetSidIdentifierAuthority(pSid);

      // Obtain sid subauthority count
      DWORD dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

      // Compute buffer length
      // S-SID_REVISION-+identifierauthority-+subauthorities-+NULL
      DWORD dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1)
            * sizeof(TCHAR);

      // Check provided buffer length.
      // If not large enough, indicate proper size and SetLastError
      if (*pdwBufferLen < dwSidSize) {
         *pdwBufferLen = dwSidSize;
         SetLastError(ERROR_INSUFFICIENT_BUFFER);
         goto leave;
      }

      // prepare S-SID_REVISION-
      dwSidSize = wsprintf(TextualSid, TEXT("S-%lu-"), SID_REVISION);

      // Prepare Sid identifier authority
      if ((psia->Value[0] != 0) || (psia->Value[1] != 0)) {

         dwSidSize += wsprintf(TextualSid + lstrlen(TextualSid),
               TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
               (USHORT) psia->Value[0], (USHORT) psia->Value[1],
               (USHORT) psia->Value[2], (USHORT) psia->Value[3],
               (USHORT) psia->Value[4], (USHORT) psia->Value[5]);

      } else {

         dwSidSize += wsprintf(TextualSid + lstrlen(TextualSid),
               TEXT("%lu"), (ULONG) (psia->Value[5])
               + (ULONG) (psia->Value[4] << 8)
               + (ULONG) (psia->Value[3] << 16)
               + (ULONG) (psia->Value[2] << 24));
      }

      // Loop through sid subauthorities
      DWORD dwCounter;
      for (dwCounter = 0; dwCounter < dwSubAuthorities; dwCounter++) {
         dwSidSize += wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
               *GetSidSubAuthority(pSid, dwCounter));
      }

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


void UpdatePrivileges() {

   PTOKEN_PRIVILEGES ptpPrivileges = NULL;

   PTSTR pszName     = NULL;
   PTSTR pszDispName = NULL;

   // Clear out both list boxes
   SendMessage(g_hwndEnablePrivileges, LB_RESETCONTENT , 0, 0);
   SendMessage(g_hwndDeletedPrivileges, LB_RESETCONTENT , 0, 0);

   try {{

      // No token?  Then we fly
      if (g_hToken == NULL)
         goto leave;

      // Get that token-privilege information
      ptpPrivileges = (PTOKEN_PRIVILEGES) AllocateTokenInfo(g_hToken,
            TokenPrivileges);
      if (ptpPrivileges == NULL)
         goto leave;

      // Iterate through the privileges
      DWORD dwIndex;
      for (dwIndex = 0; dwIndex < ptpPrivileges->PrivilegeCount; dwIndex++) {

         // Get size of the name
         DWORD dwSize = 0;
         LookupPrivilegeName(NULL, &(ptpPrivileges->Privileges[dwIndex].Luid),
               pszName, &dwSize);
         pszName = (PTSTR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));
         if (pszName == NULL)
            goto leave;

         // Get the name itself
         if (!LookupPrivilegeName(NULL,
               &(ptpPrivileges->Privileges[dwIndex].Luid), pszName, &dwSize))
            goto leave;

         // Get the diaplay name size
         DWORD dwDispSize = 0;
         DWORD dwLangID;
         LookupPrivilegeDisplayName(NULL, pszName, NULL, &dwDispSize,
               &dwLangID);
         pszDispName = (PTSTR) LocalAlloc(LPTR, (dwDispSize + dwSize + 3)
               * sizeof(TCHAR));
         if (pszDispName == NULL)
            goto leave;

         // Create the composite string
         lstrcpy(pszDispName, pszName);
         lstrcat(pszDispName, TEXT("--"));

         // Get the display name
         if (!LookupPrivilegeDisplayName(NULL, pszName, pszDispName
               + lstrlen(pszDispName), &dwDispSize, &dwLangID))
            goto leave;

         // Add the string to the enable/disable privilege list
         DWORD dwItem = SendMessage(g_hwndEnablePrivileges, LB_ADDSTRING, 0,
               (LPARAM) pszDispName);

         // Now the item data
         SendMessage(g_hwndEnablePrivileges, LB_SETITEMDATA, dwItem,
               (LPARAM) dwIndex);

         // If it is enabled then enable it in the list box
         if (ptpPrivileges->Privileges[dwIndex].Attributes
               & SE_PRIVILEGE_ENABLED)
            SendMessage(g_hwndEnablePrivileges, LB_SETSEL, TRUE, dwItem);

         // Now the deleted privileges
         dwItem = SendMessage(g_hwndDeletedPrivileges, LB_ADDSTRING, 0,
               (LPARAM) pszName);
         SendMessage(g_hwndDeletedPrivileges, LB_SETITEMDATA, dwItem,
               (LPARAM) dwIndex);

         // Free up our recurring buffers
         LocalFree(pszName);
         pszName = NULL;

         LocalFree(pszDispName);
         pszDispName = NULL;
      }

   } leave:;
   } catch(...) {}

   // Cleanup
   if (ptpPrivileges != NULL)
      LocalFree(ptpPrivileges);

   if (pszName != NULL)
      LocalFree(pszName);

   if (pszDispName != NULL)
      LocalFree(pszDispName);
}


///////////////////////////////////////////////////////////////////////////////


void UpdateGroups() {

   PTOKEN_GROUPS ptgGroups = NULL;

   // Clear out both list boxes
   SendMessage(g_hwndEnableGroups, LB_RESETCONTENT , 0, 0);
   SendMessage(g_hwndDisabledSids, LB_RESETCONTENT , 0, 0);

   try {{

      if (g_hToken == NULL)
         goto leave;

      // Get that group information
      ptgGroups = (PTOKEN_GROUPS) AllocateTokenInfo(g_hToken, TokenGroups);
      if (ptgGroups == NULL)
         goto leave;

      // Iterate through them
      DWORD dwIndex;
      for (dwIndex = 0; dwIndex < ptgGroups->GroupCount; dwIndex++) {

         DWORD dwSize = 0;
         TCHAR szDomName[255] = {TEXT("")};
         DWORD dwSizeDom = chDIMOF(szDomName);

         // Get the text name size
         SID_NAME_USE sNameUse;
         LookupAccountSid(NULL, (ptgGroups->Groups[dwIndex].Sid), NULL,
               &dwSize, szDomName, &dwSizeDom, &sNameUse);
         PTSTR pszName = (PTSTR) LocalAlloc(LPTR, dwSize * sizeof(TCHAR));
         if (pszName == NULL)
            goto leave;

         // Get the name
         TCHAR szCompositeName[1024];
         if (LookupAccountSid(NULL, (ptgGroups->Groups[dwIndex].Sid), pszName,
               &dwSize, szDomName, &dwSizeDom, &sNameUse)) {

            // Make the composite string
            lstrcpy(szCompositeName, szDomName);
            lstrcat(szCompositeName, TEXT("\\"));
            lstrcat(szCompositeName, pszName);

            // If it is neither mandatory nor the logon ID then add it to
            // the enable/disable list box
            DWORD dwItem;
            if (!((ptgGroups->Groups[dwIndex].Attributes & SE_GROUP_MANDATORY)
                  || (ptgGroups->Groups[dwIndex].Attributes
                  & SE_GROUP_LOGON_ID))) {

               // Add the string to the list box
               dwItem = SendMessage(g_hwndEnableGroups, LB_ADDSTRING, 0,
                     (LPARAM) szCompositeName);
               SendMessage(g_hwndEnableGroups, LB_SETITEMDATA, dwItem,
                     (LPARAM) dwIndex);
               if (ptgGroups->Groups[dwIndex].Attributes & SE_GROUP_ENABLED)
                  SendMessage(g_hwndEnableGroups, LB_SETSEL, TRUE, dwItem);
            }

            // Add to the CreateRestrictedToken list
            dwItem = SendMessage(g_hwndDisabledSids, LB_ADDSTRING, 0,
                  (LPARAM) szCompositeName);
            SendMessage(g_hwndDisabledSids, LB_SETITEMDATA, dwItem,
                  (LPARAM) dwIndex);
         }
         LocalFree(pszName);
      }

   } leave:;
   } catch(...) {}

   // Cleanup
   if (ptgGroups != NULL)
      LocalFree(ptgGroups);
}


///////////////////////////////////////////////////////////////////////////////


void DumpSIDAttributes(DWORD dwAttrib, CPrintBuf* pbufToken) {

   pbufToken->Print(TEXT("SID Attribs:\r\n"));

   if (dwAttrib & SE_GROUP_MANDATORY)
      pbufToken->Print(TEXT("\t\tSE_GROUP_MANDATORY\r\n"));

   if (dwAttrib & SE_GROUP_ENABLED_BY_DEFAULT)
      pbufToken->Print(TEXT("\t\tSE_GROUP_ENABLED_BY_DEFAULT\r\n"));

   if (dwAttrib & SE_GROUP_ENABLED)
      pbufToken->Print(TEXT("\t\tSE_GROUP_ENABLED\r\n"));

   if (dwAttrib & SE_GROUP_OWNER)
      pbufToken->Print(TEXT("\t\tSE_GROUP_OWNER\r\n"));

   if (dwAttrib & SE_GROUP_USE_FOR_DENY_ONLY)
      pbufToken->Print(TEXT("\t\tSE_GROUP_USE_FOR_DENY_ONLY\r\n"));

   if (dwAttrib & SE_GROUP_LOGON_ID)
      pbufToken->Print(TEXT("\t\tSE_GROUP_LOGON_ID\r\n"));
}


///////////////////////////////////////////////////////////////////////////////


void DumpSID(PSID psid, CPrintBuf* pbufToken) {

   PTSTR szSIDText = NULL;

   try {{

      SID_NAME_USE snuUse;
      PTSTR pszUse;
      TCHAR szName[255]    = {TEXT("[Logon SID]")};
      DWORD dwSize         = chDIMOF(szName);
      TCHAR szDomName[255] = {TEXT("")};
      DWORD dwDomSize      = chDIMOF(szDomName);

      // What is the SID type
      if (LookupAccountSid(NULL, psid, szName, &dwSize, szDomName, &dwDomSize,
            &snuUse)) {

         switch (snuUse) {

            case SidTypeUser:
               pszUse = TEXT("User SID");
               break;

            case SidTypeGroup:
               pszUse = TEXT("Group SID");
               break;

            case SidTypeDomain:
               pszUse = TEXT("Domain SID");
               break;

            case SidTypeAlias:
               pszUse = TEXT("Alias SID");
               break;

            case SidTypeWellKnownGroup:
               pszUse = TEXT("Well-Known Group SID");
               break;

            case SidTypeDeletedAccount:
               pszUse = TEXT("Deleted Account");
               break;

            case SidTypeInvalid:
               pszUse = TEXT("Invalid SID");
               break;

            default:
               pszUse = TEXT("Unknown SID");
               break;
         }

      } else {

         // We couldn't look it up, maybe it is the logon SID
         PSID_IDENTIFIER_AUTHORITY psia = GetSidIdentifierAuthority(psid);
         DWORD dwFirstSub = *GetSidSubAuthority(psid, 0);
         if (dwFirstSub == 5 && psia->Value[6] == 5)
            pszUse = TEXT("Logon SID");
         else
            pszUse = TEXT("");
      }

      // Get the textual SID
      dwSize = 0;
      GetTextualSid(psid, szSIDText, &dwSize);
      szSIDText = (PTSTR) LocalAlloc(LPTR, dwSize);
      if (szSIDText == NULL)
         goto leave;
      if (!GetTextualSid(psid, szSIDText, &dwSize))
         goto leave;

      // Output the things we have learned
      pbufToken->Print(TEXT("SID:\t\t"));
      pbufToken->Print(szSIDText);

      pbufToken->Print(TEXT("\r\nUse:\t\t"));
      pbufToken->Print(pszUse);

      pbufToken->Print(TEXT("\r\nName:\t\t"));
      pbufToken->Print(szName);

      pbufToken->Print(TEXT("\r\nDomain Name:\t"));
      pbufToken->Print(szDomName);

      pbufToken->Print(TEXT("\r\n\r\n"));

   } leave:;
   } catch(...) {}

   if (szSIDText != NULL)
      LocalFree(szSIDText);
}


///////////////////////////////////////////////////////////////////////////////


BOOL DumpTokenGroups(HANDLE hToken, CPrintBuf* pbufToken) {

   BOOL          fSuccess  = FALSE;
   PTOKEN_GROUPS ptgGroups = NULL;

   try {{

      // Get token information
      ptgGroups = (PTOKEN_GROUPS) AllocateTokenInfo(hToken, TokenGroups);
      if (ptgGroups == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("GROUP SIDS\r\n"));
      pbufToken->Print(TEXT("Group Count:\t%lu"), ptgGroups->GroupCount);
      pbufToken->Print(TEXT("\r\n") DIVIDERL);

      // Dump the SID and Attributes for each group
      DWORD dwIndex = ptgGroups->GroupCount;
      for (dwIndex = 0; dwIndex < ptgGroups->GroupCount; dwIndex++) {
         pbufToken->Print(TEXT("Sid #%d\r\n"), dwIndex);
         DumpSID(ptgGroups->Groups[dwIndex].Sid, pbufToken);
         DumpSIDAttributes(ptgGroups->Groups[dwIndex].Attributes, pbufToken);
         pbufToken->Print(DIVIDERS);
      }

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   if (ptgGroups != NULL)
      LocalFree(ptgGroups);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL DumpTokenUser(HANDLE hToken, CPrintBuf* pbufToken) {

   BOOL        fSuccess = FALSE;
   PTOKEN_USER ptuUser  = NULL;

   try {{

      // Get the token information
      ptuUser = (PTOKEN_USER) AllocateTokenInfo(hToken, TokenUser);
      if (ptuUser == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("USER SID\r\n") DIVIDERL
            TEXT("\r\n"));
      DumpSID(ptuUser->User.Sid, pbufToken);

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   if (ptuUser != NULL)
      LocalFree(ptuUser);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenPrivileges(HANDLE hToken, CPrintBuf* pbufToken) {

   PTOKEN_PRIVILEGES ptpPrivileges = NULL;

   try {{

      // Get the token privilege information
      ptpPrivileges = (PTOKEN_PRIVILEGES)
         AllocateTokenInfo(hToken, TokenPrivileges);
      if (ptpPrivileges == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("Priveleges\r\n"));
      pbufToken->Print(DIVIDERL);

      DWORD dwIndex;
      for (dwIndex = 0; dwIndex < ptpPrivileges->PrivilegeCount; dwIndex++) {

         // Get the privilege name and print it with attribute information
         TCHAR szName[255];
         DWORD dwSize = chDIMOF(szName);
         if (LookupPrivilegeName(NULL,
               &(ptpPrivileges->Privileges[dwIndex].Luid), szName, &dwSize)) {

            pbufToken->Print(szName);
            if (ptpPrivileges->Privileges[dwIndex].Attributes
                  & SE_PRIVILEGE_ENABLED_BY_DEFAULT)
               pbufToken->Print(TEXT("\r\n\t")
                     TEXT("SE_PRIVILEGE_ENABLED_BY_DEFAULT"));

            if (ptpPrivileges->Privileges[dwIndex].Attributes
                  & SE_PRIVILEGE_ENABLED)
               pbufToken->Print(TEXT("\r\n\tSE_PRIVILEGE_ENABLED"));

            if (ptpPrivileges->Privileges[dwIndex].Attributes
                  & SE_PRIVILEGE_USED_FOR_ACCESS)
               pbufToken->Print(TEXT("\r\n\tSE_PRIVILEGE_USED_FOR_ACCESS"));

            pbufToken->Print(TEXT("\r\n"));
         }
      }

   } leave:;
   } catch(...) {}

   if (ptpPrivileges != NULL)
      LocalFree(ptpPrivileges);
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenOwner(HANDLE hToken, CPrintBuf* pbufToken) {

   PTOKEN_OWNER ptoOwner = NULL;

   try {{

      ptoOwner = (PTOKEN_OWNER) AllocateTokenInfo(hToken, TokenOwner);
      if (ptoOwner == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("Token Owner\r\n"));
      pbufToken->Print(DIVIDERL);
      DumpSID(ptoOwner->Owner, pbufToken);

   } leave:;
   } catch(...) {}

   if (ptoOwner != NULL)
      LocalFree(ptoOwner);
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenPrimaryGroup(HANDLE hToken, CPrintBuf* pbufToken) {

   PTOKEN_OWNER ptgGroup = NULL;

   try {{

      ptgGroup = (PTOKEN_OWNER) AllocateTokenInfo(hToken, TokenPrimaryGroup);
      if (ptgGroup == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("Token Primary Group\r\n"));
      pbufToken->Print(DIVIDERL);
      DumpSID(ptgGroup->Owner, pbufToken);

   } leave:;
   } catch(...) {}

   if (ptgGroup != NULL)
      LocalFree(ptgGroup);
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenDefaultDacl(HANDLE hToken, CPrintBuf* pbufToken) {

   PTOKEN_DEFAULT_DACL ptdDacl  = NULL;
   PEXPLICIT_ACCESS    pEntries = NULL;

   try {{

      // Get the DACL info
      ptdDacl = (PTOKEN_DEFAULT_DACL) AllocateTokenInfo(hToken,
            TokenDefaultDacl);
      if (ptdDacl == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("Token Default DACL\r\n"));
      pbufToken->Print(DIVIDERL);

      // Get ACL size information
      ACL_SIZE_INFORMATION sizeInfo;
      if (!GetAclInformation(ptdDacl->DefaultDacl, &sizeInfo, sizeof(sizeInfo),
            AclSizeInformation))
         goto leave;

      // Iterate through the aces
      DWORD dwCount = sizeInfo.AceCount;
      DWORD dwIndex;
      for (dwIndex = 0; dwIndex < dwCount; dwIndex++) {

         // Get the ACE by index
         PACCESS_ALLOWED_ACE paceAllowed;
         if (GetAce(ptdDacl->DefaultDacl, dwIndex, (PVOID*) &paceAllowed)) {

            // Output what type of ACE it is
            switch (paceAllowed->Header.AceFlags) {

               case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                  pbufToken->Print(TEXT("ACCESS ALLOWED OBJECT"));
                  break;

               case ACCESS_ALLOWED_ACE_TYPE:
                  pbufToken->Print(TEXT("ACCESS ALLOWED"));
                  break;

               case ACCESS_DENIED_ACE_TYPE:
                  pbufToken->Print(TEXT("ACCESS DENIED"));
                  break;

               case ACCESS_DENIED_OBJECT_ACE_TYPE:
                  pbufToken->Print(TEXT("ACCESS DENIED OBJECT"));
                  break;
            }

            // Get the account name associated with the ace and output it
            TCHAR szName[255];
            TCHAR szDomain[255];

            DWORD dwSize       = chDIMOF(szName);
            DWORD dwDomainSize = chDIMOF(szDomain);

            SID_NAME_USE snuUse;
            LookupAccountSid(NULL, &(paceAllowed->SidStart), szName, &dwSize,
                  szDomain, &dwDomainSize, &snuUse);

            TCHAR szAccount[255];
            lstrcpy(szAccount, szDomain);
            lstrcat(szAccount, TEXT("/"));
            lstrcat(szAccount, szName);

            pbufToken->Print(TEXT("\t"));
            pbufToken->Print(szAccount);

            // Now we output the access mask in binary
            pbufToken->Print(TEXT("\r\nACCESS_MASK:  "));
            DWORD dwMask = paceAllowed->Mask;
            DWORD dwIndex2;
            for (dwIndex2 = 0; dwIndex2 < 32; dwIndex2++) {
               if (dwMask&(1 << 31))
                  pbufToken->Print(TEXT("1"));
               else
                  pbufToken->Print(TEXT("0"));
               dwMask = (dwMask << 1);
            }

            // Determine the bits that map to standard access rights and print
            // the flag name
            pbufToken->Print(TEXT("\r\n"));
            if (paceAllowed->Mask & (1 << 16))
               pbufToken->Print(TEXT("\tDELETE\r\n"));
            if (paceAllowed->Mask & (1 << 17))
               pbufToken->Print(TEXT("\tREAD_CONTROL\r\n"));
            if (paceAllowed->Mask & (1 << 18))
               pbufToken->Print(TEXT("\tWRITE_DAC\r\n"));
            if (paceAllowed->Mask & (1 << 19))
               pbufToken->Print(TEXT("\tWRITE_OWNER\r\n"));
            if (paceAllowed->Mask & (1 << 20))
               pbufToken->Print(TEXT("\tSYNCHRONIZE\r\n"));
            if (paceAllowed->Mask & (1 << 24))
               pbufToken->Print(TEXT("\tACCESS_SYSTEM_SECURITY\r\n"));
            if (paceAllowed->Mask & (1 << 25))
               pbufToken->Print(TEXT("\tMAXIMUM_ALLOWED\r\n"));
            if (paceAllowed->Mask & (1 << 28))
               pbufToken->Print(TEXT("\tGENERIC_ALL\r\n"));
            if (paceAllowed->Mask & (1 << 29))
               pbufToken->Print(TEXT("\tGENERIC_EXECUTE\r\n"));
            if (paceAllowed->Mask & (1 << 30))
               pbufToken->Print(TEXT("\tGENERIC_WRITE\r\n"));
            if (paceAllowed->Mask & (1 << 31))
               pbufToken->Print(TEXT("\tGENERIC_READ\r\n"));
            pbufToken->Print(TEXT("\r\n"));
         }
      }

      pbufToken->Print(TEXT("\r\n"));

   } leave:;
   } catch(...) {}

   if (ptdDacl != NULL)
      LocalFree(ptdDacl);

   if (pEntries != NULL)
      LocalFree(pEntries);
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenSource(HANDLE hToken, CPrintBuf* pbufToken) {

   PTOKEN_SOURCE ptsSource = NULL;

   try {{

      ptsSource = (PTOKEN_SOURCE) AllocateTokenInfo(hToken, TokenSource);
      if (ptsSource == NULL)
         goto leave;

      TCHAR szName[9];
      int nIndex = 8;
      while (nIndex-- != 0)
         szName[nIndex] = ptsSource->SourceName[nIndex];

      szName[8] = 0;
      pbufToken->Print(DIVIDERL TEXT("Token Source:  "));
      pbufToken->Print(szName);
      pbufToken->Print(TEXT("\r\n") DIVIDERL);

   } leave:;
   } catch(...) {}

   if (ptsSource != NULL)
      LocalFree(ptsSource);
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenImpersonationLevel(HANDLE hToken, CPrintBuf* pbufToken) {

   PSECURITY_IMPERSONATION_LEVEL psilImpersonation = NULL;

   try {{

      psilImpersonation = (PSECURITY_IMPERSONATION_LEVEL) AllocateTokenInfo(
            hToken, TokenImpersonationLevel);
      if (psilImpersonation == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("Token Impersonation Level:  "));

      switch (*psilImpersonation) {

         case SecurityAnonymous:
            pbufToken->Print(TEXT("Anonymous"));
            break;

         case SecurityIdentification:
            pbufToken->Print(TEXT("Identification"));
            break;

         case SecurityImpersonation:
            pbufToken->Print(TEXT("Impersonation"));
            break;

         case SecurityDelegation:
            pbufToken->Print(TEXT("Delegation"));
            break;
      }

      pbufToken->Print(TEXT("\r\n") DIVIDERL);

   } leave:;
   } catch(...) {}

   if (psilImpersonation != NULL)
      LocalFree(psilImpersonation );
}


///////////////////////////////////////////////////////////////////////////////


void DumpTokenType(HANDLE hToken, CPrintBuf* pbufToken) {

   PTOKEN_TYPE pttType = NULL;

   try {{

      pttType = (PTOKEN_TYPE) AllocateTokenInfo(hToken, TokenType);
      if (pttType == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL TEXT("Token Type:  "));

      BOOL fImpersonation = FALSE;
      switch (*pttType) {

         case TokenPrimary:
            pbufToken->Print(TEXT("Primary"));
            break;

         case TokenImpersonation:
            pbufToken->Print(TEXT("Impersonation"));
            fImpersonation = TRUE;
            break;
      }

      pbufToken->Print(TEXT("\r\n") DIVIDERL);

      if (fImpersonation)
         DumpTokenImpersonationLevel(hToken, pbufToken);

   } leave:;
   } catch(...) {}

   if (pttType != NULL)
      LocalFree(pttType);
}


///////////////////////////////////////////////////////////////////////////////


BOOL DumpTokenRestrictedSids(HANDLE hToken, CPrintBuf* pbufToken) {

   BOOL          fSuccess  = FALSE;
   PTOKEN_GROUPS ptgGroups = NULL;

   try {{

      BOOL fRestricted = IsTokenRestricted(hToken);
      ptgGroups = (PTOKEN_GROUPS) AllocateTokenInfo(hToken,
            TokenRestrictedSids);
      if (ptgGroups == NULL)
         goto leave;

      pbufToken->Print(DIVIDERL);
      pbufToken->Print(fRestricted ? TEXT("Token is Restricted\r\n") :
            TEXT("Token is not Restricted\r\n"));
      pbufToken->Print(TEXT("Restricted SID Count:\t%lu\r\n"),
            ptgGroups->GroupCount);
      pbufToken->Print(TEXT("Restricted SIDs\r\n"));
      pbufToken->Print(DIVIDERL);

      DWORD dwIndex = ptgGroups->GroupCount;
      for (dwIndex = 0; dwIndex < ptgGroups->GroupCount; dwIndex++) {
         pbufToken->Print(TEXT("Sid #%d\r\n"), dwIndex);
         DumpSIDAttributes(ptgGroups->Groups[dwIndex].Attributes,
               pbufToken);
         DumpSID(ptgGroups->Groups[dwIndex].Sid, pbufToken);
         pbufToken->Print(DIVIDERS);
      }

      fSuccess = TRUE;

   } leave:;
   } catch(...) {}

   if (ptgGroups != NULL)
      LocalFree(ptgGroups);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


void DumpToken() {

   CPrintBuf* pbufToken = NULL;

   try {{

      // No token?  Then slip out the back jack
      if (g_hToken == NULL)
         goto leave;

      // Update other controls
      UpdatePrivileges();
      UpdateGroups();
      SendMessage(g_hwndRestrictedSids, LB_RESETCONTENT, 0, 0);

      // Create a print buf object for buffering output to the edit control.
      // This class and usage instructions can be found in the "CmnCls.h"
      pbufToken = new CPrintBuf;
      if (pbufToken == NULL)
         goto leave;

      // Display the token user
      if (!DumpTokenUser(g_hToken, pbufToken))
         pbufToken->Print(TEXT("Unable to dump Token User\r\n"));

      // Display the token groups
      if (!DumpTokenGroups(g_hToken, pbufToken))
         pbufToken->Print(TEXT("Unable to dump Token Groups\r\n"));

      // Display the token privileges
      DumpTokenPrivileges(g_hToken, pbufToken);

      // Display the token owner
      DumpTokenOwner(g_hToken, pbufToken);

      // Display the token primary group
      DumpTokenPrimaryGroup(g_hToken, pbufToken);

      // Display the token default dacl
      DumpTokenDefaultDacl(g_hToken, pbufToken);

      // Display the token source
      DumpTokenSource(g_hToken, pbufToken);

      // Display the token type
      DumpTokenType(g_hToken, pbufToken);

      // Display the token restricted sids
      if (!DumpTokenRestrictedSids(g_hToken, pbufToken))
         pbufToken->Print(TEXT("Unable to dump Token Restricted Sids\r\n"));

   } leave:;
   } catch(...) {}

   // If we have a buffer object
   if (pbufToken != NULL) {

      // Then output its contents to our edit control
      SetWindowText(g_hwndToken, *pbufToken);

      // Then delete it
      delete pbufToken;
   }

}


///////////////////////////////////////////////////////////////////////////////


void RefreshSnapShot() {

   // If we already have one, close it
   if (g_hSnapShot != NULL)
      CloseHandle(g_hSnapShot);

   g_hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS
         | TH32CS_SNAPTHREAD, 0);
}


///////////////////////////////////////////////////////////////////////////////


void PopulateProcessCombo() {

   // Get the ID of our last selection so that the selection will "stick"
   LRESULT lItem = SendMessage(g_hwndProcessCombo, CB_GETCURSEL, 0, 0);
   DWORD dwLastID = SendMessage(g_hwndProcessCombo, CB_GETITEMDATA, lItem, 0);

   // Clear the combo box
   SendMessage(g_hwndProcessCombo, CB_RESETCONTENT, 0, 0);

   // No snapshot means we empty the combo
   if (g_hSnapShot != NULL) {

      // Iterate through the process list adding them to the combo
      PROCESSENTRY32 pentry;
      pentry.dwSize = sizeof(pentry);
      BOOL fIsProcess = Process32First(g_hSnapShot, &pentry);
      while (fIsProcess) {

         if (pentry.th32ProcessID != 0)
            lItem = SendMessage(g_hwndProcessCombo, CB_ADDSTRING, 0,
                  (LPARAM) pentry.szExeFile);
         else {

            // Special Case... The Idle Process has a zero ID
            lItem = SendMessage(g_hwndProcessCombo, CB_ADDSTRING, 0,
                  (LPARAM) TEXT("[System Idle Process]"));
            SendMessage(g_hwndProcessCombo, CB_SETCURSEL, lItem, 0);
         }

         // Set the item data to the processes ID
         SendMessage(g_hwndProcessCombo, CB_SETITEMDATA, lItem,
               pentry.th32ProcessID);

         // If the process ID matches the last one, we found it
         if (pentry.th32ProcessID == dwLastID)
            SendMessage(g_hwndProcessCombo, CB_SETCURSEL, lItem, 0);

         fIsProcess = Process32Next(g_hSnapShot, &pentry);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////


void PopulateThreadCombo() {

   // Get process id
   LRESULT lIndex = SendMessage(g_hwndProcessCombo, CB_GETCURSEL, 0, 0);
   DWORD   dwID   = SendMessage(g_hwndProcessCombo, CB_GETITEMDATA, lIndex, 0);

   // We want the selected thread to stick, if possible
   lIndex = SendMessage(g_hwndThreadCombo, CB_GETCURSEL, 0, 0);
   DWORD dwLastThreadID = SendMessage(g_hwndThreadCombo, CB_GETITEMDATA,
         lIndex, 0);

   SendMessage(g_hwndThreadCombo, CB_RESETCONTENT, 0, 0);

   // Add that "No Thread" option
   lIndex = SendMessage(g_hwndThreadCombo, CB_ADDSTRING, 0,
         (LPARAM) TEXT("[No Thread]"));
   SendMessage(g_hwndThreadCombo, CB_SETITEMDATA, lIndex, 0);
   SendMessage(g_hwndThreadCombo, CB_SETCURSEL, 0, 0);

   if (g_hSnapShot != NULL) {

      // Iterate through the threads adding them
      TCHAR szBuf[256];
      THREADENTRY32 entry;
      entry.dwSize = sizeof(entry);
      BOOL fIsThread = Thread32First(g_hSnapShot, &entry);
      while (fIsThread) {

         if (entry.th32OwnerProcessID == dwID) {

            wsprintf(szBuf, TEXT("ID = %d"), entry.th32ThreadID);
            lIndex = SendMessage(g_hwndThreadCombo, CB_ADDSTRING, 0,
                  (LPARAM) szBuf);
            SendMessage(g_hwndThreadCombo, CB_SETITEMDATA, lIndex,
                  entry.th32ThreadID);

            // Last thread selected?  If so reselect
            if (entry.th32ThreadID == dwLastThreadID)
               SendMessage(g_hwndThreadCombo, CB_SETCURSEL, lIndex, 0);
         }
         fIsThread = Thread32Next(g_hSnapShot, &entry);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////



void PopulateStaticCombos() {

   int nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Batch");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_BATCH);

   nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Network");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_NETWORK);

   nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Network Cleartext");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_NETWORK_CLEARTEXT);

   nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"New Credentials");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_NEW_CREDENTIALS);

   nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Service");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_SERVICE);

   nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Unlock");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_UNLOCK);

   nIndex = SendMessage(g_hwndLogonTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Interactive");
   SendMessage(g_hwndLogonTypes, CB_SETITEMDATA, nIndex,
         LOGON32_LOGON_INTERACTIVE);
   SendMessage(g_hwndLogonTypes, CB_SETCURSEL, nIndex, 0);


   nIndex = SendMessage(g_hwndLogonProviders, CB_ADDSTRING, 0,
         (LPARAM) L"Windows 2000");
   SendMessage(g_hwndLogonProviders, CB_SETITEMDATA, nIndex,
         LOGON32_PROVIDER_WINNT50);

   nIndex = SendMessage(g_hwndLogonProviders, CB_ADDSTRING, 0,
         (LPARAM) L"Windows NT 4.0");
   SendMessage(g_hwndLogonProviders, CB_SETITEMDATA, nIndex,
         LOGON32_PROVIDER_WINNT40);

   nIndex = SendMessage(g_hwndLogonProviders, CB_ADDSTRING, 0,
         (LPARAM) L"Windows NT 3.5");
   SendMessage(g_hwndLogonProviders, CB_SETITEMDATA, nIndex,
         LOGON32_PROVIDER_WINNT35);

   nIndex = SendMessage(g_hwndLogonProviders, CB_ADDSTRING, 0,
         (LPARAM) L"Default");
   SendMessage(g_hwndLogonProviders, CB_SETITEMDATA, nIndex,
         LOGON32_PROVIDER_DEFAULT);
   SendMessage(g_hwndLogonProviders, CB_SETCURSEL, nIndex, 0);

   nIndex = SendMessage(g_hwndImpersonationLevels, CB_ADDSTRING, 0,
         (LPARAM) L"SecurityAnonymous");
   SendMessage(g_hwndImpersonationLevels, CB_SETITEMDATA, nIndex,
         SecurityAnonymous);

   nIndex = SendMessage(g_hwndImpersonationLevels, CB_ADDSTRING, 0,
         (LPARAM) L"SecurityIdentification");
   SendMessage(g_hwndImpersonationLevels, CB_SETITEMDATA, nIndex,
         SecurityIdentification);

   nIndex = SendMessage(g_hwndImpersonationLevels, CB_ADDSTRING, 0,
         (LPARAM) L"SecurityDelegation");
   SendMessage(g_hwndImpersonationLevels, CB_SETITEMDATA, nIndex,
         SecurityDelegation);

   nIndex = SendMessage(g_hwndImpersonationLevels, CB_ADDSTRING, 0,
         (LPARAM) L"SecurityImpersonation");
   SendMessage(g_hwndImpersonationLevels, CB_SETITEMDATA, nIndex,
         SecurityImpersonation);
   SendMessage(g_hwndImpersonationLevels, CB_SETCURSEL, nIndex, 0);

   nIndex = SendMessage(g_hwndTokenTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Impersonation");
   SendMessage(g_hwndTokenTypes, CB_SETITEMDATA, nIndex,
         TokenImpersonation);

   nIndex = SendMessage(g_hwndTokenTypes, CB_ADDSTRING, 0,
         (LPARAM) L"Primary");
   SendMessage(g_hwndTokenTypes, CB_SETITEMDATA, nIndex, TokenPrimary);
   SendMessage(g_hwndTokenTypes, CB_SETCURSEL, nIndex, 0);
}


///////////////////////////////////////////////////////////////////////////////


void GetToken(HWND hwnd) {

   DWORD  dwStatus;
   HANDLE hThread    = NULL;
   HANDLE hProcess   = NULL;
   HANDLE hToken     = NULL;
   PTSTR  pszStatus  = TEXT("Dumped Process Token");

   PSECURITY_DESCRIPTOR pSD = NULL;

   if (g_hToken != NULL) {
      CloseHandle(g_hToken);
      g_hToken = NULL;
   }

   try {{

      // Find the process ID
      LRESULT lIndex = SendMessage(g_hwndProcessCombo, CB_GETCURSEL, 0, 0);
      DWORD dwProcessID = SendMessage(g_hwndProcessCombo, CB_GETITEMDATA,
            lIndex, 0);

      // Get the thread ID
      lIndex = SendMessage(g_hwndThreadCombo, CB_GETCURSEL, 0, 0);
      DWORD dwThreadID = SendMessage(g_hwndThreadCombo, CB_GETITEMDATA,
            lIndex, 0);

      // Open the thread if we can
      hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadID);

      // Thread?
      if (hThread != NULL) {

         // Get its token
         if (!OpenThreadToken(hThread, TOKEN_READ | TOKEN_QUERY_SOURCE, TRUE,
               &hToken)) {

            hToken = NULL;
            if (GetLastError() == ERROR_NO_TOKEN) {

               // Not a critical error, the thread doesn't have a token
               pszStatus = TEXT("Thread does not have a token, ")
                     TEXT(" dumping process token");
               SetLastError(0);

            } else {

               pszStatus = TEXT("OpenThreadToken");
               goto leave;
            }
         }
      }

      // So we don't have a token yet, lets get it from the process
      if (hToken == NULL) {

         // Get the handle to the process
         hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
         if (hProcess == NULL) {
            pszStatus = TEXT("OpenProcess");
            goto leave;
         }

         // Get the token (All access so we can change and launch things
         if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
            hToken = NULL;
            pszStatus = TEXT("OpenProcessToken");
            goto leave;
         }
      }

      // Get memory for an SD
      pSD = (PSECURITY_DESCRIPTOR) GlobalAlloc(GPTR,
            SECURITY_DESCRIPTOR_MIN_LENGTH);

      if (pSD == NULL) {
         pszStatus = TEXT("GlobalAlloc");
         goto leave;
      }

      // Initialize it
      if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
         pszStatus = TEXT("InitializeSecurityDescriptor");
         goto leave;
      }

      // Add a NULL DACL to the security descriptor..
      if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE)) {
         pszStatus = TEXT("SetSecurityDescriptorDacl");
         goto leave;
      }

      // We made the security descriptor just in case they want a duplicate.
      // We make the duplicate have all access to everyone.
      SECURITY_ATTRIBUTES sa;
      sa.nLength              = sizeof(sa);
      sa.lpSecurityDescriptor = pSD;
      sa.bInheritHandle       = TRUE;

      // If the user chooses not to copy the token, then changes made to it
      // will effect the owning process
      if (IDNO == MessageBox(hwnd, TEXT("Would you like to make a copy of ")
            TEXT("this process token?\n(Selecting \"No\" will cause the ")
            TEXT("\"AdjustToken\" and \"SetToken\"\nfeatures to affect the ")
            TEXT("owning process.) "), TEXT("Duplicate Token?"), MB_YESNO)) {
         g_hToken = hToken;

      }  else {

         // Duplicate the token
         if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, &sa,
               SecurityImpersonation, TokenPrimary, &g_hToken)) {
            g_hToken = NULL;
            pszStatus = TEXT("DuplicateTokenEx");
            goto leave;
         }
      }

      SetLastError(0);

   } leave:;
   } catch(...) {}

   // Status and Cleanup
   dwStatus = GetLastError();

   if (hToken && hToken != g_hToken)
      CloseHandle(hToken);

   if (hProcess != NULL)
      CloseHandle(hProcess);

   if (hThread != NULL)
      CloseHandle(hThread);

   if (pSD != NULL)
      GlobalFree(pSD);

   Status(pszStatus, dwStatus);
}


///////////////////////////////////////////////////////////////////////////////


BOOL GetAccountName(HWND hwnd, PTSTR szBuf, DWORD dwSize, BOOL fAllowGroups,
      BOOL fAllowUsers) {

   BOOL fSuccess      = FALSE;
   BOOL fGotStgMedium = FALSE;

   STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL, NULL};

   IDsObjectPicker* pdsObjectPicker = NULL;
   IDataObject*     pdoNames        = NULL;

   *szBuf = 0;

   try {{

      // Yes we are using COM
      HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
      if (FAILED(hr))
         goto leave;

      // Create an instance of the object picker.
      hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER,
            IID_IDsObjectPicker, (void**) &pdsObjectPicker);
      if (FAILED(hr))
         goto leave;

      // Initialize the DSOP_SCOPE_INIT_INFO array.
      DSOP_SCOPE_INIT_INFO dsopScopeInitInfo;
      ZeroMemory(&dsopScopeInitInfo, sizeof(dsopScopeInitInfo));

      // The scope in our case is the the current system + downlevel domains
      dsopScopeInitInfo.cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
      dsopScopeInitInfo.flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER
            | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN
            | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;

      // What are we selecting?
      dsopScopeInitInfo.FilterFlags.flDownlevel = 0;

      if (fAllowGroups)
         dsopScopeInitInfo.FilterFlags.flDownlevel |=
               DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS
               | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS
               | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;

      if (fAllowUsers)
         dsopScopeInitInfo.FilterFlags.flDownlevel |=
               DSOP_DOWNLEVEL_FILTER_USERS;

      // Initialize the DSOP_INIT_INFO structure.
      DSOP_INIT_INFO dsopInitInfo;
      ZeroMemory(&dsopInitInfo, sizeof(dsopInitInfo));
      dsopInitInfo.cbSize            = sizeof(dsopInitInfo);
      dsopInitInfo.pwzTargetComputer = NULL; // Target is the local computer
      dsopInitInfo.cDsScopeInfos     = 1;
      dsopInitInfo.aDsScopeInfos     = &dsopScopeInitInfo;
      dsopInitInfo.flOptions         = 0;

      // Initialize the object picker instance.
      hr = pdsObjectPicker->Initialize(&dsopInitInfo);
      if (FAILED(hr))
         goto leave;

      // Invoke the modal dialog where the user selects the user or group
      hr = pdsObjectPicker->InvokeDialog(hwnd, &pdoNames);
      if (FAILED(hr))
         goto leave;

      if (hr == S_OK) {

         // Get the global memory block containing the user's selections.
         FORMATETC formatetc = {(CLIPFORMAT)
               RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST), NULL,
               DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
         hr = pdoNames->GetData(&formatetc, &stgmedium);
         if (FAILED(hr))
            goto leave;
         fGotStgMedium = TRUE;

         // Retrieve pointer to DS_SELECTION_LIST structure.
         PDS_SELECTION_LIST pdsSelList = NULL;
         pdsSelList = (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);
         if (pdsSelList == NULL)
            goto leave;

         // We only allowed the selection of one, so our list is 1 or 0
         if (pdsSelList->cItems == 1) {

            // Copy the account name to the buffer passed in
            lstrcpynW(szBuf, pdsSelList->aDsSelection->pwzName, dwSize);
            fSuccess = TRUE;
         }

      }

   } leave:;
   } catch(...) {}

   if (fGotStgMedium)   {

      // Unlock that buffer
      GlobalUnlock(stgmedium.hGlobal);

      // Release the data
      ReleaseStgMedium(&stgmedium);
   }

   // Release the picker
   if (pdsObjectPicker != NULL)
      pdsObjectPicker->Release();

   // Release the data
   if (pdoNames != NULL)
      pdoNames->Release();

   // Done with COM for the moment
   CoUninitialize();

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_Processes(UINT codeNotify) {

   switch (codeNotify) {

      case CBN_DROPDOWN:
         RefreshSnapShot();
         PopulateProcessCombo();
         break;

      case CBN_SELCHANGE:
         PopulateThreadCombo();
         EnableWindow(g_hwndThreadCombo, SendMessage(g_hwndProcessCombo,
               CB_GETCURSEL, 0, 0));
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_Threads(HWND hwnd, UINT codeNotify) {

   int nSel = SendMessage(g_hwndThreadCombo, CB_GETCURSEL, 0, 0);
   if (nSel == 0)
      SetDlgItemText(hwnd, IDB_DUMPTOKEN, TEXT("OpenProcessToken"));
   else
      SetDlgItemText(hwnd, IDB_DUMPTOKEN, TEXT("OpenThreadToken"));
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_LogonUser(HWND hwnd) {

   // Do we have a token?  If so, kill it.
   if (g_hToken != NULL) {
      CloseHandle(g_hToken);
      g_hToken = NULL;
   }

   // Get the credentials
   TCHAR szName[1024];
   GetDlgItemText(hwnd, IDE_USERNAME, szName, chDIMOF(szName));

   TCHAR szPassword[1024];
   GetDlgItemText(hwnd, IDE_PASSWORD, szPassword, chDIMOF(szPassword));

   int nLogonType = SendMessage(g_hwndLogonTypes, CB_GETCURSEL, 0, 0);
   nLogonType = SendMessage(g_hwndLogonTypes, CB_GETITEMDATA, nLogonType, 0);

   int nLogonProvider = SendMessage(g_hwndLogonProviders, CB_GETCURSEL, 0, 0);
   nLogonProvider = SendMessage(g_hwndLogonProviders, CB_GETITEMDATA,
         nLogonProvider, 0);

   DWORD dwStatus = 0;
   PTSTR szStatus = TEXT("User Logon Succeeded, Dumped Token");

   // Get a token for the credentials.
   if (LogonUser(szName, NULL, szPassword, nLogonType, nLogonProvider,
         &g_hToken))      {

      // Success?  Display it
      DumpToken();

   } else {

      // Failure?  Clear the dialog box
      dwStatus = GetLastError();
      szStatus = TEXT("LogonUser");
      g_hToken = NULL;
      SetDlgItemText(hwnd, IDE_TOKEN, TEXT(""));
      SendMessage(g_hwndEnablePrivileges, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndEnableGroups, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndDeletedPrivileges, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndDisabledSids, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndRestrictedSids, LB_RESETCONTENT, 0, 0);
   }

   // Display the status either way
   Status(szStatus, dwStatus);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_DuplicateToken(HWND hwnd) {

   DWORD dwStatus = 0;
   PTSTR szStatus = TEXT("Token Duplicated, Dumped Token");

   // Do we have a token?  If not, fail and complain
   if (g_hToken == NULL) {

      szStatus = TEXT("No Token.  A starting token is required");

   } else {

      HANDLE hOldToken = g_hToken;
      g_hToken = NULL;

      int nIndex = SendMessage(g_hwndImpersonationLevels, CB_GETCURSEL, 0, 0);
      SECURITY_IMPERSONATION_LEVEL nLevel = (SECURITY_IMPERSONATION_LEVEL)
            SendMessage( g_hwndImpersonationLevels, CB_GETITEMDATA, nIndex, 0);

      nIndex = SendMessage(g_hwndTokenTypes, CB_GETCURSEL, 0, 0);
      TOKEN_TYPE nType = (TOKEN_TYPE) SendMessage(g_hwndTokenTypes,
         CB_GETITEMDATA, nIndex, 0);

      // Copy our token
      HANDLE hNewToken;
      if (DuplicateTokenEx(hOldToken, TOKEN_ALL_ACCESS, NULL, nLevel, nType,
            &hNewToken) == TRUE) {

         g_hToken = hNewToken;
         DumpToken();

      } else {

         // Failure?  Clear the dialog box
         dwStatus = GetLastError();
         szStatus = TEXT("DuplicateTokenEx");
         SetDlgItemText(hwnd, IDE_TOKEN, TEXT(""));
         SendMessage(g_hwndEnablePrivileges, LB_RESETCONTENT, 0, 0);
         SendMessage(g_hwndEnableGroups, LB_RESETCONTENT, 0, 0);
         SendMessage(g_hwndDeletedPrivileges, LB_RESETCONTENT, 0, 0);
         SendMessage(g_hwndDisabledSids, LB_RESETCONTENT, 0, 0);
         SendMessage(g_hwndRestrictedSids, LB_RESETCONTENT, 0, 0);
      }

      // No matter what, we kill the last token and take on the new
      CloseHandle(hOldToken);
   }

   // Display the status either way
   Status(szStatus, dwStatus);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_DumpToken(HWND hwnd) {

   // Get the token for the process
   GetToken(hwnd);
   if (g_hToken != NULL) {

      // Success? Display it
      DumpToken();

   } else {

      // No?   Then clear out the dialog box
      SetDlgItemText(hwnd, IDE_TOKEN, TEXT(""));
      SendMessage(g_hwndEnablePrivileges, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndEnableGroups, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndDeletedPrivileges, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndDisabledSids, LB_RESETCONTENT, 0, 0);
      SendMessage(g_hwndRestrictedSids, LB_RESETCONTENT, 0, 0);
   }
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_AdjustGroups()  {

   PTOKEN_GROUPS ptgGroups = NULL;

   try {{

      // No token?  Give up
      if (g_hToken == NULL) {
         Status(TEXT("No Token"), 0);
         goto leave;
      }

      // Allocate a buffer with the token information
      ptgGroups = (PTOKEN_GROUPS) AllocateTokenInfo(g_hToken, TokenGroups);
      if (ptgGroups == NULL)
         goto leave;

      // Enumerate through the list box and find groups to enable in the token
      DWORD dwItem = SendMessage(g_hwndEnableGroups, LB_GETCOUNT, 0, 0);
      while (dwItem-- != 0) {

         DWORD dwIndex = SendMessage(g_hwndEnableGroups, LB_GETITEMDATA,
               dwItem, 0);
         BOOL fSel = SendMessage(g_hwndEnableGroups, LB_GETSEL, dwItem, 0);
         if (fSel)
            ptgGroups->Groups[dwIndex].Attributes |= SE_GROUP_ENABLED;
         else
            ptgGroups->Groups[dwIndex].Attributes &= (~SE_GROUP_ENABLED);
      }

      // Actually adjust the token groups for the token
      if (!AdjustTokenGroups(g_hToken, FALSE, ptgGroups, 0, NULL, NULL))
         Status(TEXT("AdjustTokenGroups"), GetLastError());
      else
         DumpToken();   // Display the new token

   } leave:;
   } catch(...) {}

   // Cleanup
   if (ptgGroups != NULL)
      LocalFree(ptgGroups);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_AdjustTokenPrivileges() {

   PTOKEN_PRIVILEGES ptpPrivileges = NULL;

   try {{

      // No token?  Buh-Bye
      if (g_hToken == NULL) {
         Status(TEXT("No Token"), 0);
         goto leave;
      }

      // Get the token information for privileges
      ptpPrivileges = (PTOKEN_PRIVILEGES) AllocateTokenInfo(g_hToken,
            TokenPrivileges);
      if (ptpPrivileges == NULL)
         goto leave;

      // Enumerate privileges to enable
      DWORD dwItem = SendMessage(g_hwndEnablePrivileges, LB_GETCOUNT, 0, 0);
      while (dwItem-- != 0) {

         DWORD dwIndex = SendMessage(g_hwndEnablePrivileges, LB_GETITEMDATA,
               dwItem, 0);
         BOOL fSel = SendMessage(g_hwndEnablePrivileges, LB_GETSEL, dwItem, 0);
         if (fSel)
            ptpPrivileges->Privileges[dwIndex].Attributes |=
                  SE_PRIVILEGE_ENABLED;
         else
            ptpPrivileges->Privileges[dwIndex].Attributes &=
                  ~SE_PRIVILEGE_ENABLED;
      }

      // Adjust the privileges for the token
      if (!AdjustTokenPrivileges(g_hToken, FALSE, ptpPrivileges, 0, NULL,
            NULL))
         Status(TEXT("AdjustTokenPrivileges"), GetLastError());
      else
         DumpToken();   // Display the new token

   } leave:;
   } catch(...) {}

   // Cleanup
   if (ptpPrivileges != NULL)
      LocalFree(ptpPrivileges);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_SetDACL(HWND hwnd) {

   PSECURITY_DESCRIPTOR pSD  = NULL;
   CSecurityPage*       pSec = NULL;

   PTSTR szStatus = TEXT("Default DACL Set");

   try {{

      // No token?  Outta here
      if (g_hToken == NULL) {
         szStatus = TEXT("No Token");
         goto leave;
      }

      // Get the default dacl for the token
      PTOKEN_DEFAULT_DACL ptdDacl = (PTOKEN_DEFAULT_DACL) AllocateTokenInfo(
            g_hToken, TokenDefaultDacl);
      if (ptdDacl == NULL) {
         szStatus = TEXT("Unable to get default DACL");
         SetLastError(0);
         goto leave;
      }

      // Allocate memory for an SD
      pSD = (PSECURITY_DESCRIPTOR) GlobalAlloc(GPTR,
            SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (pSD == NULL) {
         szStatus = TEXT("GlobalAlloc");
         goto leave;
      }

      // Initialize it
      if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
         szStatus = TEXT("InitializeSecurityDescriptor");
         goto leave;
      }

      // Set the security descriptor DACL to the default DACL of the token
      if (!SetSecurityDescriptorDacl(pSD, TRUE, ptdDacl->DefaultDacl, FALSE)) {
         szStatus = TEXT("SetSecurityDescriptorDacl");
         goto leave;
      }

      // Create an instance of the CSecurityPage object with our SD.
      // This is derived from the ISecurityInformation interface defined by
      // the SDK.  It will be passed to the EditSecurity function to produce
      // the common ACL editor Dialog Box.
      pSec = new CSecurityPage(pSD);
      if (pSec == NULL)
         goto leave;

      // Common dialog box for ACL editing
      if (EditSecurity(hwnd, pSec) && pSec->IsModified())
         DumpToken(); // If success, then redisplay

      SetLastError(0);

   } leave:;
   } catch(...) {}

   // Cleanup
   Status(szStatus, GetLastError());

   if (pSD != NULL)
      GlobalFree(pSD);

   if (pSec != NULL)
      pSec->Release();
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_Browse(HWND hwnd) {

   // Get current filename text
   TCHAR szFileBuf[MAX_PATH];
   *szFileBuf = 0;
   GetDlgItemText(hwnd, IDE_FILENAME, szFileBuf, chDIMOF(szFileBuf));

   // Use common dialog to fetch a filename
   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner   = hwnd;
   ofn.lpstrFilter = TEXT("Executables\000*.exe\000All Files\000*.*\000");
   ofn.lpstrFile   = szFileBuf;
   ofn.nMaxFile    = chDIMOF(szFileBuf);
   ofn.lpstrTitle  = TEXT("Select a File Launchable by CreateProcessAsUser");
   ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON
         | OFN_PATHMUSTEXIST;
   ofn.lpstrDefExt = TEXT("EXE");
   if (GetOpenFileName(&ofn))
      SetDlgItemText(hwnd, IDE_FILENAME, szFileBuf);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_CreateProcess(HWND hwnd) {

   try {{

      // No token?  Adios!
      if (g_hToken == NULL) {
         Status(TEXT("No Token"), 0);
         goto leave;
      }

      // Get current filename text
      TCHAR szFileBuf[MAX_PATH];
      *szFileBuf = 0;
      GetDlgItemText(hwnd, IDE_FILENAME, szFileBuf, chDIMOF(szFileBuf));

      STARTUPINFO si;
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);

      // Create a new process with our current token
      PROCESS_INFORMATION pi;
      if (CreateProcessAsUser(g_hToken, NULL, szFileBuf, NULL, NULL, FALSE, 0,
            NULL, NULL, &si, &pi)) {
         CloseHandle(pi.hProcess);
         CloseHandle(pi.hThread);
      } else
         Status(TEXT("CreateProcessAsUser"), GetLastError());

   } leave:;
   } catch(...) {}
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_AddRestricted(HWND hwnd) {

   // Add the new name to the restricted sids list control
   TCHAR szName[1024];
   if (g_hToken && GetAccountName(hwnd, szName, chDIMOF(szName), TRUE, TRUE))
      SendMessage(g_hwndRestrictedSids, LB_ADDSTRING, 0, (LPARAM) szName);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_RemoveRestricted() {

   LRESULT nIndex = SendMessage(g_hwndRestrictedSids, LB_GETCURSEL, 0, 0);
   if (nIndex != LB_ERR)
      SendMessage(g_hwndRestrictedSids, LB_DELETESTRING, nIndex, 0);

}


///////////////////////////////////////////////////////////////////////////////


void Cmd_SetOwnerGroup(HWND hwnd, BOOL fOwner) {

   PTSTR szStatus = TEXT("Item Set");
   PSID  psid     = NULL;

   try {{

      // No token?  Scram!
      if (g_hToken == NULL) {
         szStatus = TEXT("No Token");
         SetLastError(0);
         goto leave;
      }

      // Get an account name
      TCHAR szName[1024];
      if (!GetAccountName(hwnd, szName, chDIMOF(szName), TRUE, TRUE))  {
         szStatus = TEXT("User Cancelled Item Selection");
         SetLastError(0);
         goto leave;
      }

      // Get the Sid for the name
      DWORD dwSize = 0;
      TCHAR szDomainName[1024];
      DWORD dwDomSize = chDIMOF(szDomainName);
      SID_NAME_USE sidUse;
      LookupAccountName(NULL, szName, NULL, &dwSize, szDomainName, &dwDomSize,
            &sidUse);

      psid = (PSID) GlobalAlloc(GPTR, dwSize);
      if (psid == NULL) {
         szStatus = TEXT("GlobalAlloc");
         goto leave;
      }

      if (!LookupAccountName(NULL, szName, psid, &dwSize, szDomainName,
            &dwDomSize, &sidUse)) {
         szStatus = TEXT("LookupAccountName");
         goto leave;
      }

      // Set the token information using the TOKEN_OWNER structure,
      TOKEN_OWNER to;
      to.Owner = psid;
      if (!SetTokenInformation(g_hToken, fOwner ? TokenOwner :
            TokenPrimaryGroup, &to, sizeof(to))) {
         szStatus = TEXT("SetTokenInformation");
         goto leave;
      }

      DumpToken();

      SetLastError(0);

   } leave:;
   } catch(...) {}

   // Cleanup
   Status(szStatus, GetLastError());

   if (psid != NULL)
      GlobalFree(psid);
}


///////////////////////////////////////////////////////////////////////////////


void Cmd_CreateRestrictedToken() {

   PSID_AND_ATTRIBUTES  psidToDisableAttrib  = NULL;
   PLUID_AND_ATTRIBUTES pluidPrivAttribs     = NULL;
   PSID_AND_ATTRIBUTES  psidToRestrictAttrib = NULL;

   DWORD dwIndex;
   DWORD dwDisableSize  = 0;
   DWORD dwRestrictSize = 0;

   PTSTR szStatus = TEXT("Restricted Token Created");

   try {{

      // No Token?  Gotta run...
      if (g_hToken == NULL) {
         szStatus = TEXT("No Token");
         SetLastError(0);
         goto leave;
      }

      // How big of a structure do I allocate for restricted sids
      dwRestrictSize = SendMessage(g_hwndRestrictedSids, LB_GETCOUNT, 0 , 0);
      psidToRestrictAttrib = (PSID_AND_ATTRIBUTES) LocalAlloc(LPTR,
            dwRestrictSize * sizeof(SID_AND_ATTRIBUTES));
      if (psidToRestrictAttrib == NULL) {
         szStatus = TEXT("LocalAlloc");
         goto leave;
      }

      ZeroMemory(psidToRestrictAttrib, dwRestrictSize
            * sizeof(SID_AND_ATTRIBUTES));

      DWORD dwData;
      DWORD dwSidSize;
      DWORD dwDomainSize;
      TCHAR szBuffer[1024];
      TCHAR szDomain[1024];
      PSID  pSid;

      SID_NAME_USE sidNameUse;

      // For each sid, we find the sid and add it to our array
      for (dwIndex = 0; dwIndex < dwRestrictSize; dwIndex++) {

         dwData = SendMessage(g_hwndRestrictedSids, LB_GETTEXT, dwIndex,
               (LPARAM) szBuffer);
         if (dwData == LB_ERR) {
            dwIndex--;
            break;
         }

         dwSidSize = 0;
         dwDomainSize = chDIMOF(szDomain);

         // Size of SID?
         LookupAccountName(NULL, szBuffer, NULL, &dwSidSize, szDomain,
               &dwDomainSize, &sidNameUse);
         pSid = LocalAlloc(LPTR, dwSidSize);
         if (pSid == NULL) {

            szStatus = TEXT("LocalAlloc");
            goto leave;
         }

         // Get the SID
         if (!LookupAccountName(NULL, szBuffer, pSid, &dwSidSize, szDomain,
                        &dwDomainSize, &sidNameUse)) {

            szStatus = TEXT("LookupAccountName");
            goto leave;
         }

         psidToRestrictAttrib[dwIndex].Sid = pSid;
         psidToRestrictAttrib[dwIndex].Attributes = 0;
      }
      dwRestrictSize = dwIndex;

      // How much memory do we need for our disabled SIDS?
      dwDisableSize = SendMessage(g_hwndDisabledSids, LB_GETCOUNT, 0 , 0);
      psidToDisableAttrib = (PSID_AND_ATTRIBUTES)
            LocalAlloc(LPTR, dwDisableSize * sizeof(SID_AND_ATTRIBUTES));     
      if (psidToDisableAttrib == NULL) {

         szStatus = TEXT("LocalAlloc");
         goto leave;
      }
      ZeroMemory(psidToDisableAttrib,
         dwDisableSize * sizeof(SID_AND_ATTRIBUTES));

      DWORD dwEnd = dwDisableSize;
      dwDisableSize = 0;

      // For each one, add it to our array
      for (dwIndex = 0; dwIndex < dwEnd; dwIndex++) {


         if (SendMessage(g_hwndDisabledSids, LB_GETSEL, dwIndex, 0)) {

            dwData = SendMessage(g_hwndDisabledSids, LB_GETTEXT,
                              dwIndex, (LPARAM) szBuffer);
            if (dwData == LB_ERR) {

               dwIndex--;
               break;
            }

            dwSidSize = 0;
            dwDomainSize = chDIMOF(szDomain);

            // SID size?
            LookupAccountName(NULL, szBuffer, NULL, &dwSidSize, szDomain,
                           &dwDomainSize, &sidNameUse);
            pSid = LocalAlloc(LPTR, dwSidSize);
            if (pSid == NULL) {

               szStatus = TEXT("LocalAlloc");
               goto leave;
            }
            // Get the SID
            if (!LookupAccountName(NULL, szBuffer, pSid, &dwSidSize,
                  szDomain, &dwDomainSize, &sidNameUse)) {

               szStatus = TEXT("LookupAccountName");
               goto leave;
            }

            psidToDisableAttrib[dwDisableSize].Sid = pSid;
            psidToDisableAttrib[dwDisableSize].Attributes = 0;
            dwDisableSize++;
         }
      }

      // Now we find out how much memory we need for our privileges
      DWORD dwPrivSize = SendMessage(g_hwndDeletedPrivileges, LB_GETCOUNT,
            0, 0);
      pluidPrivAttribs = (PLUID_AND_ATTRIBUTES) LocalAlloc(LPTR, dwPrivSize
            * sizeof(LUID_AND_ATTRIBUTES));
      if (pluidPrivAttribs == NULL) {
         szStatus = TEXT("LocalAlloc");
         goto leave;
      }
      ZeroMemory(pluidPrivAttribs, dwPrivSize * sizeof(LUID_AND_ATTRIBUTES));

      // Add the privileges that are to be removed from the token to the list
      dwEnd = dwPrivSize;
      dwPrivSize = 0;
      for (dwIndex = 0; dwIndex < dwEnd; dwIndex++) {

         if (SendMessage(g_hwndDeletedPrivileges, LB_GETSEL, dwIndex, 0)) {

            dwData = SendMessage(g_hwndDeletedPrivileges, LB_GETTEXT, dwIndex,
                  (LPARAM) szBuffer);
            if (dwData == LB_ERR) {
               dwIndex--;
               break;
            }

            LUID luid;
            if (!LookupPrivilegeValue(NULL, szBuffer, &luid)) {
               szStatus = TEXT("LookupPrivilegeValue");
               goto leave;
            }

            pluidPrivAttribs[dwPrivSize].Luid = luid;
            pluidPrivAttribs[dwPrivSize].Attributes = 0;
            dwPrivSize++;
         }
      }

      // Attempt to create restricted token with the structures we built
      HANDLE hNewToken;
      if (!CreateRestrictedToken(g_hToken, 0, dwDisableSize,
            psidToDisableAttrib, dwPrivSize, pluidPrivAttribs, dwRestrictSize,
            psidToRestrictAttrib, &hNewToken)) {
         szStatus = TEXT("CreateRestrictedToken");
         goto leave;
      }

      // Close out our old token
      CloseHandle(g_hToken);

      // This is our new token
      g_hToken = hNewToken;

      // Display it
      DumpToken();

      SetLastError(0);

   } leave:;
   } catch(...) {}

   Status(szStatus, GetLastError());

   // We have to loop to remove all of those sids we allocated
   if (psidToDisableAttrib != NULL) {
      for (dwIndex = 0; dwIndex < dwDisableSize; dwIndex++)
         if (psidToDisableAttrib[dwIndex].Sid != NULL)
            LocalFree(psidToDisableAttrib[dwIndex].Sid);
      LocalFree(psidToDisableAttrib);
   }

   // The privileges we can just free
   if (pluidPrivAttribs != NULL)
      LocalFree(pluidPrivAttribs);

   // More looping to free up sids we allocated
   if (psidToRestrictAttrib != NULL) {
      for (dwIndex = 0; dwIndex < dwRestrictSize; dwIndex++)
         if (psidToRestrictAttrib[dwIndex].Sid != NULL)
            LocalFree(psidToRestrictAttrib[dwIndex].Sid);
      LocalFree(psidToRestrictAttrib);
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   // Special handling for focus notifications
   switch (codeNotify) {

      case EN_SETFOCUS:
      case CBN_SETFOCUS:
         SmartDefaultButton(hwnd, id);
         break;

      case EN_KILLFOCUS:
      case CBN_KILLFOCUS:
         SetDlgDefaultButton(hwnd, IDB_NODEFAULT);
         break;
   }

   // Forward WM_COMMAND messages to specific Cmd_ functions
   switch (id) {

      case IDC_PROCESSES:
         Cmd_Processes(codeNotify);
         break;

      case IDC_THREADS:
         Cmd_Threads(hwnd, codeNotify);
         break;

      case IDB_DUMPTOKEN:
         Cmd_DumpToken(hwnd);
         break;

      case IDB_ADJUSTTOKENPRIVILEGES:
         Cmd_AdjustTokenPrivileges();
         break;

      case IDB_ADJUSTTOKENGROUPS:
         Cmd_AdjustGroups();
         break;

      case IDB_SETDACL:
         Cmd_SetDACL(hwnd);
         break;

      case IDB_BROWSE:
         Cmd_Browse(hwnd);
         break;

      case IDB_CREATEPROCESS:
         Cmd_CreateProcess(hwnd);
         break;

      case IDB_ADDRESTRICTED:
         Cmd_AddRestricted(hwnd);
         break;

      case IDB_REMOVERESTRICTED:
         Cmd_RemoveRestricted();
         break;

      case IDB_SETOWNER:
         Cmd_SetOwnerGroup(hwnd, TRUE);
         break;

      case IDB_SETGROUP:
         Cmd_SetOwnerGroup(hwnd, FALSE);
         break;

      case IDB_CREATERESTRICTEDTOKEN:
         Cmd_CreateRestrictedToken();
         break;

      case IDB_LOGONUSER:
         Cmd_LogonUser(hwnd);
         break;

      case IDB_DUPLICATE:
         Cmd_DuplicateToken(hwnd);
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnDestroy(HWND hwnd) {

   // If we have a process snapshot... Close it up
   if (g_hSnapShot != NULL)
      CloseHandle(g_hSnapShot);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnSize(HWND hwnd, UINT state, int cx, int cy) {

   // Simply call the AdjustControls function of our handy resizer class
   g_pResizer.AdjustControls(cx, cy);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnGetMinMaxInfo(HWND hwnd, PMINMAXINFO pMinMaxInfo) {

   // Just calling another resizer function
   g_pResizer.HandleMinMax(pMinMaxInfo);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_TOKENMASTER);

   // Create a resizer object.  This is a helper class for resizing controls.
   // See "CmnCls.h" for an explanation of use, as well as the source for
   // The CResizeHandler class
   g_pResizer.Initialize(hwnd);

   // Submit most of the controls to the will of the resizer class
   g_pResizer.AnchorControl(CUILayout::AP_TOPMIDDLE, CUILayout::AP_TOPRIGHT, IDS_CREATEPROCESS, TRUE);
   g_pResizer.AnchorControl(CUILayout::AP_CENTER, CUILayout::AP_BOTTOMRIGHT, IDS_ADJUSTPRIV, TRUE);
   g_pResizer.AnchorControl(CUILayout::AP_BOTTOMMIDDLE, CUILayout::AP_BOTTOMRIGHT, IDS_SETTOKENINFORMATION, TRUE);
   g_pResizer.AnchorControl(CUILayout::AP_TOPMIDDLE, CUILayout::AP_MIDDLERIGHT, IDS_ADJUSTGROUPS, TRUE);
   g_pResizer.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_BOTTOMMIDDLE, IDE_TOKEN);
   g_pResizer.AnchorControl(CUILayout::AP_CENTER, CUILayout::AP_BOTTOMRIGHT, IDL_ENABLEPRIVILEGES);
   g_pResizer.AnchorControl(CUILayout::AP_CENTER, CUILayout::AP_MIDDLERIGHT, IDB_ADJUSTTOKENGROUPS);
   g_pResizer.AnchorControl(CUILayout::AP_TOPMIDDLE, CUILayout::AP_MIDDLERIGHT, IDL_ENABLEGROUPS);
   g_pResizer.AnchorControl(CUILayout::AP_TOPRIGHT, CUILayout::AP_TOPRIGHT, IDB_BROWSE);
   g_pResizer.AnchorControl(CUILayout::AP_TOPMIDDLE, CUILayout::AP_TOPRIGHT, IDE_FILENAME);
   g_pResizer.AnchorControl(CUILayout::AP_TOPMIDDLE, CUILayout::AP_TOPMIDDLE, IDB_CREATEPROCESS);
   g_pResizer.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPMIDDLE, IDE_STATUS);

   g_pResizer.AnchorControls(CUILayout::AP_BOTTOMLEFT, CUILayout::AP_BOTTOMLEFT, FALSE, IDL_DELETEDPRIVILEGES,
      IDL_DISABLEDSIDS, IDL_RESTRICTEDSIDS, IDB_ADDRESTRICTED,
      IDB_REMOVERESTRICTED, IDB_CREATERESTRICTEDTOKEN, (UINT)-1);

   g_pResizer.AnchorControls(CUILayout::AP_BOTTOMLEFT, CUILayout::AP_BOTTOMLEFT, TRUE, IDS_CREATEARESTRICTEDTOKEN,
      IDS_DELETEDPRIVILEGES, IDS_DISABLEDSIDS, IDS_ADDREMOVERESTRICTED, (UINT)-1);

   g_pResizer.AnchorControls(CUILayout::AP_BOTTOMMIDDLE, CUILayout::AP_BOTTOMRIGHT, FALSE, IDB_ADJUSTTOKENPRIVILEGES,
      IDB_SETOWNER, IDB_SETGROUP, IDB_SETDACL, (UINT)-1);

   // Store some window handles
   g_hwndProcessCombo = GetDlgItem(hwnd, IDC_PROCESSES);
   g_hwndThreadCombo = GetDlgItem(hwnd, IDC_THREADS);
   g_hwndToken = GetDlgItem(hwnd, IDE_TOKEN);
   g_hwndStatus = GetDlgItem(hwnd, IDE_STATUS);
   g_hwndEnablePrivileges = GetDlgItem(hwnd, IDL_ENABLEPRIVILEGES);
   g_hwndEnableGroups = GetDlgItem(hwnd, IDL_ENABLEGROUPS);
   g_hwndDeletedPrivileges = GetDlgItem(hwnd, IDL_DELETEDPRIVILEGES);
   g_hwndDisabledSids = GetDlgItem(hwnd, IDL_DISABLEDSIDS);
   g_hwndRestrictedSids = GetDlgItem(hwnd, IDL_RESTRICTEDSIDS);
   g_hwndLogonTypes = GetDlgItem(hwnd, IDC_LOGONTYPE);
   g_hwndLogonProviders = GetDlgItem(hwnd, IDC_LOGONPROVIDER);
   g_hwndImpersonationLevels = GetDlgItem(hwnd, IDC_IMPERSONATIONLEVEL);
   g_hwndTokenTypes = GetDlgItem(hwnd, IDC_TOKENTYPE);

   // Refresh the Process snapshot and combo boxes
   RefreshSnapShot();
   PopulateProcessCombo();
   PopulateStaticCombos();

   // Status... Who is this program running as?
   TCHAR szName[1024];
   lstrcpy(szName, TEXT("Token Master running as "));
   DWORD dwLen = lstrlen(szName);
   DWORD dwSize = chDIMOF(szName) - dwLen;
   GetUserName(szName + dwLen, &dwSize);
   Status(szName, 0);

   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnClose(HWND hwnd) {

   EndDialog(hwnd, 0);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityPage::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo) {
   
   pObjectInfo->dwFlags = SI_EDIT_PERMS;
   pObjectInfo->hInstance = GetModuleHandle(NULL);
   pObjectInfo->pszServerName = NULL; 
   pObjectInfo->pszObjectName = L"Default DACL";

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityPage::SetSecurity(SECURITY_INFORMATION SecurityInformation,
      PSECURITY_DESCRIPTOR pSecurityDescriptor) {

   HRESULT hResult = 1;

   try {{

      if (g_hToken == NULL)
         goto leave;

      BOOL fBool;
      TOKEN_DEFAULT_DACL tokenDacl;
      if (!GetSecurityDescriptorDacl(pSecurityDescriptor, &fBool,
            &(tokenDacl.DefaultDacl), &fBool))
         goto leave;

      if (!SetTokenInformation(g_hToken, TokenDefaultDacl, &tokenDacl,
            sizeof(tokenDacl)))
         goto leave;

      Modified();
      hResult = S_OK;

   } leave:;
   } catch(...) {}

   return(hResult);
}


///////////////////////////////////////////////////////////////////////////////


SI_ACCESS CSecurityPage::m_siAccessAllRights[] = { //"winnt.h"
    {&m_guidNULL, GENERIC_ALL,     L"GENERIC_ALL",     SI_ACCESS_GENERAL },
    {&m_guidNULL, GENERIC_READ,    L"GENERIC_READ",    SI_ACCESS_GENERAL },
    {&m_guidNULL, GENERIC_WRITE,   L"GENERIC_WRITE",   SI_ACCESS_GENERAL },
    {&m_guidNULL, GENERIC_EXECUTE, L"GENERIC_EXECUTE", SI_ACCESS_GENERAL },
    
    {&m_guidNULL, DELETE,       L"DELETE",       SI_ACCESS_GENERAL },
    {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", SI_ACCESS_GENERAL },
    {&m_guidNULL, WRITE_DAC,    L"WRITE_DAC",    SI_ACCESS_GENERAL },
    {&m_guidNULL, WRITE_OWNER,  L"WRITE_OWNER",  SI_ACCESS_GENERAL },
    {&m_guidNULL, SYNCHRONIZE,  L"SYNCHRONIZE",  SI_ACCESS_GENERAL },

    {&m_guidNULL, STANDARD_RIGHTS_REQUIRED, L"STANDARD_RIGHTS_REQUIRED", SI_ACCESS_GENERAL }, 
    {&m_guidNULL, STANDARD_RIGHTS_READ,     L"STANDARD_RIGHTS_READ",     SI_ACCESS_GENERAL },
    {&m_guidNULL, STANDARD_RIGHTS_WRITE,    L"STANDARD_RIGHTS_WRITE",    SI_ACCESS_GENERAL },
    {&m_guidNULL, STANDARD_RIGHTS_EXECUTE,  L"STANDARD_RIGHTS_EXECUTE",  SI_ACCESS_GENERAL }, 
    {&m_guidNULL, STANDARD_RIGHTS_ALL,      L"STANDARD_RIGHTS_ALL",      SI_ACCESS_GENERAL }, 
    {&m_guidNULL, SPECIFIC_RIGHTS_ALL,      L"SPECIFIC_RIGHTS_ALL",      SI_ACCESS_GENERAL },
    
    {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", SI_ACCESS_GENERAL },
    {&m_guidNULL, MAXIMUM_ALLOWED,        L"MAXIMUM_ALLOWED",        SI_ACCESS_GENERAL }
};


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityPage::GetAccessRights(const GUID* pguidObjectType,
      DWORD dwFlags, PSI_ACCESS* ppAccess, PULONG pcAccesses,
      PULONG piDefaultAccess) {

   // Skip the Generic Access
   *ppAccess = m_siAccessAllRights;
   *pcAccesses = chDIMOF(m_siAccessAllRights);
   *piDefaultAccess = 0;

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityPage::MapGeneric(const GUID* pguidObjectType,
      PUCHAR pAceFlags, ACCESS_MASK* pMask) {

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


CSecurityPage::CSecurityPage(PSECURITY_DESCRIPTOR pSD) {   
   m_pSD = NULL;
   UseSDCopy(pSD);
}


CSecurityPage::~CSecurityPage() {
   if (m_pSD != NULL) {
      LocalFree(m_pSD);
   }
} 


///////////////////////////////////////////////////////////////////////////////


BOOL CSecurityPage::UseSDCopy(PSECURITY_DESCRIPTOR pSD) {
    
   __try {
      if (pSD == NULL) __leave; 
      if (m_pSD != NULL) {
         LocalFree(m_pSD);
         m_pSD = NULL;
      }

      m_pSD = LocalAllocSDCopy(pSD);
      
   }
   __finally {
   } 
   return(m_pSD != NULL);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityPage::GetSecurity(SECURITY_INFORMATION RequestedInformation, 
   PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) {

   HRESULT hr = 1;
   *ppSecurityDescriptor = m_pSD;
   if (m_pSD != NULL)
      hr = S_OK;
   m_pSD = NULL;
   return(hr);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityPage::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, ULONG* pcInheritTypes) {
   *ppInheritTypes = NULL; 
   *pcInheritTypes = 0;
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
      HANDLE_MSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
      HANDLE_MSG(hwnd, WM_SIZE, Dlg_OnSize);
      HANDLE_MSG(hwnd, WM_CLOSE, Dlg_OnClose);
      HANDLE_MSG(hwnd, WM_GETMINMAXINFO, Dlg_OnGetMinMaxInfo);
      HANDLE_MSG(hwnd, WM_COMMAND, Dlg_OnCommand);
      HANDLE_MSG(hwnd, WM_DESTROY, Dlg_OnDestroy);
   }

   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   TCHAR szUserName[256] = { 0 };
   DWORD dwSize = chDIMOF(szUserName);

   GetUserName(szUserName, &dwSize);

   try {{

      if ((lstrcmpi(szUserName, TEXT("System")) != 0)
            && (lstrcmpi(pszCmdLine, TEXT("NoRelaunch")) != 0))
         if (TryRelaunch())
            goto leave;

      g_hInst = hinstExe;
      DialogBox(hinstExe, MAKEINTRESOURCE(IDD_DUMPTOKEN), NULL, Dlg_Proc);

   } leave:;
   } catch(...) {}

   return(0);
}


///////////////////////////////// End of File /////////////////////////////////