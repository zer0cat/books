/******************************************************************************
Module:  EventLog.h
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"                 /* See Appendix A. */


/******************************************************************************


To add a message compiler file to Visual Studio's Build Environment
perform the following steps:

  1) Insert the *.mc file into the project
  2) Select the MC file in the Project Settings dialog box
  3) Change the description to "Message Compiler"
  4) Add the following 2 Build Command(s):
     "mc -s -U -h $(ProjDir) -r $(ProjDir) $(InputName)"
     "del $(ProjDir)\$(InputName).rc"
  5) Add the following 2 Output file(s):
     "$(InputName).h"
     "Msg00001.bin"
  6) Include the generated header file in the source file(s)
     that call the ReportEvent function
  7) Since I delete the MC generated .rc file, you must manually
     import the MSG0001.bin file into your project's .rc file
     using a resource type of 11 and a resource ID of 1.


******************************************************************************/


class CEventLog {
public:
   CEventLog(PCTSTR pszAppName);
   ~CEventLog();
   BOOL Install(DWORD dwTypesSupported, 
      PCTSTR pszEventMsgFilePaths = NULL, 
      PCTSTR pszParameterMsgFilePaths = NULL, 
      DWORD dwCategoryCount = 0, 
      PCTSTR pszCategoryMsgFilePaths = NULL);
   BOOL Uninstall();

   // Records an event into the event log
   enum REPORTEVENTUSER { REUSER_NOTAPPLICABLE, REUSER_SERVICE, REUSER_CLIENT }; 
   BOOL ReportEvent(WORD wType,  WORD wCategory, 
      DWORD dwEventID, REPORTEVENTUSER reu = REUSER_NOTAPPLICABLE, 
      WORD  wNumStrings = 0, PCTSTR *pStrings = NULL, 
      DWORD dwDataSize = 0, PVOID pvRawData = NULL);  

private:
   PCTSTR m_pszAppName;
   HANDLE m_hEventLog;
};


///////////////////////////////////////////////////////////////////////////////


#ifdef EVENTLOG_IMPL


///////////////////////////////////////////////////////////////////////////////


CEventLog::CEventLog(PCTSTR pszAppName) {
   m_pszAppName = pszAppName;
   m_hEventLog = NULL;
}


CEventLog::~CEventLog() {
   if (m_hEventLog != NULL) { 
      ::DeregisterEventSource(m_hEventLog);
   }
}


//////////////////////////////////////////////////////////////////////////////


BOOL CEventLog::Install(DWORD dwTypesSupported, 
   PCTSTR pszEventMsgFilePaths, PCTSTR pszParameterMsgFilePaths, 
   DWORD dwCategoryCount, PCTSTR pszCategoryMsgFilePaths) {

   // Make sure at least one valid Support Type is specified
   chASSERT(0 != (dwTypesSupported & 
      (EVENTLOG_INFORMATION_TYPE | EVENTLOG_WARNING_TYPE | 
      EVENTLOG_ERROR_TYPE)));

   BOOL fOk = TRUE;
   TCHAR szSubKey[_MAX_PATH];
   wsprintf(szSubKey, 
      TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\%s"),
      m_pszAppName);

   // If the application doesn't support any types (the default), 
   // don't install an event log for this service
   HKEY hkey = NULL;
   __try {
      LONG l;
      l = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, 
         REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, NULL);
      if (l != NO_ERROR) __leave;
      
      l = RegSetValueEx(hkey, TEXT("TypesSupported"), 0, REG_DWORD, 
         (PBYTE) &dwTypesSupported, sizeof(dwTypesSupported));
      if (l != NO_ERROR) __leave;

      TCHAR szModulePathname[MAX_PATH];
      GetModuleFileName(NULL, szModulePathname, chDIMOF(szModulePathname));

      if (pszEventMsgFilePaths == NULL) 
         pszEventMsgFilePaths = szModulePathname;
      l = RegSetValueEx(hkey, TEXT("EventMessageFile"), 0, REG_EXPAND_SZ, 
         (PBYTE) pszEventMsgFilePaths, chSIZEOFSTRING(pszEventMsgFilePaths));
      if (l != NO_ERROR) __leave;

      if (pszParameterMsgFilePaths == NULL) 
         pszParameterMsgFilePaths = szModulePathname;
      l = RegSetValueEx(hkey, TEXT("ParameterMessageFile"), 0, REG_EXPAND_SZ,  
         (PBYTE) pszParameterMsgFilePaths, chSIZEOFSTRING(pszParameterMsgFilePaths));
      if (l != NO_ERROR) __leave;

      if (dwCategoryCount > 0) {
         if (pszCategoryMsgFilePaths == NULL) 
            pszCategoryMsgFilePaths = szModulePathname;
         l = RegSetValueEx(hkey, TEXT("CategoryMessageFile"), 0, REG_EXPAND_SZ,   
            (PBYTE) pszCategoryMsgFilePaths, chSIZEOFSTRING(pszCategoryMsgFilePaths));
         if (l != NO_ERROR) __leave;

         l = RegSetValueEx(hkey, TEXT("CategoryCount"), 0, REG_DWORD, 
            (PBYTE) &dwCategoryCount, sizeof(dwCategoryCount));
         if (l != NO_ERROR) __leave;
      }
      fOk = TRUE;
   }
   __finally {
      if (hkey != NULL) RegCloseKey(hkey);
   }
   return(fOk);
}


//////////////////////////////////////////////////////////////////////////////


BOOL CEventLog::Uninstall() {

   // Install each service's event log
   TCHAR szSubKey[_MAX_PATH];
   wsprintf(szSubKey, 
      TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), 
      m_pszAppName);
   return(NO_ERROR == RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey));
}


//////////////////////////////////////////////////////////////////////////////


BOOL CEventLog::ReportEvent(WORD wType, WORD wCategory, DWORD dwEventID, 
   REPORTEVENTUSER reu, WORD wNumStrings, PCTSTR* pStrings, DWORD dwDataSize, 
   PVOID pvRawData) {

   BOOL fOk = TRUE;    // Assume success

   if (m_hEventLog == NULL) {
      // This is the first time that ReportEvent is being 
      // called, open the log first
      m_hEventLog = ::RegisterEventSource(NULL, m_pszAppName);
   }
   if (m_hEventLog != NULL) {
      PSID psidUser = NULL;
      if (reu != REUSER_NOTAPPLICABLE) {
         HANDLE hToken;
         if (REUSER_SERVICE == reu)
            fOk = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
         else 
            fOk = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
         if (fOk) {
            BYTE bTokenUser[1024];
            PTOKEN_USER ptuUserSID = (PTOKEN_USER) bTokenUser;
            DWORD dwReturnLength;
            GetTokenInformation(hToken, TokenUser, ptuUserSID, 
               sizeof(bTokenUser), &dwReturnLength);  
            CloseHandle(hToken);
            psidUser = ptuUserSID->User.Sid;
         }
      }

      fOk = fOk && ::ReportEvent(m_hEventLog, wType, wCategory, dwEventID, 
         psidUser, wNumStrings, dwDataSize, pStrings, pvRawData);
   }
   return(fOk);
}

   
///////////////////////////////////////////////////////////////////////////////


#endif   // SERVICECTRL_IMPL


///////////////////////////////// End of File /////////////////////////////////
