/******************************************************************************
Module:  RoboService.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"              /* See Appendix A. */

#include <Process.h>
#include <LMAccess.h>

#define SERVICESTATUS_IMPL
#include "..\03-TimeService\ServiceStatus.h"

#define AUTOBUF_IMPL
#include "..\ClassLib\AutoBuf.h"

// Control the services behavior
#define THREADPOOL         10
#define MAXCONNECTIONS     100
#define PENDINGCONNECTIONS 10

// Server messages
#define ROBOMSG_ERR             3

#define ROBOMSG_QUERYROBOTNAMES 10
#define ROBOMSG_ROBOTNAME       11  // Requires MsgDataName extra data

#define ROBOMSG_CREATEROBOT     12  // Requires MsgDataName extra data
#define ROBOMSG_DELETEROBOT     13  // Requires MsgDataName extra data
#define ROBOMSG_ROBOTREMOVED    14  // Requires MsgDataName extra data

#define ROBOMSG_CHANGENAME      15  // Requires an array of two MsgDataName

#define ROBOMSG_LOCK            16  // Requires MsgDataName extra data

#define ROBOMSG_QUERY           17  // Requires MsgDataName extra data
#define ROBOMSG_ROBOTMSG        18  // Requires a following string

#define ROBOMSG_ACTION          19  // Requires MsgDataName extra data

#define ROBOMSG_QUERYSECURITY   20  // Requires MsgDataName extra data
#define ROBOMSG_RETURNSECURITY  21  // Requires MsgDataSD extra data

#define ROBOMSG_SETSECURITY     22  // Requires MsgDataName and MsgDataSD
                                    // extra data

// Error defines
#define ROBOERROR_NAMEEXISTS    1
#define ROBOERROR_ROBOTNOTFOUND 2
#define ROBOERROR_ACCESSDENIED  3

// Action defines
#define ROBOACTION_GATHER       1
#define ROBOACTION_ASSEMBLE     2

// IO purposes
#define IOS_CONNECT  1
#define IOS_READ     2
#define IOS_WRITE    3

// IOCP Context defines
#define CONTEXT_SERVICE 1
#define CONTEXT_IO      2

// Service control defines
#define SERVICE_CONTROL    1
#define SERVICE_EXITTHREAD 2

#define PIPENAME TEXT("\\\\.\\Pipe\\RoboService")

// Service name
TCHAR g_szServiceName[] = 
   TEXT("Programming Server-Side Applications RoboService");

// Service status global class instance
CServiceStatus g_ssRobo;

// What we can do to a robot
#define ROBOT_SETNAME      (0x0001)    // Change its name 
#define ROBOT_LOCK         (0x0002)    // Lock it 
#define ROBOT_GATHER       (0x0004)    // Gather Material 
#define ROBOT_ASSEMBLE     (0x0008)    // Assemble Material 
#define ROBOT_QUERY        (0x0010)    // Query Status 
#define ROBOT_OVERRIDELOCK (0x0020)    // Unlock it (even if not the "locker")
#define ROBOT_ALL_ACCESS   (STANDARD_RIGHTS_REQUIRED \
                           | ROBOT_SETNAME \
                           | ROBOT_LOCK \
                           | ROBOT_GATHER \
                           | ROBOT_ASSEMBLE \
                           | ROBOT_QUERY \
                           | ROBOT_OVERRIDELOCK)

// Generic mappings for robots
GENERIC_MAPPING g_gmRobots = { ROBOT_QUERY, ROBOT_SETNAME, ROBOT_LOCK 
   | ROBOT_GATHER | ROBOT_ASSEMBLE, ROBOT_ALL_ACCESS };

// Self linked node containing robot information
typedef struct _Robot Robot;
typedef struct _Robot {
   TCHAR    m_szName[256];
   PSID     m_psidLockee; // NULL = unlocked
   PSECURITY_DESCRIPTOR m_pSD;
   Robot*   m_pNext;
} Robot;

// Stores state information for each connection
typedef struct _ConnectionInfo {
   LONG     m_lInUse;
   HANDLE   m_hPipe;
   HANDLE   m_hToken;
} ConnectionInfo;

// Stores state information for the service
typedef struct _ServerInfo {
   HANDLE   m_hIOCP;
   HANDLE   m_hThreads[THREADPOOL + 1];
   CRITICAL_SECTION m_csSerialize;
   LONG     m_lActiveConnections;
   LONG     m_lPendingConnections;
   SECURITY_ATTRIBUTES m_saPipeSecurity;
   ConnectionInfo m_infoConnections[MAXCONNECTIONS];
   Robot*   m_pFirstRobot;

} ServerInfo;

// The base message
typedef struct _MessageBase {
   ULONG m_lMsgType;
   ULONG m_lInfo;
   ULONG m_lExtraDataSize;
} MessageBase;
#define MSGSIZE sizeof(MessageBase)

// Structure used to receive messages
typedef struct _MessageReceiver {
   MessageBase m_baseMsg;
   ULONG m_lMsgBytesRead;
   ULONG m_lDataBytesRead;
   PBYTE m_pbData;
} MessageReceiver;

// Structure used to send messages
typedef struct _MessageSender {
   MessageBase m_baseMsg;
   BYTE        m_bData[1]; // Place holder for the "data block"
} MessageSender;

// A data structure for a message with a robot name in the data
typedef struct _MsgDataName {
   TCHAR m_szName[256];
} MsgDataName;

// A data structure for a message with a security descriptor
typedef struct _MsgDataSD {
   SECURITY_DESCRIPTOR m_sdSecurity;
} MsgDataSD;

// This structure wraps OVERLAPPED and a message buffer
typedef struct _IOStruct:OVERLAPPED {
   int m_nPurpose;
   ConnectionInfo* m_pinfoConnection;
   union {
      MessageReceiver*  m_pMessageRcv;
      MessageSender*    m_pMessageSnd;
   };
} IOStruct;


///////////////////////////////////////////////////////////////////////////////


HANDLE OpenCurrentToken(ULONG lAccess, BOOL fOpenAsSelf) {

   HANDLE hToken = NULL;

   if (!OpenThreadToken(GetCurrentThread(), lAccess, fOpenAsSelf, &hToken) 
         && GetLastError() == ERROR_NO_TOKEN) {
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
   PTOKEN_USER pUser = {0};
   
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
      // via freesid()
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


void RemoveRobot(ServerInfo* pInfo, Robot* pRobot) {

   if (pInfo->m_pFirstRobot == pRobot) {
      pInfo->m_pFirstRobot = pRobot->m_pNext;
   } else {
      Robot* pTemp = pInfo->m_pFirstRobot;
      while (pTemp->m_pNext != pRobot) {
         pTemp = pTemp->m_pNext;
      }
      pTemp->m_pNext = pRobot->m_pNext;
   }

   if (pRobot->m_psidLockee != NULL)
      FreeSid(pRobot->m_psidLockee);
   if (pRobot->m_pSD != NULL)
      DestroyPrivateObjectSecurity(&(pRobot->m_pSD));
   HeapFree(GetProcessHeap(), 0, pRobot);
}


///////////////////////////////////////////////////////////////////////////////


void AddRobot(ServerInfo* pInfo, PTSTR szRobot) {

   Robot* pTemp;
   Robot* pNew = NULL;
   BOOL fReturn = FALSE;
   HANDLE hToken = NULL;

   try { {

      pNew = (Robot*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
            sizeof(Robot));
      if (pNew == NULL)
         goto leave;
      lstrcpy(pNew->m_szName, szRobot);

      hToken = OpenCurrentToken(TOKEN_QUERY, TRUE);
      if (hToken == NULL)
         goto leave;

      // Back to service context
      RevertToSelf();
      PSECURITY_DESCRIPTOR pSD;
      
      // Create default security for the robot
      if (!CreatePrivateObjectSecurity(NULL, NULL, &pSD, FALSE, hToken, 
            &g_gmRobots)) {
         ImpersonateLoggedOnUser(hToken);
         goto leave;
      }
      
      // Now to client context
      ImpersonateLoggedOnUser(hToken);

      pNew->m_pSD = pSD;

      pTemp = pInfo->m_pFirstRobot;
      if (pTemp == NULL)
         pInfo->m_pFirstRobot = pNew;
      else {
         while (pTemp->m_pNext != NULL)
            pTemp = pTemp->m_pNext;
         pTemp->m_pNext = pNew;
      }

      fReturn = TRUE;
   
   } leave:;
   }
   catch (...) {
   }
   
   if (!fReturn && pNew != NULL)
      HeapFree(GetProcessHeap(), 0, pNew);

   if (hToken != NULL)
      CloseHandle(hToken);
}


///////////////////////////////////////////////////////////////////////////////


Robot* FindRobot(ServerInfo* pInfo, PTSTR szName) {

   Robot* pTemp = pInfo->m_pFirstRobot;
   while (pTemp!= NULL) {
      if (lstrcmp(szName, pTemp->m_szName) == 0)
         break;
      pTemp = pTemp->m_pNext;
   }

   return (pTemp);
}


///////////////////////////////////////////////////////////////////////////////


void InitRobots(ServerInfo* pInfo) {
   
   PTSTR szInitRobots[] = { TEXT("Inky"), TEXT("Pinky"), TEXT("Blinky"), 
         TEXT("Clyde"), TEXT("Akabei"), TEXT("Aosuke"), TEXT("Guzuta"), 
         TEXT("Sue"), TEXT("Tim") };

   // Add some starting robots
   int nIndex = chDIMOF(szInitRobots);
   while (nIndex-- != 0) {
      AddRobot(pInfo, szInitRobots[nIndex]);
   }
}


///////////////////////////////////////////////////////////////////////////////


DWORD WINAPI RoboHandlerEx(DWORD dwControl, DWORD dwEventType,
      PVOID pvEventData, PVOID pvContext) {

   DWORD dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
   BOOL fPostControlToServiceThread = FALSE;

   // Handle service control notifications
   switch (dwControl) {
      
      case SERVICE_CONTROL_STOP:
      case SERVICE_CONTROL_SHUTDOWN:
         g_ssRobo.SetUltimateState(SERVICE_STOPPED, 2000);
         fPostControlToServiceThread = TRUE;
         break;

      case SERVICE_CONTROL_PAUSE:
      case SERVICE_CONTROL_CONTINUE:
         break;

      case SERVICE_CONTROL_INTERROGATE:
         g_ssRobo.ReportStatus();
         break;

      case SERVICE_CONTROL_PARAMCHANGE:
         break;

      case SERVICE_CONTROL_DEVICEEVENT:
      case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
      case SERVICE_CONTROL_POWEREVENT:
         break;
   }

   HANDLE hIOCP = (HANDLE) pvContext;
   if (fPostControlToServiceThread) {
      // The Handler thread is very simple and executes very quickly because
      // it just passes the control code off to the servicemain thread.
      PostQueuedCompletionStatus(hIOCP, SERVICE_CONTROL, CONTEXT_SERVICE, 
            NULL);
      dwReturn = NO_ERROR;
   }

   return(dwReturn);
}


///////////////////////////////////////////////////////////////////////////////


// These keep track of the current allocated blocks of each type,
// They are not used and are only a sanity check for debugging
#ifdef _DEBUG
long g_nMessageData = 0;
long g_nMessage = 0;
long g_nIOStruct = 0;
#endif


///////////////////////////////////////////////////////////////////////////////


BOOL AllocateMessageData(MessageReceiver* pmsg) {

   pmsg->m_pbData = (PBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
         pmsg->m_baseMsg.m_lExtraDataSize);
   
   #ifdef _DEBUG // sanity check
   InterlockedIncrement(&g_nMessageData);
   #endif
   
   return (pmsg->m_pbData != NULL);
}


///////////////////////////////////////////////////////////////////////////////


inline void FreeMessageData(MessageReceiver* pmsg) {
   
   HeapFree(GetProcessHeap(), 0, pmsg->m_pbData);
   
   #ifdef _DEBUG
   InterlockedDecrement(&g_nMessageData);
   #endif
   
   pmsg->m_pbData = NULL;
}


///////////////////////////////////////////////////////////////////////////////


inline MessageReceiver* AllocateMessageRcv() {
   
   #ifdef _DEBUG
   InterlockedIncrement(&g_nMessage);
   #endif
   
   return (MessageReceiver*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
         sizeof(MessageReceiver));
}


///////////////////////////////////////////////////////////////////////////////


void FreeMessageRcv(MessageReceiver* pmsg) {
   
   // If there is data, free it too
   if (pmsg->m_pbData != NULL)
      FreeMessageData(pmsg);
   
   #ifdef _DEBUG
   InterlockedDecrement(&g_nMessage);
   #endif
   
   HeapFree(GetProcessHeap(), 0, pmsg);
}


///////////////////////////////////////////////////////////////////////////////


MessageSender* AllocateMessageSnd(ULONG lType, PVOID pvData, int nDataSize) {
   
   MessageSender* pMsg = (MessageSender*) HeapAlloc(GetProcessHeap(), 
         HEAP_ZERO_MEMORY, sizeof(MessageBase) + nDataSize);
   pMsg->m_baseMsg.m_lExtraDataSize = nDataSize;
   pMsg->m_baseMsg.m_lMsgType = lType;
   CopyMemory(pMsg->m_bData, pvData, nDataSize);
   
   #ifdef _DEBUG
   InterlockedIncrement(&g_nMessage);
   #endif
   
   return (pMsg);
}


///////////////////////////////////////////////////////////////////////////////


inline MessageSender* CopyMessageSnd(MessageSender* pmsg) {

   return (AllocateMessageSnd(pmsg->m_baseMsg.m_lMsgType, 
         pmsg->m_baseMsg.m_lExtraDataSize?pmsg->m_bData:NULL, 
         pmsg->m_baseMsg.m_lExtraDataSize));
}


///////////////////////////////////////////////////////////////////////////////


inline void FreeMessageSnd(MessageSender* pmsg) {

   HeapFree(GetProcessHeap(), 0, pmsg);
   
   #ifdef _DEBUG
   InterlockedDecrement(&g_nMessage);
   #endif
}


///////////////////////////////////////////////////////////////////////////////


LPOVERLAPPED AllocateIOStruct(int nPurpose, ConnectionInfo* pinfo, 
      MessageReceiver* pmsg = NULL) {

   IOStruct* pIO;

   #ifdef _DEBUG
   InterlockedIncrement(&g_nIOStruct);
   #endif
   
   // Get memory for an IO struct and attach the message
   pIO = (IOStruct*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
         sizeof(IOStruct));
   if (pIO != NULL) {
      pIO->m_nPurpose = nPurpose;
      pIO->m_pinfoConnection = pinfo;
      pIO->m_pMessageRcv = pmsg;
   }
   
   return (pIO);
}


///////////////////////////////////////////////////////////////////////////////


// The sender version of the function
inline LPOVERLAPPED AllocateIOStruct(int nPurpose, ConnectionInfo* pinfo, 
      MessageSender* pmsg) {

   return (AllocateIOStruct(nPurpose, pinfo, (MessageReceiver*) pmsg));
}


///////////////////////////////////////////////////////////////////////////////


void FreeIOStruct(IOStruct* pIO) {
   
   // If there is a message then free it too
   if (pIO->m_pMessageRcv!= NULL) {
      if (pIO->m_nPurpose == IOS_READ) {
         FreeMessageRcv(pIO->m_pMessageRcv);
      } else {
         FreeMessageSnd(pIO->m_pMessageSnd);
      }
   }

   #ifdef _DEBUG
   InterlockedDecrement(&g_nIOStruct);
   #endif

   HeapFree(GetProcessHeap(), 0, pIO);
}


///////////////////////////////////////////////////////////////////////////////


void CleanupConnection(ConnectionInfo* pinfo) {
   
   // Done with the pipe
   if (pinfo->m_hPipe != NULL) {
      CloseHandle(pinfo->m_hPipe);
      pinfo->m_hPipe = NULL;
   }
   
   // If we ever got a token then we close it
   if (pinfo->m_hToken!= NULL) {
      CloseHandle(pinfo->m_hToken);
      pinfo->m_hToken = NULL;
   }
   
   // Now you may use the connection again
   InterlockedDecrement(&(pinfo->m_lInUse));
}


///////////////////////////////////////////////////////////////////////////////


void UpdatePendingConnections(ServerInfo* pinfo) {

   // It is possible for us to end up with more then 10
   // Pending connections since we don't serialize here
   while ((pinfo->m_lPendingConnections < PENDINGCONNECTIONS)
         && (pinfo->m_lPendingConnections + pinfo->m_lActiveConnections) 
         < MAXCONNECTIONS) {

      // Find a free connection struct
      int nIndex;
      nIndex = MAXCONNECTIONS;
      while (nIndex-- != 0) {
         
         if (pinfo->m_infoConnections[nIndex].m_lInUse == 0) {
            
            LONG lResult = InterlockedIncrement(
                  &(pinfo->m_infoConnections[nIndex].m_lInUse));
            
            // if greater than one, then contention here, and I lost
            if (lResult > 1) {
               
               // So decrement and move on
               InterlockedDecrement(
                     &(pinfo->m_infoConnections[nIndex].m_lInUse));
            } else
               break; // Found one, and own it
         }
      }

      if (nIndex!= -1) { // Did we find a free connection struct?
         
         // Lets get a pipe and add it to the port
         pinfo->m_infoConnections[nIndex].m_hPipe = CreateNamedPipe(PIPENAME,
               PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE 
               | PIPE_READMODE_BYTE | PIPE_WAIT, MAXCONNECTIONS, 0, 0, 1000, 
               &pinfo->m_saPipeSecurity);
         
         if (pinfo->m_infoConnections[nIndex].m_hPipe 
               != INVALID_HANDLE_VALUE) {
            
            if (CreateIoCompletionPort(
                  pinfo->m_infoConnections[nIndex].m_hPipe,
                  pinfo->m_hIOCP, CONTEXT_IO, 0) != NULL) {
               
               InterlockedIncrement(&(pinfo->m_lPendingConnections));
               LPOVERLAPPED pOvl = AllocateIOStruct(IOS_CONNECT, 
                     &pinfo->m_infoConnections[nIndex]);
               
               if (pOvl!= NULL) { // Lets get it listening
                  ConnectNamedPipe(pinfo->m_infoConnections[nIndex].m_hPipe, 
                        pOvl);
               }
               else { // Ooops cleanup then
                  CleanupConnection(&pinfo->m_infoConnections[nIndex]);
                  InterlockedDecrement(&(pinfo->m_lPendingConnections));
               }

            } else { // Oops cleanup then
               CleanupConnection(&pinfo->m_infoConnections[nIndex]);
               InterlockedDecrement(&(pinfo->m_lPendingConnections));
            }

         }

      } else { // If not, lets sleep and try again, we will soon
         Sleep(1000);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////


void PostReadMessageData(ServerInfo* pInfo, IOStruct* pIOS) {

   // Get a message data block allocated
   if (pIOS->m_pMessageRcv->m_pbData == NULL) {
      AllocateMessageData(pIOS->m_pMessageRcv);
   }
   
   // Clear it
   ZeroMemory((LPOVERLAPPED) pIOS, sizeof(OVERLAPPED));
   
   // Read the data
   BOOL fSuccess = ReadFile(pIOS->m_pinfoConnection->m_hPipe, 
         pIOS->m_pMessageRcv->m_pbData, 
         pIOS->m_pMessageRcv->m_baseMsg.m_lExtraDataSize, NULL, pIOS);
   if (!fSuccess && GetLastError() == ERROR_BROKEN_PIPE) {
      CleanupConnection(pIOS->m_pinfoConnection); // Broken pipes mean goodbye
      InterlockedDecrement(&(pInfo->m_lActiveConnections));
      FreeIOStruct(pIOS);
   }
}


///////////////////////////////////////////////////////////////////////////////


void PostReadMessage(ServerInfo* pInfo, IOStruct* pIOS, 
      ConnectionInfo* pinfoConnection) {

   // Allocate IO struct and recieve message buffer
   if (pIOS == NULL) {
      MessageReceiver* pmsg = AllocateMessageRcv();
      pIOS = (IOStruct*) AllocateIOStruct(IOS_READ, pinfoConnection, pmsg);
   }
   
   // ReadFile
   BOOL fSuccess = ReadFile(pIOS->m_pinfoConnection->m_hPipe, 
         pIOS->m_pMessageRcv, MSGSIZE, NULL, pIOS);
   if (!fSuccess && GetLastError() == ERROR_BROKEN_PIPE) {
      CleanupConnection(pinfoConnection); // broken pipes mean end connection
      InterlockedDecrement(&(pInfo->m_lActiveConnections));
      FreeIOStruct(pIOS);
   }
}


///////////////////////////////////////////////////////////////////////////////


void PostWriteMessage(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageSender* pMsgNew) {
   
   // Allocate iostruct and attach message, then write it
   IOStruct* pIOS = (IOStruct*) AllocateIOStruct(IOS_WRITE, pinfoConnection, 
      pMsgNew);
   BOOL fSuccess = WriteFile(pIOS->m_pinfoConnection->m_hPipe, 
      pIOS->m_pMessageSnd, MSGSIZE 
      + pIOS ->m_pMessageSnd->m_baseMsg.m_lExtraDataSize, NULL, pIOS);
   ULONG lErr = GetLastError();
   if (!fSuccess && lErr != ERROR_IO_PENDING) {
      FreeIOStruct(pIOS);
   }
}


///////////////////////////////////////////////////////////////////////////////


void PostErrorMessage(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      ULONG lErr) {
   
   MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ERR, NULL, 0);
   pMsgNew->m_baseMsg.m_lInfo = lErr;
   PostWriteMessage(pInfo, pinfoConnection, pMsgNew);
}


///////////////////////////////////////////////////////////////////////////////


void PostWriteMessageToAll(ServerInfo* pInfo, MessageSender* pMsgNew) {
   
   int nIndex = MAXCONNECTIONS;
   MessageSender* pMsgCopy;
   // Copy the message and send it to all conections
   while (nIndex-- != 0) {
      if (pInfo->m_infoConnections[nIndex].m_lInUse != 0 &&
            pInfo->m_infoConnections[nIndex].m_hToken != NULL) {
         pMsgCopy = CopyMessageSnd(pMsgNew);
         PostWriteMessage(pInfo, &(pInfo->m_infoConnections[nIndex]), 
            pMsgCopy);
      }
   }
   FreeMessageSnd(pMsgNew);
}


///////////////////////////////////////////////////////////////////////////////


void PostExitMsgs(ServerInfo* pInfo) {
   
   int nIndex = THREADPOOL;
   while (nIndex-- != 0)
      PostQueuedCompletionStatus(pInfo->m_hIOCP, SERVICE_EXITTHREAD, 
         CONTEXT_SERVICE, NULL);
}


///////////////////////////////////////////////////////////////////////////////


void RemoveService() {

   // Open the SCM on this machine.
   SC_HANDLE hSCM =
      OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

   // Open this service for DELETE access
   SC_HANDLE hService =
      OpenService(hSCM, g_szServiceName, DELETE);

   // Remove this service from the SCM's database.
   DeleteService(hService);

   if (hSCM != NULL)
      CloseServiceHandle(hSCM);

   if (hService != NULL)
      CloseServiceHandle(hService);
}


///////////////////////////////////////////////////////////////////////////////


void QueryRobotNames(ServerInfo* pInfo, ConnectionInfo* pinfoConnection) {
   
   Robot* pTempRobot = pInfo->m_pFirstRobot;
   MessageSender* pMsgNew;
   MsgDataName dataName;

   try {
   
      EnterCriticalSection(&pInfo->m_csSerialize); // serialize
      while (pTempRobot != NULL) { // Loop through all robots and post names
         lstrcpy(dataName.m_szName, pTempRobot->m_szName);
         pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTNAME, &dataName, 
            sizeof(dataName));
         PostWriteMessage(pInfo, pinfoConnection, pMsgNew);
         pTempRobot = pTempRobot->m_pNext;
      }

   }
   catch (...) { }
   
   // Always un serialize
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


void CreateRobot(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
   
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;

   try {
   
      // Serialize
      EnterCriticalSection(&pInfo->m_csSerialize);
      if (FindRobot(pInfo, pName->m_szName)) { // err name exists
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_NAMEEXISTS);
      } else {
         AddRobot(pInfo, pName->m_szName); // Create a new one and tell all
         MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTNAME, pName,
            sizeof(*pName));
         PostWriteMessageToAll(pInfo, pMsgNew);
      }

   }
   catch (...) {
   }

   // Always un serialize
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CheckRobotSecurity(Robot* pRobot, ULONG lAccess) {
   
   BOOL fAllow = FALSE;
   
   // Get current token
   HANDLE hToken = OpenCurrentToken(TOKEN_ALL_ACCESS, TRUE);
   if (hToken != NULL) {
      
      PRIVILEGE_SET psPriv = {0};
      ULONG lPrivs = sizeof(PRIVILEGE_SET);
      ULONG lGranted;
      BOOL fAllowed;
   
      // Run an access check on the requested access
      if (AccessCheck(pRobot->m_pSD, hToken, lAccess, &g_gmRobots, &psPriv, 
            &lPrivs, &lGranted, &fAllowed))
         fAllow = fAllowed;
      CloseHandle(hToken);
   }

   return (fAllow);
}


///////////////////////////////////////////////////////////////////////////////


BOOL LockedOut(Robot* pRobot) {
   
   BOOL fLocked = FALSE;
   PSID psid = NULL;
   
   try { {
   
      // If not locked then return
      if (pRobot->m_psidLockee == NULL)
         goto leave;

      // Get the current sid and check agains lock
      psid = GetCurrentSID();
      if (EqualSid(psid, pRobot->m_psidLockee))
         goto leave;

      fLocked = TRUE;

   } leave:;
   }
   catch (...) {
   }

   // Cleanup
   if (psid != NULL)
      FreeSid(psid);
   
   return (fLocked);
}


///////////////////////////////////////////////////////////////////////////////


void DeleteRobot(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
   
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;

   try { {
   
      // Serialize and find robot
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName->m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // Locked out or not allowed?
      if (LockedOut(pRobot) || !CheckRobotSecurity(pRobot, DELETE)) {

         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }
   
      // Ok then, remove the robot and let everybody know
      RemoveRobot(pInfo, pRobot);
      MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTREMOVED, pName, 
         sizeof(* pName));
      PostWriteMessageToAll(pInfo, pMsgNew);

   } leave:;
   }
   catch (...) {
   }

   // Always leave
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


void ChangeRobotName(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
   
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;

   try { {
   
      // Serialize and find robot
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName[0].m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // If locked out, or no access, then fail
      if (LockedOut(pRobot) || !CheckRobotSecurity(pRobot, ROBOT_SETNAME)) {

         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }
      
      // Otherwise, change the name, and send a message to all for update
      lstrcpy(pRobot->m_szName, pName[1].m_szName);
      MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTREMOVED, 
            &pName[0], sizeof(* pName));
      PostWriteMessageToAll(pInfo, pMsgNew);
      pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTNAME, &pName[1], 
            sizeof(*pName));
      PostWriteMessageToAll(pInfo, pMsgNew);

   } leave:;
   }
   catch (...) {
   }

   // Always exit critical section
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


void RoboQuery(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
   
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;

   try { {
   
      // Serialize and find robot
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName[0].m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // Can we check lock status?
      if (!CheckRobotSecurity(pRobot, ROBOT_QUERY)) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }

      // Build locked text including "locker" name
      CAutoBuf < TCHAR, sizeof(TCHAR) > szResponse;
      if (pRobot->m_psidLockee == NULL) {
         szResponse = sizeof(TEXT("Robot Unlocked")) / sizeof(TCHAR);
         lstrcpy(szResponse, TEXT("Robot Unlocked"));
      }
      else {

         CAutoBuf < TCHAR, sizeof(TCHAR) > szDomain;
         SID_NAME_USE sidUse;
         BOOL fSuccess;
         int nLen = sizeof(TEXT("Robot Locked by ")) / sizeof(TCHAR);
         szResponse = nLen;
         
         do {
            lstrcpy(szResponse, TEXT("Robot Locked by "));
            fSuccess = LookupAccountSid(NULL, pRobot->m_psidLockee, 
                  ((PTSTR) szResponse) + (nLen - 1), 
                  ((PULONG) szResponse) - nLen, szDomain, szDomain, &sidUse);
            szResponse = ((ULONG) szResponse) + nLen;
         } while (!fSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
         
         if (!fSuccess) {
            PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
            goto leave;
         }
      }
      
      // Send the message
      MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTMSG, szResponse,
            (lstrlen(szResponse) + 1) * sizeof(TCHAR));
      PostWriteMessage(pInfo, pinfoConnection, pMsgNew);

   } leave:;
   }
   catch (...) {
   }
   
   // Always exit serialization
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


void RoboAction(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
   
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;
   int nAction = pMsg->m_baseMsg.m_lInfo;

   try { {
   
      // Setup response text and required access
      ULONG lAccessRequired;
      PTSTR pszText;
      switch (nAction) {

         case ROBOACTION_GATHER:
            lAccessRequired = ROBOT_GATHER;
            pszText = TEXT("Gather Command Successful");
            break;
         
         default: // roboaction_assemble:
            lAccessRequired = ROBOT_ASSEMBLE;
            pszText = TEXT("Assemble Command Successful");
            break;
      }

      // Serialize and find robot
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName[0].m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // If locked or no access then fail
      if (LockedOut(pRobot) || !CheckRobotSecurity(pRobot, lAccessRequired)) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }

      // Respond (the robot "actions" don't actually do anything, because
      // We don't actually have any robots :)
      MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTMSG, pszText, 
            (lstrlen(pszText) + 1) * sizeof(TCHAR));
      PostWriteMessage(pInfo, pinfoConnection, pMsgNew);

   } leave:;
   }
   catch (...) {
   }

   // Always exit critical section
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


BOOL TokenHasTakeOwnership() {
   
   BOOL fHas = FALSE;
   
   // Get current token
   HANDLE hToken = OpenCurrentToken(TOKEN_ALL_ACCESS, TRUE);
   if (hToken != NULL) {
   
      PRIVILEGE_SET psPriv = {0};
      psPriv.Control = 0;
      psPriv.PrivilegeCount = 1;
      psPriv.Privilege[0].Attributes = 0;
      LookupPrivilegeValue(NULL, SE_TAKE_OWNERSHIP_NAME, 
            &psPriv.Privilege[0].Luid);

      // Check for the SE_TAKE_OWNERSHIP_NAME privilege
      if (!PrivilegeCheck(hToken, &psPriv, &fHas)) {
         fHas = FALSE;
      }
   
      CloseHandle(hToken);
   }
   
   return (fHas);
}


///////////////////////////////////////////////////////////////////////////////


void SetRobotSecurity(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
   
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;
   MsgDataSD* pSecurity = (MsgDataSD*) (pName + 1);

   try { {
   
      // Serialize and find robot
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName[0].m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // What access are they asking for?
      ULONG lAccess = 0;
      if (pMsg->m_baseMsg.m_lInfo & DACL_SECURITY_INFORMATION)
         lAccess |= WRITE_DAC;
      
      // Override lack of WRITE_OWNER access if they have take ownership priv
      if ((pMsg->m_baseMsg.m_lInfo & OWNER_SECURITY_INFORMATION) 
            && !TokenHasTakeOwnership())
         lAccess |= WRITE_OWNER;
      
      if (pMsg->m_baseMsg.m_lInfo & SACL_SECURITY_INFORMATION)
         lAccess |= ACCESS_SYSTEM_SECURITY;

      // Check the access
      if (!CheckRobotSecurity(pRobot, lAccess)) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }

      // Rever to service context before seting new security
      RevertToSelf();
      BOOL fSuccess = SetPrivateObjectSecurity(pMsg->m_baseMsg.m_lInfo, 
            &(pSecurity->m_sdSecurity), &(pRobot->m_pSD), &g_gmRobots, 
            pinfoConnection->m_hToken);

      // Reimpersonate
      ImpersonateLoggedOnUser(pinfoConnection->m_hToken);
      if (!fSuccess) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      } else {
         PTSTR pszText = TEXT("Robot security successfully set");
         MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTMSG, 
               pszText, (lstrlen(pszText) + 1) * sizeof(TCHAR));
         PostWriteMessage(pInfo, pinfoConnection, pMsgNew);
      }

   } leave:;
   }
   catch (...) {
   }

   // Cleanup and always exit critical section
   LeaveCriticalSection(&pInfo->m_csSerialize);
}


///////////////////////////////////////////////////////////////////////////////


void QueryRobotSecurity(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg) {
 
   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;
   PBYTE pbData = NULL;

   try { {
   
      // Serialize
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName[0].m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // Do we have rights?
      if (!CheckRobotSecurity(pRobot, READ_CONTROL)) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }

      // Get the SD
      CAutoBuf < SECURITY_DESCRIPTOR > pSD;
      BOOL fSuccess;
      
      do {
         fSuccess = GetPrivateObjectSecurity(pRobot->m_pSD, 
               pMsg->m_baseMsg.m_lInfo, pSD, pSD, pSD);
      } while (!fSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
      
      if (!fSuccess) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // Copy it into data to be sent in a message
      ULONG lLength = GetSecurityDescriptorLength(pSD);
      pbData = (PBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
            lLength + sizeof(MsgDataName));
      if (pbData == NULL)
         goto leave;
      CopyMemory(pbData, pName, sizeof(MsgDataName));
      CopyMemory(pbData + sizeof(MsgDataName), pSD, lLength);

      // Send the SD
      MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_RETURNSECURITY, 
            pbData, lLength + sizeof(MsgDataName));
      pMsgNew->m_baseMsg.m_lInfo = pMsg->m_baseMsg.m_lInfo;
      PostWriteMessage(pInfo, pinfoConnection, pMsgNew);

   } leave:;
   }
   catch (...) {
   }
   
   // Cleanup and always exit critical section
   LeaveCriticalSection(&pInfo->m_csSerialize);
   if (pbData != NULL)
      HeapFree(GetProcessHeap(), 0, pbData);
}


///////////////////////////////////////////////////////////////////////////////


void LockRobot(ServerInfo* pInfo, ConnectionInfo* pinfoConnection, 
      MessageReceiver* pMsg, BOOL fLock) {

   MsgDataName* pName = (MsgDataName*) pMsg->m_pbData;
   PSID psid = NULL;

   try { {
   
      // Serialize and find robot
      EnterCriticalSection(&pInfo->m_csSerialize);
      Robot* pRobot = FindRobot(pInfo, pName[0].m_szName);
      if (pRobot == NULL) {
         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ROBOTNOTFOUND);
         goto leave;
      }

      // May we lock this robot?
      if ((!CheckRobotSecurity(pRobot, ROBOT_LOCK)) 
            || ((pRobot->m_psidLockee == NULL) != fLock)) {

         PostErrorMessage(pInfo, pinfoConnection, ROBOERROR_ACCESSDENIED);
         goto leave;
      }

      // Get the current SID to set as the "lock" for the robot
      PTSTR pszText;
      if (fLock) {
         pRobot->m_psidLockee = GetCurrentSID();
         pszText = TEXT("Robot Locked");
      } else {
      
         psid = GetCurrentSID();
         
         // Only the locker can unlock with ROBOT_LOCK access
         if (!EqualSid(psid, pRobot->m_psidLockee)) {
            if (!CheckRobotSecurity(pRobot, ROBOT_OVERRIDELOCK)) {
               PostErrorMessage(pInfo, pinfoConnection, 
                     ROBOERROR_ACCESSDENIED);
               goto leave;
            }
         }

         // Free the lock
         if (pRobot->m_psidLockee != NULL)
            FreeSid(pRobot->m_psidLockee);
         pRobot->m_psidLockee = NULL;
         pszText = TEXT("Robot Unlocked");
      }
      
      // Respond
      MessageSender* pMsgNew = AllocateMessageSnd(ROBOMSG_ROBOTMSG, pszText, 
            (lstrlen(pszText) + 1) * sizeof(TCHAR));
      PostWriteMessage(pInfo, pinfoConnection, pMsgNew);

   } leave:;
   }
   catch (...) {
   }
   
   // Cleanup and always leave critical section
   LeaveCriticalSection(&pInfo->m_csSerialize);
   if (psid != NULL)
      FreeSid(psid);
}


///////////////////////////////////////////////////////////////////////////////


void HandleMsg(ServerInfo* pInfo, MessageReceiver* pMsg, 
      ConnectionInfo* pinfoConnection) {

   // We impersonate the stored token before handling any messages...
   // This is not strictly necessary but demonstrates a server technique
   ImpersonateLoggedOnUser(pinfoConnection->m_hToken);
   switch (pMsg->m_baseMsg.m_lMsgType) {

      case ROBOMSG_QUERYROBOTNAMES:
         QueryRobotNames(pInfo, pinfoConnection);
         break;

      case ROBOMSG_CREATEROBOT:
         CreateRobot(pInfo, pinfoConnection, pMsg);
         break;
      
      case ROBOMSG_DELETEROBOT:
         DeleteRobot(pInfo, pinfoConnection, pMsg);
         break;
      
      case ROBOMSG_CHANGENAME:
         ChangeRobotName(pInfo, pinfoConnection, pMsg);
         break;
      
      case ROBOMSG_LOCK:
         LockRobot(pInfo, pinfoConnection, pMsg, pMsg->m_baseMsg.m_lInfo);
         break;
      
      case ROBOMSG_QUERY:
         RoboQuery(pInfo, pinfoConnection, pMsg);
         break;
      
      case ROBOMSG_ACTION:
         RoboAction(pInfo, pinfoConnection, pMsg);
         break;
      
      case ROBOMSG_QUERYSECURITY:
         QueryRobotSecurity(pInfo, pinfoConnection, pMsg);
         break;
      
      case ROBOMSG_SETSECURITY:
         SetRobotSecurity(pInfo, pinfoConnection, pMsg);
         break;
   }
   
   // Back to the service's security context
   RevertToSelf();
}


///////////////////////////////////////////////////////////////////////////////


void HandleIO(ServerInfo* pInfo, IOStruct* pIOS, ULONG lBytes) {

   switch (pIOS->m_nPurpose) {
   
      case IOS_CONNECT:

         // Update active and downgrade pending
         InterlockedIncrement(&pInfo->m_lActiveConnections);
         InterlockedDecrement(&pInfo->m_lPendingConnections);
         
         // Bring pending connections up to count
         UpdatePendingConnections(pInfo);

         // Get the connection info structure, and then free IO structure
         ConnectionInfo* pinfoConnection;
         pinfoConnection = pIOS->m_pinfoConnection;
         FreeIOStruct(pIOS);

         // Post our first read for reading a message
         PostReadMessage(pInfo, NULL, pinfoConnection);
         break;

      case IOS_WRITE: // When writes complete we just free the IO struct
         FreeIOStruct(pIOS);
         break;
      
      case IOS_READ:
         
         // Store token with connection info if this is our first read.
         if (pIOS->m_pinfoConnection->m_hToken == NULL) {

            BOOL fSuccess = FALSE;
            if (ImpersonateNamedPipeClient(pIOS->m_pinfoConnection->m_hPipe)) {
               HANDLE hToken;
               if (OpenThreadToken(GetCurrentThread(),TOKEN_QUERY 
                     | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_PRIVILEGES 
                     | TOKEN_IMPERSONATE | TOKEN_DUPLICATE 
                     | TOKEN_ADJUST_GROUPS, TRUE, &hToken)) {
                  fSuccess = TRUE;
                  pIOS->m_pinfoConnection->m_hToken = hToken;
               }
               RevertToSelf();
            }
         
            // Can fail and close connection
            if (!fSuccess) {
               CleanupConnection(pIOS->m_pinfoConnection);
               FreeIOStruct(pIOS);
               InterlockedDecrement(&pInfo->m_lActiveConnections);
            }
         }

         // If MSGSIZE has already been read, then we must be reading datablock
         if (pIOS->m_pMessageRcv->m_lMsgBytesRead == MSGSIZE) {
            
            pIOS->m_pMessageRcv->m_lDataBytesRead += lBytes;

            // If we finished data read, then handle the msg and post new read
            if (pIOS->m_pMessageRcv->m_lDataBytesRead 
                  == pIOS->m_pMessageRcv->m_baseMsg.m_lExtraDataSize) {
               
               HandleMsg(pInfo, pIOS->m_pMessageRcv, pIOS->m_pinfoConnection);
               ConnectionInfo* pinfoConnection;
               pinfoConnection = pIOS->m_pinfoConnection;
               FreeIOStruct(pIOS);
            
               // Read next message
               PostReadMessage(pInfo, NULL, pinfoConnection);
            }
         
         } else { // We haven't yet read the main message part
            
            pIOS->m_pMessageRcv->m_lMsgBytesRead += lBytes;

            // If it is completely read then we handle or read for data
            if (pIOS->m_pMessageRcv->m_lMsgBytesRead == MSGSIZE) { 
            
               // finished read
               if (pIOS->m_pMessageRcv->m_baseMsg.m_lExtraDataSize > 0) { 
               
                  // If there is MSG data
                  PostReadMessageData(pInfo, pIOS);
               
               } else { // if there is not MSG data
                  
                  HandleMsg(pInfo, pIOS->m_pMessageRcv,
                        pIOS->m_pinfoConnection);
                  ConnectionInfo* pinfoConnection;
                  pinfoConnection = pIOS->m_pinfoConnection;
                  FreeIOStruct(pIOS);
               
                  // Read next message
                  PostReadMessage(pInfo, NULL, pinfoConnection);
               }
            }
         }
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


BOOL HandleService(ServerInfo* pInfo, ULONG lCmd) {
   
   BOOL fContinue = TRUE;
   switch (lCmd) {

      case SERVICE_CONTROL: // User elected to stop the service
         if (g_ssRobo == SERVICE_STOP_PENDING) {
            g_ssRobo.AdvanceState(20000, 0);

            PostExitMsgs(pInfo);

            fContinue = FALSE;
         }
         break;
      
      // Notification to tell threads to exit for shutdown
      case SERVICE_EXITTHREAD:
         fContinue = FALSE;
         break;
   }

   return (fContinue);
}


///////////////////////////////////////////////////////////////////////////////


ULONG WINAPI RoboThread(ServerInfo* pInfo) {
   
   // Wait for initialization to finish
   EnterCriticalSection(&pInfo->m_csSerialize);
   LeaveCriticalSection(&pInfo->m_csSerialize);

   ULONG lBytes;
   ULONG lKey;
   LPOVERLAPPED pOvl;

   BOOL fContinue;

   // This is the main loop of the service that all threads spin on
   do {
      
      // Get IO
      fContinue = GetQueuedCompletionStatus(pInfo->m_hIOCP, &lBytes, &lKey, 
            &pOvl, INFINITE);
      switch (lKey) {

         case CONTEXT_SERVICE: // Handle service controls
            fContinue = HandleService(pInfo, lBytes);
            break;
         
         case CONTEXT_IO: // handle IO
            
            if (fContinue) { // Successful IO is handled
            
               HandleIO(pInfo, (IOStruct*) pOvl, lBytes);
            
            } else { // Failed IO may mean we end the connection

               ULONG lErr = GetLastError(); 
               IOStruct* pIOS = (IOStruct*) pOvl;
               fContinue = TRUE;

               // End the connection and cleanup if the pipe is broken
               if (lErr == ERROR_BROKEN_PIPE && pIOS->m_nPurpose == IOS_READ) {
                  CleanupConnection(pIOS->m_pinfoConnection);
                  FreeIOStruct(pIOS);
                  InterlockedDecrement(&(pInfo->m_lActiveConnections));
               } else {
                  if (pOvl != NULL)
                     FreeIOStruct(pIOS);
               }
            }
            break;
         
         default: // This should never happen, if it does, stop the service
            fContinue = FALSE;
      }

   } while (fContinue);

   return (0);
}


///////////////////////////////////////////////////////////////////////////////


BOOL InitializePipeSecurity(SECURITY_ATTRIBUTES* pSA) {

   BOOL fReturn = FALSE;
   PSECURITY_DESCRIPTOR pSD = NULL;
   PSID psidOwner = NULL;
   PSID psidEveryone = NULL;

   try { {
   
      // Setup the SECURITY_ATTRIBUTES structure
      pSA->nLength = sizeof(SECURITY_ATTRIBUTES);
      pSA->bInheritHandle = FALSE;
      pSA->lpSecurityDescriptor = NULL;

      // Create a SID for the "Everyone" well-known group
      SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_WORLD_SID_AUTHORITY;
      if (!AllocateAndInitializeSid(&sidAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 
            0, 0, 0, 0, &psidEveryone))
         goto leave;

      // Get the SID for the Token User
      psidOwner = GetCurrentSID();
      if (psidOwner == NULL)
         goto leave;

      // Calculate the size of the SD and ACL needed
      ULONG lSDSize = sizeof(SECURITY_DESCRIPTOR);
      ULONG lACLSize = 0;
      lACLSize += GetLengthSid(psidEveryone);
      lACLSize += GetLengthSid(psidOwner);
      lACLSize += sizeof(ACCESS_ALLOWED_ACE) * 2;
      lACLSize += sizeof(ACL);

      // Allocate the memory
      pSD = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lSDSize + lACLSize);
      if (pSD == NULL)
         goto leave;

      // Setup SD and DACL, and assign DACL to SD
      InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

      PACL pacl = (PACL) (((PBYTE) pSD) + lSDSize);
      InitializeAcl(pacl, lACLSize, ACL_REVISION);
      SetSecurityDescriptorDacl(pSD, TRUE, pacl, FALSE);

      // Add aces to the DACL
      if (!AddAccessAllowedAce(pacl, ACL_REVISION, FILE_GENERIC_READ 
            | FILE_GENERIC_WRITE, psidEveryone))
         goto leave;

      if (!AddAccessAllowedAce(pacl, ACL_REVISION, FILE_ALL_ACCESS, psidOwner))
         goto leave;

      pSA->lpSecurityDescriptor = pSD;
      fReturn = TRUE;

   } leave:;
   }
   catch (...) {
   }

   // Cleanup
   if (psidOwner != NULL)
      FreeSid(psidOwner);

   if (psidEveryone != NULL)
      FreeSid(psidEveryone);

   if (!fReturn &&pSD != NULL)
      HeapFree(GetProcessHeap(), 0, pSD);

   return (fReturn);
}


///////////////////////////////////////////////////////////////////////////////


void WINAPI RoboServiceMain(DWORD dwArgc, PTSTR* pszArgv) {

   // This structure maintains "global-style" state information for the service
   ServerInfo info = {0};

   try { {
   
      // Create our robots
      InitRobots(&info);

      // The IO Completion Port that we will be using for IO and service cmds
      info.m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
      if (info.m_hIOCP == NULL)
         goto leave;

      // Uses global service stats object to report service state
      g_ssRobo.Initialize(g_szServiceName, RoboHandlerEx, (PVOID) info.m_hIOCP,
            TRUE);
      g_ssRobo.AcceptControls(SERVICE_ACCEPT_STOP);

      // Report back
      g_ssRobo.AdvanceState(10000, 0);

      // We use a single critical section for all serialization in this sample
      InitializeCriticalSection(&info.m_csSerialize);
      EnterCriticalSection(&info.m_csSerialize);

      // Create threadpool.  The service main thread also acts as an IO thread
      // So you can elect to set the thread pool value to zero for a single -
      // Threaded service
      int nIndex;
      for (nIndex = 0; nIndex < THREADPOOL;nIndex++) {
         info.m_hThreads[nIndex] = chBEGINTHREADEX(NULL, 0, RoboThread, &info, 
               0, NULL);
         if (info.m_hThreads[nIndex] == NULL) {
            LeaveCriticalSection(&info.m_csSerialize);
            goto leave;
         }
      }

      // Setup security for the pipe
      if (!InitializePipeSecurity(&info.m_saPipeSecurity)) {
         LeaveCriticalSection(&info.m_csSerialize);
         goto leave;
      }

      // Make sure that we have the requested number of "listening" pipes
      UpdatePendingConnections(&info);
      
      // Done initializeing
      g_ssRobo.ReportUltimateState();

      // Exit serialize, and enter main thread into functional thread
      LeaveCriticalSection(&info.m_csSerialize);
      RoboThread(&info);

   } leave:;
   }
   catch (...) {
   }

   // Cleanup
   g_ssRobo.AdvanceState(40000, 0);
   int nIndex;
   for (nIndex = 0;nIndex < THREADPOOL;nIndex++) {
      if (info.m_hThreads[nIndex] == NULL)
         break;
   }

   // However many threads we got, we wait for
   WaitForMultipleObjects(nIndex, info.m_hThreads, TRUE, INFINITE);

   // And then we close their handles
   while (nIndex-- != 0) {
      CloseHandle(info.m_hThreads[nIndex]);
   }

   // Destroy the critical section object
   DeleteCriticalSection(&info.m_csSerialize);

   if (info.m_saPipeSecurity.lpSecurityDescriptor != NULL)
      HeapFree(GetProcessHeap(), 0, 
            info.m_saPipeSecurity.lpSecurityDescriptor);

   // Cleanup connections
   for (nIndex = 0;nIndex < MAXCONNECTIONS;nIndex++) {
      if (info.m_infoConnections[nIndex].m_lInUse != 0)
         CloseHandle(info.m_infoConnections[nIndex].m_hPipe);
   }

   // Clear out remaining canceled IO and free buffers
   ULONG lBytes;
   ULONG lKey;
   LPOVERLAPPED pOvl;
   while (GetQueuedCompletionStatus(info.m_hIOCP, &lBytes, &lKey, &pOvl, 1000) 
         || GetLastError() != WAIT_TIMEOUT) {
      if (lKey == CONTEXT_IO) {
         if (pOvl != NULL)
            FreeIOStruct((IOStruct*) pOvl);
      }
   }

   // Final cleanup
   if (info.m_hIOCP!= NULL)
      CloseHandle(info.m_hIOCP);

   while (info.m_pFirstRobot != NULL)
      RemoveRobot(&info, info.m_pFirstRobot);

   g_ssRobo.ReportUltimateState();
}


///////////////////////////////////////////////////////////////////////////////


void InstallService() {

   // Open the SCM on this machine.
   SC_HANDLE hSCM =
      OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

   // Get our full pathname
   TCHAR szModulePathname[_MAX_PATH * 2];
   GetModuleFileName(NULL, szModulePathname, chDIMOF(szModulePathname));

   // Append the switch that causes the process to run as a service.
   lstrcat(szModulePathname, TEXT(" /service"));

   // Add this service to the SCM's database.
   SC_HANDLE hService =
      CreateService(hSCM, g_szServiceName, g_szServiceName,
         SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
         SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
         szModulePathname, NULL, NULL, NULL, NULL, NULL);

   SERVICE_DESCRIPTION sd = {
      TEXT("Sample RoboService from ")
      TEXT("Programming Server-Side Applications for Microsoft Windows Book")
   };
   ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);

   if (hSCM != NULL)
      CloseServiceHandle(hSCM);

   if (hService != NULL)
      CloseServiceHandle(hService);
}


//////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, LPTSTR pszCmdLine, int) {

   int nArgc = __argc;

#ifdef UNICODE
   PCTSTR* ppArgv = (PCTSTR*) CommandLineToArgvW(GetCommandLine(), &nArgc);
#else
   PCTSTR* ppArgv = (PCTSTR*) __argv;
#endif

   // Handle all the command-line options.
   if (nArgc < 2) {
      
      MessageBox(NULL,
         TEXT("Programming Server-Side Applications for ")
         TEXT("Microsoft Windows: RoboService Sample")
         TEXT("\n\nUsage: RoboService.exe [/install] [/remove] [/debug]")
         TEXT("[/service]\n")
         TEXT("   /install\t\tInstalls the service in the SCM's database.\n")
         TEXT("   /remove\t\tRemoves the service from the SCM's database.\n")
         TEXT("   /debug\t\tRuns the service as a normal process.\n")
         TEXT("   /service\t\tRuns the process as a service ")
         TEXT("(should only be set in the SCM's database)."),
         g_szServiceName, MB_OK);

   }  else {

      for (int i = 1; i < nArgc; i++) {
      
         if ((ppArgv[i][0] == TEXT('-')) || (ppArgv[i][0] == TEXT('/'))) {
         
            // Command line switch
            if (lstrcmpi(&ppArgv[i][1], TEXT("install")) == 0)
               InstallService();

            if (lstrcmpi(&ppArgv[i][1], TEXT("remove"))  == 0)
               RemoveService();

            if (lstrcmpi(&ppArgv[i][1], TEXT("debug"))   == 0) {
               g_ssRobo.SetDebugMode();

               // Execute the service code
               RoboServiceMain(0, NULL);
            }

            if (lstrcmpi(&ppArgv[i][1], TEXT("service")) == 0) {
               // Connect to the service control dispatcher
               SERVICE_TABLE_ENTRY ServiceTable[] = { 
                  {g_szServiceName, RoboServiceMain}, 
                  {NULL, NULL}    // End of list
               };
               chVERIFY(StartServiceCtrlDispatcher(ServiceTable));
            }
         }
      }
   }

#ifdef UNICODE
   HeapFree(GetProcessHeap(), 0, (PVOID) ppArgv);
#endif

   return(0);
}


///////////////////////////////// End of File /////////////////////////////////