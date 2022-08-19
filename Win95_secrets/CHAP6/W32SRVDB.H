//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SRVDB.H
//==================================
typedef struct
{
    PSTR    pszServiceName;     // NULL if not known
    BOOL    fIgnore;
} WIN32_SERVICE_CALL, *PWIN32_SERVICE_CALL;

typedef struct
{
    PSTR                pszVxDName;
    DWORD               vxdID;          // 0x002A0000, etc...
    PWIN32_SERVICE_CALL pServiceCalls;
    DWORD               cServiceCalls;
    BOOL                fIgnore;
} W32_SERVICE_VXD, *PW32_SERVICE_VXD;

extern W32_SERVICE_VXD W32ServiceTable[];
extern unsigned CWin32ServiceVxDs;

PWIN32_SERVICE_CALL LookupWin32ServiceCall( DWORD serviceID );
