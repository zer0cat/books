/******************************************************************************
Module:  HWInputMon.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.
#include <WindowsX.h>

#define HWINPUTPERFDATAMAP_IMPL
#include "HWInputPerfDataMap.h"


///////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, 
   WPARAM wParam, LPARAM lParam) {

   if (nCode == HC_ACTION) {
      switch (wParam) {
      case WM_KEYDOWN:  case WM_SYSKEYDOWN:
      case WM_KEYUP:    case WM_SYSKEYUP: 
         g_PerfData.GetCtr32(HWINPUT_KEYS)++; 
         g_PerfData.GetCtr32(HWINPUT_KEYSPERSEC)++;
         break;
      }
   }
   return(CallNextHookEx(NULL, nCode, wParam, lParam));
}


///////////////////////////////////////////////////////////////////////////////


typedef enum { 
   mciFirst = 0, 
   mciTotal = mciFirst, 
   mciLeft, 
   mciMiddle, 
   mciRight,
   mciLast = mciRight
} MOUSECLCKINST;

CPerfData::INSTID g_MouseClckInstToPrfInstId[mciLast + 1];


///////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {

   if (nCode == HC_ACTION) {
      if (wParam == WM_MOUSEMOVE) {
         g_PerfData.GetCtr32(HWINPUT_MOUSEMOVES)++;
         g_PerfData.GetCtr32(HWINPUT_MOUSEMOVESPERSEC)++;
      }

      BOOL fDown = ((wParam == WM_LBUTTONDOWN) || 
         (wParam == WM_MBUTTONDOWN) || (wParam == WM_RBUTTONDOWN));
      if (fDown) {
         MOUSECLCKINST mci = mciLeft;
         if ((wParam == WM_LBUTTONDOWN) || (wParam == WM_LBUTTONUP))
            mci = mciLeft;
         if ((wParam == WM_MBUTTONDOWN) || (wParam == WM_MBUTTONUP))
            mci = mciMiddle;
         if ((wParam == WM_RBUTTONDOWN) || (wParam == WM_RBUTTONUP))
            mci = mciRight;

         g_PerfData.GetCtr32(MOUSECLCKS_CLICKS,       
            g_MouseClckInstToPrfInstId[mciTotal])++;
         g_PerfData.GetCtr32(MOUSECLCKS_CLICKSPERSEC, 
            g_MouseClckInstToPrfInstId[mciTotal])++;
         g_PerfData.GetCtr32(MOUSECLCKS_CLICKS,       
            g_MouseClckInstToPrfInstId[mci])++;
         g_PerfData.GetCtr32(MOUSECLCKS_CLICKSPERSEC, 
            g_MouseClckInstToPrfInstId[mci])++;
      }
   }
   return(CallNextHookEx(NULL, nCode, wParam, lParam));
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   static TCHAR szAppName[] = TEXT("Hardware Input Monitor");
   
   if (MessageBox(NULL, 
      TEXT("Install Performance Counter Data into Registry?"),
      szAppName, MB_YESNO) == IDYES) {

      TCHAR szPath[_MAX_PATH];
      GetModuleFileName(hinstExe, szPath, chDIMOF(szPath));
      lstrcpy(_tcsrchr(szPath, TEXT('\\')) + 1, 
         TEXT("07 HWInputMonPerfInfo.dll"));
      g_PerfData.Install(szPath);
   }

   if (MessageBox(NULL, 
      TEXT("Collect Performance Counter Data?"),
      szAppName, MB_YESNO) == IDYES) {
      
      g_PerfData.Activate();

      // Add the four Mouse Click Object Instances
      g_MouseClckInstToPrfInstId[mciTotal]  = 
         g_PerfData.AddInstance(TRUE, PERFOBJ_MOUSECLCKS, TEXT("_Total"));
      g_MouseClckInstToPrfInstId[mciLeft]   = 
         g_PerfData.AddInstance(TRUE, PERFOBJ_MOUSECLCKS, TEXT("Left"));
      g_MouseClckInstToPrfInstId[mciMiddle] = 
         g_PerfData.AddInstance(TRUE, PERFOBJ_MOUSECLCKS, TEXT("Middle"));
      g_MouseClckInstToPrfInstId[mciRight]  = 
         g_PerfData.AddInstance(TRUE, PERFOBJ_MOUSECLCKS, TEXT("Right"));

      // Install the low-level keyboard & mouse hooks
      HHOOK hhkLowLevelKybd  = SetWindowsHookEx(WH_KEYBOARD_LL, 
         LowLevelKeyboardProc, hinstExe, 0);
      HHOOK hhkLowLevelMouse = SetWindowsHookEx(WH_MOUSE_LL, 
         LowLevelMouseProc, hinstExe, 0);

      // Keep this app running until we're told to stop
      int x = IDRETRY;
      while (x == IDRETRY) {

         if (x == IDRETRY) {
            // Reset all of the counters to zero
            g_PerfData.LockCtrs();
            g_PerfData.GetCtr32(HWINPUT_KEYS) = 0;
            g_PerfData.GetCtr32(HWINPUT_KEYSPERSEC) = 0;
            g_PerfData.GetCtr32(HWINPUT_MOUSEMOVES) = 0;
            g_PerfData.GetCtr32(HWINPUT_MOUSEMOVESPERSEC) = 0;

            MOUSECLCKINST mci = mciFirst; 
            while (mci <= mciLast) {
               g_PerfData.GetCtr32(MOUSECLCKS_CLICKS,       
                  g_MouseClckInstToPrfInstId[mci]) = 0;
               g_PerfData.GetCtr32(MOUSECLCKS_CLICKSPERSEC, 
                  g_MouseClckInstToPrfInstId[mci]) = 0;

               mci = (MOUSECLCKINST) ((int) mci + 1);
            }
            g_PerfData.UnlockCtrs();
         }

         x = MessageBox(NULL, 
            TEXT("Click \"Retry\"  to reset the counters.\n")
            TEXT("Click \"Cancel\" to terminate the application."),
            szAppName, MB_RETRYCANCEL);
      }
      UnhookWindowsHookEx(hhkLowLevelKybd);
      UnhookWindowsHookEx(hhkLowLevelMouse);

      // Remove the four Mouse Click Object Instances
      g_PerfData.RemoveInstance(PERFOBJ_MOUSECLCKS, 
         g_MouseClckInstToPrfInstId[mciTotal]);
      g_PerfData.RemoveInstance(PERFOBJ_MOUSECLCKS, 
         g_MouseClckInstToPrfInstId[mciLeft]);
      g_PerfData.RemoveInstance(PERFOBJ_MOUSECLCKS, 
         g_MouseClckInstToPrfInstId[mciMiddle]);
      g_PerfData.RemoveInstance(PERFOBJ_MOUSECLCKS, 
         g_MouseClckInstToPrfInstId[mciRight]);
   }

   if (MessageBox(NULL, 
      TEXT("Remove Performance Counter Data from the Registry?"),
      szAppName, MB_YESNO) == IDYES) {
      g_PerfData.Uninstall();
   }

   return(0);
}


///////////////////////////////// End Of File /////////////////////////////////
