/******************************************************************************
Module:  FileCopy.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                    // See Appendix A.
#include <WindowsX.h>

#include "..\ClassLib\IOCP.h"             // See Appendix B.
#include "..\ClassLib\EnsureCleanup.h"    // See Appendix B.
#include "Resource.h"


///////////////////////////////////////////////////////////////////////////////


// Each I/O Request needs an OVERLAPPED structure and a data buffer
class CIOReq : public OVERLAPPED {
public:
   CIOReq() {
      Internal = InternalHigh = 0;   
      Offset = OffsetHigh = 0;   
      hEvent = NULL;
      m_nBuffSize = 0;
      m_pvData = NULL;
   }

   ~CIOReq() {
      if (m_pvData != NULL)
         VirtualFree(m_pvData, 0, MEM_RELEASE);
   }

   BOOL AllocBuffer(SIZE_T nBuffSize) {
      m_nBuffSize = nBuffSize;
      m_pvData = VirtualAlloc(NULL, m_nBuffSize, MEM_COMMIT, PAGE_READWRITE);
      return(m_pvData != NULL);
   }

   BOOL Read(HANDLE hDevice, PLARGE_INTEGER pliOffset = NULL) {
      if (pliOffset != NULL) {
         Offset     = pliOffset->LowPart;
         OffsetHigh = pliOffset->HighPart;
      }
      return(::ReadFile(hDevice, m_pvData, m_nBuffSize, NULL, this));
   }

   BOOL Write(HANDLE hDevice, PLARGE_INTEGER pliOffset = NULL) {
      if (pliOffset != NULL) {
         Offset     = pliOffset->LowPart;
         OffsetHigh = pliOffset->HighPart;
      }
      return(::WriteFile(hDevice, m_pvData, m_nBuffSize, NULL, this));
   }

private:
   SIZE_T m_nBuffSize;
   PVOID  m_pvData;
};


///////////////////////////////////////////////////////////////////////////////


#define BUFFSIZE              (64 * 1024) // The size of an I/O buffer
#define MAX_PENDING_IO_REQS   4           // The maximum # of of I/Os


// The completion key values indicate the type of completed I/O.
#define CK_READ  1
#define CK_WRITE 2


///////////////////////////////////////////////////////////////////////////////


BOOL FileCopy(PCTSTR pszFileSrc, PCTSTR pszFileDst) {

   BOOL fOk = FALSE;    // Assume file copy fails
   LARGE_INTEGER liFileSizeSrc = { 0 }, liFileSizeDst;

   try {
      {
      // Open the source file without buffering & get its size
      CEnsureCloseFile hfileSrc = CreateFile(pszFileSrc, GENERIC_READ, 
         FILE_SHARE_READ, NULL, OPEN_EXISTING, 
         FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
      if (hfileSrc.IsInvalid()) goto leave;

      // Get the file's size
      GetFileSizeEx(hfileSrc, &liFileSizeSrc);

      // Non-buffered I/O requires sector-sized transfers.
      // I'll use buffer-size transfers since it's easier to calculate.
      liFileSizeDst.QuadPart = chROUNDUP(liFileSizeSrc.QuadPart, BUFFSIZE);

      // Open the destination file without buffering & set its size
      CEnsureCloseFile hfileDst = CreateFile(pszFileDst, GENERIC_WRITE, 
         0, NULL, CREATE_ALWAYS, 
         FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, hfileSrc);
      if (hfileDst.IsInvalid()) goto leave;

      // File systems extend files synchronously. Extend the destination file 
      // now so that I/Os execute asynchronously improving performance.
      SetFilePointerEx(hfileDst, liFileSizeDst, NULL, FILE_BEGIN);
      SetEndOfFile(hfileDst);

      // Create an I/O completion port and associate the files with it.
      CIOCP iocp(0);
      iocp.AssociateDevice(hfileSrc, CK_READ);  // Read from source file
      iocp.AssociateDevice(hfileDst, CK_WRITE); // Write to destination file

      // Initialize record-keeping variables
      CIOReq ior[MAX_PENDING_IO_REQS];
      LARGE_INTEGER liNextReadOffset = { 0 };
      int nReadsInProgress  = 0;
      int nWritesInProgress = 0;

      // Prime the file copy engine by simulating that writes have completed.
      // This causes read operations to be issued.
      for (int nIOReq = 0; nIOReq < chDIMOF(ior); nIOReq++) {

         // Each I/O request requires a data buffer for transfers
         chVERIFY(ior[nIOReq].AllocBuffer(BUFFSIZE));
         nWritesInProgress++;
         iocp.PostStatus(CK_WRITE, 0, &ior[nIOReq]);
      }

      // Loop while outstanding I/O requests still exist
      while ((nReadsInProgress > 0) || (nWritesInProgress > 0)) {

         // Suspend the thread until an I/O completes
         ULONG_PTR CompKey;
         DWORD dwNumBytes;
         CIOReq* pior;
         iocp.GetStatus(&CompKey, &dwNumBytes, (OVERLAPPED**) &pior, INFINITE);

         switch (CompKey) {
         case CK_READ:  // Read completed, write to destination
            nReadsInProgress--;
            pior->Write(hfileDst);  // Write to same offset read from source
            nWritesInProgress++;
            break;

         case CK_WRITE: // Write completed, read from source
            nWritesInProgress--;
            if (liNextReadOffset.QuadPart < liFileSizeDst.QuadPart) {
               // Not EOF, read the next block of data from the source file.
               pior->Read(hfileSrc, &liNextReadOffset);
               nReadsInProgress++;
               liNextReadOffset.QuadPart += BUFFSIZE; // Advance source offset
            }
            break;
         }
      }
      fOk = TRUE;
      }
   leave:;
   }
   catch (...) {
   }

   if (fOk) {
      // The destination file size is a multiple of the page size. Open the
      // file WITH buffering to shrink its size to the source file's size.
      CEnsureCloseFile hfileDst = CreateFile(pszFileDst, GENERIC_WRITE, 
         0, NULL, OPEN_EXISTING, 0, NULL);
      if (hfileDst.IsValid()) {
         
         SetFilePointerEx(hfileDst, liFileSizeSrc, NULL, FILE_BEGIN);
         SetEndOfFile(hfileDst);
      }
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_FILECOPY);

   // Disable Copy button since no file is selected yet.
   EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
   
   TCHAR szPathname[_MAX_PATH];

   switch (id) {
   case IDCANCEL:
      EndDialog(hwnd, id);
      break;

   case IDOK:
      // Copy the source file to the destination file.
      Static_GetText(GetDlgItem(hwnd, IDC_SRCFILE), 
         szPathname, sizeof(szPathname));
      SetCursor(LoadCursor(NULL, IDC_WAIT));
      chMB(FileCopy(szPathname, TEXT("FileCopy.cpy")) 
         ? "File Copy Successful" : "File Copy Failed");
      break;

   case IDC_PATHNAME:
      OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };
      ofn.hwndOwner = hwnd;
      ofn.lpstrFilter = TEXT("*.*\0");
      lstrcpy(szPathname, TEXT("*.*"));
      ofn.lpstrFile = szPathname;
      ofn.nMaxFile = chDIMOF(szPathname);
      ofn.lpstrTitle = TEXT("Select file to copy");
      ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
      BOOL fOk = GetOpenFileName(&ofn);
      if (fOk) {
         // Show user the source file's size
         Static_SetText(GetDlgItem(hwnd, IDC_SRCFILE), szPathname);
         CEnsureCloseFile hfile = CreateFile(szPathname, 0, 0, NULL, 
            OPEN_EXISTING, 0, NULL);
         if (hfile.IsValid()) {
            LARGE_INTEGER liFileSize;
            GetFileSizeEx(hfile, &liFileSize);
            // NOTE: Only shows bottom 32-bits of size
            SetDlgItemInt(hwnd, IDC_SRCFILESIZE, liFileSize.LowPart, FALSE);
         }
      }
      EnableWindow(GetDlgItem(hwnd, IDOK), fOk);
      break;
   }
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
   chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
   chHANDLE_DLGMSG(hwnd, WM_COMMAND,    Dlg_OnCommand);
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_FILECOPY), NULL, Dlg_Proc);
   return(0);
}


//////////////////////////////// End of File //////////////////////////////////
