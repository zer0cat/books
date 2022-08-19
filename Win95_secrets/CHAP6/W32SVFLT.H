//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SVFLT.H
//==================================

BOOL CALLBACK W32SpyFilterDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL SaveFilterValues(void);

BOOL LoadSavedFilterValues(void);
