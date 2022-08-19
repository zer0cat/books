//==================================
// WIN32WLK - Matt Pietrek 1995
// FILE: TDBX.H
//==================================
#pragma pack(1)

typedef struct _TDBX
{
    DWORD   ptdb;               // 00h  // PTHREAD_DATABASE
    DWORD   ppdb;               // 04h  // PPROCESDS_DATABASE
    DWORD   ContextHandle;      // 08h
    DWORD   un1;                // 0Ch
    DWORD   TimeOutHandle;      // 10h
    DWORD   WakeParam;          // 14h
    DWORD   BlockHandle;        // 18h
    DWORD   BlockState;         // 1Ch
    DWORD   SuspendCount;       // 20h
    DWORD   SuspendHandle;      // 24h
    DWORD   MustCompleteCount;  // 28h
    DWORD   WaitExFlags;        // 2Ch
                                // 0x00000001 - WAITEXBIT
                                // 0x00000002 - WAITACKBIT
                                // 0x00000004 - SUSPEND_APC_PENDING
                                // 0x00000008 - SUSPEND_TERMINATED
                                // 0x00000010 - BLOCKED_FOR_TERMINATION
                                // 0x00000020 - EMULATE_NPX
                                // 0x00000040 - WIN32_NPX
                                // 0x00000080 - EXTENDED_HANDLES
                                // 0x00000100 - FROZEN
                                // 0x00000200 - DONT_FREEZE
                                // 0x00000400 - DONT_UNFREEZE
                                // 0x00000800 - DONT_TRACE
                                // 0x00001000 - STOP_TRACING
                                // 0x00002000 - WAITING_FOR_CRST_SAFE
                                // 0x00004000 - CRST_SAFE
                                // 0x00040000 - BLOCK_TERMINATE_APC
    DWORD   SyncWaitCount;      // 30h
    DWORD   QueuedSyncFuncs;    // 34h
    DWORD   UserAPCList;        // 38h
    DWORD   KernAPCList;        // 3Ch
    DWORD   pPMPSPSelector;     // 40h
    DWORD   BlockedOnID;        // 44h
    DWORD   un2[7];             // 48h
    DWORD   TraceRefData;       // 64h
    DWORD   TraceCallBack;      // 68h
    DWORD   TraceEventHandle;   // 6Ch
    WORD    TraceOutLastCS;     // 70h
    WORD    K16TDB;             // 72h
    WORD    K16PDB;             // 74h
    WORD    DosPDBSeg;          // 76h
    WORD    ExceptionCount;     // 78h
} TDBX, *PTDBX;

#pragma pack()
