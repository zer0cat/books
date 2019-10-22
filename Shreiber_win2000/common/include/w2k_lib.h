
// w2k_lib.h
// 02-12-2001 Sven B. Schreiber
// sbs@orgon.com

#ifndef UNICODE
#define UNICODE     // ANSI not supported by this library
#endif

// =================================================================
// PROGRAM VERSION
// =================================================================

#define W2K_LIB_MODULE          w2k_lib

#define W2K_LIB_BUILD           1
#define W2K_LIB_VERSION_HIGH    1
#define W2K_LIB_VERSION_LOW     0

#define W2K_LIB_VERSION         ((W2K_LIB_VERSION_HIGH * 100) \
                                + W2K_LIB_VERSION_LOW)

////////////////////////////////////////////////////////////////////
#ifdef _W2K_LIB_DLL_
////////////////////////////////////////////////////////////////////

// =================================================================
// PROGRAM IDENTIFICATION
// =================================================================

#define MAIN_BUILD              W2K_LIB_BUILD
#define MAIN_VERSION_HIGH       W2K_LIB_VERSION_HIGH
#define MAIN_VERSION_LOW        W2K_LIB_VERSION_LOW

// -----------------------------------------------------------------

#define MAIN_DAY                12
#define MAIN_MONTH              02
#define MAIN_YEAR               2001

// -----------------------------------------------------------------

#define MAIN_PREFIX             SBS
#define MAIN_MODULE             W2K_LIB_MODULE
#define MAIN_NAME               SBS Windows 2000 Utility Library
#define MAIN_COMPANY            Sven B. Schreiber
#define MAIN_AUTHOR             Sven B. Schreiber
#define MAIN_EMAIL              sbs@orgon.com
#define MAIN_DLL

////////////////////////////////////////////////////////////////////
#endif // #ifdef _W2K_LIB_DLL_
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

#define ENTRY(_a)        ((DWORD) ((PBYTE) ((_a)+1) - (PBYTE) (_a)))
#define OFFSET(_s,_m)    ((DWORD) &(((_s *) 0)->_m))
#define MEMBER(_s,_m)    sizeof (((_s *) 0)->_m)
#define SKIP(_p,_d)      ((PVOID) (((PBYTE) (_p)) + (_d)))
#define BACK(_p,_d)      ((PVOID) (((PBYTE) (_p)) - (_d)))
#define BASE(_p,_s,_m)   ((_s *) BACK (_p, OFFSET (_s, _m)))

#define COUNT(_n,_1,_x)  ((_n) == 1 ? (_1) : (_x))
#define COUNTX(_n,_1,_x) (_n), COUNT(_n,_1,_x)

#define LCASE(_c)        ((WORD) CharLower ((PWORD) (_c)))
#define UCASE(_c)        ((WORD) CharUpper ((PWORD) (_c)))

#define W2K_VERSION1(_q) ((WORD) ((_q) >> 48))
#define W2K_VERSION2(_q) ((WORD) ((_q) >> 32))
#define W2K_VERSION3(_q) ((WORD) ((_q) >> 16))
#define W2K_VERSION4(_q) ((WORD)  (_q)       )

// =================================================================
// VARIOUS CONSTANTS
// =================================================================

#define N_STRING                    0x0100
#define N_CONSOLE                   0x1000
#define N_WRITE                     0x4000 // WriteFile() block size

#define N_NAME                      N_STRING
#define N_LEVEL                     N_STRING

#define N_DECIMAL16                 ( 5 + 1)
#define N_DECIMAL32                 (10 + 1)
#define N_HEX16                     ( 4 + 1)
#define N_HEX32                     ( 8 + 1)

#define N_VERSION                   (2 * N_DECIMAL32)   // a.bb
#define N_VERSION_EX                (4 * N_DECIMAL16)   // a.b.c.d
#define N_BUILD                     (3 * N_DECIMAL32)   // a.bb.c

#define HKEY_NULL                   ((HKEY ) -1)
#define VERSION_NULL                ((QWORD) -1)

#define INVALID_UNICODE             0x7F

// =================================================================
// TYPE SIZE DEFINITIONS
// =================================================================

#define ENUM_SERVICE_STATUS_        sizeof (ENUM_SERVICE_STATUS)

#define BYTE_                       sizeof (BYTE)
#define WORD_                       sizeof (WORD)
#define DWORD_                      sizeof (DWORD)
#define PVOID_                      sizeof (PVOID)
#define HANDLE_                     sizeof (HANDLE)
#define HMODULE_                    sizeof (HMODULE)

#define BYTE__                      (BYTE_  * 8)
#define WORD__                      (WORD_  * 8)
#define DWORD__                     (DWORD_ * 8)

// =================================================================
// REGISTRY R/W MODES
// =================================================================

#define W2K_MODE_DWORD              0xFFFFFFFF
#define W2K_MODE_TEXT               0xFFFFFFFE
#define W2K_MODE_BINARY(_m)         ((DWORD) (_m) < W2K_MODE_TEXT)

// =================================================================
// POOL NOTIFICATION IDS
// =================================================================

#define W2K_NOTIFY_UNDEFINED        0
#define W2K_NOTIFY_ENUMERATE        1
#define W2K_NOTIFY_ADD              2
#define W2K_NOTIFY_REMOVE           3
#define W2K_NOTIFY_DESTROY          4

// =================================================================
// LIST NODE TYPES
// =================================================================

#define W2K_NODE_ANY                0

// =================================================================
// SYSTEM IDS
// =================================================================

#define W2K_SYSTEM_WINNT3           3
#define W2K_SYSTEM_WINNT4           4
#define W2K_SYSTEM_WIN2000          5

// =================================================================
// POINTER TYPES
// =================================================================

typedef VOID                                    **PPVOID;
typedef WORD                                    **PPWORD;
typedef DWORD                                   **PPDWORD;
typedef DWORDLONG QWORD,    *PQWORD,            **PPQWORD;
typedef COLORREF            *PCOLORREF,         **PPCOLORREF;
typedef RGBQUAD             *PRGBQUAD,          **PPRGBQUAD;
typedef HBITMAP             *PHBITMAP,          **PPHBITMAP;
typedef HICON               *PHICON,            **PPHICON;
typedef HMODULE             *PHMODULE,          **PPHMODULE;
typedef HWND                *PHWND,             **PPHWND;
typedef SC_HANDLE           *PSC_HANDLE,        **PPSC_HANDLE;
typedef SERVICE_STATUS      *PSERVICE_STATUS,   **PPSERVICE_STATUS;

typedef ENUM_SERVICE_STATUS  * PENUM_SERVICE_STATUS,
                            **PPENUM_SERVICE_STATUS;

// =================================================================
// FUNCTION TYPES
// =================================================================

typedef BOOL (CALLBACK *W2K_WALK  ) (PVOID             pThis,
                                     PVOID             pData);

typedef BOOL (CALLBACK *W2K_NOTIFY) (struct _W2K_POOL *pwp,
                                     DWORD             dMode,
                                     PWORD             pwName,
                                     PVOID             pData);

// =================================================================
// STRUCTURES
// =================================================================

typedef struct _W2K_MEMORY
    {
    DWORD dTag;
    DWORD dSize;
    BYTE  abData [];
    }
    W2K_MEMORY, *PW2K_MEMORY, **PPW2K_MEMORY;

#define W2K_MEMORY_ sizeof (W2K_MEMORY)
#define W2K_MEMORY_TAG '>k2w' // w2k>

// -----------------------------------------------------------------

typedef struct _W2K_ENTRY
    {
    WORD  awName [N_NAME];
    PVOID pData;
    }
    W2K_ENTRY, *PW2K_ENTRY, **PPW2K_ENTRY;

#define W2K_ENTRY_ sizeof (W2K_ENTRY)

// -----------------------------------------------------------------

typedef struct _W2K_POOL
    {
    CRITICAL_SECTION cs;
    SYSTEMTIME       stCreate;
    SYSTEMTIME       stUpdate;
    SYSTEMTIME       stAdd;
    SYSTEMTIME       stRemove;
    DWORD            dCount;
    W2K_ENTRY        awe [];
    }
    W2K_POOL, *PW2K_POOL, **PPW2K_POOL;

#define W2K_POOL_ sizeof (W2K_POOL)

#define W2K_POOL__(_n) \
        (W2K_POOL_ + ((_n) * W2K_ENTRY_))

// -----------------------------------------------------------------

typedef struct _W2K_NODE
    {
    struct _W2K_NODE *pwnBack;
    struct _W2K_NODE *pwnNext;
    PVOID             pThis;
    DWORD             dType;
    }
    W2K_NODE, *PW2K_NODE, **PPW2K_NODE;

#define W2K_NODE_ sizeof (W2K_NODE)

// -----------------------------------------------------------------

typedef struct _W2K_VALUE
    {
    PWORD pwValue;
    DWORD dMode;
    union
        {
        DWORD dData;
        PWORD pwData;
        PVOID pData;
        };
    }
    W2K_VALUE, *PW2K_VALUE, **PPW2K_VALUE;

#define W2K_VALUE_ sizeof (W2K_VALUE)

// -----------------------------------------------------------------

typedef struct _W2K_PATH
    {
    HKEY  hk;
    HKEY  hkBase;
    PWORD pwParts;
    PWORD pwKey;
    PWORD pwValue;
    }
    W2K_PATH, *PW2K_PATH, **PPW2K_PATH;

#define W2K_PATH_ sizeof (W2K_PATH)

// -----------------------------------------------------------------

typedef struct _W2K_SYSTEM
    {
    DWORD dMajor;
    DWORD dMinor;
    DWORD dBuild;
    DWORD dSystem;
    WORD  awName    [N_NAME];
    WORD  awVersion [N_VERSION];
    WORD  awBuild   [N_BUILD];
    WORD  awLevel   [N_LEVEL];
    }
    W2K_SYSTEM, *PW2K_SYSTEM, **PPW2K_SYSTEM;

#define W2K_SYSTEM_ sizeof (W2K_SYSTEM)

// -----------------------------------------------------------------

typedef struct _W2K_SERVICES
    {
    DWORD               dEntries;     // number of entries in aess[]
    DWORD               dBytes;       // overall number of bytes
    DWORD               dDisplayName; // maximum display name length
    DWORD               dServiceName; // maximum service name length
    ENUM_SERVICE_STATUS aess [];      // service/driver status array
    }
    W2K_SERVICES, *PW2K_SERVICES, **PPW2K_SERVICES;

#define W2K_SERVICES_ sizeof (W2K_SERVICES)

#define W2K_SERVICES__(_n) \
        (W2K_SERVICES_ + ((_n) * ENUM_SERVICE_STATUS_))

// -----------------------------------------------------------------

typedef struct _W2K_PLACEMENT
    {
    DWORD dMode;
    INT   iLeft;
    INT   iTop;
    INT   iWidth;
    INT   iHeight;
    }
    W2K_PLACEMENT, *PW2K_PLACEMENT, **PPW2K_PLACEMENT;

#define W2K_PLACEMENT_ sizeof (W2K_PLACEMENT)

// -----------------------------------------------------------------

typedef struct _W2K_SETTINGS
    {
    LOGFONT       lf;
    COLORREF      crForeground;
    COLORREF      crBackground;
    W2K_PLACEMENT wp;
    }
    W2K_SETTINGS, *PW2K_SETTINGS, **PPW2K_SETTINGS;

#define W2K_SETTINGS_ sizeof (W2K_SETTINGS)

// =================================================================
// VERSION INFO STRUCTURES
// =================================================================

#define VS_HEADER(_n)   \
struct _VS_HEADER_##_n  \
    {                   \
    WORD wLength;       \
    WORD wValueLength;  \
    WORD wType;         \
    WORD awKey [_n];    \
    }

typedef VS_HEADER ( 0) VS_HEADER,    *PVS_HEADER;
typedef VS_HEADER ( 1) VS_HEADER_01, *PVS_HEADER_01;
typedef VS_HEADER ( 9) VS_HEADER_09, *PVS_HEADER_09;
typedef VS_HEADER (13) VS_HEADER_13, *PVS_HEADER_13;
typedef VS_HEADER (15) VS_HEADER_15, *PVS_HEADER_15;
typedef VS_HEADER (17) VS_HEADER_17, *PVS_HEADER_17;

// -----------------------------------------------------------------

typedef struct _VS_STRING
    {
    VS_HEADER_01     Header;    // key and value
    WORD             awData [];
    }
    VS_STRING, *PVS_STRING, **PPVS_STRING;

#define VS_STRING_ sizeof (VS_STRING)

// -----------------------------------------------------------------

typedef struct _VS_STRINGTABLE
    {
    VS_HEADER_09     Header;    // language and code page
    VS_STRING        String;
    }
    VS_STRINGTABLE, *PVS_STRINGTABLE, **PPVS_STRINGTABLE;

#define VS_STRINGTABLE_ sizeof (VS_STRINGTABLE)

// -----------------------------------------------------------------

typedef struct _VS_STRINGFILEINFO
    {
    VS_HEADER_15     Header;    // "StringFileInfo"
    VS_STRINGTABLE   StringTable;
    }
    VS_STRINGFILEINFO, *PVS_STRINGFILEINFO, **PPVS_STRINGFILEINFO;

#define VS_STRINGFILEINFO_ sizeof (VS_STRINGFILEINFO)

// -----------------------------------------------------------------

typedef struct _VS_TRANSLATION
    {
    WORD wLanguage;
    WORD wCodePage;
    }
    VS_TRANSLATION, *PVS_TRANSLATION, **PPVS_TRANSLATION;

#define VS_TRANSLATION_ sizeof (VS_TRANSLATION)

// -----------------------------------------------------------------

typedef struct _VS_VAR
    {
    VS_HEADER_13     Header;    // "Translation" + padding
    VS_TRANSLATION   Translation [];
    }
    VS_VAR, *PVS_VAR, **PPVS_VAR;

#define VS_VAR_ sizeof (VS_VAR)

// -----------------------------------------------------------------

typedef struct _VS_VARFILEINFO
    {
    VS_HEADER_13     Header;    // "VarFileInfo" + padding
    VS_VAR           Var;
    }
    VS_VARFILEINFO, *PVS_VARFILEINFO, **PPVS_VARFILEINFO;

#define VS_VARFILEINFO_ sizeof (VS_VARFILEINFO)

// -----------------------------------------------------------------

typedef struct _VS_VERSIONINFO
    {
    VS_HEADER_17     Header;    // "VS_VERSION_INFO" + padding
    VS_FIXEDFILEINFO FixedFileInfo;
    BYTE             abData [];
    }
    VS_VERSIONINFO, *PVS_VERSIONINFO, **PPVS_VERSIONINFO;

#define VS_VERSIONINFO_ sizeof (VS_VERSIONINFO)

// -----------------------------------------------------------------

typedef struct _VS_CODE_PAGE
    {
    DWORD dId;
    PWORD pwName;
    }
    VS_CODE_PAGE, *PVS_CODE_PAGE, **PPVS_CODE_PAGE;

#define VS_CODE_PAGE_ sizeof (VS_CODE_PAGE)

// -----------------------------------------------------------------

typedef struct _VS_VERSIONQUAD
    {
    union
        {
        struct
            {
            QWORD qVersion;
            };
        struct
            {
            DWORD dVersionL;
            DWORD dVersionH;
            };
        struct
            {
            WORD wVersionLL;
            WORD wVersionL;
            WORD wVersionH;
            WORD wVersionHH;
            };
        };
    }
    VS_VERSIONQUAD, *PVS_VERSIONQUAD, **PPVS_VERSIONQUAD;

#define VS_VERSIONQUAD_ sizeof (VS_VERSIONQUAD)

// -----------------------------------------------------------------

typedef struct _VS_VERSIONDATA
    {
    VS_VERSIONQUAD vvqFile;
    VS_VERSIONQUAD vvqProduct;
    DWORD          dTableIndex;
    DWORD          dTableCount;
    VS_TRANSLATION Translation;
    WORD           awLanguage         [N_STRING];
    WORD           awCodePage         [N_STRING];
    WORD           awCompanyName      [N_STRING];
    WORD           awFileDescription  [N_STRING];
    WORD           awFileVersion      [N_STRING];
    WORD           awInternalName     [N_STRING];
    WORD           awLegalCopyright   [N_STRING];
    WORD           awOriginalFilename [N_STRING];
    WORD           awProductName      [N_STRING];
    WORD           awProductVersion   [N_STRING];
    WORD           awComments         [N_STRING];
    WORD           awLegalTrademarks  [N_STRING];
    WORD           awPrivateBuild     [N_STRING];
    WORD           awSpecialBuild     [N_STRING];
    }
    VS_VERSIONDATA, *PVS_VERSIONDATA, **PPVS_VERSIONDATA;

#define VS_VERSIONDATA_ sizeof (VS_VERSIONDATA)

// =================================================================
// SPECIAL API PROTOTYPES
// =================================================================

DWORD WINAPI _vsprintf (PWORD pwBuffer,
                        PWORD pwFormat,
                        PVOID pArguments);

INT WINAPI _vmprintf (HWND  hWnd,
                      UINT  uiType,
                      PWORD pwCaption,
                      PWORD pwFormat,
                      PVOID pArguments);

DWORD WINAPI _vfprintf (HANDLE hFile,
                        PWORD  pwFormat,
                        PVOID  pArguments);

DWORD WINAPI _veprintf (PWORD pwFormat,
                        PVOID pArguments);

DWORD WINAPI _vprintf (PWORD pwFormat,
                       PVOID pArguments);

// -----------------------------------------------------------------

DWORD WINAPI _sprintf (PWORD pwBuffer,
                       PWORD pwFormat,
                       ...);

INT WINAPI _mprintf (HWND  hWnd,
                     UINT  uiType,
                     PWORD pwCaption,
                     PWORD pwFormat,
                     ...);

DWORD WINAPI _fprintf (HANDLE hFile,
                       PWORD  pwFormat,
                       ...);

DWORD WINAPI _eprintf (PWORD pwFormat,
                      ...);

DWORD WINAPI _printf (PWORD pwFormat,
                      ...);

// -----------------------------------------------------------------

PWORD WINAPI _strcpy (PWORD pwTo,
                      PWORD pwFrom,
                      PWORD pwDefault);

PWORD WINAPI _strcpyn (PWORD pwTo,
                       PWORD pwFrom,
                       PWORD pwDefault,
                       DWORD dTo);

// =================================================================
// API PROTOTYPES
// =================================================================

DWORD WINAPI w2kLibVersion (void);

DWORD WINAPI w2kLibTest (DWORD dVersion);

PBYTE WINAPI w2kMemoryAnsi (DWORD dSize);

PWORD WINAPI w2kMemoryUnicode (DWORD dSize);

PVOID WINAPI w2kMemoryCreate (DWORD dSize);

PVOID WINAPI w2kMemoryCreateEx (DWORD dSize,
                                DWORD dTag);

PW2K_MEMORY WINAPI w2kMemoryBase (PVOID pData);

PW2K_MEMORY WINAPI w2kMemoryBaseEx (PVOID pData,
                                    DWORD dTag);

PVOID WINAPI w2kMemoryResize (PVOID pData,
                              DWORD dSize,
                              PBOOL pfOk);

PVOID WINAPI w2kMemoryResizeEx (PVOID pData,
                                DWORD dSize,
                                PBOOL pfOk,
                                DWORD dTag);

PVOID WINAPI w2kMemoryDestroy (PVOID pData);

PVOID WINAPI w2kMemoryDestroyEx (PVOID pData,
                                 DWORD dTag);

void WINAPI w2kMemoryReset (void);

void WINAPI w2kMemoryTrack (DWORD dSize,
                            BOOL  fAdd);

BOOL WINAPI w2kMemoryStatus (PDWORD pdMemoryNow,
                             PDWORD pdMemoryMax);

PVOID WINAPI w2kMemoryCopy (PVOID pTarget,
                            PVOID pSource,
                            DWORD dBytes);

PVOID WINAPI w2kMemoryFill (PVOID pTarget,
                            DWORD dBytes,
                            BYTE  bData);

PVOID WINAPI w2kMemoryZero (PVOID pTarget,
                            DWORD dBytes);

BOOL WINAPI w2kStringFilter (PWORD pwFilter,
                             PWORD pwData,
                             BOOL  fIgnoreCase);

PW2K_POOL WINAPI w2kPoolCreate (DWORD dCount);

PW2K_POOL WINAPI w2kPoolDestroy (PW2K_POOL  pwp,
                                 W2K_NOTIFY Notify,
                                 PDWORD     pdErrors);

BOOL WINAPI w2kPoolLock (PW2K_POOL pwp);

BOOL WINAPI w2kPoolUnlock (PW2K_POOL pwp);

BOOL WINAPI w2kPoolAdd (PW2K_POOL  pwp,
                        W2K_NOTIFY Notify,
                        PWORD      pwName,
                        PVOID      pData,
                        BOOL       fUnique);

DWORD WINAPI w2kPoolRemove (PW2K_POOL  pwp,
                            W2K_NOTIFY Notify,
                            PWORD      pwName);

DWORD WINAPI w2kPoolEnumerate (PW2K_POOL  pwp,
                               W2K_NOTIFY Notify,
                               PWORD      pwName);

BOOL WINAPI w2kConsoleTest (void);

BOOL WINAPI w2kConsoleOpen (void);

BOOL WINAPI w2kConsoleClose (void);

DWORD WINAPI w2kFormatAnsi (PWORD pwBuffer,
                            DWORD dOffset,
                            PBYTE pbData,
                            DWORD dData,
                            DWORD dWidth,
                            DWORD dPrecision,
                            WORD  wFill,
                            BOOL  fRight,
                            BOOL  fZero);

DWORD WINAPI w2kFormatUnicode (PWORD pwBuffer,
                               DWORD dOffset,
                               PWORD pwData,
                               DWORD dData,
                               DWORD dWidth,
                               DWORD dPrecision,
                               WORD  wFill,
                               BOOL  fRight,
                               BOOL  fZero);

DWORD WINAPI w2kFormatDecimal (PWORD pwBuffer,
                               DWORD dOffset,
                               DWORD dData,
                               DWORD dWidth,
                               DWORD dPrecision,
                               WORD  wFill,
                               BOOL  fRight,
                               BOOL  fZero,
                               BOOL  fPrefix,
                               BOOL  fSigned);

DWORD WINAPI w2kFormatHex (PWORD pwBuffer,
                           DWORD dOffset,
                           DWORD dData,
                           DWORD dWidth,
                           DWORD dPrecision,
                           WORD  wFill,
                           BOOL  fRight,
                           BOOL  fZero,
                           BOOL  fPrefix,
                           BOOL  fLower);

DWORD WINAPI w2kFormatSingle (PWORD  pwBuffer,
                              DWORD  dOffset,
                              PWORD  pwFormat,
                              PDWORD pdFormat,
                              PVOID *ppData,
                              WORD   wFill,
                              BOOL   fUnicode);

DWORD WINAPI w2kFormatMulti (PWORD pwBuffer,
                             DWORD dOffset,
                             PWORD pwFormat,
                             PVOID pArguments,
                             WORD  wFill,
                             BOOL  fUnicode);

PBYTE WINAPI w2kFormatW2A (PWORD pwFrom,
                           PBYTE pbTo);

DWORD WINAPI w2kCrc32Start (PDWORD pdCrc32);

BOOL WINAPI w2kCrc32Stop (PDWORD pdCrc32);

DWORD WINAPI w2kCrc32Byte (PDWORD pdCrc32,
                           BYTE   bData);

DWORD WINAPI w2kCrc32Word (PDWORD pdCrc32,
                           WORD   wData);

DWORD WINAPI w2kCrc32Dword (PDWORD pdCrc32,
                            DWORD  dData);

DWORD WINAPI w2kCrc32Multi (PDWORD pdCrc32,
                            PVOID  pData,
                            DWORD  dData);

DWORD WINAPI w2kCrc32Text (PDWORD pdCrc32,
                           PWORD  pwData,
                           DWORD  dData);

DWORD WINAPI w2kRandomInitialize (PDWORD pdData,
                                  PDWORD pdAddress,
                                  DWORD  dSeed);

BYTE WINAPI w2kRandomByte (PDWORD pdData,
                           PDWORD pdAddress);

WORD WINAPI w2kRandomWord (PDWORD pdData,
                           PDWORD pdAddress);

DWORD WINAPI w2kRandomDword (PDWORD pdData,
                             PDWORD pdAddress);

PWORD WINAPI w2kEnvironmentString (PWORD pwStrings,
                                   PWORD pwName);

DWORD WINAPI w2kPathNormalize (PWORD pwPath);

PWORD WINAPI w2kPathEvaluate (PWORD  pwPath,
                              PDWORD pdData);

DWORD WINAPI w2kPathRoot (PWORD pwPath);

DWORD WINAPI w2kPathName (PWORD  pwPath,
                          PDWORD pdExtension);

DWORD WINAPI w2kFilePath (HINSTANCE hInstance,
                          PWORD     pwFile,
                          PWORD     pwBuffer,
                          DWORD     dBuffer);

HANDLE WINAPI w2kFileOpen (PWORD pwPath,
                           BOOL  fWrite);

HANDLE WINAPI w2kFileNew (PWORD pwPath);

HANDLE WINAPI w2kFileClose (HANDLE hf);

BOOL WINAPI w2kFileTest (PWORD pwPath);

PVOID WINAPI w2kFileLoad (PWORD     pwPath,
                          PDWORD    pdSize,
                          PFILETIME pft);

BOOL WINAPI w2kFileSave (PWORD pwPath,
                         PVOID pData,
                         DWORD dData);

BOOL WINAPI w2kFileAppend (PWORD pwPath,
                           PVOID pData,
                           DWORD dData);

BOOL WINAPI w2kFileWrite (HANDLE hf,
                          PVOID  pData,
                          DWORD  dData);

BOOL WINAPI w2kRegistryRoot (PWORD pwRoot,
                             PHKEY phk);

BOOL WINAPI w2kRegistryParse (PWORD  pwPath,
                              PHKEY  phk,
                              PPWORD ppwKey,
                              PPWORD ppwValue);

PWORD WINAPI w2kRegistryParseEx (PWORD  pwPath,
                                 PHKEY  phk,
                                 PPWORD ppwKey,
                                 PPWORD ppwValue);

HKEY WINAPI w2kRegistryClose (HKEY hk);

HKEY WINAPI w2kRegistryOpen (HKEY  hkBase,
                             PWORD pwKey);

HKEY WINAPI w2kRegistryOpenEx (HKEY  hkBase,
                               DWORD dKeys,
                               PWORD pwKey,
                               ...);

HKEY WINAPI w2kRegistryCreate (HKEY  hkBase,
                               PWORD pwKey);

HKEY WINAPI w2kRegistryCreateEx (HKEY  hkBase,
                                 DWORD dKeys,
                                 PWORD pwKey,
                                 ...);

PW2K_PATH WINAPI w2kRegistryPath (PWORD pwPath,
                                  BOOL  fCreate);

PW2K_PATH WINAPI w2kRegistryPathOpen (PWORD pwPath);

PW2K_PATH WINAPI w2kRegistryPathCreate (PWORD pwPath);

PW2K_PATH WINAPI w2kRegistryPathClose (PW2K_PATH pwp);

PVOID WINAPI w2kRegistryPathRead (PWORD  pwPath,
                                  DWORD  dMode,
                                  PDWORD pdSize);

BOOL WINAPI w2kRegistryPathWrite (PWORD pwPath,
                                  DWORD dMode,
                                  PVOID pData);

PVOID WINAPI w2kRegistryData (HKEY   hk,
                              PWORD  pwValue,
                              PDWORD pdType,
                              PDWORD pdSize);

PVOID WINAPI w2kRegistryRead (HKEY   hk,
                              PWORD  pwValue,
                              DWORD  dMode,
                              PDWORD pdSize);

BOOL WINAPI w2kRegistryReadValue (HKEY       hk,
                                  PW2K_VALUE pwv);

BOOL WINAPI w2kRegistryReadDword (HKEY   hk,
                                  PWORD  pwValue,
                                  PDWORD pdData);

PWORD WINAPI w2kRegistryReadText (HKEY   hk,
                                  PWORD  pwValue,
                                  PDWORD pdSize);

PWORD WINAPI w2kRegistryReadModule (HKEY   hk,
                                    PWORD  pwValue,
                                    PDWORD pdSize);

PVOID WINAPI w2kRegistryReadBinary (HKEY  hk,
                                    PWORD pwValue,
                                    DWORD dSize);

BOOL WINAPI w2kRegistryWrite (HKEY  hk,
                              PWORD pwValue,
                              DWORD dMode,
                              PVOID pData);

BOOL WINAPI w2kRegistryWriteValue (HKEY       hk,
                                   PW2K_VALUE pwv);

BOOL WINAPI w2kRegistryWriteDword (HKEY  hk,
                                   PWORD pwValue,
                                   DWORD dData);

BOOL WINAPI w2kRegistryWriteText (HKEY  hk,
                                  PWORD pwValue,
                                  PWORD pwData);

BOOL WINAPI w2kRegistryWriteBinary (HKEY  hk,
                                    PWORD pwValue,
                                    PVOID pData,
                                    DWORD dSize);

HKEY WINAPI w2kApplicationKey (PWORD pwCompany,
                               PWORD pwId,
                               PWORD pwVersion,
                               PWORD pwKey,
                               BOOL  fUser,
                               BOOL  fCreate);

HKEY WINAPI w2kApplicationKeyOpen (PWORD pwCompany,
                                   PWORD pwId,
                                   PWORD pwVersion,
                                   PWORD pwKey,
                                   BOOL  fUser);

HKEY WINAPI w2kApplicationKeyCreate (PWORD pwCompany,
                                     PWORD pwId,
                                     PWORD pwVersion,
                                     PWORD pwKey,
                                     BOOL  fUser);

PVOID WINAPI w2kApplicationLoad (PWORD  pwCompany,
                                 PWORD  pwId,
                                 PWORD  pwVersion,
                                 PWORD  pwKey,
                                 BOOL   fUser,
                                 PWORD  pwValue,
                                 DWORD  dMode,
                                 PDWORD pdSize);

BOOL WINAPI w2kApplicationLoadEx (PWORD pwCompany,
                                  PWORD pwId,
                                  PWORD pwVersion,
                                  PWORD pwKey,
                                  BOOL  fUser,
                                  PWORD pwValue,
                                  ...);

BOOL WINAPI w2kApplicationSave (PWORD pwCompany,
                                PWORD pwId,
                                PWORD pwVersion,
                                PWORD pwKey,
                                BOOL  fUser,
                                PWORD pwValue,
                                DWORD dMode,
                                PVOID pData);

BOOL WINAPI w2kApplicationSaveEx (PWORD pwCompany,
                                  PWORD pwId,
                                  PWORD pwVersion,
                                  PWORD pwKey,
                                  BOOL  fUser,
                                  PWORD pwValue,
                                  ...);

BOOL WINAPI w2kApplicationCreate (PWORD pwCompany,
                                  PWORD pwId,
                                  PWORD pwVersion,
                                  BOOL  fUser,
                                  PWORD pwCompanyName,
                                  PWORD pwProductName,
                                  PWORD pwProductVersion,
                                  PWORD pwInternalName,
                                  PWORD pwOriginalFilename,
                                  PWORD pwFileVersion,
                                  PWORD pwFileDescription,
                                  PWORD pwLegalCopyright,
                                  PWORD pwLegalTrademarks,
                                  PWORD pwComments,
                                  PWORD pwPrivateBuild,
                                  PWORD pwSpecialBuild);

PW2K_SETTINGS WINAPI w2kApplicationSettings (PWORD pwCompany,
                                             PWORD pwId,
                                             PWORD pwVersion,
                                             PWORD pwKey,
                                             BOOL  fUser,
                                             PWORD pwValue);

PWORD WINAPI w2kClassText (PWORD  pwClsId,
                           PWORD  pwKey,
                           PWORD  pwValue,
                           PDWORD pdSize);

PWORD WINAPI w2kClassPath (PWORD  pwClsId,
                           PWORD  pwKey,
                           PWORD  pwValue,
                           PDWORD pdSize);

PWORD WINAPI w2kClassApplication (PWORD  pwClsId,
                                  PDWORD pdSize);

PWORD WINAPI w2kClassLibrary (PWORD  pwClsId,
                              PDWORD pdSize);

PWORD WINAPI w2kProgramClsId (PWORD  pwProgID,
                              PDWORD pdSize);

PWORD WINAPI w2kProgramApplication (PWORD  pwProgID,
                                    PDWORD pdSize);

PWORD WINAPI w2kProgramLibrary (PWORD  pwProgID,
                                PDWORD pdSize);

PW2K_SYSTEM WINAPI w2kVersionSystem (void);

QWORD WINAPI w2kVersionToken (PWORD  pwVersion,
                              PDWORD pdOffset);

QWORD WINAPI w2kVersionBinary (PWORD pwVersion,
                               BOOL  fFlushLeft);

PWORD WINAPI w2kVersionText (QWORD qVersion,
                             PWORD pwBuffer,
                             DWORD dBuffer);

QWORD WINAPI w2kVersionFile (PWORD pwPath);

QWORD WINAPI w2kVersionModule (PWORD pwPath);

QWORD WINAPI w2kVersionRegistryDword (PWORD pwPath);

QWORD WINAPI w2kVersionRegistryText (PWORD pwPath);

QWORD WINAPI w2kVersionComponent (PWORD pwPath);

QWORD WINAPI w2kVersionResource (PWORD pwPath);

QWORD WINAPI w2kVersionApplication (PWORD pwProgID);

QWORD WINAPI w2kVersionLibrary (PWORD pwProgID);

BOOL WINAPI w2kVersionVerify (PVS_VERSIONINFO pvvi);

PVS_VERSIONINFO WINAPI w2kVersionInfo (PWORD  pwPath,
                                       PDWORD pdSize);

DWORD WINAPI w2kVersionAlign (DWORD dSize);

PVS_STRINGFILEINFO WINAPI w2kVersionStrings (PVS_VERSIONINFO pvvi);

PVS_VARFILEINFO WINAPI w2kVersionVars (PVS_VERSIONINFO pvvi);

PVS_TRANSLATION WINAPI w2kVersionTranslation (PVS_VERSIONINFO pvvi,
                                              PDWORD       pdCount);

PVS_STRINGTABLE WINAPI w2kVersionTable (PVS_VERSIONINFO pvvi,
                                        PWORD           pwTable);

PVS_STRINGTABLE WINAPI w2kVersionTableEx (PVS_VERSIONINFO pvvi,
                                          DWORD           dIndex,
                                          PDWORD          pdCount);

PVS_STRING WINAPI w2kVersionTableString (PVS_STRINGTABLE pvst,
                                         PWORD           pwName);

PVS_STRING WINAPI w2kVersionString (PVS_VERSIONINFO pvvi,
                                    PWORD           pwName,
                                    PWORD           pwTable);

PVS_STRING WINAPI w2kVersionStringEx (PVS_VERSIONINFO pvvi,
                                      PWORD           pwName,
                                      DWORD           dIndex,
                                      PDWORD          pdCount);

PWORD WINAPI w2kVersionValue (PVS_STRING pvs);

BOOL WINAPI w2kVersionValueCopy (PVS_STRING pvs,
                                 PWORD      pwDefault,
                                 PWORD      pwBuffer,
                                 DWORD      dBuffer);

BOOL WINAPI w2kVersionLanguage (DWORD dId,
                                PWORD pwBuffer,
                                DWORD dBuffer);

PWORD WINAPI w2kVersionCodePage (DWORD dId);

PVS_VERSIONDATA WINAPI w2kVersionData (PWORD pwPath,
                                       DWORD dIndex);

INT WINAPI w2kVersionDisplay (HWND  hWnd,
                              UINT  uiType,
                              PWORD pwCaption,
                              PWORD pwPath,
                              DWORD dIndex);

SC_HANDLE WINAPI w2kServiceConnect (void);

SC_HANDLE WINAPI w2kServiceDisconnect (SC_HANDLE hManager);

SC_HANDLE WINAPI w2kServiceManager (SC_HANDLE  hManager,
                                    PSC_HANDLE phManager,
                                    BOOL       fOpen);

SC_HANDLE WINAPI w2kServiceOpen (SC_HANDLE hManager,
                                 PWORD     pwName);

BOOL WINAPI w2kServiceClose (SC_HANDLE hService);

BOOL WINAPI w2kServiceAdd (SC_HANDLE hManager,
                           PWORD     pwName,
                           PWORD     pwInfo,
                           PWORD     pwPath);

BOOL WINAPI w2kServiceRemove (SC_HANDLE hManager,
                              PWORD     pwName);

BOOL WINAPI w2kServiceStart (SC_HANDLE hManager,
                             PWORD     pwName);

BOOL WINAPI w2kServiceControl (SC_HANDLE hManager,
                               PWORD     pwName,
                               DWORD     dControl);

BOOL WINAPI w2kServiceStop (SC_HANDLE hManager,
                            PWORD     pwName);

BOOL WINAPI w2kServicePause (SC_HANDLE hManager,
                             PWORD     pwName);

BOOL WINAPI w2kServiceContinue (SC_HANDLE hManager,
                                PWORD     pwName);

SC_HANDLE WINAPI w2kServiceLoad (PWORD pwName,
                                 PWORD pwInfo,
                                 PWORD pwPath,
                                 BOOL  fStart);

SC_HANDLE WINAPI w2kServiceLoadEx (PWORD pwPath,
                                   BOOL  fStart);

BOOL WINAPI w2kServiceUnload (PWORD     pwName,
                              SC_HANDLE hManager);

BOOL WINAPI w2kServiceUnloadEx (PWORD     pwPath,
                                SC_HANDLE hManager);

PW2K_SERVICES WINAPI w2kServiceList (BOOL fDriver,
                                     BOOL fWin32,
                                     BOOL fActive,
                                     BOOL fInactive);

void WINAPI w2kChainLock (void);

void WINAPI w2kChainUnlock (void);

PVOID WINAPI w2kChainThis (PW2K_NODE pwn);

BOOL WINAPI w2kChainAdd (PW2K_NODE pwn,
                         PW2K_NODE pwnBack,
                         PVOID     pThis,
                         DWORD     dType);

BOOL WINAPI w2kChainRemove (PW2K_NODE pwn);

BOOL WINAPI w2kChainRemoveEx (PW2K_NODE pwn,
                              W2K_WALK  Handler,
                              PVOID     pData);

BOOL WINAPI w2kChainMove (PW2K_NODE pwn,
                          PW2K_NODE pwnBack);

PW2K_NODE WINAPI w2kChainBack (PW2K_NODE pwn);

PW2K_NODE WINAPI w2kChainNext (PW2K_NODE pwn);

PW2K_NODE WINAPI w2kChainFirst (PW2K_NODE pwn);

PW2K_NODE WINAPI w2kChainLast (PW2K_NODE pwn);

DWORD WINAPI w2kChainWalk (PW2K_NODE pwn,
                           BOOL      fComplete,
                           BOOL      fBack,
                           W2K_WALK  Handler,
                           PVOID     pData);

// =================================================================
// LINKER CONTROL
// =================================================================

#ifdef _W2K_LIB_DLL_

#pragma comment (linker, "/entry:\"DllMain\"")
#pragma comment (linker, "/defaultlib:version.lib")

#else

#pragma comment (linker, "/defaultlib:w2k_lib.lib")

#endif

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// END OF FILE
// =================================================================
