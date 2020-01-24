/******************************************************************************
Module:  SecInfo.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class wraps the ISecurityInformation interface, which is used 
         in calls to the EditSecurity function.
         See Appendix B.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"              // See Appendix A.
#include <aclapi.h>
#include <aclui.h>


///////////////////////////////////////////////////////////////////////////////


class CSecInfo: public ISecurityInformation {
public:
   CSecInfo() { m_nRef = 1; m_fMod = FALSE; }
   BOOL IsModified() { return(m_fMod); }

protected:
   virtual ~CSecInfo() {}

protected:
   void Modified() { m_fMod = TRUE; }
   static GUID m_guidNULL;
   static SI_ACCESS m_siAccessAllRights[];

private:
   ULONG m_nRef; 
   BOOL  m_fMod;    

public:
   HRESULT WINAPI QueryInterface(REFIID riid, PVOID* ppvObj);
   ULONG WINAPI AddRef();
   ULONG WINAPI Release();
   HRESULT UseStandardAccessRights(const GUID* pguidObjectType, DWORD dwFlags,
      PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess);

protected:
   HRESULT WINAPI GetObjectInformation(PSI_OBJECT_INFO pObjectInfo) = 0;
   HRESULT WINAPI GetSecurity(SECURITY_INFORMATION RequestedInformation,
      PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) = 0;
   HRESULT WINAPI SetSecurity(SECURITY_INFORMATION SecurityInformation,
      PSECURITY_DESCRIPTOR pSecurityDescriptor) = 0;
   HRESULT WINAPI GetAccessRights(const GUID* pguidObjectType,
      DWORD dwFlags, // SI_EDIT_AUDITS, SI_EDIT_PROPERTIES
      PSI_ACCESS *ppAccess, ULONG *pcAccesses, ULONG *piDefaultAccess) = 0;
   HRESULT WINAPI MapGeneric(const GUID *pguidObjectType,
      UCHAR *pAceFlags, ACCESS_MASK *pMask) = 0;
   HRESULT WINAPI GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, 
      ULONG *pcInheritTypes);
   HRESULT WINAPI PropertySheetPageCallback(HWND hwnd, UINT uMsg, 
      SI_PAGE_TYPE uPage);

   PSECURITY_DESCRIPTOR LocalAllocSDCopy(PSECURITY_DESCRIPTOR psd);
};


///////////////////////////////////////////////////////////////////////////////


#ifdef SECINFO_IMPL


///////////////////////////////////////////////////////////////////////////////


GUID CSecInfo::m_guidNULL = GUID_NULL;

#define RIGHT(code, text, fGeneral, fSpecific)     \
   { &m_guidNULL, code, L ## text,                 \
      (0 | (fGeneral ? SI_ACCESS_GENERAL : 0) |    \
           (fSpecific ? SI_ACCESS_SPECIFIC : 0)) }

SI_ACCESS CSecInfo::m_siAccessAllRights[] = {
   RIGHT(DELETE,                   "DELETE",                   TRUE,  FALSE),
   RIGHT(READ_CONTROL,             "READ_CONTROL",             TRUE,  FALSE),
   RIGHT(WRITE_DAC,                "WRITE_DAC",                TRUE,  FALSE),
   RIGHT(WRITE_OWNER,              "WRITE_OWNER",              TRUE,  FALSE),
   RIGHT(SYNCHRONIZE,              "SYNCHRONIZE",              TRUE,  FALSE),

   RIGHT(STANDARD_RIGHTS_REQUIRED, "STANDARD_RIGHTS_REQUIRED", TRUE,  FALSE),
   RIGHT(STANDARD_RIGHTS_READ,     "STANDARD_RIGHTS_READ",     TRUE,  FALSE),
   RIGHT(STANDARD_RIGHTS_WRITE,    "STANDARD_RIGHTS_WRITE",    TRUE,  FALSE),
   RIGHT(STANDARD_RIGHTS_EXECUTE,  "STANDARD_RIGHTS_EXECUTE",  TRUE,  FALSE),
   RIGHT(STANDARD_RIGHTS_ALL,      "STANDARD_RIGHTS_ALL",      TRUE,  FALSE),
   RIGHT(SPECIFIC_RIGHTS_ALL,      "SPECIFIC_RIGHTS_ALL",      TRUE,  FALSE),

   RIGHT(ACCESS_SYSTEM_SECURITY,  "ACCESS_SYSTEM_SECURITY",    TRUE,  FALSE),
   RIGHT(MAXIMUM_ALLOWED,         "MAXIMUM_ALLOWED",           TRUE,  FALSE),
};


///////////////////////////////////////////////////////////////////////////////


PSECURITY_DESCRIPTOR CSecInfo::LocalAllocSDCopy(PSECURITY_DESCRIPTOR pSD) {
   DWORD dwSize = 0;
   SECURITY_DESCRIPTOR_CONTROL sdc;
   PSECURITY_DESCRIPTOR pSDNew = NULL;
   DWORD	dwVersion;
    
   __try {
      if (pSD == NULL) __leave; 
      
      if (!GetSecurityDescriptorControl(pSD, &sdc, &dwVersion)) __leave;
      if ((sdc & SE_SELF_RELATIVE) != 0) {
         dwSize = GetSecurityDescriptorLength(pSD);
         if (dwSize == 0) __leave;

         pSDNew = LocalAlloc(LPTR, dwSize);
         if (pSDNew == NULL) __leave; 
         CopyMemory(pSDNew, pSD, dwSize);
      } else {
         if (MakeSelfRelativeSD(pSD, NULL, &dwSize)) __leave;
         else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) __leave;
         pSDNew = LocalAlloc(LPTR, dwSize);
         if (pSDNew == NULL) __leave;
         if (!MakeSelfRelativeSD(pSD, pSDNew, &dwSize)) {
            LocalFree(pSDNew);
            pSDNew = NULL;
         }		
      }
   }
   __finally {
   } 
   return(pSDNew);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecInfo::QueryInterface(REFIID riid, PVOID* ppvObj) {
   HRESULT hr = E_NOINTERFACE;
   if ((riid == IID_ISecurityInformation) || (riid == IID_IUnknown)) {
      *ppvObj = this;
      AddRef();
      hr = S_OK; 
   }
   return(hr); 
}


///////////////////////////////////////////////////////////////////////////////


ULONG CSecInfo::AddRef() {
   m_nRef++;
   return(m_nRef); 
}


///////////////////////////////////////////////////////////////////////////////


ULONG CSecInfo::Release() {
   ULONG nRef = --m_nRef;
   if (m_nRef == 0) 
      delete this;
   return(nRef);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecInfo::UseStandardAccessRights(const GUID* pguidObjectType,
   DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, 
   ULONG* piDefaultAccess) {

   *ppAccess = m_siAccessAllRights;
   *pcAccesses = chDIMOF(m_siAccessAllRights);
   *piDefaultAccess = 0;
   return(S_OK); 
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecInfo::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, 
   ULONG* pcInheritTypes) {

   *ppInheritTypes = NULL; 
   *pcInheritTypes = 0;
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecInfo::PropertySheetPageCallback(HWND hwnd, UINT uMsg, 
   SI_PAGE_TYPE uPage) {
   
   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


#pragma comment(lib, "ACLUI.lib")   // Force linking against this library


///////////////////////////////////////////////////////////////////////////////


#endif   // SECINFO_IMPL


///////////////////////////////// End of File /////////////////////////////////
