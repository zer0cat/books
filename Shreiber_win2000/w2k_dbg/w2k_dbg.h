
// w2k_dbg.h
// 08-27-2000 Sven B. Schreiber
// sbs@orgon.com

#ifndef UNICODE
#define UNICODE // ANSI not supported by this library
#endif

////////////////////////////////////////////////////////////////////
#ifdef _W2K_DBG_DLL_
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
#define MAIN_MODULE             w2k_dbg
#define MAIN_NAME               SBS Windows 2000 Debugging Library
#define MAIN_COMPANY            Sven B. Schreiber
#define MAIN_AUTHOR             Sven B. Schreiber
#define MAIN_EMAIL              sbs@orgon.com
#define MAIN_DLL

////////////////////////////////////////////////////////////////////
#endif // #ifdef _W2K_DBG_DLL_
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

#include <imagehlp.h>
#include <psapi.h>

// =================================================================
// MACROS
// =================================================================

#define LCASE(_c)               ((WORD) CharLowerW ((PWORD) (_c)))
#define UCASE(_c)               ((WORD) CharUpperW ((PWORD) (_c)))
#define OFFSET(_s,_m)           ((DWORD) &(((_s *) 0)->_m))

// =================================================================
// CONSTANTS
// =================================================================

#define SIZE_MINIMUM            0x00000100 // minimum list size
#define SIZE_MAXIMUM            0x00010000 // maximum list size
#define SIZE_INCREMENT          0x00010000 // memory block increment
#define SIZE_ALIGNMENT          3          // alignment shift factor

#define UNICODE_UNMAPPED        0x7F       // substitute character

#define DBG_SORT_TYPE           0x000000FF // type mask
#define DBG_SORT_NONE           0x00000000 // sort disabled
#define DBG_SORT_RESTORE        0x00000001 // restore original order
#define DBG_SORT_DWORD          0x00000002 // unsigned number
#define DBG_SORT_PVOID          0x00000003 // pointer
#define DBG_SORT_STRING         0x00000004 // unicode string

#define DBG_SORT_FLAGS          0x0000FF00 // flag mask
#define DBG_SORT_REVERSE        0x00000100 // reverse order
#define DBG_SORT_CASE           0x00000200 // case sensitive string

#define DBG_UNSORTED            0
#define DBG_SORT_BY_ADDRESS     1
#define DBG_SORT_BY_SIZE        2
#define DBG_SORT_BY_ID          3
#define DBG_SORT_BY_NAME        4
#define DBG_SORT_BY_NAME_CS     5

// =================================================================
// POINTER TYPES
// =================================================================

typedef HMODULE                 *PHMODULE;
typedef PVOID                   *PPVOID;
typedef PBYTE                   *PPBYTE;

// =================================================================
// STRUCTURES
// =================================================================

typedef struct _DBG_MEMORY
    {
    DWORD dTag;
    DWORD dSize;
    BYTE  abData [];
    }
    DBG_MEMORY, *PDBG_MEMORY, **PPDBG_MEMORY;

#define DBG_MEMORY_ sizeof (DBG_MEMORY)
#define DBG_MEMORY_TAG '>gbd' // dbg>

// -----------------------------------------------------------------

typedef struct _DBG_LIST
    {
    DWORD      dTag;
    DWORD      dFirst;
    DWORD      dMemory;
    DWORD      dOffset;
    DWORD      dEntries;
    DWORD      dCrc32;
    DWORD      dContext;
    PVOID      pContext;
    SYSTEMTIME st;
    BYTE       abData [];
    }
    DBG_LIST, *PDBG_LIST, **PPDBG_LIST;

#define DBG_LIST_ sizeof (DBG_LIST)
#define DBG_LIST_TAG 'LGBD' // DBGL

// -----------------------------------------------------------------

typedef struct _DBG_SYMBOL
    {
    DWORD dNext;
    DWORD dSize;
    PVOID pBase;
    WORD  awName [];
    }
    DBG_SYMBOL, *PDBG_SYMBOL, **PPDBG_SYMBOL;

#define DBG_SYMBOL_ sizeof (DBG_SYMBOL)

// -----------------------------------------------------------------

typedef struct _DBG_PROCESS
    {
    DWORD dNext;
    DWORD dSize;
    PVOID pBase;
    PVOID pStart;
    DWORD dDown;
    DWORD dId;
    DWORD dModules;
    DWORD dFile;
    WORD  awPath [];
    }
    DBG_PROCESS, *PDBG_PROCESS, **PPDBG_PROCESS;

#define DBG_PROCESS_ sizeof (DBG_PROCESS)

// -----------------------------------------------------------------

typedef struct _DBG_MODULE
    {
    DWORD dNext;
    DWORD dSize;
    PVOID pBase;
    PVOID pStart;
    DWORD dFile;
    WORD  awPath [];
    }
    DBG_MODULE, *PDBG_MODULE, **PPDBG_MODULE;

#define DBG_MODULE_ sizeof (DBG_MODULE)

// -----------------------------------------------------------------

typedef struct _DBG_DRIVER
    {
    DWORD dNext;
    DWORD dSize;
    PVOID pBase;
    DWORD dFile;
    WORD  awPath [];
    }
    DBG_DRIVER, *PDBG_DRIVER, **PPDBG_DRIVER;

#define DBG_DRIVER_ sizeof (DBG_DRIVER)

// -----------------------------------------------------------------

typedef struct _DBG_INDEX
    {
    union
        {
        PPDBG_SYMBOL        ppds;
        PPDBG_PROCESS       ppdp;
        PPDBG_MODULE        ppdm;
        PPDBG_DRIVER        ppdd;
        struct _DBG_INDEX **ppdi;
        PPBYTE              ppbData;
        };
    PDBG_LIST pdl;
    PDBG_LIST pdlDestroy;
    DWORD     dMemberNext;
    DWORD     dData;
    DWORD     dEntries;
    DWORD     dContext;
    PVOID     pContext;
    PBYTE     apbData [];
    }
    DBG_INDEX, *PDBG_INDEX, **PPDBG_INDEX;

#define DBG_INDEX_ sizeof (DBG_INDEX)
#define DBG_INDEX__(_n) (DBG_INDEX_ + ((_n) * sizeof (PVOID)))

// =================================================================
// API PROTOTYPES
// =================================================================

DWORD WINAPI dbgCrc32Start (PDWORD pdCrc32);

BOOL WINAPI dbgCrc32Stop (PDWORD pdCrc32);

DWORD WINAPI dbgCrc32Byte (PDWORD pdCrc32,
                           BYTE   bData);

DWORD WINAPI dbgCrc32Block (PDWORD pdCrc32,
                            PVOID  pData,
                            DWORD  dData);

BOOL WINAPI dbgPrivilegeSet (PWORD pwName);

BOOL WINAPI dbgPrivilegeDebug (void);

PVOID WINAPI dbgMemoryCreate (DWORD dSize);

PVOID WINAPI dbgMemoryCreateEx (DWORD dSize,
                                DWORD dTag);

PDBG_MEMORY WINAPI dbgMemoryBase (PVOID pData);

PDBG_MEMORY WINAPI dbgMemoryBaseEx (PVOID pData,
                                    DWORD dTag);

PVOID WINAPI dbgMemoryResize (PVOID pData,
                              DWORD dSize,
                              PBOOL pfOk);

PVOID WINAPI dbgMemoryResizeEx (PVOID pData,
                                DWORD dSize,
                                PBOOL pfOk,
                                DWORD dTag);

PVOID WINAPI dbgMemoryDestroy (PVOID pData);

PVOID WINAPI dbgMemoryDestroyEx (PVOID pData,
                                 DWORD dTag);

void WINAPI dbgMemoryReset (void);

void WINAPI dbgMemoryTrack (DWORD dSize,
                            BOOL  fAdd);

BOOL WINAPI dbgMemoryStatus (PDWORD pdMemoryNow,
                             PDWORD pdMemoryMax);

DWORD WINAPI dbgMemoryAlign (DWORD dSize);

DWORD WINAPI dbgMemoryAlignEx (DWORD dFixed,
                               DWORD dText);

DWORD WINAPI dbgFileRoot (PWORD pwPath);

HANDLE WINAPI dbgFileOpen (PWORD pwPath);

HANDLE WINAPI dbgFileNew (PWORD pwPath);

HANDLE WINAPI dbgFileClose (HANDLE hf);

PVOID WINAPI dbgFileUnload (PVOID pData);

PVOID WINAPI dbgFileLoad (PWORD  pwPath,
                          PDWORD pdData);

BOOL WINAPI dbgFileSave (PWORD pwPath,
                         PVOID pData,
                         DWORD dData);

DWORD WINAPI dbgSizeDivide (DWORD dShift,
                            DWORD dSize,
                            BOOL  fRoundUp,
                            BOOL  fRoundDown);

DWORD WINAPI dbgSizeKB (DWORD dBytes,
                        BOOL  fRoundUp,
                        BOOL  fRoundDown);

DWORD WINAPI dbgSizeMB (DWORD dBytes,
                        BOOL  fRoundUp,
                        BOOL  fRoundDown);

PBYTE WINAPI dbgStringAnsi (PWORD pwData,
                            PBYTE pbData);

BOOL WINAPI dbgStringMatch (PWORD pwFilter,
                            PWORD pwData,
                            BOOL  fCase);

PWORD WINAPI dbgStringDay (DWORD dDay);

DWORD WINAPI dbgPathFile (PWORD pwPath);

DWORD WINAPI dbgPathDriver (PWORD pwFile,
                            PWORD pwBuffer,
                            DWORD dBuffer);

PDBG_LIST WINAPI dbgListCreate (void);

PDBG_LIST WINAPI dbgListCreateEx (DWORD       dData,
                                  DWORD       dEntries,
                                  DWORD       dContext,
                                  PVOID       pContext,
                                  PSYSTEMTIME pst);

PDBG_LIST WINAPI dbgListDestroy (PDBG_LIST pdl);

PDBG_LIST WINAPI dbgListResize (PDBG_LIST pdl,
                                DWORD     dData);

DWORD WINAPI dbgListNext (PDBG_LIST pdl,
                          DWORD     dData,
                          BOOL      fCount);

PDBG_LIST WINAPI dbgListFinish (PDBG_LIST pdl);

PDBG_INDEX WINAPI dbgListIndex (PDBG_LIST pdl,
                                DWORD     dMemberNext);

PDBG_LIST WINAPI dbgListLoad (PWORD pwPath);

BOOL WINAPI dbgListSave (PWORD     pwPath,
                         PDBG_LIST pdl);

PDBG_INDEX WINAPI dbgIndexCreate (PDBG_LIST pdl,
                                  DWORD     dMemberNext);

PDBG_INDEX WINAPI dbgIndexCreateEx (PDBG_LIST pdl,
                                    DWORD     dMemberNext,
                                    DWORD     dMemberAddress,
                                    DWORD     dMemberSize,
                                    DWORD     dMemberId,
                                    DWORD     dMemberNameData,
                                    DWORD     dMemberNameOffset,
                                    DWORD     dSort,
                                    BOOL      fReverse);

PDBG_INDEX WINAPI dbgIndexDestroy (PDBG_INDEX pdi);

PDBG_INDEX WINAPI dbgIndexDestroyEx (PDBG_INDEX pdi);

void WINAPI dbgIndexReverse (PDBG_INDEX pdi);

INT WINAPI dbgIndexCompare (PVOID pData1,
                            PVOID pData2,
                            DWORD dControl);

void WINAPI dbgIndexSort (PDBG_INDEX pdi,
                          DWORD      dMemberData,
                          DWORD      dMemberOffset,
                          DWORD      dControl);

PDBG_LIST WINAPI dbgIndexList (PDBG_INDEX pdi);

PDBG_LIST WINAPI dbgIndexListEx (PDBG_INDEX pdi);

BOOL WINAPI dbgIndexSave (PWORD      pwPath,
                          PDBG_INDEX pdi);

BOOL WINAPI dbgIndexSaveEx (PWORD      pwPath,
                            PDBG_INDEX pdi);

PDWORD WINAPI dbgProcessIds (PDWORD pdCount);

PHMODULE WINAPI dbgProcessModules (HANDLE hProcess,
                                   PDWORD pdCount);

PDBG_LIST WINAPI dbgProcessAdd (PDBG_LIST pdl,
                                DWORD     dId);

PDBG_LIST WINAPI dbgProcessList (void);

PDBG_INDEX WINAPI dbgProcessIndex (PWORD pwImage,
                                   DWORD dSort,
                                   BOOL  fReverse);

PDBG_INDEX WINAPI dbgProcessIndexEx (PWORD pwImage,
                                     DWORD dSort,
                                     BOOL  fReverse);

PWORD WINAPI dbgProcessGuess (DWORD dIndex);

PDBG_LIST WINAPI dbgModuleList (PDBG_PROCESS pdp);

PDBG_INDEX WINAPI dbgModuleIndex (PDBG_PROCESS pdp,
                                  DWORD        dSort,
                                  BOOL         fReverse);

PPVOID WINAPI dbgDriverAddresses (PDWORD pdCount);

PDBG_LIST WINAPI dbgDriverAdd (PDBG_LIST pdl,
                               PVOID     pBase);

PDBG_LIST WINAPI dbgDriverList (void);

PDBG_INDEX WINAPI dbgDriverIndex (PWORD pwImage,
                                  DWORD dSort,
                                  BOOL  fReverse);

BOOL CALLBACK dbgSymbolCallback (PSTR       psSymbolName,
                                 DWORD      dSymbolAddress,
                                 DWORD      dSymbolSize,
                                 PPDBG_LIST ppdl);

PLOADED_IMAGE WINAPI dbgSymbolLoad (PWORD  pwPath,
                                    PVOID  pBase,
                                    HANDLE hProcess);

PLOADED_IMAGE WINAPI dbgSymbolUnload (PLOADED_IMAGE pli,
                                      PVOID         pBase,
                                      HANDLE        hProcess);

PDBG_LIST WINAPI dbgSymbolList (PWORD pwPath,
                                PVOID pBase);

PDBG_INDEX WINAPI dbgSymbolIndex (PWORD pwPath,
                                  PVOID pBase,
                                  PWORD pwImage,
                                  DWORD dSort,
                                  BOOL  fReverse);

PDBG_SYMBOL WINAPI dbgSymbolLookup (PDBG_INDEX pdi,
                                    PVOID      pAddress,
                                    PDWORD     pdOffset);

PVOID WINAPI dbgBaseModule (PWORD  pwPath,
                            PDWORD pdSize);

PVOID WINAPI dbgBaseDriver (PWORD  pwPath,
                            PDWORD pdSize);

// =================================================================
// NTDLL.DLL STRUCTURES
// =================================================================

typedef struct _UNICODE_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
    }
    UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING;

#define UNICODE_STRING_ sizeof (UNICODE_STRING)

// =================================================================
// NTDLL.DLL API FUNCTIONS
// =================================================================

NTSYSAPI LONG NTAPI
RtlCompareUnicodeString (PUNICODE_STRING String1,
                         PUNICODE_STRING String2,
                         BOOLEAN         CaseInSensitive);

NTSYSAPI VOID NTAPI
RtlInitUnicodeString (PUNICODE_STRING DestinationString,
                      PCWSTR          SourceString);

// =================================================================
// LINKER CONTROL
// =================================================================

#ifdef _W2K_DBG_DLL_

#pragma comment (linker, "/entry:\"DllMain\"")
#pragma comment (linker, "/defaultlib:imagehlp.lib")
#pragma comment (linker, "/defaultlib:psapi.lib")
#pragma comment (linker, "/defaultlib:ntdll.lib")

#else

#pragma comment (linker, "/defaultlib:w2k_dbg.lib")

#endif

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
