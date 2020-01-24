/******************************************************************************
Module:  AutoBuf.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class manages an auto-sizing data buffer.
         See Appendix B.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"              // See Appendix A.


/////////////////// CAutoBuf Template C++ Class Description ///////////////////


/*
The CAutoBuf template C++ class implements type safe buffers that
automatically grow to meet the needs of your code.  Memory is also
automatically freed when the object is destroyed (typically when your
code goes out of frame and it is popped off of the stack).

Examples of use:

   // Create a buffer with no explicit data type, 
   // the buffer grown in increments of a byte
   CAutoBuf<PVOID> buf;

   // Create a buffer of TCHARs, 
   // the buffer grows in increments of sizeof(TCHAR)
   CAutoBuf<PTSTR, sizeof(TCHAR)> buf; 

   // Force the buffer to be 10 bytes big
   buf = 10;

*/


///////////////////////////////////////////////////////////////////////////////


// This class is only ever used as a base class of the CAutoBuf template class.
// The base class exists so that all instances of the template class share
// a single instance of the common code.
class CAutoBufBase {
public:
   UINT  Size() { return(* (PDWORD) PSize()); }
   UINT  Size(UINT uSize);
   
   PUINT PSize() { 
      AdjustBuffer(); 
      m_uNewSize = m_uCurrentSize; 
      return(&m_uNewSize); 
   }
   void  Free() { Reconstruct(); }
   
protected:
   CAutoBufBase(PBYTE *ppbData, int nMult) {
      m_nMult = nMult;
      m_ppbBuffer = ppbData; // Derived class holds address of buffer to allow
                             // debugger's Quick Watch to work with typed data.
      Reconstruct(TRUE);
   }

   virtual ~CAutoBufBase() { Free(); }

   void Reconstruct(BOOL fFirstTime = FALSE);

   PBYTE Buffer() { 
      AdjustBuffer(); 
      return(*m_ppbBuffer); 
   }

private:
   void AdjustBuffer();   

private:
   PBYTE* m_ppbBuffer;    // Address of address of data buffer
   int    m_nMult;        // Multiplier (in bytes) used for buffer growth
   UINT   m_uNewSize;     // Requested buffer size (in m_nMult units)
   UINT   m_uCurrentSize; // Actual size (in m_nMult units)
};


///////////////////////////////////////////////////////////////////////////////


template <class TYPE, int MULT = 1> 
class CAutoBuf : private CAutoBufBase {
public:
   CAutoBuf() : CAutoBufBase((PBYTE*) &m_pData, MULT) {}
   void Free() { CAutoBufBase::Free(); }

public:
   operator TYPE*()  { return(Buffer()); }
   
   UINT operator=(UINT uSize) { return(CAutoBufBase::Size(uSize)); }
   operator UINT()   { return( Size()); }
   operator ULONG()  { return( Size()); }

   operator PUINT()  { return( PSize()); }
   operator PLONG()  { return((PLONG) PSize()); }
   operator PULONG() { return((PULONG) PSize()); }

   operator PBYTE()  { return((PBYTE) Buffer()); }
   operator PVOID()  { return((PVOID) Buffer()); }

   TYPE& operator[](int nIndex) { return(*(Buffer() + nIndex)); }

private:
   TYPE* Buffer() { return((TYPE*) CAutoBufBase::Buffer()); }

private:
   TYPE* m_pData;
};


///////////////////////////////////////////////////////////////////////////////


#define GROWUNTIL(fail, func)                        \
   do {                                              \
      if ((func) != (fail))                          \
         break;                                      \
   } while ((GetLastError() == ERROR_MORE_DATA) ||   \
            (GetLastError() == ERROR_INSUFFICIENT_BUFFER));


///////////////////////////////////////////////////////////////////////////////


#ifdef AUTOBUF_IMPL


///////////////////////////////////////////////////////////////////////////////


void CAutoBufBase::Reconstruct(BOOL fFirstTime) {

   if (!fFirstTime) {
      if (*m_ppbBuffer != NULL)
         HeapFree(GetProcessHeap(), 0, *m_ppbBuffer);
   }

   *m_ppbBuffer = NULL; // Derived class doesn't point to a data buffer
   m_uNewSize = 0;      // Initially, buffer has no bytes in it
   m_uCurrentSize = 0;  // Initially, buffer has no bytes in it
}


///////////////////////////////////////////////////////////////////////////////


UINT CAutoBufBase::Size(UINT uSize) {

   // Set buffer to desired number of m_nMult bytes.
   if (uSize == 0) {
      Reconstruct();
   } else {
      m_uNewSize = uSize;
      AdjustBuffer();      
   }
   return(m_uNewSize);
}


///////////////////////////////////////////////////////////////////////////////


void CAutoBufBase::AdjustBuffer() {

   if (m_uCurrentSize < m_uNewSize) {

      // We're growing the buffer
      HANDLE hHeap = GetProcessHeap();

      if (*m_ppbBuffer != NULL) {
         // We already have a buffer, re-size it
         PBYTE pNew = (PBYTE) 
            HeapReAlloc(hHeap, 0, *m_ppbBuffer, m_uNewSize * m_nMult);
         if (pNew != NULL) {
            m_uCurrentSize = m_uNewSize;
            *m_ppbBuffer = pNew;
         } 
      } else {
         // We don't have a buffer, create new one.
         *m_ppbBuffer = (PBYTE) HeapAlloc(hHeap, 0, m_uNewSize * m_nMult);
         if (*m_ppbBuffer != NULL) 
            m_uCurrentSize = m_uNewSize;
      }
   }
}


///////////////////////////////////////////////////////////////////////////////


#endif   // AUTOBUF_IMPL


///////////////////////////////// End of File /////////////////////////////////
