/******************************************************************************
Module:  SSPIChat.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"              /* See Appendix A. */
#include <WindowsX.h>
#include <Process.h>
#include <Malloc.h>
#include <ShlWAPI.h>
#include "Resource.h"

// Force linking against the Secur32 library
#pragma comment(lib, "Secur32.lib")   

// Force linking against the Ws2_32 library
#pragma comment(lib, "Ws2_32.lib") 

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
#include "Transport.h"


#define HANDLE_WM_USER_CONNECT(hwnd, wParam, lParam, fn) ((fn) (hwnd), 0L)
#define HANDLE_WM_USER_DISCONNECT(hwnd, wParam, lParam, fn) ((fn) (hwnd), 0L)

#define MSG(Text) MessageBox(NULL, Text, TEXT("SSPIChat"), MB_OK)

#define WM_USER_CONNECT       (WM_USER + 5150)
#define WM_USER_DISCONNECT    (WM_USER + 5151)

#define GUI_STATE_READY       1
#define GUI_STATE_CONNECTING  2
#define GUI_STATE_CONNECTED   3


HINSTANCE g_hInst = NULL;

#ifdef UNICODE
struct {

   PTSTR m_pszFriendlyName;
   PTSTR m_pszProgName;

} g_infoPackage[3] = {
   
   {TEXT("NT LanMan"), NTLMSP_NAME},
   {TEXT("Kerberos"),  MICROSOFT_KERBEROS_NAME},
   {TEXT("Negotiate"), NEGOSSP_NAME}

};
#else
struct {

   PTSTR m_pszFriendlyName;
   PTSTR m_pszProgName;

} g_infoPackage[3] = { 
   
   {TEXT("NT LanMan"), NTLMSP_NAME_A},
   {TEXT("Kerberos"),  MICROSOFT_KERBEROS_NAME_A},
   {TEXT("Negotiate"), NEGOSSP_NAME_A}

};
#endif

typedef struct _SSPIChatState {

   _SSPIChatState(){
      ZeroMemory(this, sizeof(*this));
      m_prntScriptText = new CPrintBuf(1024*1024);
      m_prntInfoText = new CPrintBuf(1024*1024);
   }

   ~_SSPIChatState(){
      delete m_prntScriptText;
      delete m_prntInfoText;
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
   TCHAR       m_szRemoteUserName[1024];
   TCHAR       m_szLocalUserName[1024];
   HANDLE      m_hDelegationThread;

   // SSPI info
   PTSTR       m_pszPackage;
   CredHandle  m_hCredentials;
   CtxtHandle  m_hContext;
   BOOL        m_fMutualAuth;
   BOOL        m_fDelegation;
   BOOL        m_fEncryption;
   TCHAR       m_szSrvActName[1024];

   // Communication info
   CTransport* m_pTransport;

   // Threads and thread sync
   HANDLE      m_hReadThread;
   CRITICAL_SECTION m_CriticalSec;

} SSPIChatState, *PSSPIChatState;


// Prototype is necessary due to circular references to Dlg_Proc
INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////////////////////////////


void ReportSSPIError(PTSTR szAPI, SECURITY_STATUS ss) {

   CPrintBuf bufError;
   bufError.Print(TEXT("%s returned\n"), szAPI, ss);
   bufError.PrintError(ss);
   MSG((PCTSTR) bufError);
}


///////////////////////////////////////////////////////////////////////////////


void AddTextToScriptEdit(HWND hwnd, PCTSTR pszText) {

   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   EnterCriticalSection(&(pscState->m_CriticalSec));
   //This exception block is here because the print buf throws an exception 
   //if buffer is overrun
   try{
      pscState->m_prntScriptText->Print(TEXT("%s\r\n"), pszText);
   }catch(...){
      pscState->m_prntScriptText->Clear();
      pscState->m_prntScriptText->Print(
         TEXT("<< Print buffer's memory exceeded.  Buffer cleared! >>\r\n"));
   }
   LeaveCriticalSection(&(pscState->m_CriticalSec));

   SetDlgItemText(hwnd, IDE_SCRIPT, *(pscState->m_prntScriptText));   
   Edit_SetSel(GetDlgItem(hwnd, IDE_SCRIPT), 0, -1);
   Edit_ScrollCaret(GetDlgItem(hwnd, IDE_SCRIPT));   
}


///////////////////////////////////////////////////////////////////////////////


void AddTextToInfoEdit(HWND hwnd, PCTSTR pszText) {

   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   EnterCriticalSection(&(pscState->m_CriticalSec));
   //This exception block is here because the print buf throws an exception 
   //if buffer is overrun
   try{
      pscState->m_prntInfoText->Print(TEXT("%s\r\n"), pszText);
   }catch(...){
      pscState->m_prntInfoText->Clear();
      pscState->m_prntInfoText->Print(
         TEXT("<< Print buffers memory exceeded.  Buffer cleared! >>\r\n"));
   }
   LeaveCriticalSection(&(pscState->m_CriticalSec));

   SetDlgItemText(hwnd, IDE_INFO, *(pscState->m_prntInfoText));
   Edit_SetSel(GetDlgItem(hwnd, IDE_INFO), 0, -1);
   Edit_ScrollCaret(GetDlgItem(hwnd, IDE_INFO));
   
}


///////////////////////////////////////////////////////////////////////////////


static void PrintHexDump(PSSPIChatState pscState, PTSTR pszTitle,
      PBYTE buffer, ULONG length) {

   try{

      ULONG i;
      ULONG count;
      ULONG index;
      TCHAR szDigits[]   = TEXT("0123456789abcdef");
      TCHAR szLine[100];
      TCHAR cbLine;
      ULONG lLineWidth   = 8;
      ULONG lBreakColumn = 3;

      if (pszTitle != NULL)
         AddTextToInfoEdit(pscState->m_hwndDialog, pszTitle);

      for (index = 0; length > 0; length -= count, buffer += count, 
            index += count) {

         count = (length > lLineWidth) ? lLineWidth : length;

         wsprintf(szLine, TEXT("%4.4x  "), index);
         cbLine = 6;

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

   }catch(...){}

}


///////////////////////////////////////////////////////////////////////////////


BOOL ServerHandshakeAuth(PSSPIChatState pscState, PCredHandle phCredentials,
      PULONG plAttributes, PCtxtHandle phContext) {

   BOOL fSuccess = FALSE;

   try {{

      SECURITY_STATUS ss;

      // Declare In and Out buffers
      SecBuffer     secBufferIn[1];
      SecBufferDesc secBufDescriptorIn;

      SecBuffer     secBufferOut[1];
      SecBufferDesc secBufDescriptorOut;

      // Set up some "loop state" information
      BOOL fFirstPass = TRUE;
      ss = SEC_I_CONTINUE_NEEDED;
      while (ss == SEC_I_CONTINUE_NEEDED) {

         // Client communication !!!
         ULONG lSize;
         PBYTE pbTokenBuf;
         pbTokenBuf = pscState->m_pTransport->ReceiveMsg(&lSize);
         if (pbTokenBuf != NULL)
            PrintHexDump(pscState, TEXT("<IN: Auth-blob from Client>"), 
                  pbTokenBuf, lSize);

         // Point "In" buffer to blob
         secBufferIn[0].BufferType = SECBUFFER_TOKEN;
         secBufferIn[0].cbBuffer   = lSize;
         secBufferIn[0].pvBuffer   = pbTokenBuf;

         // Point "In" buffer description to in buffer
         secBufDescriptorIn.ulVersion = SECBUFFER_VERSION;
         secBufDescriptorIn.cBuffers  = 1;
         secBufDescriptorIn.pBuffers  = secBufferIn;

         // Setup "Out" buffer (SSPI will be allocating buffers for us)
         secBufferOut[0].BufferType = SECBUFFER_TOKEN;
         secBufferOut[0].cbBuffer   = 0;
         secBufferOut[0].pvBuffer   = NULL;

         // Point "Out" buffer description to out buffer
         secBufDescriptorOut.ulVersion = SECBUFFER_VERSION;
         secBufDescriptorOut.cBuffers  = 1;
         secBufDescriptorOut.pBuffers  = secBufferOut;

         // Here is our blob management function
         ss = AcceptSecurityContext(phCredentials, 
               fFirstPass ? NULL : phContext, &secBufDescriptorIn,
               *plAttributes | ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM,
               SECURITY_NETWORK_DREP, phContext, &secBufDescriptorOut, 
               plAttributes, NULL);

         // No longer first pass through the loop
         fFirstPass = FALSE;

         // Was a blob output?  If so, send it
         if (secBufferOut[0].cbBuffer != 0) {

            // Client communication !!!
            BOOL fSent = pscState->m_pTransport->SendMsg(
                  (PBYTE) secBufferOut[0].pvBuffer, secBufferOut[0].cbBuffer);
            if (fSent)
               PrintHexDump(pscState, TEXT("<OUT: Auth-blob to Client>"),
                     (PBYTE) secBufferOut[0].pvBuffer, 
                     secBufferOut[0].cbBuffer);

            // Free out buffer
            FreeContextBuffer(secBufferOut[0].pvBuffer);
         }

         if (pbTokenBuf != NULL) LocalFree(pbTokenBuf);

      } // Loop if ss == SEC_I_CONTINUE_NEEDED;

      // Final result
      if (ss != SEC_E_OK)
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   // Clear the context handle if we fail
   if (!fSuccess)
      ZeroMemory(phContext, sizeof(*phContext));

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL ClientHandshakeAuth(PSSPIChatState pscState, PCredHandle phCredentials,
      PULONG plAttributes, PCtxtHandle phContext, PTSTR pszServer) {
   
   BOOL fSuccess = FALSE;
   SECURITY_STATUS ss;

   try {{

      // Declare In and Out buffers
      SecBuffer     secBufferOut[1];
      SecBufferDesc secBufDescriptorOut;

      SecBuffer     secBufferIn[1];
      SecBufferDesc secBufDescriptorIn;

      // Set up some "loop state" information
      BOOL fFirstPass = TRUE;
      ss = SEC_I_CONTINUE_NEEDED;
      while (ss == SEC_I_CONTINUE_NEEDED) {

         // In blob pointer
         PBYTE pbData = NULL;

         if (fFirstPass) { // First pass, no in buffers

            secBufDescriptorIn.cBuffers  = 0;
            secBufDescriptorIn.pBuffers  = NULL;
            secBufDescriptorIn.ulVersion = SECBUFFER_VERSION;

         } else { // Succesive passes

            ULONG lSize;
            pbData = pscState->m_pTransport->ReceiveMsg(&lSize);
            if (pbData != NULL)
               PrintHexDump(pscState, TEXT("<IN: Auth-blob from Server>"),
                     pbData, lSize);

            // Point "In" buffer to blob
            secBufferIn[0].BufferType = SECBUFFER_TOKEN;
            secBufferIn[0].cbBuffer   = lSize;
            secBufferIn[0].pvBuffer   = pbData;

            // Point "In" buffer description to in buffer
            secBufDescriptorIn.cBuffers  = 1;
            secBufDescriptorIn.pBuffers  = secBufferIn;
            secBufDescriptorIn.ulVersion = SECBUFFER_VERSION;
         }

         // Setup "Out" buffer (SSPI will be allocating buffers for us)
         secBufferOut[0].BufferType = SECBUFFER_TOKEN;
         secBufferOut[0].cbBuffer   = 0;
         secBufferOut[0].pvBuffer   = NULL;

         // Point "Out" buffer description to out buffer
         secBufDescriptorOut.cBuffers  = 1;
         secBufDescriptorOut.pBuffers  = secBufferOut;
         secBufDescriptorOut.ulVersion = SECBUFFER_VERSION;

         ss = InitializeSecurityContext(phCredentials, 
               fFirstPass ? NULL : phContext, pszServer,
               *plAttributes | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM,
               0, SECURITY_NETWORK_DREP, &secBufDescriptorIn, 0, phContext,
               &secBufDescriptorOut, plAttributes, NULL);

         // No longer first pass through the loop
         fFirstPass = FALSE;

         // Was a blob output?  If so, send it
         if (secBufferOut[0].cbBuffer!= 0) {

            // Server communication !!!
            // Send the blob
            BOOL fSent = pscState->m_pTransport->SendMsg((PBYTE) 
                  secBufferOut[0].pvBuffer, secBufferOut[0].cbBuffer);
            if (fSent)
               PrintHexDump(pscState, TEXT("<OUT: Auth-blob to Server>"),
                     (PBYTE) secBufferOut[0].pvBuffer,
                     secBufferOut[0].cbBuffer);

            // Free out buffer
            FreeContextBuffer(secBufferOut[0].pvBuffer);
         }

         if (pbData != NULL) LocalFree(pbData);

      } // Loop if ss == SEC_I_CONTINUE_NEEDED;

      // Final result
      if (ss != SEC_E_OK)
         goto leave;

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   // Clear the context handle if we fail
   if (!fSuccess)
      ZeroMemory(phContext, sizeof(*phContext));

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


DWORD WINAPI DelegateThread(HANDLE hToken) {

   HANDLE hTokenDelegation;
   if (DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityDelegation, 
         TokenImpersonation, &hTokenDelegation)) {

      ImpersonateLoggedOnUser(hTokenDelegation);
      DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHAT), NULL, Dlg_Proc, 
            (LPARAM) hTokenDelegation);
      RevertToSelf();
      CloseHandle(hTokenDelegation);
   }else{
      MSG(TEXT("Delegation was requested, but was not available.")
         TEXT("\nThis is most likely because your server account\n")
         TEXT("has not been trusted for delegation by a domain\n")
         TEXT("administrator.\n\n")
         TEXT("Additionally, it is also possible that the client\n")
         TEXT("did not request delegation, or that \"Mutual Auth\"")
         TEXT("\nwas not selected.\n"));
   }
   CloseHandle(hToken);

   return(0);
}


///////////////////////////////////////////////////////////////////////////////


BOOL InitializeSecureConnection(PSSPIChatState pscState, BOOL fServer,
      PTSTR szServer) {

   SECURITY_STATUS ss;
   BOOL       fSuccess = FALSE;
   CredHandle hCredentials;
   CtxtHandle hContext;
   TimeStamp  tsExpires;
   ULONG      lAttributes;

   try {{

      if (fServer)
         AddTextToInfoEdit(pscState->m_hwndDialog, 
               TEXT("Waiting for Client..."));
      else
         AddTextToInfoEdit(pscState->m_hwndDialog, 
               TEXT("Attempting connection to Server..."));
      if (!pscState->m_pTransport->WaitForConversation())
         goto leave;


      ss = AcquireCredentialsHandle(NULL, pscState->m_pszPackage, 
            SECPKG_CRED_BOTH, NULL, NULL, NULL, NULL, &hCredentials, 
            &tsExpires);
      if (ss != SEC_E_OK) {
         ReportSSPIError(TEXT("AcquireCredentialsHandle"), ss);
         goto leave;
      }

      if (fServer) {

         lAttributes = 0;
         lAttributes |= pscState->m_fMutualAuth?ASC_REQ_MUTUAL_AUTH : 0;
         lAttributes |= pscState->m_fEncryption ? ASC_REQ_CONFIDENTIALITY : 0;
         lAttributes |= pscState->m_fDelegation ? ASC_REQ_DELEGATE : 0;
         if (!ServerHandshakeAuth(pscState, &hCredentials, &lAttributes,
               &hContext)) {
            MSG(TEXT("ServerHandshakeAuth failed"));
            goto leave;
         }

         HANDLE hToken;
         ss = QuerySecurityContextToken(&hContext, &hToken);
         if (ss != SEC_E_OK)
            goto leave;

         ImpersonateLoggedOnUser(hToken);
         ULONG lLen = chDIMOF(pscState->m_szRemoteUserName);
         GetUserName(pscState->m_szRemoteUserName, &lLen);
         RevertToSelf();

         // Did they ask for delegation?  Did we get it?
         if (pscState->m_fDelegation) {

           // If all is good, lets create another window
           HANDLE hThread = chBEGINTHREADEX(NULL, 0, DelegateThread,
                (PVOID) hToken, 0, NULL);
           CloseHandle(hThread);

         } else
			CloseHandle(hToken);
         
      } else {

         lAttributes = 0;
         lAttributes |= pscState->m_fMutualAuth ? ISC_REQ_MUTUAL_AUTH : 0;
         lAttributes |= pscState->m_fEncryption ? ISC_REQ_CONFIDENTIALITY : 0;
         lAttributes |= pscState->m_fDelegation ? ISC_REQ_DELEGATE : 0;

         if (!ClientHandshakeAuth(pscState, &hCredentials, &lAttributes, 
               &hContext, szServer)) {
            MSG(TEXT("ClientHandshakeAuth failed"));
            goto leave;
         }

         lstrcpy(pscState->m_szRemoteUserName, TEXT("<Server>"));
      }

      pscState->m_hCredentials = hCredentials;
      pscState->m_hContext = hContext;
      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL SendSignedMessage(PSSPIChatState pscState, PVOID pvData, ULONG lSize) {

   BOOL fSuccess = FALSE;

   try {{

      SECURITY_STATUS ss;

      // Find some important max buffer size information
      SecPkgContext_Sizes sizes;
      ss = QueryContextAttributes(&(pscState->m_hContext), SECPKG_ATTR_SIZES, 
            &sizes);
      if (ss != SEC_E_OK){
         goto leave;
      }

      PVOID pvSignature = alloca(sizes.cbMaxSignature);

      SecBuffer secBuffer[2];

      // Setup buffer to receive signature
      secBuffer[0].BufferType = SECBUFFER_TOKEN;
      secBuffer[0].cbBuffer   = sizes.cbMaxSignature;
      secBuffer[0].pvBuffer   = pvSignature;
      
      // Setup buffer to point to message data
      secBuffer[1].BufferType = SECBUFFER_DATA;
      secBuffer[1].cbBuffer   = lSize;
      secBuffer[1].pvBuffer   = pvData;
      
      // Setup buffer descriptor
      SecBufferDesc secBufferDesc;
      secBufferDesc.cBuffers  = 2;
      secBufferDesc.pBuffers  = secBuffer;
      secBufferDesc.ulVersion = SECBUFFER_VERSION;
      
      // Make signature
      ss = MakeSignature(&(pscState->m_hContext), 0, &secBufferDesc, 0);
      if (ss != SEC_E_OK) {
         ReportSSPIError(TEXT("MakeSignature"), ss);
         goto leave;
      }

      // Send signature
      BOOL fSent = pscState->m_pTransport->SendMsg(
            (PBYTE) secBuffer[0].pvBuffer, secBuffer[0].cbBuffer);
      if (fSent)
         PrintHexDump(pscState, TEXT("<OUT: Signature-blob sent>"),
               (PBYTE) secBuffer[0].pvBuffer, secBuffer[0].cbBuffer);

      // Send message
      fSent = pscState->m_pTransport->SendMsg((PBYTE) secBuffer[1].pvBuffer,
            secBuffer[1].cbBuffer);
      if (fSent)
         PrintHexDump(pscState, TEXT("<OUT: Message-blob sent>"), 
               (PBYTE) secBuffer[1].pvBuffer, secBuffer[1].cbBuffer);

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


BOOL SendEncryptedMessage(PSSPIChatState pscState, PVOID pvData, ULONG lSize) {
   BOOL fSuccess = FALSE;

   try {{

      SECURITY_STATUS ss;

      // Get some important size information
      SecPkgContext_Sizes sizes;
      ss = QueryContextAttributes(&(pscState->m_hContext), SECPKG_ATTR_SIZES,
            &sizes);
      if (ss != SEC_E_OK)
         goto leave;

      // Allocate our buffers
      PVOID pvPadding   = alloca(sizes.cbBlockSize);
      PVOID pvSignature = alloca(sizes.cbSecurityTrailer);
      
      // Best to copy the message buffer, since it is encrypted in place
      PVOID pvMessage = alloca(lSize);
      CopyMemory(pvMessage, pvData, lSize);

      SecBuffer secBuffer[3] = {0};
      
      // Setup the signature buffer
      secBuffer[0].BufferType = SECBUFFER_TOKEN;
      secBuffer[0].cbBuffer   = sizes.cbSecurityTrailer;
      secBuffer[0].pvBuffer   = pvSignature;
      
      // Setup the message buffer
      secBuffer[1].BufferType = SECBUFFER_DATA;
      secBuffer[1].cbBuffer   = lSize;
      secBuffer[1].pvBuffer   = pvMessage;
      
      // Setup the padding buffer
      secBuffer[2].BufferType = SECBUFFER_PADDING;
      secBuffer[2].cbBuffer   = sizes.cbBlockSize;
      secBuffer[2].pvBuffer   = pvPadding;
      
      // Setup buffer descriptor
      SecBufferDesc secBufferDesc;
      secBufferDesc.cBuffers  = 3;
      secBufferDesc.pBuffers  = secBuffer;
      secBufferDesc.ulVersion = SECBUFFER_VERSION;
      
      // Encrypt message
      ss = EncryptMessage(&(pscState->m_hContext), 0, &secBufferDesc, 0);
      if (ss != SEC_E_OK) {
         ReportSSPIError(TEXT("EncryptMessage"), ss);
         goto leave;
      }

      // Send token
      BOOL fSent = pscState->m_pTransport->SendMsg((PBYTE) 
            secBuffer[0].pvBuffer, secBuffer[0].cbBuffer);
      if (fSent)
         PrintHexDump(pscState, TEXT("<OUT: Signature-blob sent>"),
               (PBYTE) secBuffer[0].pvBuffer, secBuffer[0].cbBuffer);

      // Send message
      fSent = pscState->m_pTransport->SendMsg((PBYTE) secBuffer[1].pvBuffer, 
            secBuffer[1].cbBuffer);
      if (fSent)
         PrintHexDump(pscState, TEXT("<OUT: Message-blob sent>"), 
               (PBYTE) secBuffer[1].pvBuffer, secBuffer[1].cbBuffer);


      // Send padding
      fSent = pscState->m_pTransport->SendMsg((PBYTE) secBuffer[2].pvBuffer,
            secBuffer[2].cbBuffer);
      if (fSent)
         PrintHexDump(pscState, TEXT("<OUT: Padding-blob sent>"),
               (PBYTE) secBuffer[2].pvBuffer, secBuffer[2].cbBuffer);

      fSuccess = TRUE;

   } leave:;
   } catch (...) {}

   return(fSuccess);
}


///////////////////////////////////////////////////////////////////////////////


PVOID GetEncryptedMessage(PSSPIChatState pscState, PULONG plSize) {

   PVOID pvMessage = NULL;
   PVOID pvDataSig = NULL;
   PVOID pvDataMsg = NULL;
   PVOID pvDataPad = NULL;

   try {{

      SECURITY_STATUS ss;

      // Get signature
      ULONG lSigLen;
      pvDataSig = pscState->m_pTransport->ReceiveMsg(&lSigLen);
      if (pvDataSig == NULL)
         goto leave;
      
      PrintHexDump(pscState, TEXT("<IN: Signature-blob received>"), 
            (PBYTE) pvDataSig, lSigLen);

      // Get message
      ULONG lMsgLen;
      pvDataMsg = pscState->m_pTransport->ReceiveMsg(&lMsgLen);
      if (pvDataMsg == NULL)
         goto leave;
      
      PrintHexDump(pscState, TEXT("<IN: Message-blob received>"), 
            (PBYTE) pvDataSig, lMsgLen);

      // Get padding
      ULONG lPadLen;
      pvDataPad = pscState->m_pTransport->ReceiveMsg(&lPadLen);
      if (pvDataPad == NULL)
         goto leave;

      PrintHexDump(pscState, TEXT("<IN: Padding-blob received>"), 
            (PBYTE) pvDataSig, lPadLen);

      SecBuffer secBuffer[3] = {0};
      
      // Setup signature buffer
      secBuffer[0].BufferType = SECBUFFER_TOKEN;
      secBuffer[0].cbBuffer   = lSigLen;
      secBuffer[0].pvBuffer   = pvDataSig;
      
      // Setup message buffer
      secBuffer[1].BufferType = SECBUFFER_DATA;
      secBuffer[1].cbBuffer   = lMsgLen;
      secBuffer[1].pvBuffer   = pvDataMsg;
      
      // Setup padding buffer
      secBuffer[2].BufferType = SECBUFFER_PADDING;
      secBuffer[2].cbBuffer   = lPadLen;
      secBuffer[2].pvBuffer   = pvDataPad;
      
      // Setup buffer descriptor
      SecBufferDesc secBufferDesc;
      secBufferDesc.cBuffers  = 3;
      secBufferDesc.pBuffers  = secBuffer;
      secBufferDesc.ulVersion = SECBUFFER_VERSION;
      ULONG lQual = 0;
      ss = DecryptMessage(&(pscState->m_hContext), &secBufferDesc, 0, &lQual);
      if (ss != SEC_E_OK) {
         ReportSSPIError(TEXT("DecryptMessage"), ss);
         goto leave;
      }

      // Return a buffer that must be freed, containing message
      pvMessage = LocalAlloc(LPTR, secBuffer[1].cbBuffer + sizeof(TCHAR));
      if (pvMessage != NULL) {
         *plSize = secBuffer[1].cbBuffer;
         CopyMemory(pvMessage, secBuffer[1].pvBuffer, secBuffer[1].cbBuffer);
      }

   } leave:;
   } catch (...) {}

   if (pvDataSig != NULL) LocalFree(pvDataSig);
   if (pvDataMsg != NULL) LocalFree(pvDataMsg);
   if (pvDataPad != NULL) LocalFree(pvDataPad);

   return(pvMessage);
}


///////////////////////////////////////////////////////////////////////////////


PVOID GetSignedMessage(PSSPIChatState pscState, PULONG plSize) {

   PVOID pvMessage = NULL;
   PVOID pvDataSig = NULL;
   PVOID pvDataMsg = NULL;

   try {{

      SECURITY_STATUS ss;

      ULONG lSigLen;
      
      // Get signature
      pvDataSig = pscState->m_pTransport->ReceiveMsg(&lSigLen);
      if (pvDataSig == NULL)
         goto leave;

      PrintHexDump(pscState, TEXT("<IN: Signature-blob received>"), 
            (PBYTE) pvDataSig, lSigLen);

      ULONG lMsgLen;
      pvDataMsg = pscState->m_pTransport->ReceiveMsg(&lMsgLen);
      if (pvDataMsg == NULL)
         goto leave;

      PrintHexDump(pscState, TEXT("<IN: Message-blob received>"), 
            (PBYTE) pvDataMsg, lMsgLen);

      SecBuffer secBuffer[2];
      
      // Setup signature buffer
      secBuffer[0].BufferType = SECBUFFER_TOKEN;
      secBuffer[0].cbBuffer   = lSigLen;
      secBuffer[0].pvBuffer   = pvDataSig;
      
      // Setup message buffer
      secBuffer[1].BufferType = SECBUFFER_DATA;
      secBuffer[1].cbBuffer   = lMsgLen;
      secBuffer[1].pvBuffer   = pvDataMsg;
      
      // Setup buffer discriptor
      SecBufferDesc secBufferDesc;
      secBufferDesc.cBuffers  = 2;
      secBufferDesc.pBuffers  = secBuffer;
      secBufferDesc.ulVersion = SECBUFFER_VERSION;
      ULONG lQual = 0;
      
      // Verify signature      
      ss = VerifySignature(&(pscState->m_hContext), &secBufferDesc, 0, &lQual);
      
      if (ss != SEC_E_OK) {
         ReportSSPIError(TEXT("VerifySignature"), ss);
         goto leave;
      }

      // Return a buffer that must be freed, containing message
      pvMessage = LocalAlloc(LPTR, secBuffer[1].cbBuffer + sizeof(TCHAR));
      if (pvMessage != NULL) {
         *plSize = secBuffer[1].cbBuffer;
         CopyMemory(pvMessage, secBuffer[1].pvBuffer,
            secBuffer[1].cbBuffer);
      }

   } leave:;
   } catch (...) {}

   if (pvDataSig != NULL) LocalFree(pvDataSig);
   if (pvDataMsg != NULL) LocalFree(pvDataMsg);

   return(pvMessage);
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

      UINT nSettingsControls[] = {IDR_SERVER, IDE_SERVER, IDE_SERVERACCOUNT,
            IDR_CLIENT, IDC_PACKAGE, IDC_ENCRYPT, IDC_AUTH, IDC_DELEGATION};
      nIndex = chDIMOF(nSettingsControls);
      while (nIndex-- != 0)
         EnableWindow(GetDlgItem(hwnd, nSettingsControls[nIndex]), fSettings);
   }

   if (lState == GUI_STATE_READY) {

      BOOL fClient = !Button_GetCheck(GetDlgItem(hwnd, IDR_SERVER));
      EnableWindow(GetDlgItem(hwnd, IDE_SERVER), fClient);
      EnableWindow(GetDlgItem(hwnd, IDE_SERVERACCOUNT), fClient);

      int nIndex = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_PACKAGE));
      if (nIndex == 0) {

         EnableWindow(GetDlgItem(hwnd, IDC_AUTH), FALSE);
         Button_SetCheck(GetDlgItem(hwnd, IDC_AUTH), FALSE);
         EnableWindow(GetDlgItem(hwnd, IDC_DELEGATION), FALSE);
         Button_SetCheck(GetDlgItem(hwnd, IDC_DELEGATION), FALSE);

      } else {

         EnableWindow(GetDlgItem(hwnd, IDC_AUTH), TRUE);
         EnableWindow(GetDlgItem(hwnd, IDC_DELEGATION), TRUE);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_SSPICHAT);

   PSSPIChatState pscState = new SSPIChatState;
   chASSERT(pscState != NULL);
   
   // Set the pointer to the state structure as user data in the window
   SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) pscState);

   pscState->m_hDelegationThread = (HANDLE) lParam;

   pscState->m_hwndDialog = hwnd;
   pscState->m_hwndInfo   = GetDlgItem(hwnd, IDE_INFO);
   pscState->m_hwndScript = GetDlgItem(hwnd, IDE_SCRIPT);

   ULONG lLen = chDIMOF(pscState->m_szLocalUserName);
   GetUserName(pscState->m_szLocalUserName, &lLen);

   // Setup resizer control
   pscState->m_UILayout.Initialize(hwnd);
   pscState->m_UILayout.AnchorControls(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_TOPRIGHT, FALSE, IDC_PACKAGE, IDE_SERVERACCOUNT, 
         (UINT) -1);
   pscState->m_UILayout.AnchorControls(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_TOPMIDDLE, FALSE, IDS_PACKAGE, IDS_SERVERACCOUNT, 
         (UINT) -1);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_TOPRIGHT, IDS_RULES, TRUE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, 
         CUILayout::AP_TOPMIDDLE, IDE_SERVER, FALSE);
   pscState->m_UILayout.AnchorControls(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_TOPMIDDLE, FALSE, IDC_AUTH, IDC_ENCRYPT, IDB_CONNECT, 
         IDB_DISCONNECT, IDB_CANCELCONNECT, IDC_DELEGATION, (UINT) -1);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_BOTTOMRIGHT, IDS_INFO, TRUE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPMIDDLE, 
         CUILayout::AP_BOTTOMRIGHT, IDE_INFO, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, 
         CUILayout::AP_BOTTOMMIDDLE, IDE_SCRIPT, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_BOTTOMLEFT, 
         CUILayout::AP_BOTTOMMIDDLE, IDE_MESSAGE, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_BOTTOMMIDDLE, 
         CUILayout::AP_BOTTOMMIDDLE, IDB_SEND, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_BOTTOMRIGHT, 
         CUILayout::AP_BOTTOMRIGHT, IDB_CLEAR, FALSE);
   pscState->m_UILayout.AnchorControl(CUILayout::AP_TOPRIGHT, 
         CUILayout::AP_TOPRIGHT, IDB_HELP, FALSE);

   HFONT hFixedFont = (HFONT) GetStockObject(ANSI_FIXED_FONT);
   SendDlgItemMessage(hwnd, IDE_INFO, WM_SETFONT, (WPARAM) hFixedFont, 0);
   SendDlgItemMessage(hwnd, IDE_SCRIPT, WM_SETFONT, (WPARAM) hFixedFont, 0);

   TCHAR szTitle[1024];
   lstrcpy(szTitle, TEXT("SSPIChat is running as \""));
   ULONG lSize = chDIMOF(szTitle) - lstrlen(szTitle);
   GetUserName(szTitle + lstrlen(szTitle), &lSize);
   lstrcat(szTitle, TEXT("\""));
   SetWindowText(hwnd, szTitle);

   SetDlgItemText(hwnd, IDE_SERVER, TEXT("127.0.0.1"));

   // We start out as a server by default
   Button_SetCheck(GetDlgItem(hwnd, IDR_SERVER), TRUE);
   pscState->m_fServer = TRUE;

   HWND hwndCombo = GetDlgItem(hwnd, IDC_PACKAGE);
   int  nIndex    = chDIMOF(g_infoPackage);
   while (nIndex-- != 0)
      ComboBox_InsertString(hwndCombo, 0, 
            g_infoPackage[nIndex].m_pszFriendlyName);
   ComboBox_SetCurSel(hwndCombo, 0);

   UpdateControls(hwnd, GUI_STATE_READY, TRUE);

   InitializeCriticalSection(&(pscState->m_CriticalSec));
   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


DWORD WINAPI ConnectAndReadThread(PVOID lpParam) {

   PBYTE pszBuffer = NULL;
   DWORD dwBytesRead;

   // Get state info
   PSSPIChatState pscState = (PSSPIChatState) lpParam;
   if (pscState->m_hReadThread != NULL)
      ImpersonateLoggedOnUser(pscState->m_hDelegationThread);

   BOOL fConnected = InitializeSecureConnection(pscState, pscState->m_fServer,
         (PTSTR) pscState->m_szSrvActName);

   if (fConnected) {

      SendMessage(pscState->m_hwndDialog, WM_USER_CONNECT, 0, 0);
      for(;;) {

         if (pscState->m_fEncryption)
            pszBuffer = (PBYTE) GetEncryptedMessage(pscState, &dwBytesRead);
         else
            pszBuffer = (PBYTE) GetSignedMessage(pscState, &dwBytesRead);
         
         if (pszBuffer != NULL) {

            CPrintBuf bufTemp;
            bufTemp.Print(TEXT("%s>  %s"), pscState->m_szRemoteUserName, 
                  pszBuffer);

            AddTextToScriptEdit(pscState->m_hwndDialog, (PCTSTR) bufTemp);
            LocalFree(pszBuffer);
         }else
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
   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   delete pscState->m_pTransport;
   pscState->m_pTransport = NULL;

   if (pscState->m_hContext.dwLower != 0 ||
      pscState->m_hContext.dwUpper != 0){
      DeleteSecurityContext(&pscState->m_hContext);
      ZeroMemory(&pscState->m_hContext, sizeof(pscState->m_hContext));
   }

   if (pscState->m_hCredentials.dwLower != 0 ||
      pscState->m_hCredentials.dwUpper != 0){
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
   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   SetDlgItemText(hwnd, IDE_SCRIPT, *(pscState->m_prntScriptText));
   pscState->m_prntScriptText->Clear();
   UpdateControls(hwnd, GUI_STATE_CONNECTED, TRUE);
   AddTextToInfoEdit(hwnd, TEXT("Secure Connection Established"));
}


///////////////////////////////////////////////////////////////////////////////


void GUIToState(HWND hwnd) {
   
   // Get state info
   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
      DWLP_USER);

   // Are we the server
   pscState->m_fServer = Button_GetCheck(GetDlgItem(hwnd, IDR_SERVER));

   // What features do we want?
   pscState->m_fEncryption = Button_GetCheck(GetDlgItem(hwnd, IDC_ENCRYPT));
   pscState->m_fMutualAuth = Button_GetCheck(GetDlgItem(hwnd, IDC_AUTH));
   pscState->m_fDelegation = Button_GetCheck(GetDlgItem(hwnd, IDC_DELEGATION));

   // Get server account name
   Edit_GetText(GetDlgItem(hwnd, IDE_SERVERACCOUNT), pscState->m_szSrvActName,
         chDIMOF(pscState->m_szSrvActName));

   // Get package
   int nIndex = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_PACKAGE));
   pscState->m_pszPackage = g_infoPackage[nIndex].m_pszProgName;

}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   DWORD dwId;

   // Get state info
   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   switch (id) {

      case IDCANCEL:
         //If connected, handle disconnect
         if (pscState->m_pTransport != NULL){
            pscState->m_pTransport->CloseConversation();
            //Wait for read thread to exit
            WaitForSingleObject(pscState->m_hReadThread, INFINITE);
            //Repost cancel message to this window
            FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, PostMessage);
         }else{
         
            DeleteCriticalSection(&pscState->m_CriticalSec);
            delete pscState;
            EndDialog(hwnd, id);
         }
         break;

      case IDB_HELP:          
         {
         TCHAR szPath[1024];
         GetModuleFileName(NULL, szPath, chDIMOF(szPath));

         PTSTR szPos = StrRStrI(szPath, NULL, TEXT("\\x86\\"));
         if (szPos != NULL) {
            lstrcpy(szPos + 5, TEXT("12 SSPIChat Info.htm"));
            ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
         } else
            chMB("Can't find help file \"12 SSPIChat Info.htm\"");
         }         
         break;

      case IDB_SEND: {

            TCHAR pszInputEditText[2048];

            Edit_GetText(GetDlgItem(hwnd, IDE_MESSAGE), pszInputEditText, 
                  chDIMOF(pszInputEditText));
            ULONG lLen = lstrlen(pszInputEditText);
            if (lLen > 0) {

               BOOL fSent = FALSE;
               if (pscState->m_fEncryption)
                  fSent = SendEncryptedMessage(pscState, pszInputEditText, 
                        (lLen + 1) * sizeof(TCHAR));
               else
                  fSent = SendSignedMessage(pscState, pszInputEditText, 
                        (lLen + 1) * sizeof(TCHAR));

               if (fSent) {

                  CPrintBuf bufTemp;
                  bufTemp.Print(TEXT("%s>  %s"), pscState->m_szLocalUserName,
                        pszInputEditText);

                  AddTextToScriptEdit(hwnd, (PCTSTR) bufTemp);
                  Edit_SetText(GetDlgItem(hwnd, IDE_MESSAGE), TEXT(""));

               } else
                  pscState->m_pTransport->CloseConversation();
            }
         }
         break;

      case IDB_CLEAR:

         pscState->m_prntInfoText->Clear();
         SetWindowText(pscState->m_hwndInfo, TEXT(""));
         break;

      case IDB_CONNECT: {

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
   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

   // Simply call the AdjustControls function of our handy resizer class
   pscState->m_UILayout.AdjustControls(cx, cy);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnGetMinMaxInfo(HWND hwnd, PMINMAXINFO pMinMaxInfo) {

   // Get state info
   PSSPIChatState pscState = (PSSPIChatState) GetWindowLongPtr(hwnd, 
         DWLP_USER);

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
   DialogBoxParam(hinstExe, MAKEINTRESOURCE(IDD_CHAT), NULL, Dlg_Proc, NULL);
   return(0);
}


///////////////////////////////// End of File /////////////////////////////////