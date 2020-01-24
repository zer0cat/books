/******************************************************************************
Module:  Transport.h
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"           /* See Appendix A. */


///////////////////////////////////////////////////////////////////////////////


DWORD APIDbgMsg(PTSTR pszAPI, DWORD dwError) {

   if (dwError == 0)
      dwError = GetLastError();

   PVOID pvMessageBuffer;
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (PTSTR) &pvMessageBuffer, 0, NULL);

   TCHAR szErrMsgBuffer[500];
   wsprintf(szErrMsgBuffer,
         TEXT("ERROR: API = %s.\n")
         TEXT("ERROR CODE = %d.\nMESSAGE = %s.\n"),
         pszAPI, dwError, (PTSTR) pvMessageBuffer);

#ifdef _DEBUG
   MessageBox(NULL, szErrMsgBuffer, TEXT("ERROR"), MB_OK);
#endif

   OutputDebugString(szErrMsgBuffer);

   // Free the buffer allocated by the system
   LocalFree(pvMessageBuffer);

   return (dwError);
}


///////////////////////////////////////////////////////////////////////////////


class CTransport {
public:
   virtual ~CTransport() {};

public:
   BOOL SendMsg(PBYTE pBuf, DWORD cbBuf);
   PBYTE ReceiveMsg(PDWORD pcbRead);

   virtual BOOL SendData(PVOID pData, PULONG plBytes)          = 0;
   virtual BOOL ReceiveData(PVOID pData, PULONG plBytes)       = 0;
   virtual BOOL InitializeConversation(PTSTR szServerNameOrIP) = 0;
   virtual BOOL WaitForConversation()                          = 0;
   virtual void CloseConversation()                            = 0;

private:
   BOOL ReceiveDataUntil(PVOID pData, ULONG lBytes);

};


///////////////////////////////////////////////////////////////////////////////


#ifdef TRANSPORT_IMPL


///////////////////////////////////////////////////////////////////////////////


BOOL CTransport::SendMsg(PBYTE pBuf, DWORD cbBuf) {

   BOOL fSuccess = FALSE;

   try {{

      //Success but no send on a zero-length message
      if (cbBuf == 0) {   
         fSuccess = TRUE;
         goto leave;
      }

      ULONG lSize = sizeof(cbBuf);
      if (!SendData((PBYTE) &cbBuf, &lSize)) {
         goto leave;
      }

      lSize = cbBuf;
      if (!SendData(pBuf, &lSize) || 0 == lSize) {
         goto leave;
      }

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return (fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CTransport::ReceiveDataUntil(PVOID pData, ULONG lBytes) {
   BOOL fSuccess = FALSE;

   try {{

      PBYTE pbTemp = (PBYTE) pData;
      ULONG cbBufSize = lBytes;      
      do { 

         ULONG cbAttempt = cbBufSize;
         if (!ReceiveData (pbTemp, &cbAttempt)) {
            goto leave;
         }

         pbTemp += cbAttempt;
         cbBufSize -= cbAttempt;

      }  while (cbBufSize > 0);

      fSuccess = TRUE;
   } leave:;
   } catch (...) {}

   return (fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


PBYTE CTransport::ReceiveMsg(PDWORD pcbRead) {
   
   PBYTE pbRet = NULL;

   try {{

      // Get the size of the msg
      ULONG cbData;
      if (!ReceiveDataUntil(&cbData, sizeof(cbData))) {
         goto leave;
      }
      
      // Allocate mem for the msg
      pbRet = (PBYTE) LocalAlloc(LPTR, cbData + sizeof(TCHAR));
      if (pbRet == NULL){
         goto leave;
      }

      ZeroMemory(pbRet + cbData, sizeof(TCHAR));

      if(!ReceiveDataUntil(pbRet, cbData)) {
         LocalFree(pbRet);
         pbRet = NULL;
         goto leave;
      }

      if (pcbRead != NULL)
         *pcbRead = cbData;

   } leave:;
   } catch (...) {}

   return (pbRet);
}


///////////////////////////////////////////////////////////////////////////////


#endif   // TRANSPORT_IMPL


///////////////////////////////////////////////////////////////////////////////


class CSocketTransport:public CTransport {
public:
   CSocketTransport();
   ~CSocketTransport() { ResetInstance();WSACleanup(); };

public:
   virtual BOOL SendData(PVOID pData, PULONG lBytes);
   virtual BOOL ReceiveData(PVOID pData, PULONG plBytes);
   virtual BOOL InitializeConversation(PTSTR szServerNameOrIP);
   virtual BOOL WaitForConversation();
   virtual void CloseConversation();

private:
   SOCKET   m_Socket;
   ULONG    m_lAddress;

   void ResetInstance(BOOL fConstructing = FALSE);
};


///////////////////////////////////////////////////////////////////////////////


#ifdef SOCKETTRANSPORT_IMPL


///////////////////////////////////////////////////////////////////////////////


#define SOCKETPORT 2112

CSocketTransport::CSocketTransport() {

   int     nRes;
   WSADATA wsaData;
   WORD    wVerRequested = 0x0101; // Ver 1.1

   // Init the sockets interface
   nRes = WSAStartup(wVerRequested, &wsaData);
   if (nRes != 0) {
      TCHAR szErrMsg[128];
      wsprintf(szErrMsg, TEXT("Couldn't init winsock: %d\n"), nRes);
      MessageBox(NULL, szErrMsg, TEXT("ERROR"), MB_OK);
   }

   ResetInstance(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void CSocketTransport::ResetInstance(BOOL fConstructing) {

   m_lAddress = 0;
   if (!fConstructing) {
      if (m_Socket != INVALID_SOCKET)
         closesocket(m_Socket);
   }

   m_Socket = INVALID_SOCKET;
}


///////////////////////////////////////////////////////////////////////////////


BOOL CSocketTransport::SendData(PVOID pData, PULONG plBytes) {

   ULONG cbRemaining = *plBytes;
   ULONG cbToSend    = *plBytes;   
   *plBytes = 0;

   try {{

      PBYTE pTemp = (PBYTE) pData;
      while (cbRemaining != 0) {

         ULONG cbSent = send(m_Socket, (char*) pTemp, cbRemaining, 0);
         if (SOCKET_ERROR == cbSent) {
            APIDbgMsg(TEXT("send"), WSAGetLastError());
            goto leave;
         }

         pTemp       += cbSent;
         cbRemaining -= cbSent;
         *plBytes    += cbSent;
      }

   } leave:;
   } catch (...) {}

   return (cbToSend == *plBytes);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CSocketTransport::ReceiveData(PVOID pData, PULONG plBytes) {
   
   BOOL fSuccess = FALSE;
   int  cbRead   = *plBytes;
   *plBytes = 0;

   try {{

      *plBytes = recv(m_Socket, (char*) pData, cbRead, 0);
      if (SOCKET_ERROR == *plBytes) {
         if (WSAGetLastError() != WSAECONNRESET && 
            WSAGetLastError() != WSAECONNABORTED) {
            APIDbgMsg(TEXT("recv"), WSAGetLastError());
         }

         *plBytes = 0;
         goto leave;
      }

      fSuccess = (*plBytes != 0);

   } leave:;
   } catch (...) {}

   return (fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CSocketTransport::InitializeConversation(PTSTR szServerNameOrIP) {
   
   BOOL fSuccess = FALSE;

   try {{

      if (szServerNameOrIP == NULL) {  // Server

         SOCKADDR_IN sin;
         int nRes;

         // Create listening socket
         m_Socket = socket (PF_INET, SOCK_STREAM, 0);
         if (INVALID_SOCKET == m_Socket) {
            APIDbgMsg(TEXT("socket"), WSAGetLastError());
            goto leave;
         }

         // Bind to local port
         sin.sin_family      = AF_INET;
         sin.sin_addr.s_addr = 0;
         sin.sin_port        = htons(SOCKETPORT);

         nRes = bind (m_Socket, (PSOCKADDR) &sin, sizeof (sin));
         if (SOCKET_ERROR == nRes) {
            APIDbgMsg(TEXT("bind"), WSAGetLastError());
            goto leave;
         }

         // Listen for client
         nRes = listen (m_Socket, 1);
         if (SOCKET_ERROR == nRes) {
            APIDbgMsg(TEXT("listen"), WSAGetLastError());
            goto leave;
         }

      } else { // Client

         struct hostent* pHost;
         SOCKADDR_IN sin;
         PSTR pszSvr;

         pszSvr = (PCHAR) szServerNameOrIP;

#ifdef UNICODE
         CAutoBuf<char> szSvrAnsi;

         // Set the size of the buffer
         szSvrAnsi = lstrlen(szServerNameOrIP) + 1;
         wsprintfA(szSvrAnsi, "%S", szServerNameOrIP);
         pszSvr = szSvrAnsi;
#endif

         // Lookup the address for the server name
         m_lAddress = inet_addr (pszSvr);
         if (INADDR_NONE == m_lAddress) {

            pHost = gethostbyname (pszSvr);
            if (NULL == pHost) {
               APIDbgMsg(TEXT("gethostbyname"), WSAGetLastError());
               goto leave;
            }

            memcpy((char FAR*) &m_lAddress, pHost->h_addr, pHost->h_length);
         }

         // Create the socket
         m_Socket = socket(PF_INET, SOCK_STREAM, 0);
         if (INVALID_SOCKET == m_Socket) {
            APIDbgMsg(TEXT("socket"), WSAGetLastError());
            goto leave;
         }

         sin.sin_family      = AF_INET;
         sin.sin_addr.s_addr = m_lAddress;
         sin.sin_port        = htons(SOCKETPORT);
      }

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   if (!fSuccess)
      ResetInstance();

   return (fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CSocketTransport::WaitForConversation() {

   BOOL   fSuccess     = FALSE;
   SOCKET ListenSocket = INVALID_SOCKET;

   try {{

      if (m_lAddress == 0) { // Server

         ListenSocket = m_Socket;

         // Accept client
         m_Socket = accept (ListenSocket, NULL, NULL);
         if (INVALID_SOCKET == m_Socket) {
            APIDbgMsg(TEXT("accept"), WSAGetLastError());
            goto leave;
         }

         Beep(500, 250);

      } else {

         SOCKADDR_IN sin;

         sin.sin_family      = AF_INET;
         sin.sin_addr.s_addr = m_lAddress;
         sin.sin_port        = htons (SOCKETPORT);

         // Connect to remote endpoint
         if (SOCKET_ERROR == connect(m_Socket, (PSOCKADDR) &sin, 
               sizeof(sin))) {

            APIDbgMsg(TEXT("connect"), WSAGetLastError());
            closesocket(m_Socket);
            m_Socket = INVALID_SOCKET;
            goto leave;
         };
      }


      fSuccess = TRUE;
   
   } leave:;
   } catch (...) {}

   if (ListenSocket != INVALID_SOCKET)
      closesocket(ListenSocket);

   return (fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


void CSocketTransport::CloseConversation() {

   ResetInstance();
}


///////////////////////////////////////////////////////////////////////////////


#endif   // SOCKETTRANSPORT_IMPL


///////////////////////////////// End of File /////////////////////////////////