/******************************************************************************
Module:  Gate.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class creates a normally open gate that only one thread can
         pass through at a time.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"              /* See Appendix A. */


///////////////////////////////////////////////////////////////////////////////


class CGate {
public:
   CGate(BOOL fInitiallyUp = TRUE, PCTSTR pszName = NULL) { 
      m_hevt = ::CreateEvent(NULL, FALSE, fInitiallyUp, pszName); 
   }

   ~CGate() { 
      ::CloseHandle(m_hevt); 
   }

   DWORD WaitToEnterGate(DWORD dwTimeout = INFINITE, BOOL fAlertable = FALSE) {
      return(::WaitForSingleObjectEx(m_hevt, dwTimeout, fAlertable)); 
   }
   
   VOID LiftGate() { ::SetEvent(m_hevt); }

private:
    HANDLE m_hevt;
};


///////////////////////////////// End of File /////////////////////////////////
