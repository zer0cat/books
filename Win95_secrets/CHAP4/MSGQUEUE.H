//==================================
// SHOWWND - Matt Pietrek 1995
// FILE: MSGQUEUE.H
//==================================

#pragma pack(1)

// type == LT_USER_VWININFO(0x1B), offset 10h in msg queue
typedef struct _PERQUEUEDATA
{
WORD    npNext;         // 00h a USER heap handle (type == LT_USER_VWININFO)
WORD    un2;            // 02h
WORD    un3;            // 04h
WORD    npQMsg;         // 06h type == LT_USER_QMSG
WORD    un5;            // 08h
WORD    un6;            // 0Ah
WORD    un7;            // 0Ch
WORD    un8;            // 0Eh
WORD    un9;            // 10h
WORD    un10;           // 12h
WORD    somehQueue1;    // 14h a msg queue handle
WORD    somehQueue2;    // 16h a msg queue handle
DWORD   hWndCapture;    // 18h
DWORD   hWndFocus;      // 1Ch
DWORD   hWndActive;     // 20h
} PERQUEUEDATA, *PPERQUEUEDATA;

// type == LT_USER_QMSG(0x1A)
typedef struct _QUEUEMSG
{
WORD    hWnd;           // 00h
WORD    msg;            // 02h
WORD    wParamLow;      // 04h
DWORD   lParam;         // 06h
DWORD   messageTime;    // 0Ah  GetMessageTime
DWORD   messagePos;     // 0Eh  GetMessagePos
WORD    wParamHigh;     // 12h  HIWORD of wParam for 32 bit apps
DWORD   extraInfo;      // 14h  GetMessageExtraInfo
WORD    nextQueueMsg;   // 18h  Near offset to next QUEUEMSG
} QUEUEMSG, *PQUEUEMSG;

// LT_USER_PROCESS(0x1D), offset 16h in the msg queue
// All queues belonging to the same process have a pointer to this struct
typedef struct _QUEUEPROCESSDATA
{
WORD    npNext;     // 00h ptr to next QUEUEPROCESSDATA
WORD    un2;        // 02h type == LT_USER_SUBSYSTEM
WORD    flags;      // 04h
WORD    un3;        // 06h
DWORD   processId;  // 08h e.g., GetCurrentProcessId
WORD    un5;        // 0Ch
WORD    hQueue;     // 0Eh an hQueue belonging to this process (which one?)
} QUEUEPROCESSDATA, *PQUEUEPROCESSDATA;

typedef struct _MSGQUEUE
{
WORD    nextQueue;  // 00h  next queue in the list
WORD    hTask;      // 02h  Task that this queue is associated with
WORD    headMsg;    // 04h  Near ptr to head of linked list of QUEUEMSGs
WORD    tailMsg;    // 06h  Near ptr to end of list of QUEUEMSGs
WORD    lastMsg;    // 08h  Near ptr to last msg retrieved (not really!)
WORD    cMsgs;      // 0Ah  Number of messages
BYTE    un1;        // 0Ch  ???
BYTE    sig[3];     // 0Dh  "MJT" (Jon Thomason?)
WORD    npPerQueue; // 10h  16 bit offset in USER DGROUP to PERQUEUEDATA
                    //      type == LT_USER_VWININFO???
WORD    un2;        // 12h  ???
WORD    un2_5;      // 14h  ??
WORD    npProcess;  // 16h  near pointer in USER DGROUP to a QUEUEPROCESSDATA
DWORD   un3[3];     // 18h  ???
DWORD   messageTime;// 24h  retrieved by GetMessageTime()
DWORD   messagePos; // 28h  retrived by GetMessagePos()
WORD    un4;        // 2Ch  ??? (seems to always be 0)
WORD    lastMsg2;   // 2Eh  Near ptr to last retrieved QUEUEMSG
DWORD   extraInfo;  // 30h  returned by GetMessageExtraInfo()
DWORD   un5[2];     // 34h  ???
DWORD   threadId;   // 3Ch  See GetWindowProcessThreadId
WORD    un6;        // 40h  ??
WORD    expWinVer;  // 42h  Version of Windows this app expects
DWORD   un7;        // 44h  ???
WORD    ChangeBits; // 48h  high order word returned by GetQueueStatus
WORD    WakeBits;   // 4Ah  low order word returned by GetQueueStatus
WORD    WakeMask;   // 4Ch  The QS_xxx bits that GetMessage/PeekMessage are
                    //      waiting for
WORD    un8;        // 4Eh  ???
WORD    hQueueSend; // 50h  App that's in SendMessage to this queue
DWORD   un9;        // 52h  ???
WORD    sig2;       // 56h  "HQ"
} MSGQUEUE, *PMSGQUEUE;

#pragma pack()
