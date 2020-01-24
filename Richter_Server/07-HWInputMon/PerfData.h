/******************************************************************************
Module:  PerfData.h
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"        // See Appendix A.
#include <StdDef.h>           // For offsetof
#include <WinPerf.h>          // Not included by "Windows.H"

#ifdef PERFDATA_IMPL
#define OPTEX_IMPL
#endif
#include "Optex.h"            // High-speed, cross-process, critical section

#include "RegKey.h"           // Wraps registry functions


///////////////////////////////////////////////////////////////////////////////


// I use this to declare type-safe values
#define DECLARE_VALUE(name) \
   struct name##__ { BYTE unused; }; typedef struct name##__ *name


///////////////////////////////////////////////////////////////////////////////


class CPerfData {
public:     // Static member functions for install/uninstall
   // Installs the performance DLL info into the registry
   static void Install(PCTSTR pszDllPathname);

   // Removes the performance DLL info from the registry
   static void Uninstall();

public:     // Static member function to activate counter information
   // Creates and initializes the shared counter data block
   static DWORD Activate(); // Returns last error

public:  
   // The following should not be used directly. They are public because
   // of the BEGIN_PERFITEM_MAP/PERFOBJ/PERFCTR/END_PERFITEM_MAP macros.

   // The types of record that can be in a PerfITEM map
   typedef enum { PIT_END, PIT_OBJECT, PIT_COUNTER } PERFITEMTYPE;

   // Data types to make the code more readable
   typedef int PIINDEX;
   DECLARE_VALUE(OBJORD);
   DECLARE_VALUE(CTRORD);
   DECLARE_VALUE(OBJID);
   DECLARE_VALUE(CTRID);
   DECLARE_VALUE(INSTID);

   // Each object/counter is identified with this information
   typedef struct _PERFITEM {
      PERFITEMTYPE pit;
      // The following fields apply to both PIT_OBJECT & PIT_COUNTER
      DWORD    dwId;
      PCTSTR   pszName;
      PCTSTR   pszHelp; 
      DWORD    dwDetailLevel; 

      // The following fields apply to PIT_OBJECT only
      CTRID    DefCounter;
      INSTID   MaxInstances;
      DWORD    cchMaxInstName;
      CTRORD   NumCounters;
      PIINDEX  IndexNextObj;
      DWORD    cbOffsetToNextObj;
      BOOL     fCollectThisObj;

      // The following fields apply to PIT_COUNTER only   
      DWORD    dwDefaultScale; 
      DWORD    dwCounterType;
   } PERFITEM, *PPERFITEM;

public:  // Public member functions
   // Constructs a CPerfData
   CPerfData(PCTSTR pszAppName, PPERFITEM pPerfItems);

   // Destructs a CPerfData
   ~CPerfData();

   // Functions to allow mutual exclusive access to counter data
   void LockCtrs()    const;
   BOOL TryLockCtrs() const;
   void UnlockCtrs()  const;

   // Adds an instance to an object (returns InstId)
   INSTID AddInstance(BOOL fIgnoreIfExists, OBJID ObjId, 
      LONG lUniqueId, OBJID ObjIdParent = 0, INSTID InstIdParent = 0);
   INSTID AddInstance(BOOL fIgnoreIfExists, OBJID ObjId, 
      PCTSTR pszInstName, OBJID ObjIdParent = 0, INSTID InstIdParent = 0);

   // Checks if an instance exists (returns InstId)
   INSTID  FindInstanceId(OBJID ObjId, LONG lUniqueID);
   INSTID  FindInstanceId(OBJID ObjId, PCTSTR pszInstName);

   // Removes an instance from an object
   void RemoveInstance(OBJID ObjId, INSTID InstId);

   // Returns 32-bit address of counter in shared data block
   LONG& GetCtr32(CTRID CtrId, INSTID InstId = 0) const;

   // Returns 64-bit address of counter in shared data block
   __int64& GetCtr64(CTRID CtrId, INSTID InstId = 0) const;

private: // For debugging
   BOOL IsValidObjOrd(OBJORD ObjOrd) const;
   BOOL IsValidInstId(OBJORD ObjOrd, INSTID InstId) const;

private: // Fuctions for install/remove
   void VerifyPerfItemTable();
   void InstallPerfData(PCTSTR pszDllPathname);
   void UninstallPerfData();
   void AppendRegStrings(CRegKey& regPerfLib, 
      BOOL fCounter, PDWORD pdwIndex);
   void RemoveRegStrings(CRegKey& regPerfLib, 
      BOOL fCounter, DWORD dwIndexLo, DWORD dwIndexHi);

private: // Data types used internally by the class   
   enum { kMaxCounterSize = sizeof(__int64) };
   enum { 
      PERF_MASK_SIZE_FLD      = 0x00000300,
      PERF_MASK_CTR_TYPE      = 0x00000C00,
      PERF_MASK_CTR_SUBTYPE   = 0x000F0000,
      PERF_MASK_TIME_BASE     = 0x00300000,
      PERF_MASK_CALC_MODS     = 0x0FC00000,
      PERF_MASK_DISPLAY_FLAGS = 0xF0000000
   };
   typedef struct {
      BOOL                      fSupportsInstances;
      long                      MaxInstances;
      PPERF_OBJECT_TYPE         pPOT;
      int                       cbPOT;
      PPERF_COUNTER_DEFINITION  pPCD;
      int                       cbPCD;
      PPERF_INSTANCE_DEFINITION pPID;
      int                       cbPID;
      PPERF_COUNTER_BLOCK       pPCB;
      int                       cbPCB;
      PWSTR                     pPIN;
      int                       cbPIN;
   } PERFMETRICS, *PPERFMETRICS;

private: // Functions to convert betwen value types
   DWORD     ActivatePerfData(); // Returns last error
   void      CalcPerfMetrics(OBJORD ObjOrd, INSTID InstId, 
      PPERFMETRICS pPM, PPERFITEM* ppPerfItem = NULL) const; 

   PIINDEX   CvtIdToPerfItemIndex(BOOL fObjectId, DWORD dwId) const;
   PIINDEX   CvtObjIdToPerfItemIndex(OBJID ObjId) const;
   OBJORD    CvtObjIdToObjOrd(OBJID ObjId) const;
   PIINDEX   CvtObjOrdToPerfItemIndex(OBJORD ObjOrd) const;
   PPERFITEM CvtObjOrdToPerfItem(OBJORD ObjOrd) const;
   PPERFITEM CvtObjIdToPerfItem(OBJID ObjId) const;
   PIINDEX   CvtCtrIdToPerfItemIndex(CTRID CtrId, int* pnCtrIndexInObj = NULL)
      const;
   PPERFITEM CvtCtrOrdToPerfItem(OBJORD ObjOrd, CTRORD CtrOrd) const;
   OBJORD    CvtCtrIdToObjOrd(CTRID CtrId, int* pnCtrIndexInObj = NULL) const;
   PPERFITEM CvtCtrIdToPerfItem(CTRID CtrId) const;

   PBYTE     GetCtr(CTRID CtrId, INSTID InstId) const;
   INSTID    FindFreeInstance(OBJORD ObjOrd) const;
   INSTID    AddInstance(OBJID ObjId, PCTSTR pszInstName, LONG lUniqueId, 
               OBJID ObjIdParent, INSTID InstIdParent);
   INSTID    FindInstanceId(OBJID ObjId, PCTSTR pszInstName, LONG lUniqueID);

#ifdef PERFDATA_COLLECT_SUPPORTED
public:  // Static function required to support PerfData collection
   static DWORD Collect(PWSTR pszValueName, PBYTE* ppbData, 
      PDWORD pcbTotalBytes, PDWORD pdwNumObjectTypes);

private: // Functions required to support PerfData collection
   void  DetermineObjsToCollect(OBJORD ObjOrd) const;
   DWORD CollectAllObjs(PWSTR pszValueName, PBYTE *ppbData, 
            PDWORD pcbTotalBytes, PDWORD pdwNumObjectTypes) const;
   DWORD CollectAnObj(OBJORD ObjOrd, PBYTE* ppbData) const;
   DWORD CalcBytesForPerfObj(OBJORD ObjOrd) const;
   int   HowManyInstancesInUse(OBJORD ObjOrd) const;
   int   CvtInstIdToInUseInstId(OBJORD ObjOrd, INSTID InstId) const;
#endif

private: // static member that points to the 1 instance of this class
   static CPerfData* sm_pPerfData;

private: // Internal data members to support the class
   PCTSTR    m_pszAppName;     // App name for registry
   DWORD     m_dwFirstCounter; // 1st object/counter index
   DWORD     m_dwFirstHelp;    // 1st object/counter help index
   DWORD     m_dwLastCounter;  // Last object/counter index
   DWORD     m_dwLastHelp;     // Last object/counter help index

   PPERFITEM m_pPerfItems;     // Array of object/counter structures
   PIINDEX   m_nNumPerfItems;  // Number of items in the array
   OBJORD    m_nNumObjects;    // Num objects in array

   HANDLE    m_hfm;            // File mapping for shared data
   PBYTE     m_pPerfData;      // Mapped view of shared data
   mutable   COptex m_Optex;   // Protects access to shared data
};


///////////////////////////////////////////////////////////////////////////////


inline void CPerfData::Install(PCTSTR pszDllPathname) {
   sm_pPerfData->InstallPerfData(pszDllPathname);
}

inline void CPerfData::Uninstall() {
   sm_pPerfData->UninstallPerfData();
}

inline DWORD CPerfData::Activate() { 
   return(sm_pPerfData->ActivatePerfData());
}

inline BOOL CPerfData::IsValidObjOrd(OBJORD ObjOrd) const { 
   return(chINRANGE(0, ObjOrd, m_nNumObjects - 1)); 
}

inline BOOL CPerfData::IsValidInstId(OBJORD ObjOrd, INSTID InstId) const {
   return(IsValidObjOrd(ObjOrd) && chINRANGE(0, InstId, 
      (CvtObjOrdToPerfItem(ObjOrd)->MaxInstances == 
         (INSTID) PERF_NO_INSTANCES) ? 0 : 
         CvtObjOrdToPerfItem(ObjOrd)->MaxInstances - 1)); 
}

inline void CPerfData::LockCtrs()    const { m_Optex.Enter(); }
inline BOOL CPerfData::TryLockCtrs() const { return(m_Optex.TryEnter()); }
inline void CPerfData::UnlockCtrs()  const { m_Optex.Leave(); }

inline CPerfData::INSTID CPerfData::AddInstance(BOOL fIgnoreIfExists, 
   OBJID ObjId, LONG lUniqueId, OBJID ObjIdParent, INSTID InstIdParent) {
   INSTID InstId = (INSTID) -1;
   if (fIgnoreIfExists) InstId = FindInstanceId(ObjId, lUniqueId);
   if (InstId == (INSTID) -1) 
      InstId = AddInstance(ObjId, NULL, lUniqueId, ObjIdParent, InstIdParent);
   return(InstId);
}

inline CPerfData::INSTID CPerfData::AddInstance(BOOL fIgnoreIfExists, 
   OBJID ObjId, PCTSTR pszInstName, OBJID ObjIdParent, INSTID InstIdParent) {
   INSTID InstId = (INSTID) -1;
   if (fIgnoreIfExists) InstId = FindInstanceId(ObjId, pszInstName);
   if (InstId == (INSTID) -1) 
      InstId = AddInstance(ObjId, pszInstName, PERF_NO_UNIQUE_ID, 
         ObjIdParent, InstIdParent);
   return(InstId);
}

inline CPerfData::INSTID CPerfData::FindInstanceId(OBJID ObjId,
   LONG lUniqueID) {
   return(FindInstanceId(ObjId, NULL, lUniqueID));
}

inline CPerfData::INSTID CPerfData::FindInstanceId(OBJID ObjId, 
   PCTSTR pszInstName) {
   return(FindInstanceId(ObjId, pszInstName, PERF_NO_UNIQUE_ID));
}

inline LONG& CPerfData::GetCtr32(CTRID CtrId, INSTID InstId) const {
   // Make sure the caller wants the right-size for this counter
   chASSERT((CvtCtrIdToPerfItem(CtrId)->dwCounterType & PERF_MASK_SIZE_FLD) 
      == PERF_SIZE_DWORD);
   return(* (PLONG) GetCtr(CtrId, InstId));
}

inline __int64& CPerfData::GetCtr64(CTRID CtrId, INSTID InstId) const {
   // Make sure the caller wants the right-size for this counter
   chASSERT((CvtCtrIdToPerfItem(CtrId)->dwCounterType & PERF_MASK_SIZE_FLD) 
      == PERF_SIZE_LARGE);
   return(* (__int64*) GetCtr(CtrId, InstId));
}

inline CPerfData::PIINDEX CPerfData::CvtObjIdToPerfItemIndex(OBJID ObjId) 
   const {
   return((CPerfData::PIINDEX) CvtIdToPerfItemIndex(TRUE, (DWORD) ObjId));
}

inline CPerfData::PPERFITEM CPerfData::CvtObjIdToPerfItem(OBJID ObjId) const {
   return(&m_pPerfItems[(int) CvtObjIdToPerfItemIndex(ObjId)]);
}

inline CPerfData::PPERFITEM CPerfData::CvtCtrIdToPerfItem(CTRID CtrId) const {
   return(&m_pPerfItems[(int) CvtCtrIdToPerfItemIndex(CtrId)]);
}


///////////////////////////////////////////////////////////////////////////////


#define PERFDATA_DEFINE_OBJECT(ObjSymbol, ObjVal)                        \
   extern CPerfData g_PerfData;                                          \
   const CPerfData::OBJID ObjSymbol = (CPerfData::OBJID) ObjVal;

#define PERFDATA_DEFINE_COUNTER(CtrSymbol, CtrVal)                       \
   const CPerfData::CTRID CtrSymbol = (CPerfData::CTRID) CtrVal;

#define PERFDATA_MAP_BEGIN()                                             \
static CPerfData::PERFITEM gs_PerfItems[] = {

#define PERFDATA_MAP_OBJ(dwId, pszName, pszHelp, dwDetailLevel,          \
   CtrIdDefCounter, lMaxInstances, cchMaxInstName)                       \
   { CPerfData::PIT_OBJECT, (DWORD) dwId, pszName, pszHelp,              \
      dwDetailLevel, CtrIdDefCounter, (CPerfData::INSTID) lMaxInstances, \
      cchMaxInstName, 0, 0, 0, FALSE, 0, 0 },

#define PERFDATA_MAP_CTR(dwId, pszName, pszHelp, dwDetailLevel,          \
   dwDefScale, dwType)                                                   \
   { CPerfData::PIT_COUNTER, (DWORD) dwId, pszName, pszHelp,             \
      dwDetailLevel, (CPerfData::CTRID) -1,                              \
      0, 0, 0, 0, 0, FALSE, dwDefScale, dwType },

#define PERFDATA_MAP_END(pszAppName)                                     \
   { CPerfData::PIT_END, (DWORD) -1, NULL, NULL,                         \
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }};                                   \
   CPerfData g_PerfData(TEXT(pszAppName), gs_PerfItems);


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_IMPL


///////////////////////////////////////////////////////////////////////////////


#include <malloc.h>


///////////////////////////////////////////////////////////////////////////////


// Call this function to force a breakpoint in PerfMon.exe
#ifdef _DEBUG
void ForceDebugBreak() {
   __try { 
      DebugBreak(); 
   }
   __except(UnhandledExceptionFilter(GetExceptionInformation())) {
   }
}
#else
#define ForceDebugBreak()
#endif


///////////////////////////////////////////////////////////////////////////////


// Helper function that finds a set of bytes in a memory block
PBYTE FindMemory(PBYTE pbBuf, DWORD cbBuf, 
   PBYTE pbSearchBytes, DWORD cbSearchBytes) {

   for (DWORD n = 0; n < (cbBuf - cbSearchBytes); n++) {
      if (pbBuf[n] == pbSearchBytes[0]) {
         for (DWORD x = 1; x < cbSearchBytes; x++) {
            if (pbBuf[n + x] != pbSearchBytes[x]) 
               break; // Not a match
         }
         if (x == cbSearchBytes) return(&pbBuf[n]); // Match!
      }
   }
   return(NULL);  // Not found at all
}


///////////////////////////////////////////////////////////////////////////////


// Address of the ONE instance of this class (See Collect)
CPerfData* CPerfData::sm_pPerfData = NULL;


///////////////////////////////////////////////////////////////////////////////


// Constructor: initializes member variables 
CPerfData::CPerfData(PCTSTR pszAppName, PPERFITEM pPerfItems) :
   m_pszAppName(pszAppName), 
   m_pPerfItems(pPerfItems), 
   m_nNumPerfItems(0),
   m_nNumObjects(0),
   m_pPerfData(NULL),
   m_hfm(NULL),
   m_dwFirstCounter(0),
   m_dwLastCounter(0),
   m_dwFirstHelp(1),
   m_dwLastHelp(1) {

   chASSERT(sm_pPerfData == NULL);  // Only one instance can exist
   sm_pPerfData = this;

   // Sanity check performance data object/counter map
   VerifyPerfItemTable();
      
   // The optex object grants EVERYONE GENERIC_ALL access.
   SECURITY_DESCRIPTOR sd;
   InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
   PSID psidEveryone;
   SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
   AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID, 0,
      0, 0, 0, 0, 0, 0, &psidEveryone);
   DWORD cbAcl = sizeof(ACL) + offsetof(ACCESS_ALLOWED_ACE, SidStart) 
      + GetLengthSid(psidEveryone);
   PACL pAcl = (PACL) _alloca(cbAcl); 
   InitializeAcl(pAcl, cbAcl, ACL_REVISION);
   SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE);
   AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidEveryone);
   m_Optex.Initialize(pszAppName, &sd, 4000);
}


///////////////////////////////////////////////////////////////////////////////


// Destructor
CPerfData::~CPerfData() {

   if (m_pPerfData != NULL) UnmapViewOfFile(m_pPerfData);
   if (m_hfm       != NULL) CloseHandle(m_hfm);
}


///////////////////////////////////////////////////////////////////////////////


// Installs performance object/counter map info into registry
void CPerfData::InstallPerfData(PCTSTR pszDllPathname) {

   chASSERT(pszDllPathname != NULL);

   // Read the last counter/help global values from the registry
   CRegKey regPerfLib, regApp;
   chVERIFY(ERROR_SUCCESS == 
      regPerfLib.Open(FALSE, HKEY_LOCAL_MACHINE, 
      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib")));
   DWORD dwLastCounter = 0, dwLastHelp = 1;
   regPerfLib.GetDWORD(TEXT("Last Counter"), &dwLastCounter); 
   regPerfLib.GetDWORD(TEXT("Last Help"),    &dwLastHelp); 

   // Read the first/last counter/help app values from the registry
   TCHAR szSubkey[512];
   wsprintf(szSubkey, 
      TEXT("SYSTEM\\CurrentControlSet\\Services\\%s\\Performance"), 
      m_pszAppName);
   chVERIFY(ERROR_SUCCESS == 
      regApp.Open(FALSE, HKEY_LOCAL_MACHINE, szSubkey));

   // Advance to the next counter / help
   m_dwFirstCounter = m_dwLastCounter = dwLastCounter + 2;
   m_dwFirstHelp    = m_dwLastHelp    = dwLastHelp + 2;

   // Install our counters into the registry
   AppendRegStrings(regPerfLib, TRUE,  &m_dwLastCounter);
   AppendRegStrings(regPerfLib, FALSE, &m_dwLastHelp);

   // Tell the registry where the next set of counter can go
   regPerfLib.SetDWORD(TEXT("Last Counter"), m_dwLastCounter); 
   regPerfLib.SetDWORD(TEXT("Last Help"), m_dwLastHelp); 

   // Save the installation results for our app
   regApp.SetString(TEXT("Library"),       pszDllPathname); 
   regApp.SetString(TEXT("Open"),          TEXT("PerfData_Open")); 
   regApp.SetString(TEXT("Close"),         TEXT("PerfData_Close")); 
   regApp.SetString(TEXT("Collect"),       TEXT("PerfData_Collect")); 
   regApp.SetDWORD (TEXT("First Counter"), m_dwFirstCounter); 
   regApp.SetDWORD (TEXT("First Help"),    m_dwFirstHelp); 
   regApp.SetDWORD (TEXT("Last Counter"),  m_dwLastCounter); 
   regApp.SetDWORD (TEXT("Last Help"),     m_dwLastHelp); 
}


///////////////////////////////////////////////////////////////////////////////


// Takes this app's performance info out of the registry
void CPerfData::UninstallPerfData() {

   // Read the last counter/help global values from the registry
   CRegKey regPerfLib, regApp;
   chVERIFY(ERROR_SUCCESS == 
      regPerfLib.Open(FALSE, HKEY_LOCAL_MACHINE, 
      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib")));

   DWORD dwLastCounter = 0, dwLastHelp = 1;
   regPerfLib.GetDWORD(TEXT("Last Counter"), &dwLastCounter); 
   regPerfLib.GetDWORD(TEXT("Last Help"),    &dwLastHelp); 

   // Read the first/last counter/help app values from the registry
   TCHAR szSubkey[512];
   wsprintf(szSubkey, 
      TEXT("SYSTEM\\CurrentControlSet\\Services\\%s\\Performance"), 
      m_pszAppName);

   chVERIFY(ERROR_SUCCESS == 
      regApp.Open(FALSE, HKEY_LOCAL_MACHINE, szSubkey));

   regApp.GetDWORD(TEXT("First Counter"), &m_dwFirstCounter); 
   regApp.GetDWORD(TEXT("First Help"),    &m_dwFirstHelp); 
   regApp.GetDWORD(TEXT("Last Counter"),  &m_dwLastCounter); 
   regApp.GetDWORD(TEXT("Last Help"),     &m_dwLastHelp); 

   // Our counters are in the registry, do sanity checks
   chASSERT((DWORD) m_nNumPerfItems == 
      (m_dwLastCounter - m_dwFirstCounter) / 2 + 1);
   chASSERT((DWORD) m_nNumPerfItems == 
      (m_dwLastHelp - m_dwFirstHelp) / 2 + 1);
   chASSERT((m_dwFirstCounter <= m_dwLastCounter) && 
      (m_dwFirstHelp <= m_dwLastHelp));
   chASSERT((m_dwLastCounter <= dwLastCounter) && 
      (m_dwLastHelp <= dwLastHelp));

   // Remove the strings from the registry
   RemoveRegStrings(regPerfLib, TRUE,  m_dwFirstCounter, m_dwLastCounter);
   RemoveRegStrings(regPerfLib, FALSE, m_dwFirstHelp,    m_dwLastHelp);

   // If these counters are the last in, truncate the end
   // otherwise we leave a gap in the performance counter numbers
   if (m_dwLastCounter == dwLastCounter) {
      dwLastCounter -= (int) m_nNumPerfItems * 2;
      dwLastHelp    -= (int) m_nNumPerfItems * 2;
      regPerfLib.SetDWORD(TEXT("Last Counter"), dwLastCounter); 
      regPerfLib.SetDWORD(TEXT("Last Help"), dwLastHelp); 
   }

   // Delete the app's registry subkey
   regApp.Close(); 
   ::RegDeleteKey(HKEY_LOCAL_MACHINE, szSubkey);
}


///////////////////////////////////////////////////////////////////////////////


// Appends our performance object/counter text/help in the registry
void CPerfData::AppendRegStrings(CRegKey& regPerfLib, 
   BOOL fCounter, PDWORD pdwIndex) {

   // Calculate the number of required for the stuff we want to add
   DWORD cbInc = 0;
   for (PIINDEX pii = 0; m_pPerfItems[pii].pit != PIT_END; pii++) {
      cbInc += (6 + 1) * sizeof(TCHAR); // 6 digit index plus 0-char
      cbInc += (lstrlen(fCounter ? m_pPerfItems[pii].pszName 
         : m_pPerfItems[pii].pszHelp) + 1) * sizeof(TCHAR);
   }

   // Allocate buffer big enough for original + new data & read it in
   TCHAR szLangId[10];
   wsprintf(szLangId, TEXT("%03d"), GetSystemDefaultLangID() & 0xff);
   CRegKey regLang(FALSE, regPerfLib, szLangId);
   PCTSTR pszValue = fCounter ? TEXT("Counter") : TEXT("Help");
   DWORD cbOrig = 0;
   regLang.GetSize(pszValue, &cbOrig);
   PTSTR psz = (PTSTR) _alloca(cbOrig + cbInc);
   regLang.GetMultiString(pszValue, psz, cbOrig); 

   // Append our new stuff to the end of the buffer
   // Subtract 1 for extra terminating 0 char
   PTSTR pszInc = (PTSTR) ((PBYTE) psz + cbOrig) - 1;  

   // Append our strings
   for (pii = 0; m_pPerfItems[pii].pit != PIT_END; pii++) {
      PCTSTR psz = fCounter 
         ? m_pPerfItems[pii].pszName : m_pPerfItems[pii].pszHelp;
      lstrcpy(pszInc += wsprintf(pszInc, TEXT("%d"), *pdwIndex) + 1, psz);
      pszInc += lstrlen(psz) + 1;
      *pdwIndex += 2;   // Always increment the number by 2
   }
   *pdwIndex -= 2;

   *pszInc++ = 0;   // Append extra terminating 0-char
   regLang.SetMultiString(pszValue, psz); 
}


///////////////////////////////////////////////////////////////////////////////


// Remove the object/counter strings from the registry
void CPerfData::RemoveRegStrings(CRegKey& regPerfLib, 
   BOOL fCounter, DWORD dwIndexLo, DWORD dwIndexHi) {

   // Allocate buffer big enough for original data & read it in
   CRegKey regLang(FALSE, regPerfLib, TEXT("009"));
   PCTSTR pszValue = fCounter ? TEXT("Counter") : TEXT("Help");
   DWORD cbOrig = 0;
   regLang.GetSize(pszValue, &cbOrig);
   PTSTR psz = (PTSTR) _alloca(cbOrig);
   regLang.GetMultiString(pszValue, psz, cbOrig); 

   // Find the bounds of what we want to remove
   TCHAR szNum[10] = { 0 };   // Look for \0#\0
   wsprintf(&szNum[1], TEXT("%d"), dwIndexLo);
   PBYTE pbLo = FindMemory((PBYTE) psz, cbOrig, 
      (PBYTE) szNum, sizeof(WCHAR) * (lstrlen(&szNum[1]) + 2));
   pbLo += sizeof(WCHAR);  // 1st byte of stuff to remove
   wsprintf(&szNum[1], TEXT("%d"), dwIndexHi);
   PBYTE pbHi = FindMemory((PBYTE) psz, cbOrig, 
      (PBYTE) szNum, sizeof(WCHAR) * (lstrlen(&szNum[1]) + 2));
   pbHi += sizeof(TCHAR);  // 1st byte of last counter to remove

   // Skip over number and text
   pbHi += (lstrlen((PCTSTR) pbHi) + 1) * sizeof(TCHAR); 
   pbHi += (lstrlen((PCTSTR) pbHi) + 1) * sizeof(TCHAR);

   // Shift the strings to keep down over the stuff to delete
   chASSERT(pbLo <= pbHi);
   MoveMemory(pbLo, pbHi, ((PBYTE) psz + cbOrig) - pbHi);

   // Save the updated string information
   regLang.SetMultiString(pszValue, psz); 
}


///////////////////////////////////////////////////////////////////////////////


// Creates the shared memory region for the performance information
DWORD CPerfData::ActivatePerfData() {

   // Read the first/last object/counter text/help values
   TCHAR szSubkey[512];
   wsprintf(szSubkey, 
      TEXT("SYSTEM\\CurrentControlSet\\Services\\%s\\Performance"), 
      m_pszAppName);

   CRegKey regApp;
   DWORD dwErr = regApp.Open(TRUE, HKEY_LOCAL_MACHINE, szSubkey);
   if (dwErr != ERROR_SUCCESS) {
      // Registry settings don't exist, was Install() ever called?
      return(dwErr);
   }
   
   regApp.GetDWORD(TEXT("First Counter"), &m_dwFirstCounter); 
   regApp.GetDWORD(TEXT("First Help"),    &m_dwFirstHelp); 
   regApp.GetDWORD(TEXT("Last Counter"),  &m_dwLastCounter); 
   regApp.GetDWORD(TEXT("Last Help"),     &m_dwLastHelp); 

   // Do sanity checks
   chASSERT((DWORD) m_nNumPerfItems == 
      (m_dwLastCounter - m_dwFirstCounter) / 2 + 1);
   chASSERT((DWORD) m_nNumPerfItems == 
      (m_dwLastHelp - m_dwFirstHelp) / 2 + 1);
   chASSERT(m_pPerfData == NULL);   // This can only be done once

   // Calculate how many bytes are needed for the shared memory
   DWORD cbBytesNeededForAllObjs = 0;
   for (OBJORD ObjOrd = 0; ObjOrd < (OBJORD) m_nNumObjects; ObjOrd++) {
      PERFMETRICS pm;
      PPERFITEM pPerfObj;
      CalcPerfMetrics(ObjOrd, 0, &pm, &pPerfObj);
      // No instances                   Instances
      // ---------------------------    ---------------------------
      // 1 PERF_OBJECT_TABLE            1 PERF_OBJECT_TABLE
      // 1 PERF_COUNTER_DEFINITION      1 PERF_COUNTER_DEFINITION 
      // 0 PERF_INSTANCE_DEFINITION     x PERF_INSTANCE_DEFINITIONs
      // 1 PERF_COUNTER_BLOCK           x PERF_COUNTER_BLOCKs
      // 0 instance names               x instance names
      pPerfObj->cbOffsetToNextObj = pm.cbPOT + pm.cbPCD + 
         pm.cbPID * (pm.fSupportsInstances ? pm.MaxInstances : 0) + 
         pm.cbPCB * (pm.fSupportsInstances ? pm.MaxInstances : 1) + 
         pm.cbPIN * (pm.fSupportsInstances ? pm.MaxInstances : 0);
      cbBytesNeededForAllObjs += pPerfObj->cbOffsetToNextObj;
   }

   // The file-mapping object grants EVERYONE GENERIC_ALL access.
   SECURITY_DESCRIPTOR sd;
   InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
   PSID psidEveryone;
   SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
   AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID, 0,
      0, 0, 0, 0, 0, 0, &psidEveryone);
   DWORD cbAcl = sizeof(ACL) + offsetof(ACCESS_ALLOWED_ACE, SidStart) 
      + GetLengthSid(psidEveryone);
   PACL pAcl = (PACL) _alloca(cbAcl); 
   InitializeAcl(pAcl, cbAcl, ACL_REVISION);
   SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE);
   AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidEveryone);
   SECURITY_ATTRIBUTES sa = { sizeof(sa), &sd, FALSE };

   // Attempt to allocate a MMF big enough for the data
   m_hfm = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 
      0, cbBytesNeededForAllObjs, m_pszAppName);
   // If dwErr = ERROR_ALREADY_EXISTS, another app has created the 
   // shared data area. This instance doesn't need to initialize it
   dwErr = GetLastError();
   if (m_hfm == NULL) return(dwErr);

   m_pPerfData = (PBYTE) MapViewOfFile(m_hfm, 
      FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
   if (dwErr == ERROR_ALREADY_EXISTS) return(dwErr);

   // This instance actually allocated the shared data, initialize it
   DWORD dwCounter = m_dwFirstCounter, dwHelp = m_dwFirstHelp;

   // Set the PERF_OBJECT_TYPEs for each object
   for (ObjOrd = 0; ObjOrd < m_nNumObjects; ObjOrd++) {
      PERFMETRICS pm;
      PPERFITEM pPerfObj;
      CalcPerfMetrics(ObjOrd, 0, &pm, &pPerfObj);
      // Set the PERF_OBJECT_TYPE members
      pm.pPOT->TotalByteLength      = 0;   // Set in Collect function
      pm.pPOT->DefinitionLength     = sizeof(PERF_OBJECT_TYPE) + 
         sizeof(PERF_COUNTER_DEFINITION) * 
         (int) pPerfObj->NumCounters; 
      pm.pPOT->HeaderLength         = sizeof(PERF_OBJECT_TYPE); 
      pm.pPOT->ObjectNameTitleIndex = dwCounter;
      pm.pPOT->ObjectNameTitle      = NULL; // Set by PerfMon
      pm.pPOT->ObjectHelpTitleIndex = dwHelp;
      pm.pPOT->ObjectHelpTitle      = NULL; // Set by PerfMon
      pm.pPOT->DetailLevel          = pPerfObj->dwDetailLevel; 
      pm.pPOT->NumCounters          = (DWORD) pPerfObj->NumCounters;

      if (pPerfObj->DefCounter == (CTRID) -1) {
         pm.pPOT->DefaultCounter = -1;
      } else {
         // If a default CTRID specified, convert it to index
         CvtCtrIdToPerfItemIndex(pPerfObj->DefCounter, 
            (int*) &pm.pPOT->DefaultCounter);
      }

      pm.pPOT->NumInstances      = (int) pPerfObj->MaxInstances;
      pm.pPOT->CodePage          = NULL; 
      pm.pPOT->PerfTime.QuadPart = 0; 
      pm.pPOT->PerfFreq.QuadPart = 0; 

      dwCounter += 2;
      dwHelp += 2;

      // Set the PERF_COUNTER_DEFINITIONs for each counter
      for (CTRORD CtrOrd = 0; CtrOrd < pPerfObj->NumCounters; CtrOrd++) {
         PPERFITEM pPerfCtr = CvtCtrOrdToPerfItem(ObjOrd, CtrOrd);
         PPERF_COUNTER_DEFINITION pPCD = &pm.pPCD[(int) CtrOrd];
         pPCD->ByteLength            = sizeof(PERF_COUNTER_DEFINITION); 
         pPCD->CounterNameTitleIndex = dwCounter;
         pPCD->CounterNameTitle      = NULL; 
         pPCD->CounterHelpTitleIndex = dwHelp;
         pPCD->CounterHelpTitle      = NULL;
         pPCD->DefaultScale          = pPerfCtr->dwDefaultScale;
         pPCD->DetailLevel           = pPerfCtr->dwDetailLevel;
         pPCD->CounterType           = pPerfCtr->dwCounterType;
         pPCD->CounterSize           = kMaxCounterSize; 
         pPCD->CounterOffset         = sizeof(PERF_COUNTER_BLOCK) + 
            kMaxCounterSize * (int) CtrOrd; 
         dwCounter += 2;
         dwHelp += 2;
      }

      // Set the PERF_COUNTER_BLOCKs for the 1 (or each) instance
      if (pPerfObj->MaxInstances == (INSTID) PERF_NO_INSTANCES) {
         pm.pPCB->ByteLength = pm.cbPCB;
      } else {
         for (INSTID InstId = 0; InstId < pPerfObj->MaxInstances; InstId++) {
            CalcPerfMetrics(ObjOrd, InstId, &pm);
            pm.pPCB->ByteLength = pm.cbPCB;
         }
      }
   }
   return(dwErr);
}


///////////////////////////////////////////////////////////////////////////////


// Finds an empty instance entry
CPerfData::INSTID CPerfData::FindFreeInstance(OBJORD ObjOrd) const {

   // Make sure the object is valid and supports instances
   chASSERT(IsValidObjOrd(ObjOrd));
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, 0, &pm);
   chASSERT(pm.fSupportsInstances);

   LockCtrs();
   // Find an unused instance entry
   INSTID InstId = 0;
   for (; InstId < (INSTID) pm.MaxInstances; InstId++) {
      CalcPerfMetrics(ObjOrd, InstId, &pm);
      if (pm.pPIN[0] == 0) break;
   }
   UnlockCtrs();

   // -1 means all instances are in use
   return((InstId == (INSTID) pm.MaxInstances) ? (INSTID) -1 : InstId);
}


///////////////////////////////////////////////////////////////////////////////


// Adds an instance to an object
CPerfData::INSTID CPerfData::AddInstance(OBJID ObjId, PCTSTR pszInstName, 
   LONG lUniqueID, OBJID ObjIdParent, INSTID InstIdParent) {

   // Make sure that instance has a valid identity
   chASSERT(
      ((pszInstName == NULL) && (lUniqueID != PERF_NO_UNIQUE_ID)) || 
      ((pszInstName != NULL) && (lUniqueID == PERF_NO_UNIQUE_ID)));

   // Make sure the object is valid and supports instances
   OBJORD ObjOrd = CvtObjIdToObjOrd(ObjId);
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, 0, &pm);
   chASSERT(pm.fSupportsInstances);

   // Find a place to put this instance
   INSTID InstId = FindFreeInstance(ObjOrd);
   if (InstId != (INSTID) -1) {
      CalcPerfMetrics(ObjOrd, InstId, &pm);
      // Store the Parent Object's ID/Instance ID and Unique ID.
      // The Collect function converts IDs to the appropriate values.
      pm.pPID->ParentObjectTitleIndex = (DWORD) ObjIdParent;
      pm.pPID->ParentObjectInstance = (DWORD) InstIdParent;
      pm.pPID->UniqueID = lUniqueID; 

      if (pszInstName == NULL) {    // Instance has a string name
         pm.pPIN[0] = 1;            // Mark instance as in use
         pm.pPID->NameOffset = 0;
         pm.pPID->NameLength = 0;
      } else {                      // Instance has no string name
         PWSTR pszInstNameW = NULL;
#ifndef UNICODE
         // Convert from ANSI to Unicode
         pszInstNameW = (PWSTR) 
            _alloca((lstrlenA(pszInstName) + 1) * sizeof(WCHAR));
         wsprintfW(pszInstNameW, L"%S", pszInstName);
#else
         pszInstNameW = (PWSTR) pszInstName;
#endif
         chASSERT(lstrlenW(pszInstNameW) < 
            (int) CvtObjOrdToPerfItem(ObjOrd)->cchMaxInstName);
         lstrcpyW(pm.pPIN, pszInstNameW);  // Mark instance as in use
         pm.pPID->NameLength = (lstrlenW(pszInstNameW) + 1) * sizeof(WCHAR);
         pm.pPID->NameOffset = pm.cbPID;
      }
   }
   return(InstId);
}


///////////////////////////////////////////////////////////////////////////////


// Checks to see if the specified instance already exists.
CPerfData::INSTID CPerfData::FindInstanceId(OBJID ObjId, 
   PCTSTR pszInstName, LONG lUniqueID) {

   // Make sure that instance has a valid identity
   chASSERT(
      ((pszInstName == NULL) && (lUniqueID != PERF_NO_UNIQUE_ID)) || 
      ((pszInstName != NULL) && (lUniqueID == PERF_NO_UNIQUE_ID)));

   // Make sure the object is valid and supports instances
   OBJORD ObjOrd = CvtObjIdToObjOrd(ObjId);
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, 0, &pm);
   chASSERT(pm.fSupportsInstances);

   PWSTR pszInstNameW = NULL;
   if (pszInstName != NULL) {
#ifndef UNICODE
      // Convert from ANSI to Unicode
      pszInstNameW = (PWSTR)
         _alloca((lstrlenA(pszInstName) + 1) * sizeof(WCHAR));
      wsprintfW(pszInstNameW, L"%S", pszInstName);
#else
      pszInstNameW = (PWSTR) pszInstName;
#endif
   }

   // Scan the existing instances for a match
   LockCtrs();
   for(INSTID InstId = 0; InstId < (INSTID) pm.MaxInstances; InstId++) {

      CalcPerfMetrics(ObjOrd, InstId, &pm);
      if (pm.pPIN[0] != 0) {  // This instance is in use
         if ((pszInstNameW != NULL) && (lstrcmpW(pm.pPIN, pszInstNameW) == 0))
            break;   // Found it!
         if ((pszInstNameW == NULL) && (pm.pPID->UniqueID == lUniqueID))
            break;   // Found it!
      }
   }
   UnlockCtrs();

   // -1 means not found
   return((InstId == (INSTID) pm.MaxInstances) ? (INSTID) -1 : InstId);
}


///////////////////////////////////////////////////////////////////////////////


// Removes an instance from an object
void CPerfData::RemoveInstance(OBJID ObjId, INSTID InstId) {

   LockCtrs();
   PERFMETRICS pm;
   CalcPerfMetrics(CvtObjIdToObjOrd(ObjId), InstId, &pm);
   chASSERT(pm.fSupportsInstances);
   chASSERT(pm.pPIN[0] != 0); // Can't remove an unassigned instance
   pm.pPIN[0] = 0;   // Mark instance as NOT in use
   UnlockCtrs();
}


///////////////////////////////////////////////////////////////////////////////

 
// Sanity checks the performance object/counter map  
void CPerfData::VerifyPerfItemTable() {

   PIINDEX piiCrntObj = (PIINDEX) -1;  // Object being processed

   // Loop through all entries in the object/counter map
   for (PIINDEX pii = 0; m_pPerfItems[pii].pit != PIT_END; pii++) {
      chASSERT(m_pPerfItems[pii].dwId != 0); // 0 is an invalid ID
      m_nNumPerfItems++;

      switch (m_pPerfItems[pii].pit) {
      case PIT_END:
         // Make sure the current object is in a good state
         // i.e., object has at least one counter in it
         chASSERT(!((piiCrntObj != -1) && 
            (m_pPerfItems[piiCrntObj].NumCounters < (CTRORD) 1)));
         break;

      case PIT_OBJECT:     // We found a new object
         // Every Object ID in the table must be unique
         chASSERT(CvtObjIdToPerfItemIndex(
            (OBJID) m_pPerfItems[pii].dwId) == pii);

         // Sanity check its parameters
         chASSERT((m_pPerfItems[pii].DefCounter == (CTRID) -1) ||
            (CvtCtrIdToPerfItemIndex(m_pPerfItems[pii].DefCounter) != -1));
         chASSERT((m_pPerfItems[pii].MaxInstances == 
            (INSTID) PERF_NO_INSTANCES) || 
            (m_pPerfItems[pii].MaxInstances > 0));
         chASSERT(
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_NOVICE)   || 
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_ADVANCED) || 
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_EXPERT)   ||
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_WIZARD));

         m_nNumObjects++;

         // Finish up the object that we were just working on
         if (piiCrntObj != -1) {
            // Make sure the current object is in a good state
            // i.e., object has at least one counter in it
            chASSERT(m_pPerfItems[piiCrntObj].NumCounters > 0);

            // The previous object must point to the current object
            m_pPerfItems[piiCrntObj].IndexNextObj = pii; 
         }

         piiCrntObj = pii; // Save new current object
         m_pPerfItems[piiCrntObj].NumCounters = 0;

         // Assume that this is the last object in the list
         m_pPerfItems[piiCrntObj].IndexNextObj = -1; 
         break;

      case PIT_COUNTER:
         chASSERT(pii != 0); // First entry in map must be PIT_OBJECT

         // Every Counter ID in the table must be unique
         chASSERT(CvtCtrIdToPerfItemIndex(
            (CTRID) m_pPerfItems[pii].dwId) == pii); 

         // Sanity check its parameters
         chASSERT(
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_NOVICE)   || 
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_ADVANCED) || 
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_EXPERT)   ||
            (m_pPerfItems[pii].dwDetailLevel == PERF_DETAIL_WIZARD));

         m_pPerfItems[piiCrntObj].NumCounters++;
         break;
      }
   }
}


///////////////////////////////////////////////////////////////////////////////


// Calculates an object's performance info (addresses and sizes)
void CPerfData::CalcPerfMetrics(OBJORD ObjOrd, INSTID InstId, 
   PPERFMETRICS pPM, PPERFITEM* ppPerfItem) const {

   chASSERT(IsValidInstId(ObjOrd, InstId));

   PPERFITEM pPerfItem = CvtObjOrdToPerfItem(ObjOrd);
   if (ppPerfItem != NULL) *ppPerfItem = pPerfItem;

   ZeroMemory(pPM, sizeof(*pPM));
   pPM->fSupportsInstances = 
      (pPerfItem->MaxInstances != (INSTID) PERF_NO_INSTANCES);
   pPM->MaxInstances = (long) pPerfItem->MaxInstances;

   // Find the start of this object's performance data
   DWORD cb = 0;
   PIINDEX pii = 0;
   for (; 0 != ObjOrd--; pii = m_pPerfItems[pii].IndexNextObj) 
      cb += m_pPerfItems[pii].cbOffsetToNextObj;

   // This object's PERF_OBJECT_TYPE structure & size
   pPM->pPOT = (PPERF_OBJECT_TYPE) (m_pPerfData + cb);
   pPM->cbPOT = sizeof(PERF_OBJECT_TYPE);

   // PERF_COUNTER_DEFINITIONs follow PERF_OBJECT_TYPE
   pPM->pPCD = (PPERF_COUNTER_DEFINITION) (&pPM->pPOT[1]);
   pPM->cbPCD = (int) pPerfItem->NumCounters * 
      sizeof(PERF_COUNTER_DEFINITION);

   if (pPerfItem->MaxInstances != (INSTID) PERF_NO_INSTANCES) {
      long  MaxInstances   = (long) pPerfItem->MaxInstances;
      PBYTE pbEndOfCtrDefs = (PBYTE) pPM->pPCD + pPM->cbPCD;

      // PERF_INSTANCE_DEFINITIONs follow PERF_COUNTER_DEFINITIONs
      pPM->pPID = (PPERF_INSTANCE_DEFINITION) pbEndOfCtrDefs;
      pPM->pPID = &pPM->pPID[(int) InstId]; 
      pPM->cbPID = sizeof(PERF_INSTANCE_DEFINITION);

      // PERF_COUNTER_BLOCKs follow PERF_INSTANCE_DEFINITIONs
      pPM->pPCB = (PPERF_COUNTER_BLOCK) (pbEndOfCtrDefs + 
         sizeof(PERF_INSTANCE_DEFINITION) * MaxInstances);
      pPM->cbPCB = sizeof(PERF_COUNTER_BLOCK) + 
         (long) pPerfItem->NumCounters * kMaxCounterSize;
      pPM->pPCB = (PPERF_COUNTER_BLOCK) ((PBYTE) pPM->pPCB + 
         (int) InstId * pPM->cbPCB);

      // Instance names follow PERF_COUNTER_BLOCKs
      pPM->pPIN = (PWSTR) (pbEndOfCtrDefs + 
         (sizeof(PERF_INSTANCE_DEFINITION) + pPM->cbPCB) * 
         MaxInstances);
      pPM->cbPIN = pPerfItem->cchMaxInstName * sizeof(WCHAR);
      pPM->pPIN += pPerfItem->cchMaxInstName * (int) InstId;

   } else {
      // PERF_COUNTER_BLOCK follows PERF_COUNTER_DEFINITION
      pPM->pPCB = (PPERF_COUNTER_BLOCK) ((PBYTE) pPM->pPCD + pPM->cbPCD);
      pPM->cbPCB = sizeof(PERF_COUNTER_BLOCK) + 
         (int) pPerfItem->NumCounters * kMaxCounterSize;
   }
}


///////////////////////////////////////////////////////////////////////////////


// Returns the address of a counter in the shared data block
PBYTE CPerfData::GetCtr(CTRID CtrId, INSTID InstId) const {

   int nCtrNum;
   OBJORD ObjOrd = CvtCtrIdToObjOrd(CtrId, &nCtrNum);
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, InstId, &pm);
   // NOTE: for convenience, all counters are 64-bit values
   return((PBYTE) pm.pPCB + sizeof(PERF_COUNTER_BLOCK) + 
      nCtrNum * kMaxCounterSize);
}


///////////////////////////////////////////////////////////////////////////////


// Converts an object/counter ID to its map index
CPerfData::PIINDEX CPerfData::CvtIdToPerfItemIndex(
   BOOL fObjectId, DWORD dwId) const {

   for (PIINDEX pii = 0; m_pPerfItems[pii].pit != PIT_END; pii++) {

      if (fObjectId && (m_pPerfItems[pii].pit == PIT_OBJECT) && 
          (m_pPerfItems[pii].dwId == dwId)) 
         return(pii);

      if (!fObjectId && (m_pPerfItems[pii].pit == PIT_COUNTER) && 
          (m_pPerfItems[pii].dwId == dwId)) 
         return(pii);
   }
   return(-1); // Not found
}


///////////////////////////////////////////////////////////////////////////////


// Converts an object ID to its map ordinal
CPerfData::OBJORD CPerfData::CvtObjIdToObjOrd(OBJID ObjId) const {

   for (OBJORD ObjOrd = 0; ObjOrd < m_nNumObjects; ObjOrd++) {
      if (CvtObjOrdToPerfItem(ObjOrd)->dwId == (DWORD) ObjId)
         return(ObjOrd);
   }
   chASSERTFAIL(__FILE__, __LINE__, "Object ID Not in list");
   return((OBJORD) -1);
}


///////////////////////////////////////////////////////////////////////////////


// Converts an objeict ordinal to its map index
CPerfData::PIINDEX CPerfData::CvtObjOrdToPerfItemIndex(OBJORD ObjOrd) const {

   chASSERT(IsValidObjOrd(ObjOrd));
   PIINDEX pii = 0;
   for (; 0 != ObjOrd--; pii = m_pPerfItems[pii].IndexNextObj) ;   
   return(pii);
}


///////////////////////////////////////////////////////////////////////////////


// Converts an object's ordinal to the address of its map info
CPerfData::PPERFITEM CPerfData::CvtObjOrdToPerfItem(OBJORD ObjOrd) const {

   return(&m_pPerfItems[CvtObjOrdToPerfItemIndex(ObjOrd)]);
}


///////////////////////////////////////////////////////////////////////////////


// Converts a counter's ID to its map index
CPerfData::PIINDEX CPerfData::CvtCtrIdToPerfItemIndex(
   CTRID CtrId, int* pnCtrIndexInObj) const {

   int nCtrIndexInObj = 0;
   PIINDEX pii = CvtIdToPerfItemIndex(FALSE, (DWORD) CtrId);
   if (pii != -1) {
      PIINDEX piiT = pii;
      while (m_pPerfItems[--piiT].pit != PIT_OBJECT) 
         nCtrIndexInObj++;
   }
   if (pnCtrIndexInObj != NULL) 
      *pnCtrIndexInObj = nCtrIndexInObj;
   return(pii); // -1 if not found
}


///////////////////////////////////////////////////////////////////////////////


// Converts a counter's ordinal to the address of its map info
CPerfData::PPERFITEM CPerfData::CvtCtrOrdToPerfItem(
   OBJORD ObjOrd, CTRORD CtrOrd) const {

   PPERFITEM pPerfItem = CvtObjOrdToPerfItem(ObjOrd);
   return(&pPerfItem[(int) CtrOrd + 1]);
}


///////////////////////////////////////////////////////////////////////////////


// Converts a counter's ID its owning object's ordinal
CPerfData::OBJORD  CPerfData::CvtCtrIdToObjOrd(
   CTRID CtrId, int* pnCtrIndexInObj) const {

   PIINDEX pii = CvtCtrIdToPerfItemIndex(CtrId, pnCtrIndexInObj);
   OBJORD ObjOrd = 0;
   while (--pii > 0) {
      if (m_pPerfItems[pii].pit == PIT_OBJECT) ObjOrd++;
   }
   return(ObjOrd);
}


///////////////////////////////////////////////////////////////////////////////


// The following functions are only necessary in the collection DLL
// They are not needed by an app that simply updates the counters
#ifdef PERFDATA_COLLECT_SUPPORTED


extern "C" {
DWORD __declspec(dllexport) WINAPI PerfData_Open(PWSTR pDevNames) {
   // Create/open the shared memory containing the performance info
   CPerfData::Activate();
   return(ERROR_SUCCESS);
}


DWORD __declspec(dllexport) WINAPI PerfData_Close(void) {
   // Nothing to do here
   return(ERROR_SUCCESS);
}


DWORD __declspec(dllexport) WINAPI PerfData_Collect(PWSTR pszValueName, 
   PVOID* ppData, PDWORD pcbTotalBytes, PDWORD pNumObjectTypes) {

   // Call the class's static Collect function to populate the passed
   // memory block with our performance info
   return(CPerfData::Collect(pszValueName, (PBYTE*) ppData, 
      pcbTotalBytes, pNumObjectTypes));
}
}  // extern "C"


// Export these 3 functions as the interface to our Performance Data
#pragma comment(linker, "/export:PerfData_Open=_PerfData_Open@4")
#pragma comment(linker, "/export:PerfData_Close=_PerfData_Close@0")
#pragma comment(linker, "/export:PerfData_Collect=_PerfData_Collect@16")


#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED


// This is a help class used to return only the requested objects
class CWhichCtrs {
public:
   CWhichCtrs(LPCWSTR pszObjNums = NULL);
   ~CWhichCtrs();
   BOOL IsNumInList(int nNum);

private:
   LPWSTR m_pszObjNums;
};


CWhichCtrs::CWhichCtrs(LPCWSTR pszObjNums) {

   // Save the list of requested object numbers
   if ((lstrcmpiW(L"Global", pszObjNums) == 0)) {
      m_pszObjNums = NULL;
   } else {
      m_pszObjNums = (LPWSTR) HeapAlloc(GetProcessHeap(), 0, 
         (lstrlenW(pszObjNums) + 3) * sizeof(WCHAR));
      // Put spaces around all the numbers
      wsprintfW(m_pszObjNums, L" %s ", pszObjNums);
   }
}


CWhichCtrs::~CWhichCtrs() {

   if (m_pszObjNums != NULL) 
      HeapFree(GetProcessHeap(), 0, m_pszObjNums);
}


BOOL CWhichCtrs::IsNumInList(int nNum) {

   BOOL fIsNumInList = TRUE;
   if (m_pszObjNums != NULL) {
      // Put spaces around this number and see if it's in the list
      WCHAR szNum[10];
      wsprintfW(szNum, L" %d ", nNum);
      fIsNumInList = (wcsstr(m_pszObjNums, szNum) != NULL);
   }
   return(fIsNumInList);
}


#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
// Determine which parent objects need to be collected
void CPerfData::DetermineObjsToCollect(OBJORD ObjOrd) const {

   PPERFITEM pPerfItem = CvtObjOrdToPerfItem(ObjOrd);

   // Assume this object is being collected
   chASSERT(pPerfItem->fCollectThisObj);
   
   if (pPerfItem->MaxInstances != (INSTID) PERF_NO_INSTANCES) {
      // If this counter supports instances, collect the 
      // counters that any instances refer to (recursively). 
      INSTID InstId = 0;
      for (; InstId < (INSTID) pPerfItem->MaxInstances; InstId++) {
         PERFMETRICS pm;
         CalcPerfMetrics(ObjOrd, InstId, &pm);
         if ((pm.pPIN[0] != 0) && (pm.pPID->ParentObjectTitleIndex != 0)) {
            // Instance is in use and refers to a parent object
            // Collect the parent object
            PPERFITEM pPerfItemParent = CvtObjIdToPerfItem((OBJID) 
               pm.pPID->ParentObjectTitleIndex);
            if (pPerfItemParent->fCollectThisObj == FALSE) {
               pPerfItemParent->fCollectThisObj = TRUE;
               DetermineObjsToCollect(CvtObjIdToObjOrd(
                  (OBJID) pm.pPID->ParentObjectTitleIndex));
            }
         }
      }
   }
}
#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
DWORD CPerfData::Collect(PWSTR pValueName, PBYTE* ppbData, 
   PDWORD pcbTotalBytes, PDWORD pNumObjectTypes) {

   DWORD dwErr = ERROR_SUCCESS;  // Assume success
   PBYTE ppbOriginalStartOfBuffer = *ppbData;

   // Wrap everything inside an SEH frame so that we NEVER bring
   // down an app that is trying to collect our performance info
   __try {
      // While we do all this work, lock out other threads so that
      // our data structures do not become corrupted.
      sm_pPerfData->LockCtrs();

      dwErr = sm_pPerfData->CollectAllObjs(pValueName, ppbData, 
         pcbTotalBytes, pNumObjectTypes);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {
      *ppbData = ppbOriginalStartOfBuffer;
      *pcbTotalBytes = 0;
      *pNumObjectTypes = 0;
   }
   sm_pPerfData->UnlockCtrs();
   return(dwErr);
}
#endif



///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
// Collects all of the requested objects
DWORD CPerfData::CollectAllObjs(PWSTR pszValueName, PBYTE *ppbData, 
   PDWORD pcbTotalBytes, PDWORD pdwNumObjectTypes) const {

   // pszValueName:      [in]  Set of object numbers requested
   // ppbData:           [in]  Buffer where object info goes
   //                    [out] Address after our data
   // pcbTotalBytes:     [in]  Size of buffer
   //                    [out] Bytes we put in buffer
   // pdwNumObjectTypes: [in]  Ignore 
   //                    [out] Number of objects we put in the buffer
   // Return Value:      ERROR_MORE_DATA or ERROR_SUCCESS
   DWORD dwErr = ERROR_SUCCESS;
   CWhichCtrs CtrList(pszValueName);

   *pdwNumObjectTypes = 0;

   // Default to collecting none of our objects
   for (OBJORD ObjOrd = 0; ObjOrd < m_nNumObjects; ObjOrd++) {
      CvtObjOrdToPerfItem(ObjOrd)->fCollectThisObj = FALSE;
   }

   // Collect only the objects explicitly specified
   // and any instances' parent objects
   for (ObjOrd = 0; ObjOrd < m_nNumObjects; ObjOrd++) {
      // Should this object's counters be returned?
      if (CtrList.IsNumInList(
         CvtObjOrdToPerfItemIndex(ObjOrd) * 2 + m_dwFirstCounter)) {
         CvtObjOrdToPerfItem(ObjOrd)->fCollectThisObj = TRUE;
         DetermineObjsToCollect(ObjOrd);
      }
   }

   // Calculcate the bytes required for the desired objects
   DWORD cbBytesForAllObjs = 0;
   for (ObjOrd = 0; ObjOrd < m_nNumObjects; ObjOrd++) {
      if (CvtObjOrdToPerfItem(ObjOrd)->fCollectThisObj)
         cbBytesForAllObjs += CalcBytesForPerfObj(ObjOrd);
   }

   if (*pcbTotalBytes < cbBytesForAllObjs) {
      // If buffer too small for desired objects, return failure
      *pcbTotalBytes = 0;
      dwErr = ERROR_MORE_DATA;
   } else {
      // Buffer is big enough, append objects' data to buffer
      *pcbTotalBytes = 0;

      for (ObjOrd = 0; ObjOrd < m_nNumObjects; ObjOrd++) {
         if (CvtObjOrdToPerfItem(ObjOrd)->fCollectThisObj) {
            CollectAnObj(ObjOrd, ppbData);
            *pcbTotalBytes += CalcBytesForPerfObj(ObjOrd);
            (*pdwNumObjectTypes)++;
         }
      }
   }
   return(dwErr);
}
#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
DWORD CPerfData::CollectAnObj(OBJORD ObjOrd, PBYTE *ppbData) const {

   chASSERT(IsValidObjOrd(ObjOrd));
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, 0, &pm);

   // Append PERF_OBJECT_TYPE
   CopyMemory(*ppbData, pm.pPOT, pm.cbPOT);
   PPERF_OBJECT_TYPE pPOT = (PPERF_OBJECT_TYPE) *ppbData;
   pPOT->TotalByteLength = CalcBytesForPerfObj(ObjOrd); 
   pPOT->NumInstances = (pm.MaxInstances == PERF_NO_INSTANCES) 
      ? PERF_NO_INSTANCES : HowManyInstancesInUse(ObjOrd);
   *ppbData += pm.cbPOT;
   
   // Append array of PERF_COUNTER_DEFINITIONs
   CopyMemory(*ppbData, pm.pPCD, pm.cbPCD);
   *ppbData += pm.cbPCD;

   if (!pm.fSupportsInstances) {
      // Append 1 PERF_COUNTER_BLOCK
      CopyMemory(*ppbData, pm.pPCB, pm.cbPCB);
      *ppbData += pm.cbPCB;
   } else {
      // Append PERF_INSTANCE_DEFINITION/PERF_COUNTER_BLOCKs
      if (pPOT->NumInstances > 0) {
         for (INSTID InstId = 0; InstId < (INSTID) pm.MaxInstances; InstId++) {
            CalcPerfMetrics(ObjOrd, InstId, &pm);

            if (pm.pPIN[0] != 0) {  // This instance is in use

               // Append PERF_INSTANCE_DEFINITIONs
               CopyMemory(*ppbData, pm.pPID, pm.cbPID);
               PPERF_INSTANCE_DEFINITION pPID = 
                  (PPERF_INSTANCE_DEFINITION) *ppbData;
               *ppbData += pm.cbPID;

               pPID->ByteLength = sizeof(PERF_INSTANCE_DEFINITION);

               // The ParentObjectTitleIndex contains the parent 
               // object's ID. If this is not 0 (an invalid object ID),
               // convert the ID to the Performance Object number
               if (pPID->ParentObjectTitleIndex != 0) {
                  PIINDEX pii = CvtObjIdToPerfItemIndex(
                     (OBJID) pm.pPID->ParentObjectTitleIndex);
                  pPID->ParentObjectTitleIndex = m_dwFirstCounter + 2 * pii;

                  // Convert Instance ID to In-Use Instance number.
                  pPID->ParentObjectInstance = 
                     CvtInstIdToInUseInstId(CvtObjIdToObjOrd(
                        (OBJID) pm.pPID->ParentObjectTitleIndex), 
                        (INSTID) pm.pPID->ParentObjectInstance);
               }

               // Append instance name after PERF_INSTANCE_DEFINITION
               if (pPID->UniqueID == PERF_NO_UNIQUE_ID) {
                  DWORD cchName = lstrlenW(pm.pPIN) + 1;
                  DWORD cbName = cchName * sizeof(WCHAR);
                  CopyMemory(*ppbData, pm.pPIN, cbName);

                  // If an odd number of characters, add 2 bytes so
                  // that next structure starts on 32-bit boundary 
                  if ((cchName & 1) == 1) cbName += 2;  
                  *ppbData += cbName;
                  pPID->ByteLength += cbName;
               }

               // Append PERF_COUNTER_BLOCK to the buffer
               CopyMemory(*ppbData, pm.pPCB, pm.cbPCB);
               *ppbData += pm.cbPCB;
            }
         }
      }
   }
   // Each object should be aligned on an 8-byte boundary
   *ppbData = (PBYTE) chROUNDUP((UINT_PTR) *ppbData, 8);
   return(ERROR_SUCCESS);
}
#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
DWORD CPerfData::CalcBytesForPerfObj(OBJORD ObjOrd) const {

   chASSERT(IsValidObjOrd(ObjOrd));
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, 0, &pm);
   DWORD cbBytesNeeded = pm.cbPOT + pm.cbPCD;

   if (!pm.fSupportsInstances) {
      cbBytesNeeded += pm.cbPCB;
   } else {
      cbBytesNeeded += (pm.cbPID + pm.cbPCB) * HowManyInstancesInUse(ObjOrd);

      for (INSTID InstId = 0; InstId < (INSTID) pm.MaxInstances; InstId++) {
         CalcPerfMetrics(ObjOrd, InstId, &pm);
         if (pm.pPIN[0] != 0) {
            DWORD cch = lstrlenW(pm.pPIN) + 1;  // For 0 character
            if ((cch & 1) == 1) cch++;  

            // If an odd number of characters, add 2 bytes so
            // that next structure starts on 32-bit boundary 
            cbBytesNeeded += sizeof(WCHAR) * cch;
         }
      }
   }
   // Each object should be aligned on an 8-byte boundary
   return(chROUNDUP(cbBytesNeeded, 8));   
}
#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
int CPerfData::HowManyInstancesInUse(OBJORD ObjOrd) const {

   chASSERT(IsValidObjOrd(ObjOrd));
   PERFMETRICS pm;
   CalcPerfMetrics(ObjOrd, 0, &pm);
   chASSERT(pm.fSupportsInstances);

   int nNumInstancesInUse = 0;
   for (INSTID InstId = 0; InstId < (INSTID) pm.MaxInstances; InstId++) {
      if (pm.pPIN[0] != 0) nNumInstancesInUse++;
      pm.pPIN += pm.cbPIN / sizeof(pm.pPIN[0]);
   }
   return(nNumInstancesInUse);
}
#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef PERFDATA_COLLECT_SUPPORTED
int CPerfData::CvtInstIdToInUseInstId(OBJORD ObjOrd, INSTID InstId) const {

   chASSERT(IsValidInstId(ObjOrd, InstId));
   int nIndexInstInUse = 0;
   for (INSTID Id = 0; Id < InstId; Id++) {
      PERFMETRICS pm;
      CalcPerfMetrics(ObjOrd, Id, &pm);
      if (pm.pPIN[0] != 0) nIndexInstInUse++;
   }
   return(nIndexInstInUse);
}
#endif


///////////////////////////////////////////////////////////////////////////////


#endif   // PERFDATA_IMPL


///////////////////////////////// End Of File /////////////////////////////////
