/******************************************************************************
Module:  MsgTableDump.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
Credit:  Thanks to Gary Peluso for initial implementation of MsgTableDump
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.
#include <WindowsX.h>

#define UILAYOUT_IMPL
#include "..\ClassLib\UILayout.h"      // See Appendix B.

#define PRINTBUF_IMPL
#include "..\ClassLib\PrintBuf.h"      // See Appendix B.

#include "Resource.h"


///////////////////////////////////////////////////////////////////////////////


CUILayout g_UILayout;   // Repositions controls when dialog box size changes.


///////////////////////////////////////////////////////////////////////////////


void MsgTableDump(PTSTR pszDLLName, HWND hwnd) {   

   CPrintBuf buf(128 * 1024 * 1024);

   // Load the DLL for resource access only
   HMODULE hDLL = LoadLibraryEx(pszDLLName, NULL, LOAD_LIBRARY_AS_DATAFILE);
   if (hDLL == NULL) {
      buf.Print(TEXT("Module could not be loaded."));
   } else {
      // Find the module's message table resource.
      HRSRC hRsrc = FindResource(hDLL, MAKEINTRESOURCE(1), RT_MESSAGETABLE);
      if (hRsrc == NULL) {
         buf.Print(TEXT("Module doesn't contain a message table resource."));
      } else {
         // Get a pointer to the message table resource.
         PVOID pvMsgTable = LockResource(LoadResource(hDLL, hRsrc));
         chASSERT(pvMsgTable != NULL);

         // Enumerate the strings in the message table resource.
         // Get number of message blocks
         DWORD dwNumBlocks = ((PMESSAGE_RESOURCE_DATA) 
            pvMsgTable)->NumberOfBlocks;

         // Point to array of block structures
         PMESSAGE_RESOURCE_BLOCK pMsgResBlock = 
            ((PMESSAGE_RESOURCE_DATA) pvMsgTable)->Blocks;

         // Iterate through message blocks
         for (DWORD dwBlock = 0; dwBlock < dwNumBlocks; dwBlock++) {

            // Get the low and high ids
            DWORD dwLowId = pMsgResBlock[dwBlock].LowId;
            DWORD dwHighId = pMsgResBlock[dwBlock].HighId;
            DWORD dwOffset = 0;  // First message in block is at offset zero

            // Iterate through each message in this block
            for (DWORD dwId = dwLowId; dwId <= dwHighId; dwId++) {
               // Get offset to the message
               PMESSAGE_RESOURCE_ENTRY pMsgResEntry = 
                  (PMESSAGE_RESOURCE_ENTRY) ((PBYTE) pvMsgTable + 
                   (DWORD) pMsgResBlock[dwBlock].OffsetToEntries + dwOffset);

#ifdef UNICODE
               buf.Print(TEXT("%08X   %s"), dwId, pMsgResEntry->Text);
#else
               buf.Print(TEXT("%08X   %S"), dwId, pMsgResEntry->Text);
#endif
               // Move offset to the next message
               dwOffset += pMsgResEntry->Length;
            }
         }
      }
      FreeLibrary(hDLL);
   }

   // Show the results to the user.   
   SetDlgItemText(hwnd, IDC_DUMP, buf);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_MSGDUMP);

   g_UILayout.Initialize(hwnd); 
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_TOPRIGHT, 
      IDC_FILENAME, FALSE);
   g_UILayout.AnchorControl(CUILayout::AP_TOPLEFT, CUILayout::AP_BOTTOMRIGHT, 
      IDC_DUMP, FALSE);
   return(TRUE); 
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {  
   
   switch (id) {
   case IDCANCEL:
      EndDialog(hwnd, id);
      break;
      
   case IDC_PATHNAME:
      // Get current filename text
      TCHAR szPathname[MAX_PATH] = { 0 };
      OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 };
      ofn.hwndOwner = hwnd;
      ofn.lpstrFilter = TEXT("Executables\0*.exe\0DLL Files\0*.dll\0")
         TEXT("All Files\0*.*\0");
      ofn.lpstrFile = szPathname;
      ofn.nMaxFile = chDIMOF(szPathname);
      ofn.lpstrTitle = TEXT("Select a file containing a message resource");
      ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
      ofn.lpstrDefExt = TEXT("DLL");
      if (GetOpenFileName(&ofn)) {
         SetDlgItemText(hwnd, IDC_FILENAME, szPathname);

         // Dump the file's message table resource.
         MsgTableDump(szPathname, hwnd);
      }
      break;
   }
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnSize(HWND hwnd, UINT state, int cx, int cy) {

   // Reposition the child controls
   g_UILayout.AdjustControls(cx, cy);    
}


///////////////////////////////////////////////////////////////////////////////


void Dlg_OnGetMinMaxInfo(HWND hwnd, PMINMAXINFO pMinMaxInfo) {

   // Return minimum size of dialog box
   g_UILayout.HandleMinMax(pMinMaxInfo);
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

   switch (uMsg) {
   chHANDLE_DLGMSG(hwnd, WM_INITDIALOG,    Dlg_OnInitDialog);
   chHANDLE_DLGMSG(hwnd, WM_COMMAND,       Dlg_OnCommand);
   chHANDLE_DLGMSG(hwnd, WM_SIZE,          Dlg_OnSize);
   chHANDLE_DLGMSG(hwnd, WM_GETMINMAXINFO, Dlg_OnGetMinMaxInfo);
   }
   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_MSGDMP), NULL, Dlg_Proc);
   return(0);
}


//////////////////////////////// End of File //////////////////////////////////
