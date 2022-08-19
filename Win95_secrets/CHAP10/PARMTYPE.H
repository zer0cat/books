//==================================
// APISPY32 - Matt Pietrek 1995
// FILE: PARMTYPE.H
//==================================

typedef enum
{
PARAM_NONE=0,
PARAM_DWORD=1,
PARAM_WORD=2,
PARAM_BYTE=3,
PARAM_LPSTR=4,
PARAM_LPWSTR=5,
PARAM_LPDATA=6,
PARAM_HANDLE=7,
PARAM_HWND=8,
PARAM_BOOL=9,
PARAM_LPCODE=10,
} PARAMTYPE;