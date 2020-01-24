/******************************************************************************
Module:  UILayout.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class manages child window positioning and sizing when a parent 
         window is resized.
         See Appendix B.
******************************************************************************/


#pragma once   // Include this header file once per compilation unit


///////////////////////////////////////////////////////////////////////////////


#include "..\CmnHdr.h"                 // See Appendix A.


///////////////////////////////////////////////////////////////////////////////


class CUILayout {
public:
   enum ANCHORPOINT { 
      AP_TOPLEFT, 
      AP_TOPMIDDLE, 
      AP_TOPRIGHT, 
      AP_MIDDLERIGHT, 
      AP_BOTTOMRIGHT, 
      AP_BOTTOMMIDDLE, 
      AP_BOTTOMLEFT, 
      AP_MIDDLELEFT, 
      AP_CENTER
   };

public:
   void Initialize(HWND hwndParent, int nMinWidth = 0, int nMinHeight = 0);
   BOOL AnchorControl(ANCHORPOINT apUpperLeft, ANCHORPOINT apLowerRight, 
      int nID, BOOL fRedraw = FALSE);
   BOOL AnchorControls(ANCHORPOINT apUpperLeft, ANCHORPOINT apLowerRight,
      BOOL fRedraw, ...);

   BOOL AdjustControls(int cx, int cy);
   void HandleMinMax(PMINMAXINFO pMinMax) 
      { pMinMax->ptMinTrackSize = m_ptMinParentDims; }

private:
   struct CONTROL {
      int         m_nID; 
      BOOL        m_fRedraw;
      ANCHORPOINT m_apUpperLeft;
      ANCHORPOINT m_apLowerRight;
      POINT       m_ptULDelta;
      POINT       m_ptLRDelta;
   }; 

private:
   void PixelFromAnchorPoint(ANCHORPOINT ap, 
      int cxParent, int cyParent, PPOINT ppt);

private:    
   CONTROL m_CtrlInfo[255]; // Max controls allowed in a dialog template
   int     m_nNumControls;
   HWND    m_hwndParent;
   POINT   m_ptMinParentDims; 
}; 


///////////////////////////////////////////////////////////////////////////////


#ifdef UILAYOUT_IMPL


///////////////////////////////////////////////////////////////////////////////


void CUILayout::Initialize(HWND hwndParent, int nMinWidth, int nMinHeight) {
   
   m_hwndParent = hwndParent;
   m_nNumControls = 0;

   if ((nMinWidth == 0) || (nMinHeight == 0)) {
      RECT rc;
      GetWindowRect(m_hwndParent, &rc);
      m_ptMinParentDims.x = rc.right  - rc.left; 
      m_ptMinParentDims.y = rc.bottom - rc.top; 
   }
   if (nMinWidth  != 0) m_ptMinParentDims.x = nMinWidth;
   if (nMinHeight != 0) m_ptMinParentDims.y = nMinHeight; 
}


///////////////////////////////////////////////////////////////////////////////


BOOL CUILayout::AnchorControl(ANCHORPOINT apUpperLeft, 
   ANCHORPOINT apLowerRight, int nID, BOOL fRedraw) {

   BOOL fOk = FALSE;
   try {
      {
      HWND hwndControl = GetDlgItem(m_hwndParent, nID);
      if (hwndControl == NULL) goto leave;
      if (m_nNumControls >= chDIMOF(m_CtrlInfo)) goto leave;

      m_CtrlInfo[m_nNumControls].m_nID = nID;
      m_CtrlInfo[m_nNumControls].m_fRedraw = fRedraw;
      m_CtrlInfo[m_nNumControls].m_apUpperLeft = apUpperLeft;
      m_CtrlInfo[m_nNumControls].m_apLowerRight = apLowerRight;

      RECT rcControl;
      GetWindowRect(hwndControl, &rcControl);  // Screen coords of control
      // Convert coords to parent-relative coordinates
      MapWindowPoints(HWND_DESKTOP, m_hwndParent, (PPOINT) &rcControl, 2);

      RECT rcParent; 
      GetClientRect(m_hwndParent, &rcParent);

      POINT pt; 
      PixelFromAnchorPoint(apUpperLeft, rcParent.right, rcParent.bottom, &pt);
      m_CtrlInfo[m_nNumControls].m_ptULDelta.x = pt.x - rcControl.left; 
      m_CtrlInfo[m_nNumControls].m_ptULDelta.y = pt.y - rcControl.top;

      PixelFromAnchorPoint(apLowerRight, rcParent.right, rcParent.bottom, &pt);
      m_CtrlInfo[m_nNumControls].m_ptLRDelta.x = pt.x - rcControl.right;
      m_CtrlInfo[m_nNumControls].m_ptLRDelta.y = pt.y - rcControl.bottom;

      m_nNumControls++;
      fOk = TRUE;
      }
   leave:;
   }
   catch (...) {
   }
   chASSERT(fOk);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CUILayout::AnchorControls(ANCHORPOINT apUpperLeft, 
   ANCHORPOINT apLowerRight, BOOL fRedraw, ...) {
   
   BOOL fOk = TRUE;

   va_list arglist;
   va_start(arglist, fRedraw);
   int nID = va_arg(arglist, int);
   while (fOk && (nID != -1)) {
      fOk = fOk && AnchorControl(apUpperLeft, apLowerRight, nID, fRedraw);
      nID = va_arg(arglist, int);
   }           
   va_end(arglist);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL CUILayout::AdjustControls(int cx, int cy) {
   BOOL fOk = FALSE;

   // Create region consisting of all areas occupied by controls
   HRGN hrgnPaint = CreateRectRgn(0, 0, 0, 0);
   for (int n = 0; n < m_nNumControls; n++) {

      HWND hwndControl = GetDlgItem(m_hwndParent, m_CtrlInfo[n].m_nID);
      RECT rcControl; 
      GetWindowRect(hwndControl, &rcControl);  // Screen coords of control
      // Convert coords to parent-relative coordinates
      MapWindowPoints(HWND_DESKTOP, m_hwndParent, (PPOINT) &rcControl, 2);

      HRGN hrgnTemp = CreateRectRgnIndirect(&rcControl);
      CombineRgn(hrgnPaint, hrgnPaint, hrgnTemp, RGN_OR);
      DeleteObject(hrgnTemp);
   }

   for (n = 0; n < m_nNumControls; n++) {

      // Get control's upper/left position w/respect to parent's width/height
      RECT rcControl; 
      PixelFromAnchorPoint(m_CtrlInfo[n].m_apUpperLeft, 
         cx, cy, (PPOINT) &rcControl);
      rcControl.left   -= m_CtrlInfo[n].m_ptULDelta.x; 
      rcControl.top    -= m_CtrlInfo[n].m_ptULDelta.y; 

      // Get control's lower/right position w/respect to parent's width/height
      PixelFromAnchorPoint(m_CtrlInfo[n].m_apLowerRight, 
         cx, cy, (PPOINT) &rcControl.right);
      rcControl.right  -= m_CtrlInfo[n].m_ptLRDelta.x;
      rcControl.bottom -= m_CtrlInfo[n].m_ptLRDelta.y;

      // Position/size the control
      HWND hwndControl = GetDlgItem(m_hwndParent, m_CtrlInfo[n].m_nID);
      MoveWindow(hwndControl, rcControl.left, rcControl.top, 
         rcControl.right - rcControl.left, 
         rcControl.bottom - rcControl.top, FALSE);
      if (m_CtrlInfo[n].m_fRedraw) {
         InvalidateRect(hwndControl, NULL, FALSE);
      } else {
         // Remove the regions occupied by the control's new position
         HRGN hrgnTemp = CreateRectRgnIndirect(&rcControl);
         CombineRgn(hrgnPaint, hrgnPaint, hrgnTemp, RGN_DIFF);
         DeleteObject(hrgnTemp);
         // Make the control repaint itself
         InvalidateRect(hwndControl, NULL, TRUE);
         SendMessage(hwndControl, WM_NCPAINT, 1, 0);
         UpdateWindow(hwndControl);
      }
   }

   // Paint the newly exposed portion of the dialog box's client area
   HDC hdc = GetDC(m_hwndParent);
   HBRUSH hbrColor = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
   FillRgn(hdc, hrgnPaint, hbrColor);
   DeleteObject(hbrColor);
   ReleaseDC(m_hwndParent, hdc);
   DeleteObject(hrgnPaint);
   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


void CUILayout::PixelFromAnchorPoint(ANCHORPOINT ap, 
   int cxParent, int cyParent, PPOINT ppt) {

   ppt->x = ppt->y = 0;

   switch (ap) {
   case AP_TOPMIDDLE: 
   case AP_CENTER: 
   case AP_BOTTOMMIDDLE:
      ppt->x = cxParent / 2;
      break;

   case AP_TOPRIGHT: 
   case AP_MIDDLERIGHT: 
   case AP_BOTTOMRIGHT:
      ppt->x = cxParent;
      break;
   }

   switch (ap) {
   case AP_MIDDLELEFT: 
   case AP_CENTER: 
   case AP_MIDDLERIGHT:
      ppt->y = cyParent / 2;
      break;

   case AP_BOTTOMLEFT: 
   case AP_BOTTOMMIDDLE: 
   case AP_BOTTOMRIGHT:
      ppt->y = cyParent;
      break;
   }
}


///////////////////////////////////////////////////////////////////////////////


#endif   // UILAYOUT_IMPL


///////////////////////////////// End of File /////////////////////////////////
