/******************************************************************************
Module:  PrintBuf.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class wraps allows sprintf-like operations while automatically
         growing the resulting data buffer.
         See Appendix B.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"                 // See Appendix A.
#include <StdIO.h>                     // For _vstprintf


///////////////////////////////////////////////////////////////////////////////


class CPrintBuf {
public:
   CPrintBuf(SIZE_T nMaxSizeInBytes = 64 * 1024); // 64KB is default
   virtual ~CPrintBuf();

   BOOL Print(PCTSTR pszFmt, ...);
   BOOL PrintError(DWORD dwError = GetLastError());
   operator PCTSTR() { return(m_pszBuffer); }
   void Clear();

private:
   LONG Filter(EXCEPTION_POINTERS* pep);

private:
   int   m_nMaxSizeInBytes;
   int   m_nCurSize;    // In number of characters
   PTSTR m_pszBuffer;
};


///////////////////////////////////////////////////////////////////////////////


#ifdef PRINTBUF_IMPL


///////////////////////////////////////////////////////////////////////////////


CPrintBuf::CPrintBuf(SIZE_T nMaxSizeInBytes) {

   // This constructor sets initial values of members, and reserves a block
   // of addresses of size nMaxSizeInBytes and commits a single page.
   m_nMaxSizeInBytes = nMaxSizeInBytes;
   m_nCurSize = 0;
   m_pszBuffer = (PTSTR) 
      VirtualAlloc(NULL, m_nMaxSizeInBytes, MEM_RESERVE, PAGE_READWRITE);
   chASSERT(m_pszBuffer != NULL);
   chVERIFY(VirtualAlloc(m_pszBuffer, 1, MEM_COMMIT, PAGE_READWRITE) != NULL);
}


///////////////////////////////////////////////////////////////////////////////


CPrintBuf::~CPrintBuf() {

   VirtualFree(m_pszBuffer, 0, MEM_RELEASE);
}


///////////////////////////////////////////////////////////////////////////////


void CPrintBuf::Clear() {

   VirtualFree(m_pszBuffer, m_nMaxSizeInBytes, MEM_DECOMMIT);
   chVERIFY(VirtualAlloc(m_pszBuffer, 1, MEM_COMMIT, PAGE_READWRITE) != NULL);
   m_nCurSize = 0;
}


///////////////////////////////////////////////////////////////////////////////


LONG CPrintBuf::Filter(EXCEPTION_POINTERS* pep) {

   LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;
   EXCEPTION_RECORD* per = pep->ExceptionRecord;
   __try {
      // Is exception is an access violation in the data buffer's region?
      if (per->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) 
         __leave;

      if (!chINRANGE(m_pszBuffer, (PVOID) per->ExceptionInformation[1], 
         ((PBYTE) m_pszBuffer) + m_nMaxSizeInBytes - 1)) {
         __leave;
      }

      // Attempt to commit storage to the region
      if (VirtualAlloc((PVOID) pep->ExceptionRecord->ExceptionInformation[1], 
         1, MEM_COMMIT, PAGE_READWRITE) == NULL) {
         __leave;
      }

      lDisposition = EXCEPTION_CONTINUE_EXECUTION;
   }
   __finally {
   }
   return(lDisposition);
}


///////////////////////////////////////////////////////////////////////////////


int CPrintBuf::Print(PCTSTR pszFmt , ...) {

   // This function appends text to the formatted print buffer.
   int nLength = -1; // Assume failure
   va_list arglist;
   va_start(arglist, pszFmt);   
   __try {
      // Append string to end of buffer
      nLength = _vstprintf(m_pszBuffer + m_nCurSize, pszFmt, arglist);
      if (nLength > 0) 
         m_nCurSize += nLength;
   }
   __except (Filter(GetExceptionInformation())) {
      chMB("CPrintBuf attempted to go over the maximum size.");
      DebugBreak();
   }
   va_end(arglist);
   return(nLength);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CPrintBuf::PrintError(DWORD dwErr) {

   // Append the last error string text to the buffer.
   PTSTR pszMsg = NULL;
   BOOL fOk = (0 != FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
      dwErr, 0, (PTSTR) &pszMsg, 0, NULL));
   fOk = fOk && (Print(TEXT("Error %d: %s"), dwErr, pszMsg) >= 0);
   if (pszMsg != NULL)
      LocalFree(pszMsg);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


#endif   // PRINTBUF_IMPL


///////////////////////////////// End of File /////////////////////////////////
