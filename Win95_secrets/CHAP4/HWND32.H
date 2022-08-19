//==================================
// SHOWWND - Matt Pietrek 1995
// FILE: HWND32.H
//==================================
#pragma pack (1)
typedef struct _RECTS
{
    WORD    left;
    WORD    top;
    WORD    right;
    WORD    bottom;
} RECTS, *PRECTS, *LPRECTS;

typedef struct _WND32
{
struct _WND32 *hWndNext;    // 00h (GW_HWNDNEXT) HWND of next sibling window
struct _WND32 *hWndChild;   // 04h (GW_CHILD) First child window
struct _WND32 *hWndParent;  // 08h Parent window handle
struct _WND32 *hWndOwner;   // 0Ch Owning window handle
RECTS   rectWindow;         // 10h Rectangle describing entire window
RECTS   rectClient;         // 18h Rectangle for client area of window
WORD    hQueue;             // 20h Application message queue handle
WORD    hrgnUpdate;         // 22h window region needing an update
WORD    wndClass;           // 24h handle to an INTWNDCLASS
WORD    hInstance;          // 26h hInstance of creating application
WNDPROC lpfnWndProc;        // 28h Window procedure address
DWORD   dwFlags;            // 2Ch internal state flags
DWORD   dwStyleFlags;       // 30h WS_XXX style flags
DWORD   dwExStyleFlags;     // 34h WS_EX_XXX extended style flags
DWORD   moreFlags;          // 38h flags
HANDLE  ctrlID;             // 3Ch GetDlgCtrlId or hMenu
WORD    windowTextOffset;   // 40h Offset of the window's text in atom heap
WORD    scrollBar;          // 42h DWORD associated with the scroll bars
WORD    properties;         // 44h Handle for first window property
WORD    hWnd16;             // 46h Actual HWND value for this window
struct _WND32 *lastActive;  // 48h Last active owned popup window
HANDLE  hMenuSystem;        // 4Ch handle to the system menu
DWORD   un1;                // 50h
WORD    un2;                // 54h
WORD    classAtom;          // 56h See also offs. 2 in the field 24 struct ptr
DWORD   alternatePID;       // 58h
DWORD   alternateTID;       // 5Ch
} WND32, *PWND32;

#pragma pack()
