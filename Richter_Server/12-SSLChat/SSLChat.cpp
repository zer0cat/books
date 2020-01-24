/******************************************************************************
Module:  SSLChat.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"              /* See Appendix A. */
#include <WindowsX.h>
#include <Process.h>
#include <Malloc.h>
#include <SChannel.h>
#include <WinCrypt.h>
#include <shlwapi.h>
#include "Resource.h"

// Force linking against the Secur32 library
#pragma comment(lib, "Secur32.lib")

// Force linking against the Ws2_32 library
#pragma comment(lib, "Ws2_32.lib")

// Force linking against the Crypt32 library
#pragma comment(lib, "Crypt32.lib")

#define  SECURITY_WIN32
#include <Security.h>

#define  UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"   /* See Appendix B. */

#define  AUTOBUF_IMPL
#include "..\ClassLib\AutoBuf.h"    /* See Appendix B. */

#define  PRINTBUF_IMPL
#include "..\ClassLib\PrintBuf.h"   /* See Appendix B. */

#define  TRANSPORT_IMPL
#define  SOCKETTRANSPORT_IMPL
#include "..\12-SSPIChat\Transport.h"


#define HANDLE_WM_USER_CONNECT(hwnd, wParam, lParam, fn) ((fn) (hwnd), 0L)
#define HANDLE_WM_USER_DISCONNECT(hwnd, wParam, lParam, fn) ((fn) (hwnd), 0L)

#define MSG(Text) MessageBox(NULL, Text, TEXT("SSLChat"), MB_OK)

#define WM_USER_CONNECT       (WM_USER + 5150)
#define WM_USER_DISCONNECT    (WM_USER + 5151)

#define GUI_STATE_READY       1
#define GUI_STATE_CONNECTING  2
#define GUI_STATE_CONNECTED   3

HINSTANCE g_hInst = NULL;


typedef struct _SSLChatState {

   _SSLChatState() {

      ZeroMemory(this, sizeof(*this));
      m_prntScriptText = new CPrintBuf(1024 * 1024);
      m_prntInfoText   = new CPrintBuf(1024 * 1024);

      PSecPkgInfo psecInfo;
      SECURITY_STATUS ss = QuerySecurityPackageInfo(UNISP_NAME, &psecInfo);
      if (ss == SEC_E_OK) {
         m_lReadBufferLen = psecInfo->cbMaxToken;
         m_pbReadBuffer   = new BYTE[m_lReadBufferLen];
         FreeContextBuffer(psecInfo);
      }
   }

   ~_SSLChatState() {

      delete m_prntScriptText;
      delete m_prntInfoText;
      if (m_pbReadBuffer != NULL)
         delete []m_pbReadBuffer;
   }

   // Dialog and layout state
   CUILayout   m_UILayout;
   HWND        m_hwndDialog;

   // "Output" state
   HWND        m_hwndScript;
   CPrintBuf*  m_prntScriptText;
   HWND        m_hwndInfo;
   CPrintBuf*  m_prntInfoText;

   // Server and client info
   BOOL        m_fServer;
   BOOL        m_fRunAsSystem;
   TCHAR       m_szRemoteUserName[1024];
   TCHAR       m_szLocalUserName[1024];

   // SSL info
   CtxtHandle  m_hContext;
   CredHandle  m_hCredentials;
   CredHandle  m_hCertCredentials;
   BOOL        m_fMutualAuth;
   TCHAR       m_szSrvActName[1024];
   ULONG       m_lExtraData;

   // Communication info
   CTransport* m_pTransport;
   PBYTE       m_pbReadBuffer;
   ULONG       m_lReadBufferLen;

   // Threads and thread sync
   HANDLE      m_hReadThread;
   CRITICAL_SECTION m_CriticalSec;

} SSLChatState, *PSSLChatState;


// Prototype is necessary due to circular references to Dlg_Proc
INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////////////////////////////


void ReportSSLError(PTSTR szAPI, SECURITY_STATUS ss) {

   CPrintBuf bufError;
   bufError.Print(TEXT("%s returned\n"), szAPI, ss);
   bufError.PrintError(ss);
   MSG((PCTSTR) bufError);
}


///////////////////////////////////////////////////////////////////////////////


void AddTextToScriptEdit(HWND hwnd, PCTSTR pszText) {

   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   EnterCriticalSection(&(pscState->m_CriticalSec));
   
   // Catch the exception if the buffer is overrun
   try {

      pscState->m_prntScriptText->Print(TEXT("%s\r\n"), pszText);

   } catch (...) {
      
      pscState->m_prntScriptText->Clear();
      pscState->m_prntScriptText->Print(
         TEXT("<<Print buffer's memory exceeded.  Buffer cleared!>>\r\n"));
   }

   LeaveCriticalSection(&(pscState->m_CriticalSec));

   SetDlgItemText(hwnd, IDE_SCRIPT, *(pscState->m_prntScriptText));
   Edit_SetSel(GetDlgItem(hwnd, IDE_SCRIPT), 0, -1);
   Edit_ScrollCaret(GetDlgItem(hwnd, IDE_SCRIPT));
}


///////////////////////////////////////////////////////////////////////////////


void AddTextToInfoEdit(HWND hwnd, PCTSTR pszText) {

   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   EnterCriticalSection(&(pscState->m_CriticalSec));
   
   // Catch the exception if the buffer is overrun
   try {
      
      pscState->m_prntInfoText->Print(TEXT("%s\r\n"), pszText);
   
   } catch (...) {

      pscState->m_prntInfoText->Clear();
      pscState->m_prntInfoText->Print(
         TEXT("<<Print buffers memory exceeded.  Buffer cleared!>>\r\n"));
   }

   LeaveCriticalSection(&(pscState->m_CriticalSec));

   SetDlgItemText(hwnd, IDE_INFO, *(pscState->m_prntInfoText));
   Edit_SetSel(GetDlgItem(hwnd, IDE_INFO), 0, -1);
   Edit_ScrollCaret(GetDlgItem(hwnd, IDE_INFO));
}


///////////////////////////////////////////////////////////////////////////////


static void PrintHexDump(PSSLChatState pscState, PTSTR pszTitle,
      PBYTE buffer, ULONG length) {

   try {

      if (pszTitle != NULL)
         AddTextToInfoEdit(pscState->m_hwndDialog, pszTitle);

      TCHAR szLine[100];
      TCHAR cbLine;
      ULONG index = 0;
      ULONG count = 0;
      const ULONG lLineWidth   = 8;
      const ULONG lBreakColumn = 3;

      for (; length > 0; length -= count, buffer += count, index += count) {

         count = (length > lLineWidth) ? lLineWidth : length;

         wsprintf(szLine, TEXT("%4.4x  "), index);
         cbLine = 6;

         ULONG i;
         TCHAR szDigits[] = TEXT("0123456789abcdef");
         for (i = 0; i < count; i++) {

            szLine[cbLine++] = szDigits[buffer[i] >> 4];
            szLine[cbLine++] = szDigits[buffer[i] & 0x0f];
            if (i == lBreakColumn)
               szLine[cbLine++] = TEXT(':');
            else
               szLine[cbLine++] = TEXT(' ');
         }
         
         for (; i < lLineWidth; i++) {

            szLine[cbLine++] = TEXT(' ');
            szLine[cbLine++] = TEXT(' ');
            szLine[cbLine++] = TEXT(' ');
         }

         szLine[cbLine++] = ' ';

         for (i = 0; i < count; i++) {

            if (buffer[i] < 32 || buffer[i] > 126)
               szLine[cbLine++] = TEXT('.');
            else
               szLine[cbLine++] = buffer[i];
         }

         szLine[cbLine++] = 0;
         AddTextToInfoEdit(pscState->m_hwndDialog, szLine);
      }

   } catch (...) {}
}


///////////////////////////////////////////////////////////////////////////////


BOOL SSLServerHandshakeAuth(PSSLChatState pscState, PCredHandle phCredentials,
      PULONG plAttributes, PCtxtHandle phContext, PBYTE pbExtraData,
      PULONG pcbExtraData, ULONG lSizeExtraDataBuf) {

   BOOL fSuccess = FALSE;

   try {{

      // Setup a buffer in terms of local variables
      ULONG lEndBufIndex = *pcbExtraData;
      ULONG lBufMaxSize  = lSizeExtraDataBuf;
      PBYTE pbTokenBuf   = pbExtraData;

      // Declare in and out buffers
      SecBuffer secBufferIn[3] = {0};
      SecBufferDesc secBufDescriptorIn;

      SecBuffer secBufferOut;
      SecBufferDesc secBufDescriptorOut;

      // Setup loop state information
      BOOL fFirstPass = TRUE;
      SECURITY_STATUS ss = SEC_I_CONTINUE_NEEDED;
      while (ss == SEC_I_CONTINUE_NEEDED || ss == SEC_E_INCOMPLETE_MESSAGE) {

         // How much data can be read per pass
         ULONG lReadBuffSize;

         // Reset this if we are not doing an "incomplete" loop
         if (ss != SEC_E_INCOMPLETE_MESSAGE) {

            // Reset state for another "blob exchange"
            lEndBufIndex  = 0;
            lReadBuffSize = lBufMaxSize;
         }

         // Receive blob data from client
         if (pscState->m_pTransport->ReceiveData(pbTokenBuf + lEndBufIndex, 
               &lReadBuffSize))
            PrintHexDump(pscState, TEXT("<IN: Auth-blob from Client>"),
                  pbTokenBuf + lEndBufIndex, lReadBuffSize);
         else
            goto leave;

         // Here is how much we have read to date
         lEndBufIndex += lReadBuffSize;

         // Setup our in buffers
         secBufferIn[0].BufferType = SECBUFFER_TOKEN;
         secBufferIn[0].cbBuffer   = lEndBufIndex;
         secBufferIn[0].pvBuffer   = pbTokenBuf;

         // This becomes a SECBUFFER_EXTRA buffer to let us know if 
         // we have extra data afterward
         secBufferIn[1].BufferType = SECBUFFER_EMPTY;
         secBufferIn[1].cbBuffer   = 0;
         secBufferIn[1].pvBuffer   = NULL;

         // Setup in buffer descriptor
         secBufDescriptorIn.ulVersion = SECBUFFER_VERSION;
         secBufDescriptorIn.cBuffers  = 2;
         secBufDescriptorIn.pBuffers  = secBufferIn;

         // Setup out buffer (allocated by SSPI)
         secBufferOut.BufferType = SECBUFFER_TOKEN;
         secBufferOut.cbBuffer   = 0;
         secBufferOut.pvBuffer   = NULL;

         // Setup out buffer descriptor
         secBufDescriptorOut.ulVersion = SECBUFFER_VERSION;
         secBufDescriptorOut.cBuffers  = 1;
         secBufDescriptorOut.pBuffers  = &secBufferOut;

         // This inner loop handles the "continue case" where there
         // is no blob data to be sent.  In this case, there are still
         // more "sections" in our last blob entry that must be processed
         BOOL fMoreSections;

         // This loop processes all complete "sections" of data in buffer
         do {
            
            fMoreSections = FALSE;
            
            // Blob processing
            ss = AcceptSecurityContext(phCredentials, fFirstPass ? NULL 
                  : phContext, &secBufDescriptorIn, *plAttributes 
                  | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM,
                  SECURITY_NATIVE_DREP, phContext, &secBufDescriptorOut,
                  plAttributes, NULL);
            
            // Are there more "sections" to process?
            if ((ss == SEC_I_CONTINUE_NEEDED) 
                  && (secBufferOut.cbBuffer == 0)) {

               fMoreSections = TRUE; // Set state to loop
               
               // This is how much data was left over
               ULONG lExtraData = secBufferIn[1].cbBuffer;
               
               // Move this data back to the beginning of our buffer
               MoveMemory(pbTokenBuf, pbTokenBuf + (lEndBufIndex - lExtraData),
                     lExtraData);

               // This is how much data is in our buffer
               lEndBufIndex = lExtraData;

               // Reset input buffers
               secBufferIn[0].BufferType = SECBUFFER_TOKEN;
               secBufferIn[0].cbBuffer   = lEndBufIndex;
               secBufferIn[0].pvBuffer   = pbTokenBuf;

               secBufferIn[1].BufferType = SECBUFFER_EMPTY;
               secBufferIn[1].cbBuffer   = 0;
               secBufferIn[1].pvBuffer   = NULL;
            }

         } while (fMoreSections);

         // This is how much data our next read from the wire
         // can bring in without overflowing our buffer
         lReadBuffSize = lBufMaxSize - lEndBufIndex;

         if (ss != SEC_E_INCOMPLETE_MESSAGE)
            fFirstPass = FALSE;

         // Was there data to be sent?
         if (secBufferOut.cbBuffer != 0) {

            // Send it then
            ULONG lOut = secBufferOut.cbBuffer;
            if (pscState->m_pTransport->SendData(secBufferOut.pvBuffer, &lOut))
               PrintHexDump(pscState, TEXT("<OUT: Auth-blob to Client>"),
                     (PBYTE) secBufferOut.pvBuffer, lOut);
            else
               goto leave;

            // And free up that out buffer
            FreeContextBuffer(secBufferOut.pvBuffer);
         }
      }

      if (ss == SEC_E_OK) {

         // If there is extra data this is encrypted application
         // layer data. We'll put it in a buffer and the application
         // layer can later decrypt it with DecryptMessage.
         int nIndex = 1;
         while (secBufferIn[nIndex].BufferType != SECBUFFER_EXTRA 
               && (nIndex-- != 0));

         if ((nIndex != -1) && (secBufferIn[nIndex].cbBuffer != 0)) {
             *pcbExtraData = secBufferIn[nIndex].cbBuffer;
             PBYTE pbTempBuf = pbTokenBuf;
             pbTempBuf += (lEndBufIndex - *pcbExtraData);
             MoveMemory(pbExtraData, pbTempBuf, *pcbExtraData);
         }else
            *pcbExtraData = 0;

         fSuccess = TRUE;
      }

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL SSLClientHandshakeAuth(PSSLChatState pscState, PCredHandle phCredentials,
      PCredHandle phCertCredentials, PULONG plAttributes, 
      PCtxtHandle phContext, PTSTR pszServer, PBYTE pbExtraData, 
      PULONG pcbExtraData, ULONG lSizeExtraDataBuf) {

   BOOL fSuccess = FALSE;

   try {{

      // Setup our own copy of the credentials handle
      CredHandle credsUse;
      CopyMemory(&credsUse, phCredentials, sizeof(credsUse));

      // Setup buffer interms of local variables for readability
      ULONG lEndBufIndex = *pcbExtraData;
      ULONG lBufMaxSize  = lSizeExtraDataBuf;
      PBYTE pbData       = pbExtraData;

      // Declare in and out buffers
      SecBuffer secBufferOut;
      SecBufferDesc secBufDescriptorOut;

      SecBuffer secBufferIn[2];
      SecBufferDesc secBufDescriptorIn;

      // Setup loop state information
      BOOL fFirstPass = TRUE;
      SECURITY_STATUS ss = SEC_I_CONTINUE_NEEDED;
      while ((ss == SEC_I_CONTINUE_NEEDED) 
            || (ss == SEC_E_INCOMPLETE_MESSAGE)) {
         
         // How much data can we read per pass
         ULONG lReadBuffSize;
         
         // Reset if we are not doing an "incomplete" loop
         if (ss != SEC_E_INCOMPLETE_MESSAGE) {
            
            // Reset state for another blob exchange
            lEndBufIndex  = 0;
            lReadBuffSize = lBufMaxSize;
         }

         // Some stuff we only due after the first pass
         if (!fFirstPass) {
            
            // Receive as much data as we can
            if (pscState->m_pTransport->ReceiveData(pbData + lEndBufIndex, 
                  &lReadBuffSize))
               PrintHexDump(pscState, TEXT("<IN: Auth-blob from Server>"),
                     pbData + lEndBufIndex, lReadBuffSize);
            else
               goto leave;

            // This is how much data we have so far
            lEndBufIndex += lReadBuffSize;

            // Setup in buffer with our current data
            secBufferIn[0].BufferType = SECBUFFER_TOKEN;
            secBufferIn[0].cbBuffer   = lEndBufIndex;
            secBufferIn[0].pvBuffer   = pbData;

            // This becomes a SECBUFFER_EXTRA buffer to let us
            // know if we have extra data afterward
            secBufferIn[1].BufferType = SECBUFFER_EMPTY;
            secBufferIn[1].cbBuffer   = 0;
            secBufferIn[1].pvBuffer   = NULL;

            // Setup in buffer descriptor
            secBufDescriptorIn.cBuffers  = 2;
            secBufDescriptorIn.pBuffers  = secBufferIn;
            secBufDescriptorIn.ulVersion = SECBUFFER_VERSION;
         }

         // Setup out buffer (allocated by SSPI)
         secBufferOut.BufferType = SECBUFFER_TOKEN;
         secBufferOut.cbBuffer   = 0;
         secBufferOut.pvBuffer   = NULL;

         // Setup out buffer descriptor
         secBufDescriptorOut.cBuffers  = 1;
         secBufDescriptorOut.pBuffers  = &secBufferOut;
         secBufDescriptorOut.ulVersion = SECBUFFER_VERSION;

         // This inner loop handles the "continue case" where there is
         // no blob data to be sent.  In this case, there are still more
         // "sections" in our last blob entry that must be processed
         BOOL fNoOutBuffer;
         do {
            
            fNoOutBuffer = FALSE;
            
            // Blob processing
            ss = InitializeSecurityContext(&credsUse, fFirstPass ? NULL 
                  : phContext, fFirstPass ? pszServer : NULL, *plAttributes
                  | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM, 0, 
                  SECURITY_NATIVE_DREP, fFirstPass ? NULL 
                  : &secBufDescriptorIn, 0, phContext, &secBufDescriptorOut,
                  plAttributes, NULL);

            // Are there more sections to process?
            if ((ss == SEC_I_CONTINUE_NEEDED) 
                  && (secBufferOut.cbBuffer == 0)) {

               fNoOutBuffer = TRUE; // Set state to loop

               // Here is how much data was left over
               ULONG lExtraData = secBufferIn[1].cbBuffer;
               
               // We want to move this data back to the beginning of our buffer
               MoveMemory(pbData, pbData + (lEndBufIndex - lExtraData),
                     lExtraData);

               // Now we have a new lendbufindex
               lEndBufIndex = lExtraData;

               // Lets reset input buffers
               secBufferIn[0].BufferType = SECBUFFER_TOKEN;
               secBufferIn[0].cbBuffer   = lEndBufIndex;
               secBufferIn[0].pvBuffer   = pbData;

               secBufferIn[1].BufferType = SECBUFFER_EMPTY;
               secBufferIn[1].cbBuffer   = 0;
               secBufferIn[1].pvBuffer   = NULL;
            }

            if (ss == SEC_I_INCOMPLETE_CREDENTIALS) {
               
               // Server requested credentials.  Copy credentials with cert.
               // Normally, we would call AcquireCredentialsHandle here
               // to pick up new credentials... However, we have already passed
               // in cert credentials in this sample function.
               CopyMemory(&credsUse, phCertCredentials, sizeof(credsUse));

               // No input needed this pass
               secBufDescriptorIn.cBuffers = 0;
               
               // Keep on truckin
               fNoOutBuffer = TRUE; // Set state to loop
            }

         }  while (fNoOutBuffer);

         // This is how much data our next read from the wire
         // can bring in without overflowing our buffer
         lReadBuffSize = lBufMaxSize - lEndBufIndex;

         // Was there data to be sent?
         if (secBufferOut.cbBuffer!= 0) {

            // Send it then
            ULONG lOut = secBufferOut.cbBuffer;
            if (pscState->m_pTransport->SendData(secBufferOut.pvBuffer, &lOut))
               PrintHexDump(pscState, TEXT("<OUT: Auth-blob to Server>"),
                     (PBYTE) secBufferOut.pvBuffer, lOut);
            else
               goto leave;

            // And free up that out buffer
            FreeContextBuffer(secBufferOut.pvBuffer);
         }

         if (ss != SEC_E_INCOMPLETE_MESSAGE)
            fFirstPass = FALSE;
      }

      if (ss == SEC_E_OK) {

         int nIndex = 1;
         while (secBufferIn[nIndex].BufferType
            != SECBUFFER_EXTRA && (nIndex-- != 0));

         if ((nIndex != -1) && (secBufferIn[nIndex].cbBuffer != 0)) {
             
             *pcbExtraData   = secBufferIn[nIndex].cbBuffer;
             PBYTE pbTempBuf = pbData;
             pbTempBuf += (lEndBufIndex - *pcbExtraData);
             MoveMemory(pbExtraData, pbTempBuf, *pcbExtraData);
         }else
            *pcbExtraData = 0;

         fSuccess = TRUE;
      }

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL GetAnonymousCredentials(PCredHandle phCredentials) {
   
   BOOL fSuccess = FALSE;

   try {{

      SECURITY_STATUS ss;

      SCHANNEL_CRED sslCredentials = {0};
      sslCredentials.dwVersion = SCHANNEL_CRED_VERSION;
      sslCredentials.dwFlags   = SCH_CRED_NO_DEFAULT_CREDS;

      TimeStamp tsExpires;
      ss = AcquireCredentialsHandle(NULL, UNISP_NAME, SECPKG_CRED_OUTBOUND,
            NULL, &sslCredentials, NULL, NULL, phCredentials, &tsExpires);
      if (ss != SEC_E_OK) {
         ReportSSLError(TEXT("AcquireCredentialsHandle"), ss);
         goto leave;
      }

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL GetCertCredentials(PCredHandle phCredentials, PTSTR pszName, 
      BOOL fRunAsSystem, ULONG lCredUse) {

   BOOL fSuccess = FALSE;
   PCCERT_CONTEXT pCertContext = NULL;

   try {{

      SECURITY_STATUS ss;

      // Open personal certificate store for the local machine or current user
      ULONG lStore = fRunAsSystem ? CERT_SYSTEM_STORE_LOCAL_MACHINE
            : CERT_SYSTEM_STORE_CURRENT_USER;
      HCERTSTORE hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, lStore, "MY");
      if (hMyCertStore == NULL)
         goto leave;

      // Fill in an attribute structure for the certificate common name
      CHAR pszCommonName[1024];
#ifdef UNICODE
      if (WideCharToMultiByte(CP_ACP, 0, pszName, -1, pszCommonName, 
            chDIMOF(pszCommonName), NULL, NULL) == 0)
         goto leave;
#else
      lstrcpy(pszCommonName, pszName);
#endif

      CERT_RDN_ATTR certRDNAttr[1];
      certRDNAttr[0].pszObjId     = szOID_COMMON_NAME;
      certRDNAttr[0].dwValueType  = CERT_RDN_PRINTABLE_STRING;
      certRDNAttr[0].Value.pbData = (PBYTE) pszCommonName;
      certRDNAttr[0].Value.cbData = lstrlenA(pszCommonName);
      
      CERT_RDN certRDN = {1, certRDNAttr};

      // Find the certificate context
      pCertContext = CertFindCertificateInStore(hMyCertStore,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_ATTR,
            &certRDN, NULL);
      if (pCertContext == NULL)
         goto leave;

      // Fill in an SCHANNEL_CRED variable with certificate information
      SCHANNEL_CRED sslCredentials = {0};
      sslCredentials.dwVersion = SCHANNEL_CRED_VERSION;
      sslCredentials.cCreds    = 1;
      sslCredentials.paCred    = &pCertContext;

      // Get a credentials handle
      TimeStamp tsExpires;
      ss = AcquireCredentialsHandle(NULL, UNISP_NAME, lCredUse, NULL,
            &sslCredentials, NULL, NULL, phCredentials, &tsExpires);
      if (ss != SEC_E_OK) {
         ReportSSLError(TEXT("AcquireCredentialsHandle"), ss);
         goto leave;
      }

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   // Free the certificate context
   if (pCertContext != NULL)
      CertFreeCertificateContext(pCertContext);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL GetCertNameFromContext(PCtxtHandle phContext, PTSTR pszName,
      ULONG lBufSize) {

   BOOL fSuccess = FALSE;
   PCCERT_CONTEXT pCertContext = NULL;

   try {{

      // Get server's certificate.
      SECURITY_STATUS ss = QueryContextAttributes(phContext,
            SECPKG_ATTR_REMOTE_CERT_CONTEXT, (PVOID) &pCertContext);
      if (ss != SEC_E_OK) {
         lstrcpy(pszName, TEXT("<Anonymous>"));
         fSuccess = TRUE;
         goto leave;
      }

      // Find the size of the block that we are decoding from the certificate
      ULONG lSize = 0;
      CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, X509_NAME,
            pCertContext->pCertInfo->Subject.pbData, // Data to decode
            pCertContext->pCertInfo->Subject.cbData, // Size of data
	         0, NULL, &lSize);

      // Allocate memory for the block
      PCERT_NAME_INFO pcertNameInfo = (PCERT_NAME_INFO) alloca(lSize);

      // Actually decode the subject block from the certificate
      if (!CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
            X509_NAME, pCertContext->pCertInfo->Subject.pbData,
            pCertContext->pCertInfo->Subject.cbData, 0, pcertNameInfo, &lSize))
         goto leave;

      // Look up the common name attribute of the certificate
      PCERT_RDN_ATTR pcertRDNAttr =
         CertFindRDNAttr(szOID_COMMON_NAME, pcertNameInfo);
      if (pcertRDNAttr == NULL)
         goto leave;

      // Translate the information in the CERT_RDN_ATTR structure to a string
      // to use in the application
      if (CertRDNValueToStr(CERT_RDN_PRINTABLE_STRING, &pcertRDNAttr->Value,
            pszName, lBufSize) == 0)
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   if (pCertContext != NULL)
      CertFreeCertificateContext(pCertContext);

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL InitializeSecureConnection(PSSLChatState pscState, BOOL fServer,
      PTSTR szServer) {

   BOOL       fSuccess         = FALSE;
   CredHandle hCredentials     = {0};
   CredHandle hCertCredentials = {0};
   CtxtHandle hContext         = {0};
   ULONG      lAttributes;
   SECURITY_STATUS ss;

   try {{

      if (fServer)
         AddTextToInfoEdit(pscState->m_hwndDialog, 
               TEXT("Waiting for Client..."));
      else
         AddTextToInfoEdit(pscState->m_hwndDialog,
               TEXT("Attempting connection to Server..."));

      if (!pscState->m_pTransport->WaitForConversation())
         goto leave;

      if (fServer) {

         if (!GetCertCredentials(&hCredentials, pscState->m_szLocalUserName,
               pscState->m_fRunAsSystem, SECPKG_CRED_INBOUND))
            goto leave;

         lAttributes = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT
               | ASC_REQ_CONFIDENTIALITY | ASC_RET_EXTENDED_ERROR;
         lAttributes |= pscState->m_fMutualAuth ? ASC_REQ_MUTUAL_AUTH : 0;

         if (!SSLServerHandshakeAuth(pscState, &hCredentials, &lAttributes,
               &hContext, pscState->m_pbReadBuffer, &pscState->m_lExtraData,
               pscState->m_lReadBufferLen)) {
            MSG(TEXT("ServerHandshakeAuth failed"));
            goto leave;
         }

         ss = ImpersonateSecurityContext(&hContext);
         if (ss == SEC_E_OK) {

            TCHAR szMsg[2048];
            lstrcpy(szMsg, TEXT("The client's certificate maps to a\n")
                           TEXT("user account in this domain.  SSLChat\n")
                           TEXT("temporarily impersonated the client\n")
                           TEXT("connection to find the name of the\n")
                           TEXT("account.\n\nUser Account Name: "));
            
            int lLen = lstrlen(szMsg);
            ULONG lMax = chDIMOF(szMsg) - lLen - 1;
            GetUserName(szMsg + lLen, &lMax);
            MSG(szMsg);
            RevertSecurityContext(&hContext);
         }

      }  else {

         if (!GetAnonymousCredentials(&hCredentials))
            goto leave;

         // If the user has selected a client cert, create credentials for it
         if (lstrcmpi(pscState->m_szLocalUserName, TEXT("[Anonymous]")) != 0) {

            if (!GetCertCredentials(&hCertCredentials,
                  pscState->m_szLocalUserName, pscState->m_fRunAsSystem,
                  SECPKG_CRED_OUTBOUND))
               goto leave;

         } else {

            // Just copy the anonymous credentials to use in both instances
            CopyMemory(&hCertCredentials, &hCredentials,
                  sizeof(hCertCredentials));
         }

         lAttributes = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT
               | ISC_REQ_CONFIDENTIALITY | ISC_RET_EXTENDED_ERROR;
         lAttributes |= pscState->m_fMutualAuth ? ISC_REQ_MUTUAL_AUTH : 0;

         // Even if the user has specified a client cert, we start the
         // conversation anonymously, and then upgrade to the cert credentials
         // if and only if the server requested a certificate.  This is not
         // necessary, and we could start out with the certificate credentials.
         // Doing it this way, however more closely mirrors common real-world
         // scenarios.
         if (!SSLClientHandshakeAuth(pscState, &hCredentials,
               &hCertCredentials, &lAttributes, &hContext, szServer, 
               pscState->m_pbReadBuffer, &pscState->m_lExtraData,
               pscState->m_lReadBufferLen)) {

            MSG(TEXT("ClientHandshakeAuth failed\n\n")
                  TEXT("Check to make sure that the name in the \n")
                  TEXT("\"Server Certificate Name\" field matches\n")
                  TEXT("the common name of the server's certificate."));
            goto leave;
         }
      }

      if (!GetCertNameFromContext(&hContext, pscState->m_szRemoteUserName,
            chDIMOF(pscState->m_szRemoteUserName)))
         goto leave;

      // Keep a copy of the security context and credentials handles
      CopyMemory(&pscState->m_hContext, &hContext,
            sizeof(pscState->m_hContext));
      CopyMemory(&pscState->m_hCredentials, &hCredentials, 
            sizeof(pscState->m_hCredentials));
      CopyMemory(&pscState->m_hCertCredentials, &hCertCredentials,
            sizeof(pscState->m_hCertCredentials));

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL SendEncryptedMessage(PSSLChatState pscState, PCtxtHandle phContext,
      PVOID pvData, ULONG lSize) {

   BOOL fSuccess = FALSE;

   try {{

      SecPkgContext_StreamSizes Sizes;

      // Get stream data properties.
      SECURITY_STATUS ss = QueryContextAttributes(phContext,
            SECPKG_ATTR_STREAM_SIZES, &Sizes);
      if (ss != SEC_E_OK)
         goto leave;

      // Can we handle this much data?
      if (lSize > Sizes.cbMaximumMessage)
         goto leave;

      // This buffer is going to be a header, plus message, plus trailer
      ULONG lIOBufferLength = Sizes.cbHeader + Sizes.cbMaximumMessage
            + Sizes.cbTrailer;
      PBYTE pbIOBuffer = (PBYTE) alloca(lIOBufferLength);
      if (pbIOBuffer == NULL)
         goto leave;

      // The data is copied into the buffer after the header.
      CopyMemory(pbIOBuffer + Sizes.cbHeader, (PBYTE) pvData, lSize);

      SecBuffer Buffers[3];

      // Setup the header in buffer
      Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
      Buffers[0].pvBuffer   = pbIOBuffer;
      Buffers[0].cbBuffer   = Sizes.cbHeader;

      // Setup the data in buffer
      Buffers[1].BufferType = SECBUFFER_DATA;
      Buffers[1].pvBuffer   = pbIOBuffer + Sizes.cbHeader;
      Buffers[1].cbBuffer   = lSize;

      // Setup the trailer in buffer
      Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
      Buffers[2].pvBuffer   = pbIOBuffer + Sizes.cbHeader + lSize;
      Buffers[2].cbBuffer   = Sizes.cbTrailer;

      // Setup the buffer descriptor
      SecBufferDesc secBufDescIn;
      secBufDescIn.ulVersion = SECBUFFER_VERSION;
      secBufDescIn.cBuffers  = 3;
      secBufDescIn.pBuffers  = Buffers;

      // Encrypt the data
      ss = EncryptMessage(phContext, 0, &secBufDescIn, 0);
      if (ss != SEC_E_OK)
         goto leave;

      // Send all three buffers in one chunk
      ULONG lOut = Buffers[0].cbBuffer + Buffers[1].cbBuffer +
         Buffers[2].cbBuffer;
      if (pscState->m_pTransport->SendData(pbIOBuffer, &lOut))
         PrintHexDump(pscState, TEXT("<OUT: Message-blob sent>"),
               (PBYTE) pbIOBuffer, lOut);
      else
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


PVOID GetEncryptedMessage(PSSLChatState pscState, PCtxtHandle phContext,
      PULONG plSize, PBYTE ppbExtraData, PULONG pcbExtraData,
      ULONG lSizeExtraDataBuf, PBOOL pfReneg) {

   PVOID pDecrypMsg = NULL;
   *pfReneg = FALSE;

   try {{
      
      // Declare buffer descriptor
      SecBufferDesc SecBuffDesc = {0};
      
      // Declate buffers
      SecBuffer Buffers[4] = {0};
      
      // Extra data coming in
      PBYTE pbData = ppbExtraData;
      ULONG cbData = *pcbExtraData;

      SECURITY_STATUS ss = SEC_E_INCOMPLETE_MESSAGE;
      do {

         // Setup initial data buffer to point to extra data
         Buffers[0].BufferType = SECBUFFER_DATA;
         Buffers[0].pvBuffer   = pbData;
         Buffers[0].cbBuffer   = cbData;

         // Setup 3 empty out buffers
         Buffers[1].BufferType = SECBUFFER_EMPTY;
         Buffers[2].BufferType = SECBUFFER_EMPTY;
         Buffers[3].BufferType = SECBUFFER_EMPTY;

         // Setup descriptor
         SecBuffDesc.ulVersion = SECBUFFER_VERSION;
         SecBuffDesc.cBuffers  = 4;
         SecBuffDesc.pBuffers  = Buffers;

         // If we have any data at all, try to decrypt it
         if (cbData != 0)
            ss = DecryptMessage(phContext, &SecBuffDesc, 0, NULL);

         if (ss == SEC_E_INCOMPLETE_MESSAGE || cbData == 0) {

            // Need to read in more data and try again.
            ULONG lReadSize = lSizeExtraDataBuf - cbData;
            if (pscState->m_pTransport->ReceiveData(pbData + cbData, 
                  &lReadSize))
               PrintHexDump(pscState, TEXT("<IN: Message-blob received>"),
                  (PBYTE) pbData + cbData, lReadSize);
            else
               goto leave;

            cbData += lReadSize;
         }

      } while (ss == SEC_E_INCOMPLETE_MESSAGE);

      // Was there actually a successful decrypt?
      if (ss == SEC_E_OK) {

         // Allocate a buffer for the caller and copy decrypted msg to it
         *plSize = Buffers[1].cbBuffer;
         pDecrypMsg = (PVOID) LocalAlloc(0, *plSize);
         if (pDecrypMsg == NULL)
            goto leave;

         CopyMemory(pDecrypMsg, Buffers[1].pvBuffer, *plSize);

         // If there is extra data, move it to the beginning of the
         // buffer and then set the size of the extra data returned
         int nIndex = 3;
         while (Buffers[nIndex].BufferType != SECBUFFER_EXTRA 
               && (nIndex-- != 0));

         if (nIndex != -1) {

            // There is more data to handle, move it to front of
            // the extra data buffer and the caller can handle
            // the decrypted message then come back to finish
            *pcbExtraData = Buffers[nIndex].cbBuffer;
            MoveMemory(pbData, pbData + (cbData - *pcbExtraData),
                  *pcbExtraData);
         }else
            *pcbExtraData = 0;
      }

      if (ss == SEC_I_RENEGOTIATE)
         *pfReneg = TRUE;

   } leave:;
   } catch (...) {}

   // Return decrypted message
   return(pDecrypMsg);
}


///////////////////////////////////////////////////////////////////////////////


void UpdateControls(HWND hwnd, ULONG lState, BOOL fStateChange) {

   if (fStateChange) {

      BOOL fChat;
      BOOL fSettings;
      UINT nShowControl;

      switch (lState) {

         case GUI_STATE_CONNECTING:
            fChat        = FALSE;
            fSettings    = FALSE;
            nShowControl = IDB_CANCELCONNECT;
            break;

         case GUI_STATE_CONNECTED:
            fChat        = TRUE;
            fSettings    = FALSE;
            nShowControl = IDB_DISCONNECT;
            break;

         default: // gui_state_ready
            fChat        = FALSE;
            fSettings    = TRUE;
            nShowControl = IDB_CONNECT;
            break;
      }

      ShowWindow(GetDlgItem(hwnd, IDB_CANCELCONNECT), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, IDB_CONNECT), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, IDB_DISCONNECT), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, nShowControl), SW_SHOW);

      UINT nChatControls[] = {IDE_MESSAGE, IDB_SEND};
      int nIndex = chDIMOF(nChatControls);
      while (nIndex-- != 0)
         EnableWindow(GetDlgItem(hwnd, nChatControls[nIndex]), fChat);

      UINT nSettingsControls[] = {IDR_SERVER, IDE_SERVER, IDE_SERVERCERT,
            IDR_CLIENT, IDC_AUTH, IDC_CERTIFICATE};
      nIndex = chDIMOF(nSettingsControls);
      while (nIndex-- != 0)
         EnableWindow(GetDlgItem(hwnd, nSettingsControls[nIndex]), fSettings);
   }

   if (lState == GUI_STATE_READY) {
      BOOL fClient = !Button_GetCheck(GetDlgItem(hwnd, IDR_SERVER));
      EnableWindow(GetDlgItem(hwnd, IDE_SERVER), fClient);
      EnableWindow(GetDlgItem(hwnd, IDE_SERVERCERT), fClient);
   }
}


///////////////////////////////////////////////////////////////////////////////


void PopulateCertificateCombo(HWND hwnd) {

   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   HWND hwndCtrl = GetDlgItem(hwnd, IDC_CERTIFICATE);
   HCERTSTORE hMyCertStore = NULL;

   try {{

      ULONG lStore = pscState->m_fRunAsSystem ? CERT_SYSTEM_STORE_LOCAL_MACHINE
            : CERT_SYSTEM_STORE_CURRENT_USER;

      // Open the personal certificate store for the local machine
      hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, lStore, "MY");
      if (hMyCertStore == NULL)
         goto leave;

      PCCERT_CONTEXT pCertContext = NULL;
      do {

         // Find the certificate context
         PCCERT_CONTEXT pCertTemp = CertFindCertificateInStore(hMyCertStore,
               X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY,
               NULL, pCertContext);

         pCertContext = pCertTemp;

         if (pCertContext != NULL) {

            TCHAR szCommonName[1024];
            ULONG lBytes = CertGetNameString(pCertContext, CERT_NAME_ATTR_TYPE,
                  0, szOID_COMMON_NAME, szCommonName, chDIMOF(szCommonName));
            if (lBytes != 0)
               ComboBox_AddString(hwndCtrl, szCommonName);
         }

      } while (pCertContext);

   } leave:;
   } catch (...) {}

   if (hMyCertStore != NULL)
      CertCloseStore(hMyCertStore, 0);

   ComboBox_InsertString(hwndCtrl, 0, TEXT("[Anonymous]"));
   ComboBox_SetCurSel(hwndCtrl, 0);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_SSLCHAT);

   PSSLChatState pscState = new SSLChatState;
   chASSERT(pscState != NULL);

   // Set the pointer to the state structure as user data in the window
   SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) pscState);

   pscState->m_hwndDialog = hwnd;
   pscState->m_hwndInfo   = GetDlgItem(hwnd, IDE_INFO);
   pscState->m_hwndScript = GetDlgItem(hwnd, IDE_SCRIPT);

   // Setup resizer control
   pscState->m_UILayout.Initialize(hwnd);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_TOPRIGHT, IDC_CERTIFICATE, FALSE);
   pscState->m_UILayout.AnchorControls(CUILayout::AP_TOPMIDDLE,
         CUILayout::AP_TOPMIDDLE, FALSE, IDS_CERTIFICATE, IDS_INFO,
         IDS_SERVERCERT, (UINT) -1);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE,
         CUILayout::AP_TOPRIGHT, IDS_RULES, TRUE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, 
         CUILayout::AP_TOPMIDDLE, IDE_SERVER, FALSE);
   pscState->m_UILayout.AnchorControls(CUILayout::AP_TOPMIDDLE,
         CUILayout::AP_TOPMIDDLE, FALSE, IDC_AUTH, IDB_CONNECT, 
         IDB_CANCELCONNECT, IDB_DISCONNECT, (UINT) -1);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_BOTTOMRIGHT, IDE_INFO, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, 
         CUILayout::AP_BOTTOMMIDDLE, IDE_SCRIPT, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_BOTTOMLEFT, 
         CUILayout::AP_BOTTOMMIDDLE, IDE_MESSAGE, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_BOTTOMMIDDLE, 
         CUILayout::AP_BOTTOMMIDDLE, IDB_SEND, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE,
         CUILayout::AP_TOPRIGHT, IDE_SERVERCERT, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_BOTTOMRIGHT, 
         CUILayout::AP_BOTTOMRIGHT, IDB_CLEAR, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, 
         CUILayout::AP_TOPRIGHT, IDB_CERTHELP, FALSE);


   HFONT hFixedFont = (HFONT) GetStockObject(ANSI_FIXED_FONT);
   SendDlgItemMessage(hwnd, IDE_INFO, WM_SETFONT, (WPARAM) hFixedFont, 0);
   SendDlgItemMessage(hwnd, IDE_SCRIPT, WM_SETFONT, (WPARAM) hFixedFont, 0);

   TCHAR szTitle[1024];
   lstrcpy(szTitle, TEXT("SSLChat is running as \""));
   ULONG lSize = chDIMOF(szTitle) - lstrlen(szTitle);
   ULONG lName = lstrlen(szTitle);
   GetUserName(szTitle + lName, &lSize);
   pscState->m_fRunAsSystem = (lstrcmpi(TEXT("system"), szTitle + lName) == 0);
   lstrcat(szTitle, TEXT("\""));
   SetWindowText(hwnd, szTitle);

   SetDlgItemText(hwnd, IDE_SERVER, TEXT("127.0.0.1"));

   // We start out as a server by default
   Button_SetCheck(GetDlgItem(hwnd, IDR_SERVER), TRUE);
   pscState->m_fServer = TRUE;

   PopulateCertificateCombo(hwnd);

   UpdateControls(hwnd, GUI_STATE_READY, TRUE);

   InitializeCriticalSection(&(pscState->m_CriticalSec));
   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


DWORD WINAPI ConnectAndReadThread(PVOID lpParam) {

   PBYTE pszBuffer = NULL;
   DWORD dwBytesRead;

   // Get state info
   PSSLChatState pscState = (PSSLChatState) lpParam;

   BOOL fConnected = InitializeSecureConnection(pscState, pscState->m_fServer,
         (PTSTR) pscState->m_szSrvActName);

   if (fConnected) {

      SendMessage(pscState->m_hwndDialog, WM_USER_CONNECT, 0, 0);
      for (;;) {

         BOOL fReneg;
         pszBuffer = (PBYTE) GetEncryptedMessage(pscState,
               &pscState->m_hContext, &dwBytesRead, pscState->m_pbReadBuffer,
               &pscState->m_lExtraData, pscState->m_lReadBufferLen, &fReneg);

         if (pszBuffer != NULL) {

            CPrintBuf bufTemp;
            bufTemp.Print(TEXT("%s > %s"), pscState->m_szRemoteUserName,
                  pszBuffer);

            AddTextToScriptEdit(pscState->m_hwndDialog, (PCTSTR) bufTemp);
            LocalFree(pszBuffer);
         } else
            break;
      }
   }

   if (pscState->m_hReadThread != NULL)
	   RevertToSelf();

   // Connection has been closed
   PostMessage(pscState->m_hwndDialog, WM_USER_DISCONNECT, 0, 0);

   return(TRUE);

}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnDisconnected(HWND hwnd) {

   // Get state info
   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   delete pscState->m_pTransport;
   pscState->m_pTransport = NULL;

   if (pscState->m_hContext.dwLower != 0
         || pscState->m_hContext.dwUpper != 0) {
      DeleteSecurityContext(&pscState->m_hContext);
      ZeroMemory(&pscState->m_hContext, sizeof(pscState->m_hContext));
   }

   if ((pscState->m_hCertCredentials.dwLower 
         != pscState->m_hCredentials.dwLower)
         || (pscState->m_hCertCredentials.dwUpper
         != pscState->m_hCredentials.dwUpper))
      FreeCredentialsHandle(&pscState->m_hCertCredentials);

   ZeroMemory(&pscState->m_hCertCredentials, 
         sizeof(pscState->m_hCertCredentials));

   if (pscState->m_hCredentials.dwLower != 0 
         || pscState->m_hCredentials.dwUpper != 0) {
      FreeCredentialsHandle(&pscState->m_hCredentials);
      ZeroMemory(&pscState->m_hCredentials, sizeof(pscState->m_hCredentials));
   }

   WaitForSingleObject(pscState->m_hReadThread, INFINITE);
   CloseHandle(pscState->m_hReadThread);
   pscState->m_hReadThread = NULL;
   UpdateControls(hwnd, GUI_STATE_READY, TRUE);

   AddTextToInfoEdit(hwnd, TEXT("Connection Closed"));
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnConnected(HWND hwnd) {

   // Get state info
   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   SetDlgItemText(hwnd, IDE_SCRIPT, *(pscState->m_prntScriptText));
   pscState->m_prntScriptText->Clear();
   UpdateControls(hwnd, GUI_STATE_CONNECTED, TRUE);
   AddTextToInfoEdit(hwnd, TEXT("Secure Connection Established"));
}


///////////////////////////////////////////////////////////////////////////////


void GUIToState(HWND hwnd) {

   // Get state info
   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   // Are we the server
   pscState->m_fServer = Button_GetCheck(GetDlgItem(hwnd, IDR_SERVER));

   // Do we want mutual auth?
   pscState->m_fMutualAuth = Button_GetCheck(GetDlgItem(hwnd, IDC_AUTH));

   // Get server account name
   Edit_GetText(GetDlgItem(hwnd, IDE_SERVERCERT), pscState->m_szSrvActName,
         chDIMOF(pscState->m_szSrvActName));

   // Get certificate name
   ComboBox_GetText(GetDlgItem(hwnd, IDC_CERTIFICATE), 
         pscState->m_szLocalUserName, chDIMOF(pscState->m_szLocalUserName));
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   // Get state info
   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   switch (id) {      

      case IDCANCEL:
         // If connected, handle disconnect
         if (pscState->m_pTransport != NULL) {
            
            pscState->m_pTransport->CloseConversation();
            
            // Wait for read thread to exit
            WaitForSingleObject(pscState->m_hReadThread, INFINITE);
            
            // Repost cancel message to this window
            FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, PostMessage);
         
         } else {
            DeleteCriticalSection(&pscState->m_CriticalSec);
            delete pscState;
            EndDialog(hwnd, id);
         }
         break;

      case IDB_CERTHELP: 
         {
         TCHAR szPath[1024];
         GetModuleFileName(NULL, szPath, chDIMOF(szPath));

         PTSTR szPos = StrRStrI(szPath, NULL, TEXT("\\x86\\"));
         if (szPos != NULL) {
            lstrcpy(szPos + 5, TEXT("12 SSLChat Info.htm"));
            ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
         } else
            chMB("Can't find help file \"12 SSLChat Info.htm\"");
         }
         break;

      case IDB_SEND:
         {
         TCHAR pszInputEditText[2048];

         Edit_GetText(GetDlgItem(hwnd, IDE_MESSAGE), pszInputEditText,
            chDIMOF(pszInputEditText));
         ULONG lLen = lstrlen(pszInputEditText);
         if (lLen > 0) {
            BOOL fSent = SendEncryptedMessage(pscState,
               &pscState->m_hContext, pszInputEditText, 
               (lLen + 1) * sizeof(TCHAR));

            if (fSent) {
               CPrintBuf bufTemp;
               bufTemp.Print(TEXT("%s > %s"), pscState->m_szLocalUserName,
                  pszInputEditText);

               AddTextToScriptEdit(hwnd, (PCTSTR) bufTemp);
               Edit_SetText(GetDlgItem(hwnd, IDE_MESSAGE), TEXT(""));
            } else pscState->m_pTransport->CloseConversation();
         }
         }
         break;

      case IDB_CLEAR:
         pscState->m_prntInfoText->Clear();
         SetWindowText(pscState->m_hwndInfo, TEXT(""));
         break;

      case IDB_CONNECT: 
         {
         // Collect information from GUI
         GUIToState(hwnd);

         // Create the conversation handle
         pscState->m_pTransport = new CSocketTransport;
         if (pscState->m_fServer)
            pscState->m_pTransport->InitializeConversation(NULL);
         else {
            TCHAR szServer[1024];
            Edit_GetText(GetDlgItem(hwnd, IDE_SERVER), szServer,
               chDIMOF(szServer));
            pscState->m_pTransport->InitializeConversation(szServer);
         }

         DWORD dwId;
         pscState->m_hReadThread = chBEGINTHREADEX(NULL, 0,
            ConnectAndReadThread, (PVOID) pscState, 0, &dwId);
         chASSERT(pscState->m_hReadThread != NULL);
         UpdateControls(hwnd, GUI_STATE_CONNECTING, TRUE);
         }
         break;

      case IDC_PACKAGE:
      case IDR_CLIENT:
      case IDR_SERVER:
         UpdateControls(hwnd, GUI_STATE_READY, FALSE);
         break;

      case IDB_DISCONNECT:
      case IDB_CANCELCONNECT:
         pscState->m_pTransport->CloseConversation();
         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnSize(HWND hwnd, UINT state, int cx, int cy) {

   // Get state info
   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   // Simply call the AdjustControls function of our handy resizer class
   pscState->m_UILayout.AdjustControls(cx, cy);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnGetMinMaxInfo(HWND hwnd, PMINMAXINFO pMinMaxInfo) {

   // Get state info
   PSSLChatState pscState = (PSSLChatState) GetWindowLongPtr(hwnd, DWLP_USER);

   // Simply call the AdjustControls function of our handy resizer class
   pscState->m_UILayout.HandleMinMax(pMinMaxInfo);
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
      chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
      chHANDLE_DLGMSG(hwnd, WM_SIZE, Dlg_OnSize);
      chHANDLE_DLGMSG(hwnd, WM_GETMINMAXINFO, Dlg_OnGetMinMaxInfo);
      chHANDLE_DLGMSG(hwnd, WM_COMMAND, Dlg_OnCommand);
      chHANDLE_DLGMSG(hwnd, WM_USER_CONNECT, Dlg_OnConnected);
      chHANDLE_DLGMSG(hwnd, WM_USER_DISCONNECT, Dlg_OnDisconnected);
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   g_hInst = hinstExe;
   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_CHAT), NULL, Dlg_Proc);
   return(0);
}


///////////////////////////////// End of File /////////////////////////////////