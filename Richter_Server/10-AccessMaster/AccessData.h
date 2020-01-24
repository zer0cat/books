/******************************************************************************
Module:  AccessData.h
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"              /* See Appendix A. */
#include <ACLAPI.h>
#include <ACLUI.h>
#include <lmshare.h>


///////////////////////////////////////////////////////////////////////////////


#ifndef UNICODE
#error This module must be compiled natively using Unicode.
#endif


///////////////////////////////////////////////////////////////////////////////

typedef enum AM_SECURETYPE{
   AM_FILE = 0, AM_DIR, AM_SERVICE, 
   AM_PRINTER, AM_REGISTRY, AM_SHARE,   
   AM_PROCESS, AM_THREAD, AM_JOB,     
   AM_SEMAPHORE, AM_EVENT, AM_MUTEX,    
   AM_MAPPING, AM_TIMER, AM_TOKEN,    
   AM_NAMEDPIPE, AM_ANONPIPE, 
   AM_WINDOWSTATION, AM_DESKTOP       
};


typedef struct _ObjEntry {
   AM_SECURETYPE  m_nSpecificType;
   SE_OBJECT_TYPE m_objType;
   PTSTR          m_pszComboText;
   PTSTR          m_pszUsageText;
   BOOL           m_fUseName;
   BOOL           m_fUseHandle;
   BOOL           m_fUsePID;
   BOOL           m_fIsChild;
   BOOL           m_fIsContainer;
} ObjEntry;


ObjEntry g_objMap[] = {
   {AM_TOKEN, SE_KERNEL_OBJECT, TEXT("Access Token"),
         TEXT("To view the access rights of an access token enter the ")
         TEXT("numerical value of its handle and process ID in the ")
         TEXT("\"Handle\" and \"PID/TID\" fields."),
         FALSE, TRUE, TRUE, FALSE, FALSE},   
   {AM_ANONPIPE, SE_KERNEL_OBJECT, TEXT("Anonymous Pipe"),
         TEXT("To view the access rights of an Anonymous Pipe enter the ")
         TEXT("numerical value of its handle and process ID in the ")
         TEXT("\"Handle\" and \"PID/TID\" fields."),
         FALSE, TRUE, TRUE, FALSE, FALSE},
   {AM_DESKTOP, SE_WINDOW_OBJECT, TEXT("Desktop"),
         TEXT("To view the access rights of a Desktop object either enter ")
         TEXT("the Window Station and Desktop names in the \"Name\" field, ")
         TEXT("or the numerical value of its handle and process ID in the ")
         TEXT("\"Handle\" and \"PID/TID\" ")
         TEXT("fields.\r\nExample: \"Winsta0\\Default\""),
         TRUE, TRUE, TRUE, FALSE, FALSE},
   {AM_DIR, SE_FILE_OBJECT, TEXT("Directory"),
         TEXT("To view the access rights of a directory enter the fully ")
         TEXT("qualified path in the \"Name\" field."),
         TRUE, FALSE, FALSE, TRUE, TRUE},
   {AM_EVENT, SE_KERNEL_OBJECT, TEXT("Event"),
         TEXT("To view the access rights of an Event either enter the ")
         TEXT("object's name in the \"Name\" field, or the numerical value ")
         TEXT("of its handle and process ID in ")
         TEXT("the \"Handle\" and \"PID/TID\" fields."),
         TRUE, TRUE, TRUE, FALSE, FALSE},
   {AM_FILE, SE_FILE_OBJECT, TEXT("File"),
         TEXT("To view the access rights of a file either enter the fully ")
         TEXT("qualified path and filname in the \"Name\" field, or the ")
         TEXT("numerical value of the handle and process in the \"Handle\" ")
         TEXT("and \"PID/TID\" fields."),
         TRUE, TRUE, TRUE, TRUE, FALSE},
   {AM_MAPPING, SE_KERNEL_OBJECT, TEXT("File Mapping"),
         TEXT("To view the access rights of a file mapping either enter ")
         TEXT("the object's name in the \"Name\" field, or the numerical ")
         TEXT("value of its handle and process ID in the \"Handle\" and ")
         TEXT("\"PID/TID\" fields."),
         TRUE, TRUE, TRUE, FALSE, FALSE},
   {AM_JOB, SE_KERNEL_OBJECT, TEXT("Job"),
         TEXT("Enter the numerical value of the Job's handle and process ")
         TEXT("ID's in the \"Handle\" and \"PID/TID\" fields."),
         FALSE, TRUE, TRUE, FALSE, FALSE},   
   {AM_MUTEX, SE_KERNEL_OBJECT, TEXT("Mutex"),
         TEXT("To view the access rights of a Mutex either enter the ")
         TEXT("object's name in the \"Name\" field, or the numerical value ")
         TEXT("of its handle and process ID in ")
         TEXT("the \"Handle\" and \"PID/TID\" fields."),
         TRUE, TRUE, TRUE, FALSE, FALSE},
   {AM_NAMEDPIPE, SE_KERNEL_OBJECT, TEXT("Named Pipe"),
         TEXT("To view the access rights of a Named Pipe enter the numerical ")
         TEXT("value of its handle and process ID in ")
         TEXT("the \"Handle\" and \"PID/TID\" fields."),
         FALSE, TRUE, TRUE, FALSE, FALSE},
   {AM_PRINTER, SE_PRINTER, TEXT("Printer"),
         TEXT("To view the access rights of a printer or print server enter ")
         TEXT("the object's fully qualified UNC name in the \"Name\" field."),
         TRUE, FALSE, FALSE, FALSE, FALSE},
   {AM_PROCESS, SE_KERNEL_OBJECT, TEXT("Process"),
         TEXT("Enter a Process' ID in the \"PID/TID\" field."),
         FALSE, FALSE, TRUE, FALSE, FALSE},   
   {AM_REGISTRY, SE_REGISTRY_KEY, TEXT("Registry Key"),
         TEXT("To view the access rights of a registry key either enter the ")
         TEXT("fully qualified path and keyname in the \"Name\" field, or ")
         TEXT("the numerical value of the handle and process in the ")
         TEXT("\"Handle\" and \"PID/TID\" fields.  \r\nExample: \"\\\\machine")
         TEXT("name\\CLASSES_ROOT\\somepath\r\n\r\nThe following predefined ")
         TEXT("registry key values can be used:  \"CLASSES_ROOT\", \"CURRENT_")
         TEXT("USER\", \"MACHINE\", and \"USERS\"."),
         TRUE, TRUE, TRUE, TRUE, TRUE},
   {AM_SEMAPHORE, SE_KERNEL_OBJECT, TEXT("Semaphore"),
         TEXT("To view the access rights of a Semaphore either enter the ")
         TEXT("object's name in the \"Name\" field, or the numerical value ")
         TEXT("of its handle and process ID in the \"Handle\" ")
         TEXT("and \"PID/TID\" fields."),
         TRUE, TRUE, TRUE, FALSE, FALSE},   
   {AM_SERVICE, SE_SERVICE, TEXT("Service"),
         TEXT("To view the access rights of a service enter the service's ")
         TEXT("programmatic name in the \"Name\" field."),
         TRUE, FALSE, FALSE, FALSE, FALSE},
   {AM_SHARE, SE_LMSHARE, TEXT("Share"),
         TEXT("Enter a network share name in the \"Name\" field.  A share ")
         TEXT("object can be local, such as \"sharename\"; or ")
         TEXT("remote, such as \"\\\\machinename\\sharename\"."),
         TRUE, FALSE, FALSE, FALSE, FALSE},      
   {AM_THREAD, SE_KERNEL_OBJECT, TEXT("Thread"),
         TEXT("Enter a Thread's Thread-ID or TID in the \"PID/TID\" field"),
         FALSE, FALSE, TRUE, FALSE, FALSE},   
   {AM_TIMER, SE_KERNEL_OBJECT, TEXT("Waitable Timer"),
         TEXT("To view the access rights of a Waitable Timer either enter ")
         TEXT("the object's name in the \"Name\" field, or the numerical value ")
         TEXT("of its handle and process ID in ")
         TEXT("the \"Handle\" and \"PID/TID\" fields."),
         TRUE, TRUE, TRUE, FALSE, FALSE},   
   {AM_WINDOWSTATION, SE_WINDOW_OBJECT, TEXT("Window Station"),
         TEXT("To view the access rights of a Window Station either enter ")
         TEXT("the object's name in the \"Name\" field, or the numerical ")
         TEXT("value of its handle and process ID in the \"Handle\" and ")
         TEXT("\"PID/TID\" fields.\r\nExample: \"Winsta0\""),
         TRUE, TRUE, TRUE, FALSE, FALSE}   
};


typedef struct _ObjInf {
   ObjEntry*   m_pEntry;
   HANDLE      m_hHandle;
   TCHAR       m_szName[1024];
   TCHAR       m_szObjectName[2048];
} ObjInf;


///////////////////////////////////////////////////////////////////////////////


class CSecurityInformation: public ISecurityInformation {

public:
   CSecurityInformation(ObjInf* pInfo, BOOL fBinary) { 
      m_pInfo = pInfo; 
      m_nRef = 1; 
      m_fBinary = fBinary; 
   }

private:
   static GUID m_guidNULL;
   static SI_ACCESS m_siAccessAllRights[][19];
   static SI_ACCESS m_siAccessBinaryRights[32];
   static SI_INHERIT_TYPE m_siInheritType[];

   ULONG    m_nRef;
   ObjInf*  m_pInfo;

   BOOL     m_fBinary;

public:
   HRESULT WINAPI QueryInterface(REFIID riid, PVOID* ppvObj);
   ULONG WINAPI AddRef();
   ULONG WINAPI Release();

private:
   HRESULT WINAPI GetObjectInformation(PSI_OBJECT_INFO pObjectInfo);
   HRESULT WINAPI GetSecurity(SECURITY_INFORMATION RequestedInformation,
      PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault);
   HRESULT WINAPI SetSecurity(SECURITY_INFORMATION SecurityInformation,
      PSECURITY_DESCRIPTOR pSecurityDescriptor);
   HRESULT WINAPI GetAccessRights(const GUID* pguidObjectType,
      DWORD dwFlags, // si_edit_audits, si_edit_properties
      PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess);
   HRESULT WINAPI MapGeneric(const GUID* pguidObjectType,
      UCHAR* pAceFlags, ACCESS_MASK* pMask);
   HRESULT WINAPI GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, 
         ULONG* pcInheritTypes);
   HRESULT WINAPI PropertySheetPageCallback(HWND hwnd, UINT uMsg, 
         SI_PAGE_TYPE uPage);
};

GUID CSecurityInformation::m_guidNULL = GUID_NULL;


// Binary aces
SI_ACCESS CSecurityInformation::m_siAccessBinaryRights[] = {
   {&m_guidNULL, 0x0001, L"0000000000000001 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0002, L"0000000000000010 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0004, L"0000000000000100 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0008, L"0000000000001000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0010, L"0000000000010000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0020, L"0000000000100000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0040, L"0000000001000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0080, L"0000000010000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0100, L"0000000100000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0200, L"0000001000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0400, L"0000010000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x0800, L"0000100000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x1000, L"0001000000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x2000, L"0010000000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x4000, L"0100000000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x8000, L"1000000000000000 [Specific 15-0]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x010000, L"00000001 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x020000, L"00000010 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x040000, L"00000100 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x080000, L"00001000 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x100000, L"00010000 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x200000, L"00100000 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x400000, L"01000000 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x800000, L"10000000 [Standard 23-16]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x1000000, L"1 [SACL 24]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x2000000, L"1 [Maximum Allowed 25]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x4000000, L"01 [Reserved 27-26]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x8000000, L"10 [Reserved 27-26]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x10000000, L"0001 [Generic 31-28]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x20000000, L"0010 [Generic 31-28]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x40000000, L"0100 [Generic 31-28]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
   {&m_guidNULL, 0x80000000, L"1000 [Generic 31-28]", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC}
};


///////////////////////////////////////////////////////////////////////////////


#define ACCESS_NULL_ENTRY NULL, 0, NULL, 0
SI_ACCESS CSecurityInformation::m_siAccessAllRights[][19] = { 
   
   {  // File (0)
      {&m_guidNULL, FILE_ALL_ACCESS, L"FILE_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_DATA, L"FILE_READ_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_DATA, L"FILE_WRITE_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_APPEND_DATA, L"FILE_APPEND_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_EA, L"FILE_READ_EA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_EA, L"FILE_WRITE_EA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_EXECUTE, L"FILE_EXECUTE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_ATTRIBUTES, L"FILE_READ_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_ATTRIBUTES, L"FILE_WRITE_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Dir (14)
      {&m_guidNULL, FILE_ALL_ACCESS, L"FILE_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_LIST_DIRECTORY, L"FILE_LIST_DIRECTORY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_ADD_FILE, L"FILE_ADD_FILE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_ADD_SUBDIRECTORY, L"FILE_ADD_SUBDIRECTORY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_EA, L"FILE_READ_EA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_EA, L"FILE_WRITE_EA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_TRAVERSE, L"FILE_TRAVERSE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_DELETE_CHILD, L"FILE_DELETE_CHILD", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_ATTRIBUTES, L"FILE_READ_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_ATTRIBUTES, L"FILE_WRITE_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Service (29)
      {&m_guidNULL, SERVICE_ALL_ACCESS, L"SERVICE_ALL_ACCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_CHANGE_CONFIG, L"SERVICE_CHANGE_CONFIG", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_ENUMERATE_DEPENDENTS, 
            L"SERVICE_ENUMERATE_DEPENDENTS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_INTERROGATE, L"SERVICE_INTERROGATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_PAUSE_CONTINUE, L"SERVICE_PAUSE_CONTINUE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_QUERY_CONFIG, L"SERVICE_QUERY_CONFIG", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_QUERY_STATUS, L"SERVICE_QUERY_STATUS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_START, L"SERVICE_START", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_STOP, L"SERVICE_STOP", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVICE_USER_DEFINED_CONTROL, 
            L"SERVICE_USER_DEFINED_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Printer (44)
      {&m_guidNULL, SERVER_ACCESS_ADMINISTER, L"SERVER_ACCESS_ADMINISTER", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SERVER_ACCESS_ENUMERATE, L"SERVER_ACCESS_ENUMERATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PRINTER_ACCESS_ADMINISTER, L"PRINTER_ACCESS_ADMINISTER", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PRINTER_ACCESS_USE, L"PRINTER_ACCESS_USE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, JOB_ACCESS_ADMINISTER, L"JOB_ACCESS_ADMINISTER", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Registry (55)
      {&m_guidNULL, KEY_ALL_ACCESS, L"KEY_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, KEY_QUERY_VALUE, L"KEY_QUERY_VALUE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, KEY_SET_VALUE, L"KEY_SET_VALUE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, KEY_CREATE_SUB_KEY, L"KEY_CREATE_SUB_KEY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, KEY_ENUMERATE_SUB_KEYS, L"KEY_ENUMERATE_SUB_KEYS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, KEY_NOTIFY, L"KEY_NOTIFY", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, KEY_CREATE_LINK, L"KEY_CREATE_LINK", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC}, 
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Share (68)
      {&m_guidNULL, PERM_FILE_READ, L"PERM_FILE_READ", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PERM_FILE_WRITE, L"PERM_FILE_WRITE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PERM_FILE_CREATE, L"PERM_FILE_CREATE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Process (74)
      {&m_guidNULL, PROCESS_ALL_ACCESS, L"PROCESS_TERMINATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_TERMINATE, L"PROCESS_TERMINATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_CREATE_THREAD, L"PROCESS_CREATE_THREAD", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_SET_SESSIONID, L"PROCESS_SET_SESSIONID", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_VM_OPERATION, L"PROCESS_VM_OPERATION", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_VM_READ, L"PROCESS_VM_READ", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_VM_WRITE, L"PROCESS_VM_WRITE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_DUP_HANDLE, L"PROCESS_DUP_HANDLE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_CREATE_PROCESS, L"PROCESS_CREATE_PROCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_SET_QUOTA, L"PROCESS_SET_QUOTA", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_SET_INFORMATION, L"PROCESS_SET_INFORMATION", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, PROCESS_QUERY_INFORMATION, L"PROCESS_QUERY_INFORMATION", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Thread (91)
      {&m_guidNULL, THREAD_ALL_ACCESS, L"THREAD_ALL_ACCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_TERMINATE, L"THREAD_TERMINATE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_SUSPEND_RESUME, L"THREAD_SUSPEND_RESUME", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_GET_CONTEXT, L"THREAD_GET_CONTEXT", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_SET_CONTEXT, L"THREAD_SET_CONTEXT", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_SET_INFORMATION, L"THREAD_SET_INFORMATION", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_QUERY_INFORMATION, L"THREAD_QUERY_INFORMATION", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_SET_THREAD_TOKEN, L"THREAD_SET_THREAD_TOKEN", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_IMPERSONATE, L"THREAD_IMPERSONATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, THREAD_DIRECT_IMPERSONATION, 
            L"THREAD_DIRECT_IMPERSONATION", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Job (107)
      {&m_guidNULL, JOB_OBJECT_ALL_ACCESS, L"JOB_OBJECT_ALL_ACCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, JOB_OBJECT_ASSIGN_PROCESS, L"JOB_OBJECT_ASSIGN_PROCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, JOB_OBJECT_SET_ATTRIBUTES, L"JOB_OBJECT_SET_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, JOB_OBJECT_QUERY, L"JOB_OBJECT_QUERY", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, JOB_OBJECT_TERMINATE, L"JOB_OBJECT_TERMINATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, JOB_OBJECT_SET_SECURITY_ATTRIBUTES, 
            L"JOB_OBJECT_SET_SECURITY_ATTRIBUTES", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Semaphore
      {&m_guidNULL, SEMAPHORE_ALL_ACCESS, L"SEMAPHORE_ALL_ACCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SEMAPHORE_MODIFY_STATE, L"SEMAPHORE_MODIFY_STATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Event
      {&m_guidNULL, EVENT_ALL_ACCESS, L"EVENT_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, EVENT_MODIFY_STATE, L"EVENT_MODIFY_STATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Mutex
      {&m_guidNULL, MUTEX_ALL_ACCESS, L"MUTEX_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, MUTEX_MODIFY_STATE, L"MUTEX_MODIFY_STATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Mapping
      {&m_guidNULL, FILE_MAP_COPY, L"FILE_MAP_COPY", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_MAP_WRITE, L"FILE_MAP_WRITE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_MAP_READ, L"FILE_MAP_READ", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_MAP_ALL_ACCESS, L"FILE_MAP_ALL_ACCESS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SECTION_EXTEND_SIZE, L"SECTION_EXTEND_SIZE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Timer
      {&m_guidNULL, TIMER_ALL_ACCESS, L"TIMER_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TIMER_QUERY_STATE, L"TIMER_QUERY_STATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TIMER_MODIFY_STATE, L"TIMER_MODIFY_STATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Token
      {&m_guidNULL, TOKEN_ALL_ACCESS, L"TOKEN_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_ASSIGN_PRIMARY, L"TOKEN_ASSIGN_PRIMARY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_DUPLICATE, L"TOKEN_DUPLICATE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_IMPERSONATE, L"TOKEN_IMPERSONATE", 
             SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_QUERY, L"TOKEN_QUERY", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_QUERY_SOURCE, L"TOKEN_QUERY_SOURCE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_ADJUST_PRIVILEGES, L"TOKEN_ADJUST_PRIVILEGES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_ADJUST_GROUPS, L"TOKEN_ADJUST_GROUPS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_ADJUST_DEFAULT, L"TOKEN_ADJUST_DEFAULT", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, TOKEN_ADJUST_SESSIONID, L"TOKEN_ADJUST_SESSIONID", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Namedpipe
      {&m_guidNULL, FILE_ALL_ACCESS, L"FILE_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_DATA, L"FILE_READ_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_DATA, L"FILE_WRITE_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_CREATE_PIPE_INSTANCE, L"FILE_CREATE_PIPE_INSTANCE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_ATTRIBUTES, L"FILE_READ_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_ATTRIBUTES, L"FILE_WRITE_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Anonpipe
      {&m_guidNULL, FILE_ALL_ACCESS, L"FILE_ALL_ACCESS", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_DATA, L"FILE_READ_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_DATA, L"FILE_WRITE_DATA", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_CREATE_PIPE_INSTANCE, L"FILE_CREATE_PIPE_INSTANCE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_READ_ATTRIBUTES, L"FILE_READ_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, FILE_WRITE_ATTRIBUTES, L"FILE_WRITE_ATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Windowstation
      {&m_guidNULL, WINSTA_ACCESSCLIPBOARD, L"WINSTA_ACCESSCLIPBOARD", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_ACCESSGLOBALATOMS, L"WINSTA_ACCESSGLOBALATOMS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_CREATEDESKTOP, L"WINSTA_CREATEDESKTOP", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_ENUMDESKTOPS, L"WINSTA_ENUMDESKTOPS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_ENUMERATE, L"WINSTA_ENUMERATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_EXITWINDOWS, L"WINSTA_EXITWINDOWS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_READATTRIBUTES, L"WINSTA_READATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_READSCREEN, L"WINSTA_READSCREEN", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WINSTA_WRITEATTRIBUTES, L"WINSTA_WRITEATTRIBUTES", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   },

   {  // Desktop
      {&m_guidNULL, DESKTOP_CREATEMENU, L"DESKTOP_CREATEMENU", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_CREATEWINDOW, L"DESKTOP_CREATEWINDOW", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_ENUMERATE, L"DESKTOP_ENUMERATE", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_HOOKCONTROL, L"DESKTOP_HOOKCONTROL", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_JOURNALPLAYBACK, L"DESKTOP_JOURNALPLAYBACK", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_JOURNALRECORD, L"DESKTOP_JOURNALRECORD", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_READOBJECTS, L"DESKTOP_READOBJECTS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_SWITCHDESKTOP, L"DESKTOP_SWITCHDESKTOP", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DESKTOP_WRITEOBJECTS, L"DESKTOP_WRITEOBJECTS", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, READ_CONTROL, L"READ_CONTROL", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_DAC, L"WRITE_DAC", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, WRITE_OWNER, L"WRITE_OWNER", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, DELETE, L"DELETE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, SYNCHRONIZE, L"SYNCHRONIZE", 
         SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {&m_guidNULL, ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY", 
            SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC},
      {ACCESS_NULL_ENTRY}
   }
};


///////////////////////////////// End of File /////////////////////////////////