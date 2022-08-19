//==================================
// WIN32WLK - Matt Pietrek 1995
// FILE: THREADDB.H
//==================================
#pragma pack(1)

#include "tib.h"
#include "tdbx.h"

typedef struct _THREAD_DATABASE
{
DWORD   Type;               // 00h

DWORD   cReference;         // 04h

PPROCESS_DATABASE pProcess; // 08h

DWORD   someEvent;          // 0Ch An event object (What's it used for???)

DWORD   pvExcept;           // 10h This field through field 3CH is a TIB
                            //      structure (see TIB.H)
                                
DWORD   TopOfStack;         // 14h

DWORD   StackLow;           // 18h

WORD    W16TDB;             // 1Ch

WORD    StackSelector16;    // 1Eh Used when thunking down to 16 bits

DWORD   SelmanList;         // 20h

DWORD   UserPointer;        // 24h

PTIB    pTIB;               // 28h

WORD    TIBFlags;           // 2Ch  TIBF_WIN32 = 1, TIBF_TRAP = 2

WORD    Win16MutexCount;    // 2Eh

DWORD   DebugContext;       // 30h

PDWORD  pCurrentPriority;   // 34h

DWORD   MessageQueue;       // 38h

DWORD   pTLSArray;          // 3Ch

PPROCESS_DATABASE pProcess2;// 40h Another copy of the thread's process???

DWORD   Flags;              // 44h
                            // 0x00000001 - fCreateThreadEvent
                            // 0x00000002 - fCancelExceptionAbort
                            // 0x00000004 - fOnTempStack
                            // 0x00000008 - fGrowableStack
                            // 0x00000010 - fDelaySingleStep
                            // 0x00000020 - fOpenExeAsImmovableFile
                            // 0x00000040 - fCreateSuspended
                            // 0x00000080 - fStackOverflow
                            // 0x00000100 - fNestedCleanAPCs
                            // 0x00000200 - fWasOemNowAnsi
                            // 0x00000400 - fOKToSetThreadOem

DWORD   TerminationStatus;  // 48h Returned by GetExitCodeThread

WORD    TIBSelector;        // 4Ch

WORD    EmulatorSelector;   // 4Eh

DWORD   cHandles;           // 50h

DWORD   WaitNodeList;       // 54h

DWORD   un4;                // 58h

DWORD   Ring0Thread;        // 5Ch

PTDBX   pTDBX;              // 60

DWORD   StackBase;          // 64h

DWORD   TerminationStack;   // 68h

DWORD   EmulatorData;       // 6Ch

DWORD   GetLastErrorCode;   // 70h

DWORD   DebuggerCB;         // 74h

DWORD   DebuggerThread;     // 78h

PCONTEXT    ThreadContext;  // 7Ch  // register context defined in WINNT.H

DWORD   Except16List;       // 80h

DWORD   ThunkConnect;       // 84h

DWORD   NegStackBase;       // 88h

DWORD   CurrentSS;          // 8Ch

DWORD   SSTable;            // 90h

DWORD   ThunkSS16;          // 94h

DWORD   TLSArray[64];       // 98h

DWORD   DeltaPriority;      // 198h

// The retail version breaks off somewhere around here.
// All the remaining fields are most likely only in the debug version

DWORD   un5[7];             // 19Ch

DWORD   pCreateData16;      // 1B8h

DWORD   APISuspendCount;    // 1BCh # of times SuspendThread has been called

DWORD   un6;                // 1C0h

DWORD   WOWChain;           // 1C4h

WORD    wSSBig;             // 1C8h

WORD    un7;                // 1CAh

DWORD   lp16SwitchRec;      // 1CCh

DWORD   un8[6];             // 1D0h

DWORD   pSomeCritSect1;     // 1E8h

DWORD   pWin16Mutex;        // 1ECh

DWORD   pWin32Mutex;        // 1F0h

DWORD   pSomeCritSect2;     // 1F4h

DWORD   un9;                // 1F8h

DWORD   ripString;          // 1FCh

DWORD   LastTlsSetValueEIP[64]; // 200h (parallel to TlsArray, contains EIP
                                //      where TLS value was last set from)
                                            
} THREAD_DATABASE, *PTHREAD_DATABASE;
#pragma pack()
