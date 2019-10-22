
// __________________________________________________________
//
//                         w2k_spy.c
//             SBS Windows 2000 Spy Device V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#define  _W2K_SPY_SYS_
#include <ddk\ntddk.h>
#include "w2k_spy.h"

// =================================================================
// DISCLAIMER
// =================================================================

/*

This software is provided "as is" and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed.
In no event shall the author Sven B. Schreiber be liable for any
direct, indirect, incidental, special, exemplary, or consequential
damages (including, but not limited to, procurement of substitute
goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software,
even if advised of the possibility of such damage.

*/

// =================================================================
// REVISION HISTORY
// =================================================================

/*

08-27-2000 V1.00 Original version (SBS).

*/

// =================================================================
// EXTERNAL VARIABLES
// =================================================================

extern PWORD                     NlsAnsiCodePage;
extern PWORD                     NlsOemCodePage;
extern PWORD                     NtBuildNumber;
extern PDWORD                    NtGlobalFlag;
extern PDWORD                    KeI386MachineType;
extern PSERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTable;

// =================================================================
// GLOBAL DATA
// =================================================================

PRESET_UNICODE_STRING (usDeviceName,       CSTRING (DRV_DEVICE));
PRESET_UNICODE_STRING (usSymbolicLinkName, CSTRING (DRV_LINK  ));

PDEVICE_OBJECT  gpDeviceObject  = NULL;
PDEVICE_CONTEXT gpDeviceContext = NULL;

BOOL            gfSpyHookState  = FALSE;
BOOL            gfSpyHookPause  = FALSE;
BOOL            gfSpyHookFilter = FALSE;
HANDLE          ghSpyHookThread = 0;

BYTE            abHex []        = "0123456789ABCDEF";

// =================================================================
// DISCARDABLE FUNCTIONS
// =================================================================

NTSTATUS DriverInitialize (PDRIVER_OBJECT  pDriverObject,
                           PUNICODE_STRING pusRegistryPath);

NTSTATUS DriverEntry      (PDRIVER_OBJECT  pDriverObject,
                           PUNICODE_STRING pusRegistryPath);

// -----------------------------------------------------------------

#ifdef ALLOC_PRAGMA

#pragma alloc_text (INIT, DriverInitialize)
#pragma alloc_text (INIT, DriverEntry)

#endif

// =================================================================
// WINDOWS NT 4.0 SYSTEM SERVICE NAMES
// =================================================================
// number of entries must match SDT_SYMBOLS_NT4 in w2k_spy.h

PBYTE apbSdtSymbolsNT4 [] =
    {
    "NtAcceptConnectPort",
    "NtAccessCheck",
    "NtAccessCheckAndAuditAlarm",
    "NtAddAtom",
    "NtAdjustGroupsToken",
    "NtAdjustPrivilegesToken",
    "NtAlertResumeThread",
    "NtAlertThread",
    "NtAllocateLocallyUniqueId",
    "NtAllocateUuids",
    "NtAllocateVirtualMemory",
    "NtCallbackReturn",
    "NtCancelIoFile",
    "NtCancelTimer",
    "NtClearEvent",
    "NtClose",
    "NtCloseObjectAuditAlarm",
    "NtCompleteConnectPort",
    "NtConnectPort",
    "NtContinue",
    "NtCreateDirectoryObject",
    "NtCreateEvent",
    "NtCreateEventPair",
    "NtCreateFile",
    "NtCreateIoCompletion",
    "NtCreateKey",
    "NtCreateMailslotFile",
    "NtCreateMutant",
    "NtCreateNamedPipeFile",
    "NtCreatePagingFile",
    "NtCreatePort",
    "NtCreateProcess",
    "NtCreateProfile",
    "NtCreateSection",
    "NtCreateSemaphore",
    "NtCreateSymbolicLinkObject",
    "NtCreateThread",
    "NtCreateTimer",
    "NtCreateToken",
    "NtDelayExecution",
    "NtDeleteAtom",
    "NtDeleteFile",
    "NtDeleteKey",
    "NtDeleteObjectAuditAlarm",
    "NtDeleteValueKey",
    "NtDeviceIoControlFile",
    "NtDisplayString",
    "NtDuplicateObject",
    "NtDuplicateToken",
    "NtEnumerateKey",
    "NtEnumerateValueKey",
    "NtExtendSection",
    "NtFindAtom",
    "NtFlushBuffersFile",
    "NtFlushInstructionCache",
    "NtFlushKey",
    "NtFlushVirtualMemory",
    "NtFlushWriteBuffer",
    "NtFreeVirtualMemory",
    "NtFsControlFile",
    "NtGetContextThread",
    "NtGetPlugPlayEvent",
    "NtGetTickCount",
    "NtImpersonateClientOfPort",
    "NtImpersonateThread",
    "NtInitializeRegistry",
    "NtListenPort",
    "NtLoadDriver",
    "NtLoadKey",
    "NtLoadKey2",
    "NtLockFile",
    "NtLockVirtualMemory",
    "NtMakeTemporaryObject",
    "NtMapViewOfSection",
    "NtNotifyChangeDirectoryFile",
    "NtNotifyChangeKey",
    "NtOpenDirectoryObject",
    "NtOpenEvent",
    "NtOpenEventPair",
    "NtOpenFile",
    "NtOpenIoCompletion",
    "NtOpenKey",
    "NtOpenMutant",
    "NtOpenObjectAuditAlarm",
    "NtOpenProcess",
    "NtOpenProcessToken",
    "NtOpenSection",
    "NtOpenSemaphore",
    "NtOpenSymbolicLinkObject",
    "NtOpenThread",
    "NtOpenThreadToken",
    "NtOpenTimer",
    "NtPlugPlayControl",
    "NtPrivilegeCheck",
    "NtPrivilegedServiceAuditAlarm",
    "NtPrivilegeObjectAuditAlarm",
    "NtProtectVirtualMemory",
    "NtPulseEvent",
    "NtQueryInformationAtom",
    "NtQueryAttributesFile",
    "NtQueryDefaultLocale",
    "NtQueryDirectoryFile",
    "NtQueryDirectoryObject",
    "NtQueryEaFile",
    "NtQueryEvent",
    "NtQueryFullAttributesFile",
    "NtQueryInformationFile",
    "NtQueryIoCompletion",
    "NtQueryInformationPort",
    "NtQueryInformationProcess",
    "NtQueryInformationThread",
    "NtQueryInformationToken",
    "NtQueryIntervalProfile",
    "NtQueryKey",
    "NtQueryMultipleValueKey",
    "NtQueryMutant",
    "NtQueryObject",
    "NtQueryOleDirectoryFile",
    "NtQueryPerformanceCounter",
    "NtQuerySection",
    "NtQuerySecurityObject",
    "NtQuerySemaphore",
    "NtQuerySymbolicLinkObject",
    "NtQuerySystemEnvironmentValue",
    "NtQuerySystemInformation",
    "NtQuerySystemTime",
    "NtQueryTimer",
    "NtQueryTimerResolution",
    "NtQueryValueKey",
    "NtQueryVirtualMemory",
    "NtQueryVolumeInformationFile",
    "NtQueueApcThread",
    "NtRaiseException",
    "NtRaiseHardError",
    "NtReadFile",
    "NtReadFileScatter",
    "NtReadRequestData",
    "NtReadVirtualMemory",
    "NtRegisterThreadTerminatePort",
    "NtReleaseMutant",
    "NtReleaseSemaphore",
    "NtRemoveIoCompletion",
    "NtReplaceKey",
    "NtReplyPort",
    "NtReplyWaitReceivePort",
    "NtReplyWaitReplyPort",
    "NtRequestPort",
    "NtRequestWaitReplyPort",
    "NtResetEvent",
    "NtRestoreKey",
    "NtResumeThread",
    "NtSaveKey",
    "NtSetIoCompletion",
    "NtSetContextThread",
    "NtSetDefaultHardErrorPort",
    "NtSetDefaultLocale",
    "NtSetEaFile",
    "NtSetEvent",
    "NtSetHighEventPair",
    "NtSetHighWaitLowEventPair",
    "NtSetHighWaitLowThread (INT 2B)",
    "NtSetInformationFile",
    "NtSetInformationKey",
    "NtSetInformationObject",
    "NtSetInformationProcess",
    "NtSetInformationThread",
    "NtSetInformationToken",
    "NtSetIntervalProfile",
    "NtSetLdtEntries",
    "NtSetLowEventPair",
    "NtSetLowWaitHighEventPair",
    "NtSetLowWaitHighThread (INT 2C)",
    "NtSetSecurityObject",
    "NtSetSystemEnvironmentValue",
    "NtSetSystemInformation",
    "NtSetSystemPowerState",
    "NtSetSystemTime",
    "NtSetTimer",
    "NtSetTimerResolution",
    "NtSetValueKey",
    "NtSetVolumeInformationFile",
    "NtShutdownSystem",
    "NtSignalAndWaitForSingleObject",
    "NtStartProfile",
    "NtStopProfile",
    "NtSuspendThread",
    "NtSystemDebugControl",
    "NtTerminateProcess",
    "NtTerminateThread",
    "NtTestAlert",
    "NtUnloadDriver",
    "NtUnloadKey",
    "NtUnlockFile",
    "NtUnlockVirtualMemory",
    "NtUnmapViewOfSection",
    "NtVdmControl",
    "NtWaitForMultipleObjects",
    "NtWaitForSingleObject",
    "NtWaitHighEventPair",
    "NtWaitLowEventPair",
    "NtWriteFile",
    "NtWriteFileGather",
    "NtWriteRequestData",
    "NtWriteVirtualMemory",
    "NtCreateChannel",
    "NtListenChannel",
    "NtOpenChannel",
    "NtReplyWaitSendChannel",
    "NtSendWaitReplyChannel",
    "NtSetContextChannel",
    "NtYieldExecution",
    NULL
    };

// =================================================================
// WINDOWS 2000 SYSTEM SERVICE NAMES
// =================================================================
// number of entries must match SDT_SYMBOLS_NT5 in w2k_spy.h

PBYTE apbSdtSymbolsNT5 [] =
    {
    "NtAcceptConnectPort",
    "NtAccessCheck",
    "NtAccessCheckAndAuditAlarm",
    "NtAccessCheckByType",
    "NtAccessCheckByTypeAndAuditAlarm",
    "NtAccessCheckByTypeResultList",
    "NtAccessCheckByTypeResultListAndAuditAlarm",
    "NtAccessCheckByTypeResultListAndAuditAlarmByHandle",
    "NtAddAtom",
    "NtAdjustGroupsToken",
    "NtAdjustPrivilegesToken",
    "NtAlertResumeThread",
    "NtAlertThread",
    "NtAllocateLocallyUniqueId",
    "NtAllocateUserPhysicalPages",
    "NtAllocateUuids",
    "NtAllocateVirtualMemory",
    "NtAreMappedFilesTheSame",
    "NtAssignProcessToJobObject",
    "NtCallbackReturn",
    "NtCancelIoFile",
    "NtCancelTimer",
    "NtCancelDeviceWakeupRequest",
    "NtClearEvent",
    "NtClose",
    "NtCloseObjectAuditAlarm",
    "NtCompleteConnectPort",
    "NtConnectPort",
    "NtContinue",
    "NtCreateDirectoryObject",
    "NtCreateEvent",
    "NtCreateEventPair",
    "NtCreateFile",
    "NtCreateIoCompletion",
    "NtCreateJobObject",
    "NtCreateKey",
    "NtCreateMailslotFile",
    "NtCreateMutant",
    "NtCreateNamedPipeFile",
    "NtCreatePagingFile",
    "NtCreatePort",
    "NtCreateProcess",
    "NtCreateProfile",
    "NtCreateSection",
    "NtCreateSemaphore",
    "NtCreateSymbolicLinkObject",
    "NtCreateThread",
    "NtCreateTimer",
    "NtCreateToken",
    "NtCreateWaitablePort",
    "NtDelayExecution",
    "NtDeleteAtom",
    "NtDeleteFile",
    "NtDeleteKey",
    "NtDeleteObjectAuditAlarm",
    "NtDeleteValueKey",
    "NtDeviceIoControlFile",
    "NtDisplayString",
    "NtDuplicateObject",
    "NtDuplicateToken",
    "NtEnumerateKey",
    "NtEnumerateValueKey",
    "NtExtendSection",
    "NtFilterToken",
    "NtFindAtom",
    "NtFlushBuffersFile",
    "NtFlushInstructionCache",
    "NtFlushKey",
    "NtFlushVirtualMemory",
    "NtFlushWriteBuffer",
    "NtFreeUserPhysicalPages",
    "NtFreeVirtualMemory",
    "NtFsControlFile",
    "NtGetContextThread",
    "NtGetDevicePowerState",
    "NtGetPlugPlayEvent",
    "NtGetTickCount",
    "NtGetWriteWatch",
    "NtImpersonateAnonymousToken",
    "NtImpersonateClientOfPort",
    "NtImpersonateThread",
    "NtInitializeRegistry",
    "NtInitiatePowerAction",
    "NtIsSystemResumeAutomatic",
    "NtListenPort",
    "NtLoadDriver",
    "NtLoadKey",
    "NtLoadKey2",
    "NtLockFile",
    "NtLockVirtualMemory",
    "NtMakeTemporaryObject",
    "NtMapUserPhysicalPages",
    "NtMapUserPhysicalPagesScatter",
    "NtMapViewOfSection",
    "NtNotifyChangeDirectoryFile",
    "NtNotifyChangeKey",
    "NtNotifyChangeMultipleKeys",
    "NtOpenDirectoryObject",
    "NtOpenEvent",
    "NtOpenEventPair",
    "NtOpenFile",
    "NtOpenIoCompletion",
    "NtOpenJobObject",
    "NtOpenKey",
    "NtOpenMutant",
    "NtOpenObjectAuditAlarm",
    "NtOpenProcess",
    "NtOpenProcessToken",
    "NtOpenSection",
    "NtOpenSemaphore",
    "NtOpenSymbolicLinkObject",
    "NtOpenThread",
    "NtOpenThreadToken",
    "NtOpenTimer",
    "NtPlugPlayControl",
    "NtPowerInformation",
    "NtPrivilegeCheck",
    "NtPrivilegedServiceAuditAlarm",
    "NtPrivilegeObjectAuditAlarm",
    "NtProtectVirtualMemory",
    "NtPulseEvent",
    "NtQueryInformationAtom",
    "NtQueryAttributesFile",
    "NtQueryDefaultLocale",
    "NtQueryDefaultUILanguage",
    "NtQueryDirectoryFile",
    "NtQueryDirectoryObject",
    "NtQueryEaFile",
    "NtQueryEvent",
    "NtQueryFullAttributesFile",
    "NtQueryInformationFile",
    "NtQueryInformationJobObject",
    "NtQueryIoCompletion",
    "NtQueryInformationPort",
    "NtQueryInformationProcess",
    "NtQueryInformationThread",
    "NtQueryInformationToken",
    "NtQueryInstallUILanguage",
    "NtQueryIntervalProfile",
    "NtQueryKey",
    "NtQueryMultipleValueKey",
    "NtQueryMutant",
    "NtQueryObject",
    "NtQueryOpenSubKeys",
    "NtQueryPerformanceCounter",
    "NtQueryQuotaInformationFile",
    "NtQuerySection",
    "NtQuerySecurityObject",
    "NtQuerySemaphore",
    "NtQuerySymbolicLinkObject",
    "NtQuerySystemEnvironmentValue",
    "NtQuerySystemInformation",
    "NtQuerySystemTime",
    "NtQueryTimer",
    "NtQueryTimerResolution",
    "NtQueryValueKey",
    "NtQueryVirtualMemory",
    "NtQueryVolumeInformationFile",
    "NtQueueApcThread",
    "NtRaiseException",
    "NtRaiseHardError",
    "NtReadFile",
    "NtReadFileScatter",
    "NtReadRequestData",
    "NtReadVirtualMemory",
    "NtRegisterThreadTerminatePort",
    "NtReleaseMutant",
    "NtReleaseSemaphore",
    "NtRemoveIoCompletion",
    "NtReplaceKey",
    "NtReplyPort",
    "NtReplyWaitReceivePort",
    "NtReplyWaitReceivePortEx",
    "NtReplyWaitReplyPort",
    "NtRequestDeviceWakeup",
    "NtRequestPort",
    "NtRequestWaitReplyPort",
    "NtRequestWakeupLatency",
    "NtResetEvent",
    "NtResetWriteWatch",
    "NtRestoreKey",
    "NtResumeThread",
    "NtSaveKey",
    "NtSaveMergedKeys",
    "NtSecureConnectPort",
    "NtSetIoCompletion",
    "NtSetContextThread",
    "NtSetDefaultHardErrorPort",
    "NtSetDefaultLocale",
    "NtSetDefaultUILanguage",
    "NtSetEaFile",
    "NtSetEvent",
    "NtSetHighEventPair",
    "NtSetHighWaitLowEventPair",
    "NtSetInformationFile",
    "NtSetInformationJobObject",
    "NtSetInformationKey",
    "NtSetInformationObject",
    "NtSetInformationProcess",
    "NtSetInformationThread",
    "NtSetInformationToken",
    "NtSetIntervalProfile",
    "NtSetLdtEntries",
    "NtSetLowEventPair",
    "NtSetLowWaitHighEventPair",
    "NtSetQuotaInformationFile",
    "NtSetSecurityObject",
    "NtSetSystemEnvironmentValue",
    "NtSetSystemInformation",
    "NtSetSystemPowerState",
    "NtSetSystemTime",
    "NtSetThreadExecutionState",
    "NtSetTimer",
    "NtSetTimerResolution",
    "NtSetUuidSeed",
    "NtSetValueKey",
    "NtSetVolumeInformationFile",
    "NtShutdownSystem",
    "NtSignalAndWaitForSingleObject",
    "NtStartProfile",
    "NtStopProfile",
    "NtSuspendThread",
    "NtSystemDebugControl",
    "NtTerminateJobObject",
    "NtTerminateProcess",
    "NtTerminateThread",
    "NtTestAlert",
    "NtUnloadDriver",
    "NtUnloadKey",
    "NtUnlockFile",
    "NtUnlockVirtualMemory",
    "NtUnmapViewOfSection",
    "NtVdmControl",
    "NtWaitForMultipleObjects",
    "NtWaitForSingleObject",
    "NtWaitHighEventPair",
    "NtWaitLowEventPair",
    "NtWriteFile",
    "NtWriteFileGather",
    "NtWriteRequestData",
    "NtWriteVirtualMemory",
    "NtCreateChannel",
    "NtListenChannel",
    "NtOpenChannel",
    "NtReplyWaitSendChannel",
    "NtSendWaitReplyChannel",
    "NtSetContextChannel",
    "NtYieldExecution",
    NULL
    };

// =================================================================
// SYSTEM SERVICE HOOK FORMAT STRINGS
// =================================================================
// each string must contain the exact function name

PBYTE apbSdtFormats [] =
    {
    "%s=NtCancelIoFile(%!,%i)",
    "%s=NtClose(%-)",
    "%s=NtCreateFile(%+,%n,%o,%i,%l,%n,%n,%n,%n,%p,%n)",
    "%s=NtCreateKey(%+,%n,%o,%n,%u,%n,%d)",
    "%s=NtDeleteFile(%o)",
    "%s=NtDeleteKey(%-)",
    "%s=NtDeleteValueKey(%!,%u)",
    "%s=NtDeviceIoControlFile(%!,%p,%p,%p,%i,%n,%p,%n,%p,%n)",
    "%s=NtEnumerateKey(%!,%n,%n,%p,%n,%d)",
    "%s=NtEnumerateValueKey(%!,%n,%n,%p,%n,%d)",
    "%s=NtFlushBuffersFile(%!,%i)",
    "%s=NtFlushKey(%!)",
    "%s=NtFsControlFile(%!,%p,%p,%p,%i,%n,%p,%n,%p,%n)",
    "%s=NtLoadKey(%o,%o)",
    "%s=NtLoadKey2(%o,%o,%n)",
    "%s=NtNotifyChangeKey(%!,%p,%p,%p,%i,%n,%b,%p,%n,%b)",
    "%s=NtNotifyChangeMultipleKeys(%!,%n,%o,%p,%p,%p,%i,%n,%b,%p,%n,%b)",
    "%s=NtOpenFile(%+,%n,%o,%i,%n,%n)",
    "%s=NtOpenKey(%+,%n,%o)",
    "%s=NtOpenProcess(%+,%n,%o,%c)",
    "%s=NtOpenThread(%+,%n,%o,%c)",
    "%s=NtQueryDirectoryFile(%!,%p,%p,%p,%i,%p,%n,%n,%b,%u,%b)",
    "%s=NtQueryInformationFile(%!,%i,%p,%n,%n)",
    "%s=NtQueryInformationProcess(%!,%n,%p,%n,%d)",
    "%s=NtQueryInformationThread(%!,%n,%p,%n,%d)",
    "%s=NtQueryKey(%!,%n,%p,%n,%d)",
    "%s=NtQueryMultipleValueKey(%!,%p,%n,%p,%d,%d)",
    "%s=NtQueryOpenSubKeys(%o,%d)",
    "%s=NtQuerySystemInformation(%n,%p,%n,%d)",
    "%s=NtQuerySystemTime(%l)",
    "%s=NtQueryValueKey(%!,%u,%n,%p,%n,%d)",
    "%s=NtQueryVolumeInformationFile(%!,%i,%p,%n,%n)",
    "%s=NtReadFile(%!,%p,%p,%p,%i,%p,%n,%l,%d)",
    "%s=NtReplaceKey(%o,%!,%o)",
    "%s=NtSetInformationKey(%!,%n,%p,%n)",
    "%s=NtSetInformationFile(%!,%i,%p,%n,%n)",
    "%s=NtSetInformationProcess(%!,%n,%p,%n)",
    "%s=NtSetInformationThread(%!,%n,%p,%n)",
    "%s=NtSetSystemInformation(%n,%p,%n)",
    "%s=NtSetSystemTime(%l,%l)",
    "%s=NtSetValueKey(%!,%u,%n,%n,%p,%n)",
    "%s=NtSetVolumeInformationFile(%!,%i,%p,%n,%n)",
    "%s=NtUnloadKey(%o)",
    "%s=NtWriteFile(%!,%p,%p,%p,%i,%p,%n,%l,%d)",
    NULL
    };

// =================================================================
// SYSTEM SERVICE HOOK ENTRIES
// =================================================================

SPY_HOOK_ENTRY aSpyHooks [SDT_SYMBOLS_MAX];

// =================================================================
// STRING FUNCTIONS
// =================================================================

PBYTE strcpyn (PBYTE pbBuffer,
               PBYTE pbData,
               DWORD dBuffer)
    {
    DWORD i;

    if (dBuffer)
        {
        for (i = 0; (i < dBuffer-1) && pbData [i]; i++)
            {
            pbBuffer [i] = pbData [i];
            }
        pbBuffer [i] = 0;
        }
    return pbBuffer;
    }

// -----------------------------------------------------------------

PWORD wcscpyn (PWORD pwBuffer,
               PWORD pwData,
               DWORD dBuffer)
    {
    DWORD i;

    if (dBuffer)
        {
        for (i = 0; (i < dBuffer-1) && pwData [i]; i++)
            {
            pwBuffer [i] = pwData [i];
            }
        pwBuffer [i] = 0;
        }
    return pwBuffer;
    }

// =================================================================
// MEMORY MANAGEMENT
// =================================================================

PVOID SpyMemoryCreate (DWORD dSize)
    {
    return ExAllocatePoolWithTag (PagedPool, max (dSize, 1),
                                  SPY_TAG);
    }

// -----------------------------------------------------------------

PVOID SpyMemoryDestroy (PVOID pData)
    {
    if (pData != NULL) ExFreePool (pData);
    return NULL;
    }

// =================================================================
// SHIFT/AND SEARCH ENGINE
// =================================================================

void SpySearchReset (PSPY_SEARCH pss)
    {
    pss->qTest = 0;
    pss->dNext = 0;
    pss->dHit  = MAXDWORD;
    return;
    }

// -----------------------------------------------------------------

BOOL SpySearchNew (PSPY_SEARCH pss,
                   PBYTE       pbPattern)
    {
    DWORD  i;
    QWORD  qMask;
    PQWORD pqFlags = pss->aqFlags;

    for (i = 0; i < 256; i++) pqFlags [i] = 0;

    for (i = 0, qMask = 1; pbPattern [i] && qMask; i++, qMask <<= 1)
        {
        pqFlags [pbPattern [i]] |= qMask;
        }
    pss->qMask  = (qMask ? qMask >> 1 : 0x8000000000000000);
    pss->dBytes = i;

    SpySearchReset (pss);
    return (i && (!pbPattern [i]));
    }

// -----------------------------------------------------------------

BOOL SpySearchTest (PSPY_SEARCH pss,
                    BYTE        bData)
    {
    BOOL fOk = FALSE;

    if (pss->qMask)
        {
        pss->qTest <<= 1;
        pss->qTest  |= 1;
        pss->qTest  &= pss->aqFlags [bData];
        pss->dNext++;

        if (pss->qTest & pss->qMask)
            {
            pss->qTest = 0;
            pss->dHit  = pss->dNext - pss->dBytes;

            fOk = TRUE;
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL SpySearchText (PSPY_SEARCH pss,
                    PBYTE       pbText)
    {
    DWORD i;
    BOOL  fHit = FALSE;

    SpySearchReset (pss);

    for (i = 0; (!fHit) && pbText [i]; i++)
        {
        fHit = SpySearchTest (pss, pbText [i]);
        }
    return fHit;
    }

// -----------------------------------------------------------------

PBYTE SpySearchFormat (PBYTE  pbSymbol,
                       PPBYTE ppbFormats)
    {
    SPY_SEARCH ss;
    DWORD      i;
    PBYTE      pbFormat = NULL;

    if (SpySearchNew (&ss, pbSymbol))
        {
        for (i = 0; (pbFormat = ppbFormats [i]) != NULL; i++)
            {
            if (SpySearchText (&ss, pbFormat)) break;
            }
        }
    return pbFormat;
    }

// =================================================================
// SELECTORS, DESCRIPTORS, GATES, AND SEGMENTS
// =================================================================

BOOL SpySelector (DWORD         dSegment,
                  DWORD         dSelector,
                  PX86_SELECTOR pSelector)
    {
    X86_SELECTOR Selector = {0, 0};
    BOOL         fOk      = FALSE;

    if (pSelector != NULL)
        {
        fOk = TRUE;

        switch (dSegment)
            {
            case X86_SEGMENT_OTHER:
                {
                if (fOk = ((dSelector >> X86_SELECTOR_SHIFT)
                           <= X86_SELECTOR_LIMIT))
                    {
                    Selector.wValue = (WORD) dSelector;
                    }
                break;
                }
            case X86_SEGMENT_CS:
                {
                __asm mov Selector.wValue, cs
                break;
                }
            case X86_SEGMENT_DS:
                {
                __asm mov Selector.wValue, ds
                break;
                }
            case X86_SEGMENT_ES:
                {
                __asm mov Selector.wValue, es
                break;
                }
            case X86_SEGMENT_FS:
                {
                __asm mov Selector.wValue, fs
                break;
                }
            case X86_SEGMENT_GS:
                {
                __asm mov Selector.wValue, gs
                break;
                }
            case X86_SEGMENT_SS:
                {
                __asm mov Selector.wValue, ss
                break;
                }
            case X86_SEGMENT_TSS:
                {
                __asm str Selector.wValue
                break;
                }
            default:
                {
                fOk = FALSE;
                break;
                }
            }
        RtlCopyMemory (pSelector, &Selector, X86_SELECTOR_);
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID SpyDescriptorBase (PX86_DESCRIPTOR pDescriptor)
    {
    return (PVOID) ((pDescriptor->Base1      ) |
                    (pDescriptor->Base2 << 16) |
                    (pDescriptor->Base3 << 24));
    }

// -----------------------------------------------------------------

DWORD SpyDescriptorLimit (PX86_DESCRIPTOR pDescriptor)
    {
    return (pDescriptor->G ? (pDescriptor->Limit1 << 12) |
                             (pDescriptor->Limit2 << 28) | 0xFFF
                           : (pDescriptor->Limit1      ) |
                             (pDescriptor->Limit2 << 16));
    }

// -----------------------------------------------------------------

DWORD SpyDescriptorType (PX86_DESCRIPTOR pDescriptor,
                         PBOOL           pfSystem)
    {
    if (pfSystem != NULL) *pfSystem = !pDescriptor->S;
    return pDescriptor->Type;
    }

// -----------------------------------------------------------------

BOOL SpyDescriptor (PX86_SELECTOR   pSelector,
                    PX86_DESCRIPTOR pDescriptor)
    {
    X86_SELECTOR    ldt;
    X86_TABLE       gdt;
    DWORD           dType, dLimit;
    BOOL            fSystem;
    PX86_DESCRIPTOR pDescriptors = NULL;
    BOOL            fOk          = FALSE;

    if (pDescriptor != NULL)
        {
        if (pSelector != NULL)
            {
            if (pSelector->TI) // ldt descriptor
                {
                __asm
                    {
                    sldt ldt.wValue
                    sgdt gdt.wLimit
                    }
                if ((!ldt.TI) && ldt.Index &&
                    ((ldt.wValue & X86_SELECTOR_INDEX)
                     <= gdt.wLimit))
                    {
                    dType  = SpyDescriptorType  (gdt.pDescriptors +
                                                 ldt.Index,
                                                 &fSystem);

                    dLimit = SpyDescriptorLimit (gdt.pDescriptors +
                                                 ldt.Index);

                    if (fSystem && (dType == X86_DESCRIPTOR_SYS_LDT)
                        &&
                        ((DWORD) (pSelector->wValue
                                  & X86_SELECTOR_INDEX)
                         <= dLimit))
                        {
                        pDescriptors =
                            SpyDescriptorBase (gdt.pDescriptors +
                                               ldt.Index);
                        }
                    }
                }
            else // gdt descriptor
                {
                if (pSelector->Index)
                    {
                    __asm
                        {
                        sgdt gdt.wLimit
                        }
                    if ((pSelector->wValue & X86_SELECTOR_INDEX)
                        <= gdt.wLimit)
                        {
                        pDescriptors = gdt.pDescriptors;
                        }
                    }
                }
            }
        if (pDescriptors != NULL)
            {
            RtlCopyMemory (pDescriptor,
                           pDescriptors + pSelector->Index,
                           X86_DESCRIPTOR_);
            fOk = TRUE;
            }
        else
            {
            RtlZeroMemory (pDescriptor,
                           X86_DESCRIPTOR_);
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID SpyGateOffset (PX86_GATE pGate)
    {
    return (PVOID) (pGate->Offset1 | (pGate->Offset2 << 16));
    }

// -----------------------------------------------------------------

BOOL SpyIdtGate (PX86_SELECTOR pSelector,
                 PX86_GATE     pGate)
    {
    X86_TABLE idt;
    PX86_GATE pGates = NULL;
    BOOL      fOk    = FALSE;

    if (pGate != NULL)
        {
        if (pSelector != NULL)
            {
            __asm
                {
                sidt idt.wLimit
                }
            if ((pSelector->wValue & X86_SELECTOR_INDEX)
                <= idt.wLimit)
                {
                pGates = idt.pGates;
                }
            }
        if (pGates != NULL)
            {
            RtlCopyMemory (pGate,
                           pGates + pSelector->Index,
                           X86_GATE_);
            fOk = TRUE;
            }
        else
            {
            RtlZeroMemory (pGate, X86_GATE_);
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL SpySegment (DWORD        dSegment,
                 DWORD        dSelector,
                 PSPY_SEGMENT pSegment)
    {
    BOOL fOk = FALSE;

    if (pSegment != NULL)
        {
        fOk = TRUE;
        
        if (!SpySelector   (dSegment, dSelector,
                            &pSegment->Selector))
            {
            fOk = FALSE;
            }
        if (!SpyDescriptor (&pSegment->Selector,
                            &pSegment->Descriptor))
            {
            fOk = FALSE;
            }
        pSegment->pBase  =
            SpyDescriptorBase  (&pSegment->Descriptor);

        pSegment->dLimit =
            SpyDescriptorLimit (&pSegment->Descriptor);

        pSegment->fOk = fOk;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL SpyInterrupt (DWORD          dInterrupt,
                   PSPY_INTERRUPT pInterrupt)
    {
    BOOL fOk = FALSE;

    if (pInterrupt != NULL)
        {
        if (dInterrupt <= X86_SELECTOR_LIMIT)
            {
            fOk = TRUE;

            if (!SpySelector (X86_SEGMENT_OTHER,
                              dInterrupt << X86_SELECTOR_SHIFT,
                              &pInterrupt->Selector))
                {
                fOk = FALSE;
                }
            if (!SpyIdtGate  (&pInterrupt->Selector,
                              &pInterrupt->Gate))
                {
                fOk = FALSE;
                }
            if (!SpySegment  (X86_SEGMENT_OTHER,
                              pInterrupt->Gate.Selector,
                              &pInterrupt->Segment))
                {
                fOk = FALSE;
                }
            pInterrupt->pOffset = SpyGateOffset (&pInterrupt->Gate);
            }
        else
            {
            RtlZeroMemory (pInterrupt, SPY_INTERRUPT_);
            }
        pInterrupt->fOk = fOk;
        }
    return fOk;
    }

// =================================================================
// MEMORY ACCESS FUNCTIONS
// =================================================================

BOOL SpyMemoryPageEntry (PVOID           pVirtual,
                         PSPY_PAGE_ENTRY pspe)
    {
    SPY_PAGE_ENTRY spe;
    BOOL           fOk = FALSE;

    spe.pe       = X86_PDE_ARRAY [X86_PDI (pVirtual)];
    spe.dSize    = X86_PAGE_4M;
    spe.fPresent = FALSE;

    if (spe.pe.pde4M.P)
        {
        if (spe.pe.pde4M.PS)
            {
            fOk = spe.fPresent = TRUE;
            }
        else
            {
            spe.pe    = X86_PTE_ARRAY [X86_PAGE (pVirtual)];
            spe.dSize = X86_PAGE_4K;

            if (spe.pe.pte4K.P)
                {
                fOk = spe.fPresent = TRUE;
                }
            else
                {
                fOk = (spe.pe.pnpe.PageFile != 0);
                }
            }
        }
    if (pspe != NULL) *pspe = spe;
    return fOk;
    }

// -----------------------------------------------------------------

BOOL SpyMemoryTestAddress (PVOID pVirtual)
    {
    return SpyMemoryPageEntry (pVirtual, NULL);
    }

// -----------------------------------------------------------------

BOOL SpyMemoryTestBlock (PVOID pVirtual,
                         DWORD dBytes)
    {
    PBYTE pbData;
    DWORD dData;
    BOOL  fOk = TRUE;

    if (dBytes)
        {
        pbData = (PBYTE) ((DWORD_PTR) pVirtual & X86_PAGE_MASK);
        dData  = (((dBytes + X86_OFFSET_4K (pVirtual) - 1)
                   / PAGE_SIZE) + 1) * PAGE_SIZE;
        do  {
            fOk = SpyMemoryTestAddress (pbData);

            pbData += PAGE_SIZE;
            dData  -= PAGE_SIZE;
            }
        while (fOk && dData);
        }
    return fOk;
    }

// -----------------------------------------------------------------

DWORD SpyMemoryReadBlock (PSPY_MEMORY_BLOCK psmb,
                          PSPY_MEMORY_DATA  psmd,
                          DWORD             dSize)
    {
    DWORD i;
    DWORD n = SPY_MEMORY_DATA__ (psmb->dBytes);

    if (dSize >= n)
        {
        psmd->smb = *psmb;

        for (i = 0; i < psmb->dBytes; i++)
            {
            psmd->awData [i] =
                (SpyMemoryTestAddress (psmb->pbAddress + i)
                 ? SPY_MEMORY_DATA_VALUE (psmb->pbAddress [i], TRUE)
                 : SPY_MEMORY_DATA_VALUE (0, FALSE));
            }
        }
    else
        {
        if (dSize >= SPY_MEMORY_DATA_)
            {
            psmd->smb.pbAddress = NULL;
            psmd->smb.dBytes    = 0;
            }
        n = 0;
        }
    return n;
    }

// =================================================================
// HANDLE MANAGEMENT
// =================================================================

DWORD SpyHandleSlot (PSPY_PROTOCOL psp,
                     HANDLE        hProcess,
                     HANDLE        hObject)
    {
    DWORD dSlot = 0;

    if (hObject != NULL)
        {
        while ((dSlot < psp->sh.dHandles)
               &&
               ((psp->ahProcesses [dSlot] != hProcess) ||
                (psp->ahObjects   [dSlot] != hObject ))) dSlot++;

        dSlot = (dSlot < psp->sh.dHandles ? dSlot+1 : 0);
        }
    return dSlot;
    }

// -----------------------------------------------------------------

DWORD SpyHandleName (PSPY_PROTOCOL psp,
                     HANDLE        hProcess,
                     HANDLE        hObject,
                     PWORD         pwName,
                     DWORD         dName)
    {
    WORD  w;
    DWORD i;
    DWORD dSlot = SpyHandleSlot (psp, hProcess, hObject);

    if ((pwName != NULL) && dName)
        {
        i = 0;

        if (dSlot)
            {
            while ((i+1 < dName) &&
                   (w = psp->awNames [psp->adNames [dSlot-1] + i]))
                {
                pwName [i++] = w;
                }
            }
        pwName [i] = 0;
        }
    return dSlot;
    }

// -----------------------------------------------------------------

DWORD SpyHandleUnregister (PSPY_PROTOCOL psp,
                           HANDLE        hProcess,
                           HANDLE        hObject,
                           PWORD         pwName,
                           DWORD         dName)
    {
    DWORD i, j;
    DWORD dSlot = SpyHandleName (psp, hProcess, hObject,
                                 pwName, dName);
    if (dSlot)
        {
        if (dSlot == psp->sh.dHandles)
            {
            // remove last name entry
            psp->sh.dName = psp->adNames [dSlot-1];
            }
        else
            {
            i = psp->adNames [dSlot-1];
            j = psp->adNames [dSlot  ];

            // shift left all remaining name entries
            while (j < psp->sh.dName)
                {
                psp->awNames [i++] = psp->awNames [j++];
                }
            j -= (psp->sh.dName = i);

            // shift left all remaining handles and name offsets
            for (i = dSlot; i < psp->sh.dHandles; i++)
                {
                psp->ahProcesses [i-1] = psp->ahProcesses [i];
                psp->ahObjects   [i-1] = psp->ahObjects   [i];
                psp->adNames     [i-1] = psp->adNames     [i] - j;
                }
            }
        psp->sh.dHandles--;
        }
    return dSlot;
    }

// -----------------------------------------------------------------

DWORD SpyHandleRegister (PSPY_PROTOCOL   psp,
                         HANDLE          hProcess,
                         HANDLE          hObject,
                         PUNICODE_STRING puName)
    {
    PWORD pwName;
    DWORD dName;
    DWORD i;
    DWORD dSlot = 0;

    if (hObject != NULL)
        {
        // unregister old handle with same value
        SpyHandleUnregister (psp, hProcess, hObject, NULL, 0);

        if (psp->sh.dHandles == SPY_HANDLES)
            {
            // unregister oldest handle if overflow
            SpyHandleUnregister (psp, psp->ahProcesses [0],
                                 psp->ahObjects [0], NULL, 0);
            }
        pwName = ((puName != NULL) && SpyMemoryTestAddress (puName)
                  ? puName->Buffer
                  : NULL);

        dName  = ((pwName != NULL) && SpyMemoryTestAddress (pwName)
                  ? puName->Length / WORD_
                  : 0);

        if (dName + 1 <= SPY_NAME_BUFFER - psp->sh.dName)
            {
            // append object to end of list
            psp->ahProcesses [psp->sh.dHandles] = hProcess;
            psp->ahObjects   [psp->sh.dHandles] = hObject;
            psp->adNames     [psp->sh.dHandles] = psp->sh.dName;

            for (i = 0; i < dName; i++)
                {
                psp->awNames [psp->sh.dName++] = pwName [i];
                }
            psp->awNames [psp->sh.dName++] = 0;

            psp->sh.dHandles++;
            dSlot = psp->sh.dHandles;
            }
        }
    return dSlot;
    }

// =================================================================
// HOOK PROTOCOL MANAGEMENT (READ)
// =================================================================

DWORD SpyReadData (PSPY_PROTOCOL psp,
                   PBYTE         pbData,
                   DWORD         dData)
    {
    DWORD i = psp->sh.dRead;
    DWORD n = 0;

    while ((n < dData) && (i != psp->sh.dWrite))
        {
        pbData [n++] = psp->abData [i++];
        if (i == SPY_DATA_BUFFER) i = 0;
        }
    psp->sh.dRead = i;
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyReadLine (PSPY_PROTOCOL psp,
                   PBYTE         pbData,
                   DWORD         dData)
    {
    BYTE  b = 0;
    DWORD i = psp->sh.dRead;
    DWORD n = 0;

    while ((b != '\n') && (i != psp->sh.dWrite))
        {
        b = psp->abData [i++];
        if (i == SPY_DATA_BUFFER) i = 0;
        if (n < dData) pbData [n++] = b;
        }
    if (b == '\n')
        {
        // remove current line from buffer
        psp->sh.dRead = i;
        }
    else
        {
        // don't return any data until full line available
        n = 0;
        }
    if (n)
        {
        pbData [n-1] = 0;
        }
    else
        {
        if (dData) pbData [0] = 0;
        }
    return n;
    }

// =================================================================
// HOOK PROTOCOL MANAGEMENT (WRITE)
// =================================================================

void SpyWriteReset (PSPY_PROTOCOL psp)
    {
    KeQuerySystemTime (&psp->sh.liStart);

    psp->sh.dRead    = 0;
    psp->sh.dWrite   = 0;
    psp->sh.dCalls   = 0;
    psp->sh.dHandles = 0;
    psp->sh.dName    = 0;
    return;
    }

// -----------------------------------------------------------------

DWORD SpyWriteData (PSPY_PROTOCOL psp,
                    PBYTE         pbData,
                    DWORD         dData)
    {
    BYTE  b;
    DWORD i = psp->sh.dRead;
    DWORD j = psp->sh.dWrite;
    DWORD n = 0;

    while (n < dData)
        {
        psp->abData [j++] = pbData [n++];
        if (j == SPY_DATA_BUFFER) j = 0;

        if (j == i)
            {
            // remove first line from buffer
            do  {
                b = psp->abData [i++];
                if (i == SPY_DATA_BUFFER) i = 0;
                }
            while ((b != '\n') && (i != j));

            // remove half line only if single line
            if ((i == j) &&
                ((i += (SPY_DATA_BUFFER / 2)) >= SPY_DATA_BUFFER))
                {
                i -= SPY_DATA_BUFFER;
                }
            }
        }
    psp->sh.dRead  = i;
    psp->sh.dWrite = j;
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteChar (PSPY_PROTOCOL psp,
                    BYTE          bPrefix,
                    BYTE          bData)
    {
    DWORD n = 0;

    if (bPrefix) n += SpyWriteData (psp, &bPrefix, 1);
    if (bData  ) n += SpyWriteData (psp, &bData,   1);
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteAnsi (PSPY_PROTOCOL psp,
                    BYTE          bPrefix,
                    PBYTE         pbData)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if (pbData != NULL)
        {
        n += SpyWriteData (psp, pbData, strlen (pbData));
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteBoolean (PSPY_PROTOCOL psp,
                       BYTE          bPrefix,
                       BOOLEAN       bData)
    {
    return SpyWriteAnsi (psp, bPrefix, (bData ? "TRUE" : "FALSE"));
    }

// -----------------------------------------------------------------

DWORD SpyWriteNumber (PSPY_PROTOCOL psp,
                      BYTE          bPrefix,
                      DWORD         dData)
    {
    BYTE  abNumber [8+1];
    DWORD x, i;

    abNumber [i = 8] = 0;
    x = dData;

    do  {
        abNumber [--i] = abHex [x & 0xF];
        x >>= 4;
        }
    while (x);

    return SpyWriteAnsi (psp, bPrefix, abNumber+i);
    }

// -----------------------------------------------------------------

DWORD SpyWriteWide (PSPY_PROTOCOL psp,
                    BYTE          bPrefix,
                    PWORD         pwData,
                    DWORD         dData)
    {
    UNICODE_STRING us;
    ANSI_STRING    as;
    WORD           awChar [] = L"?";
    BYTE           abChar [] =  "?";
    DWORD          dData1, i;
    DWORD          n = SpyWriteChar (psp, 0, bPrefix);

    if ((pwData != NULL) && SpyMemoryTestAddress (pwData))
        {
        dData1 = (dData != MAXDWORD ? dData : wcslen (pwData));

        RtlInitUnicodeString (&us, awChar);
        RtlInitAnsiString    (&as, abChar);

        for (i = 0; i < dData1; i++)
            {
            if (pwData [i] < 0x100)
                {
                abChar [0] = (BYTE) pwData [i];
                }
            else
                {
                awChar [0] = pwData [i];

                if (RtlUnicodeStringToAnsiString (&as, &us, FALSE)
                    != STATUS_SUCCESS)
                    {
                    abChar [0] = '?';
                    }
                }
            n += SpyWriteChar (psp, 0, abChar [0]);
            }
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteString (PSPY_PROTOCOL psp,
                      BYTE          bPrefix,
                      PWORD         pwData,
                      DWORD         dData,
                      BYTE          bStart,
                      BYTE          bStop)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if ((pwData != NULL) && SpyMemoryTestAddress (pwData))
        {
        n += SpyWriteChar (psp, 0, bStart);
        n += SpyWriteWide (psp, 0, pwData, dData);
        n += SpyWriteChar (psp, 0, bStop);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteName (PSPY_PROTOCOL psp,
                    BYTE          bPrefix,
                    PWORD         pwData,
                    DWORD         dData)
    {
    return SpyWriteString (psp, bPrefix, pwData, dData, '\"', '\"');
    }

// -----------------------------------------------------------------

DWORD SpyWriteUnicode (PSPY_PROTOCOL   psp,
                       BYTE            bPrefix,
                       PUNICODE_STRING puData)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if ((puData != NULL) && SpyMemoryTestAddress (puData))
        {
        n += SpyWriteName (psp, 0, puData->Buffer,
                                   puData->Length / WORD_);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteObject (PSPY_PROTOCOL      psp,
                      BYTE               bPrefix,
                      POBJECT_ATTRIBUTES poa)
    {
    return SpyWriteUnicode (psp, bPrefix, OBJECT_NAME (poa));
    }

// -----------------------------------------------------------------

DWORD SpyWriteLarge (PSPY_PROTOCOL  psp,
                     BYTE           bPrefix,
                     PLARGE_INTEGER pliData)
    {
    BYTE      abNumber [16+1];
    DWORDLONG x;
    DWORD     i;
    DWORD     n = SpyWriteChar (psp, 0, bPrefix);

    if ((pliData != NULL) && SpyMemoryTestAddress (pliData))
        {
        abNumber [i = 16] = 0;
        x = (pliData->QuadPart);

        do  {
            abNumber [--i] = abHex [x & 0xF];
            x >>= 4;
            }
        while (x);

        n += SpyWriteAnsi (psp, 0, abNumber+i);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteStatus (PSPY_PROTOCOL psp,
                      BYTE          bPrefix,
                      NTSTATUS      NtStatus)
    {
    return SpyWriteNumber (psp, bPrefix, NtStatus);
    }

// -----------------------------------------------------------------

DWORD SpyWriteIoStatus (PSPY_PROTOCOL    psp,
                        BYTE             bPrefix,
                        PIO_STATUS_BLOCK pisb)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if ((pisb != NULL) && SpyMemoryTestAddress (pisb))
        {
        n += SpyWriteNumber (psp, 0, pisb->Status);
        n += SpyWriteChar   (psp, 0, '.');
        n += SpyWriteNumber (psp, 0, pisb->Information);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteClientId (PSPY_PROTOCOL psp,
                        BYTE          bPrefix,
                        PCLIENT_ID    pci)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if ((pci != NULL) && SpyMemoryTestAddress (pci))
        {
        n += SpyWriteNumber (psp, 0, (DWORD) pci->UniqueProcess);
        n += SpyWriteChar   (psp, 0, '.');
        n += SpyWriteNumber (psp, 0, (DWORD) pci->UniqueThread);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteDword (PSPY_PROTOCOL psp,
                     BYTE          bPrefix,
                     PDWORD        pdData)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if ((pdData != NULL) && SpyMemoryTestAddress (pdData))
        {
        n += SpyWriteNumber (psp, 0, *pdData);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWritePointer (PSPY_PROTOCOL psp,
                       BYTE          bPrefix,
                       PVOID         pData)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if (pData != NULL)
        {
        n += SpyWriteNumber (psp, 0, (DWORD) pData);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteHandle (PSPY_PROTOCOL psp,
                      BYTE          bPrefix,
                      HANDLE        hProcess,
                      HANDLE        hObject)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    n += SpyWriteNumber (psp, 0, (DWORD) hProcess);
    n += SpyWriteChar   (psp, 0, '.');
    n += SpyWriteNumber (psp, 0, (DWORD) hObject);
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteNewHandle (PSPY_PROTOCOL psp,
                         BYTE          bPrefix,
                         HANDLE        hProcess,
                         PHANDLE       phObject)
    {
    DWORD n = SpyWriteChar (psp, 0, bPrefix);

    if ((phObject != NULL) && SpyMemoryTestAddress (phObject))
        {
        n += SpyWriteHandle (psp, 0, hProcess, *phObject);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteOpenHandle (PSPY_PROTOCOL psp,
                          BYTE          bPrefix,
                          HANDLE        hProcess,
                          HANDLE        hObject)
    {
    WORD  awName [SPY_NAME];
    DWORD n = SpyWriteHandle (psp, bPrefix, hProcess, hObject);

    if (SpyHandleName (psp, hProcess, hObject,
                       awName, SPY_NAME))
        {
        n += SpyWriteChar (psp, 0, '=');
        n += SpyWriteName (psp, 0, awName, MAXDWORD);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteClosedHandle (PSPY_PROTOCOL psp,
                            BYTE          bPrefix,
                            HANDLE        hProcess,
                            HANDLE        hObject)
    {
    WORD  awName [SPY_NAME];
    DWORD n = SpyWriteHandle (psp, bPrefix, hProcess, hObject);

    if (SpyHandleUnregister (psp, hProcess, hObject,
                             awName, SPY_NAME))
        {
        n += SpyWriteChar (psp, 0, '=');
        n += SpyWriteName (psp, 0, awName, MAXDWORD);
        }
    return n;
    }

// -----------------------------------------------------------------

BOOL SpyWriteFilter (PSPY_PROTOCOL psp,
                     PBYTE         pbFormat,
                     PVOID         pParameters,
                     DWORD         dParameters)
    {
    PHANDLE            phObject = NULL;
    HANDLE             hObject  = NULL;
    POBJECT_ATTRIBUTES poa      = NULL;
    PDWORD             pdNext;
    DWORD              i, j;

    pdNext = pParameters;
    i = j  = 0;

    while (pbFormat [i])
        {
        while (pbFormat [i] && (pbFormat [i] != '%')) i++;

        if (pbFormat [i] && pbFormat [++i])
            {
            j++;

            switch (pbFormat [i++])
                {
                case 'b':
                case 'a':
                case 'w':
                case 'u':
                case 'n':
                case 'l':
                case 's':
                case 'i':
                case 'c':
                case 'd':
                case 'p':
                    {
                    break;
                    }
                case 'o':
                    {
                    if (poa == NULL)
                        {
                        poa = (POBJECT_ATTRIBUTES) *pdNext;
                        }
                    break;
                    }
                case '+':
                    {
                    if (phObject == NULL)
                        {
                        phObject = (PHANDLE) *pdNext;
                        }
                    break;
                    }
                case '!':
                case '-':
                    {
                    if (hObject == NULL)
                        {
                        hObject = (HANDLE) *pdNext;
                        }
                    break;
                    }
                default:
                    {
                    j--;
                    break;
                    }
                }
            pdNext++;
            }
        }
    return // number of arguments ok
           (j == dParameters)
           &&
            // no handles involved
           (((phObject == NULL) && (hObject == NULL))
            ||
            // new handle, successfully registered
            ((phObject != NULL) &&
             SpyHandleRegister (psp, PsGetCurrentProcessId (),
                                *phObject, OBJECT_NAME (poa)))
            ||
            // registered handle
            SpyHandleSlot (psp, PsGetCurrentProcessId (), hObject)
            ||
            // filter disabled
            (!gfSpyHookFilter));
    }

// -----------------------------------------------------------------

DWORD SpyWriteType (PSPY_PROTOCOL psp,
                    BYTE          bEscape,
                    BYTE          bType,
                    PVOID         pData)
    {
    HANDLE hProcess = PsGetCurrentProcessId ();
    DWORD  n        = 0;

    switch (bType)
        {
        case 'b':
            {
            n = SpyWriteBoolean
                    (psp, bType, *(BOOLEAN *) pData);
            break;
            }
        case 'a':
            {
            n = SpyWriteAnsi
                    (psp, bType, *(PBYTE *) pData);
            break;
            }
        case 'w':
            {
            n = SpyWriteWide
                    (psp, bType, *(PWORD *) pData, MAXDWORD);
            break;
            }
        case 'u':
            {
            n = SpyWriteUnicode
                    (psp, bType, *(PUNICODE_STRING *) pData);
            break;
            }
        case 'n':
            {
            n = SpyWriteNumber
                    (psp, bType, *(DWORD *) pData);
            break;
            }
        case 'l':
            {
            n = SpyWriteLarge
                    (psp, bType, *(PLARGE_INTEGER *) pData);
            break;
            }
        case 's':
            {
            n = SpyWriteStatus
                    (psp, bType, *(NTSTATUS *) pData);
            break;
            }
        case 'i':
            {
            n = SpyWriteIoStatus
                    (psp, bType, *(PIO_STATUS_BLOCK *) pData);
            break;
            }
        case 'c':
            {
            n = SpyWriteClientId
                    (psp, bType, *(PCLIENT_ID *) pData);
            break;
            }
        case 'd':
            {
            n = SpyWriteDword
                    (psp, bType, *(PDWORD *) pData);
            break;
            }
        case 'p':
            {
            n = SpyWritePointer
                    (psp, bType, *(PVOID *) pData);
            break;
            }
        case 'o':
            {
            n = SpyWriteObject
                    (psp, bType, *(POBJECT_ATTRIBUTES *) pData);
            break;
            }
        case '+':
            {
            n = SpyWriteNewHandle
                    (psp, bType, hProcess, *(PHANDLE *) pData);
            break;
            }
        case '!':
            {
            n = SpyWriteOpenHandle
                    (psp, bType, hProcess, *(HANDLE *) pData);
            break;
            }
        case '-':
            {
            n = SpyWriteClosedHandle
                    (psp, bType, hProcess, *(HANDLE *) pData);
            break;
            }
        default:
            {
            n = (bEscape == bType
                 ? SpyWriteChar (psp, 0,       bType)
                 : SpyWriteChar (psp, bEscape, bType));
            break;
            }
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyWriteFormat (PSPY_PROTOCOL psp,
                      PBYTE         pbFormat,
                      PVOID         pParameters)
    {
    PBYTE  pbData;
    PDWORD pdData;
    DWORD  i;
    DWORD  n = 0;

    pbData = pbFormat;
    pdData = pParameters;

    while (*pbData)
        {
        for (i = 0; pbData [i] && (pbData [i] != '%'); i++);

        n      += SpyWriteData (psp, pbData, i);
        pbData += i;

        if (*pbData)
            {
            n += SpyWriteType (psp, *pbData, *(pbData+1), pdData++);
            if (*++pbData) ++pbData;
            }
        }
    return n;
    }

// =================================================================
// SERVICE DESCRIPTOR TABLE HOOKS
// =================================================================

NTSTATUS SpyHookWait (void)
    {
    return MUTEX_WAIT (gpDeviceContext->kmProtocol);
    }

// -----------------------------------------------------------------

LONG SpyHookRelease (void)
    {
    return MUTEX_RELEASE (gpDeviceContext->kmProtocol);
    }

// -----------------------------------------------------------------

void SpyHookReset (void)
    {
    SpyHookWait    ();
    SpyWriteReset  (&gpDeviceContext->SpyProtocol);
    SpyHookRelease ();
    return;
    }

// -----------------------------------------------------------------

DWORD SpyHookRead (PBYTE pbData,
                   DWORD dData,
                   BOOL  fLine)
    {
    DWORD n = 0;

    SpyHookWait ();

    n = (fLine ? SpyReadLine : SpyReadData)
            (&gpDeviceContext->SpyProtocol, pbData, dData);

    SpyHookRelease ();
    return n;
    }

// -----------------------------------------------------------------

DWORD SpyHookWrite (PBYTE pbData,
                    DWORD dData)
    {
    DWORD n = 0;

    SpyHookWait ();

    n = SpyWriteData
            (&gpDeviceContext->SpyProtocol, pbData, dData);

    SpyHookRelease ();
    return n;
    }

// -----------------------------------------------------------------
// <#>:<status>=<function>(<arguments>)<time>,<thread>,<handles>

void SpyHookProtocol (PSPY_CALL psc)
    {
    LARGE_INTEGER liTime;
    PSPY_PROTOCOL psp = &gpDeviceContext->SpyProtocol;

    KeQuerySystemTime (&liTime);

    SpyHookWait ();

    if (SpyWriteFilter (psp, psc->pshe->pbFormat,
                             psc->adParameters,
                             psc->dParameters))
        {
        SpyWriteNumber (psp, 0, ++(psp->sh.dCalls));   // <#>:
        SpyWriteChar   (psp, 0, ':');
                                                  // <status>=
        SpyWriteFormat (psp, psc->pshe->pbFormat, //  <function>
                             psc->adParameters);  //   (<arguments>)

        SpyWriteLarge  (psp, 0, &liTime);              // <time>,
        SpyWriteChar   (psp, 0, ',');

        SpyWriteNumber (psp, 0, (DWORD) psc->hThread); // <thread>,
        SpyWriteChar   (psp, 0, ',');

        SpyWriteNumber (psp, 0, psp->sh.dHandles);     // <handles>
        SpyWriteChar   (psp, 0, '\n');
        }
    SpyHookRelease ();
    return;
    }

// -----------------------------------------------------------------

BOOL SpyHookPause (BOOL fPause)
    {
    BOOL fPause1 = (BOOL)
                   InterlockedExchange ((PLONG) &gfSpyHookPause,
                                        ( LONG) fPause);
    if (!fPause) SpyHookReset ();
    return fPause1;
    }

// -----------------------------------------------------------------

BOOL SpyHookFilter (BOOL fFilter)
    {
    return (BOOL) InterlockedExchange ((PLONG) &gfSpyHookFilter,
                                       ( LONG) fFilter);
    }

// -----------------------------------------------------------------
// The SpyHook macro defines a hook entry point in inline assembly
// language. The common entry point SpyHook2 is entered by a call
// instruction, allowing the hook to be identified by its return
// address on the stack. The call is executed through a register to
// remove any degrees of freedom from the encoding of the call.

#define SpyHook                              \
        __asm   push    eax                  \
        __asm   mov     eax, offset SpyHook2 \
        __asm   call    eax

// -----------------------------------------------------------------
// The SpyHookInitializeEx() function initializes the aSpyHooks[]
// array with the hook entry points and format strings. It also
// hosts the hook entry points and the hook dispatcher.

void SpyHookInitializeEx (PPBYTE ppbSymbols,
                          PPBYTE ppbFormats)
    {
    DWORD dHooks1, dHooks2, i, j, n;

    __asm
        {
        jmp     SpyHook9
        ALIGN   8
SpyHook1:       ; start of hook entry point section
        }

// the number of entry points defined in this section
// must be equal to SDT_SYMBOLS_MAX (i.e. 0xF8)

SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //08
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //10
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //18
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //20
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //28
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //30
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //38
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //40
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //48
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //50
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //58
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //60
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //68
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //70
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //78
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //80
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //88
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //90
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //98
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //A0
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //A8
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //B0
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //B8
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //C0
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //C8
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //D0
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //D8
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //E0
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //E8
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //F0
SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook SpyHook //F8

    __asm
        {
SpyHook2:       ; end of hook entry point section
        pop     eax                     ; get stub return address
        pushfd
        push    ebx
        push    ecx
        push    edx
        push    ebp
        push    esi
        push    edi
        sub     eax, offset SpyHook1    ; compute entry point index
        mov     ecx, SDT_SYMBOLS_MAX
        mul     ecx
        mov     ecx, offset SpyHook2
        sub     ecx, offset SpyHook1
        div     ecx
        dec     eax
        mov     ecx, gfSpyHookPause     ; test pause flag
        add     ecx, -1
        sbb     ecx, ecx
        not     ecx
        lea     edx, [aSpyHooks + eax * SIZE SPY_HOOK_ENTRY]
        test    ecx, [edx.pbFormat]     ; format string == NULL?
        jz      SpyHook5
        push    eax
        push    edx
        call    PsGetCurrentThreadId    ; get thread id
        mov     ebx, eax
        pop     edx
        pop     eax
        cmp     ebx, ghSpyHookThread    ; ignore hook installer
        jz      SpyHook5
        mov     edi, gpDeviceContext
        lea     edi, [edi.SpyCalls]     ; get call context array
        mov     esi, SPY_CALLS          ; get number of entries
SpyHook3:
        mov     ecx, 1                  ; set in-use flag
        xchg    ecx, [edi.fInUse]
        jecxz   SpyHook4                ; unused entry found
        add     edi, SIZE SPY_CALL      ; try next entry
        dec     esi
        jnz     SpyHook3
        mov     edi, gpDeviceContext
        inc     [edi.dMisses]           ; count misses
        jmp     SpyHook5                ; array overflow
SpyHook4:
        mov     esi, gpDeviceContext
        inc     [esi.dLevel]            ; set nesting level
        mov     [edi.hThread], ebx      ; save thread id
        mov     [edi.pshe], edx         ; save PSPY_HOOK_ENTRY
        mov     ecx, offset SpyHook6    ; set new return address
        xchg    ecx, [esp+20h]
        mov     [edi.pCaller], ecx      ; save old return address
        mov     ecx, KeServiceDescriptorTable
        mov     ecx, [ecx].ntoskrnl.ArgumentTable
        movzx   ecx, byte ptr [ecx+eax] ; get argument stack size
        shr     ecx, 2
        inc     ecx                     ; add 1 for result slot
        mov     [edi.dParameters], ecx  ; save number of parameters
        lea     edi, [edi.adParameters]
        xor     eax, eax                ; initialize result slot
        stosd
        dec     ecx
        jz      SpyHook5                ; no arguments
        lea     esi, [esp+24h]          ; save argument stack
        rep     movsd
SpyHook5:
        mov     eax, [edx.Handler]      ; get original handler
        pop     edi
        pop     esi
        pop     ebp
        pop     edx
        pop     ecx
        pop     ebx
        popfd
        xchg    eax, [esp]              ; restore eax and...
        ret                             ; ...jump to handler
SpyHook6:
        push    eax
        pushfd
        push    ebx
        push    ecx
        push    edx
        push    ebp
        push    esi
        push    edi
        push    eax
        call    PsGetCurrentThreadId    ; get thread id
        mov     ebx, eax
        pop     eax
        mov     edi, gpDeviceContext
        lea     edi, [edi.SpyCalls]     ; get call context array
        mov     esi, SPY_CALLS          ; get number of entries
SpyHook7:
        cmp     ebx, [edi.hThread]      ; find matching thread id
        jz      SpyHook8
        add     edi, SIZE SPY_CALL      ; try next entry
        dec     esi
        jnz     SpyHook7
        push    ebx                     ; entry not found ?!?
        call    KeBugCheck
SpyHook8:
        push    edi                     ; save SPY_CALL pointer
        mov     [edi.adParameters], eax ; store NTSTATUS
        push    edi
        call    SpyHookProtocol
        pop     edi                     ; restore SPY_CALL pointer
        mov     eax, [edi.pCaller]
        mov     [edi.hThread], 0        ; clear thread id
        mov     esi, gpDeviceContext
        dec     [esi.dLevel]            ; reset nesting level
        dec     [edi.fInUse]            ; clear in-use flag
        pop     edi
        pop     esi
        pop     ebp
        pop     edx
        pop     ecx
        pop     ebx
        popfd
        xchg    eax, [esp]              ; restore eax and...
        ret                             ; ...return to caller
SpyHook9:
        mov     dHooks1, offset SpyHook1
        mov     dHooks2, offset SpyHook2
        }
    n = (dHooks2 - dHooks1) / SDT_SYMBOLS_MAX;

    for (i = j = 0; i < SDT_SYMBOLS_MAX; i++, dHooks1 += n)
        {
        if ((ppbSymbols     != NULL) && (ppbFormats != NULL) &&
            (ppbSymbols [j] != NULL))
            {
            aSpyHooks [i].Handler  = (NTPROC) dHooks1;
            aSpyHooks [i].pbFormat =
                SpySearchFormat (ppbSymbols [j++], ppbFormats);
            }
        else
            {
            aSpyHooks [i].Handler  = NULL;
            aSpyHooks [i].pbFormat = NULL;
            }
        }
    return;
    }

// -----------------------------------------------------------------

BOOL SpyHookInitialize (void)
    {
    BOOL fOk = TRUE;

    switch (KeServiceDescriptorTable->ntoskrnl.ServiceLimit)
        {
        case SDT_SYMBOLS_NT4:
            {
            SpyHookInitializeEx (apbSdtSymbolsNT4, apbSdtFormats);
            break;
            }
        case SDT_SYMBOLS_NT5:
            {
            SpyHookInitializeEx (apbSdtSymbolsNT5, apbSdtFormats);
            break;
            }
        default:
            {
            SpyHookInitializeEx (NULL, NULL);
            fOk = FALSE;
            break;
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

DWORD SpyHookExchange (void)
    {
    PNTPROC ServiceTable;
    BOOL    fPause;
    DWORD   i;
    DWORD   n = 0;

    fPause       = SpyHookPause (TRUE);
    ServiceTable = KeServiceDescriptorTable->ntoskrnl.ServiceTable;

    for (i = 0; i < SDT_SYMBOLS_MAX; i++)
        {
        if (aSpyHooks [i].pbFormat != NULL)
            {
            aSpyHooks [i].Handler = (NTPROC)
                InterlockedExchange ((PLONG) ServiceTable+i,
                                     ( LONG) aSpyHooks [i].Handler);
            n++;
            }
        }
    gfSpyHookState = !gfSpyHookState;
    SpyHookPause (fPause);
    return n;
    }

// -----------------------------------------------------------------

NTSTATUS SpyHookInstall (BOOL   fReset,
                         PDWORD pdCount)
    {
    DWORD    n  = 0;
    NTSTATUS ns = STATUS_INVALID_DEVICE_STATE;

    if (!gfSpyHookState)
        {
        ghSpyHookThread = PsGetCurrentThreadId ();

        n = SpyHookExchange ();
        if (fReset) SpyHookReset ();

        ns = STATUS_SUCCESS;
        }
    *pdCount = n;
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyHookRemove (BOOL   fReset,
                        PDWORD pdCount)
    {
    LARGE_INTEGER liDelay;
    BOOL          fInUse;
    DWORD         i;
    DWORD         n  = 0;
    NTSTATUS      ns = STATUS_INVALID_DEVICE_STATE;

    if (gfSpyHookState)
        {
        n = SpyHookExchange ();
        if (fReset) SpyHookReset ();

        do  {
            for (i = 0; i < SPY_CALLS; i++)
                {
                if (fInUse = gpDeviceContext->SpyCalls [i].fInUse)
                    break;
                }
            liDelay.QuadPart = -1000000;
            KeDelayExecutionThread (KernelMode, FALSE, &liDelay);
            }
        while (fInUse);

        ghSpyHookThread = 0;

        ns = STATUS_SUCCESS;
        }
    *pdCount = n;
    return ns;
    }

// -----------------------------------------------------------------

void SpyHookCleanup (void)
    {
    DWORD dCount;

    SpyHookRemove (FALSE, &dCount);
    return;
    }

// =================================================================
// MODULE INFO MANAGEMENT
// =================================================================

PMODULE_LIST SpyModuleList (PDWORD    pdData,
                            PNTSTATUS pns)
    {
    DWORD        dSize;
    DWORD        dData = 0;
    NTSTATUS     ns    = STATUS_INVALID_PARAMETER;
    PMODULE_LIST pml   = NULL;

    for (dSize = PAGE_SIZE; (pml == NULL) && dSize; dSize <<= 1)
        {
        if ((pml = SpyMemoryCreate (dSize)) == NULL)
            {
            ns = STATUS_NO_MEMORY;
            break;
            }
        ns = ZwQuerySystemInformation (SystemModuleInformation,
                                       pml, dSize, &dData);
        if (ns != STATUS_SUCCESS)
            {
            pml   = SpyMemoryDestroy (pml);
            dData = 0;

            if (ns != STATUS_INFO_LENGTH_MISMATCH) break;
            }
        }
    if (pdData != NULL) *pdData = dData;
    if (pns    != NULL) *pns    = ns;
    return pml;
    }

// -----------------------------------------------------------------

PMODULE_LIST SpyModuleFind (PBYTE     pbModule,
                            PDWORD    pdIndex,
                            PNTSTATUS pns)
    {
    DWORD        i;
    DWORD        dIndex = -1;
    NTSTATUS     ns     = STATUS_INVALID_PARAMETER;
    PMODULE_LIST pml    = NULL;

    if ((pml = SpyModuleList (NULL, &ns)) != NULL)
        {
        for (i = 0; i < pml->dModules; i++)
            {
            if (!_stricmp (pml->aModules [i].abPath +
                           pml->aModules [i].wNameOffset,
                           pbModule))
                {
                dIndex = i;
                break;
                }
            }
        if (dIndex == -1)
            {
            pml = SpyMemoryDestroy (pml);
            ns  = STATUS_NO_SUCH_FILE;
            }
        }
    if (pdIndex != NULL) *pdIndex = dIndex;
    if (pns     != NULL) *pns     = ns;
    return pml;
    }

// -----------------------------------------------------------------

PVOID SpyModuleBase (PBYTE     pbModule,
                     PNTSTATUS pns)
    {
    PMODULE_LIST pml;
    DWORD        dIndex;
    NTSTATUS     ns    = STATUS_INVALID_PARAMETER;
    PVOID        pBase = NULL;

    if ((pml = SpyModuleFind (pbModule, &dIndex, &ns)) != NULL)
        {
        pBase = pml->aModules [dIndex].pBase;
        SpyMemoryDestroy (pml);
        }
    if (pns != NULL) *pns = ns;
    return pBase;
    }

// -----------------------------------------------------------------

PIMAGE_NT_HEADERS SpyModuleHeader (PBYTE     pbModule,
                                   PPVOID    ppBase,
                                   PNTSTATUS pns)
    {
    PVOID             pBase = NULL;
    NTSTATUS          ns    = STATUS_INVALID_PARAMETER;
    PIMAGE_NT_HEADERS pinh  = NULL;

    if (((pBase = SpyModuleBase (pbModule, &ns)) != NULL) &&
        ((pinh  = RtlImageNtHeader (pBase))      == NULL))
        {
        ns = STATUS_INVALID_IMAGE_FORMAT;
        }
    if (ppBase != NULL) *ppBase = pBase;
    if (pns    != NULL) *pns    = ns;
    return pinh;
    }

// -----------------------------------------------------------------

PIMAGE_EXPORT_DIRECTORY SpyModuleExport (PBYTE     pbModule,
                                         PPVOID    ppBase,
                                         PNTSTATUS pns)
    {
    PIMAGE_NT_HEADERS       pinh;
    PIMAGE_DATA_DIRECTORY   pidd;
    PVOID                   pBase = NULL;
    NTSTATUS                ns    = STATUS_INVALID_PARAMETER;
    PIMAGE_EXPORT_DIRECTORY pied  = NULL;

    if ((pinh = SpyModuleHeader (pbModule, &pBase, &ns)) != NULL)
        {
        pidd = pinh->OptionalHeader.DataDirectory
               + IMAGE_DIRECTORY_ENTRY_EXPORT;

        if (pidd->VirtualAddress &&
            (pidd->Size >= IMAGE_EXPORT_DIRECTORY_))
            {
            pied = PTR_ADD (pBase, pidd->VirtualAddress);
            }
        else
            {
            ns = STATUS_DATA_ERROR;
            }
        }
    if (ppBase != NULL) *ppBase = pBase;
    if (pns    != NULL) *pns    = ns;
    return pied;
    }

// -----------------------------------------------------------------

PVOID SpyModuleSymbol (PBYTE     pbModule,
                       PBYTE     pbName,
                       PPVOID    ppBase,
                       PNTSTATUS pns)
    {
    PIMAGE_EXPORT_DIRECTORY pied;
    PDWORD                  pdNames, pdFunctions;
    PWORD                   pwOrdinals;
    DWORD                   i, j;
    PVOID                   pBase    = NULL;
    NTSTATUS                ns       = STATUS_INVALID_PARAMETER;
    PVOID                   pAddress = NULL;

    if ((pied = SpyModuleExport (pbModule, &pBase, &ns)) != NULL)
        {
        pdNames     = PTR_ADD (pBase, pied->AddressOfNames);
        pdFunctions = PTR_ADD (pBase, pied->AddressOfFunctions);
        pwOrdinals  = PTR_ADD (pBase, pied->AddressOfNameOrdinals);

        for (i = 0; i < pied->NumberOfNames; i++)
            {
            j = pwOrdinals [i];

            if (!strcmp (PTR_ADD (pBase, pdNames [i]), pbName))
                {
                if (j < pied->NumberOfFunctions)
                    {
                    pAddress = PTR_ADD (pBase, pdFunctions [j]);
                    }
                break;
                }
            }
        if (pAddress == NULL)
            {
            ns = STATUS_PROCEDURE_NOT_FOUND;
            }
        }
    if (ppBase != NULL) *ppBase = pBase;
    if (pns    != NULL) *pns    = ns;
    return pAddress;
    }

// -----------------------------------------------------------------

PVOID SpyModuleSymbolEx (PBYTE     pbSymbol,
                         PPVOID    ppBase,
                         PNTSTATUS pns)
    {
    DWORD    i;
    BYTE     abModule [MAXIMUM_FILENAME_LENGTH] = "ntoskrnl.exe";
    PBYTE    pbName   = pbSymbol;
    PVOID    pBase    = NULL;
    NTSTATUS ns       = STATUS_INVALID_PARAMETER;
    PVOID    pAddress = NULL;

    for (i = 0; pbSymbol [i] && (pbSymbol [i] != '!'); i++);

    if (pbSymbol [i++])
        {
        if (i <= MAXIMUM_FILENAME_LENGTH)
            {
            strcpyn (abModule, pbSymbol, i);
            pbName = pbSymbol + i;
            }
        else
            {
            pbName = NULL;
            }
        }
    if (pbName != NULL)
        {
        pAddress = SpyModuleSymbol (abModule, pbName, &pBase, &ns);
        }
    if (ppBase != NULL) *ppBase = pBase;
    if (pns    != NULL) *pns    = ns;
    return pAddress;
    }

// =================================================================
// KERNEL CALL INTERFACE
// =================================================================

void SpyCall (PSPY_CALL_INPUT  psci,
              PSPY_CALL_OUTPUT psco)
    {
    PVOID pStack;

    __asm
        {
        pushfd
        pushad
        xor     eax, eax
        mov     ebx, psco               ; get output parameter block
        lea     edi, [ebx.uliResult]    ; get result buffer
        mov     [edi  ], eax            ; clear result buffer (lo)
        mov     [edi+4], eax            ; clear result buffer (hi)
        mov     ebx, psci               ; get input parameter block
        mov     ecx, [ebx.dArgumentBytes]
        cmp     ecx, -9                 ; call or store/copy?
        jb      SpyCall2
        mov     esi, [ebx.pEntryPoint]  ; get entry point
        not     ecx                     ; get number of bytes
        jecxz   SpyCall1                ; 0 -> store entry point
        rep     movsb                   ; copy data from entry point
        jmp     SpyCall5
SpyCall1:
        mov     [edi], esi              ; store entry point
        jmp     SpyCall5
SpyCall2:
        mov     esi, [ebx.pArguments]
        cmp     [ebx.fFastCall], eax    ; __fastcall convention?
        jz      SpyCall3
        cmp     ecx, 4                  ; 1st argument available?
        jb      SpyCall3
        mov     eax, [esi]              ; eax = 1st argument
        add     esi, 4                  ; remove argument from list
        sub     ecx, 4
        cmp     ecx, 4                  ; 2nd argument available?
        jb      SpyCall3
        mov     edx, [esi]              ; edx = 2nd argument
        add     esi, 4                  ; remove argument from list
        sub     ecx, 4
SpyCall3:
        mov     pStack, esp             ; save stack pointer
        jecxz   SpyCall4                ; no (more) arguments
        sub     esp, ecx                ; copy argument stack
        mov     edi, esp
        shr     ecx, 2
        rep     movsd
SpyCall4:
        mov     ecx, eax                ; load 1st __fastcall arg
        call    [ebx.pEntryPoint]       ; call entry point
        mov     esp, pStack             ; restore stack pointer
        mov     ebx, psco               ; get output parameter block
        mov     [ebx.uliResult.LowPart ], eax   ; store result (lo)
        mov     [ebx.uliResult.HighPart], edx   ; store result (hi)
SpyCall5:
        popad
        popfd
        }
    return;
    }

// -----------------------------------------------------------------

NTSTATUS SpyCallEx (PSPY_CALL_INPUT  psci,
                    PSPY_CALL_OUTPUT psco)
    {
    NTSTATUS ns = STATUS_SUCCESS;

    __try
        {
        SpyCall (psci, psco);
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {
        ns = STATUS_ACCESS_VIOLATION;
        }
    return ns;
    }

// =================================================================
// DEVICE INPUT FUNCTIONS
// =================================================================

NTSTATUS SpyInputBinary (PVOID  pData,
                         DWORD  dData,
                         PVOID  pInput,
                         DWORD  dInput)
    {
    NTSTATUS ns = STATUS_INVALID_BUFFER_SIZE;

    if (dData <= dInput)
        {
        RtlCopyMemory (pData, pInput, dData);
        ns = STATUS_SUCCESS;
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyInputDword (PDWORD pdValue,
                        PVOID  pInput,
                        DWORD  dInput)
    {
    return SpyInputBinary (pdValue, DWORD_, pInput, dInput);
    }

// -----------------------------------------------------------------

NTSTATUS SpyInputBool (PBOOL  pfValue,
                       PVOID  pInput,
                       DWORD  dInput)
    {
    return SpyInputBinary (pfValue, BOOL_, pInput, dInput);
    }

// -----------------------------------------------------------------

NTSTATUS SpyInputPointer (PPVOID ppAddress,
                          PVOID  pInput,
                          DWORD  dInput)
    {
    return SpyInputBinary (ppAddress, PVOID_, pInput, dInput);
    }

// -----------------------------------------------------------------

NTSTATUS SpyInputHandle (PHANDLE phObject,
                         PVOID   pInput,
                         DWORD   dInput)
    {
    return SpyInputBinary (phObject, HANDLE_, pInput, dInput);
    }

// -----------------------------------------------------------------

NTSTATUS SpyInputMemory (PSPY_MEMORY_BLOCK psmb,
                         PVOID             pInput,
                         DWORD             dInput)
    {
    return SpyInputBinary (psmb, SPY_MEMORY_BLOCK_, pInput, dInput);
    }

// =================================================================
// DEVICE OUTPUT FUNCTIONS
// =================================================================

NTSTATUS SpyOutputMemory (PSPY_MEMORY_BLOCK psmb,
                          PVOID             pOutput,
                          DWORD             dOutput,
                          PDWORD            pdInfo)
    {
    NTSTATUS ns = STATUS_BUFFER_TOO_SMALL;

    if (*pdInfo = SpyMemoryReadBlock (psmb, pOutput, dOutput))
        {
        ns = STATUS_SUCCESS;
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputBinary (PVOID  pData,
                          DWORD  dData,
                          PVOID  pOutput,
                          DWORD  dOutput,
                          PDWORD pdInfo)
    {
    NTSTATUS ns = STATUS_BUFFER_TOO_SMALL;

    *pdInfo = 0;

    if (dData <= dOutput)
        {
        RtlCopyMemory (pOutput, pData, *pdInfo = dData);
        ns = STATUS_SUCCESS;
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputBlock (PSPY_MEMORY_BLOCK psmb,
                         PVOID             pOutput,
                         DWORD             dOutput,
                         PDWORD            pdInfo)
    {
    NTSTATUS ns = STATUS_INVALID_PARAMETER;

    if (SpyMemoryTestBlock (psmb->pAddress, psmb->dBytes))
        {
        ns = SpyOutputBinary (psmb->pAddress, psmb->dBytes,
                              pOutput, dOutput, pdInfo);
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputDword (DWORD  dValue,
                         PVOID  pOutput,
                         DWORD  dOutput,
                         PDWORD pdInfo)
    {
    return SpyOutputBinary (&dValue, DWORD_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputBool (BOOL   fValue,
                        PVOID  pOutput,
                        DWORD  dOutput,
                        PDWORD pdInfo)
    {
    return SpyOutputBinary (&fValue, BOOL_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputPointer (PVOID  pValue,
                           PVOID  pOutput,
                           DWORD  dOutput,
                           PDWORD pdInfo)
    {
    return SpyOutputBinary (&pValue, PVOID_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputVersionInfo (PVOID  pOutput,
                               DWORD  dOutput,
                               PDWORD pdInfo)
    {
    SPY_VERSION_INFO svi;

    svi.dVersion = SPY_VERSION;

    wcscpyn (svi.awName, USTRING (CSTRING (DRV_NAME)), SPY_NAME);

    return SpyOutputBinary (&svi, SPY_VERSION_INFO_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputOsInfo (PVOID  pOutput,
                          DWORD  dOutput,
                          PDWORD pdInfo)
    {
    SPY_SEGMENT     ss;
    SPY_OS_INFO     soi;
    NT_PRODUCT_TYPE NtProductType;
    PKPCR           pkpcr;

    NtProductType = (SharedUserData->ProductTypeIsValid
                     ? SharedUserData->NtProductType
                     : 0);

    SpySegment (X86_SEGMENT_FS, 0, &ss);
    pkpcr = ss.pBase;

    soi.dPageSize               =  PAGE_SIZE;
    soi.dPageShift              =  PAGE_SHIFT;
    soi.dPtiShift               =  PTI_SHIFT;
    soi.dPdiShift               =  PDI_SHIFT;
    soi.dPageMask               =  X86_PAGE_MASK;
    soi.dPtiMask                =  X86_PTI_MASK;
    soi.dPdiMask                =  X86_PDI_MASK;
    soi.PteArray                =  X86_PTE_ARRAY;
    soi.PdeArray                =  X86_PDE_ARRAY;
    soi.pLowestUserAddress      =  MM_LOWEST_USER_ADDRESS;
    soi.pThreadEnvironmentBlock =  pkpcr->NtTib.Self;
    soi.pHighestUserAddress     = *MmHighestUserAddress;
    soi.pUserProbeAddress       =  (PVOID) *MmUserProbeAddress;
    soi.pSystemRangeStart       = *MmSystemRangeStart;
    soi.pLowestSystemAddress    =  MM_LOWEST_SYSTEM_ADDRESS;
    soi.pSharedUserData         =  SharedUserData;
    soi.pProcessorControlRegion =  pkpcr;
    soi.pProcessorControlBlock  =  pkpcr->Prcb;
    soi.dGlobalFlag             = *NtGlobalFlag;
    soi.dI386MachineType        = *KeI386MachineType;
    soi.dNumberProcessors       = *KeNumberProcessors;
    soi.dProductType            =  NtProductType;
    soi.dBuildNumber            = *NtBuildNumber;
    soi.dNtMajorVersion         =  SharedUserData->NtMajorVersion;
    soi.dNtMinorVersion         =  SharedUserData->NtMinorVersion;

    wcscpyn (soi.awNtSystemRoot, SharedUserData->NtSystemRoot,
             MAX_PATH);

    return SpyOutputBinary (&soi, SPY_OS_INFO_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputSegment (DWORD  dSelector,
                           PVOID  pOutput,
                           DWORD  dOutput,
                           PDWORD pdInfo)
    {
    SPY_SEGMENT ss;

    SpySegment (X86_SEGMENT_OTHER, dSelector, &ss);

    return SpyOutputBinary (&ss, SPY_SEGMENT_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputInterrupt (DWORD  dInterrupt,
                             PVOID  pOutput,
                             DWORD  dOutput,
                             PDWORD pdInfo)
    {
    SPY_INTERRUPT si;

    SpyInterrupt (dInterrupt, &si);

    return SpyOutputBinary (&si, SPY_INTERRUPT_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputCpuInfo (PVOID  pOutput,
                           DWORD  dOutput,
                           PDWORD pdInfo)
    {
    SPY_CPU_INFO  sci;
    PSPY_CPU_INFO psci = &sci;

    __asm
        {
        push    eax
        push    ebx
        mov     ebx, psci

        mov     eax, cr0
        mov     [ebx.cr0], eax

        mov     eax, cr2
        mov     [ebx.cr2], eax

        mov     eax, cr3
        mov     [ebx.cr3], eax

        sidt    [ebx.idt.wLimit]
        mov     [ebx.idt.wReserved], 0

        sgdt    [ebx.gdt.wLimit]
        mov     [ebx.gdt.wReserved], 0

        sldt    [ebx.ldt.wValue]
        mov     [ebx.ldt.wReserved], 0

        pop     ebx
        pop     eax
        }
    SpySegment (X86_SEGMENT_CS,  0, &sci.cs);
    SpySegment (X86_SEGMENT_DS,  0, &sci.ds);
    SpySegment (X86_SEGMENT_ES,  0, &sci.es);
    SpySegment (X86_SEGMENT_FS,  0, &sci.fs);
    SpySegment (X86_SEGMENT_GS,  0, &sci.gs);
    SpySegment (X86_SEGMENT_SS,  0, &sci.ss);
    SpySegment (X86_SEGMENT_TSS, 0, &sci.tss);

    return SpyOutputBinary (&sci, SPY_CPU_INFO_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputHandleInfo (HANDLE hObject,
                              PVOID  pOutput,
                              DWORD  dOutput,
                              PDWORD pdInfo)
    {
    SPY_HANDLE_INFO           shi;
    OBJECT_HANDLE_INFORMATION ohi;
    NTSTATUS                  ns = STATUS_INVALID_PARAMETER;

    if (hObject != NULL)
        {
        ns = ObReferenceObjectByHandle (hObject,
                                        STANDARD_RIGHTS_READ,
                                        NULL, KernelMode,
                                        &shi.pObjectBody, &ohi);
        }
    if (ns == STATUS_SUCCESS)
        {
        shi.dHandleAttributes = ohi.HandleAttributes;

        ns = SpyOutputBinary (&shi, SPY_HANDLE_INFO_,
                              pOutput, dOutput, pdInfo);

        ObDereferenceObject (shi.pObjectBody);
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputHookInfo (PVOID  pOutput,
                            DWORD  dOutput,
                            PDWORD pdInfo)
    {
    SPY_HOOK_INFO shi;
    DWORD         i;

    shi.sh            =  gpDeviceContext->SpyProtocol.sh;
    shi.psc           =  gpDeviceContext->SpyCalls;
    shi.psp           = &gpDeviceContext->SpyProtocol;
    shi.psdt          =  KeServiceDescriptorTable;
    shi.sdt           = *KeServiceDescriptorTable;
    shi.ServiceLimit  =  shi.sdt.ntoskrnl.ServiceLimit;

    RtlCopyMemory (shi.ServiceTable,
                   shi.sdt.ntoskrnl.ServiceTable,
                   shi.ServiceLimit * NTPROC_);

    RtlCopyMemory (shi.ArgumentTable,
                   shi.sdt.ntoskrnl.ArgumentTable,
                   shi.ServiceLimit * BYTE_);

    RtlCopyMemory (shi.SpyHooks,
                   aSpyHooks,
                   shi.ServiceLimit * SPY_HOOK_ENTRY_);

    for (i = shi.ServiceLimit; i < SDT_SYMBOLS_MAX; i++)
        {
        shi.ServiceTable  [i]          = NULL;
        shi.ArgumentTable [i]          = 0;
        shi.SpyHooks      [i].Handler  = NULL;
        shi.SpyHooks      [i].pbFormat = NULL;
        }
    return SpyOutputBinary (&shi, SPY_HOOK_INFO_,
                            pOutput, dOutput, pdInfo);
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputHookRead (BOOL   fLine,
                            PVOID  pOutput,
                            DWORD  dOutput,
                            PDWORD pdInfo)
    {
    *pdInfo = SpyHookRead (pOutput, dOutput, fLine);
    return STATUS_SUCCESS;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputModuleInfo (PBYTE  pbModule,
                              PVOID  pOutput,
                              DWORD  dOutput,
                              PDWORD pdInfo)
    {
    SPY_MODULE_INFO smi;
    PMODULE_LIST    pml;
    PMODULE_INFO    pmi;
    DWORD           dIndex;
    NTSTATUS        ns = STATUS_INVALID_PARAMETER;

    if ((pbModule != NULL) && SpyMemoryTestAddress (pbModule) &&
        ((pml = SpyModuleFind (pbModule, &dIndex, &ns)) != NULL))
        {
        pmi = pml->aModules + dIndex;

        smi.pBase       = pmi->pBase;
        smi.dSize       = pmi->dSize;
        smi.dFlags      = pmi->dFlags;
        smi.dIndex      = pmi->wIndex;
        smi.dLoadCount  = pmi->wLoadCount;
        smi.dNameOffset = pmi->wNameOffset;

        strcpyn (smi.abPath, pmi->abPath, MAXIMUM_FILENAME_LENGTH);

        ns = SpyOutputBinary (&smi, SPY_MODULE_INFO_,
                              pOutput, dOutput, pdInfo);

        SpyMemoryDestroy (pml);
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputPeHeader (PVOID  pBase,
                            PVOID  pOutput,
                            DWORD  dOutput,
                            PDWORD pdInfo)
    {
    PIMAGE_NT_HEADERS pinh;
    NTSTATUS          ns = STATUS_INVALID_PARAMETER;

    if ((pBase != NULL) && SpyMemoryTestAddress (pBase) &&
        ((pinh = RtlImageNtHeader (pBase)) != NULL))
        {
        ns = SpyOutputBinary (pinh, IMAGE_NT_HEADERS_,
                              pOutput, dOutput, pdInfo);
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputPeExport (PVOID  pBase,
                            PVOID  pOutput,
                            DWORD  dOutput,
                            PDWORD pdInfo)
    {
    PIMAGE_NT_HEADERS       pinh;
    PIMAGE_DATA_DIRECTORY   pidd;
    PIMAGE_EXPORT_DIRECTORY pied;
    PVOID                   pData;
    DWORD                   dData, dBias, i;
    PDWORD                  pdData;
    NTSTATUS                ns = STATUS_INVALID_PARAMETER;

    if ((pBase != NULL) && SpyMemoryTestAddress (pBase) &&
        ((pinh = RtlImageNtHeader (pBase)) != NULL))
        {
        pidd = pinh->OptionalHeader.DataDirectory
               + IMAGE_DIRECTORY_ENTRY_EXPORT;

        if (pidd->VirtualAddress &&
            (pidd->Size >= IMAGE_EXPORT_DIRECTORY_))
            {
            pData = (PBYTE) pBase + pidd->VirtualAddress;
            dData = pidd->Size;

            if ((ns = SpyOutputBinary (pData, dData,
                                       pOutput, dOutput, pdInfo))
                == STATUS_SUCCESS)
                {
                pied  = pOutput;
                dBias = pidd->VirtualAddress;

                pied->Name                  -= dBias;
                pied->AddressOfFunctions    -= dBias;
                pied->AddressOfNames        -= dBias;
                pied->AddressOfNameOrdinals -= dBias;

                pdData = PTR_ADD (pied, pied->AddressOfFunctions);

                for (i = 0; i < pied->NumberOfFunctions; i++)
                    {
                    pdData [i] += (DWORD) pBase;
                    }
                pdData = PTR_ADD (pied, pied->AddressOfNames);

                for (i = 0; i < pied->NumberOfNames; i++)
                    {
                    pdData [i] -= dBias;
                    }
                }
            }
        else
            {
            ns = STATUS_DATA_ERROR;
            }
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputPeSymbol (PBYTE  pbSymbol,
                            PVOID  pOutput,
                            DWORD  dOutput,
                            PDWORD pdInfo)
    {
    PVOID    pAddress;
    NTSTATUS ns = STATUS_INVALID_PARAMETER;

    if ((pbSymbol != NULL) && SpyMemoryTestAddress (pbSymbol)
        &&
        ((pAddress = SpyModuleSymbolEx (pbSymbol, NULL, &ns))
         != NULL))
        {
        ns = SpyOutputPointer (pAddress,
                               pOutput, dOutput, pdInfo);
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS SpyOutputCall (PSPY_CALL_INPUT psci,
                        PVOID           pOutput,
                        DWORD           dOutput,
                        PDWORD          pdInfo)
    {
    SPY_CALL_OUTPUT sco;
    NTSTATUS        ns = STATUS_INVALID_PARAMETER;

    if (psci->pbSymbol != NULL)
        {
        psci->pEntryPoint =
            (SpyMemoryTestAddress (psci->pbSymbol)
             ? SpyModuleSymbolEx  (psci->pbSymbol, NULL, &ns)
             : NULL);
        }
    if ((psci->pEntryPoint != NULL)              &&
        SpyMemoryTestAddress (psci->pEntryPoint) &&
        ((ns = SpyCallEx (psci, &sco)) == STATUS_SUCCESS))
        {
        ns = SpyOutputBinary (&sco, SPY_CALL_OUTPUT_,
                              pOutput, dOutput, pdInfo);
        }
    return ns;
    }

// =================================================================
// DEVICE I/O CONTROL HANDLER
// =================================================================

NTSTATUS SpyDispatcher (PDEVICE_CONTEXT pDeviceContext,
                        DWORD           dCode,
                        PVOID           pInput,
                        DWORD           dInput,
                        PVOID           pOutput,
                        DWORD           dOutput,
                        PDWORD          pdInfo)
    {
    SPY_MEMORY_BLOCK smb;
    SPY_PAGE_ENTRY   spe;
    SPY_CALL_INPUT   sci;
    PHYSICAL_ADDRESS pa;
    DWORD            dValue, dCount;
    BOOL             fReset, fPause, fFilter, fLine;
    PVOID            pAddress;
    PBYTE            pbName;
    HANDLE           hObject;
    NTSTATUS         ns = STATUS_INVALID_PARAMETER;

    MUTEX_WAIT (pDeviceContext->kmDispatch);

    *pdInfo = 0;

    switch (dCode)
        {
        case SPY_IO_VERSION_INFO:
            {
            ns = SpyOutputVersionInfo (pOutput, dOutput, pdInfo);
            break;
            }
        case SPY_IO_OS_INFO:
            {
            ns = SpyOutputOsInfo (pOutput, dOutput, pdInfo);
            break;
            }
        case SPY_IO_SEGMENT:
            {
            if ((ns = SpyInputDword (&dValue,
                                     pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputSegment (dValue,
                                       pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_INTERRUPT:
            {
            if ((ns = SpyInputDword (&dValue,
                                     pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputInterrupt (dValue,
                                         pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_PHYSICAL:
            {
            if ((ns = SpyInputPointer (&pAddress,
                                       pInput, dInput))
                == STATUS_SUCCESS)
                {
                pa = MmGetPhysicalAddress (pAddress);

                ns = SpyOutputBinary (&pa, PHYSICAL_ADDRESS_,
                                      pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_CPU_INFO:
            {
            ns = SpyOutputCpuInfo (pOutput, dOutput, pdInfo);
            break;
            }
        case SPY_IO_PDE_ARRAY:
            {
            ns = SpyOutputBinary (X86_PDE_ARRAY, SPY_PDE_ARRAY_,
                                  pOutput, dOutput, pdInfo);
            break;
            }
        case SPY_IO_PAGE_ENTRY:
            {
            if ((ns = SpyInputPointer (&pAddress,
                                       pInput, dInput))
                == STATUS_SUCCESS)
                {
                SpyMemoryPageEntry (pAddress, &spe);

                ns = SpyOutputBinary (&spe, SPY_PAGE_ENTRY_,
                                      pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_MEMORY_DATA:
            {
            if ((ns = SpyInputMemory (&smb,
                                      pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputMemory (&smb,
                                      pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_MEMORY_BLOCK:
            {
            if ((ns = SpyInputMemory (&smb,
                                      pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputBlock (&smb,
                                     pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HANDLE_INFO:
            {
            if ((ns = SpyInputHandle (&hObject,
                                      pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputHandleInfo (hObject,
                                          pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HOOK_INFO:
            {
            ns = SpyOutputHookInfo (pOutput, dOutput, pdInfo);
            break;
            }
        case SPY_IO_HOOK_INSTALL:
            {
            if (((ns = SpyInputBool (&fReset,
                                     pInput, dInput))
                 == STATUS_SUCCESS)
                &&
                ((ns = SpyHookInstall (fReset, &dCount))
                 == STATUS_SUCCESS))
                {
                ns = SpyOutputDword (dCount,
                                     pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HOOK_REMOVE:
            {
            if (((ns = SpyInputBool (&fReset,
                                     pInput, dInput))
                 == STATUS_SUCCESS)
                &&
                ((ns = SpyHookRemove (fReset, &dCount))
                 == STATUS_SUCCESS))
                {
                ns = SpyOutputDword (dCount,
                                     pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HOOK_PAUSE:
            {
            if ((ns = SpyInputBool (&fPause,
                                    pInput, dInput))
                == STATUS_SUCCESS)
                {
                fPause = SpyHookPause (fPause);

                ns = SpyOutputBool (fPause,
                                    pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HOOK_FILTER:
            {
            if ((ns = SpyInputBool (&fFilter,
                                    pInput, dInput))
                == STATUS_SUCCESS)
                {
                fFilter = SpyHookFilter (fFilter);

                ns = SpyOutputBool (fFilter,
                                    pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HOOK_RESET:
            {
            SpyHookReset ();
            ns = STATUS_SUCCESS;
            break;
            }
        case SPY_IO_HOOK_READ:
            {
            if ((ns = SpyInputBool (&fLine,
                                    pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputHookRead (fLine,
                                        pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_HOOK_WRITE:
            {
            SpyHookWrite (pInput, dInput);
            ns = STATUS_SUCCESS;
            break;
            }
        case SPY_IO_MODULE_INFO:
            {
            if ((ns = SpyInputPointer (&pbName,
                                       pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputModuleInfo (pbName,
                                          pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_PE_HEADER:
            {
            if ((ns = SpyInputPointer (&pAddress,
                                       pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputPeHeader (pAddress,
                                        pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_PE_EXPORT:
            {
            if ((ns = SpyInputPointer (&pAddress,
                                       pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputPeExport (pAddress,
                                        pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_PE_SYMBOL:
            {
            if ((ns = SpyInputPointer (&pbName,
                                       pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputPeSymbol (pbName,
                                        pOutput, dOutput, pdInfo);
                }
            break;
            }
        case SPY_IO_CALL:
            {
            if ((ns = SpyInputBinary (&sci, SPY_CALL_INPUT_,
                                      pInput, dInput))
                == STATUS_SUCCESS)
                {
                ns = SpyOutputCall (&sci,
                                    pOutput, dOutput, pdInfo);
                }
            break;
            }
        }
    MUTEX_RELEASE (pDeviceContext->kmDispatch);
    return ns;
    }

// =================================================================
// DEVICE REQUEST HANDLER
// =================================================================

NTSTATUS DeviceDispatcher (PDEVICE_CONTEXT pDeviceContext,
                           PIRP            pIrp)
    {
    PIO_STACK_LOCATION pisl;
    DWORD              dInfo = 0;
    NTSTATUS           ns    = STATUS_NOT_IMPLEMENTED;

    pisl = IoGetCurrentIrpStackLocation (pIrp);

    switch (pisl->MajorFunction)
        {
        case IRP_MJ_CREATE:
        case IRP_MJ_CLEANUP:
        case IRP_MJ_CLOSE:
            {
            ns = STATUS_SUCCESS;
            break;
            }
        case IRP_MJ_DEVICE_CONTROL:
            {
            ns = SpyDispatcher (pDeviceContext,

                                pisl->Parameters.DeviceIoControl
                                                .IoControlCode,

                                pIrp->AssociatedIrp.SystemBuffer,
                                pisl->Parameters.DeviceIoControl
                                                .InputBufferLength,

                                pIrp->AssociatedIrp.SystemBuffer,
                                pisl->Parameters.DeviceIoControl
                                                .OutputBufferLength,
                                &dInfo);
            break;
            }
        }
    pIrp->IoStatus.Status      = ns;
    pIrp->IoStatus.Information = dInfo;

    IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    return ns;
    }

// =================================================================
// DRIVER REQUEST HANDLER
// =================================================================

NTSTATUS DriverDispatcher (PDEVICE_OBJECT pDeviceObject,
                           PIRP           pIrp)
    {
    return (pDeviceObject == gpDeviceObject
            ? DeviceDispatcher (gpDeviceContext, pIrp)
            : STATUS_INVALID_PARAMETER_1);
    }

// -----------------------------------------------------------------

void DriverUnload (PDRIVER_OBJECT pDriverObject)
    {
    SpyHookCleanup ();

    IoDeleteSymbolicLink (&usSymbolicLinkName);
    IoDeleteDevice       (gpDeviceObject);
    return;
    }

// =================================================================
// DRIVER INITIALIZATION
// =================================================================

NTSTATUS DriverInitialize (PDRIVER_OBJECT  pDriverObject,
                           PUNICODE_STRING pusRegistryPath)
    {
    DWORD          i;
    PDEVICE_OBJECT pDeviceObject = NULL;
    NTSTATUS       ns = STATUS_DEVICE_CONFIGURATION_ERROR;

    if ((ns = IoCreateDevice (pDriverObject, DEVICE_CONTEXT_,
                              &usDeviceName, FILE_DEVICE_SPY,
                              0, FALSE, &pDeviceObject))
        == STATUS_SUCCESS)
        {
        if ((ns = IoCreateSymbolicLink (&usSymbolicLinkName,
                                        &usDeviceName))
            == STATUS_SUCCESS)
            {
            gpDeviceObject  = pDeviceObject;
            gpDeviceContext = pDeviceObject->DeviceExtension;

            gpDeviceContext->pDriverObject = pDriverObject;
            gpDeviceContext->pDeviceObject = pDeviceObject;

            MUTEX_INITIALIZE (gpDeviceContext->kmDispatch);
            MUTEX_INITIALIZE (gpDeviceContext->kmProtocol);

            gpDeviceContext->dMisses = 0;

            for (i = 0; i < SPY_CALLS; i++)
                {
                gpDeviceContext->SpyCalls [i].fInUse  = FALSE;
                gpDeviceContext->SpyCalls [i].hThread = 0;
                }
            SpyWriteReset (&gpDeviceContext->SpyProtocol);
            }
        else
            {
            IoDeleteDevice (pDeviceObject);
            }
        }
    return ns;
    }

// -----------------------------------------------------------------

NTSTATUS DriverEntry (PDRIVER_OBJECT  pDriverObject,
                      PUNICODE_STRING pusRegistryPath)
    {
    PDRIVER_DISPATCH *ppdd;
    NTSTATUS          ns = STATUS_DEVICE_CONFIGURATION_ERROR;

    if ((ns = DriverInitialize (pDriverObject, pusRegistryPath))
        == STATUS_SUCCESS)
        {
        ppdd = pDriverObject->MajorFunction;

        ppdd [IRP_MJ_CREATE                  ] =
        ppdd [IRP_MJ_CREATE_NAMED_PIPE       ] =
        ppdd [IRP_MJ_CLOSE                   ] =
        ppdd [IRP_MJ_READ                    ] =
        ppdd [IRP_MJ_WRITE                   ] =
        ppdd [IRP_MJ_QUERY_INFORMATION       ] =
        ppdd [IRP_MJ_SET_INFORMATION         ] =
        ppdd [IRP_MJ_QUERY_EA                ] =
        ppdd [IRP_MJ_SET_EA                  ] =
        ppdd [IRP_MJ_FLUSH_BUFFERS           ] =
        ppdd [IRP_MJ_QUERY_VOLUME_INFORMATION] =
        ppdd [IRP_MJ_SET_VOLUME_INFORMATION  ] =
        ppdd [IRP_MJ_DIRECTORY_CONTROL       ] =
        ppdd [IRP_MJ_FILE_SYSTEM_CONTROL     ] =
        ppdd [IRP_MJ_DEVICE_CONTROL          ] =
        ppdd [IRP_MJ_INTERNAL_DEVICE_CONTROL ] =
        ppdd [IRP_MJ_SHUTDOWN                ] =
        ppdd [IRP_MJ_LOCK_CONTROL            ] =
        ppdd [IRP_MJ_CLEANUP                 ] =
        ppdd [IRP_MJ_CREATE_MAILSLOT         ] =
        ppdd [IRP_MJ_QUERY_SECURITY          ] =
        ppdd [IRP_MJ_SET_SECURITY            ] =
        ppdd [IRP_MJ_POWER                   ] =
        ppdd [IRP_MJ_SYSTEM_CONTROL          ] =
        ppdd [IRP_MJ_DEVICE_CHANGE           ] =
        ppdd [IRP_MJ_QUERY_QUOTA             ] =
        ppdd [IRP_MJ_SET_QUOTA               ] =
        ppdd [IRP_MJ_PNP                     ] = DriverDispatcher;
        pDriverObject->DriverUnload            = DriverUnload;

        SpyHookInitialize ();
        }
    return ns;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
