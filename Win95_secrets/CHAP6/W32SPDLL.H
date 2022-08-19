//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SPDLL.H
//==================================
typedef struct
{
    DWORD   serviceId;
    DWORD   processId;
    DWORD   threadId;
    DWORD   param1;
    char    szName[8];      // Not null terminated if 8 bytes long!!!
} WIN32SERVICECALLINFO, *PWIN32SERVICECALLINFO;

void InitWin32ServiceSpyDLL(void);
void StopWin32ServiceSpy(void);
BOOL GetWin32ServiceLogInfo( PDWORD, PWIN32SERVICECALLINFO * );
