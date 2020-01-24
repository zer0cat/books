/******************************************************************************
Module:  RoboClient.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#define _UNICODE
#define UNICODE

// #define _WIN32_WINNT 0x0500
#include "..\CmnHdr.h"              /* See Appendix A. */

#include <WindowsX.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

#include <Process.h>
#include <ACLAPI.h>
#include <ACLUI.h>

#pragma comment(lib, "ACLUI.lib")   // Force linking against this library

#include "Resource.h"


#define UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"

HINSTANCE g_hInst;

#define PIPENAME TEXT("\\Pipe\\RoboService")

typedef struct _RoboClientState {
   HANDLE            m_hPipe;
   HANDLE            m_hThread;
   HWND              m_hwndDlg;
   HWND              m_hwndList;
   CUILayout*        m_pResizer;
} RoboClientState;

#define ROBOMSG_STOPSERVICE      1
#define ROBOMSG_REMOVESERVICE    2
#define ROBOMSG_ERR              3

#define ROBOMSG_QUERYROBOTNAMES  10
#define ROBOMSG_ROBOTNAME        11 // Requires MsgDataName extra data

#define ROBOMSG_CREATEROBOT      12 // Requires MsgDataName extra data
#define ROBOMSG_DELETEROBOT      13 // Requires MsgDataName extra data
#define ROBOMSG_ROBOTREMOVED     14 // Requires MsgDataName extra data

#define ROBOMSG_CHANGENAME       15 // Requires an array of two MsgDataName

#define ROBOMSG_LOCK             16 // Requires MsgDataName extra data

#define ROBOMSG_QUERY            17 // Requires MsgDataName extra data
#define ROBOMSG_ROBOTMSG         18 // Requires a following string

#define ROBOMSG_ACTION           19 // Requires MsgDataName extra data

#define ROBOMSG_QUERYSECURITY    20 // Requires MsgDataName extra data
#define ROBOMSG_RETURNSECURITY   21 // Requires MsgDataSD extra data

#define ROBOMSG_SETSECURITY      22 // Requires MsgDataName and 
                                    // MsgDataSD extra data

#define ROBOERROR_NAMEEXISTS     1
#define ROBOERROR_ROBOTNOTFOUND  2
#define ROBOERROR_ACCESSDENIED   3

#define ROBOACTION_GATHER        1
#define ROBOACTION_ASSEMBLE      2

// What we can do to a robot
// Change its name
#define ROBOT_SETNAME      (0x0001)
// Lock it
#define ROBOT_LOCK         (0x0002)
// Gather material
#define ROBOT_GATHER       (0x0004)
// Assemble material
#define ROBOT_ASSEMBLE     (0x0008)
// Query status
#define ROBOT_QUERY        (0x0010)
// Unlock it (even if you aren't the "locker")
#define ROBOT_OVERRIDELOCK (0x0020)
// Delete it
// Set security
// Get security
// Set owner
#define ROBOT_ALL_ACCESS   (STANDARD_RIGHTS_REQUIRED |\
                           ROBOT_SETNAME |\
                           ROBOT_LOCK |\
                           ROBOT_GATHER |\
                           ROBOT_ASSEMBLE |\
                           ROBOT_QUERY|\
                           ROBOT_OVERRIDELOCK)

typedef struct _MessageBase {
   ULONG m_lMsgType;
   ULONG m_lInfo;
   ULONG m_lExtraDataSize;
} MessageBase;

typedef struct _Message {
   MessageBase m_baseMsg;
   PVOID  m_pvData; // Points to data
} Message;
#define MSGSIZE sizeof(MessageBase)

typedef struct _MsgDataName {
   TCHAR m_szName[256];
} MsgDataName;

typedef struct _MsgDataSD {
   SECURITY_DESCRIPTOR m_sdSecurity;
} MsgDataSD;


///////////////////////////////////////////////////////////////////////////////


void EnableControls(HWND hwnd, BOOL fEnable) {

   UINT nControls[] = { IDB_GATHER, IDB_SECURITY, IDB_REMOVE, IDB_ASSEMBLE, 
         IDB_LOCK, IDB_UNLOCK, IDB_QUERY, IDB_TAKEOWNERSHIP, IDB_RENAME };

   EnableWindow(GetDlgItem(hwnd, IDB_ADD), fEnable);
   EnableWindow(GetDlgItem(hwnd, IDE_SERVER), !fEnable);

   HWND hwndList = GetDlgItem(hwnd, IDL_ROBOTS);
   EnableWindow(hwndList, fEnable);
   if (!fEnable) {
      ListView_DeleteAllItems(hwndList);
   }

   fEnable = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED) >= 0;

   for (int nIndex = 0;nIndex < chDIMOF(nControls);nIndex++) {
      EnableWindow(GetDlgItem(hwnd, nControls[nIndex]), fEnable);
   }
}


///////////////////////////////////////////////////////////////////////////////


BOOL EnablePrivilege(LPTSTR szPriv, BOOL fEnabled) {

   TOKEN_PRIVILEGES  tp;
   HANDLE            hToken = NULL;
   LUID              luid;
   BOOL              fRet = FALSE;

   __try {

      // First lookup the system unique luid for the privilege
      if (!LookupPrivilegeValue(NULL, szPriv, &luid)) {

         // If the name is bogus...
         __leave;
      }

      // Then get the processes token
      if (!OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
            &hToken)) {
         __leave;
      }

      // Set up our token privileges "array" (in our case an array of one)
      tp.PrivilegeCount           = 1;
      tp.Privileges[0].Luid       = luid;
      tp.Privileges[0].Attributes = fEnabled?SE_PRIVILEGE_ENABLED:0;

      // Adjust our token privileges by enabling or disabling this one
      if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
            NULL, NULL)) {
         __leave;
      }
      fRet = TRUE;
   
   }
   __finally {

      // Cleanup
      if (hToken != NULL)
         CloseHandle(hToken);
   }

   return(fRet);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_ROBOCLIENT);

   // Enable the take ownership privilege right away, just in case
   EnablePrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE);

   // We are using common controls in this sample
   InitCommonControls();

   RoboClientState* prcState = new RoboClientState;
   ZeroMemory(prcState, sizeof(RoboClientState));

   // Set the pointer to the state structure as user data in the window
   SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) prcState);

   prcState->m_hwndDlg = hwnd;
   prcState->m_hwndList = GetDlgItem(hwnd, IDL_ROBOTS);

   TCHAR szTitle[1024];
   lstrcpy(szTitle, TEXT("RoboClient running as \""));
   ULONG lSize = chDIMOF(szTitle) - lstrlen(szTitle);
   GetUserName(szTitle + lstrlen(szTitle), &lSize);
   lstrcat(szTitle, TEXT("\""));
   SetWindowText(hwnd, szTitle);

   // Setup resizer control
   prcState->m_pResizer = new CUILayout;
   prcState->m_pResizer->Initialize(hwnd);
   prcState->m_pResizer->AnchorControl(CUILayout::AP_TOPLEFT, 
         CUILayout::AP_TOPRIGHT, IDE_SERVER, FALSE);
   prcState->m_pResizer->AnchorControl(CUILayout::AP_TOPLEFT, 
         CUILayout::AP_BOTTOMRIGHT, IDL_ROBOTS, FALSE);
   prcState->m_pResizer->AnchorControl(CUILayout::AP_TOPRIGHT, 
         CUILayout::AP_TOPRIGHT, IDB_CONNECT, FALSE);
   prcState->m_pResizer->AnchorControls(CUILayout::AP_BOTTOMRIGHT, 
         CUILayout::AP_BOTTOMRIGHT, FALSE,
         IDB_GATHER, IDB_ASSEMBLE, IDB_LOCK, IDB_UNLOCK, IDB_RENAME,
         IDB_SECURITY, IDS_ACTIONS, IDS_ADMIN, IDB_QUERY, IDB_TAKEOWNERSHIP, 
         (UINT) -1);
   prcState->m_pResizer->AnchorControls(CUILayout::AP_BOTTOMLEFT, 
         CUILayout::AP_BOTTOMLEFT, FALSE, IDB_ADD, IDB_REMOVE, (UINT) -1);

   HWND hwndList = GetDlgItem(hwnd, IDL_ROBOTS);

   RECT rect;
   GetClientRect(hwndList, &rect);

   // Add Columns to privilege list control
   LVCOLUMN column = {0};
   column.mask = LVCF_TEXT|LVCF_WIDTH;
   column.pszText = TEXT("Robots");
   column.cx = rect.right / 2;
   ListView_InsertColumn(hwndList, 0, &column);

   EnableControls(hwnd, FALSE);

   return (TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void SendWriteMessage(HANDLE hPipe, Message* pMsgNew) {

   BOOL fSuccess = FALSE;
   HANDLE hEvent = NULL;

   try { {
      hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
      if(!hEvent)
         goto leave;

      OVERLAPPED ovl = {0};
      ovl.hEvent = hEvent;
      ULONG lWritten = 0;
      fSuccess = WriteFile(hPipe, pMsgNew, MSGSIZE, &lWritten, &ovl);
      if (!fSuccess && GetLastError() == ERROR_IO_PENDING) {
         ULONG lWrote;
         fSuccess = GetOverlappedResult(hPipe, &ovl, &lWrote, TRUE);
      }      

      if (!fSuccess)
         goto leave;
      
      if (pMsgNew->m_baseMsg.m_lExtraDataSize > 0) {
         ZeroMemory(&ovl, sizeof(ovl));
         ResetEvent(hEvent);
         ovl.hEvent = hEvent;
         fSuccess = WriteFile(hPipe, pMsgNew->m_pvData, 
            pMsgNew->m_baseMsg.m_lExtraDataSize, &lWritten, &ovl);
         if (!fSuccess && GetLastError() == ERROR_IO_PENDING) {
            ULONG lWrote;
            fSuccess = GetOverlappedResult(hPipe, &ovl, &lWrote, TRUE);
         }     
         if (!fSuccess)
            goto leave;
      }

   } leave:;
   }
   catch (...) {
   }
   if(hEvent != NULL)
      CloseHandle(hEvent);

   if (!fSuccess) {
      // Perhaps clear the connection no matter what
   }
}


///////////////////////////////////////////////////////////////////////////////


void SendSimpleMessage(HWND hwnd, ULONG lMsg) {

   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   Message msg = {0};
   msg.m_baseMsg.m_lMsgType = lMsg;
   SendWriteMessage(prcState->m_hPipe, &msg);
}


///////////////////////////////////////////////////////////////////////////////


void SendRobotMessage(HWND hwnd, ULONG lMsg, ULONG lInfo) {

   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   int nCur = ListView_GetNextItem(prcState->m_hwndList, -1, LVNI_SELECTED);
   if (nCur >= 0) {
      TCHAR szName[256];
      ListView_GetItemText(prcState->m_hwndList, nCur, 0, szName, 
            chDIMOF(szName));

      Message msg = {0};
      msg.m_baseMsg.m_lMsgType = lMsg;
      msg.m_baseMsg.m_lInfo = lInfo;
      msg.m_pvData = szName;
      msg.m_baseMsg.m_lExtraDataSize = (lstrlen(szName) + 1) * sizeof(TCHAR);
      SendWriteMessage(prcState->m_hPipe, &msg);
   }
}


///////////////////////////////////////////////////////////////////////////////


class CSecurityInformation: public ISecurityInformation {

public:
   CSecurityInformation(RoboClientState* prcState, PTSTR pszRobot, 
         PSECURITY_DESCRIPTOR pSD) { m_prcState = prcState; m_nRef = 1;
         m_pSD = pSD; m_pszRobot = pszRobot; }

private:
static GUID m_guidNULL;
static SI_ACCESS m_siAccessRights[11];

ULONG                m_nRef;
RoboClientState*     m_prcState;
PSECURITY_DESCRIPTOR m_pSD;
PTSTR                m_pszRobot;

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


///////////////////////////////////////////////////////////////////////////////


GUID CSecurityInformation::m_guidNULL = GUID_NULL;

SI_ACCESS CSecurityInformation::m_siAccessRights[11] = {
   {&m_guidNULL, ROBOT_ALL_ACCESS, L"Full Control", SI_ACCESS_GENERAL},
   {&m_guidNULL, ROBOT_SETNAME, L"Edit Name", SI_ACCESS_GENERAL},
   {&m_guidNULL, ROBOT_LOCK, L"Lock", SI_ACCESS_GENERAL},
   {&m_guidNULL, ROBOT_GATHER, L"Gather Goods", SI_ACCESS_GENERAL},
   {&m_guidNULL, ROBOT_ASSEMBLE, L"Assemble Goods", SI_ACCESS_GENERAL},
   {&m_guidNULL, ROBOT_QUERY, L"Query State", SI_ACCESS_GENERAL},
   {&m_guidNULL, ROBOT_OVERRIDELOCK, L"Override Lock", SI_ACCESS_GENERAL},
   {&m_guidNULL, DELETE, L"Delete", SI_ACCESS_GENERAL},
   {&m_guidNULL, READ_CONTROL, L"Read Security Information", 
         SI_ACCESS_GENERAL},
   {&m_guidNULL, WRITE_DAC, L"Write Security Information", SI_ACCESS_GENERAL},
   {&m_guidNULL, WRITE_OWNER, L"Set Objects Owner", SI_ACCESS_GENERAL}
};


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::QueryInterface(REFIID riid, PVOID* ppvObj) {

   HRESULT hr = E_NOINTERFACE;
   if ((riid == IID_ISecurityInformation) || (riid == IID_IUnknown)) {
      *ppvObj = this;
      AddRef();
      hr = S_OK;
   }

   return(hr);
}


///////////////////////////////////////////////////////////////////////////////


ULONG CSecurityInformation::AddRef() {
   
   m_nRef++;

   return(m_nRef);
}


///////////////////////////////////////////////////////////////////////////////


ULONG CSecurityInformation::Release() {

   ULONG nRef = --m_nRef;
   if (m_nRef == 0)
      delete this;
   
   return(nRef);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
   pObjectInfo->dwFlags = SI_EDIT_ALL | SI_ADVANCED;
   pObjectInfo->hInstance = GetModuleHandle(NULL);
   pObjectInfo->pszServerName = NULL;
   pObjectInfo->pszObjectName = m_pszRobot;

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetSecurity(
      SECURITY_INFORMATION RequestedInformation,
      PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) {

   HRESULT hr = 1;
   ULONG lLength = GetSecurityDescriptorLength(m_pSD);
   PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, lLength);

   if (pSD!= NULL) {
      CopyMemory(pSD, m_pSD, lLength);
      hr = S_OK;
      *ppSecurityDescriptor = pSD;
   }

   return(hr);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetAccessRights(const GUID* pguidObjectType,
      DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, 
      ULONG* piDefaultAccess) {

   *ppAccess = m_siAccessRights;
   *pcAccesses = chDIMOF(m_siAccessRights);
   *piDefaultAccess = 8;

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::MapGeneric(const GUID* pguidObjectType, 
      UCHAR* pAceFlags, ACCESS_MASK* pMask) {

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes,
      ULONG* pcInheritTypes) {

   *pcInheritTypes = 0;

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::PropertySheetPageCallback(HWND hwnd, UINT uMsg, 
      SI_PAGE_TYPE uPage) {

   return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HRESULT CSecurityInformation::SetSecurity(
      SECURITY_INFORMATION SecurityInformation,
      PSECURITY_DESCRIPTOR pSecurityDescriptor) {

   ULONG        lLength = GetSecurityDescriptorLength(pSecurityDescriptor);
   ULONG        lLen    = lLength;
   PBYTE        pbData  = new BYTE[sizeof(MsgDataName) + lLength];
   MsgDataName* pName   = (MsgDataName*) pbData;
   MsgDataSD*   pSD     = (MsgDataSD*) (pName + 1);

   lstrcpy(pName->m_szName, m_pszRobot);
   if (!MakeSelfRelativeSD(pSecurityDescriptor, &pSD->m_sdSecurity, &lLen)) {
      CopyMemory(&pSD->m_sdSecurity, pSecurityDescriptor, lLength);
   }

   Message msg = {0};
   msg.m_baseMsg.m_lMsgType = ROBOMSG_SETSECURITY;
   msg.m_pvData = pbData;
   msg.m_baseMsg.m_lExtraDataSize = sizeof(MsgDataName)
         + GetSecurityDescriptorLength(&pSD->m_sdSecurity);
   msg.m_baseMsg.m_lInfo = SecurityInformation;
   SendWriteMessage(m_prcState->m_hPipe, &msg);

   delete[] pbData;

   return (S_OK);
}


///////////////////////////////////////////////////////////////////////////////


HANDLE OpenCurrentToken(ULONG lAccess, BOOL fOpenAsSelf) {

   HANDLE hToken = NULL;

   if (!OpenThreadToken(GetCurrentThread(), lAccess, fOpenAsSelf, &hToken) 
         &&GetLastError() == ERROR_NO_TOKEN) {
      if (!OpenProcessToken(GetCurrentProcess(), lAccess, &hToken)) {
         hToken = NULL;
      }
   }

   return (hToken);
}


///////////////////////////////////////////////////////////////////////////////


PSID GetCurrentSID() {
   
   HANDLE hToken = NULL;
   PSID psid = NULL;
   PTOKEN_USER pUser = NULL;
   
   try { {
   
      hToken = OpenCurrentToken(TOKEN_QUERY, TRUE);
      if (hToken == NULL)
         goto leave;
      
      ULONG lSize = 0;
      GetTokenInformation(hToken, TokenUser, NULL, 0, &lSize);
      pUser = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), 0, lSize);
      if (pUser == NULL)
         goto leave;

      if (!GetTokenInformation(hToken, TokenUser, pUser, lSize, &lSize))
         goto leave;

      // Go through this hoopla because we want the returned SID to be freeable
      // via Freesid()
      if (!AllocateAndInitializeSid(GetSidIdentifierAuthority(pUser->User.Sid),
            *GetSidSubAuthorityCount(pUser->User.Sid),
            *GetSidSubAuthority(pUser->User.Sid, 0),
            *GetSidSubAuthority(pUser->User.Sid, 1),
            *GetSidSubAuthority(pUser->User.Sid, 2),
            *GetSidSubAuthority(pUser->User.Sid, 3),
            *GetSidSubAuthority(pUser->User.Sid, 4),
            *GetSidSubAuthority(pUser->User.Sid, 5),
            *GetSidSubAuthority(pUser->User.Sid, 6),
            *GetSidSubAuthority(pUser->User.Sid, 7),
            &psid)) {
         psid = NULL;
         goto leave;
      }

   } leave:;
   }
   catch (...) {
   }

   if (pUser != NULL)
      HeapFree(GetProcessHeap(), 0, pUser);
   
   if (hToken != NULL)
      CloseHandle(hToken);
   
   return (psid);
}


///////////////////////////////////////////////////////////////////////////////


void HandleTakeOwnership(HWND hwnd) {

   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   PSID psidCurrent = NULL;

   try { {

      int nCur = ListView_GetNextItem(prcState->m_hwndList, -1, LVNI_SELECTED);
      if (nCur == -1)
         goto leave;

      TCHAR szName[256];
      ListView_GetItemText(prcState->m_hwndList, nCur, 0, szName, 
            chDIMOF(szName));

      psidCurrent = GetCurrentSID();
      if (psidCurrent == NULL)
         goto leave;

      SECURITY_DESCRIPTOR sd;
      if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
         goto leave;

      if (!SetSecurityDescriptorOwner(&sd, psidCurrent, FALSE))
         goto leave;

      ULONG lLength = GetSecurityDescriptorLength(&sd);
      PBYTE pbData = new BYTE[sizeof(MsgDataName) + lLength];
      MsgDataName* pName = (MsgDataName*) pbData;
      MsgDataSD* pSD = (MsgDataSD*) (pName + 1);
      lstrcpy(pName->m_szName, szName);
      ULONG lLen = lLength;
      if (!MakeSelfRelativeSD(&sd, &pSD->m_sdSecurity, &lLen)) {
         CopyMemory(&pSD->m_sdSecurity, &sd, lLength);
      }

      Message msg = {0};
      msg.m_baseMsg.m_lMsgType = ROBOMSG_SETSECURITY;
      msg.m_pvData = pbData;
      msg.m_baseMsg.m_lExtraDataSize = sizeof(MsgDataName) 
            + GetSecurityDescriptorLength(&pSD->m_sdSecurity);
      msg.m_baseMsg.m_lInfo = OWNER_SECURITY_INFORMATION;
      SendWriteMessage(prcState->m_hPipe, &msg);

      delete[] pbData;

   } leave:;
   }
   catch (...) {
   }

   if (psidCurrent != NULL)
      FreeSid(psidCurrent);
}


///////////////////////////////////////////////////////////////////////////////


void HandleReadMessage(RoboClientState* prcState, Message* pMsg) {
   
   switch (pMsg->m_baseMsg.m_lMsgType) {

      case ROBOMSG_ROBOTNAME: {

            MsgDataName* pName = (MsgDataName*) pMsg->m_pvData;

            LVITEM lvItem = {0};
            lvItem.mask = LVIF_TEXT;
            lvItem.pszText = pName->m_szName;
            ListView_InsertItem(prcState->m_hwndList, &lvItem);
         }
         break;
      
      case ROBOMSG_ROBOTMSG: {

            PTSTR pszMsg = (PTSTR) pMsg->m_pvData;
            MessageBox(prcState->m_hwndDlg, pszMsg, TEXT("RoboMessage"), 
                  MB_OK);
         }
         break;
      
      case ROBOMSG_ERR: {

            PTSTR pszText = NULL;
            switch (pMsg->m_baseMsg.m_lInfo) {

               case ROBOERROR_NAMEEXISTS:
                  pszText = TEXT("Name exists!");
                  break;
               
               case ROBOERROR_ROBOTNOTFOUND:
                  pszText = TEXT("Robot not found!");
                  break;
               
               case ROBOERROR_ACCESSDENIED:
                  pszText = TEXT("Access denied!");
                  break;
            }
            MessageBox(prcState->m_hwndDlg, pszText, TEXT("Robo-Error"), 
                  MB_OK);
         }
         break;

      case ROBOMSG_ROBOTREMOVED: {

            MsgDataName* pName = (MsgDataName*) pMsg->m_pvData;

            LVFINDINFO lvFindInfo = {0};
            lvFindInfo.flags = LVFI_STRING;
            lvFindInfo.psz = pName->m_szName;

            int nItem = ListView_FindItem(prcState->m_hwndList, -1, &lvFindInfo);
            if (nItem >= 0)
               ListView_DeleteItem(prcState->m_hwndList, nItem);

         }
         break;

      case ROBOMSG_RETURNSECURITY: {

            MsgDataName* pName = (MsgDataName*) pMsg->m_pvData;
            MsgDataSD* pSD = (MsgDataSD*) (pName + 1);

            CSecurityInformation* pSec = new CSecurityInformation(prcState, 
                  pName->m_szName, &pSD->m_sdSecurity);
            
            // Common dialog box for ACL editing
            EditSecurity(prcState->m_hwndDlg, pSec);
            if (pSec != NULL)
                  pSec->Release();
         }
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


ULONG WINAPI ReadThread(RoboClientState* prcState) {

   // All reading done here
   HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   
   do {

      ULONG lRead;
      Message msg;
      PBYTE pbData = NULL;
      OVERLAPPED ovl = {0};

      // Read message base
      ZeroMemory(&ovl, sizeof(ovl));
      ovl.hEvent = hEvent;
      BOOL fSuccess = ReadFile(prcState->m_hPipe, &msg, MSGSIZE, &lRead, &ovl);
      if (!fSuccess && GetLastError() == ERROR_IO_PENDING) {
         fSuccess = GetOverlappedResult(prcState->m_hPipe, &ovl, &lRead, TRUE);
      }
      
      // Maybe read message data
      if (fSuccess && msg.m_baseMsg.m_lExtraDataSize > 0) {

         pbData = new BYTE[msg.m_baseMsg.m_lExtraDataSize];
         ZeroMemory(&ovl, sizeof(ovl));
         ovl.hEvent = hEvent;
         fSuccess = ReadFile(prcState->m_hPipe, pbData, 
               msg.m_baseMsg.m_lExtraDataSize, NULL, &ovl);
         if (!fSuccess && GetLastError() == ERROR_IO_PENDING) {
            fSuccess = GetOverlappedResult(prcState->m_hPipe, &ovl, &lRead, 
                  TRUE);
         }
      }
      
      // If it was an overall failure we outta here
      if (!fSuccess) {
         
         if (pbData!= NULL)
            delete[] pbData;
         
         if (prcState->m_hPipe) // If the pipe still needs killing
            FORWARD_WM_COMMAND(prcState->m_hwndDlg, IDB_CONNECT, 
                  GetDlgItem(prcState->m_hwndDlg, IDB_CONNECT), BN_CLICKED, 
                  PostMessage);
         break;
      
      } else { // otherwise handle the message
         
         msg.m_pvData = pbData;
      
         try { HandleReadMessage(prcState, &msg); }
         catch (...) { }
      }

      // Cleanup
      if (pbData!= NULL)
         delete[] pbData;

   // And do it again
   } while (prcState->m_hPipe!= NULL);
   
   if (hEvent != NULL)
      CloseHandle(hEvent);

   return (0);
}


///////////////////////////////////////////////////////////////////////////////


void CleanupReadThread(RoboClientState* prcState) {

   WaitForSingleObject(prcState->m_hThread, INFINITE);
   CloseHandle(prcState->m_hThread);
   prcState->m_hThread = NULL;
}


///////////////////////////////////////////////////////////////////////////////


void HandleRename(HWND hwnd) {
   
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   int nCur = ListView_GetNextItem(prcState->m_hwndList, -1, LVNI_SELECTED);
   if (nCur >= 0) {
      SetFocus(prcState->m_hwndList);
      ListView_EditLabel(prcState->m_hwndList, nCur);
   }
}


///////////////////////////////////////////////////////////////////////////////


void HandleConnect(HWND hwnd) {
   
   // Get state info
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   if (prcState->m_hPipe == NULL) { // connect 

      TCHAR szServer[1024];

      lstrcpy(szServer, TEXT("\\\\"));
      GetDlgItemText(hwnd, IDE_SERVER, &szServer[2], chDIMOF(szServer));
      if (szServer[2] == 0)
         lstrcat(szServer, TEXT("."));

      lstrcat(szServer, PIPENAME);

      prcState->m_hPipe = CreateFile(szServer, FILE_GENERIC_READ
            | FILE_GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
      if (prcState->m_hPipe != INVALID_HANDLE_VALUE) {
      
         prcState->m_hThread = chBEGINTHREADEX(NULL, 0, ReadThread, prcState,
               0, NULL);
         if (prcState->m_hThread == NULL) {
            CloseHandle(prcState->m_hPipe);
            prcState->m_hPipe = NULL;
         }

      } else {
         
         MessageBox(hwnd, TEXT("Unable to connect to the RoberService sample ")
               TEXT("service.  \nCheck to see if the service is installed ")
               TEXT("and running."), TEXT("RoboClient Error"), MB_OK);
      }

      if (prcState->m_hThread!= NULL) {
         SendSimpleMessage(hwnd, ROBOMSG_QUERYROBOTNAMES);
         EnableControls(hwnd, TRUE);
         SetDlgItemText(hwnd, IDB_CONNECT, TEXT("Disconnect"));
      }

   } else { // disconnect

      HANDLE hPipeTemp = prcState->m_hPipe;
      prcState->m_hPipe = NULL;
      CancelIo(hPipeTemp);
      CloseHandle(hPipeTemp);
      CleanupReadThread(prcState);
      EnableControls(hwnd, FALSE);
      SetDlgItemText(hwnd, IDB_CONNECT, TEXT("Connect"));
   }

}


///////////////////////////////////////////////////////////////////////////////


void HandleAdd(HWND hwnd) {

   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   LVITEM lvItem = {0};
   lvItem.mask = LVIF_TEXT;
   lvItem.pszText = TEXT("");
   int nCur = ListView_InsertItem(prcState->m_hwndList, &lvItem);
   if (nCur >= 0) {
      SetFocus(prcState->m_hwndList);
      ListView_EditLabel(prcState->m_hwndList, nCur);
   }

}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   switch (id) {

      case IDB_CONNECT:
         HandleConnect(hwnd);
         break;

      case IDB_GATHER:
         SendRobotMessage(hwnd, ROBOMSG_ACTION, ROBOACTION_GATHER);
         break;

      case IDB_ASSEMBLE:
         SendRobotMessage(hwnd, ROBOMSG_ACTION, ROBOACTION_ASSEMBLE);
         break;

      case IDB_LOCK:
         SendRobotMessage(hwnd, ROBOMSG_LOCK, TRUE);
         break;

      case IDB_UNLOCK:
         SendRobotMessage(hwnd, ROBOMSG_LOCK, FALSE);
         break;

      case IDB_QUERY:
         SendRobotMessage(hwnd, ROBOMSG_QUERY, FALSE);
         break;

      case IDB_REMOVE:
         SendRobotMessage(hwnd, ROBOMSG_DELETEROBOT, 0);
         break;

      case IDB_ADD:
         HandleAdd(hwnd);
         break;

      case IDB_RENAME:
         HandleRename(hwnd);
         break;

      case IDB_SECURITY:
         SendRobotMessage(hwnd, ROBOMSG_QUERYSECURITY, DACL_SECURITY_INFORMATION 
            | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION
            | SACL_SECURITY_INFORMATION);
         break;

      case IDB_TAKEOWNERSHIP:
         HandleTakeOwnership(hwnd);
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnClose(HWND hwnd) {
   // Get state info
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   if (prcState->m_hPipe != NULL) { // disconnect
      HandleConnect(hwnd);
   }
   EndDialog(hwnd, 0);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnDestroy(HWND hwnd) {

   // Get state info
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   if (prcState->m_pResizer != NULL)
      delete prcState->m_pResizer;

   delete prcState;
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnSize(HWND hwnd, UINT state, int cx, int cy) {

   // Get state info
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   // Simply call the adjustcontrols function of our handy resizer class
   prcState->m_pResizer->AdjustControls(cx, cy);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo) {

   // Get state info
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   // Just calling another resizer function
   prcState->m_pResizer->HandleMinMax(lpMinMaxInfo);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmhdr) {

   // Get state info
   RoboClientState* prcState = (RoboClientState*) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   switch (pnmhdr->code) {

      case LVN_ENDLABELEDIT: {

            LPNMLVDISPINFOW pnmlvDispInfo = (LPNMLVDISPINFOW) pnmhdr;

            TCHAR szOldName[256] = {0};
            ListView_GetItemText(pnmlvDispInfo->hdr.hwndFrom, 
                  pnmlvDispInfo->item.iItem, 0, szOldName, chDIMOF(szOldName));

            BOOL fNew = (szOldName[0] == 0);
            BOOL fEdited = (pnmlvDispInfo->item.pszText != NULL) 
                  && (pnmlvDispInfo->item.pszText[0] != 0);
            if (fNew) {

               if (fEdited) {
                  LVITEM lvItem = {0};
                  if (lstrlen(pnmlvDispInfo->item.pszText) > 255)
                     pnmlvDispInfo->item.pszText[255] = 0;
                  lvItem.mask = LVIF_TEXT;
                  lvItem.iItem = pnmlvDispInfo->item.iItem;
                  lvItem.iSubItem = 0;
                  lvItem.pszText = pnmlvDispInfo->item.pszText;
                  ListView_SetItem(pnmlvDispInfo->hdr.hwndFrom, &lvItem);
                  SendRobotMessage(hwnd, ROBOMSG_CREATEROBOT, 0);
               }
               ListView_DeleteItem(pnmlvDispInfo->hdr.hwndFrom, 
                     pnmlvDispInfo->item.iItem);

            } else {

               if (fEdited) {
                  Message msg;
                  MsgDataName MsgDataNames[2];

                  lstrcpy(MsgDataNames[0].m_szName, szOldName);
                  lstrcpy(MsgDataNames[1].m_szName, 
                        pnmlvDispInfo->item.pszText);

                  msg.m_baseMsg.m_lMsgType = ROBOMSG_CHANGENAME;
                  msg.m_pvData = MsgDataNames;
                  msg.m_baseMsg.m_lExtraDataSize = sizeof(MsgDataNames);
                  // Rename
                  SendWriteMessage(prcState->m_hPipe, &msg);
               }

            }
         }
         break;

      case LVN_ITEMCHANGED:
         EnableControls(hwnd, TRUE);
         break;
   }

   return (FALSE);
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg) {

      chHANDLE_DLGMSG(hwndDlg, WM_INITDIALOG, Dlg_OnInitDialog);
      chHANDLE_DLGMSG(hwndDlg, WM_CLOSE, Dlg_OnClose);
      chHANDLE_DLGMSG(hwndDlg, WM_DESTROY, Dlg_OnDestroy);
      chHANDLE_DLGMSG(hwndDlg, WM_SIZE, Dlg_OnSize);
      chHANDLE_DLGMSG(hwndDlg, WM_GETMINMAXINFO, Dlg_OnGetMinMaxInfo);
      chHANDLE_DLGMSG(hwndDlg, WM_COMMAND, Dlg_OnCommand);
      chHANDLE_DLGMSG(hwndDlg, WM_NOTIFY, Dlg_OnNotify);
   }

   return (FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, LPTSTR pszCmdLine, int) {

   g_hInst = hinstExe;

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_ROBOCLIENT), NULL, Dlg_Proc);
   return (0);
}


///////////////////////////////// End of File /////////////////////////////////