/******************************************************************************
Module:  RegKey.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


/////////////////////////////////////////////////////////////////////////////


class CRegKey {
public:  // Constructors/destructor
   CRegKey() : m_hkey(NULL) { };

   CRegKey(BOOL fReadOnly, HKEY hkeyRoot, PCTSTR pszSubkey)
      : m_hkey(NULL) {
      (void) Open(fReadOnly, hkeyRoot, pszSubkey);
   }

   ~CRegKey() { Close(); }


public:  // Opening/closing registry subkey functions
   operator HKEY() const { return(m_hkey); }

   LONG Open(BOOL fReadOnly, HKEY hkeyRoot, PCTSTR pszSubkey) {
      Close();
      LONG lErr;
      if (fReadOnly) {
         lErr = RegOpenKeyEx(hkeyRoot, pszSubkey, 0, 
            KEY_QUERY_VALUE, &m_hkey); 
      } else {
         lErr = RegCreateKeyEx(hkeyRoot, pszSubkey, 0, NULL, 
            REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE, 
            NULL, &m_hkey, NULL);
      }
      return(lErr);
   }

   void Close() {
      if (m_hkey != NULL) { 
         RegCloseKey(m_hkey); 
         m_hkey = NULL;
      }
   }


public:  // Reading/writing registry value functions
   LONG GetDWORD(PCTSTR pszValName, PDWORD pdw) const {
      DWORD cbData = sizeof(pdw);
      return(GetVal(pszValName, (PBYTE) pdw, &cbData));
   }

   LONG SetDWORD(PCTSTR pszValName, DWORD dw) const {
      return(SetVal(pszValName, REG_DWORD, (BYTE*) &dw, sizeof(dw)));
   }


   LONG GetString(PCTSTR pszValName, PTSTR psz, int nMaxSize) const {
      DWORD cbData = nMaxSize;
      return(GetVal(pszValName, (PBYTE) psz, &cbData));
   }

   LONG SetString(PCTSTR pszValName, PCTSTR psz) const {
      return(SetVal(pszValName, REG_SZ, (PBYTE) psz, 
         sizeof(TCHAR) * (lstrlen(psz) + 1)));
   }

   
   LONG GetMultiString(PCTSTR pszValName, PTSTR psz, int nMaxSize) const {
      return(GetString(pszValName, psz, nMaxSize));
   }

   LONG SetMultiString(PCTSTR pszValName, PCTSTR psz) const {
      for (DWORD cch = 0; psz[cch] != 0; cch += 1 + lstrlen(&psz[cch])) ;
      return(SetVal(pszValName, REG_MULTI_SZ, (PBYTE) psz, 
         sizeof(TCHAR) * (cch + 1)));
   }

   
   LONG GetBinary(PCTSTR pszValName, PBYTE pb, PDWORD pcbData) const {
      return(GetVal(pszValName, pb, pcbData));
   }

   LONG SetBinary(PCTSTR pszValName, CONST BYTE* pb, int nSize) const {
      return(SetVal(pszValName, REG_BINARY, pb, nSize));
   }

   LONG GetSize(PCTSTR pszValName, PDWORD pdw) {
      return(GetVal(pszValName, NULL, pdw));
   }

private:
   LONG GetVal(PCTSTR pszValName, PBYTE pbData, PDWORD pcb) const {
      return(RegQueryValueEx(m_hkey, pszValName, NULL, NULL, pbData, pcb));
   }

   LONG SetVal(PCTSTR pszValName, DWORD dwType, CONST BYTE *pbData, DWORD cb) 
      const {
      return(RegSetValueEx(m_hkey, pszValName, 0, dwType, pbData, cb));
   }

private:
   HKEY  m_hkey;
};


//////////////////////////////// End of File //////////////////////////////////
