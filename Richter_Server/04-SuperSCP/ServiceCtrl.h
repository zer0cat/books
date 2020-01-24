/******************************************************************************
Module:  ServiceCtrl.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class wraps the control of a Service application.
         See Appendix B.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#ifndef UNICODE
#error This module must be compiled natively using Unicode.
#endif


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"              // See Appendix A.
#include <malloc.h>

#include "..\ClassLib\AutoBuf.h"    // See Appendix B.

#define SECINFO_IMPL
#include "..\ClassLib\SecInfo.h"    // See Appendix B.


///////////////////////////////////////////////////////////////////////////////


class CServiceCtrl {
public:
   CServiceCtrl();
   CServiceCtrl(CSCMCtrl& scm, BOOL fInternalName, 
      PCTSTR pszName, DWORD dwDesiredAccess);
   ~CServiceCtrl();

   BOOL InstallAndOpen(CSCMCtrl& scm, PCTSTR pszInternalName, 
      PCTSTR pszDisplayName, PCTSTR pszDescription, DWORD dwServiceType, 
      DWORD dwStartType, DWORD dwErrorControl, PCTSTR pszPathname, 
      PCTSTR pszLoadOrderGroup, PCTSTR pszDependencies, 
      PCTSTR pszUserName, PCTSTR pszUserPswd, DWORD dwDesiredAccess = 0);

   BOOL Open(CSCMCtrl& scm, BOOL fInternalName, 
      PCTSTR pszName, DWORD dwDesiredAccess);
   BOOL OpenOK() const { return(m_h != NULL); }
   operator SC_HANDLE() const { return(m_h); }
   BOOL Delete();
   void ForceClose() { Reconstruct(); }

   BOOL Start(DWORD nNumArgs, PCTSTR *pArgs, BOOL fWait = FALSE, 
      SERVICE_STATUS* pss = NULL, DWORD dwMilliseconds = INFINITE);

   BOOL Start(PCTSTR pszArgs, BOOL fWait = FALSE, 
      SERVICE_STATUS* pss = NULL, DWORD dwMilliseconds = INFINITE);

   BOOL Control(DWORD dwControl, BOOL fWait = FALSE, 
      SERVICE_STATUS* pss = NULL, DWORD dwMilliseconds = INFINITE);

   BOOL WaitForState(DWORD dwDesiredState, 
      SERVICE_STATUS* pss = NULL, DWORD dwMilliseconds = INFINITE);

   BOOL QueryStatus(SERVICE_STATUS* pss);
   BOOL QueryStatus(SERVICE_STATUS_PROCESS* pssp);

   const QUERY_SERVICE_CONFIG* QueryConfig();
   const PCTSTR QueryDescription();
   BOOL ChangeConfig(PCTSTR pszDisplayName, PCTSTR pszDescription, 
      DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl, 
      PCTSTR pBinaryPathName, PCTSTR pLoadOrderGroup, PCTSTR pDependencies, 
      PCTSTR pszUserName, PCTSTR pszUserPswd);

   const SERVICE_FAILURE_ACTIONS* QueryFailureActions();
   BOOL _cdecl ChangeFailureActions(DWORD dwResetPeriod, 
      PCTSTR pszRebootMsg, PCTSTR pszCommand, 
      SC_ACTION_TYPE Type, DWORD dwDelay, ...);

   BOOL CreateDependencySnapshot(DWORD dwServiceState);
   int GetDependencySnapshotNum();
   const ENUM_SERVICE_STATUS* GetDependencySnapshotEntry(int nIndex);

   void EditSecurity(HWND hwnd);

private:
   void Reconstruct(BOOL fFirstTime = FALSE);

private:
   SC_HANDLE m_h;
   union {
      BYTE bRaw[1024 + sizeof(PTSTR)];
      SERVICE_DESCRIPTION cooked;
   } m_sd;

   CAutoBuf<QUERY_SERVICE_CONFIG> m_pServiceConfig;
   CAutoBuf<ENUM_SERVICE_STATUS> m_pDependencySnapshot;
   DWORD m_nDependencySnapshotNum;

   union {
      BYTE bRaw[1024];
      SERVICE_FAILURE_ACTIONS cooked;
   } m_sfa;

private:
   class CServiceSecInfo : public CSecInfo {
   public:
      CServiceSecInfo(CServiceCtrl* pServiceCtrl, PTSTR pszMachine = NULL);

   private:
      HRESULT WINAPI GetObjectInformation(PSI_OBJECT_INFO pObjInfo);
      HRESULT WINAPI GetAccessRights(const GUID* pguidObjectType,
         DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, 
         ULONG* piDefaultAccess);
      HRESULT WINAPI GetSecurity(SECURITY_INFORMATION RequestedInformation,
         PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault);
      HRESULT WINAPI SetSecurity(SECURITY_INFORMATION SecurityInformation,
         PSECURITY_DESCRIPTOR psd);
      HRESULT WINAPI MapGeneric(const GUID *pguidObjectType, 
         UCHAR *pAceFlags, ACCESS_MASK *pMask);

   private:
      CServiceCtrl* m_pServiceCtrl;
      PTSTR         m_pszMachine;
   };
};


///////////////////////////////////////////////////////////////////////////////


inline PCTSTR NullStringToNull(PCTSTR psz) {
   if ((psz != NULL) && (psz[0] == 0)) psz = NULL;
   return(psz);
}


///////////////////////////////////////////////////////////////////////////////


inline CServiceCtrl::CServiceCtrl() {
   Reconstruct(TRUE);
}


inline CServiceCtrl::CServiceCtrl(CSCMCtrl& scm, 
   BOOL fInternalName, PCTSTR pszName, DWORD dwDesiredAccess) {
   Reconstruct(TRUE);
   Open(scm, fInternalName, pszName, dwDesiredAccess);
}


inline CServiceCtrl::~CServiceCtrl() {
   Reconstruct();
}


///////////////////////////////////////////////////////////////////////////////


inline BOOL CServiceCtrl::Delete() {

   chASSERT(OpenOK());
   // Mark the service for deletion.
   // NOTE: The service is not deleted until all handles
   //       to it are closed and the service stops running.
   return(::DeleteService(m_h));
}


///////////////////////////////////////////////////////////////////////////////


inline BOOL CServiceCtrl::QueryStatus(SERVICE_STATUS* pss) {

   chASSERT(OpenOK());
   return(::QueryServiceStatus(m_h, pss));
}


inline BOOL CServiceCtrl::QueryStatus(SERVICE_STATUS_PROCESS* pssp) {

   chASSERT(OpenOK());
   DWORD cb;
   return(::QueryServiceStatusEx(m_h, SC_STATUS_PROCESS_INFO, 
      (PBYTE) pssp, sizeof(*pssp), &cb));
}


///////////////////////////////////////////////////////////////////////////////


inline const PCTSTR CServiceCtrl::QueryDescription() {

   chASSERT(OpenOK());
   DWORD cb;
   BOOL fOk = ::QueryServiceConfig2(m_h, SERVICE_CONFIG_DESCRIPTION, 
      (PBYTE) &m_sd, sizeof(m_sd), &cb);
   return(fOk ? m_sd.cooked.lpDescription : NULL);
}


///////////////////////////////////////////////////////////////////////////////


inline const SERVICE_FAILURE_ACTIONS* CServiceCtrl::QueryFailureActions() {

   chASSERT(OpenOK());
   DWORD cb;
   BOOL fOk = ::QueryServiceConfig2(m_h, SERVICE_CONFIG_FAILURE_ACTIONS, 
      m_sfa.bRaw, sizeof(m_sfa), &cb);
   return(fOk ? &m_sfa.cooked : NULL);
}


///////////////////////////////////////////////////////////////////////////////


inline int CServiceCtrl::GetDependencySnapshotNum() {

   chASSERT(OpenOK());
   return(m_nDependencySnapshotNum);
}


inline const ENUM_SERVICE_STATUS* CServiceCtrl::GetDependencySnapshotEntry(
   int nIndex) {
   
   chASSERT(OpenOK() && (nIndex < GetDependencySnapshotNum()));
   return(&m_pDependencySnapshot[nIndex]);
}


///////////////////////////////////////////////////////////////////////////////


#ifdef SERVICECTRL_IMPL


///////////////////////////////////////////////////////////////////////////////


void CServiceCtrl::Reconstruct(BOOL fFirstTime) {

   if (!fFirstTime) {
      m_pServiceConfig.Free();
      m_pDependencySnapshot.Free();
      if (m_h != NULL) 
         ::CloseServiceHandle(m_h);
   }

   // Reset our state
   m_h = NULL;
   m_pServiceConfig = 0;
   m_nDependencySnapshotNum = 0;
   ZeroMemory(&m_sfa, sizeof(m_sfa));
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::InstallAndOpen(CSCMCtrl& scm, PCTSTR pszInternalName, 
   PCTSTR pszDisplayName, PCTSTR pszDescription, 
   DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl, 
   PCTSTR pszPathname, PCTSTR pszLoadOrderGroup, PCTSTR pszDependencies, 
   PCTSTR pszUserName, PCTSTR pszUserPswd, DWORD dwDesiredAccess) {

   Reconstruct();
   chASSERT(scm.OpenOK());
   m_h = ::CreateService(scm, pszInternalName, pszDisplayName, 
      dwDesiredAccess | SERVICE_CHANGE_CONFIG, dwServiceType, dwStartType, 
      dwErrorControl, pszPathname, NullStringToNull(pszLoadOrderGroup), NULL,
      NullStringToNull(pszDependencies), 
      NullStringToNull(pszUserName), NullStringToNull(pszUserPswd));

   if (OpenOK()) {
      SERVICE_DESCRIPTION sd = { (PTSTR) pszDescription };
      ::ChangeServiceConfig2(m_h, SERVICE_CONFIG_DESCRIPTION, &sd);
   }
   return(OpenOK());
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::Open(CSCMCtrl& scm, BOOL fInternalName, 
   PCTSTR pszName, DWORD dwDesiredAccess) {

   Reconstruct();
   chASSERT(scm.OpenOK());

   if (!fInternalName) {
      // Caller passed service's display name
      pszName = scm.GetInternalName(pszName); 
   }
   if (pszName != NULL)
      m_h = ::OpenService(scm, pszName, dwDesiredAccess);
   return(OpenOK());
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::Start(DWORD nNumArgs, PCTSTR* pArgs,
   BOOL fWait, SERVICE_STATUS* pss, DWORD dwMilliseconds) {

   chASSERT(OpenOK());
   if (pss != NULL) 
      ZeroMemory(pss, sizeof(*pss));

   BOOL fOk = ::StartService(m_h, nNumArgs, pArgs);
   if (fOk && fWait) {
      fOk = WaitForState(SERVICE_RUNNING, pss, dwMilliseconds);
   }
   return(fOk);   // Call GetLastError to get error
}


BOOL CServiceCtrl::Start(PCTSTR pszArgs, 
   BOOL fWait, SERVICE_STATUS* pss, DWORD dwMilliseconds) {

   chASSERT(OpenOK());
   int nNumArgs;
   PCTSTR* pArgs = (PCTSTR*) ::CommandLineToArgvW(pszArgs, &nNumArgs);
   BOOL fOk = Start(nNumArgs, pArgs, fWait, pss, dwMilliseconds);
   ::HeapFree(::GetProcessHeap(), 0, pArgs);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::Control(DWORD dwControl, 
   BOOL fWait, SERVICE_STATUS* pss, DWORD dwMilliseconds) {

   chASSERT(OpenOK());

   DWORD dwFinalState = (DWORD) -1;
   switch (dwControl) {
   case SERVICE_CONTROL_STOP:        
      dwFinalState = SERVICE_STOPPED;
      break;
   case SERVICE_CONTROL_PAUSE:     
      dwFinalState = SERVICE_PAUSED;
      break;
   case SERVICE_CONTROL_CONTINUE:    
      dwFinalState = SERVICE_RUNNING;
      break;
   }

   SERVICE_STATUS ss = { 0 };
   if (pss == NULL) pss = &ss;
   BOOL fOk = ::ControlService(m_h, dwControl, pss);
   if (fOk && fWait && (dwFinalState != -1)) {
      fOk = WaitForState(dwFinalState, pss, dwMilliseconds);
   }
   return(fOk);   // Call GetLastError to get error
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::WaitForState(DWORD dwDesiredState, 
   SERVICE_STATUS* pss, DWORD dwMilliseconds) {

   SERVICE_STATUS ss = { 0 };
   if (pss == NULL) pss = &ss;

   chASSERT(OpenOK());

   BOOL  fServiceOk = TRUE;
   BOOL  fFirstTime = TRUE; // Don't compare state/checkpoint the first time
   DWORD dwLastState = 0, dwLastCheckPoint = 0;
   DWORD dwTimeout = GetTickCount() + dwMilliseconds;

   // Loop until service reaches desired state, error occurs, or timeout
   for (;;) {
      // Get current state of service
      fServiceOk = QueryStatus(pss);

      // If we can't query the service, we're done
      if (!fServiceOk) break;

      // If the service reaches the desired state, we're done
      if (pss->dwCurrentState == dwDesiredState) break;

      // If timeout, we're done
      if ((dwMilliseconds != INFINITE) && (dwTimeout < GetTickCount())) {
         SetLastError(ERROR_TIMEOUT); 
         break;
      }

      // If first time, save service's state/checkpoint
      if (fFirstTime) {
         dwLastState      = pss->dwCurrentState;
         dwLastCheckPoint = pss->dwCheckPoint;
         fFirstTime       = FALSE;
      } else {    
         // If not first time & state changed, save state/checkpoint
         if (dwLastState != pss->dwCurrentState) {
            dwLastState      = pss->dwCurrentState;
            dwLastCheckPoint = pss->dwCheckPoint;
         } else {
            // State hasn't change, make sure checkpoint isn't decreasing
            if (pss->dwCheckPoint >= dwLastCheckPoint) {
               // Checkpoint has increased, save checkpoint
               dwLastCheckPoint = pss->dwCheckPoint;
            } else {
               // Bad checkpoint, service failed, we're done!
               fServiceOk = FALSE; 
               break;
            }
         }
      }
      // We're not done, wait the specified period of time
      DWORD dwWaitHint = pss->dwWaitHint / 10;    // Poll 1/10 of the wait hint
      if (dwWaitHint <  1000) dwWaitHint = 1000;  // At most once a second
      if (dwWaitHint > 10000) dwWaitHint = 10000; // At least every 10 seconds
      Sleep(dwWaitHint);
   }

   // Note: The last SERVICE_STATUS is returned to the caller so
   // that the caller can check the service state and error codes.
   return(fServiceOk);
}


///////////////////////////////////////////////////////////////////////////////


const QUERY_SERVICE_CONFIG* CServiceCtrl::QueryConfig() {

   chASSERT(OpenOK());

   BOOL fOk;
   GROWUNTIL(FALSE,
      fOk = ::QueryServiceConfig(m_h, m_pServiceConfig, 
         m_pServiceConfig, m_pServiceConfig));

   return(fOk ? (QUERY_SERVICE_CONFIG*) m_pServiceConfig : NULL);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::ChangeConfig(PCTSTR pszDisplayName, PCTSTR pszDescription,
   DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl, 
   PCTSTR pBinaryPathName, PCTSTR pLoadOrderGroup, PCTSTR pDependencies, 
   PCTSTR pszUserName, PCTSTR pszUserPswd) {

   chASSERT(OpenOK());

   // If "LocalSystem" is specified for the user account, pass NULL.
   if ((pszUserName != NULL) && 
      (lstrcmpi(pszUserName, TEXT("LocalSystem")) == 0))
      pszUserPswd = pszUserName = NULL;

   BOOL fOk = ::ChangeServiceConfig(m_h, dwServiceType, dwStartType, 
      dwErrorControl, pBinaryPathName, 
      ((pLoadOrderGroup == NULL) || (pLoadOrderGroup[0] == 0)) 
         ? NULL : pLoadOrderGroup,
      NULL,
      ((pDependencies == NULL) || (pDependencies[0] == 0))  
         ? NULL : pDependencies,
      ((pszUserName == NULL) || (pszUserName[0] == 0)) ? NULL : pszUserName,
      ((pszUserPswd == NULL) || (pszUserPswd[0] == 0)) ? NULL : pszUserPswd,
      pszDisplayName);

   if (fOk) {
      SERVICE_DESCRIPTION sd = { (PTSTR) pszDescription };
      ::ChangeServiceConfig2(m_h, SERVICE_CONFIG_DESCRIPTION, &sd);
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL _cdecl CServiceCtrl::ChangeFailureActions(DWORD dwResetPeriod, 
   PCTSTR pszRebootMsg, PCTSTR pszCommand, 
   SC_ACTION_TYPE Type, DWORD dwDelay, ...) {

   chASSERT(OpenOK());
   SERVICE_FAILURE_ACTIONS sfa;
   sfa.dwResetPeriod = dwResetPeriod;
   sfa.lpCommand = (PTSTR) pszCommand;
   sfa.lpRebootMsg = (PTSTR) pszRebootMsg;
   sfa.lpsaActions = NULL;

   va_list argList;
   va_start(argList, pszCommand);
   SC_ACTION_TYPE scType = va_arg(argList, SC_ACTION_TYPE);
   for (sfa.cActions = 0; scType  != -1; sfa.cActions++) {
      va_arg(argList, SC_ACTION_TYPE); // Skip dwDelay
      scType = (SC_ACTION_TYPE) va_arg(argList, DWORD);
   }
   va_end(argList);

   if (sfa.cActions > 0) {

      sfa.lpsaActions = (SC_ACTION*) _alloca(sfa.cActions * sizeof(SC_ACTION));

      va_start(argList, pszCommand);
      for (UINT x = 0; x < sfa.cActions; x++) {
         sfa.lpsaActions[x].Type  = va_arg(argList, SC_ACTION_TYPE);
         sfa.lpsaActions[x].Delay = va_arg(argList, DWORD);
      }
      va_end(argList);
   }

   return(::ChangeServiceConfig2(m_h, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa));
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceCtrl::CreateDependencySnapshot(DWORD dwServiceState) {

   chASSERT(OpenOK());

   m_nDependencySnapshotNum = 0;
   BOOL fOk;
   GROWUNTIL(FALSE,
      fOk = ::EnumDependentServices(m_h, dwServiceState, 
         m_pDependencySnapshot, m_pServiceConfig, m_pServiceConfig, 
         &m_nDependencySnapshotNum));

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


void CServiceCtrl::EditSecurity(HWND hwnd) {

   CServiceSecInfo* pssi = new CServiceSecInfo(this, NULL);
   ::EditSecurity(hwnd, pssi);
   pssi->Release();
}


///////////////////////////////////////////////////////////////////////////////


CServiceCtrl::CServiceSecInfo::CServiceSecInfo(CServiceCtrl* pServiceCtrl, 
   PTSTR pszMachine) { 
   m_pServiceCtrl = pServiceCtrl;
   m_pszMachine   = pszMachine;
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CServiceCtrl::CServiceSecInfo::GetObjectInformation(
   PSI_OBJECT_INFO pObjInfo) {

   pObjInfo->dwFlags = SI_ADVANCED | SI_EDIT_AUDITS | SI_EDIT_PERMS | 
      SI_NO_TREE_APPLY | SI_NO_ACL_PROTECT | SI_EDIT_OWNER | SI_OWNER_READONLY;
   pObjInfo->hInstance = NULL;
   pObjInfo->pszServerName = m_pszMachine;
   pObjInfo->pszObjectName = m_pServiceCtrl->QueryConfig()->lpDisplayName;
   pObjInfo->pszPageTitle = NULL;
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CServiceCtrl::CServiceSecInfo::GetSecurity(
   SECURITY_INFORMATION RequestedInformation,
   PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) {

   chASSERT(!fDefault);
   DWORD dw = GetSecurityInfo(*m_pServiceCtrl, SE_SERVICE, 
      RequestedInformation, NULL, NULL, NULL, NULL, ppSecurityDescriptor);
   return((dw == NO_ERROR) ? S_OK : E_FAIL);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CServiceCtrl::CServiceSecInfo::SetSecurity(
   SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR psd) {

   BOOL f;
   PSID psidOwner, psidGroup;
   GetSecurityDescriptorOwner(psd, &psidOwner, &f);
   GetSecurityDescriptorGroup(psd, &psidGroup, &f);
   PACL pDacl, pSacl;
   GetSecurityDescriptorDacl(psd, &f, &pDacl, &f);
   GetSecurityDescriptorSacl(psd, &f, &pSacl, &f);
   DWORD dw = SetSecurityInfo(*m_pServiceCtrl, SE_SERVICE, 
      SecurityInformation, psidOwner, psidGroup, pDacl, pSacl);
   return((dw == NO_ERROR) ? S_OK : E_FAIL);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT WINAPI CServiceCtrl::CServiceSecInfo::GetAccessRights(
   const GUID* pguidObjectType, DWORD dwFlags, PSI_ACCESS* ppAccess, 
   ULONG* pcAccesses, ULONG* piDefaultAccess) {

   static SI_ACCESS s_siAccessRights[] = {
      RIGHT(SERVICE_ALL_ACCESS,           "Full Control",         TRUE,  TRUE),
      RIGHT(SERVICE_QUERY_CONFIG,         "Query Configuration",  TRUE,  TRUE),
      RIGHT(SERVICE_CHANGE_CONFIG,        "Change Configuration", TRUE,  TRUE),
      RIGHT(SERVICE_QUERY_CONFIG,         "Query Status",         TRUE,  TRUE),
      RIGHT(SERVICE_ENUMERATE_DEPENDENTS, "Enumerate Dependents", TRUE,  TRUE),
      RIGHT(SERVICE_START,                "Start",                TRUE,  TRUE),
      RIGHT(SERVICE_STOP,                 "Stop",                 TRUE,  TRUE),
      RIGHT(SERVICE_PAUSE_CONTINUE,       "Pause & Continue",     TRUE,  TRUE),
      RIGHT(SERVICE_INTERROGATE,          "Interrogate",          TRUE,  TRUE),
      RIGHT(SERVICE_USER_DEFINED_CONTROL, "User Defined Control", TRUE,  TRUE),
      RIGHT(DELETE,                       "Delete",               TRUE,  TRUE),
      RIGHT(READ_CONTROL,                 "Read Permissions",     FALSE, TRUE),
      RIGHT(WRITE_DAC,                    "Change Permissions",   FALSE, TRUE),
      RIGHT(WRITE_OWNER,                  "Take Ownership",       FALSE, TRUE)
   };
   *ppAccess = s_siAccessRights;
   *pcAccesses = chDIMOF(s_siAccessRights);
   *piDefaultAccess = 0;

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT WINAPI CServiceCtrl::CServiceSecInfo::MapGeneric(
   const GUID *pguidObjectType, UCHAR *pAceFlags, ACCESS_MASK *pMask) {

   GENERIC_MAPPING gm;
   gm.GenericRead    = STANDARD_RIGHTS_READ    | SERVICE_QUERY_CONFIG | 
      SERVICE_QUERY_STATUS | SERVICE_INTERROGATE | 
      SERVICE_ENUMERATE_DEPENDENTS;
   gm.GenericWrite   = STANDARD_RIGHTS_WRITE   | SERVICE_CHANGE_CONFIG;
   gm.GenericExecute = STANDARD_RIGHTS_EXECUTE | SERVICE_START | 
      SERVICE_STOP | SERVICE_PAUSE_CONTINUE | SERVICE_USER_DEFINED_CONTROL;
   gm.GenericAll = SERVICE_ALL_ACCESS;
   if(*pMask & GENERIC_EXECUTE )
      GetLastError() ; 
   MapGenericMask(pMask, &gm);  

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


#endif   // SERVICECTRL_IMPL


///////////////////////////////// End of File /////////////////////////////////
