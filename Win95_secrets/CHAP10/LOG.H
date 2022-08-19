//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: LOG.H
//==================================
void __stdcall LogCall(PSTR pszName, PBYTE pParams, PDWORD pFrame);
void LogReturn(PSTR pszName, DWORD returnValue, DWORD level);
BOOL OpenLogFile(void);
BOOL CloseLogFile(void);
