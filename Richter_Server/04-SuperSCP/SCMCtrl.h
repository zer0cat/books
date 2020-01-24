/******************************************************************************
Module:  SCMCtrl.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class wraps the control of a Service Control Manager.
         See Appendix B.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"              // See Appendix A.
#include "..\ClassLib\AutoBuf.h"    // See Appendix B.


///////////////////////////////////////////////////////////////////////////////


class CSCMCtrl {
public:
   CSCMCtrl(DWORD dwDesiredAccess = SC_MANAGER_CONNECT, 
      PCTSTR pszMachine = NULL);
   ~CSCMCtrl();

   BOOL Open(DWORD dwDesiredAccess = SC_MANAGER_CONNECT, 
      PCTSTR pszMachine = NULL);
   BOOL OpenOK() const { return(m_h != NULL); }
   operator SC_HANDLE() const { return(m_h); }
   void ForceClose() { Reconstruct(); }

   BOOL LockDatabase(BOOL fLock = TRUE);
   const PCTSTR QueryLockOwner(PDWORD pdwLockDuration = NULL);

   const PCTSTR GetInternalName(PCTSTR pszDisplayName);
   const PCTSTR GetDisplayName(PCTSTR pszInternalName);

   BOOL CreateStatusSnapshot(DWORD dwServiceState);
   int  GetStatusSnapshotNum();
   const ENUM_SERVICE_STATUS* GetStatusSnapshotEntry(int nIndex);

private:
   void Reconstruct(BOOL fFirstTime = FALSE);

private:
   SC_HANDLE m_h;
   TCHAR m_szMachine[_MAX_PATH];
   
   SC_LOCK m_scLock;
   union {
      BYTE bRaw[1024];
      QUERY_SERVICE_LOCK_STATUS cooked;
   } m_qsls;

   TCHAR m_szInternalName[1024];
   TCHAR m_szDisplayName[1024];

   DWORD m_nStatusSnapshotNum;
   CAutoBuf<ENUM_SERVICE_STATUS> m_pStatusSnapshot;
};


///////////////////////////////////////////////////////////////////////////////


inline CSCMCtrl::CSCMCtrl(DWORD dwDesiredAccess, PCTSTR pszMachine) {
   Reconstruct(TRUE);
   Open(dwDesiredAccess, pszMachine);
}


inline CSCMCtrl::~CSCMCtrl() {
   Reconstruct();
}


///////////////////////////////////////////////////////////////////////////////


inline const PCTSTR CSCMCtrl::GetInternalName(PCTSTR pszDisplayName) {
   chASSERT(OpenOK());
   DWORD cch = chDIMOF(m_szInternalName);
   BOOL fOk = ::GetServiceKeyName(m_h, pszDisplayName, m_szInternalName, &cch);
   return(fOk ? m_szInternalName : NULL);
}


inline const PCTSTR CSCMCtrl::GetDisplayName(PCTSTR pszInternalName) {
   chASSERT(OpenOK());
   DWORD cch = chDIMOF(m_szDisplayName);
   BOOL fOk = ::GetServiceDisplayName(m_h, pszInternalName, 
      m_szDisplayName, &cch);
   return(fOk ? m_szDisplayName : NULL);
}


///////////////////////////////////////////////////////////////////////////////


inline BOOL CSCMCtrl::LockDatabase(BOOL fLock) {
   chASSERT(OpenOK());
   BOOL fOk = FALSE;
   if (fLock) {   // Lock the SCM database
      chASSERT(m_scLock == NULL);
      m_scLock = ::LockServiceDatabase(m_h);
      fOk = (m_scLock != NULL);
      // Note: Closing the SCM handle does not unlock it!
   } else {       // Unlock the SCM database
      chASSERT(m_scLock != NULL);
      fOk = ::UnlockServiceDatabase(m_scLock); 
      m_scLock = NULL;
   }
   return(fOk);
}


inline const PCTSTR CSCMCtrl::QueryLockOwner(PDWORD pdwLockDuration) {

   chASSERT(OpenOK());
   DWORD cb;
   BOOL fOk = ::QueryServiceLockStatus(m_h, &m_qsls.cooked, 
      sizeof(m_qsls), &cb);
   if (fOk && m_qsls.cooked.fIsLocked && (pdwLockDuration != NULL)) {
      *pdwLockDuration = m_qsls.cooked.dwLockDuration;
   }
   return((fOk && m_qsls.cooked.fIsLocked) ? m_qsls.cooked.lpLockOwner : NULL);
}


///////////////////////////////////////////////////////////////////////////////


inline int CSCMCtrl::GetStatusSnapshotNum() {

   chASSERT(OpenOK());
   return(m_nStatusSnapshotNum);
}


inline const ENUM_SERVICE_STATUS* CSCMCtrl::GetStatusSnapshotEntry(
   int nIndex) {

   chASSERT(OpenOK() && (nIndex < GetStatusSnapshotNum()));
   return(&m_pStatusSnapshot[nIndex]);
}


///////////////////////////////////////////////////////////////////////////////


#ifdef SCMCTRL_IMPL


///////////////////////////////////////////////////////////////////////////////


BOOL CSCMCtrl::Open(DWORD dwDesiredAccess, PCTSTR pszMachine) {
   Reconstruct();
   m_h = ::OpenSCManager(pszMachine, NULL, dwDesiredAccess);
   if (OpenOK()) {
      if (pszMachine == NULL) m_szMachine[0] = 0;
      else ::lstrcpy(m_szMachine, pszMachine);
   } else {
      Reconstruct();
   } 
   return(OpenOK());
}


///////////////////////////////////////////////////////////////////////////////


void CSCMCtrl::Reconstruct(BOOL fFirstTime) {

   if (!fFirstTime) {
      m_pStatusSnapshot.Free();
      if (m_h != NULL) 
         ::CloseServiceHandle(m_h);
      if (m_scLock != NULL)
         LockDatabase(FALSE);
   }

   // Reset our state
   m_h = NULL;
   m_szMachine[0] = 0;   
   m_scLock = NULL;
   ZeroMemory(&m_qsls, sizeof(m_qsls));
   m_szInternalName[0] = 0;
   m_szDisplayName[0] = 0;
   m_nStatusSnapshotNum = 0;
}


///////////////////////////////////////////////////////////////////////////////


BOOL CSCMCtrl::CreateStatusSnapshot(DWORD dwServiceState) {

   chASSERT(OpenOK());

   BOOL fOk;
   DWORD dwResumeHandle = 0;
   GROWUNTIL(FALSE,
      fOk = ::EnumServicesStatus(m_h, SERVICE_WIN32, dwServiceState,
         m_pStatusSnapshot, m_pStatusSnapshot, m_pStatusSnapshot, 
         &m_nStatusSnapshotNum, &dwResumeHandle));

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


#endif   // SCMCTRL_IMPL


///////////////////////////////// End of File /////////////////////////////////
