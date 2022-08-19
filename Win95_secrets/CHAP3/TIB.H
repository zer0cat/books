//==================================
// SHOWSEH - Matt Pietrek 1995
// FILE: TIB.H
//==================================
#pragma pack(1)

typedef struct _SEH_record
{
    struct _SEH_record *pNext;
    FARPROC             pfnHandler;
} SEH_record, *PSEH_record;

// This is semi-documented in the NTDDK.H file from the NT DDK
typedef struct _TIB
{
PSEH_record pvExcept;       // 00h Head of exception record list
PVOID   pvStackUserTop;     // 04h Top of user stack
PVOID   pvStackUserBase;    // 08h Base of user stack
WORD    pvTDB;              // 0Ch TDB
WORD    pvThunksSS;         // 0Eh SS selector used for thunking to 16 bits
DWORD   SelmanList;         // 10h
PVOID   pvArbitrary;        // 14h Available for application use
struct _tib *ptibSelf;      // 18h Linear address of TIB structure
WORD    TIBFlags;           // 1Ch
WORD    Win16MutexCount;    // 1Eh
DWORD   DebugContext;       // 20h
DWORD   pCurrentPriority;   // 24h
DWORD   pvQueue;            // 28h Message Queue selector
PVOID*  pvTLSArray;         // 2Ch Thread Local Storage array
} TIB, *PTIB;
#pragma pack()
