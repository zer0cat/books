/******************************************************************************
Module: RegWalk.h
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include <ShlWapi.h>


///////////////////////////////////////////////////////////////////////////////


class CRegWalk {
public:
   CRegWalk() {}
   virtual ~CRegWalk() {}
   BOOL Go(PCTSTR pszMachine, HKEY hkeyRoot, PCTSTR pszSubkey, BOOL fRecurse);
   enum REGWALKSTATUS { RWS_FULLSTOP, RWS_CONTINUE, RWS_RECURSE };

protected:
   virtual REGWALKSTATUS onSubkey(PCTSTR pszSubkey, int nDepth, 
      BOOL fRecurseRequested);
   virtual REGWALKSTATUS onValue(HKEY hkey, PCTSTR pszValue, int nDepth);

protected:
   HKEY   m_hkeyRootMachine;        // Root key on machine
   BOOL   m_fRecurse;               // Recurse into subkeys?
   int    m_nDepth;                 // Recurse depth
   TCHAR  m_szSubkeyPath[MAX_PATH]; // Subkey path

private:
   REGWALKSTATUS RegWalkRecurse();
   REGWALKSTATUS EnumValuesInSubkey();
};


///////////////////////////////////////////////////////////////////////////////


#ifdef REGWALK_IMPL


///////////////////////////////////////////////////////////////////////////////


#pragma comment(lib, "shlwapi")


///////////////////////////////////////////////////////////////////////////////


CRegWalk::REGWALKSTATUS CRegWalk::onSubkey(PCTSTR pszSubkey, int nDepth, 
   BOOL fRecurseRequested) { 

   return(fRecurseRequested ? RWS_RECURSE : RWS_CONTINUE);
}


CRegWalk::REGWALKSTATUS CRegWalk::onValue(HKEY hkey, PCTSTR pszValue, 
   int nDepth) {

   return(RWS_CONTINUE);
}


///////////////////////////////////////////////////////////////////////////////


CRegWalk::REGWALKSTATUS CRegWalk::EnumValuesInSubkey() {

   HKEY hkey = NULL;
   REGWALKSTATUS rws = RWS_CONTINUE;

   if (ERROR_SUCCESS == RegOpenKeyEx(m_hkeyRootMachine, m_szSubkeyPath, 0, 
      KEY_QUERY_VALUE, &hkey)) {

      for (int nIndex = 0; rws != RWS_FULLSTOP; nIndex++) {

         TCHAR szValueName[256]; // No value name exceeds 255 characters
         DWORD cbValueName = chDIMOF(szValueName);
         if (ERROR_SUCCESS != RegEnumValue(hkey, nIndex, 
            szValueName, &cbValueName, NULL, NULL, NULL, NULL))
            break;

         rws = onValue(hkey, szValueName, m_nDepth);
      }
      chVERIFY(RegCloseKey(hkey) == ERROR_SUCCESS);
   }
   return(rws);
}


///////////////////////////////////////////////////////////////////////////////


CRegWalk::REGWALKSTATUS CRegWalk::RegWalkRecurse() {

   // Report this Subkey
   REGWALKSTATUS rws = onSubkey(m_szSubkeyPath, ++m_nDepth, m_fRecurse);

   // Enumerate the values in this subkey?
   if (rws == RWS_RECURSE) rws = EnumValuesInSubkey();

   // Continue enumerating subkeys?
   if (rws != RWS_FULLSTOP) {

      HKEY hkey = NULL;
      if (ERROR_SUCCESS == RegOpenKeyEx(m_hkeyRootMachine, m_szSubkeyPath, 0, 
         KEY_ENUMERATE_SUB_KEYS, &hkey)) {

         for (int nIndex = 0; rws != RWS_FULLSTOP; nIndex++) {

            TCHAR szSubkeyName[256]; // No subkey name exceeds 255 characters
            DWORD cbSubkeyName = chDIMOF(szSubkeyName);
            if (ERROR_SUCCESS != RegEnumKeyEx(hkey, nIndex, 
               szSubkeyName, &cbSubkeyName, NULL, NULL, NULL, NULL))
               break;

            // Append the subkey to the path
            if (m_szSubkeyPath[0] != 0) StrCat(m_szSubkeyPath, TEXT("\\"));
            StrCat(m_szSubkeyPath, szSubkeyName);

            rws = RegWalkRecurse();
            
            // Truncate the last subkey from the path
            PTSTR p = StrRChr(m_szSubkeyPath, NULL, TEXT('\\'));
            if (p != NULL) *p = 0;
            else m_szSubkeyPath[0] = 0;
         }
         chVERIFY(RegCloseKey(hkey) == ERROR_SUCCESS);
      }
   }
   m_nDepth--;
   return(rws);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CRegWalk::Go(PCTSTR pszMachine, HKEY hkeyRoot, PCTSTR pszSubkey, 
   BOOL fRecurse) {

   // nDepth indicates how many levels from the top we are.
   m_nDepth = -1;
   m_fRecurse = fRecurse;
   m_hkeyRootMachine = NULL;
   
   REGWALKSTATUS rws = RWS_FULLSTOP;
   __try {
      if (ERROR_SUCCESS != 
         RegConnectRegistry(pszMachine, hkeyRoot, &m_hkeyRootMachine))
         __leave;

      lstrcpy(m_szSubkeyPath, pszSubkey);

      // Call the recursive function to walk the subkeys
      rws = RegWalkRecurse();
   }
   __finally {
      if (m_hkeyRootMachine != NULL)
         RegCloseKey(m_hkeyRootMachine);
   }
   return(rws != RWS_FULLSTOP);
}


///////////////////////////////////////////////////////////////////////////////


#endif   // REGWALK_IMPL


//////////////////////////////// End of File //////////////////////////////////
