//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: RETURN.H
//==================================
BOOL InitThreadReturnStack(void);
BOOL ShutdownThreadReturnStack(void);
BOOL InterceptFunctionReturn(PSTR pszName, PDWORD pFrame);

