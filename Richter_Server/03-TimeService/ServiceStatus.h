/******************************************************************************
Module:  ServiceStatus.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class wraps a SERVICE_STATUS structure ensuring proper use.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"              /* See Appendix A. */
#include "Gate.h"


///////////////////////////////////////////////////////////////////////////////


class CServiceStatus : public SERVICE_STATUS {
public:
   CServiceStatus();

   void SetDebugMode() { m_fDebug = TRUE; }

   BOOL Initialize(PCTSTR szServiceName, LPHANDLER_FUNCTION_EX pfnHandler, 
      PVOID pvContext, BOOL fOwnProcess, BOOL fInteractWithDesktop = FALSE);

   VOID AcceptControls(DWORD dwFlags, BOOL fAccept = TRUE);
   BOOL ReportStatus();

   BOOL SetUltimateState(DWORD dwUltimateState, DWORD dwWaitHint = 0);
   BOOL AdvanceState(DWORD dwWaitHint, DWORD dwCheckPoint = 0);
   BOOL ReportUltimateState();
   BOOL ReportWin32Error(DWORD dwError);
   BOOL ReportServiceSpecificError(DWORD dwError);

   operator DWORD() const { return(dwCurrentState); }

private:
   BOOL m_fDebug;
   SERVICE_STATUS_HANDLE m_hss;
   CGate m_gate;
};


///////////////////////////////////////////////////////////////////////////////


inline CServiceStatus::CServiceStatus() {

   ZeroMemory(this, sizeof(SERVICE_STATUS));
   m_hss = NULL;
   m_fDebug = FALSE;
}


///////////////////////////////////////////////////////////////////////////////


inline VOID CServiceStatus::AcceptControls(DWORD dwFlags, BOOL fAccept) {

   if (fAccept) dwControlsAccepted |= dwFlags;
   else dwControlsAccepted &= ~dwFlags;
}


///////////////////////////////////////////////////////////////////////////////


inline BOOL CServiceStatus::ReportStatus() {

   BOOL fOk = m_fDebug ? TRUE : ::SetServiceStatus(m_hss, this);
   chASSERT(fOk);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////

   
inline BOOL CServiceStatus::ReportWin32Error(DWORD dwError) {
   dwWin32ExitCode = dwError;
   dwServiceSpecificExitCode = 0;
   return(ReportStatus());
}


inline BOOL CServiceStatus::ReportServiceSpecificError(DWORD dwError) {
   dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
   dwServiceSpecificExitCode = dwError;
   return(ReportStatus());
}


///////////////////////////////////////////////////////////////////////////////


#ifdef SERVICESTATUS_IMPL


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceStatus::Initialize(PCTSTR szServiceName, 
   LPHANDLER_FUNCTION_EX pfnHandler, PVOID pvContext,
   BOOL fOwnProcess, BOOL fInteractWithDesktop) {

   if (!m_fDebug) {
      m_hss = RegisterServiceCtrlHandlerEx(szServiceName, pfnHandler, 
         pvContext);
      chASSERT(m_hss != NULL);
   }

   dwServiceType = fOwnProcess 
      ? SERVICE_WIN32_OWN_PROCESS : SERVICE_WIN32_SHARE_PROCESS;

   if (fInteractWithDesktop) 
      dwServiceType |= SERVICE_INTERACTIVE_PROCESS;

   dwCurrentState = SERVICE_START_PENDING;
   dwControlsAccepted = 0; 
   dwWin32ExitCode = NO_ERROR;
   dwServiceSpecificExitCode = 0;
   dwCheckPoint = 0; 
   dwWaitHint = 2000;
   return(m_fDebug ? TRUE : (m_hss != NULL));
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceStatus::SetUltimateState(DWORD dwUltimateState, 
   DWORD dwWaitHint) {

   DWORD dwPendingState = 0;  // An invalid state value
   switch (dwUltimateState) {
   case SERVICE_STOPPED: 
      dwPendingState = SERVICE_STOP_PENDING; 
      break;

   case SERVICE_RUNNING:
      dwPendingState = (dwCurrentState == SERVICE_PAUSED) 
         ? SERVICE_CONTINUE_PENDING : SERVICE_START_PENDING; 
      break;

   case SERVICE_PAUSED:
      dwPendingState = SERVICE_PAUSE_PENDING; 
      break;

   default:
      chASSERT(dwPendingState != 0);   // Invalid parameter
      break;
   }

   // When creating a new ServiceMain thread, the system assumes 
   // dwCurrentState=SERVICE_START_PENDING, dwCheckPoint=0, dwWaitHint=2000
   // So, since we must always increment the checkpoint, let's start at 1
   dwCheckPoint = 1;
   this->dwWaitHint = dwWaitHint;

   // No error to report
   dwWin32ExitCode = NO_ERROR;
   dwServiceSpecificExitCode = 0;

   BOOL fOk = FALSE; // Assume failure
   if (dwPendingState != 0) {

      // If another pending operation hasn't completed, wait for it.
      m_gate.WaitToEnterGate();

      dwCurrentState = dwPendingState; // Update the state in the structure

      // If no wait hint, we reached the desired state
      fOk = (dwWaitHint != 0) ? ReportStatus() : ReportUltimateState();
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CServiceStatus::AdvanceState(DWORD dwWaitHint, DWORD dwCheckPoint) {

   // A checkpoint of 0 is invalid, wo we'll increment the checkpoint by 1
   this->dwCheckPoint = 
      (dwCheckPoint == 0) ? this->dwCheckPoint + 1 : dwCheckPoint;

   this->dwWaitHint = dwWaitHint;

   // No error to report
   dwWin32ExitCode = NO_ERROR;
   dwServiceSpecificExitCode = 0;

   return(ReportStatus());
}


///////////////////////////////////////////////////////////////////////////////

   
BOOL CServiceStatus::ReportUltimateState() {

   DWORD dwUltimateState = 0;  // An invalid state value
   switch (dwCurrentState) {
   case SERVICE_START_PENDING:
   case SERVICE_CONTINUE_PENDING:
         dwUltimateState = SERVICE_RUNNING; 
         break;
   case SERVICE_STOP_PENDING:
         dwUltimateState = SERVICE_STOPPED; 
         break;
   case SERVICE_PAUSE_PENDING:
         dwUltimateState = SERVICE_PAUSED; 
         break;
   }
   dwCheckPoint = dwWaitHint = 0; // We reached the ultimate state

   // No error to report
   dwWin32ExitCode = NO_ERROR;
   dwServiceSpecificExitCode = 0;

   BOOL fOk = FALSE; // Assume failure

   if (dwUltimateState != 0) {
      dwCurrentState = dwUltimateState;   // Update the state in the structure
      fOk = ReportStatus();

      // Our state change is complete, allow a new state change
      m_gate.LiftGate();
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


#endif   // SERVICESTATUS_IMPL


//////////////////////////////// End of File //////////////////////////////////
