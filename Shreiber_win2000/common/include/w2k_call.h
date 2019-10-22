
// w2k_call.h
// 08-27-2000 Sven B. Schreiber
// sbs@orgon.com

#ifndef UNICODE
#define UNICODE     // ANSI not supported by this library
#endif

#ifndef _USER_MODE_
#define _USER_MODE_ // ntdef.h and ntddk.h not available
#endif

////////////////////////////////////////////////////////////////////
#ifdef _W2K_CALL_DLL_
////////////////////////////////////////////////////////////////////

// =================================================================
// PROGRAM IDENTIFICATION
// =================================================================

#define MAIN_BUILD              1
#define MAIN_VERSION_HIGH       1
#define MAIN_VERSION_LOW        0

// -----------------------------------------------------------------

#define MAIN_DAY                27
#define MAIN_MONTH              08
#define MAIN_YEAR               2000

// -----------------------------------------------------------------

#define MAIN_PREFIX             SBS
#define MAIN_MODULE             w2k_call
#define MAIN_NAME               SBS Windows 2000 Call Interface
#define MAIN_COMPANY            Sven B. Schreiber
#define MAIN_AUTHOR             Sven B. Schreiber
#define MAIN_EMAIL              sbs@orgon.com
#define MAIN_DLL

////////////////////////////////////////////////////////////////////
#endif // #ifdef _W2K_CALL_DLL_
////////////////////////////////////////////////////////////////////

// =================================================================
// HEADER FILES
// =================================================================

#include <proginfo.h>

////////////////////////////////////////////////////////////////////
#ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// MORE HEADER FILES
// =================================================================

#include <w2k_lib.h>
#include <w2k_img.h>
#include <w2k_spy.h>

// =================================================================
// CONSTANTS
// =================================================================

#define W2K_SYMBOLS_OK              0
#define W2K_SYMBOLS_UNDEFINED       1
#define W2K_SYMBOLS_MODULE_ERROR    2
#define W2K_SYMBOLS_LOAD_ERROR      3
#define W2K_SYMBOLS_VERSION_ERROR   4

// =================================================================
// TYPE SUBSTITUTES
// =================================================================

typedef PVOID PACCESS_STATE, *PPACCESS_STATE;

// =================================================================
// STRUCTURES
// =================================================================

typedef struct _W2K_CALL_INFO
    {
    HANDLE hDevice;
    PWORD  pwDevice;
    WORD   awDriver [MAX_PATH];
    }
    W2K_CALL_INFO, *PW2K_CALL_INFO, **PPW2K_CALL_INFO;

#define W2K_CALL_INFO_ sizeof (W2K_CALL_INFO)

// -----------------------------------------------------------------

typedef struct _W2K_OBJECT
    {
    POBJECT               pObject;
    POBJECT_HEADER        pHeader;
    POBJECT_CREATOR_INFO  pCreatorInfo;
    POBJECT_NAME          pName;
    POBJECT_HANDLE_DB     pHandleDB;
    POBJECT_QUOTA_CHARGES pQuotaCharges;
    POBJECT_TYPE          pType;
    PQUOTA_BLOCK          pQuotaBlock;
    POBJECT_CREATE_INFO   pCreateInfo;
    PWORD                 pwName;
    PWORD                 pwType;
    }
    W2K_OBJECT, *PW2K_OBJECT, **PPW2K_OBJECT;

#define W2K_OBJECT_ sizeof (W2K_OBJECT)

// -----------------------------------------------------------------

typedef struct _W2K_OBJECT_FRAME
    {
    OBJECT_QUOTA_CHARGES QuotaCharges;
    OBJECT_HANDLE_DB     HandleDB;
    OBJECT_NAME          Name;
    OBJECT_CREATOR_INFO  CreatorInfo;
    OBJECT_HEADER        Header;
    W2K_OBJECT           Object;
    OBJECT_TYPE          Type;
    QUOTA_BLOCK          QuotaBlock;
    OBJECT_CREATE_INFO   CreateInfo;
    WORD                 Buffer [];
    }
    W2K_OBJECT_FRAME, *PW2K_OBJECT_FRAME, **PPW2K_OBJECT_FRAME;

#define W2K_OBJECT_FRAME_ sizeof (W2K_OBJECT_FRAME)
#define W2K_OBJECT_FRAME__(_n) (W2K_OBJECT_FRAME_ + ((_n) * WORD_))

// =================================================================
// CALLBACKS
// =================================================================

typedef BOOL (CALLBACK *W2K_PE_ENUMERATE) (PBYTE pbModule,
                                           PVOID pBase,
                                           PBYTE pbFunction,
                                           PVOID pAddress,
                                           DWORD dOrdinal,
                                           DWORD dIndex,
                                           DWORD dCount);

// =================================================================
// API PROTOTYPES
// =================================================================

PSPY_MODULE_INFO WINAPI w2kPeInfo (PBYTE pbModule);

PVOID WINAPI w2kPeBase (PBYTE pbModule);

PIMAGE_NT_HEADERS WINAPI w2kPeHeader (PBYTE  pbModule,
                                      PPVOID ppBase);

PIMAGE_EXPORT_DIRECTORY WINAPI w2kPeExport (PBYTE  pbModule,
                                            PPVOID ppBase);

BOOL WINAPI w2kPeCheck (PBYTE pbModule,
                        DWORD dTimeStamp,
                        DWORD dCheckSum);

DWORD WINAPI w2kPeEnumerate (PBYTE            pbModule,
                             PPVOID           ppBase,
                             W2K_PE_ENUMERATE Callback);

PVOID WINAPI w2kPeSymbol (PBYTE pbSymbol);

PIMG_TABLE WINAPI w2kSymbolsLoad (PBYTE  pbModule,
                                  PDWORD pdStatus);

PIMG_TABLE WINAPI w2kSymbolsGlobal (PDWORD pdStatus);

DWORD WINAPI w2kSymbolsStatus (VOID);

VOID WINAPI w2kSymbolsReset (VOID);

BOOL WINAPI w2kSpyLock (VOID);

BOOL WINAPI w2kSpyUnlock (VOID);

BOOL WINAPI w2kSpyStartup (BOOL      fUnload,
                           HINSTANCE hInstance);

BOOL WINAPI w2kSpyCleanup (BOOL fUnload);

BOOL WINAPI w2kSpyControl (DWORD dCode,
                           PVOID pInput,
                           DWORD dInput,
                           PVOID pOutput,
                           DWORD dOutput);

BOOL WINAPI w2kSpyRead (PVOID pBuffer,
                        PVOID pAddress,
                        DWORD dBytes);

PVOID WINAPI w2kSpyClone (PVOID pAddress,
                          DWORD dBytes);

PANSI_STRING WINAPI w2kStringAnsi (DWORD dSize);

PUNICODE_STRING WINAPI w2kStringUnicode (DWORD dSize);

PUNICODE_STRING WINAPI w2kStringClone (PUNICODE_STRING pusSource);

PWORD WINAPI w2kStringCopy (PUNICODE_STRING pusSource,
                            PUNICODE_STRING pusTarget,
                            PWORD           pwBuffer);

BOOL WINAPI w2kCallInfo (PW2K_CALL_INFO pwci);

BOOL WINAPI w2kCallExecute (PSPY_CALL_INPUT  psci,
                            PSPY_CALL_OUTPUT psco);

BOOL WINAPI w2kCall (PULARGE_INTEGER puliResult,
                     PBYTE           pbSymbol,
                     PVOID           pEntryPoint,
                     BOOL            fFastCall,
                     DWORD           dArgumentBytes,
                     PVOID           pArguments);

BOOL WINAPI w2kCallV (PULARGE_INTEGER puliResult,
                      PBYTE           pbSymbol,
                      BOOL            fFastCall,
                      DWORD           dArgumentBytes,
                      ...);

NTSTATUS WINAPI w2kCallNT (PBYTE pbSymbol,
                           DWORD dArgumentBytes,
                           ...);

BYTE WINAPI w2kCall08 (BYTE  bDefault,
                       PBYTE pbSymbol,
                       BOOL  fFastCall,
                       DWORD dArgumentBytes,
                       ...);

WORD WINAPI w2kCall16 (WORD  wDefault,
                       PBYTE pbSymbol,
                       BOOL  fFastCall,
                       DWORD dArgumentBytes,
                       ...);

DWORD WINAPI w2kCall32 (DWORD dDefault,
                        PBYTE pbSymbol,
                        BOOL  fFastCall,
                        DWORD dArgumentBytes,
                        ...);

QWORD WINAPI w2kCall64 (QWORD qDefault,
                        PBYTE pbSymbol,
                        BOOL  fFastCall,
                        DWORD dArgumentBytes,
                        ...);

PVOID WINAPI w2kCallP (PVOID pDefault,
                       PBYTE pbSymbol,
                       BOOL  fFastCall,
                       DWORD dArgumentBytes,
                       ...);

BOOL WINAPI w2kCopy (PULARGE_INTEGER puliResult,
                     PBYTE           pbSymbol,
                     PVOID           pEntryPoint,
                     DWORD           dBytes);

BYTE WINAPI w2kCopy08 (BYTE  bDefault,
                       PBYTE pbSymbol);

WORD WINAPI w2kCopy16 (WORD  wDefault,
                       PBYTE pbSymbol);

DWORD WINAPI w2kCopy32 (DWORD dDefault,
                        PBYTE pbSymbol);

QWORD WINAPI w2kCopy64 (QWORD qDefault,
                        PBYTE pbSymbol);

PVOID WINAPI w2kCopyP (PVOID pDefault,
                       PBYTE pbSymbol);

PVOID WINAPI w2kCopyEP (PVOID pDefault,
                        PBYTE pbSymbol);

BOOL WINAPI w2kXCall (PULARGE_INTEGER puliResult,
                      PBYTE           pbSymbol,
                      DWORD           dArgumentBytes,
                      PVOID           pArguments);

BOOL WINAPI w2kXCallV (PULARGE_INTEGER puliResult,
                       PBYTE           pbSymbol,
                       DWORD           dArgumentBytes,
                       ...);

NTSTATUS WINAPI w2kXCallNT (PBYTE pbSymbol,
                            DWORD dArgumentBytes,
                            ...);

BYTE WINAPI w2kXCall08 (BYTE  bDefault,
                        PBYTE pbSymbol,
                        DWORD dArgumentBytes,
                        ...);

WORD WINAPI w2kXCall16 (WORD  wDefault,
                        PBYTE pbSymbol,
                        DWORD dArgumentBytes,
                        ...);

DWORD WINAPI w2kXCall32 (DWORD dDefault,
                         PBYTE pbSymbol,
                         DWORD dArgumentBytes,
                         ...);

QWORD WINAPI w2kXCall64 (QWORD qDefault,
                         PBYTE pbSymbol,
                         DWORD dArgumentBytes,
                         ...);

PVOID WINAPI w2kXCallP (PVOID pDefault,
                        PBYTE pbSymbol,
                        DWORD dArgumentBytes,
                        ...);

BOOL WINAPI w2kXCopy (PULARGE_INTEGER puliResult,
                      PBYTE           pbSymbol,
                      DWORD           dBytes);

BYTE WINAPI w2kXCopy08 (BYTE  bDefault,
                        PBYTE pbSymbol);

WORD WINAPI w2kXCopy16 (WORD  wDefault,
                        PBYTE pbSymbol);

DWORD WINAPI w2kXCopy32 (DWORD dDefault,
                         PBYTE pbSymbol);

QWORD WINAPI w2kXCopy64 (QWORD qDefault,
                         PBYTE pbSymbol);

PVOID WINAPI w2kXCopyP (PVOID pDefault,
                        PBYTE pbSymbol);

PVOID WINAPI w2kXCopyEP (PVOID pDefault,
                         PBYTE pbSymbol);

// -----------------------------------------------------------------

BOOL WINAPI
w2kBeep (DWORD dDuration,
         DWORD dPitch);

BOOL WINAPI
w2kBeepEx (DWORD dData,
           ...);

POBJECT_HEADER WINAPI
w2kObjectHeader (POBJECT pObject);

POBJECT_CREATOR_INFO WINAPI
w2kObjectCreatorInfo (POBJECT_HEADER pHeader,
                      POBJECT        pObject);

POBJECT_NAME WINAPI
w2kObjectName (POBJECT_HEADER pHeader,
               POBJECT        pObject);

POBJECT_HANDLE_DB WINAPI
w2kObjectHandleDB (POBJECT_HEADER pHeader,
                   POBJECT        pObject);

POBJECT_QUOTA_CHARGES WINAPI
w2kObjectQuotaCharges (POBJECT_HEADER pHeader,
                       POBJECT        pObject);

POBJECT_TYPE WINAPI
w2kObjectType (POBJECT_HEADER pHeader);

PQUOTA_BLOCK WINAPI
w2kObjectQuotaBlock (POBJECT_HEADER pHeader);

POBJECT_CREATE_INFO WINAPI
w2kObjectCreateInfo (POBJECT_HEADER pHeader);

PW2K_OBJECT WINAPI
w2kObjectOpen (POBJECT pObject);

PW2K_OBJECT WINAPI
w2kObjectClose (PW2K_OBJECT pwo);

POBJECT_DIRECTORY WINAPI
w2kDirectoryOpen (POBJECT_DIRECTORY pDir);

POBJECT_DIRECTORY WINAPI
w2kDirectoryClose (POBJECT_DIRECTORY pDir);

DWORD WINAPI
w2kDirectorySize (POBJECT_DIRECTORY pDir,
                  PWORD             pwType);

// -----------------------------------------------------------------

BOOLEAN WINAPI
_ExAcquireResourceExclusiveLite (PERESOURCE Resource,
                                 BOOLEAN    Wait);

BOOLEAN WINAPI
_ExAcquireResourceSharedLite (PERESOURCE Resource,
                              BOOLEAN    Wait);

PVOID WINAPI
_ExAllocatePool (POOL_TYPE PoolType,
                 SIZE_T    NumberOfBytes);

PVOID WINAPI
_ExAllocatePoolWithTag (POOL_TYPE PoolType,
                        SIZE_T    NumberOfBytes,
                        DWORD     Tag);

VOID WINAPI
_ExFreePool (PVOID P);

VOID WINAPI
_ExFreePoolWithTag (PVOID P,
                    DWORD Tag);

BOOLEAN WINAPI
__ExLockHandleTableEntry (PHANDLE_TABLE HandleTable,
                          PHANDLE_ENTRY HandleEntry);

BOOLEAN WINAPI
__ExLockHandleTableExclusive (PHANDLE_TABLE HandleTable);

BOOLEAN WINAPI
__ExLockHandleTableShared (PHANDLE_TABLE HandleTable);

PHANDLE_ENTRY WINAPI
__ExMapHandleToPointer (PHANDLE_TABLE HandleTable,
                        HANDLE        Handle);

VOID WINAPI
_ExReleaseResourceLite (PERESOURCE Resource);

VOID WINAPI
__ExUnlockHandleTableEntry (PHANDLE_TABLE HandleTable,
                            PHANDLE_ENTRY HandleEntry);

VOID WINAPI
__ExUnlockHandleTableShared (PERESOURCE Resource);

PHANDLE_ENTRY WINAPI
__ExpLookupHandleTableEntry (PHANDLE_TABLE HandleTable,
                             HANDLE        Handle);

PBYTE WINAPI
_FsRtlLegalAnsiCharacterArray (VOID);

BOOLEAN WINAPI
_HalMakeBeep (DWORD Pitch);

VOID WINAPI
_HalQueryRealTimeClock (PTIME_FIELDS TimeFields);

VOID WINAPI
_HalSetRealTimeClock (PTIME_FIELDS TimeFields);

PLIST_ENTRY WINAPI
__HandleTableListHead (VOID);

PERESOURCE WINAPI
__HandleTableListLock (VOID);

DWORD WINAPI
_KeI386MachineType (VOID);

BYTE WINAPI
_KeNumberProcessors (VOID);

PSERVICE_DESCRIPTOR_TABLE WINAPI
_KeServiceDescriptorTable (VOID);

QWORD WINAPI
_KeTickCount (VOID);

PHYSICAL_ADDRESS WINAPI
_MmGetPhysicalAddress (PVOID BaseAddress);

PVOID WINAPI
_MmGetSystemRoutineAddress (PWORD SystemRoutineName);

PVOID WINAPI
_MmGetVirtualForPhysical (PHYSICAL_ADDRESS PhysicalAddress);

PVOID WINAPI
_MmHighestUserAddress (VOID);

BOOLEAN WINAPI
_MmIsAddressValid (PVOID VirtualAddress);

PVOID WINAPI
_MmSystemRangeStart (VOID);

PVOID WINAPI
_MmUserProbeAddress (VOID);

WORD WINAPI
_NlsAnsiCodePage (VOID);

BOOLEAN WINAPI
_NlsMbCodePageTag (VOID);

BOOLEAN WINAPI
_NlsMbOemCodePageTag (VOID);

WORD WINAPI
_NlsOemCodePage (VOID);

WORD WINAPI
_NtBuildNumber (VOID);

NTSTATUS WINAPI
_NtClose (HANDLE Handle);

DWORD WINAPI
_NtGlobalFlag (VOID);

VOID WINAPI
_ObDereferenceObject (POBJECT Object);

NTSTATUS WINAPI
_ObOpenObjectByPointer (POBJECT         Object,
                        DWORD           HandleAttributes,
         /* optional */ PACCESS_STATE   PassedAccessState,
                        ACCESS_MASK     DesiredAccess,
                        POBJECT_TYPE    ObjectType,
                        KPROCESSOR_MODE AccessMode,
                        PHANDLE         Handle);

NTSTATUS WINAPI
_ObQueryNameString (POBJECT                  Object,
                    POBJECT_NAME_INFORMATION NameString,
        /* bytes */ DWORD                    NameStringLength,
                    PDWORD                   ReturnLength);

NTSTATUS WINAPI
__ObQueryTypeInfo (POBJECT_TYPE      ObjectType,
                   POBJECT_TYPE_INFO TypeInfo,
   /* bytes     */ DWORD             TypeInfoLength,
   /* init to 0 */ PDWORD            ReturnLength);

NTSTATUS WINAPI
__ObQueryTypeName (POBJECT                  Object,
                   POBJECT_NAME_INFORMATION NameString,
       /* bytes */ DWORD                    NameStringLength,
                   PDWORD                   ReturnLength);

NTSTATUS WINAPI
_ObReferenceObjectByHandle
                     (HANDLE                     Handle,
                      ACCESS_MASK                DesiredAccess,
       /* optional */ POBJECT_TYPE               ObjectType,
                      KPROCESSOR_MODE            AccessMode,
                      PPOBJECT                   Object,
       /* optional */ POBJECT_HANDLE_INFORMATION HandleInformation);

NTSTATUS WINAPI
_ObReferenceObjectByPointer (POBJECT         Object,
                             ACCESS_MASK     DesiredAccess,
                             POBJECT_TYPE    ObjectType,
                             KPROCESSOR_MODE AccessMode);

VOID WINAPI
_ObfDereferenceObject (POBJECT Object);

VOID WINAPI
_ObfReferenceObject (POBJECT Object);

PHANDLE_TABLE WINAPI
__ObpKernelHandleTable (VOID);

PERESOURCE WINAPI
__ObpRootDirectoryMutex (VOID);

POBJECT_DIRECTORY WINAPI
__ObpRootDirectoryObject (VOID);

POBJECT_DIRECTORY WINAPI
__ObpTypeDirectoryObject (VOID);

DWORD WINAPI
_ProbeForRead (PVOID Address,
               DWORD Length,
               DWORD Alignment);

DWORD WINAPI
_ProbeForWrite (PVOID Address,
                DWORD Length,
                DWORD Alignment);

HANDLE WINAPI
_PsGetCurrentProcessId (VOID);

HANDLE WINAPI
_PsGetCurrentThreadId (VOID);

LARGE_INTEGER WINAPI
_PsGetProcessExitTime (VOID);

BOOLEAN WINAPI
_PsGetVersion (PDWORD          MajorVersion,
               PDWORD          MinorVersion,
               PDWORD          BuildNumber,
               PUNICODE_STRING CSDVersion);

PEPROCESS WINAPI
_PsInitialSystemProcess (VOID);

BOOLEAN WINAPI
_PsIsThreadTerminating (PETHREAD Thread);

NTSTATUS WINAPI
_PsLookupProcessByProcessId (HANDLE     UniqueProcessId,
                             PPEPROCESS Process);

NTSTATUS WINAPI
_PsLookupProcessThreadByCid (PCLIENT_ID Cid,
              /* optional */ PPEPROCESS Process,
                             PPETHREAD  Thread);

NTSTATUS WINAPI
_PsLookupThreadByThreadId (HANDLE    UniqueThreadId,
                           PPETHREAD Thread);

NTSTATUS WINAPI
_RtlAnsiStringToUnicodeString (PUNICODE_STRING Destination,
                               PANSI_STRING    Source,
                               BOOLEAN         AllocateDestination);

NTSTATUS WINAPI
_RtlAppendUnicodeStringToString (PUNICODE_STRING Destination,
                                 PUNICODE_STRING Source);

NTSTATUS WINAPI
_RtlAppendUnicodeToString (PUNICODE_STRING Destination,
                           PWORD           Source);

SIZE_T WINAPI
_RtlCompareMemory (PVOID  Source1,
                   PVOID  Source2,
                   SIZE_T Length);

SIZE_T WINAPI
_RtlCompareMemoryUlong (PVOID  Source1,
                        PVOID  Source2,
            /* Bytes */ SIZE_T Length);

LONG WINAPI
_RtlCompareUnicodeString (PUNICODE_STRING String1,
                          PUNICODE_STRING String2,
                          BOOLEAN         CaseInSensitive);

VOID WINAPI
_RtlCopyUnicodeString (PUNICODE_STRING Destination,
                       PUNICODE_STRING Source);

BOOLEAN WINAPI
_RtlEqualUnicodeString (PUNICODE_STRING String1,
                        PUNICODE_STRING String2,
                        BOOLEAN         CaseInSensitive);

VOID WINAPI
_RtlFillMemory (PVOID  Destination,
                SIZE_T Length,
                BYTE   Fill);

VOID WINAPI
_RtlFillMemoryUlong (PVOID  Destination,
         /* Bytes */ SIZE_T Length,
                     DWORD  Fill);

VOID WINAPI
_RtlFreeUnicodeString (PUNICODE_STRING UnicodeString);

PVOID WINAPI
_RtlImageDirectoryEntryToData (PVOID   Base,
                               BOOLEAN MappedAsImage,
                               WORD    DirectoryEntry,
                               PDWORD  Size);

PIMAGE_NT_HEADERS WINAPI
_RtlImageNtHeader (PVOID Base);

VOID WINAPI
_RtlInitAnsiString (PANSI_STRING Destination,
                    PBYTE        Source);

VOID WINAPI
_RtlInitString (PSTRING Destination,
                PBYTE   Source);

VOID WINAPI
_RtlInitUnicodeString (PUNICODE_STRING Destination,
                       PWORD           Source);

NTSTATUS WINAPI
_RtlInt64ToUnicodeString (QWORD          Value,
                          DWORD           Base,
                          PUNICODE_STRING String);

NTSTATUS WINAPI
_RtlIntegerToUnicodeString (DWORD           Value,
                            DWORD           Base,
                            PUNICODE_STRING String);

VOID WINAPI
_RtlMoveMemory (PVOID  Destination,
                PVOID  Source,
                SIZE_T Length);

BOOLEAN WINAPI
_RtlPrefixUnicodeString (PUNICODE_STRING String1,
                         PUNICODE_STRING String2,
                         BOOLEAN         CaseInSensitive);

NTSTATUS WINAPI
_RtlUnicodeStringToAnsiString (PANSI_STRING    Destination,
                               PUNICODE_STRING Source,
                               BOOLEAN         AllocateDestination);

NTSTATUS WINAPI
_RtlUnicodeStringToInteger (PUNICODE_STRING String,
                            DWORD           Base,
                            PDWORD          Value);

NTSTATUS WINAPI
_RtlUpcaseUnicodeString (PUNICODE_STRING Destination,
                         PUNICODE_STRING Source,
                         BOOLEAN         AllocateDestination);

VOID WINAPI
_RtlZeroMemory (PVOID  Destination,
                SIZE_T Length);

PACL WINAPI
_SePublicDefaultDacl (VOID);

PACL WINAPI
_SeSystemDefaultDacl (VOID);

NTSTATUS WINAPI
_TdiDeregisterPnPHandlers (HANDLE BindingHandle);

NTSTATUS WINAPI
_TdiEnumerateAddresses (HANDLE BindingHandle);

NTSTATUS WINAPI
_TdiRegisterPnPHandlers (PTDI_CLIENT_INTERFACE_INFO Info,
                         DWORD                      InfoSize,
                         PHANDLE                    BindingHandle);

NTSTATUS WINAPI
_ZwClose (HANDLE Handle);

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__CmpKeyObjectType (VOID);

POBJECT_TYPE WINAPI
__ExCallbackObjectType (VOID);

POBJECT_TYPE WINAPI
_ExDesktopObjectType (VOID);

POBJECT_TYPE WINAPI
_ExEventObjectType (VOID);

POBJECT_TYPE WINAPI
__ExEventPairObjectType (VOID);

POBJECT_TYPE WINAPI
__ExMutantObjectType (VOID);

POBJECT_TYPE WINAPI
__ExProfileObjectType (VOID);

POBJECT_TYPE WINAPI
_ExSemaphoreObjectType (VOID);

POBJECT_TYPE WINAPI
__ExTimerObjectType (VOID);

POBJECT_TYPE WINAPI
_ExWindowStationObjectType (VOID);

POBJECT_TYPE WINAPI
_IoAdapterObjectType (VOID);

POBJECT_TYPE WINAPI
__IoCompletionObjectType (VOID);

POBJECT_TYPE WINAPI
__IoControllerObjectType (VOID);

POBJECT_TYPE WINAPI
_IoDeviceObjectType (VOID);

POBJECT_TYPE WINAPI
_IoDriverObjectType (VOID);

POBJECT_TYPE WINAPI
_IoFileObjectType (VOID);

POBJECT_TYPE WINAPI
_LpcPortObjectType (VOID);

POBJECT_TYPE WINAPI
__LpcWaitablePortObjectType (VOID);

POBJECT_TYPE WINAPI
_MmSectionObjectType (VOID);

POBJECT_TYPE WINAPI
__ObpDirectoryObjectType (VOID);

POBJECT_TYPE WINAPI
__ObpSymbolicLinkObjectType (VOID);

POBJECT_TYPE WINAPI
__ObpTypeObjectType (VOID);

POBJECT_TYPE WINAPI
_PsJobType (VOID);

POBJECT_TYPE WINAPI
_PsProcessType (VOID);

POBJECT_TYPE WINAPI
_PsThreadType (VOID);

POBJECT_TYPE WINAPI
__SepTokenObjectType (VOID);

POBJECT_TYPE WINAPI
__WmipGuidObjectType (VOID);

// =================================================================
// LINKER CONTROL
// =================================================================

#ifdef _W2K_CALL_DLL_

#pragma comment (linker, "/entry:\"DllMain\"")

#else

#pragma comment (linker, "/defaultlib:w2k_call.lib")

#endif

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
