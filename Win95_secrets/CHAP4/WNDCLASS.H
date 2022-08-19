//==================================
// SHOWWND - Matt Pietrek 1995
// FILE: WNDCLASS.H
//==================================

#pragma pack(1)

typedef struct _USER_DGROUP_WNDCLASS
{
    DWORD   lpIntWndClass;  // 00h
    WORD    hcNext;         // 04h
    ATOM    classNameAtom;  // 06h
    DWORD   style;          // 08h
} USER_DGROUP_WNDCLASS, *PUSER_DGROUP_WNDCLASS;

typedef struct _INTWNDCLASS
{
    DWORD   cClsWnds;       // 00h
    DWORD   lpfnWndProc;    // 04h
    WORD    cbClsExtra;     // 08h
    WORD    hModule;        // 0Ah
    WORD    hIcon;          // 0Ch
    WORD    hCursor;        // 0Eh
    WORD    hBrBackground;  // 10h
    DWORD   lpszMenuName;   // 12h
    WORD    hIconSm;        // 16h
    WORD    cbWndExtra;     // 18h
} INTWNDCLASS, *PINTWNDCLASS, *LPINTWNDCLASS;

#pragma pack()
