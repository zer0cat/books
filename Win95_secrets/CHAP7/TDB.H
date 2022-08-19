//==================================
// SHOW16 - Matt Pietrek 1995
// FILE: TDB.H
//==================================
#pragma pack (1)

typedef struct
{
    HTASK   TDB_next;           // next task in dispatch queue
    WORD    TDB_taskSP;         // Saved SS:SP for this task
    WORD    TDB_taskSS;         //
    WORD    TDB_nEvents;        // Task event counter
    BYTE    TDB_priority;       // Task priority (0 is highest)
    BYTE    TDB_thread_ordinal; // ordinal number of this thread
    WORD    TDB_thread_next;    // next thread
    WORD    TDB_thread_tdb;     // the real TDB for this task
    WORD    TDB_thread_list;    // list of allocated thread structures
    WORD    TDB_thread_free;    // free list of available thread structures
    WORD    TDB_thread_count;   // total count of tread structures
    WORD    TDB_FCW;            // Floating point control word
    WORD    TDB_flags;          // Task flags
    WORD    TDB_ErrMode;        // Error mode for this task
    WORD    TDB_ExpWinVer;      // Expected Windows version for this task
    WORD    TDB_HInstance;      // instance handle of task
    WORD    TDB_HMODULE;        // module database for task
    WORD    TDB_Queue;          // Task Event Queue pointer
    WORD    TDB_Parent;         // TDB of the task that started this up
    WORD    TDB_SigAction;      // Action for app task signal
    FARPROC TDB_ASignalProc;    // App's signal procedure address
    FARPROC TDB_USignalProc;    // User's Task Signal procedure address
    FARPROC TDB_GNotifyProc;    // Task global discard notify proc.
    FARPROC TDB_INTVECS[7];     // Task specfic hardware interrupts
    DWORD   TDB_CompatFlags;    // Compatibility flags
    WORD    TDB_FS_selector;    // Same selector as FS (points inside TCB)
    DWORD   TDB_ring3_thread_db;// 32 bit address of KERNEL32 thread database
    WORD    TDB_thunk_stack_ss; // selector used for stack during thunking
    WORD    TDB_filler[3];      // appears to be unused
    WORD    TDB_PSP;            // MSDOS Process Data Block (aka, the PSP)
    LPBYTE  TDB_DTA;            // MSDOS Disk Transfer Address
    BYTE    TDB_Drive;          // MSDOS current drive
    char    TDB_Directory[65];  // MSDOS current directory
    WORD    TDB_Validity;       // initial AX to be passed to a task
    HTASK   TDB_Yield_to;       // DirectedYield arg stored here
    WORD    TDB_LibInitSeg;     // segment address of libraries to init
    WORD    TDB_LibInitOff;     //
                                // MakeProcInstance thunks live here.
    WORD    TDB_MPI_Sel;        // Code selector for thunks
    WORD    TDB_more_thunks;    // selector of segment with more MPI thunks
    WORD    TDB_PT_sig;         // 'PT'
    WORD    TDB_unused1;        //
    WORD    TDB_next_MPI_thunk; //
    char    TDB_MPI_Thunks[0x38];   //
    char    TDB_ModName[8];     // Name of Module.
    WORD    TDB_sig;            // Signature word to detect bogus code
    DWORD   TDB_unused2;
    char    TDB_current_directory[0x110];   // Current directory
} TDB, FAR * LPTDB;

#define TDB_FLAGS_WIN32     0x0010
#define TDB_FLAGS_WINOLDAP  0x0001
