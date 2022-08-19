//==================================
// W32SVSPY - Matt Pietrek 1995
// FILE: W32SRVDB.C
//==================================
#include <windows.h>
#pragma hdrstop
#include "w32srvdb.h"

WIN32_SERVICE_CALL VMMServiceCalls[] = {
{ "_PageReserve", FALSE },      // 0x00010000
{ "_PageCommit", FALSE },       // 0x00010001
{ "_PageDecommit", FALSE },     // 0x00010002
{ "_PagerRegister", FALSE },        // 0x00010003
{ "_PagerQuery", FALSE },       // 0x00010004
{ "_HeapAllocate", FALSE },     // 0x00010005
{ "_ContextCreate", FALSE },        // 0x00010006
{ "_ContextDestroy", FALSE },       // 0x00010007
{ "_PageAttach", FALSE },       // 0x00010008
{ "_PageFlush", FALSE },        // 0x00010009
{ "_PageFree", FALSE },     // 0x0001000A
{ "_ContextSwitch", FALSE },        // 0x0001000B
{ "_HeapReAllocate", FALSE },       // 0x0001000C
{ "_PageModifyPermissions", FALSE },        // 0x0001000D
{ "_PageQuery", FALSE },        // 0x0001000E
{ "_GetCurrentContext", FALSE },        // 0x0001000F
{ "_HeapFree", FALSE },     // 0x00010010
{ "_RegOpenKey", FALSE },       // 0x00010011
{ "_RegCreateKey", FALSE },     // 0x00010012
{ "_RegCloseKey", FALSE },      // 0x00010013
{ "_RegDeleteKey", FALSE },     // 0x00010014
{ "_RegSetValue", FALSE },      // 0x00010015
{ "_RegDeleteValue", FALSE },       // 0x00010016
{ "_RegQueryValue", FALSE },        // 0x00010017
{ "_RegEnumKey", FALSE },       // 0x00010018
{ "_RegEnumValue", FALSE },     // 0x00010019
{ "_RegQueryValueEx", FALSE },      // 0x0001001A
{ "_RegSetValueEx", FALSE },        // 0x0001001B
{ "_RegFlushKey", FALSE },      // 0x0001001C
{ "_GetDemandPageInfo", FALSE },        // 0x0001001E
{ "_BlockOnID", FALSE },        // 0x0001001F
{ "_SignalID", FALSE },     // 0x00010020
{ "_RegLoadKey", FALSE },       // 0x00010021
{ "_RegUnLoadKey", FALSE },     // 0x00010022
{ "_RegSaveKey", FALSE },       // 0x00010023
{ "_RegRemapPreDefKey", FALSE },        // 0x00010024
{ "_PageChangePager", FALSE },      // 0x00010025
{ "_RegQueryMultipleValues", FALSE },       // 0x00010026
{ "_RegReplaceKey" , FALSE},            // 0x00010027
};

WIN32_SERVICE_CALL VWIN32ServiceCalls[] = {
{ "GetVersion", FALSE },        // 0x002A0000
{ "Stuff VWIN32 code pointers into caller supplied buffer", FALSE },        // 0x002A0001
{ "GetSystemTime", FALSE },     // 0x002A0002
{ "Stuff code pointers from caller into VWIN32's Data area", FALSE },       // 0x002A0003
{ "Block on some semaphore", FALSE },       // 0x002A0004
{ "Calls Signal_Semaphore_No_Switch on 0x002A0004 semaphore", FALSE },      // 0x002A0005
{ "Calls VMM Create_Semaphore and saves it", FALSE },       // 0x002A0006
{ "Calls VMM Destroy _Semaphore on 0x002A0006 sem", FALSE },        // 0x002A0007
{ "VWIN32_CreateThread", FALSE },       // 0x002A0008
{ "VWIN32_Sleep", FALSE },      // 0x002A0009
{ "WakeThread", FALSE },        // 0x002A000A
{ "TerminateThread ???", FALSE },       // 0x002A000B
{ "Some sort of initialization", FALSE },       // 0x002A000C
{ "_VWIN32_QueueUserApc", FALSE },      // 0x002A000D
{ "VWIN32_Initialize", FALSE },     // 0x002A000E
{ "_VWIN32_QueueKernelApc", FALSE },        // 0x002A000F
{ "VINW32_Int21Dispatch", FALSE },      // 0x002A0010
{ "Calls IFSMgr_Win32DupHandle", FALSE },       // 0x002A0011
{ "VWIN32_BlockThreadSetBit", FALSE },      // 0x002A0012
{ "Adjust_Thread_Exec_Priority", FALSE },       // 0x002A0013
{ "_VWIN32_Get_Thread_Context", FALSE },        // 0x002A0014
{ "_VWIN32_Set_Thread_Context", FALSE },        // 0x002A0015
{ "Read process memory", FALSE },       // 0x002A0016
{ "Write process memory", FALSE },      // 0x002A0017
{ "calls VMCPD_Get_CR0_State", FALSE },     // 0x002A0018
{ "calls VMCPD_Set_CR0_State", FALSE },     // 0x002A0019
{ "SuspendThread", FALSE },     // 0x002A001A
{ "ResumeThread", FALSE },      // 0x002A001B
{ "??? 0x002A001C", FALSE },        // 0x002A001C
{ "WaitCrst", FALSE },      // 0x002A001D
{ "WakeCrst", FALSE },      // 0x002A001E
{ "Something with loading/unloading VxDs", FALSE },     // 0x002A001F
{ "VMCPD_Get_Version", FALSE },     // 0x00200020
{ "Set_Thread_Win32_Pri", FALSE },      // 0x002A0021
{ "calls Boost_With_Decay", FALSE },        // 0x002A0022
{ "calls Set_Inversion_Pri", FALSE },       // 0x002A0023
{ "calls Release_Inversion_Pri_ID", FALSE },        // 0x002A0024
{ "calls Release_Inversion_Pri", FALSE },       // 0x002A0025
{ "calls Attach_Thread_To_Group", FALSE },      // 0x002A0026
{ "calls Set_Thread_Static_Boost", FALSE },     // 0x002A0027
{ "calls Set_Group_Static_Boost", FALSE },      // 0x002A0028
{ "VWIN32_Int31Dispatch", FALSE },      // 0x002A0029
{ "VWIN32_Int41Dispatch", FALSE },      // 0x002A002A
{ "VWIN32_BlockForTermination", FALSE },        // 0x002A002B
{ "TerminationHandler2 (??? )", FALSE },        // 0x002A002C
{ "??? 0x002A002D", FALSE },        // 0x002A002D
{ "dwBlockSingleWnod", FALSE },     // 0x002A002E
{ "dwBlockMultipleWnod", FALSE },       // 0x002A002F
{ "VWIN32_SetEvent", FALSE },       // 0x002A0030
{ "Something to do with delivering APCs", FALSE },      // 0x002A0031
{ "??? 0x002A0032", FALSE },        // 0x002A0032
{ "InitUserAPCList", FALSE },       // 0x002A0033
{ "??? 0x002A0034", FALSE },        // 0x002A0034
{ "Calls VMM Signal_Semaphore_No_Switch", FALSE },      // 0x002A0035
{ "calls System_Control(KERNEL32_INITIALIZED)", FALSE },        // 0x002A0036
{ "VWIN32_CommonFaultPopup", FALSE },       // 0x002A0037
{ "VWIN32_ForceCrsts", FALSE },     // 0x002A0038
{ "??? 0x002A0039", FALSE },        // 0x002A0039
{ "VWIN32_FreezeAllThreads", FALSE },       // 0x002A003A
{ "VWIN32_UnFreezeAllThreads", FALSE },     // 0x002A003B
{ "calls IFSMgr_Ring0_FileIO", FALSE },     // 0x002A003C
{ "calls Attach_Thread_To_Group and Boost_Thread_With_VM", FALSE },     // 0x002A003D
{ "VWIN32_ActiveTimeBiasSet", FALSE },      // 0x002A003E
{ "ModifyPagePermission", FALSE },      // 0x002A003F
{ "used by VirtualQueryEx", FALSE },        // 0x002A0040
{ "ForceLeaveCrst", FALSE },        // 0x002A0041
{ "ForceEnterCrst", FALSE },        // 0x002A0042
{ "Calls VMCPD_Set_Thread_Excpt_Type", FALSE },     // 0x002A0043
{ "calls VTD_Get_Real_Time", FALSE },       // 0x002A0044
{ "calls System_Control(SET_DEVICE_FOCUS)", FALSE },        // 0x002A0045
{ "calls VWIN32_UnFreezeThread", FALSE },       // 0x002A0046
{ "calls VMM_Replace_Global_Environment", FALSE },      // 0x002A0047
{ "calls System_Control(KERNEL32_SHUTDOWN)", FALSE },       // 0x002A0048
{ "??? 0x002A0049", FALSE },        // 0x002A0049
{ "VW32_AddSysCrst", FALSE },       // 0x002A004A
{ "VW32_AddSysCrst", FALSE },       // 0x002A004B
{ "VW32_Cancel_Time_Out", FALSE },      // 0x002A004C
{ "??? 0x002A004D", FALSE },        // 0x002A004D
{ "setting and reflecting hotkeys", FALSE },        // 0x002A004E
};

#define NUMSERVS(x) ( sizeof(x)/sizeof(WIN32_SERVICE_CALL) )

W32_SERVICE_VXD W32ServiceTable[] =
{
{ "VMM",    0x00010000, VMMServiceCalls, NUMSERVS(VMMServiceCalls), FALSE },
{ "VWIN32", 0x002A0000, VWIN32ServiceCalls, NUMSERVS(VWIN32ServiceCalls), FALSE },
};

unsigned CWin32ServiceVxDs = sizeof(W32ServiceTable) / sizeof(W32_SERVICE_VXD);

PWIN32_SERVICE_CALL LookupWin32ServiceCall( DWORD serviceID )
{
    unsigned i;
    
    for (i=0; i < CWin32ServiceVxDs; i++)
        if ( W32ServiceTable[i].vxdID == (serviceID & 0xFFFF0000) )
            break;
        
    if ( i >= CWin32ServiceVxDs )
        return 0;
    
    if ( LOWORD(serviceID) >= W32ServiceTable[i].cServiceCalls )
        return 0;
    
    return &W32ServiceTable[i].pServiceCalls[LOWORD(serviceID)];
}
