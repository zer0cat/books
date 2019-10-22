
// w2k_img.h
// 08-27-2000 Sven B. Schreiber
// sbs@orgon.com

////////////////////////////////////////////////////////////////////
#ifdef _W2K_IMG_DLL_
////////////////////////////////////////////////////////////////////

// =================================================================
// PROGRAM IDENTIFICATION
// =================================================================

#define MAIN_BUILD              2
#define MAIN_VERSION_HIGH       1
#define MAIN_VERSION_LOW        1

// -----------------------------------------------------------------

#define MAIN_DAY                27
#define MAIN_MONTH              08
#define MAIN_YEAR               2000

// -----------------------------------------------------------------

#define MAIN_PREFIX             SBS
#define MAIN_MODULE             w2k_img
#define MAIN_NAME               SBS Windows 2000 Image Library
#define MAIN_COMPANY            Sven B. Schreiber
#define MAIN_AUTHOR             Sven B. Schreiber
#define MAIN_EMAIL              sbs@orgon.com
#define MAIN_DLL

////////////////////////////////////////////////////////////////////
#endif // #ifdef _W2K_IMG_DLL_
////////////////////////////////////////////////////////////////////

// =================================================================
// HEADER FILES
// =================================================================

#include <proginfo.h>

////////////////////////////////////////////////////////////////////
#ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// MACROS
// =================================================================

#define LCASEA(_c) ((BYTE) CharLowerA ((PBYTE) (_c)))
#define LCASEW(_c) ((WORD) CharLowerW ((PWORD) (_c)))

#define UCASEA(_c) ((BYTE) CharUpperA ((PBYTE) (_c)))
#define UCASEW(_c) ((WORD) CharUpperW ((PWORD) (_c)))

// =================================================================
// CONSTANTS
// =================================================================

#define YEARS_1600_TO_1970 ((DWORD) (1970-1600))
#define YEARS_400          ((DWORD) 400)
#define YEARS_100          ((DWORD) 100)
#define YEARS_004          ((DWORD) 4)

#define DAYS_1600_TO_1970  ((YEARS_1600_TO_1970 * 365) +25+24+24+17)
#define DAYS_PER_400_YEARS ((YEARS_400          * 365) +25+24+24+24)
#define DAYS_PER_100_YEARS ((YEARS_100          * 365) +25)
#define DAYS_PER_4_YEARS   ((YEARS_004          * 365) + 1)
#define DAYS_PER_YEAR      ((DWORD)               365)

#define MAX_PATH_EX        (100 * MAX_PATH)

// =================================================================
// SYMBOL TYPES
// =================================================================

#define IMG_CONVENTION_UNDEFINED    0
#define IMG_CONVENTION_STDCALL      1
#define IMG_CONVENTION_CDECL        2
#define IMG_CONVENTION_FASTCALL     3

// =================================================================
// STRUCTURE SIZES
// =================================================================

#define FPO_DATA_ \
        sizeof (FPO_DATA)

#define IMAGE_DEBUG_DIRECTORY_ \
        sizeof (IMAGE_DEBUG_DIRECTORY)

#define IMAGE_DEBUG_MISC_ \
        sizeof (IMAGE_DEBUG_MISC)

#define IMAGE_SECTION_HEADER_ \
        sizeof (IMAGE_SECTION_HEADER)

#define IMAGE_SEPARATE_DEBUG_HEADER_ \
        sizeof (IMAGE_SEPARATE_DEBUG_HEADER)

// =================================================================
// OMF STRUCTURES
// =================================================================

typedef struct _OMF_HEADER
    {
    WORD wRecordSize; // in bytes, not including this member
    WORD wRecordType;
    }
    OMF_HEADER, *POMF_HEADER, **PPOMF_HEADER;

#define OMF_HEADER_ sizeof (OMF_HEADER)

// -----------------------------------------------------------------

typedef struct _OMF_NAME
    {
    BYTE bLength;     // in bytes, not including this member
    BYTE abName [];
    }
    OMF_NAME, *POMF_NAME, **PPOMF_NAME;

#define OMF_NAME_ sizeof (OMF_NAME)

// =================================================================
// CodeView STRUCTURES
// =================================================================

#define CV_SIGNATURE_NB   'BN'
#define CV_SIGNATURE_NB09 '90BN'
#define CV_SIGNATURE_NB10 '01BN'

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef union _CV_SIGNATURE
    {
    WORD  wMagic;     // 'BN'
    DWORD dVersion;   // 'xxBN'
    BYTE  abText [4]; // "NBxx"
    }
    CV_SIGNATURE, *PCV_SIGNATURE, **PPCV_SIGNATURE;

#define CV_SIGNATURE_ sizeof (CV_SIGNATURE)

// -----------------------------------------------------------------

typedef struct _CV_HEADER
    {
    CV_SIGNATURE Signature;
    LONG         lOffset;
    }
    CV_HEADER, *PCV_HEADER, **PPCV_HEADER;

#define CV_HEADER_ sizeof (CV_HEADER)

// -----------------------------------------------------------------

typedef struct _CV_DIRECTORY
    {
    WORD  wSize;      // in bytes, including this member
    WORD  wEntrySize; // in bytes
    DWORD dEntries;
    LONG  lOffset;
    DWORD dFlags;
    }
    CV_DIRECTORY, *PCV_DIRECTORY, **PPCV_DIRECTORY;

#define CV_DIRECTORY_ sizeof (CV_DIRECTORY)

// -----------------------------------------------------------------

#define sstModule     0x0120 // CV_MODULE
#define sstGlobalPub  0x012A // CV_PUBSYM
#define sstSegMap     0x012D // SV_SEGMAP

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _CV_ENTRY
    {
    WORD  wSubSectionType;   // sst*
    WORD  wModuleIndex;      // -1 if not applicable
    LONG  lSubSectionOffset; // relative to CV_HEADER
    DWORD dSubSectionSize;   // in bytes, not including padding
    }
    CV_ENTRY, *PCV_ENTRY, **PPCV_ENTRY;

#define CV_ENTRY_ sizeof (CV_ENTRY)

// -----------------------------------------------------------------

typedef struct _CV_NB09 // CodeView 4.10
    {
    CV_HEADER    Header;
    CV_DIRECTORY Directory;
    CV_ENTRY     Entries [];
    }
    CV_NB09, *PCV_NB09, **PPCV_NB09;

#define CV_NB09_ sizeof (CV_NB09)

// -----------------------------------------------------------------

typedef struct _CV_NB10 // PDB reference
    {
    CV_HEADER    Header;
    DWORD        dSignature;   // seconds since 01-01-1970
    DWORD        dAge;         // 1++
    BYTE         abPdbName []; // zero-terminated
    }
    CV_NB10, *PCV_NB10, **PPCV_NB10;

#define CV_NB10_ sizeof (CV_NB10)

// -----------------------------------------------------------------

typedef union _CV_DATA
    {
    CV_HEADER Header;
    CV_NB09   NB09;
    CV_NB10   NB10;
    }
    CV_DATA, *PCV_DATA, **PPCV_DATA;

#define CV_DATA_ sizeof (CV_DATA)

// -----------------------------------------------------------------

typedef struct _CV_SEGMENT
    {
    WORD  wSegment;
    WORD  wReserved;
    DWORD dOffset;
    DWORD dSize;
    }
    CV_SEGMENT, *PCV_SEGMENT, **PPCV_SEGMENT;

#define CV_SEGMENT_ sizeof (CV_SEGMENT)

// -----------------------------------------------------------------

typedef struct _CV_MODULE
    {
    WORD       wOverlay;
    WORD       wLibrary;
    WORD       wSegments;
    WORD       wStyle;      // "CV"
    CV_SEGMENT Segments []; // wSegments
 // OMF_NAME   Name;        // use CV_MODULE_NAME() to access
    }
    CV_MODULE, *PCV_MODULE, **PPCV_MODULE;

#define CV_MODULE_ sizeof (CV_MODULE)

#define CV_MODULE_NAME(_p) \
        ((POMF_NAME) ((PBYTE) (_p)->Segments + \
                      ((DWORD) (_p)->wSegments * CV_SEGMENT_)))

// -----------------------------------------------------------------

typedef struct _CV_SYMHASH
    {
    WORD  wSymbolHashIndex;
    WORD  wAddressHashIndex;
    DWORD dSymbolInfoSize;
    DWORD dSymbolHashSize;
    DWORD dAddressHashSize;
    }
    CV_SYMHASH, *PCV_SYMHASH, **PPCV_SYMHASH;

#define CV_SYMHASH_ sizeof (CV_SYMHASH)

// -----------------------------------------------------------------

#define S_PUB32  0x0203
#define S_ALIGN  0x0402

#define CV_PUB32 S_PUB32

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _CV_PUBSYM
    {
    OMF_HEADER Header;
    DWORD      dOffset;
    WORD       wSegment;    // 1-based section index
    WORD       wTypeIndex;  // 0
    OMF_NAME   Name;        // zero-padded to next DWORD
    }
    CV_PUBSYM, *PCV_PUBSYM, **PPCV_PUBSYM;

#define CV_PUBSYM_ sizeof (CV_PUBSYM)

#define CV_PUBSYM_DATA(_p) \
        ((PCV_PUBSYM) ((PBYTE) (_p) + CV_SYMHASH_))

#define CV_PUBSYM_SIZE(_p) \
        ((DWORD) (_p)->Header.wRecordSize + sizeof (WORD))

#define CV_PUBSYM_NEXT(_p) \
        ((PCV_PUBSYM) ((PBYTE) (_p) + CV_PUBSYM_SIZE (_p)))

// -----------------------------------------------------------------

typedef struct _CV_SEGMAPDESC
    {
    WORD  wFlags;
    WORD  wOverlay;   // 0
    WORD  wGroup;     // 0
    WORD  wFrame;     // 1-based section index
    WORD  wName;      // -1
    WORD  wClassName; // -1
    DWORD dOffset;    // 0
    DWORD dSize;      // in bytes
    }
    CV_SEGMAPDESC, *PCV_SEGMAPDESC, **PPCV_SEGMAPDESC;

#define CV_SEGMAPDESC_ sizeof (CV_SEGMAPDESC)

// -----------------------------------------------------------------

typedef struct _CV_SEGMAP
    {
    WORD          wTotal;
    WORD          wLogical;
    CV_SEGMAPDESC Descriptors [];
    }
    CV_SEGMAP, *PCV_SEGMAP, **PPCV_SEGMAP;

#define CV_SEGMAP_ sizeof (CV_SEGMAP)

// =================================================================
// PDB STRUCTURES
// =================================================================

#define PDB_SIGNATURE_100 \
        "Microsoft C/C++ program database 1.00\r\n\x1AJG\0"

#define PDB_SIGNATURE_200 \
        "Microsoft C/C++ program database 2.00\r\n\x1AJG\0"

#define PDB_SIGNATURE_TEXT 40

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _PDB_SIGNATURE
    {
    BYTE abSignature [PDB_SIGNATURE_TEXT+4]; // PDB_SIGNATURE_nnn
    }
    PDB_SIGNATURE, *PPDB_SIGNATURE, **PPPDB_SIGNATURE;

#define PDB_SIGNATURE_ sizeof (PDB_SIGNATURE)

// -----------------------------------------------------------------

#define PDB_STREAM_FREE -1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _PDB_STREAM
    {
    DWORD dStreamSize;   // in bytes, -1 = free stream
    PWORD pwStreamPages; // array of page numbers
    }
    PDB_STREAM, *PPDB_STREAM, **PPPDB_STREAM;

#define PDB_STREAM_ sizeof (PDB_STREAM)

// -----------------------------------------------------------------

#define PDB_STREAM_MASK 0x0000FFFF
#define PDB_STREAM_MAX  (PDB_STREAM_MASK+1)

#define PDB_STREAM_DIRECTORY 0
#define PDB_STREAM_PDB       1
#define PDB_STREAM_TPI       2
#define PDB_STREAM_DBI       3
#define PDB_STREAM_PUBSYM    7

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _PDB_ROOT
    {
    WORD       wCount;      // < PDB_STREAM_MAX
    WORD       wReserved;   // 0
    PDB_STREAM aStreams []; // stream #0 reserved for stream table
    }
    PDB_ROOT, *PPDB_ROOT, **PPPDB_ROOT;

#define PDB_ROOT_ sizeof (PDB_ROOT)

#define PDB_PAGES(_r) \
        ((PWORD) ((PBYTE) (_r) \
                  + PDB_ROOT_ \
                  + ((DWORD) (_r)->wCount * PDB_STREAM_)))

// -----------------------------------------------------------------

#define PDB_PAGE_SIZE_1K   0x0400 // bytes per page
#define PDB_PAGE_SIZE_2K   0x0800
#define PDB_PAGE_SIZE_4K   0x1000

#define PDB_PAGE_SHIFT_1K  10     // log2 (PDB_PAGE_SIZE_*)
#define PDB_PAGE_SHIFT_2K  11
#define PDB_PAGE_SHIFT_4K  12

#define PDB_PAGE_COUNT_1K  0xFFFF // page number < PDB_PAGE_COUNT_*
#define PDB_PAGE_COUNT_2K  0xFFFF
#define PDB_PAGE_COUNT_4K  0x7FFF

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _PDB_HEADER
    {
    PDB_SIGNATURE Signature;      // PDB_SIGNATURE_200
    DWORD         dPageSize;      // 0x0400, 0x0800, 0x1000
    WORD          wStartPage;     // 0x0009, 0x0005, 0x0002
    WORD          wFilePages;     // file size / dPageSize
    PDB_STREAM    RootStream;     // stream directory
    WORD          awRootPages []; // pages containing PDB_ROOT
    }
    PDB_HEADER, *PPDB_HEADER, **PPPDB_HEADER;

#define PDB_HEADER_ sizeof (PDB_HEADER)

// -----------------------------------------------------------------

#define PDB_PUB32 0x1009

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _PDB_PUBSYM
    {
    OMF_HEADER Header;
    DWORD      dReserved;
    DWORD      dOffset;
    WORD       wSegment;    // 1-based section index
    OMF_NAME   Name;        // zero-padded to next DWORD
    }
    PDB_PUBSYM, *PPDB_PUBSYM, **PPPDB_PUBSYM;

#define PDB_PUBSYM_ sizeof (PDB_PUBSYM)

#define PDB_PUBSYM_SIZE(_p) \
        ((DWORD) (_p)->Header.wRecordSize + sizeof (WORD))

#define PDB_PUBSYM_NEXT(_p) \
        ((PPDB_PUBSYM) ((PBYTE) (_p) + PDB_PUBSYM_SIZE (_p)))

// =================================================================
// OMAP STRUCTURES
// =================================================================

typedef struct _OMAP_TO_SRC
    {
    DWORD dTarget;
    DWORD dSource;
    }
    OMAP_TO_SRC, *POMAP_TO_SRC, **PPOMAP_TO_SRC;

#define OMAP_TO_SRC_ sizeof (OMAP_TO_SRC)

// -----------------------------------------------------------------

typedef struct _OMAP_FROM_SRC
    {
    DWORD dSource;
    DWORD dTarget;
    }
    OMAP_FROM_SRC, *POMAP_FROM_SRC, **PPOMAP_FROM_SRC;

#define OMAP_FROM_SRC_ sizeof (OMAP_FROM_SRC)

// =================================================================
// OTHER STRUCTURES
// =================================================================

typedef struct _IMG_DBG
    {
    IMAGE_SEPARATE_DEBUG_HEADER Header;
    IMAGE_SECTION_HEADER        aSections [];
    }
    IMG_DBG, *PIMG_DBG, **PPIMG_DBG;

#define IMG_DBG_ sizeof (IMG_DBG)
#define IMG_DBG__(_n) (IMG_DBG_ + ((_n) * IMAGE_SECTION_HEADER_))

#define IMG_DBG_DATA(_p,_d) \
        ((PVOID) ((PBYTE) (_p) + (_d)->PointerToRawData))

// -----------------------------------------------------------------

typedef struct _IMG_PDB
    {
    PDB_HEADER Header;
    }
    IMG_PDB, *PIMG_PDB, **PPIMG_PDB;

#define IMG_PDB_ sizeof (IMG_PDB)
#define IMG_PDB__(_n) (IMG_PDB_ + ((_n) * sizeof (WORD)))

// -----------------------------------------------------------------

typedef union _IMG_PUBSYM
    {
    OMF_HEADER Header;    // CV_PUB32 or PDB_PUB32
    CV_PUBSYM  CvPubSym;
    PDB_PUBSYM PdbPubSym;
    }
    IMG_PUBSYM, *PIMG_PUBSYM, **PPIMG_PUBSYM;

#define IMG_PUBSYM_ sizeof (IMG_PUBSYM)

#define IMG_PUBSYM_SIZE(_p) \
        ((DWORD) (_p)->Header.wRecordSize + sizeof (WORD))

#define IMG_PUBSYM_NEXT(_p) \
        ((PIMG_PUBSYM) ((PBYTE) (_p) + IMG_PUBSYM_SIZE (_p)))

// -----------------------------------------------------------------

typedef struct _IMG_SYMBOL
    {
    PIMG_PUBSYM pip;
    DWORD       dSection;
    DWORD       dRaw;
    DWORD       dBias;
    DWORD       dSource;
    DWORD       dTarget;
    DWORD       dOffset;
    DWORD       dRelative;
    PVOID       pBase;
    PVOID       pAddress;
    DWORD       dConvention; // IMG_CONVENTION_*
    DWORD       dStack;
    BYTE        abName [256];
    }
    IMG_SYMBOL, *PIMG_SYMBOL, **PPIMG_SYMBOL;

#define IMG_SYMBOL_ sizeof (IMG_SYMBOL)

// -----------------------------------------------------------------

typedef struct _IMG_ENTRY
    {
    DWORD dSection;          // 1-based section number
    PVOID pAddress;          // symbol address
    DWORD dConvention;       // calling convention IMG_CONVENTION_*
    DWORD dStack;            // number of argument stack bytes
    BOOL  fExported;         // TRUE if exported symbol
    BOOL  fSpecial;          // TRUE if special symbol
    BYTE  abSection   [IMAGE_SIZEOF_SHORT_NAME+4]; // section name
    BYTE  abSymbol    [256]; // undecorated symbol name
    BYTE  abDecorated [256]; // decorated symbol name
    }
    IMG_ENTRY, *PIMG_ENTRY, **PPIMG_ENTRY;

#define IMG_ENTRY_ sizeof (IMG_ENTRY)

// -----------------------------------------------------------------

typedef struct _IMG_INDEX
    {
    PIMG_ENTRY apEntries [1];
    }
    IMG_INDEX, *PIMG_INDEX, **PPIMG_INDEX;

#define IMG_INDEX_ sizeof (IMG_INDEX)
#define IMG_INDEX__(_n) ((_n) * IMG_INDEX_)

// -----------------------------------------------------------------

typedef struct _IMG_TABLE
    {
    DWORD      dSize;      // table size in bytes
    DWORD      dSections;  // number of sections
    DWORD      dSymbols;   // number of symbols
    DWORD      dTimeStamp; // module time stamp (sec since 1-1-1970)
    DWORD      dCheckSum;  // module checksum
    PVOID      pBase;      // module base address
    PIMG_INDEX piiAddress; // entries sorted by address
    PIMG_INDEX piiName;    // entries sorted by name
    PIMG_INDEX piiNameIC;  // entries sorted by name (ignore case)
    BOOL       fUnicode;   // character format
    union
        {
        TBYTE atPath [MAX_PATH]; // .dbg file path
        BYTE  abPath [MAX_PATH]; // .dbg file path (ANSI)
        WORD  awPath [MAX_PATH]; // .dbg file path (Unicode)
        };
    IMG_ENTRY  aEntries []; // symbol info array
    }
    IMG_TABLE, *PIMG_TABLE, **PPIMG_TABLE;

#define IMG_TABLE_ sizeof (IMG_TABLE)

#define IMG_TABLE__(_n) \
        (IMG_TABLE_ + ((_n) * IMG_ENTRY_) + (3 * IMG_INDEX__ (_n)))

// -----------------------------------------------------------------

typedef struct _IMG_INFO
    {
    PVOID                        pBase;
    PIMAGE_SEPARATE_DEBUG_HEADER pHeader;
    PIMAGE_SECTION_HEADER        pSections;
    PBYTE                        pbExports;
    PIMAGE_DEBUG_DIRECTORY       pDirectories;
    PCV_DATA                     pCvData;
    PFPO_DATA                    pFpoEntries;
    PIMAGE_DEBUG_MISC            pMiscEntries;
    POMAP_TO_SRC                 pOmapToSrc;
    POMAP_FROM_SRC               pOmapFromSrc;
    DWORD                        dSize;
    DWORD                        dSections;
    DWORD                        dExports;
    DWORD                        dDirectories;
    DWORD                        dCvData;
    DWORD                        dFpoEntries;
    DWORD                        dMiscEntries;
    DWORD                        dOmapToSrc;
    DWORD                        dOmapFromSrc;
    BOOL                         fUnicode;
    union
        {
        TBYTE atPath [MAX_PATH];
        BYTE  abPath [MAX_PATH];
        WORD  awPath [MAX_PATH];
        };
    IMG_DBG                      DbgFile;
    }
    IMG_INFO, *PIMG_INFO, **PPIMG_INFO;

#define IMG_INFO_ sizeof (IMG_INFO)
#define IMG_INFO_PREFIX ((DWORD) &(((PIMG_INFO) 0)->DbgFile))

// -----------------------------------------------------------------

typedef struct _IMG_TIME
    {
    WORD wYear;
    BYTE bMonth;
    BYTE bDay;
    BYTE bHour;
    BYTE bMinute;
    BYTE bSecond;
    BYTE bDayOfWeek;
    }
    IMG_TIME, *PIMG_TIME, **PPIMG_TIME;

#define IMG_TIME_ sizeof (IMG_TIME)

// -----------------------------------------------------------------

typedef struct _IMG_CONTEXT
    {
    union
        {
        PBYTE pbExtension;
        PWORD pwExtension;
        };
    union
        {
        PBYTE pbBuffer;
        PWORD pwBuffer;
        };
    DWORD dBuffer;
    }
    IMG_CONTEXT, *PIMG_CONTEXT, **PPIMG_CONTEXT;

#define IMG_CONTEXT_ sizeof (IMG_CONTEXT)

// =================================================================
// CALLBACK TYPES
// =================================================================

typedef DWORD (CALLBACK *IMG_CALLBACKA) (PBYTE pbModule,
                                         PBYTE pbPath,
                                         DWORD dPath,
                                         PVOID pContext);

typedef DWORD (CALLBACK *IMG_CALLBACKW) (PWORD pwModule,
                                         PWORD pwPath,
                                         DWORD dPath,
                                         PVOID pContext);

// =================================================================
// KERNEL MODULE INFORMATION
// =================================================================

#ifndef SystemModuleInformation

// -----------------------------------------------------------------

#define SystemModuleInformation   11 // SYSTEMINFOCLASS
#define MAXIMUM_FILENAME_LENGTH  256
#define PAGE_SIZE               4096

// -----------------------------------------------------------------

typedef LONG NTSTATUS, *PNTSTATUS, **PPNTSTATUS;
typedef NTSTATUS (NTAPI *NTPROC) ();

#define STATUS_SUCCESS                      ((NTSTATUS) 0x00000000)
#define STATUS_INFO_LENGTH_MISMATCH         ((NTSTATUS) 0xC0000004)

// -----------------------------------------------------------------

NTSTATUS NTAPI
NtQuerySystemInformation (DWORD  SystemInformationClass,
                          PVOID  SystemInformation,
                          DWORD  SystemInformationLength,
                          PDWORD ReturnLength);

// -----------------------------------------------------------------

typedef struct _MODULE_INFO
    {
    DWORD dReserved1;
    DWORD dReserved2;
    PVOID pBase;
    DWORD dSize;
    DWORD dFlags;
    WORD  wIndex;
    WORD  wRank;
    WORD  wLoadCount;
    WORD  wNameOffset;
    BYTE  abPath [MAXIMUM_FILENAME_LENGTH];
    }
    MODULE_INFO, *PMODULE_INFO, **PPMODULE_INFO;

#define MODULE_INFO_ sizeof (MODULE_INFO)

// -----------------------------------------------------------------

typedef struct _MODULE_LIST
    {
    DWORD       dModules;
    MODULE_INFO aModules [];
    }
    MODULE_LIST, *PMODULE_LIST, **PPMODULE_LIST;

#define MODULE_LIST_ sizeof (MODULE_LIST)

// -----------------------------------------------------------------

#endif // #ifndef SystemModuleInformation

// =================================================================
// CONDITIONAL ANSI/UNICODE SYMBOLS
// =================================================================

#ifdef UNICODE

#define imgBox              imgBoxW
#define imgAnsiMatch        imgAnsiMatchW
#define imgTimeDay          imgTimeDayW
#define imgPathRoot         imgPathRootW
#define imgPathName         imgPathNameW
#define imgPathCanonical    imgPathCanonicalW
#define imgPathCurrent      imgPathCurrentW
#define imgPathWindows      imgPathWindowsW
#define imgPathVariable     imgPathVariableW
#define imgPathEnumerate    imgPathEnumerateW
#define imgPathSymbols      imgPathSymbolsW
#define imgPathCallback     imgPathCallbackW
#define imgPathSymbolsEx    imgPathSymbolsExW
#define imgPathDbg          imgPathDbgW
#define imgPathPdb          imgPathPdbW
#define imgFileOpen         imgFileOpenW
#define imgFileNew          imgFileNewW
#define imgFileTest         imgFileTestW
#define imgFileLoad         imgFileLoadW
#define imgFileSave         imgFileSaveW
#define imgCvPdb            imgCvPdbW
#define imgDbgLoad          imgDbgLoadW
#define imgPdbLoad          imgPdbLoadW
#define imgPdbStreamEx      imgPdbStreamExW
#define imgInfoLoad         imgInfoLoadW
#define imgInfoType         imgInfoTypeW
#define imgTableLoad        imgTableLoadW
#define imgModuleFind       imgModuleFindW
#define imgModuleBase       imgModuleBaseW

#else // #ifdef UNICODE

#define imgBox              imgBoxA
#define imgAnsiMatch        imgAnsiMatchA
#define imgTimeDay          imgTimeDayA
#define imgPathRoot         imgPathRootA
#define imgPathName         imgPathNameA
#define imgPathCanonical    imgPathCanonicalA
#define imgPathCurrent      imgPathCurrentA
#define imgPathWindows      imgPathWindowsA
#define imgPathVariable     imgPathVariableA
#define imgPathEnumerate    imgPathEnumerateA
#define imgPathSymbols      imgPathSymbolsA
#define imgPathCallback     imgPathCallbackA
#define imgPathSymbolsEx    imgPathSymbolsExA
#define imgPathDbg          imgPathDbgA
#define imgPathPdb          imgPathPdbA
#define imgFileOpen         imgFileOpenA
#define imgFileNew          imgFileNewA
#define imgFileTest         imgFileTestA
#define imgFileLoad         imgFileLoadA
#define imgFileSave         imgFileSaveA
#define imgCvPdb            imgCvPdbA
#define imgDbgLoad          imgDbgLoadA
#define imgPdbLoad          imgPdbLoadA
#define imgPdbStreamEx      imgPdbStreamExA
#define imgInfoLoad         imgInfoLoadA
#define imgInfoType         imgInfoTypeA
#define imgTableLoad        imgTableLoadA
#define imgModuleFind       imgModuleFindA
#define imgModuleBase       imgModuleBaseA

#endif // #ifdef UNICODE

// =================================================================
// API PROTOTYPES
// =================================================================

INT WINAPI imgBoxA (HWND  hWnd,
                    UINT  uiType,
                    PBYTE pbCaption,
                    PBYTE pbFormat,
                    ...);

INT WINAPI imgBoxW (HWND  hWnd,
                    UINT  uiType,
                    PWORD pwCaption,
                    PWORD pwFormat,
                    ...);

PVOID WINAPI imgMemoryCreate (DWORD dBytes);

PVOID WINAPI imgMemoryResize (PVOID pData,
                              DWORD dBytes);

PVOID WINAPI imgMemoryDestroy (PVOID pData);

BOOL WINAPI imgAnsiMatchA (PBYTE pbFilter,
                           PBYTE pbData,
                           BOOL  fIgnoreCase);

BOOL WINAPI imgAnsiMatchW (PWORD pwFilter,
                           PBYTE pbData,
                           BOOL  fIgnoreCase);

IMG_TIME WINAPI imgTimeNow (BOOL fLocal);

DWORD WINAPI imgTimePack (IMG_TIME it);

IMG_TIME WINAPI imgTimeUnpack (DWORD dTime);

PBYTE WINAPI imgTimeDayA (IMG_TIME it);

PWORD WINAPI imgTimeDayW (IMG_TIME it);

DWORD WINAPI imgPathRootA (PBYTE pbPath);

DWORD WINAPI imgPathRootW (PWORD pwPath);

DWORD WINAPI imgPathNameA (PBYTE  pbPath,
                           PDWORD pdExtension);

DWORD WINAPI imgPathNameW (PWORD  pwPath,
                           PDWORD pdExtension);

DWORD WINAPI imgPathCanonicalA (PBYTE pbPath,   // NULL: current dir
                                PBYTE pbBuffer, // can be == pbPath
                                DWORD dBuffer);

DWORD WINAPI imgPathCanonicalW (PWORD pwPath,   // NULL: current dir
                                PWORD pwBuffer, // can be == pwPath
                                DWORD dBuffer);

DWORD WINAPI imgPathCurrentA (PBYTE pbBuffer,
                              DWORD dBuffer);

DWORD WINAPI imgPathCurrentW (PWORD pwBuffer,
                              DWORD dBuffer);

DWORD WINAPI imgPathWindowsA (PBYTE pbBuffer,
                              DWORD dBuffer);

DWORD WINAPI imgPathWindowsW (PWORD pwBuffer,
                              DWORD dBuffer);

DWORD WINAPI imgPathVariableA (PBYTE pbVariable,
                               PBYTE pbBuffer,
                               DWORD dBuffer,
                               DWORD dIndex);

DWORD WINAPI imgPathVariableW (PWORD pwVariable,
                               PWORD pwBuffer,
                               DWORD dBuffer,
                               DWORD dIndex);

DWORD WINAPI imgPathEnumerateA (IMG_CALLBACKA CallbackA,
                                PBYTE         pbModule,
                                PVOID         pContext);

DWORD WINAPI imgPathEnumerateW (IMG_CALLBACKW CallbackW,
                                PWORD         pwModule,
                                PVOID         pContext);

DWORD WINAPI imgPathSymbolsA (PBYTE pbModule,   // NULL: ntoskrnl
                              PBYTE pbExtension,// NULL: .dbg
                              PBYTE pbPath,     // NULL: current dir
                              PBYTE pbBuffer,   // can be == pbPath
                              DWORD dBuffer,
                              BOOL  fSymbols);

DWORD WINAPI imgPathSymbolsW (PWORD pwModule,   // NULL: ntoskrnl
                              PWORD pwExtension,// NULL: .dbg
                              PWORD pwPath,     // NULL: current dir
                              PWORD pwBuffer,   // can be == pwPath
                              DWORD dBuffer,
                              BOOL  fSymbols);

DWORD CALLBACK imgPathCallbackA (PBYTE        pbModule,
                                 PBYTE        pbPath,
                                 DWORD        dPath,
                                 PIMG_CONTEXT pic);

DWORD CALLBACK imgPathCallbackW (PWORD        pwModule,
                                 PWORD        pwPath,
                                 DWORD        dPath,
                                 PIMG_CONTEXT pic);

DWORD WINAPI imgPathSymbolsExA (PBYTE pbModule,
                                PBYTE pbExtension,
                                PBYTE pbBuffer,
                                DWORD dBuffer);

DWORD WINAPI imgPathSymbolsExW (PWORD pwModule,
                                PWORD pwExtension,
                                PWORD pwBuffer,
                                DWORD dBuffer);

DWORD WINAPI imgPathDbgA (PBYTE pbModule,
                          PBYTE pbBuffer,
                          DWORD dBuffer);

DWORD WINAPI imgPathDbgW (PWORD pwModule,
                          PWORD pwBuffer,
                          DWORD dBuffer);

DWORD WINAPI imgPathPdbA (PBYTE pbModule,
                          PBYTE pbBuffer,
                          DWORD dBuffer);

DWORD WINAPI imgPathPdbW (PWORD pwModule,
                          PWORD pwBuffer,
                          DWORD dBuffer);

HANDLE WINAPI imgFileClose (HANDLE hf);

HANDLE WINAPI imgFileOpenA (PBYTE pbPath);

HANDLE WINAPI imgFileOpenW (PWORD pwPath);

HANDLE WINAPI imgFileNewA (PBYTE pbPath);

HANDLE WINAPI imgFileNewW (PWORD pwPath);

BOOL WINAPI imgFileTestA (PBYTE pbPath);

BOOL WINAPI imgFileTestW (PWORD pwPath);

PVOID WINAPI imgFileLoadA (PBYTE  pbPath,
                           PDWORD pdSize);

PVOID WINAPI imgFileLoadW (PWORD  pwPath,
                           PDWORD pdSize);

BOOL WINAPI imgFileSaveA (PBYTE pbPath,
                          PVOID pData,
                          DWORD dData);

BOOL WINAPI imgFileSaveW (PWORD pwPath,
                          PVOID pData,
                          DWORD dData);

PCV_ENTRY WINAPI imgCvEntry (PCV_NB09 pc09,
                             DWORD    dType,
                             DWORD    dIndex);

PCV_MODULE WINAPI imgCvModule (PCV_NB09 pc09,
                               DWORD    dIndex,
                               PDWORD   pdSize);

PCV_PUBSYM WINAPI imgCvSymbols (PCV_NB09 pc09,
                                PDWORD   pdCount,
                                PDWORD   pdSize);

PCV_SEGMAP WINAPI imgCvSegments (PCV_NB09 pc09,
                                 PDWORD   pdCount);

PVOID WINAPI imgCvPdbA (PCV_NB10 pc10,
                        PBYTE    pbPath,
                        PDWORD   pdSize);

PVOID WINAPI imgCvPdbW (PCV_NB10 pc10,
                        PWORD    pwPath,
                        PDWORD   pdSize);

BOOL WINAPI imgDbgVerify (PIMG_DBG pid,
                          DWORD    dSize);

PVOID WINAPI imgDbgLoadA (PBYTE  pbPath,
                          PDWORD pdSize);

PVOID WINAPI imgDbgLoadW (PWORD  pwPath,
                          PDWORD pdSize);

PBYTE WINAPI imgDbgExports (PIMG_DBG pid,
                            PDWORD   pdCount);

PIMAGE_DEBUG_DIRECTORY WINAPI imgDbgDirectories (PIMG_DBG pid,
                                                 PDWORD   pdCount);

PIMAGE_DEBUG_DIRECTORY WINAPI imgDbgDirectory (PIMG_DBG pid,
                                               DWORD    dType);

PCV_DATA WINAPI imgDbgCv (PIMG_DBG pid,
                          PDWORD   pdSize);

PFPO_DATA WINAPI imgDbgFpo (PIMG_DBG pid,
                            PDWORD   pdCount);

PIMAGE_DEBUG_MISC WINAPI imgDbgMisc (PIMG_DBG pid,
                                     PDWORD   pdCount);

POMAP_TO_SRC WINAPI imgDbgOmapToSrc (PIMG_DBG pid,
                                     PDWORD   pdCount);

POMAP_FROM_SRC WINAPI imgDbgOmapFromSrc (PIMG_DBG pid,
                                         PDWORD   pdCount);

BOOL WINAPI imgPdbVerify (PIMG_PDB pip,
                          DWORD    dSize);

PVOID WINAPI imgPdbLoadA (PBYTE  pbPath,
                          PDWORD pdSize);

PVOID WINAPI imgPdbLoadW (PWORD  pwPath,
                          PDWORD pdSize);

PVOID WINAPI imgPdbPage (PIMG_PDB pip,
                         DWORD    dPage);

DWORD WINAPI imgPdbPages (PIMG_PDB pip,
                          DWORD    dBytes);

PVOID WINAPI imgPdbRead (PIMG_PDB    pip,
                         PPDB_STREAM pps);

PPDB_ROOT WINAPI imgPdbRoot (PIMG_PDB pip);

PVOID WINAPI imgPdbStream (PIMG_PDB pip,
                           DWORD    dStream,
                           PDWORD   pdSize);

PVOID WINAPI imgPdbStreamExA (PBYTE  pbPath,
                              DWORD  dStream,
                              PDWORD pdSize);

PVOID WINAPI imgPdbStreamExW (PWORD  pwPath,
                              DWORD  dStream,
                              PDWORD pdSize);

PPDB_PUBSYM WINAPI imgPdbSymbols (PIMG_PDB pip,
                                  PDWORD   pdCount,
                                  PDWORD   pdSize);

BOOL WINAPI imgInfoInitialize (PIMG_INFO pii,
                               PVOID     pBase,
                               DWORD     dSize);

PIMG_INFO WINAPI imgInfoLoadA (PBYTE pbPath,
                               PVOID pBase);

PIMG_INFO WINAPI imgInfoLoadW (PWORD pwPath,
                               PVOID pBase);

DWORD WINAPI imgInfoOmapToSrc (PIMG_INFO pii,
                               DWORD     dTarget,
                               PDWORD    pdOffset);

DWORD WINAPI imgInfoOmapFromSrc (PIMG_INFO pii,
                                 DWORD     dSource,
                                 PDWORD    pdOffset);

DWORD WINAPI imgInfoOmapSection (PIMG_INFO pii,
                                 DWORD     dSection);

PIMG_PUBSYM WINAPI imgInfoSymbols (PIMG_INFO pii,
                                   PDWORD    pdCount,
                                   PDWORD    pdSize,
                                   PDWORD    pdVersion);

PBYTE WINAPI imgInfoTypeA (PIMG_INFO pii,
                           DWORD     dType);

PWORD WINAPI imgInfoTypeW (PIMG_INFO pii,
                           DWORD     dType);

PIMG_PUBSYM WINAPI imgSymbolNext (PIMG_PUBSYM pip);

DWORD WINAPI imgSymbolTest (PIMG_PUBSYM pip);

DWORD WINAPI imgSymbolUndecorate (PBYTE  pbSymbol,
                                  PBYTE  pbBuffer,
                                  PDWORD pdConvention);

DWORD WINAPI imgSymbolExported (PIMG_PUBSYM pip,
                                PIMG_INFO   pii);

PVOID WINAPI imgSymbolInfo (PIMG_PUBSYM pip,
                            PIMG_INFO   pii,
                            PIMG_SYMBOL pis,
                            BOOL        fUndecorate);

DWORD WINAPI imgSymbolLookup (PVOID       pAddress,
                              DWORD       dCount,
                              PIMG_PUBSYM pip,
                              PIMG_INFO   pii,
                              PIMG_SYMBOL pis,
                              BOOL        fUndecorate);

PIMG_TABLE WINAPI imgSymbolTable (DWORD       dCount,
                                  PIMG_PUBSYM pip,
                                  PIMG_INFO   pii);

INT WINAPI imgSymbolCompare (PIMG_ENTRY pie1,
                             PIMG_ENTRY pie2,
                             BOOL       fCompareNames,
                             BOOL       fIgnoreCase);

DWORD WINAPI imgSymbolSort (DWORD      dCount,
                            PIMG_INDEX pii,
                            BOOL       fSortByName,
                            BOOL       fIgnoreCase);

PIMG_TABLE WINAPI imgTableLoadA (PBYTE pbPath,
                                 PVOID pBase);

PIMG_TABLE WINAPI imgTableLoadW (PWORD pwPath,
                                 PVOID pBase);

PIMG_ENTRY WINAPI imgTableLookup (PIMG_TABLE pit,
                                  PVOID      pAddress,
                                  PDWORD     pdOffset);

PIMG_ENTRY WINAPI imgTableResolve (PIMG_TABLE pit,
                                   PBYTE      pbSymbol);

PVOID WINAPI imgModuleExport (PBYTE pbModule,
                              PBYTE pbExport);

PMODULE_LIST WINAPI imgModuleList (PDWORD pdData);

PMODULE_LIST WINAPI imgModuleFindA (PBYTE  pbModule,
                                    PDWORD pdIndex);

PMODULE_LIST WINAPI imgModuleFindW (PWORD  pwModule,
                                    PDWORD pdIndex);

PVOID WINAPI imgModuleBaseA (PBYTE pbModule);

PVOID WINAPI imgModuleBaseW (PWORD pwModule);

// =================================================================
// LINKER CONTROL
// =================================================================

#ifdef _W2K_IMG_DLL_
#pragma comment (linker, "/entry:\"DllMain\"")
#else
#pragma comment (linker, "/defaultlib:w2k_img.lib")
#endif

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
