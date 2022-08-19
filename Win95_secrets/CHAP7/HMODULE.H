//==================================
// SHOW16 - Matt Pietrek 1995
// FILE: HMODULE.H
//==================================
#pragma pack (1)

typedef struct
{
    WORD    sector_offset;      // Offset to logical sector
    WORD    segment_length;     // Size in bytes of segment
    WORD    flags;              // flags for segment
    WORD    alloc_size;         // Segment allocation size
    WORD    handle;             // Global heap handle assigned by loader
} SEGMENT_RECORD, FAR * LPSEGMENT_RECORD;

typedef enum
{                           // Segment type constants
  CODE      = 0x0000,       // Code segment type
  DATA      = 0x0001,       // Data segment type
} SEGMENT_TYPES;

//
// In memory representation of the module table
//
typedef struct
{
   WORD     ne_signature;           // 'NE'
   WORD     ne_usage;               // reference count of module
   WORD     ne_npEntryTable;        // near pointer to entry table
   HMODULE  ne_npNextExe;           // next module database
   WORD     ne_npAutoData;          // near pointer to DGROUP segment entry
   WORD     ne_npFileInfo;          // near pointer to OFSTRUCT with file name
   WORD     ne_flags;
   WORD     ne_autodata;            // segment index of DGROUP segment
   WORD     ne_heap;
   WORD     ne_stack;
   DWORD    ne_csip;
   DWORD    ne_sssp;
   WORD     ne_cseg;
   WORD     ne_cModules;
   WORD     ne_cbNonResNamesTab;
   WORD     ne_segtab;
   WORD     ne_rsrcTab;
   WORD     ne_resNamesTab;
   WORD     ne_modRefTab;
   WORD     ne_importedNamesTab;
   DWORD    ne_nonResNamesTab;
   WORD     ne_cMovEnt;
   WORD     ne_align;
   WORD     ne_cres;
   unsigned char ne_exetyp;
   unsigned char ne_flagsother;
   WORD     ne_importedNamesTab2;
   WORD     ne_importedNamesTab3;
   WORD     ne_swaparea;        // minimum code swap area size
   WORD     ne_expver;          // expected windows version num
   DWORD    ne_Win32BaseAddr1;          // (Win32 only) Base addr of module
   DWORD    ne_Win32BaseAddr2;          // (Win32 only) Base addr of module
   DWORD    ne_Win32ResourceAddr;       // (Win32 only) Base addr of resources
} MODULE, FAR * LPMODULE;

typedef struct
{
    WORD    firstEntry;
    WORD    lastEntry;
    WORD    nextBundle;
} ENTRY_BUNDLE_HEADER, FAR *LPENTRY_BUNDLE_HEADER;

typedef struct
{
    BYTE    segType;
    BYTE    flags;
    BYTE    segNumber;
    WORD    offset;
} ENTRY, FAR * LPENTRY;

typedef struct
{
    WORD    ID;
    WORD    count;
    DWORD   function;
} RESOURCE_TYPE, FAR *LPRESOURCE_TYPE;

typedef struct
{
    WORD    offset;
    WORD    length;
    WORD    flags;
    WORD    ID;
    WORD    handle;
    WORD    usage;
} RESOURCE_INFO, FAR * LPRESOURCE_INFO;

#define NEAPPTYP        0x0700  // Application type mask
#define NEWINAPI        0x0300  // Uses windowing API
#define NEWINCOMPAT     0x0200  // Compatible with windowing API
#define NENOTWINCOMPAT  0x0100  // Not compatible with windowing API
#define NENONRES        0x0080  // Contains non-resident code segments
#define NELIM32         0x0010  // Uses LIM 3.2 API
#define NEPROT          0x0008  // Runs in protected mode only
#define NEPPLI          0x0004  // Per-Process Library Initialization

// Target operating systems

#define NE_UNKNOWN      0
#define NE_OS2          1   // Microsoft/IBM OS/2
#define NE_WINDOWS      2   // Microsoft Windows
#define NE_DOS4         3   // Microsoft European MS-DOS 4.x
#define NE_DEV386       4   // Microsoft Windows 386

#define MODFLAGS_DLL                0x8000
#define MODFLAGS_CALL_WEP           0x4000
#define MODFLAGS_SELF_LOADING       0x0800
#define MODFLAGS_APPTYPE            0x0300
                                    // 0x0300 = Uses Windows API
                                    // 0x0200 = Can be run in a window
                                    // 0x0100 = full-screen text app
#define MODFLAGS_IMPLICIT_LOAD      0x0040  // DLL is implicitly loaded
#define MODFLAGS_WIN32              0x0010
#define MODFLAGS_AUTODATA           0x0002
#define MODFLAGS_SINGLEDATA         0x0001

typedef struct tagOFSTRUCT_EXT
{
    WORD cBytes;            // This is a single BYTE in the regular OFSTRUCT
    BYTE fFixedDisk;
    UINT nErrCode;
    BYTE reserved[4];
    char szPathName[128];
} OFSTRUCT_EXT;

