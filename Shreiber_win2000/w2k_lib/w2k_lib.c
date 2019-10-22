
// __________________________________________________________
//
//                         w2k_lib.c
//              SBS Windows 2000 Utility Library
//                02-12-2001 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#define  _W2K_LIB_DLL_
#include "w2k_lib.h"

// =================================================================
// DISCLAIMER
// =================================================================

/*

This software is provided "as is" and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantibility and fitness for a particular purpose are disclaimed.
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

02-12-2001 V1.00 Original version (SBS).

*/

// =================================================================
// GLOBAL VARIABLES
// =================================================================

HINSTANCE ghInstance     = NULL;

HANDLE    ghStdInput     = INVALID_HANDLE_VALUE;
HANDLE    ghStdOutput    = INVALID_HANDLE_VALUE;
HANDLE    ghStdError     = INVALID_HANDLE_VALUE;

BOOL      gfStdHandles   = FALSE;
BOOL      gfStdFailure   = FALSE;
BOOL      gfStdTransient = FALSE;

LONG      glMemorySign   = 0;
DWORD     gdMemoryNow    = 0;
DWORD     gdMemoryMax    = 0;

WORD      gwFill         = ' ';

// =================================================================
// GLOBAL STRINGS
// =================================================================

WORD awCaption             [] = L"Windows 2000 Application";

WORD awWindowsNT           [] = L"Windows NT";
WORD awWindows2000         [] = L"Windows 2000";
WORD awProfessional        [] = L" Professional";
WORD awDomainController    [] = L" Domain Controller";
WORD awServer              [] = L" Server";

WORD awHkeyClassesRoot     [] = L"HKEY_CLASSES_ROOT";
WORD awHkeyCurrentUser     [] = L"HKEY_CURRENT_USER";
WORD awHkeyLocalMachine    [] = L"HKEY_LOCAL_MACHINE";
WORD awHkeyUsers           [] = L"HKEY_USERS";
WORD awHkeyPerformanceData [] = L"HKEY_PERFORMANCE_DATA";
WORD awHkeyCurrentConfig   [] = L"HKEY_CURRENT_CONFIG";
WORD awHkeyDynData         [] = L"HKEY_DYN_DATA";

WORD awKeyClsId            [] = L"CLSID";
WORD awKeyLocalServer32    [] = L"LocalServer32";
WORD awKeyInprocServer32   [] = L"InprocServer32";

WORD awKeyMachineSoftware  [] = L"SOFTWARE";
WORD awKeyUserSoftware     [] = L"Software";
WORD awKeyCompany          [] = L"SBS";
WORD awKeyId               [] = L"- Common -";
WORD awKeyVersion          [] = L"CurrentVersion";

WORD awRoot                [] = L"\\";
WORD awNull                [] = L"";

// =================================================================
// CODE PAGE DIRECTORY
// =================================================================

VS_CODE_PAGE avcpCodePages [] =
    {
        0, L"Code Page Independent",
       37, L"EBCDIC",
      437, L"MS-DOS United States",
      500, L"EBCDIC \"500V1\"",
      708, L"Arabic (ASMO 708)",
      709, L"Arabic (ASMO 449+, BCON V4)",
      710, L"Arabic (Transparent Arabic)",
      720, L"Arabic (Transparent ASMO)",
      737, L"Greek (formerly 437G)",
      775, L"Baltic",
      850, L"MS-DOS Multilingual (Latin I)",
      852, L"MS-DOS Slavic (Latin II)",
      855, L"IBM Cyrillic (primarily Russian)",
      857, L"IBM Turkish",
      860, L"MS-DOS Portuguese",
      861, L"MS-DOS Icelandic",
      862, L"Hebrew",
      863, L"MS-DOS Canadian-French",
      864, L"Arabic",
      865, L"MS-DOS Nordic",
      866, L"MS-DOS Russian",
      869, L"IBM Modern Greek",
      874, L"Thai",
      875, L"EBCDIC",
      932, L"Japan",
      936, L"Chinese (PRC, Singapore)",
      949, L"Korean",
      950, L"Chinese (Taiwan, Hong Kong) ",
     1026, L"EBCDIC",
     1200, L"Unicode (BMP of ISO 10646)",
     1250, L"Windows 3.1 Eastern European ",
     1251, L"Windows 3.1 Cyrillic",
     1252, L"Windows 3.1 US (ANSI)",
     1253, L"Windows 3.1 Greek",
     1254, L"Windows 3.1 Turkish",
     1255, L"Hebrew",
     1256, L"Arabic",
     1257, L"Baltic",
     1258, L"Vietnamese",
     1361, L"Korean (Johab)",
    10000, L"Macintosh Roman",
    10001, L"Macintosh Japanese",
    10006, L"Macintosh Greek I",
    10007, L"Macintosh Cyrillic",
    10029, L"Macintosh Latin 2",
    10079, L"Macintosh Icelandic",
    10081, L"Macintosh Turkish",
    10082, L"Macintosh Croatian",
    20261, L"Teletex",
    20866, L"Cyrillic (KOI8-R)",
    28592, L"Central European (ISO)",
    28595, L"Cyrillic (ISO)",
    50000, L"User Defined",
       -1, NULL,
    };

// =================================================================
// CRC32 LOOKUP TABLE
// =================================================================

DWORD adCrc32 [256] =
    {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,   //0x00..0x03
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,   //0x04..0x07
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,   //0x08..0x0B
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,   //0x0C..0x0F
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,   //0x10..0x13
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,   //0x14..0x17
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,   //0x18..0x1B
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,   //0x1C..0x1F
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,   //0x20..0x23
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,   //0x24..0x27
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,   //0x28..0x2B
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,   //0x2C..0x2F
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,   //0x30..0x33
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,   //0x34..0x37
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,   //0x38..0x3B
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,   //0x3C..0x3F
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,   //0x40..0x43
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,   //0x44..0x47
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,   //0x48..0x4B
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,   //0x4C..0x4F
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,   //0x50..0x53
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,   //0x54..0x57
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,   //0x58..0x5B
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,   //0x5C..0x5F
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,   //0x60..0x63
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,   //0x64..0x67
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,   //0x68..0x6B
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,   //0x6C..0x6F
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,   //0x70..0x73
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,   //0x74..0x77
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,   //0x78..0x7B
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,   //0x7C..0x7F
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,   //0x80..0x83
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,   //0x84..0x87
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,   //0x88..0x8B
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,   //0x8C..0x8F
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,   //0x90..0x93
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,   //0x94..0x97
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,   //0x98..0x9B
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,   //0x9C..0x9F
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,   //0xA0..0xA3
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,   //0xA4..0xA7
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,   //0xA8..0xAB
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,   //0xAC..0xAF
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,   //0xB0..0xB3
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,   //0xB4..0xB7
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,   //0xB8..0xBB
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,   //0xBC..0xBF
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,   //0xC0..0xC3
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,   //0xC4..0xC7
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,   //0xC8..0xCB
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,   //0xCC..0xCF
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,   //0xD0..0xD3
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,   //0xD4..0xD7
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,   //0xD8..0xDB
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,   //0xDC..0xDF
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,   //0xE0..0xE3
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,   //0xE4..0xE7
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,   //0xE8..0xEB
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,   //0xEC..0xEF
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,   //0xF0..0xF3
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,   //0xF4..0xF7
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,   //0xF8..0xFB
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D    //0xFC..0xFF
    };

// =================================================================
// SYNCHRONIZATION OBJECTS
// =================================================================

CRITICAL_SECTION gcsMemory;
CRITICAL_SECTION gcsChain;

// =================================================================
// VERSION TESTING
// =================================================================

DWORD WINAPI w2kLibVersion (void)
    {
    return W2K_LIB_VERSION;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kLibTest (DWORD dVersion)
    {
    return (dVersion <= W2K_LIB_VERSION ? 0 : W2K_LIB_VERSION);
    }

// =================================================================
// MEMORY MANAGEMENT
// =================================================================

PBYTE WINAPI w2kMemoryAnsi (DWORD dSize)
    {
    return w2kMemoryCreate (dSize);
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kMemoryUnicode (DWORD dSize)
    {
    return w2kMemoryCreate (dSize * WORD_);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryCreate (DWORD dSize)
    {
    return w2kMemoryCreateEx (dSize, W2K_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryCreateEx (DWORD dSize,
                                DWORD dTag)
    {
    PW2K_MEMORY pwm = NULL;

    if ((W2K_MEMORY_ + dSize >= W2K_MEMORY_)
        &&
        ((pwm = LocalAlloc (LMEM_FIXED, W2K_MEMORY_ + dSize))
         != NULL))
        {
        pwm->dTag  = dTag;
        pwm->dSize = dSize;

        w2kMemoryTrack (pwm->dSize, TRUE);
        }
    return (pwm != NULL ? pwm->abData : NULL);
    }

// -----------------------------------------------------------------

PW2K_MEMORY WINAPI w2kMemoryBase (PVOID pData)
    {
    return w2kMemoryBaseEx (pData, W2K_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PW2K_MEMORY WINAPI w2kMemoryBaseEx (PVOID pData,
                                    DWORD dTag)
    {
    PW2K_MEMORY pwm = (PW2K_MEMORY) ((PBYTE) pData - W2K_MEMORY_);

    if ((pData       == NULL    ) ||
        (pwm         == NULL    ) ||
        ((PVOID) pwm >= pData   ) ||
        (pwm->dTag   != dTag    ) ||
        (pwm->dSize  == MAXDWORD))
        {
        pwm = NULL;
        }
    return pwm;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryResize (PVOID pData,
                              DWORD dSize,
                              PBOOL pfOk)
    {
    return w2kMemoryResizeEx (pData, dSize, pfOk, W2K_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryResizeEx (PVOID pData,
                                DWORD dSize,
                                PBOOL pfOk,
                                DWORD dTag)
    {
    PW2K_MEMORY pwmNew;
    BOOL        fOk = FALSE;
    PW2K_MEMORY pwm = w2kMemoryBaseEx (pData, dTag);

    if (pwm != NULL)
        {
        fOk = TRUE;

        if (pwm->dSize != dSize)
            {
            if ((pwmNew = LocalReAlloc (pwm, W2K_MEMORY_ + dSize,
                                        LMEM_MOVEABLE))
                != NULL)
                {
                pwm = pwmNew;

                if (dSize >= pwm->dSize)
                    {
                    w2kMemoryTrack (dSize - pwm->dSize, TRUE);
                    }
                else
                    {
                    w2kMemoryTrack (pwm->dSize - dSize, FALSE);
                    }
                pwm->dSize = dSize;
                }
            else
                {
                if (pfOk != NULL)
                    {
                    fOk = FALSE;
                    }
                else
                    {
                    w2kMemoryTrack (pwm->dSize, FALSE);

                    pwm->dSize = MAXDWORD;
                    LocalFree (pwm);

                    pwm = NULL;
                    }
                }
            }
        }
    if (pfOk != NULL) *pfOk = fOk;
    return (pwm != NULL ? pwm->abData : NULL);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryDestroy (PVOID pData)
    {
    return w2kMemoryDestroyEx (pData, W2K_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryDestroyEx (PVOID pData,
                                 DWORD dTag)
    {
    PW2K_MEMORY pwm;

    if ((pwm = w2kMemoryBaseEx (pData, dTag)) != NULL)
        {
        w2kMemoryTrack (pwm->dSize, FALSE);

        pwm->dSize = MAXDWORD;
        LocalFree (pwm);
        }
    return NULL;
    }

// -----------------------------------------------------------------

void WINAPI w2kMemoryReset (void)
    {
    EnterCriticalSection (&gcsMemory);

    glMemorySign = 0;
    gdMemoryNow  = 0;
    gdMemoryMax  = 0;

    LeaveCriticalSection (&gcsMemory);
    return;
    }

// -----------------------------------------------------------------

void WINAPI w2kMemoryTrack (DWORD dSize,
                            BOOL  fAdd)
    {
    if (dSize)
        {
        EnterCriticalSection (&gcsMemory);

        if (fAdd)
            {
            if (gdMemoryNow + dSize < gdMemoryNow)
                {
                glMemorySign++;
                }
            gdMemoryNow += dSize;

            if (!glMemorySign)
                {
                gdMemoryMax = max (gdMemoryMax, gdMemoryNow);
                }
            }
        else
            {
            if (gdMemoryNow < dSize)
                {
                glMemorySign--;
                }
            gdMemoryNow -= dSize;
            }
        LeaveCriticalSection (&gcsMemory);
        }
    return;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kMemoryStatus (PDWORD pdMemoryNow,
                             PDWORD pdMemoryMax)
    {
    BOOL fMemorySign = FALSE;

    EnterCriticalSection (&gcsMemory);

    fMemorySign = (glMemorySign < 0);

    if (pdMemoryNow != NULL) *pdMemoryNow = gdMemoryNow;
    if (pdMemoryMax != NULL) *pdMemoryMax = gdMemoryMax;

    LeaveCriticalSection (&gcsMemory);
    return fMemorySign;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryCopy (PVOID pTarget,
                            PVOID pSource,
                            DWORD dBytes)
    {
    DWORD n = 0;

    if ((pTarget != NULL) && (pSource != NULL) && dBytes)
        {
        CopyMemory (pTarget, pSource, n = dBytes);
        }
    return (PBYTE) pTarget + n;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryFill (PVOID pTarget,
                            DWORD dBytes,
                            BYTE  bData)
    {
    DWORD n = 0;

    if ((pTarget != NULL) && dBytes)
        {
        FillMemory (pTarget, n = dBytes, bData);
        }
    return (PBYTE) pTarget + n;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kMemoryZero (PVOID pTarget,
                            DWORD dBytes)
    {
    DWORD n = 0;

    if ((pTarget != NULL) && dBytes)
        {
        ZeroMemory (pTarget, n = dBytes);
        }
    return (PBYTE) pTarget + n;
    }

// =================================================================
// STRING MANAGEMENT
// =================================================================

BOOL WINAPI w2kStringFilter (PWORD pwFilter,
                             PWORD pwData,
                             BOOL  fIgnoreCase)
    {
    DWORD i = 0;
    DWORD j = 0;

    if (pwData   == NULL) return FALSE;
    if (pwFilter == NULL) return TRUE;

    while (pwFilter [i] && pwData [j])
        {
        if (pwFilter [i] != '?')
            {
            if (pwFilter [i] == '*')
                {
                i++;

                if ((pwFilter [i] != '*') &&
                    (pwFilter [i] != '?'))
                    {
                    if (pwFilter [i])
                        {
                        while (pwData [j]
                               &&
                               (!w2kStringFilter (pwFilter + i,
                                                  pwData   + j,
                                                  fIgnoreCase)))
                            {
                            j++;
                            }
                        }
                    return pwData [j] != 0;
                    }
                }
            if (fIgnoreCase
                ? LCASE (pwFilter [i]) != LCASE (pwData [j])
                :        pwFilter [i]  !=        pwData [j] )
                {
                return FALSE;
                }
            }
        i++;
        j++;
        }
    if (pwFilter [i] == '*') i++;
    return !(pwFilter [i] || pwData [j]);
    }

// =================================================================
// POOL MANAGEMENT
// =================================================================

PW2K_POOL WINAPI w2kPoolCreate (DWORD dCount)
    {
    DWORD     i;
    PW2K_POOL pwp = NULL;

    if ((pwp = w2kMemoryCreate (W2K_POOL__ (dCount))) != NULL)
        {
        InitializeCriticalSection (&pwp->cs);
        GetLocalTime              (&pwp->stCreate);

        pwp->stUpdate = pwp->stCreate;
        pwp->stAdd    = pwp->stCreate;
        pwp->stRemove = pwp->stCreate;
        pwp->dCount   = dCount;

        for (i = 0; i < dCount; i++)
            {
            pwp->awe [i].awName [0] = 0;
            pwp->awe [i].pData      = NULL;
            }
        }
    return pwp;
    }

// -----------------------------------------------------------------

PW2K_POOL WINAPI w2kPoolDestroy (PW2K_POOL  pwp,
                                 W2K_NOTIFY Notify,
                                 PDWORD     pdErrors)
    {
    PW2K_ENTRY pwe;
    DWORD      i;
    DWORD      dErrors = 0;

    if (pwp != NULL)
        {
        for (i = 0; i < pwp->dCount; i++)
            {
            pwe = pwp->awe + i;

            if (pwe->awName [0])
                {
                if (Notify != NULL)
                    {
                    if (!Notify (pwp, W2K_NOTIFY_DESTROY,
                                 pwe->awName, pwe->pData))
                        {
                        dErrors++;
                        }
                    }
                else
                    {
                    w2kMemoryDestroy (pwe->pData);
                    }
                }
            }
        DeleteCriticalSection (&pwp->cs);
        w2kMemoryDestroy (pwp);
        }
    if (pdErrors != NULL) *pdErrors = dErrors;
    return NULL;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kPoolLock (PW2K_POOL pwp)
    {
    BOOL fOk = FALSE;

    if (pwp != NULL)
        {
        EnterCriticalSection (&pwp->cs);
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kPoolUnlock (PW2K_POOL pwp)
    {
    BOOL fOk = FALSE;

    if (pwp != NULL)
        {
        LeaveCriticalSection (&pwp->cs);
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kPoolAdd (PW2K_POOL  pwp,
                        W2K_NOTIFY Notify,
                        PWORD      pwName,
                        PVOID      pData,
                        BOOL       fUnique)
    {
    PW2K_ENTRY pwe;
    DWORD      i;
    BOOL       fOk = FALSE;

    if ((pwName != NULL) && pwName [0] &&
        (lstrlen (pwName) < N_NAME))
        {
        if (w2kPoolLock (pwp))
            {
            for (pwe = NULL, i = 0; i < pwp->dCount; i++)
                {
                if (pwp->awe [i].awName [0])
                    {
                    if (fUnique &&
                        (!lstrcmp (pwp->awe [i].awName, pwName)))
                        {
                        break;
                        }
                    }
                else
                    {
                    if (pwe == NULL)
                        {
                        pwe = pwp->awe + i;
                        if (!fUnique) break;
                        }
                    }
                }
            if ((pwe != NULL) && ((!fUnique) || (i == pwp->dCount)))
                {
                if ((Notify == NULL)
                    ||
                    Notify (pwp, W2K_NOTIFY_ADD,
                            pwe->awName, pwe->pData))
                    {
                    GetLocalTime (&pwp->stUpdate);
                    pwp->stAdd = pwp->stUpdate;

                    lstrcpy (pwe->awName, pwName);
                    pwe->pData = pData;

                    fOk = TRUE;
                    }
                }
            w2kPoolUnlock (pwp);
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kPoolRemove (PW2K_POOL  pwp,
                            W2K_NOTIFY Notify,
                            PWORD      pwName)
    {
    PW2K_ENTRY pwe;
    DWORD      i;
    DWORD      dCount = 0;

    if (w2kPoolLock (pwp))
        {
        for (i = 0; i < pwp->dCount; i++)
            {
            if ((pwp->awe [i].awName [0])
                &&
                ((pwName == NULL) || (!pwName [0]) ||
                 (!lstrcmp (pwp->awe [i].awName, pwName))))
                {
                pwe = pwp->awe + i;

                if ((Notify == NULL)
                    ||
                    Notify (pwp, W2K_NOTIFY_REMOVE,
                            pwe->awName, pwe->pData))
                    {
                    if (Notify == NULL)
                        {
                        w2kMemoryDestroy (pwe->pData);
                        }
                    GetLocalTime (&pwp->stUpdate);
                    pwp->stRemove = pwp->stUpdate;

                    pwp->awe [i].awName [0] = 0;
                    pwp->awe [i].pData      = NULL;

                    dCount++;
                    }
                }
            }
        w2kPoolUnlock (pwp);
        }
    return dCount;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kPoolEnumerate (PW2K_POOL  pwp,
                               W2K_NOTIFY Notify,
                               PWORD      pwName)
    {
    PW2K_ENTRY pwe;
    DWORD      i;
    DWORD      dCount = 0;

    if (w2kPoolLock (pwp))
        {
        for (i = 0; i < pwp->dCount; i++)
            {
            if ((pwp->awe [i].awName [0])
                &&
                ((pwName == NULL) || (!pwName [0]) ||
                 (!lstrcmp (pwp->awe [i].awName, pwName))))
                {
                pwe = pwp->awe + i;

                if ((Notify == NULL)
                    ||
                    Notify (pwp, W2K_NOTIFY_ENUMERATE,
                            pwe->awName, pwe->pData))
                    {
                    dCount++;
                    }
                }
            }
        w2kPoolUnlock (pwp);
        }
    return dCount;
    }

// =================================================================
// CONSOLE MANAGEMENT
// =================================================================

BOOL WINAPI w2kConsoleTest (void)
    {
    WORD awTitle [N_CONSOLE];

    return GetConsoleTitle (awTitle, N_CONSOLE) != 0;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kConsoleOpen (void)
    {
    if ((!gfStdHandles) && (!gfStdFailure))
        {
        gfStdTransient = (!w2kConsoleTest ()) && AllocConsole ();

        if (w2kConsoleTest ())
            {
            ghStdInput  = GetStdHandle (STD_INPUT_HANDLE);
            ghStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
            ghStdError  = GetStdHandle (STD_ERROR_HANDLE);

            if ((ghStdInput  != INVALID_HANDLE_VALUE) &&
                (ghStdOutput != INVALID_HANDLE_VALUE) &&
                (ghStdError  != INVALID_HANDLE_VALUE))
                {
                gfStdHandles = TRUE;
                }
            else
                {
                ghStdInput  = INVALID_HANDLE_VALUE;
                ghStdOutput = INVALID_HANDLE_VALUE;
                ghStdError  = INVALID_HANDLE_VALUE;
                }
            }
        if ((gfStdFailure = !gfStdHandles) && gfStdTransient)
            {
            FreeConsole ();
            gfStdTransient = FALSE;
            }
        }
    return gfStdHandles;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kConsoleClose (void)
    {
    BOOL fOk = FALSE;

    if (gfStdHandles)
        {
        ghStdInput   = INVALID_HANDLE_VALUE;
        ghStdOutput  = INVALID_HANDLE_VALUE;
        ghStdError   = INVALID_HANDLE_VALUE;
        gfStdHandles = FALSE;

        fOk = TRUE;
        }
    if (gfStdTransient)
        {
        if (w2kConsoleTest ()) FreeConsole ();
        gfStdTransient = FALSE;
        }
    gfStdFailure = FALSE;
    return fOk;
    }

// =================================================================
// FORMATTED OUTPUT
// =================================================================

DWORD WINAPI w2kFormatAnsi (PWORD pwBuffer,
                            DWORD dOffset,
                            PBYTE pbData,
                            DWORD dData,
                            DWORD dWidth,
                            DWORD dPrecision,
                            WORD  wFill,
                            BOOL  fRight,
                            BOOL  fZero)
    {
    PWORD pwBuffer1;
    WORD  wData;
    DWORD i, j, k;
    DWORD n = 0;

    if (pbData != NULL)
        {
        if (dData != -1)
            {
            i = dData;
            }
        else
            {
            for (i = 0; pbData [i]; i++);
            }
        }
    j = (dPrecision == -1 ? i : max (i, dPrecision));
    n = max (i, dWidth);

    if (pwBuffer != NULL)
        {
        pwBuffer1 = pwBuffer + dOffset;
        wData     = (fZero ? '0' : wFill);

        if (fRight)
            {
            k = n;
            while (k > j) pwBuffer1 [--k] = wData;
            while (k > i) pwBuffer1 [--k] = ' ';
            k = 0;
            }
        else
            {
            k = 0;
            while (k < n - j) pwBuffer1 [k++] = wData;
            while (k < n - i) pwBuffer1 [k++] = ' ';
            }
        while (i--)
            {
            pwBuffer1 [k+i] = pbData [i];
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kFormatUnicode (PWORD pwBuffer,
                               DWORD dOffset,
                               PWORD pwData,
                               DWORD dData,
                               DWORD dWidth,
                               DWORD dPrecision,
                               WORD  wFill,
                               BOOL  fRight,
                               BOOL  fZero)
    {
    PWORD pwBuffer1;
    WORD  wData;
    DWORD i, j, k;
    DWORD n = 0;

    if (pwData != NULL)
        {
        if (dData != -1)
            {
            i = dData;
            }
        else
            {
            for (i = 0; pwData [i]; i++);
            }
        }
    j = (dPrecision == -1 ? i : max (i, dPrecision));
    n = max (i, dWidth);

    if (pwBuffer != NULL)
        {
        pwBuffer1 = pwBuffer + dOffset;
        wData     = (fZero ? '0' : wFill);

        if (fRight)
            {
            k = n;
            while (k > j) pwBuffer1 [--k] = wData;
            while (k > i) pwBuffer1 [--k] = ' ';
            k = 0;
            }
        else
            {
            k = 0;
            while (k < n - j) pwBuffer1 [k++] = wData;
            while (k < n - i) pwBuffer1 [k++] = ' ';
            }
        while (i--)
            {
            pwBuffer1 [k+i] = pwData [i];
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kFormatDecimal (PWORD pwBuffer,
                               DWORD dOffset,
                               DWORD dData,
                               DWORD dWidth,
                               DWORD dPrecision,
                               WORD  wFill,
                               BOOL  fRight,
                               BOOL  fZero,
                               BOOL  fPrefix,
                               BOOL  fSigned)
    {
    BOOL  fMinus;
    PWORD pwBuffer1;
    WORD  wPrefix, wData;
    DWORD dData1, i, j, k;
    DWORD n = 0;

    fMinus = (fSigned && ((LONG) dData < 0));
    dData1 = (fMinus ? 0 - dData : dData);

    for (i = dData1, j = 1; i >= 10; i /= 10, j++);

    if (dPrecision != -1) j = max (j, dPrecision);

    k = (fPrefix || fMinus ? j+1 : j);
    n = max (k, dWidth);

    wPrefix = (k == j ? 0 : (fMinus ? '-' : (dData1 ? '+' : ' ')));

    if (pwBuffer != NULL)
        {
        pwBuffer1 = pwBuffer + dOffset;
        wData     = (fZero ? '0' : wFill);

        if (fRight)
            {
            for (i = n; i > k; i--) pwBuffer1 [i-1] = wData;
            k = 0;
            }
        else
            {
            for (i = 0; i < n - j; i++) pwBuffer1 [i] = wData;
            i = n;
            k = (fZero ? 0 : n - k);
            }
        while (j--)
            {
            pwBuffer1 [--i] = (WORD) (dData1 % 10) + '0';
            dData1 /= 10;
            }
        if (wPrefix)
            {
            pwBuffer1 [k] = wPrefix;
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kFormatHex (PWORD pwBuffer,
                           DWORD dOffset,
                           DWORD dData,
                           DWORD dWidth,
                           DWORD dPrecision,
                           WORD  wFill,
                           BOOL  fRight,
                           BOOL  fZero,
                           BOOL  fPrefix,
                           BOOL  fLower)
    {
    PWORD pwBuffer1;
    WORD  wData;
    DWORD dData1, i, j, k;
    DWORD n = 0;

    dData1 = dData;

    for (i = dData1, j = 1; i >= 0x10; i >>= 4, j++);

    if (dPrecision != -1) j = max (j, dPrecision);

    k = (fPrefix ? j+2 : j);
    n = max (k, (fPrefix ? dWidth+2 : dWidth));

    if (pwBuffer != NULL)
        {
        pwBuffer1 = pwBuffer + dOffset;
        wData     = (fZero ? '0' : wFill);

        if (fRight)
            {
            for (i = n; i > k; i--) pwBuffer1 [i-1] = wData;
            k = 0;
            }
        else
            {
            for (i = 0; i < n - j; i++) pwBuffer1 [i] = wData;
            i = n;
            k = (fZero ? 0 : n - k);
            }
        wData = (fLower ? 'a' : 'A');
        while (j--)
            {
            if ((pwBuffer1 [--i] = (WORD) (dData1 & 0xF)) < 10)
                {
                pwBuffer1 [i] += '0';
                }
            else
                {
                pwBuffer1 [i] -= 10;
                pwBuffer1 [i] += wData;
                }
            dData1 >>= 4;
            }
        if (fPrefix)
            {
            pwBuffer1 [k  ] = '0';
            pwBuffer1 [k+1] = (fLower ? 'x' : 'X');
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kFormatSingle (PWORD  pwBuffer,
                              DWORD  dOffset,
                              PWORD  pwFormat,
                              PDWORD pdFormat,
                              PVOID *ppData,
                              WORD   wFill,
                              BOOL   fUnicode)
    {
    PVOID pData, pText;
    WORD  wData, wFill1;
    BOOL  fAnsi, fShort, fSigned, fLower, fRight, fZero, fPrefix;
    BOOL  fPrecision, fDone, fSkip;
    DWORD dPrecision, dWidth, dData, dFormat, dNumber;
    DWORD n = 0;

    pData   = (ppData   != NULL ? *ppData   : NULL);
    dFormat = (pdFormat != NULL ? *pdFormat : 0);

    if (pwFormat != NULL)
        {
        fAnsi      = !fUnicode;
        fShort     = FALSE;
        fSigned    = FALSE;
        fLower     = FALSE;
        fRight     = FALSE;
        fZero      = FALSE;
        fPrefix    = FALSE;

        wData      = '%';
        wFill1     = wFill;
        dWidth     =  0;
        dPrecision = -1;
        fPrecision = FALSE;
        fDone      = FALSE;

        while ((!fDone) && pwFormat [dFormat])
            {
            fSkip = TRUE;

            switch (pwFormat [dFormat])
                {
// operand size
                case 'l':
                    {
                    fShort = fAnsi = FALSE;
                    break;
                    }
                case 'h':
                    {
                    fShort = fAnsi = TRUE;
                    break;
                    }
// strings
                case 'S':
                    {
                    fAnsi = fUnicode;
                    }
                case 's':
                    {
                    pText = (pData != NULL
                             ? *(PVOID *) pData
                             : NULL);

                    if (fAnsi)
                        {
                        n = w2kFormatAnsi
                                (pwBuffer, dOffset,
                                 pText, -1,
                                 dWidth, dPrecision, wFill1,
                                 fRight, fZero);
                        }
                    else
                        {
                        n = w2kFormatUnicode
                                (pwBuffer, dOffset,
                                 pText, -1,
                                 dWidth, dPrecision, wFill1,
                                 fRight, fZero);
                        }
                    if (pData != NULL)
                        {
                        pData = (PBYTE) pData + sizeof (PVOID);
                        }
                    fDone = TRUE;
                    break;
                    }
// characters
                case 'C':
                    {
                    fAnsi = fUnicode;
                    }
                case 'c':
                    {
                    wData = (pData != NULL
                             ? (fAnsi
                                ? *(PBYTE) pData
                                : *(PWORD) pData)
                             : ' ');

                    if (pData != NULL)
                        {
                        pData = (PBYTE) pData + sizeof (DWORD);
                        }
                    }
                case '%':
                    {
                    n = w2kFormatUnicode
                            (pwBuffer, dOffset,
                             &wData, 1,
                             dWidth, dPrecision, wFill1,
                             fRight, fZero);

                    fDone = TRUE;
                    break;
                    }
// decimal numbers
                case 'd':
                case 'i':
                    {
                    fSigned = TRUE;
                    }
                case 'u':
                    {
                    dData = (pData != NULL
                             ? (fSigned
                                ? (fShort
                                   ? *(PSHORT) pData
                                   : *(PLONG ) pData)
                                : (fShort
                                   ? *(PWORD ) pData
                                   : *(PDWORD) pData))
                             : 0);

                    n = w2kFormatDecimal
                            (pwBuffer, dOffset, dData,
                             dWidth, dPrecision, wFill1,
                             fRight, fZero, fPrefix, fSigned);

                    if (pData != NULL)
                        {
                        pData = (PBYTE) pData + sizeof (DWORD);
                        }
                    fDone = TRUE;
                    break;
                    }
// hex numbers
                case 'x':
                    {
                    fLower = TRUE;
                    }
                case 'X':
                    {
                    dData = (pData != NULL
                             ? (fShort
                                ? *(PWORD ) pData
                                : *(PDWORD) pData)
                             : 0);

                    n = w2kFormatHex
                            (pwBuffer, dOffset, dData,
                             dWidth, dPrecision, wFill1,
                             fRight, fZero, fPrefix, fLower);

                    if (pData != NULL)
                        {
                        pData = (PBYTE) pData + sizeof (DWORD);
                        }
                    fDone = TRUE;
                    break;
                    }
// output styles
                case '+':
                    {
                    if (pwFormat [dFormat+1])
                        {
                        wFill1 = pwFormat [++dFormat];
                        }
                    break;
                    }
                case '-':
                    {
                    fRight = TRUE;
                    break;
                    }
                case '0':
                    {
                    fZero = TRUE;
                    break;
                    }
                case '#':
                    {
                    fPrefix = TRUE;
                    break;
                    }
// output width
                default:
                    {
                    if (fPrecision = (pwFormat [dFormat] == '.'))
                        {
                        dFormat++;
                        dPrecision = 0;

                        fSkip = FALSE;
                        }
                    if ((pwFormat [dFormat] >= '0') &&
                        (pwFormat [dFormat] <= '9'))
                        {
                        dNumber = 0;

                        while ((pwFormat [dFormat] >= '0') &&
                               (pwFormat [dFormat] <= '9'))
                            {
                            dNumber *= 10;
                            dNumber += (pwFormat [dFormat++] - '0');
                            }
                        if (fPrecision)
                            {
                            dPrecision = dNumber;
                            }
                        else
                            {
                            dWidth = dNumber;
                            }
                        fSkip = FALSE;
                        }
                    break;
                    }
                }
            if (fSkip) dFormat++;
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;

    if (pdFormat != NULL) *pdFormat = dFormat;
    if (ppData   != NULL) *ppData   = pData;
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kFormatMulti (PWORD pwBuffer,
                             DWORD dOffset,
                             PWORD pwFormat,
                             PVOID pArguments,
                             WORD  wFill,
                             BOOL  fUnicode)
    {
    PVOID pData;
    DWORD i, j;
    DWORD n = 0;

    if (pwFormat != NULL)
        {
        if ((pData = pArguments) != NULL)
            {
            for (i = 0; pwFormat [i]; i = j)
                {
                for (j = i; pwFormat [j] && (pwFormat [j] != '%');
                     j++);

                n += w2kFormatUnicode (pwBuffer, dOffset+n,
                                       pwFormat+i, j-i,
                                       0, -1, wFill,
                                       TRUE, FALSE);

                if (pwFormat [j] == '%')
                    {
                    j++;

                    n += w2kFormatSingle (pwBuffer, dOffset+n,
                                          pwFormat, &j, &pData,
                                          wFill, fUnicode);
                    }
                }
            }
        else
            {
            n = w2kFormatUnicode (pwBuffer, dOffset,
                                  pwFormat, -1,
                                  0, -1, wFill,
                                  TRUE, FALSE);
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;
    return n;
    }

// -----------------------------------------------------------------

PBYTE WINAPI w2kFormatW2A (PWORD pwFrom,
                           PBYTE pbTo)
    {
    DWORD n;
    PBYTE pbTo1 = NULL;

    if (pwFrom != NULL)
        {
        for (n = 0; pwFrom [n]; n++);

        if ((pbTo1 = (pbTo != NULL ? pbTo : w2kMemoryAnsi (n+1)))
            != NULL)
            {
            if (!WideCharToMultiByte
                     (CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
                      pwFrom, -1, pbTo1, n+1, NULL, NULL))
                {
                do  {
                    pbTo1 [n] = (pwFrom [n] < 0x0100
                                 ? (BYTE) pwFrom [n]
                                 : INVALID_UNICODE);
                    }
                while (n--);
                }
            }
        }
    return pbTo1;
    }

// =================================================================
// vprintf() FAMILY
// =================================================================

DWORD WINAPI _vsprintf (PWORD pwBuffer,
                        PWORD pwFormat,
                        PVOID pArguments)
    {
    return w2kFormatMulti (pwBuffer, 0, pwFormat, pArguments,
                           gwFill, TRUE);
    }

// -----------------------------------------------------------------

INT WINAPI _vmprintf (HWND  hWnd,
                      UINT  uiType,
                      PWORD pwCaption,
                      PWORD pwFormat,
                      PVOID pArguments)
    {
    PWORD pwBuffer;
    DWORD dBuffer;
    INT   iId = IDABORT;

    dBuffer = _vsprintf (NULL, pwFormat, pArguments);

    if ((pwBuffer = w2kMemoryUnicode (dBuffer+1)) != NULL)
        {
        _vsprintf (pwBuffer, pwFormat, pArguments);

        iId = MessageBox (hWnd, pwBuffer,
                          (pwCaption != NULL ? pwCaption
                                             : awCaption),
                          uiType);

        w2kMemoryDestroy (pwBuffer);
        }
    return iId;
    }

// -----------------------------------------------------------------

DWORD WINAPI _vfprintf (HANDLE hFile,
                        PWORD  pwFormat,
                        PVOID  pArguments)
    {
    PWORD pwBuffer;
    PBYTE pbBuffer;
    DWORD dBuffer, n1, n2;
    DWORD n = 0;

    dBuffer = _vsprintf (NULL, pwFormat, pArguments);

    if ((pwBuffer = w2kMemoryUnicode (dBuffer+1)) != NULL)
        {
        _vsprintf (pwBuffer, pwFormat, pArguments);

        if ((pbBuffer = w2kFormatW2A (pwBuffer, NULL)) != NULL)
            {
            while (dBuffer > n)
                {
                n1 = min (dBuffer-n, N_WRITE);
                n2 = 0;

                if (WriteFile (hFile, pbBuffer+n, n1, &n2, NULL)
                    && n2)
                    {
                    n += n2;
                    }
                else break;
                }
            w2kMemoryDestroy (pbBuffer);
            }
        w2kMemoryDestroy (pwBuffer);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI _veprintf (PWORD pwFormat,
                        PVOID pArguments)
    {
    w2kConsoleOpen ();
    return _vfprintf (ghStdError, pwFormat, pArguments);
    }

// -----------------------------------------------------------------

DWORD WINAPI _vprintf (PWORD pwFormat,
                       PVOID pArguments)
    {
    w2kConsoleOpen ();
    return _vfprintf (ghStdOutput, pwFormat, pArguments);
    }

// =================================================================
// printf() FAMILY
// =================================================================

DWORD WINAPI _sprintf (PWORD pwBuffer,
                       PWORD pwFormat,
                       ...)
    {
    return _vsprintf (pwBuffer, pwFormat, (&pwFormat)+1);
    }

// -----------------------------------------------------------------

INT WINAPI _mprintf (HWND  hWnd,
                     UINT  uiType,
                     PWORD pwCaption,
                     PWORD pwFormat,
                     ...)
    {
    return _vmprintf (hWnd, uiType, pwCaption, pwFormat,
                      (&pwFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI _fprintf (HANDLE hFile,
                       PWORD  pwFormat,
                       ...)
    {
    return _vfprintf (hFile, pwFormat, (&pwFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI _eprintf (PWORD pwFormat,
                      ...)
    {
    return _veprintf (pwFormat, (&pwFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI _printf (PWORD pwFormat,
                      ...)
    {
    return _vprintf (pwFormat, (&pwFormat)+1);
    }

// =================================================================
// STRING PROCESSING
// =================================================================

PWORD WINAPI _strcpy (PWORD pwTo,
                      PWORD pwFrom,
                      PWORD pwDefault)
    {
    return (pwTo != NULL

            ? lstrcpy (pwTo,
                       (pwFrom != NULL
                        ? pwFrom
                        : (pwDefault != NULL
                           ? pwDefault
                           : awNull)))
            : NULL);
    }

// -----------------------------------------------------------------

PWORD WINAPI _strcpyn (PWORD pwTo,
                       PWORD pwFrom,
                       PWORD pwDefault,
                       DWORD dTo)
    {
    return ((pwTo != NULL) && dTo

            ? lstrcpyn (pwTo,
                        (pwFrom != NULL
                         ? pwFrom
                         : (pwDefault != NULL
                            ? pwDefault
                            : awNull)),
                        dTo)
            : NULL);
    }

// =================================================================
// CRC32 COMPUTATION
// =================================================================

DWORD WINAPI w2kCrc32Start (PDWORD pdCrc32)
    {
    *pdCrc32 = 0xFFFFFFFF;
    return *pdCrc32;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kCrc32Stop (PDWORD pdCrc32)
    {
    *pdCrc32 = ~*pdCrc32;
    return (*pdCrc32 == 0xDEBB20E3);
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCrc32Byte (PDWORD pdCrc32,
                           BYTE   bData)
    {
    *pdCrc32 = (*pdCrc32 >> 8) ^ adCrc32 [(BYTE) *pdCrc32 ^ bData];
    return *pdCrc32;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCrc32Word (PDWORD pdCrc32,
                           WORD   wData)
    {
    w2kCrc32Byte (pdCrc32, LOBYTE (wData));
    return w2kCrc32Byte (pdCrc32, HIBYTE (wData));
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCrc32Dword (PDWORD pdCrc32,
                            DWORD  dData)
    {
    w2kCrc32Word (pdCrc32, LOWORD (dData));
    return w2kCrc32Word (pdCrc32, HIWORD (dData));
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCrc32Multi (PDWORD pdCrc32,
                            PVOID  pData,
                            DWORD  dData)
    {
    DWORD dCrc32, i;

    if (pdCrc32 != NULL) dCrc32 = *pdCrc32;
    else w2kCrc32Start (&dCrc32);

    if (pData != NULL)
        {
        for (i = 0; i < dData; i++)
            {
            w2kCrc32Byte (&dCrc32, ((PBYTE) pData) [i]);
            }
        }
    if (pdCrc32 != NULL) *pdCrc32 = dCrc32;
    else w2kCrc32Stop (&dCrc32);

    return dCrc32;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCrc32Text (PDWORD pdCrc32,
                           PWORD  pwData,
                           DWORD  dData)
    {
    DWORD dCrc32, i, n;

    if (pdCrc32 != NULL) dCrc32 = *pdCrc32;
    else w2kCrc32Start (&dCrc32);

    if (pwData != NULL)
        {
        n = (dData != -1 ? dData : lstrlen (pwData));

        for (i = 0; i < n; i++)
            {
            w2kCrc32Word (&dCrc32, pwData [i]);
            }
        }
    if (pdCrc32 != NULL) *pdCrc32 = dCrc32;
    else w2kCrc32Stop (&dCrc32);

    return dCrc32;
    }

// =================================================================
// PSEUDO-RANDOM NUMBER GENERATOR
// =================================================================

DWORD WINAPI w2kRandomInitialize (PDWORD pdData,
                                  PDWORD pdAddress,
                                  DWORD  dSeed)
    {
    *pdData = *pdAddress = (dSeed ? dSeed : GetTickCount ());

    w2kRandomByte (pdData, pdAddress);
    w2kRandomByte (pdData, pdAddress);
    w2kRandomByte (pdData, pdAddress);
    w2kRandomByte (pdData, pdAddress);

    return *pdData;
    }

// -----------------------------------------------------------------

BYTE WINAPI w2kRandomByte (PDWORD pdData,
                           PDWORD pdAddress)
    {
    BYTE bRandom = 0;

    __asm
        {
                push    ebx
                mov     eax, pdData
                mov     edx, pdAddress
                mov     ebx, [eax]
                mov     ecx, [edx]
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                rol     ebx, 1
                test    bl, 10101110b
                setpo   ah
                xor     bl, ah
                shl     ecx, 1
                sets    ah
                or      cl, ah
                test    cl, 00001000b
                setpo   ah
                xor     cl, ah
                rol     ecx, 11
                bt      ebx, ecx
                rcr     al, 1
                ror     ecx, 11
        }
    __asm
        {
                mov     bRandom, al
                mov     eax, pdData
                mov     edx, pdAddress
                mov     [eax], ebx
                mov     [edx], ecx
                pop     ebx
        }
    return bRandom;
    }

// -----------------------------------------------------------------

WORD WINAPI w2kRandomWord (PDWORD pdData,
                           PDWORD pdAddress)
    {
    WORD wRandom = 0;

    wRandom  = (((WORD) w2kRandomByte (pdData, pdAddress))     );
    wRandom |= (((WORD) w2kRandomByte (pdData, pdAddress)) << 8);
    return wRandom;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kRandomDword (PDWORD pdData,
                             PDWORD pdAddress)
    {
    DWORD dRandom = 0;

    dRandom  = (((DWORD) w2kRandomWord (pdData, pdAddress))      );
    dRandom |= (((DWORD) w2kRandomWord (pdData, pdAddress)) << 16);
    return dRandom;
    }

// =================================================================
// ENVIRONMENT MANAGEMENT
// =================================================================

PWORD WINAPI w2kEnvironmentString (PWORD pwStrings,
                                   PWORD pwName)
    {
    DWORD i, j, k;
    PWORD pwValue = NULL;

    if ((pwStrings != NULL) && (pwName != NULL))
        {
        for (i = 0; pwStrings [i]; i += (lstrlen (pwStrings+i) + 1))
            {
            if (pwStrings [j=i] == '=') j++;

            while (pwStrings [j] && (pwStrings [j] != '=')) j++;

            if (pwStrings [j] == '=')
                {
                for (k = 0; (i+k < j) && pwName [k]; k++)
                    {
                    if (LCASE (pwStrings [i+k]) !=
                        LCASE (pwName    [  k])) break;
                    }
                if ((i+k == j) && (!pwName [k]))
                    {
                    pwValue = pwStrings+j+1;
                    break;
                    }
                }
            }
        }
    return pwValue;
    }

// =================================================================
// PATH MANAGEMENT
// =================================================================

DWORD WINAPI w2kPathNormalize (PWORD pwPath)
    {
    DWORD i, j;
    DWORD dData = 0;

    if (pwPath != NULL)
        {
        i = j = 0;

        while (pwPath [i])
            {
            if (pwPath [i++] != '"')
                {
                pwPath [j++] = pwPath [i-1];
                }
            else
                {
                while (pwPath [i] && (pwPath [i++] != '"'))
                    {
                    pwPath [j++] = pwPath [i-1];
                    }
                }
            }
        while (j && (pwPath [j-1] == ' ')) j--;
        for (i = 0; (i < j) && (pwPath [i] == ' '); i++);

        dData = j-i;

        if (i)
            {
            for (j = 0; j < dData; j++)
                {
                pwPath [j] = pwPath [i+j];
                }
            }
        pwPath [dData++] = 0;
        }
    return dData;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kPathEvaluate (PWORD  pwPath,
                              PDWORD pdData)
    {
    PWORD pwStrings, pwValue;
    DWORD i, j, n;
    DWORD dData   = 0;
    PWORD pwPath1 = NULL;

    if (w2kPathNormalize (pwPath) &&
        ((pwStrings = GetEnvironmentStrings ()) != NULL))
        {
        i = n = 0;

        while (pwPath [i])
            {
            while (pwPath [i] && (pwPath [i] != '%')) n++, i++;

            if (pwPath [i] == '%')
                {
                j = ++i;

                while (pwPath [j] && (pwPath [j] != '%')) j++;

                if ((pwPath [j] == '%') && (i < j))
                    {
                    pwPath [j] = 0;

                    pwValue = w2kEnvironmentString (pwStrings,
                                                    pwPath+i);
                    pwPath [j] = '%';
                    }
                else
                    {
                    pwValue = NULL;
                    }
                if (pwValue != NULL)
                    {
                    n += lstrlen (pwValue);
                    i  = j + 1;
                    }
                else
                    {
                    n += (1 + j - i);
                    i  = j;
                    }
                }
            }
        if ((pwPath1 = w2kMemoryCreate (++n * WORD_)) != NULL)
            {
            i = 0;

            while (pwPath [i])
                {
                while (pwPath [i] && (pwPath [i] != '%'))
                    {
                    pwPath1 [dData++] = pwPath [i++];
                    }
                if (pwPath [i] == '%')
                    {
                    j = ++i;

                    while (pwPath [j] && (pwPath [j] != '%')) j++;

                    if ((pwPath [j] == '%') && (i < j))
                        {
                        pwPath [j] = 0;

                        pwValue = w2kEnvironmentString (pwStrings,
                                                        pwPath+i);
                        pwPath [j] = '%';
                        }
                    else
                        {
                        pwValue = NULL;
                        }
                    if (pwValue != NULL)
                        {
                        i = j + 1;

                        for (j = 0; pwValue [j]; dData++, j++)
                            {
                            pwPath1 [dData] = pwValue [j];
                            }
                        }
                    else
                        {
                        for (--i; i < j; dData++, i++)
                            {
                            pwPath1 [dData] = pwPath [i];
                            }
                        }
                    }
                }
            pwPath1 [dData++] = 0;
            }
        FreeEnvironmentStrings (pwStrings);
        }
    if (pdData != NULL) *pdData = dData;
    return pwPath1;
    };

// -----------------------------------------------------------------

DWORD WINAPI w2kPathRoot (PWORD pwPath)
    {
    DWORD dRoot = 0;

    if ((pwPath != NULL) && pwPath [0])
        {
        if (pwPath [0] == '\\')
            {
            if (pwPath [1] == '\\')
                {
                for (dRoot = 2;
                     pwPath [dRoot] && (pwPath [dRoot] != '\\');
                     dRoot++);
                }
            }
        else
            {
            if (pwPath [1] == ':') dRoot = 2;
            }
        }
    return dRoot;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kPathName (PWORD  pwPath,
                          PDWORD pdExtension)
    {
    DWORD dRoot, dEnd;
    DWORD dExtension = 0;
    DWORD dName      = 0;

    if (pwPath != NULL)
        {
        dRoot = w2kPathRoot (pwPath);
        dEnd  = dRoot + lstrlen (pwPath + dRoot);

        for (dName = dEnd;
             (dName > dRoot) && (pwPath [dName-1] != '\\')
                             && (pwPath [dName-1] != ':' );
             dName--);

        for (dExtension = dEnd;
             (dExtension > dName) && (pwPath [dExtension-1] != '.');
             dExtension--);

        dExtension = (dExtension == dName ? dEnd : dExtension - 1);
        }
    if (pdExtension != NULL) *pdExtension = dExtension;
    return dName;
    }

// =================================================================
// FILE MANAGEMENT
// =================================================================

DWORD WINAPI w2kFilePath (HINSTANCE hInstance,
                          PWORD     pwFile,
                          PWORD     pwBuffer,
                          DWORD     dBuffer)
    {
    DWORD i, n;
    DWORD dSize = 0;

    if ((pwBuffer != NULL) && dBuffer)
        {
        if (n = GetModuleFileName (hInstance, pwBuffer, dBuffer))
            {
            if (pwFile != NULL)
                {
                for (i = n; i; i--)
                    {
                    if ((pwBuffer [i-1] == ':' ) ||
                        (pwBuffer [i-1] == '\\'))
                        {
                        break;
                        }
                    if ((pwFile   [ 0 ] == '.') &&
                        (pwFile   [ 1 ] != '.') &&
                        (pwBuffer [i-1] == '.'))
                        {
                        i--;
                        break;
                        }
                    }
                if ((n = i + lstrlen (pwFile)) < dBuffer)
                    {
                    lstrcpy (pwBuffer+i, pwFile);
                    dSize = n;
                    }
                }
            else
                {
                dSize = n;
                }
            }
        pwBuffer [dSize] = 0;
        }
    return dSize;
    }

// -----------------------------------------------------------------

HANDLE WINAPI w2kFileOpen (PWORD pwPath,
                           BOOL  fWrite)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pwPath != NULL) && pwPath [0])
        {
        if (fWrite)
            {
            hf = CreateFile (pwPath, GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                             FILE_FLAG_SEQUENTIAL_SCAN |
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
            }
        else
            {
            hf = CreateFile (pwPath, GENERIC_READ,
                             FILE_SHARE_READ, NULL, OPEN_EXISTING,
                             FILE_FLAG_SEQUENTIAL_SCAN,
                             NULL);
            }
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI w2kFileNew (PWORD pwPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pwPath != NULL) && pwPath [0])
        {
        hf = CreateFile (pwPath, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                         FILE_FLAG_SEQUENTIAL_SCAN |
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI w2kFileClose (HANDLE hf)
    {
    if (hf != INVALID_HANDLE_VALUE) CloseHandle (hf);
    return INVALID_HANDLE_VALUE;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kFileTest (PWORD pwPath)
    {
    HANDLE          hf;
    WIN32_FIND_DATA wfd;
    BOOL            fOk = FALSE;

    if ((pwPath != NULL) && pwPath [0] &&
        ((hf = FindFirstFile (pwPath, &wfd))
         != INVALID_HANDLE_VALUE))
        {
        fOk = (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
        FindClose (hf);
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kFileLoad (PWORD     pwPath,
                          PDWORD    pdSize,
                          PFILETIME pft)
    {
    HANDLE   hf;
    FILETIME ft;
    DWORD    dSize, n;
    PVOID    pData = NULL;

    if ((hf = w2kFileOpen (pwPath, FALSE))
        != INVALID_HANDLE_VALUE)
        {
        if (GetFileTime (hf, NULL, NULL, &ft) &&
            ((dSize = GetFileSize (hf, NULL)) != INVALID_FILE_SIZE))
            {
            if ((pData = w2kMemoryCreate (dSize+1)) != NULL)
                {
                n = 0;

                if ((!dSize)
                    ||
                    (ReadFile (hf, pData, dSize, &n, NULL) &&
                     (dSize == n)))
                    {
                    ((PBYTE) pData) [dSize] = 0;
                    }
                else
                    {
                    pData = w2kMemoryDestroy (pData);
                    }
                }
            }
        w2kFileClose (hf);
        }
    if (pData == NULL)
        {
        dSize             = 0;
        ft.dwLowDateTime  = 0;
        ft.dwHighDateTime = 0;
        }
    if (pdSize != NULL) *pdSize = dSize;
    if (pft    != NULL) *pft    = ft;
    return pData;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kFileSave (PWORD pwPath,
                         PVOID pData,
                         DWORD dData)
    {
    HANDLE hf;
    DWORD  dData1;
    BOOL   fOk = FALSE;

    if ((hf = w2kFileNew (pwPath))
        != INVALID_HANDLE_VALUE)
        {
        dData1 = (dData != MAXDWORD
                  ? dData
                  : (pData != NULL ? lstrlen (pData) : 0));

        fOk = w2kFileWrite (hf, pData, dData1) &&
              SetEndOfFile (hf);

        w2kFileClose (hf);

        if (!fOk) DeleteFile (pwPath);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kFileAppend (PWORD pwPath,
                           PVOID pData,
                           DWORD dData)
    {
    HANDLE hf;
    DWORD  dData1;
    BOOL   fOk = FALSE;

    if ((hf = w2kFileOpen (pwPath, TRUE))
        != INVALID_HANDLE_VALUE)
        {
        if (SetFilePointer (hf, 0, NULL, FILE_END)
            != INVALID_SET_FILE_POINTER)
            {
            dData1 = (dData != MAXDWORD
                      ? dData
                      : (pData != NULL ? lstrlen (pData) : 0));

            fOk = w2kFileWrite (hf, pData, dData1);
            }
        w2kFileClose (hf);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kFileWrite (HANDLE hf,
                          PVOID  pData,
                          DWORD  dData)
    {
    DWORD n;
    BOOL  fOk = FALSE;

    if (hf != INVALID_HANDLE_VALUE)
        {
        if (dData && (pData != NULL))
            {
            n   = 0;
            fOk = WriteFile (hf, pData, dData, &n, NULL) &&
                  (dData == n);
            }
        else
            {
            fOk = (dData == 0);
            }
        }
    return fOk;
    }

// =================================================================
// REGISTRY MANAGEMENT
// =================================================================

BOOL WINAPI w2kRegistryRoot (PWORD pwRoot,
                             PHKEY phk)
    {
    BOOL fOk = FALSE;

    if (phk != NULL)
        {
        if      (!lstrcmpi (pwRoot, awHkeyClassesRoot))
            {
            *phk = HKEY_CLASSES_ROOT;
            }
        else if (!lstrcmpi (pwRoot, awHkeyCurrentUser))
            {
            *phk = HKEY_CURRENT_USER;
            }
        else if (!lstrcmpi (pwRoot, awHkeyLocalMachine))
            {
            *phk = HKEY_LOCAL_MACHINE;
            }
        else if (!lstrcmpi (pwRoot, awHkeyUsers))
            {
            *phk = HKEY_USERS;
            }
        else if (!lstrcmpi (pwRoot, awHkeyPerformanceData))
            {
            *phk = HKEY_PERFORMANCE_DATA;
            }
        else if (!lstrcmpi (pwRoot, awHkeyCurrentConfig))
            {
            *phk = HKEY_CURRENT_CONFIG;
            }
        else if (!lstrcmpi (pwRoot, awHkeyDynData))
            {
            *phk = HKEY_DYN_DATA;
            }
        else
            {
            *phk = HKEY_NULL;
            }
        fOk = (*phk != HKEY_NULL);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryParse (PWORD  pwPath,
                              PHKEY  phk,
                              PPWORD ppwKey,
                              PPWORD ppwValue)
    {
    DWORD dStop;
    HKEY  hk     = HKEY_NULL;
    DWORD dKey   = 0;
    DWORD dValue = 0;

    if (pwPath != NULL)
        {
        while (pwPath [dKey] && (pwPath [dKey] != '\\'))
              dKey++;

        if (pwPath [dKey]) pwPath [dKey++] = 0;

        dValue = dStop = dKey + lstrlen (pwPath + dKey);

        while ((dValue > dKey) && (pwPath [dValue-1] != '\\'))
              dValue--;

        if (dValue > dKey) pwPath [dValue-1] = 0; else dKey = dStop;

        w2kRegistryRoot (pwPath, &hk);
        }
    if (phk      != NULL) *phk      = hk;
    if (ppwKey   != NULL) *ppwKey   = pwPath + dKey;
    if (ppwValue != NULL) *ppwValue = pwPath + dValue;
    return (hk != HKEY_NULL);
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kRegistryParseEx (PWORD  pwPath,
                                 PHKEY  phk,
                                 PPWORD ppwKey,
                                 PPWORD ppwValue)
    {
    PWORD pwData = NULL;

    if ((pwPath != NULL)
        &&
        ((pwData = w2kMemoryCreate ((lstrlen(pwPath) + 1) * WORD_))
         != NULL))
        {
        lstrcpy (pwData, pwPath);

        if (!w2kRegistryParse (pwData, phk, ppwKey, ppwValue))
            {
            pwData = w2kMemoryDestroy (pwData);
            }
        }
    return pwData;
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kRegistryClose (HKEY hk)
    {
    if ((hk != HKEY_NULL            ) &&
        (hk != HKEY_CLASSES_ROOT    ) &&
        (hk != HKEY_CURRENT_USER    ) &&
        (hk != HKEY_LOCAL_MACHINE   ) &&
        (hk != HKEY_USERS           ) &&
        (hk != HKEY_PERFORMANCE_DATA) &&
        (hk != HKEY_CURRENT_CONFIG  ) &&
        (hk != HKEY_DYN_DATA        ))
        {
        RegCloseKey (hk);
        }
    return HKEY_NULL;
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kRegistryOpen (HKEY  hkBase,
                             PWORD pwKey)
    {
    HKEY hk = HKEY_NULL;

    if (hkBase != HKEY_NULL)
        {
        if ((pwKey != NULL) && pwKey [0])
            {
            if (RegOpenKeyEx (hkBase, pwKey, 0, KEY_READ, &hk)
                != ERROR_SUCCESS)
                {
                hk = HKEY_NULL;
                }
            }
        else
            {
            hk = hkBase;
            }
        }
    return hk;
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kRegistryOpenEx (HKEY  hkBase,
                               DWORD dKeys,
                               PWORD pwKey,
                               ...)
    {
    HKEY   hk1;
    DWORD  i;
    PPWORD ppwKeys = &pwKey;
    HKEY   hk      = hkBase;

    for (i = 0; (hk != HKEY_NULL) && (i < dKeys); i++)
        {
        hk = w2kRegistryOpen (hk1 = hk, ppwKeys [i]);
        if (hk != hk1) w2kRegistryClose (hk1);
        }
    return hk;
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kRegistryCreate (HKEY  hkBase,
                               PWORD pwKey)
    {
    HKEY hk = HKEY_NULL;

    if (hkBase != HKEY_NULL)
        {
        if ((pwKey != NULL) && pwKey [0])
            {
            if (RegCreateKeyEx (hkBase, pwKey, 0, awNull,
                                REG_OPTION_NON_VOLATILE, KEY_WRITE,
                                NULL, &hk, NULL)
                != ERROR_SUCCESS)
                {
                hk = HKEY_NULL;
                }
            }
        else
            {
            hk = hkBase;
            }
        }
    return hk;
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kRegistryCreateEx (HKEY  hkBase,
                                 DWORD dKeys,
                                 PWORD pwKey,
                                 ...)
    {
    HKEY   hk1;
    DWORD  i;
    PPWORD ppwKeys = &pwKey;
    HKEY   hk      = hkBase;

    for (i = 0; (hk != HKEY_NULL) && (i < dKeys); i++)
        {
        hk = w2kRegistryCreate (hk1 = hk, ppwKeys [i]);
        if (hk != hk1) w2kRegistryClose (hk1);
        }
    return hk;
    }

// -----------------------------------------------------------------

PW2K_PATH WINAPI w2kRegistryPath (PWORD pwPath,
                                  BOOL  fCreate)
    {
    PW2K_PATH pwp = NULL;

    if ((pwPath != NULL) &&
        ((pwp = w2kMemoryCreate (W2K_PATH_)) != NULL))
        {
        pwp->pwParts = w2kRegistryParseEx (pwPath,
                                           &pwp->hkBase,
                                           &pwp->pwKey,
                                           &pwp->pwValue);

        pwp->hk = (pwp->pwParts != NULL
                   ? (fCreate ? w2kRegistryCreate : w2kRegistryOpen)
                         (pwp->hkBase, pwp->pwKey)
                   : HKEY_NULL);

        if (pwp->hk == HKEY_NULL)
            {
            pwp = w2kRegistryPathClose (pwp);
            }
        }
    return pwp;
    }

// -----------------------------------------------------------------

PW2K_PATH WINAPI w2kRegistryPathOpen (PWORD pwPath)
    {
    return w2kRegistryPath (pwPath, FALSE);
    }

// -----------------------------------------------------------------

PW2K_PATH WINAPI w2kRegistryPathCreate (PWORD pwPath)
    {
    return w2kRegistryPath (pwPath, TRUE);
    }

// -----------------------------------------------------------------

PW2K_PATH WINAPI w2kRegistryPathClose (PW2K_PATH pwp)
    {
    if (pwp != NULL)
        {
        w2kRegistryClose (pwp->hk);
        w2kMemoryDestroy (pwp->pwParts);
        w2kMemoryDestroy (pwp);
        }
    return NULL;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kRegistryPathRead (PWORD  pwPath,
                                  DWORD  dMode,
                                  PDWORD pdSize)
    {
    return w2kRegistryRead (HKEY_NULL, pwPath, dMode, pdSize);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryPathWrite (PWORD pwPath,
                                  DWORD dMode,
                                  PVOID pData)
    {
    return w2kRegistryWrite (HKEY_NULL, pwPath, dMode, pData);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kRegistryData (HKEY   hk,
                              PWORD  pwValue,
                              PDWORD pdType,
                              PDWORD pdSize)
    {
    DWORD     dType, dSize;
    PW2K_PATH pwp      = NULL;
    HKEY      hk1      = hk;
    PWORD     pwValue1 = pwValue;
    PVOID     pData    = NULL;

    if ((hk1 == HKEY_NULL) &&
        ((pwp = w2kRegistryPathOpen (pwValue1)) != NULL))
        {
        hk1      = pwp->hk;
        pwValue1 = pwp->pwValue;
        }
    if ((hk1 != HKEY_NULL)
        &&
        (RegQueryValueEx (hk1, pwValue1, NULL,
                          NULL, NULL, &dSize)
         == ERROR_SUCCESS)
        &&
        ((pData = w2kMemoryCreate (dSize)) != NULL))
        {
        if (RegQueryValueEx (hk1, pwValue1, NULL,
                             &dType, pData, &dSize)
            != ERROR_SUCCESS)
            {
            pData = w2kMemoryDestroy (pData);
            }
        }
    w2kRegistryPathClose (pwp);

    if (pdType != NULL) *pdType = (pData != NULL? dType : REG_NONE);
    if (pdSize != NULL) *pdSize = (pData != NULL? dSize : 0);
    return pData;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kRegistryRead (HKEY   hk,
                              PWORD  pwValue,
                              DWORD  dMode,
                              PDWORD pdSize)
    {
    DWORD dType, dSize;
    PVOID pData = NULL;

    if ((pData = w2kRegistryData (hk, pwValue, &dType, &dSize))
        != NULL)
        {
        if (((dMode == W2K_MODE_DWORD) &&
             ((dType != REG_DWORD) || (dSize != DWORD_)))
            ||
            ((dMode == W2K_MODE_TEXT) &&
             ((dType != REG_SZ) || (!(dSize /= WORD_))))
            ||
            (W2K_MODE_BINARY (dMode) &&
             (dMode != dSize)))
            {
            pData = w2kMemoryDestroy (pData);
            }
        }
    if (pdSize != NULL) *pdSize = (pData != NULL? dSize : 0);
    return pData;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryReadValue (HKEY       hk,
                                  PW2K_VALUE pwv)
    {
    BOOL fOk = FALSE;

    if (pwv != NULL)
        {
        switch (pwv->dMode)
            {
            case W2K_MODE_DWORD:
                {
                fOk = w2kRegistryReadDword (hk, pwv->pwValue,
                                            &pwv->dData);
                break;
                }
            case W2K_MODE_TEXT:
                {
                fOk = ((pwv->pwData =
                        w2kRegistryReadText (hk, pwv->pwValue,
                                             NULL))
                       != NULL);
                break;
                }
            default:
                {
                fOk = ((pwv->pData =
                        w2kRegistryReadBinary (hk, pwv->pwValue,
                                               pwv->dMode))
                       != NULL);
                break;
                }
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryReadDword (HKEY   hk,
                                  PWORD  pwValue,
                                  PDWORD pdData)
    {
    PVOID pData;
    DWORD dData = 0;
    BOOL  fOk   = FALSE;

    if ((pData = w2kRegistryRead (hk, pwValue, W2K_MODE_DWORD,
                                  NULL))
        != NULL)
        {
        dData = *(PDWORD) pData;
        w2kMemoryDestroy (pData);

        fOk = TRUE;
        }
    if (pdData != NULL) *pdData = dData;
    return fOk;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kRegistryReadText (HKEY   hk,
                                  PWORD  pwValue,
                                  PDWORD pdSize)
    {
    return w2kRegistryRead (hk, pwValue, W2K_MODE_TEXT, pdSize);
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kRegistryReadModule (HKEY   hk,
                                    PWORD  pwValue,
                                    PDWORD pdSize)
    {
    PWORD pwText;
    DWORD dSize  = 0;
    PWORD pwData = NULL;

    if ((pwText = w2kRegistryReadText (hk, pwValue, NULL)) != NULL)
        {
        pwData = w2kPathEvaluate (pwText, &dSize);
        w2kMemoryDestroy (pwText);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pwData;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kRegistryReadBinary (HKEY  hk,
                                    PWORD pwValue,
                                    DWORD dSize)
    {
    PVOID pData = NULL;

    if (W2K_MODE_BINARY (dSize))
        {
        pData = w2kRegistryRead (hk, pwValue, dSize, NULL);
        }
    return pData;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryWrite (HKEY  hk,
                              PWORD pwValue,
                              DWORD dMode,
                              PVOID pData)
    {
    PVOID     pData1;
    DWORD     dSize;
    DWORD     dNull    = 0;
    PW2K_PATH pwp      = NULL;
    HKEY      hk1      = hk;
    PWORD     pwValue1 = pwValue;
    BOOL      fOk      = FALSE;

    if ((hk1 == HKEY_NULL) &&
        ((pwp = w2kRegistryPathCreate (pwValue1)) != NULL))
        {
        hk1      = pwp->hk;
        pwValue1 = pwp->pwValue;
        }
    if (hk1 != HKEY_NULL)
        {
        switch (dMode)
            {
            case W2K_MODE_DWORD:
                {
                pData1 = (pData != NULL ? pData : &dNull);
                dSize  = DWORD_;

                fOk = (RegSetValueEx (hk1, pwValue1, 0,
                                      REG_DWORD, pData1, dSize)
                       == ERROR_SUCCESS);
                break;
                }
            case W2K_MODE_TEXT:
                {
                pData1 = (pData != NULL ? pData : awNull);
                dSize  = (lstrlen (pData1) + 1) * WORD_;

                fOk = (RegSetValueEx (hk1, pwValue1, 0,
                                      REG_SZ, pData1, dSize)
                       == ERROR_SUCCESS);
                break;
                }
            default:
                {
                dSize = dMode;

                if (pData != NULL)
                    {
                    pData1 = pData;
                    }
                else
                    {
                    if ((pData1 = w2kMemoryCreate (dSize)) != NULL)
                        {
                        w2kMemoryZero (pData1, dSize);
                        }
                    }
                if (pData1 != NULL)
                    {
                    fOk = (RegSetValueEx (hk1, pwValue1, 0,
                                          REG_BINARY, pData1, dSize)
                           == ERROR_SUCCESS);

                    if (pData1 != pData)
                        {
                        w2kMemoryDestroy (pData1);
                        }
                    }
                break;
                }
            }
        }
    w2kRegistryPathClose (pwp);
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryWriteValue (HKEY       hk,
                                   PW2K_VALUE pwv)
    {
    BOOL fOk = FALSE;

    if (pwv != NULL)
        {
        fOk = w2kRegistryWrite (hk, pwv->pwValue, pwv->dMode,
                                (pwv->dMode == W2K_MODE_DWORD
                                 ? &pwv->dData
                                 :  pwv->pData));
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryWriteDword (HKEY  hk,
                                   PWORD pwValue,
                                   DWORD dData)
    {
    return w2kRegistryWrite (hk, pwValue, W2K_MODE_DWORD, &dData);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryWriteText (HKEY  hk,
                                  PWORD pwValue,
                                  PWORD pwData)
    {
    return w2kRegistryWrite (hk, pwValue, W2K_MODE_TEXT, pwData);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kRegistryWriteBinary (HKEY  hk,
                                    PWORD pwValue,
                                    PVOID pData,
                                    DWORD dSize)
    {
    return w2kRegistryWrite (hk, pwValue, dSize, pData);
    }

// =================================================================
// APPLICATION DATA MANAGEMENT
// =================================================================

HKEY WINAPI w2kApplicationKey (PWORD pwCompany,
                               PWORD pwId,
                               PWORD pwVersion,
                               PWORD pwKey,
                               BOOL  fUser,
                               BOOL  fCreate)
    {
    PWORD pwCompany1, pwId1, pwVersion1;
    HKEY  hk = HKEY_NULL;

    pwCompany1 = (pwCompany != NULL ? pwCompany : awKeyCompany);
    pwId1      = (pwId      != NULL ? pwId      : awKeyId     );
    pwVersion1 = (pwVersion != NULL ? pwVersion : awKeyVersion);

    if (fUser)
        {
        hk = (fCreate ? w2kRegistryCreateEx : w2kRegistryOpenEx)
                 (HKEY_CURRENT_USER,
                  5, awKeyUserSoftware,
                     pwCompany1, pwId1, pwVersion1, pwKey);
        }
    else
        {
        hk = (fCreate ? w2kRegistryCreateEx : w2kRegistryOpenEx)
                 (HKEY_LOCAL_MACHINE,
                  5, awKeyMachineSoftware,
                     pwCompany1, pwId1, pwVersion1, pwKey);
        }
    return hk;
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kApplicationKeyOpen (PWORD pwCompany,
                                   PWORD pwId,
                                   PWORD pwVersion,
                                   PWORD pwKey,
                                   BOOL  fUser)
    {
    return w2kApplicationKey (pwCompany, pwId, pwVersion, pwKey,
                              fUser, FALSE);
    }

// -----------------------------------------------------------------

HKEY WINAPI w2kApplicationKeyCreate (PWORD pwCompany,
                                     PWORD pwId,
                                     PWORD pwVersion,
                                     PWORD pwKey,
                                     BOOL  fUser)
    {
    return w2kApplicationKey (pwCompany, pwId, pwVersion, pwKey,
                              fUser, TRUE);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kApplicationLoad (PWORD  pwCompany,
                                 PWORD  pwId,
                                 PWORD  pwVersion,
                                 PWORD  pwKey,
                                 BOOL   fUser,
                                 PWORD  pwValue,
                                 DWORD  dMode,
                                 PDWORD pdSize)
    {
    HKEY  hk;
    DWORD dSize = 0;
    PVOID pData = NULL;

    if ((hk = w2kApplicationKeyOpen (pwCompany, pwId,
                                     pwVersion, pwKey,
                                     fUser))
        != HKEY_NULL)
        {
        pData = w2kRegistryRead (hk, pwValue, dMode, &dSize);
        w2kRegistryClose (hk);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pData;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kApplicationLoadEx (PWORD pwCompany,
                                  PWORD pwId,
                                  PWORD pwVersion,
                                  PWORD pwKey,
                                  BOOL  fUser,
                                  PWORD pwValue,
                                  ...)
    {
    HKEY       hk;
    PW2K_VALUE pwv = (PW2K_VALUE) &pwValue;
    BOOL       fOk = FALSE;

    if ((hk = w2kApplicationKeyOpen (pwCompany, pwId,
                                     pwVersion, pwKey,
                                     fUser))
        != HKEY_NULL)
        {
        for (fOk = TRUE; pwv->pwValue != NULL; pwv++)
            {
            if (!w2kRegistryReadValue (hk, pwv)) fOk = FALSE;
            }
        w2kRegistryClose (hk);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kApplicationSave (PWORD pwCompany,
                                PWORD pwId,
                                PWORD pwVersion,
                                PWORD pwKey,
                                BOOL  fUser,
                                PWORD pwValue,
                                DWORD dMode,
                                PVOID pData)
    {
    HKEY hk;
    BOOL fOk = FALSE;

    if ((hk = w2kApplicationKeyCreate (pwCompany, pwId,
                                       pwVersion, pwKey,
                                       fUser))
        != HKEY_NULL)
        {
        fOk = w2kRegistryWrite (hk, pwValue, dMode, pData);
        w2kRegistryClose (hk);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kApplicationSaveEx (PWORD pwCompany,
                                  PWORD pwId,
                                  PWORD pwVersion,
                                  PWORD pwKey,
                                  BOOL  fUser,
                                  PWORD pwValue,
                                  ...)
    {
    HKEY       hk;
    PW2K_VALUE pwv = (PW2K_VALUE) &pwValue;
    BOOL       fOk = FALSE;

    if ((hk = w2kApplicationKeyCreate (pwCompany, pwId,
                                       pwVersion, pwKey,
                                       fUser))
        != HKEY_NULL)
        {
        for (fOk = TRUE; pwv->pwValue != NULL; pwv++)
            {
            if (!w2kRegistryWriteValue (hk, pwv)) fOk = FALSE;
            }
        w2kRegistryClose (hk);
        }
    return fOk;
    }

// -----------------------------------------------------------------

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
                                  PWORD pwSpecialBuild)
    {
    return
        w2kApplicationSaveEx
            (pwCompany, pwId, pwVersion, awNull, fUser,
             L"CompanyName",      W2K_MODE_TEXT, pwCompanyName,
             L"ProductName",      W2K_MODE_TEXT, pwProductName,
             L"ProductVersion",   W2K_MODE_TEXT, pwProductVersion,
             L"InternalName",     W2K_MODE_TEXT, pwInternalName,
             L"OriginalFilename", W2K_MODE_TEXT, pwOriginalFilename,
             L"FileVersion",      W2K_MODE_TEXT, pwFileVersion,
             L"FileDescription",  W2K_MODE_TEXT, pwFileDescription,
             L"LegalCopyright",   W2K_MODE_TEXT, pwLegalCopyright,
             L"LegalTrademarks",  W2K_MODE_TEXT, pwLegalTrademarks,
             L"Comments",         W2K_MODE_TEXT, pwComments,
             L"PrivateBuild",     W2K_MODE_TEXT, pwPrivateBuild,
             L"SpecialBuild",     W2K_MODE_TEXT, pwSpecialBuild,
             NULL);
    }

// -----------------------------------------------------------------

PW2K_SETTINGS WINAPI w2kApplicationSettings (PWORD pwCompany,
                                             PWORD pwId,
                                             PWORD pwVersion,
                                             PWORD pwKey,
                                             BOOL  fUser,
                                             PWORD pwValue)
    {
    HKEY          hk;
    PW2K_SETTINGS pws = NULL;

    if ((hk = w2kApplicationKeyOpen (pwCompany, pwId,
                                     pwVersion, pwKey,
                                     fUser))
        != HKEY_NULL)
        {
        pws = w2kRegistryReadBinary (hk, pwValue, W2K_SETTINGS_);
        w2kRegistryClose (hk);
        }
    return pws;
    }

// =================================================================
// CLASS MANAGEMENT
// =================================================================

PWORD WINAPI w2kClassText (PWORD  pwClsId,
                           PWORD  pwKey,
                           PWORD  pwValue,
                           PDWORD pdSize)
    {
    HKEY  hk;
    DWORD dSize  = 0;
    PWORD pwData = NULL;

    if ((pwClsId != NULL) && pwClsId [0]
        &&
        ((hk = w2kRegistryOpenEx (HKEY_CLASSES_ROOT,
                                  3, awKeyClsId, pwClsId, pwKey))
         != HKEY_NULL))
        {
        pwData = w2kRegistryReadText (hk, pwValue, &dSize);
        w2kRegistryClose (hk);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pwData;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kClassPath (PWORD  pwClsId,
                           PWORD  pwKey,
                           PWORD  pwValue,
                           PDWORD pdSize)
    {
    PWORD pwText;
    DWORD dSize  = 0;
    PWORD pwData = NULL;

    if ((pwText = w2kClassText (pwClsId, pwKey, pwValue, NULL))
        != NULL)
        {
        pwData = w2kPathEvaluate (pwText, &dSize);
        w2kMemoryDestroy (pwText);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pwData;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kClassApplication (PWORD  pwClsId,
                                  PDWORD pdSize)
    {
    return w2kClassPath (pwClsId, awKeyLocalServer32, awNull,
                         pdSize);
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kClassLibrary (PWORD  pwClsId,
                              PDWORD pdSize)
    {
    return w2kClassPath (pwClsId, awKeyInprocServer32, awNull,
                         pdSize);
    }

// =================================================================
// PROGRAM MANAGEMENT
// =================================================================

PWORD WINAPI w2kProgramClsId (PWORD  pwProgID,
                              PDWORD pdSize)
    {
    HKEY  hk;
    DWORD dSize  = 0;
    PWORD pwData = NULL;

    if ((pwProgID != NULL) && pwProgID [0]
        &&
        ((hk = w2kRegistryOpenEx (HKEY_CLASSES_ROOT,
                                  2, pwProgID, awKeyClsId))
         != HKEY_NULL))
        {
        pwData = w2kRegistryReadText (hk, awNull, &dSize);
        w2kRegistryClose (hk);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pwData;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kProgramApplication (PWORD  pwProgID,
                                    PDWORD pdSize)
    {
    PWORD pwClsId;
    DWORD dSize  = 0;
    PWORD pwData = NULL;

    if ((pwClsId = w2kProgramClsId (pwProgID, NULL)) != NULL)
        {
        pwData = w2kClassApplication (pwClsId, &dSize);
        w2kMemoryDestroy (pwClsId);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pwData;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kProgramLibrary (PWORD  pwProgID,
                                PDWORD pdSize)
    {
    PWORD pwClsId;
    DWORD dSize  = 0;
    PWORD pwData = NULL;

    if ((pwClsId = w2kProgramClsId (pwProgID, NULL)) != NULL)
        {
        pwData = w2kClassLibrary (pwClsId, &dSize);
        w2kMemoryDestroy (pwClsId);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pwData;
    }

// =================================================================
// VERSION INFO MANAGEMENT
// =================================================================

PW2K_SYSTEM WINAPI w2kVersionSystem (void)
    {
    DWORD           n;
    OSVERSIONINFO   ovi;
    OSVERSIONINFOEX ovix;
    PWORD           pwType;
    PW2K_SYSTEM     pws = NULL;

    ovi .dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    ovix.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);

    if (GetVersionEx (&ovi)                         &&
        (ovi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        ((pws = w2kMemoryCreate (W2K_SYSTEM_)) != NULL))
        {
        pws->dMajor = ovi.dwMajorVersion;
        pws->dMinor = ovi.dwMinorVersion;
        pws->dBuild = ovi.dwBuildNumber;

        if (pws->dMajor >= 5)
            {
            pws->dSystem = W2K_SYSTEM_WIN2000;

            lstrcpyn (pws->awName, awWindows2000, N_NAME);

            if (GetVersionEx ((POSVERSIONINFO) &ovix))
                {
                switch (ovix.wProductType)
                    {
                    case VER_NT_WORKSTATION:
                        {
                        pwType = awProfessional;
                        break;
                        }
                    case VER_NT_DOMAIN_CONTROLLER:
                        {
                        pwType = awDomainController;
                        break;
                        }
                    case VER_NT_SERVER:
                        {
                        pwType = awServer;
                        break;
                        }
                    default:
                        {
                        pwType = awNull;
                        break;
                        }
                    }
                n = lstrlen (pws->awName);

                lstrcpyn (pws->awName + n, pwType, N_NAME - n);
                }
            }
        else
            {
            if (pws->dMajor >= 4)
                {
                pws->dSystem = W2K_SYSTEM_WINNT4;
                }
            else
                {
                pws->dSystem = W2K_SYSTEM_WINNT3;
                }
            lstrcpyn (pws->awName, awWindowsNT, N_NAME);
            }
        if (pws->dMinor < 100)
            {
            if (pws->dMinor % 10)
                {
                _sprintf (pws->awVersion, L"%lu.%02lu",
                          pws->dMajor, pws->dMinor);
                }
            else
                {
                _sprintf (pws->awVersion, L"%lu.%lu",
                          pws->dMajor, pws->dMinor / 10);
                }
            }
        else
            {
            _sprintf (pws->awVersion, L"%lu.%lu",
                      pws->dMajor, pws->dMinor);
            }
        _sprintf (pws->awBuild, L"%lu.%02lu.%lu",
                  pws->dMajor, pws->dMinor, pws->dBuild);

        lstrcpyn (pws->awLevel, ovi.szCSDVersion, N_LEVEL);
        }
    return pws;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionToken (PWORD  pwVersion,
                              PDWORD pdOffset)
    {
    DWORD dOffset = (pdOffset != NULL ? *pdOffset : 0);
    QWORD qToken  = 0;

    if ((pwVersion != NULL) &&
        (dOffset < (DWORD) lstrlen (pwVersion)))
        {
        while ( pwVersion [dOffset]         &&
               (pwVersion [dOffset] != '.') &&
               (pwVersion [dOffset] != ',') &&
               ((pwVersion [dOffset] < '0') ||
                (pwVersion [dOffset] > '9')))
            {
            dOffset++;
            }
        while ((pwVersion [dOffset] >= '0') &&
               (pwVersion [dOffset] <= '9'))
            {
            qToken *= 10;
            qToken += (pwVersion [dOffset++] - '0');
            }
        while ( pwVersion [dOffset]         &&
               (pwVersion [dOffset] != '.') &&
               (pwVersion [dOffset] != ','))
            {
            dOffset++;
            }
        }
    if (pdOffset != NULL) *pdOffset = dOffset;
    return qToken;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionBinary (PWORD pwVersion,
                               BOOL  fFlushLeft)
    {
    DWORD dOffset, dShift;
    QWORD qToken;
    QWORD qVersion = VERSION_NULL;

    if (pwVersion != NULL)
        {
        dOffset  =  0;
        dShift   = 64;
        qVersion =  0;

        do  {
            qToken  = w2kVersionToken (pwVersion, &dOffset);
            dShift -= 16;

            if (!pwVersion [dOffset++])
                {
                dOffset--;
                if (!fFlushLeft) dShift = 0;
                }
            qVersion += (qToken << dShift);
            }
        while (dShift);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kVersionText (QWORD qVersion,
                             PWORD pwBuffer,
                             DWORD dBuffer)
    {
    WORD awVersion [N_VERSION_EX] = L"?";

    if ((pwBuffer != NULL) && dBuffer)
        {
        if (qVersion != VERSION_NULL)
            {
            _sprintf (awVersion, L"%hu.%hu.%hu.%hu",
                      W2K_VERSION1 (qVersion),
                      W2K_VERSION2 (qVersion),
                      W2K_VERSION3 (qVersion),
                      W2K_VERSION4 (qVersion));
            }
        lstrcpyn (pwBuffer, awVersion, dBuffer);
        }
    return pwBuffer;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionFile (PWORD pwPath)
    {
    PVOID             pData;
    VS_FIXEDFILEINFO *pInfo;
    DWORD             dData, dInfo, dHandle;
    QWORD             qVersion = VERSION_NULL;

    if ((pwPath != NULL) && pwPath [0] &&
        (dData = GetFileVersionInfoSize (pwPath, &dHandle)))
        {
        if ((pData = w2kMemoryCreate (dData)) != NULL)
            {
            if (GetFileVersionInfo (pwPath, 0, dData, pData)   &&
                VerQueryValue (pData, awRoot, &pInfo, &dInfo) &&
                (dInfo == sizeof (VS_FIXEDFILEINFO)))
                {
                qVersion   = pInfo->dwFileVersionMS;
                qVersion <<= 32;
                qVersion  += pInfo->dwFileVersionLS;
                }
            w2kMemoryDestroy (pData);
            }
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionModule (PWORD pwPath)
    {
    PWORD pwFile;
    QWORD qVersion = VERSION_NULL;

    if ((pwFile = w2kPathEvaluate (pwPath, NULL)) != NULL)
        {
        qVersion = w2kVersionFile (pwFile);
        w2kMemoryDestroy (pwFile);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionRegistryDword (PWORD pwPath)
    {
    DWORD dVersion;
    QWORD qVersion = VERSION_NULL;

    if (w2kRegistryReadDword (HKEY_NULL, pwPath, &dVersion))
        {
        qVersion = dVersion;
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionRegistryText (PWORD pwPath)
    {
    PWORD pwVersion;
    QWORD qVersion = VERSION_NULL;

    if ((pwVersion = w2kRegistryReadText (HKEY_NULL, pwPath, NULL))
        != NULL)
        {
        qVersion = w2kVersionBinary (pwVersion, TRUE);
        w2kMemoryDestroy (pwVersion);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionComponent (PWORD pwPath)
    {
    PWORD pwModule;
    QWORD qVersion = VERSION_NULL;

    if ((pwModule = w2kRegistryReadModule (HKEY_NULL, pwPath, NULL))
        != NULL)
        {
        qVersion = w2kVersionFile (pwModule);
        w2kMemoryDestroy (pwModule);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionResource (PWORD pwPath)
    {
    PWORD pwModule;
    DWORD i;
    QWORD qVersion = VERSION_NULL;

    if ((pwModule = w2kRegistryReadModule (HKEY_NULL, pwPath, NULL))
        != NULL)
        {
        for (i = 0; pwModule [i] && (pwModule [i] != ','); i++);
        while (i && (pwModule [i-1] == ' ')) i--;
        pwModule [i] = 0;

        qVersion = w2kVersionFile (pwModule);
        w2kMemoryDestroy (pwModule);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionApplication (PWORD pwProgID)
    {
    PWORD pwModule;
    QWORD qVersion = VERSION_NULL;

    if ((pwModule = w2kProgramApplication (pwProgID, NULL))
        != NULL)
        {
        qVersion = w2kVersionFile (pwModule);
        w2kMemoryDestroy (pwModule);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kVersionLibrary (PWORD pwProgID)
    {
    PWORD pwModule;
    QWORD qVersion = VERSION_NULL;

    if ((pwModule = w2kProgramLibrary (pwProgID, NULL))
        != NULL)
        {
        qVersion = w2kVersionFile (pwModule);
        w2kMemoryDestroy (pwModule);
        }
    return qVersion;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kVersionVerify (PVS_VERSIONINFO pvvi)
    {
    return (pvvi != NULL)                            &&
           (pvvi->Header.wLength >= VS_VERSIONINFO_) &&
           (!lstrcmpi (pvvi->Header.awKey, L"VS_VERSION_INFO"));
    }

// -----------------------------------------------------------------

PVS_VERSIONINFO WINAPI w2kVersionInfo (PWORD  pwPath,
                                       PDWORD pdSize)
    {
    DWORD           dHandle;
    DWORD           dSize = 0;
    PVS_VERSIONINFO pvvi  = NULL;

    if ((pwPath != NULL) && pwPath [0] &&
        (dSize = GetFileVersionInfoSize (pwPath, &dHandle)) &&
        ((pvvi = w2kMemoryCreate (dSize)) != NULL))
        {
        if ((!GetFileVersionInfo (pwPath, 0, dSize, pvvi)) ||
            (!w2kVersionVerify (pvvi)))
            {
            pvvi = w2kMemoryDestroy (pvvi);
            }
        }
    if (pdSize != NULL) *pdSize = (pvvi != NULL ? dSize : 0);
    return pvvi;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kVersionAlign (DWORD dSize)
    {
    return (((dSize - 1) >> 2) + 1) << 2;
    }

// -----------------------------------------------------------------

PVS_STRINGFILEINFO WINAPI w2kVersionStrings (PVS_VERSIONINFO pvvi)
    {
    DWORD              dOffset = 0;
    PVS_STRINGFILEINFO pvsfi   = NULL;

    if (w2kVersionVerify (pvvi))
        {
        while (VS_VERSIONINFO_ + dOffset + VS_STRINGFILEINFO_
               <= pvvi->Header.wLength)
            {
            pvsfi = (PVS_STRINGFILEINFO) (pvvi->abData + dOffset);

            if (!lstrcmpi (pvsfi->Header.awKey, L"StringFileInfo"))
                break;

            dOffset += w2kVersionAlign (pvsfi->Header.wLength);
            pvsfi    = NULL;
            }
        }
    return pvsfi;
    }

// -----------------------------------------------------------------

PVS_VARFILEINFO WINAPI w2kVersionVars (PVS_VERSIONINFO pvvi)
    {
    DWORD           dOffset = 0;
    PVS_VARFILEINFO pvvfi   = NULL;

    if (w2kVersionVerify (pvvi))
        {
        while (VS_VERSIONINFO_ + dOffset + VS_VARFILEINFO_
               <= pvvi->Header.wLength)
            {
            pvvfi = (PVS_VARFILEINFO) (pvvi->abData + dOffset);

            if (!lstrcmpi (pvvfi->Header.awKey, L"VarFileInfo"))
                break;

            dOffset += w2kVersionAlign (pvvfi->Header.wLength);
            pvvfi    = NULL;
            }
        }
    return pvvfi;
    }

// -----------------------------------------------------------------

PVS_TRANSLATION WINAPI w2kVersionTranslation (PVS_VERSIONINFO pvvi,
                                              PDWORD       pdCount)
    {
    PVS_VARFILEINFO pvvfi;
    DWORD           dCount = 0;
    PVS_TRANSLATION pvt    = NULL;

    if (((pvvfi = w2kVersionVars (pvvi)) != NULL) &&
        (!lstrcmpi (pvvfi->Var.Header.awKey, L"Translation")))
        {
        if (dCount = ((DWORD) &pvvfi->Var                +
                      (DWORD)  pvvfi->Var.Header.wLength -
                      (DWORD)  pvvfi->Var.Translation)
                     / VS_TRANSLATION_)
            {
            pvt = pvvfi->Var.Translation;
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pvt;
    }

// -----------------------------------------------------------------

PVS_STRINGTABLE WINAPI w2kVersionTable (PVS_VERSIONINFO pvvi,
                                        PWORD           pwTable)
    {
    DWORD              dOffset;
    PVS_STRINGFILEINFO pvsfi;
    PVS_STRINGTABLE    pvst = NULL;

    if ((pvsfi = w2kVersionStrings (pvvi)) != NULL)
        {
        dOffset = (PBYTE) &pvsfi->StringTable - (PBYTE) pvsfi;

        while (dOffset + VS_STRINGTABLE_ <= pvsfi->Header.wLength)
            {
            pvst = (PVS_STRINGTABLE) ((PBYTE) pvsfi + dOffset);

            if ((pwTable == NULL) ||
                (!lstrcmpi (pvst->Header.awKey, pwTable))) break;

            dOffset += w2kVersionAlign (pvst->Header.wLength);
            pvst     = NULL;
            }
        }
    return pvst;
    }

// -----------------------------------------------------------------

PVS_STRINGTABLE WINAPI w2kVersionTableEx (PVS_VERSIONINFO pvvi,
                                          DWORD           dIndex,
                                          PDWORD          pdCount)
    {
    WORD            awTable [8+1] = L"";
    DWORD           dCount;
    PVS_TRANSLATION pvt  = w2kVersionTranslation (pvvi, &dCount);
    PVS_STRINGTABLE pvst = NULL;

    if (dIndex == -1)
        {
        pvst = w2kVersionTable (pvvi, NULL);
        }
    else
        {
        if ((pvt != NULL) && (dIndex < dCount))
            {
            _sprintf (awTable, L"%04hX%04hX",
                      pvt [dIndex].wLanguage,
                      pvt [dIndex].wCodePage);

            pvst = w2kVersionTable (pvvi, awTable);
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pvst;
    }

// -----------------------------------------------------------------

PVS_STRING WINAPI w2kVersionTableString (PVS_STRINGTABLE pvst,
                                         PWORD           pwName)
    {
    DWORD      dOffset;
    PVS_STRING pvs = NULL;

    if (pvst != NULL)
        {
        dOffset = (PBYTE) &pvst->String - (PBYTE) pvst;

        while (dOffset + VS_STRING_ <= pvst->Header.wLength)
            {
            pvs = (PVS_STRING) ((PBYTE) pvst + dOffset);

            if (!lstrcmpi (pvs->Header.awKey, pwName)) break;

            dOffset += w2kVersionAlign (pvs->Header.wLength);
            pvs      = NULL;
            }
        }
    return pvs;
    }

// -----------------------------------------------------------------

PVS_STRING WINAPI w2kVersionString (PVS_VERSIONINFO pvvi,
                                    PWORD           pwName,
                                    PWORD           pwTable)
    {
    return w2kVersionTableString
               (w2kVersionTable (pvvi, pwTable),
                pwName);
    }

// -----------------------------------------------------------------

PVS_STRING WINAPI w2kVersionStringEx (PVS_VERSIONINFO pvvi,
                                      PWORD           pwName,
                                      DWORD           dIndex,
                                      PDWORD          pdCount)
    {
    return w2kVersionTableString
               (w2kVersionTableEx (pvvi, dIndex, pdCount),
                pwName);
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kVersionValue (PVS_STRING pvs)
    {
    DWORD n;
    PWORD pwValue = NULL;

    if (pvs != NULL)
        {
        n = w2kVersionAlign ((lstrlen (pvs->awData) + 1) * WORD_);
        pwValue = (PWORD) ((PBYTE) pvs->awData + n);
        }
    return pwValue;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kVersionValueCopy (PVS_STRING pvs,
                                 PWORD      pwDefault,
                                 PWORD      pwBuffer,
                                 DWORD      dBuffer)
    {
    PWORD pwValue = w2kVersionValue (pvs);

    _strcpyn (pwBuffer, pwValue, pwDefault, dBuffer);
    return pwValue != NULL;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kVersionLanguage (DWORD dId,
                                PWORD pwBuffer,
                                DWORD dBuffer)
    {
    BOOL fOk = FALSE;

    if ((pwBuffer != NULL) && dBuffer)
        {
        if (VerLanguageName (dId, pwBuffer, dBuffer))
            {
            fOk = TRUE;
            }
        else
            {
            pwBuffer [0] = 0;
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kVersionCodePage (DWORD dId)
    {
    DWORD i;
    PWORD pwCodePage = NULL;

    i = 0;

    do  {
        pwCodePage = avcpCodePages [i++].pwName;
        }
    while ((avcpCodePages [i-1].dId != dId) &&
           (avcpCodePages [i-1].dId != -1));

    return pwCodePage;
    }

// -----------------------------------------------------------------

PVS_VERSIONDATA WINAPI w2kVersionData (PWORD pwPath,
                                       DWORD dIndex)
    {
    DWORD           dCount, dIndex1;
    PWORD           pwData;
    PVS_TRANSLATION pvt;
    PVS_VERSIONINFO pvvi;
    PVS_VERSIONDATA pvvd = NULL;

    if ((pvvi = w2kVersionInfo (pwPath, NULL)) != NULL)
        {
        pvt     = w2kVersionTranslation (pvvi, &dCount);
        dIndex1 = (dIndex == -1 ? 0 : dIndex);

        if ((pvvd = w2kMemoryCreate (VS_VERSIONDATA_)) != NULL)
            {
            pvvd->vvqFile.dVersionL =
                pvvi->FixedFileInfo.dwFileVersionLS;

            pvvd->vvqFile.dVersionH =
                pvvi->FixedFileInfo.dwFileVersionMS;

            pvvd->vvqProduct.dVersionL =
                pvvi->FixedFileInfo.dwProductVersionLS;

            pvvd->vvqProduct.dVersionH =
                pvvi->FixedFileInfo.dwProductVersionMS;

            pvvd->dTableIndex = dIndex;
            pvvd->dTableCount = dCount;

            pvvd->Translation.wLanguage = 0;
            pvvd->Translation.wCodePage = 0;

            pvvd->awLanguage [0] = 0;
            pvvd->awCodePage [0] = 0;

            if (dCount && (dIndex1 < dCount))
                {
                pvvd->Translation = pvt [dIndex1];

                w2kVersionLanguage (pvvd->Translation.wLanguage,
                                    pvvd->awLanguage, N_STRING);

                if ((pwData = w2kVersionCodePage
                                  (pvvd->Translation.wCodePage))
                    != NULL)
                    {
                    lstrcpyn (pvvd->awCodePage, pwData, N_STRING);
                    }
                }
            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"Comments",
                                     dIndex, NULL),
                 NULL, pvvd->awComments, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"CompanyName",
                                     dIndex, NULL),
                 NULL, pvvd->awCompanyName, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"FileDescription",
                                     dIndex, NULL),
                 NULL, pvvd->awFileDescription, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"FileVersion",
                                     dIndex, NULL),
                 NULL, pvvd->awFileVersion, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"InternalName",
                                     dIndex, NULL),
                 NULL, pvvd->awInternalName, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"LegalCopyright",
                                     dIndex, NULL),
                 NULL, pvvd->awLegalCopyright, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"LegalTrademarks",
                                     dIndex, NULL),
                 NULL, pvvd->awLegalTrademarks, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"OriginalFilename",
                                     dIndex, NULL),
                 NULL, pvvd->awOriginalFilename, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"PrivateBuild",
                                     dIndex, NULL),
                 NULL, pvvd->awPrivateBuild, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"ProductName",
                                     dIndex, NULL),
                 NULL, pvvd->awProductName, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"ProductVersion",
                                     dIndex, NULL),
                 NULL, pvvd->awProductVersion, N_STRING);

            w2kVersionValueCopy
                (w2kVersionStringEx (pvvi, L"SpecialBuild",
                                     dIndex, NULL),
                 NULL, pvvd->awSpecialBuild, N_STRING);
            }
        w2kMemoryDestroy (pvvi);
        }
    return pvvd;
    }

// -----------------------------------------------------------------

INT WINAPI w2kVersionDisplay (HWND  hWnd,
                              UINT  uiType,
                              PWORD pwCaption,
                              PWORD pwPath,
                              DWORD dIndex)
    {
    PVS_VERSIONDATA pvvd;
    INT             iId = IDABORT;

    if ((pvvd = w2kVersionData (pwPath, dIndex)) != NULL)
        {
        iId = _mprintf (hWnd, uiType, pwCaption,
                        L"File Name        \t<%s>\n"
                        L"File Version     \t%u.%u.%u.%u\n"
                        L"Product Version  \t%u.%u.%u.%u\n"
                        L"Translation Code \t%04lX-%04lX\n"
                        L"Language Name    \t<%s>\n"
                        L"Code Page Name   \t<%s>\n"
                        L"\n"
                        L"Comments         \t<%s>\n"
                        L"CompanyName      \t<%s>\n"
                        L"FileDescription  \t<%s>\n"
                        L"FileVersion      \t<%s>\n"
                        L"InternalName     \t<%s>\n"
                        L"LegalCopyright   \t<%s>\n"
                        L"LegalTrademarks  \t<%s>\n"
                        L"OriginalFilename \t<%s>\n"
                        L"PrivateBuild     \t<%s>\n"
                        L"ProductName      \t<%s>\n"
                        L"ProductVersion   \t<%s>\n"
                        L"SpecialBuild     \t<%s>",
                        pwPath + w2kPathName (pwPath, NULL),
                        pvvd->vvqFile.wVersionHH,
                        pvvd->vvqFile.wVersionH,
                        pvvd->vvqFile.wVersionL,
                        pvvd->vvqFile.wVersionLL,
                        pvvd->vvqProduct.wVersionHH,
                        pvvd->vvqProduct.wVersionH,
                        pvvd->vvqProduct.wVersionL,
                        pvvd->vvqProduct.wVersionLL,
                        pvvd->Translation.wLanguage,
                        pvvd->Translation.wCodePage,
                        pvvd->awLanguage,
                        pvvd->awCodePage,
                        pvvd->awComments,
                        pvvd->awCompanyName,
                        pvvd->awFileDescription,
                        pvvd->awFileVersion,
                        pvvd->awInternalName,
                        pvvd->awLegalCopyright,
                        pvvd->awLegalTrademarks,
                        pvvd->awOriginalFilename,
                        pvvd->awPrivateBuild,
                        pvvd->awProductName,
                        pvvd->awProductVersion,
                        pvvd->awSpecialBuild);

        w2kMemoryDestroy (pvvd);
        }
    return iId;
    }

// =================================================================
// SERVICE/DRIVER MANAGEMENT
// =================================================================

SC_HANDLE WINAPI w2kServiceConnect (void)
    {
    return OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
    }

// -----------------------------------------------------------------

SC_HANDLE WINAPI w2kServiceDisconnect (SC_HANDLE hManager)
    {
    if (hManager != NULL) CloseServiceHandle (hManager);
    return NULL;
    }

// -----------------------------------------------------------------

SC_HANDLE WINAPI w2kServiceManager (SC_HANDLE  hManager,
                                    PSC_HANDLE phManager,
                                    BOOL       fOpen)
    {
    SC_HANDLE hManager1 = NULL;

    if (phManager != NULL)
        {
        if (fOpen)
            {
            if (hManager == NULL)
                {
                *phManager = w2kServiceConnect ();
                }
            else
                {
                *phManager = hManager;
                }
            }
        else
            {
            if (hManager == NULL)
                {
                *phManager = w2kServiceDisconnect (*phManager);
                }
            }
        hManager1 = *phManager;
        }
    return hManager1;
    }

// -----------------------------------------------------------------

SC_HANDLE WINAPI w2kServiceOpen (SC_HANDLE hManager,
                                 PWORD     pwName)
    {
    SC_HANDLE hManager1;
    SC_HANDLE hService = NULL;

    w2kServiceManager (hManager, &hManager1, TRUE);

    if ((hManager1 != NULL) && (pwName != NULL))
        {
        hService = OpenService (hManager1, pwName,
                                SERVICE_ALL_ACCESS);
        }
    w2kServiceManager (hManager, &hManager1, FALSE);
    return hService;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceClose (SC_HANDLE hService)
    {
    return (hService != NULL) && CloseServiceHandle (hService);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceAdd (SC_HANDLE hManager,
                           PWORD     pwName,
                           PWORD     pwInfo,
                           PWORD     pwPath)
    {
    SC_HANDLE hManager1, hService;
    PWORD     pwFile;
    WORD      awPath [MAX_PATH];
    DWORD     n;
    BOOL      fOk = FALSE;

    w2kServiceManager (hManager, &hManager1, TRUE);

    if ((hManager1 != NULL) && (pwName != NULL) &&
        (pwInfo    != NULL) && (pwPath != NULL) &&
        (n = GetFullPathName (pwPath, MAX_PATH, awPath, &pwFile)) &&
        (n < MAX_PATH))
        {
        if ((hService = CreateService (hManager1, pwName, pwInfo,
                                       SERVICE_ALL_ACCESS,
                                       SERVICE_KERNEL_DRIVER,
                                       SERVICE_DEMAND_START,
                                       SERVICE_ERROR_NORMAL,
                                       awPath, NULL, NULL,
                                       NULL, NULL, NULL))
            != NULL)
            {
            w2kServiceClose (hService);
            fOk = TRUE;
            }
        else
            {
            fOk = (GetLastError () ==
                   ERROR_SERVICE_EXISTS);
            }
        }
    w2kServiceManager (hManager, &hManager1, FALSE);
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceRemove (SC_HANDLE hManager,
                              PWORD     pwName)
    {
    SC_HANDLE hService;
    BOOL      fOk = FALSE;

    if ((hService = w2kServiceOpen (hManager, pwName)) != NULL)
        {
        if (DeleteService (hService))
            {
            fOk = TRUE;
            }
        else
            {
            fOk = (GetLastError () ==
                   ERROR_SERVICE_MARKED_FOR_DELETE);
            }
        w2kServiceClose (hService);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceStart (SC_HANDLE hManager,
                             PWORD     pwName)
    {
    SC_HANDLE hService;
    BOOL      fOk = FALSE;

    if ((hService = w2kServiceOpen (hManager, pwName)) != NULL)
        {
        if (StartService (hService, 1, &pwName))
            {
            fOk = TRUE;
            }
        else
            {
            fOk = (GetLastError () ==
                   ERROR_SERVICE_ALREADY_RUNNING);
            }
        w2kServiceClose (hService);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceControl (SC_HANDLE hManager,
                               PWORD     pwName,
                               DWORD     dControl)
    {
    SC_HANDLE      hService;
    SERVICE_STATUS ServiceStatus;
    BOOL           fOk = FALSE;

    if ((hService = w2kServiceOpen (hManager, pwName)) != NULL)
        {
        if (QueryServiceStatus (hService, &ServiceStatus))
            {
            switch (ServiceStatus.dwCurrentState)
                {
                case SERVICE_STOP_PENDING:
                case SERVICE_STOPPED:
                    {
                    fOk = (dControl == SERVICE_CONTROL_STOP);
                    break;
                    }
                case SERVICE_PAUSE_PENDING:
                case SERVICE_PAUSED:
                    {
                    fOk = (dControl == SERVICE_CONTROL_PAUSE);
                    break;
                    }
                case SERVICE_START_PENDING:
                case SERVICE_CONTINUE_PENDING:
                case SERVICE_RUNNING:
                    {
                    fOk = (dControl == SERVICE_CONTROL_CONTINUE);
                    break;
                    }
                }
            }
        fOk = fOk ||
              ControlService (hService, dControl, &ServiceStatus);

        w2kServiceClose (hService);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceStop (SC_HANDLE hManager,
                            PWORD     pwName)
    {
    return w2kServiceControl (hManager, pwName,
                              SERVICE_CONTROL_STOP);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServicePause (SC_HANDLE hManager,
                             PWORD     pwName)
    {
    return w2kServiceControl (hManager, pwName,
                              SERVICE_CONTROL_PAUSE);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceContinue (SC_HANDLE hManager,
                                PWORD     pwName)
    {
    return w2kServiceControl (hManager, pwName,
                              SERVICE_CONTROL_CONTINUE);
    }

// -----------------------------------------------------------------

SC_HANDLE WINAPI w2kServiceLoad (PWORD pwName,
                                 PWORD pwInfo,
                                 PWORD pwPath,
                                 BOOL  fStart)
    {
    BOOL      fOk;
    SC_HANDLE hManager = NULL;

    if ((hManager = w2kServiceConnect ()) != NULL)
        {
        fOk = w2kServiceAdd (hManager, pwName, pwInfo, pwPath);

        if (fOk && fStart)
            {
            if (!(fOk = w2kServiceStart (hManager, pwName)))
                {
                w2kServiceRemove (hManager, pwName);
                }
            }
        if (!fOk)
            {
            hManager = w2kServiceDisconnect (hManager);
            }
        }
    return hManager;
    }

// -----------------------------------------------------------------

SC_HANDLE WINAPI w2kServiceLoadEx (PWORD pwPath,
                                   BOOL  fStart)
    {
    PVS_VERSIONDATA pvvd;
    PWORD           pwPath1, pwInfo;
    WORD            awName [MAX_PATH];
    DWORD           dName, dExtension;
    SC_HANDLE       hManager = NULL;

    if (pwPath != NULL)
        {
        dName = w2kPathName (pwPath, &dExtension);

        lstrcpyn (awName, pwPath + dName,
                  min (MAX_PATH, dExtension - dName + 1));

        pwPath1 = w2kPathEvaluate (pwPath,  NULL);
        pvvd    = w2kVersionData  (pwPath1, -1);

        pwInfo  = ((pvvd != NULL) && pvvd->awFileDescription [0]
                   ? pvvd->awFileDescription
                   : awName);

        hManager = w2kServiceLoad (awName, pwInfo, pwPath1, fStart);

        w2kMemoryDestroy (pvvd);
        w2kMemoryDestroy (pwPath1);
        }
    return hManager;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceUnload (PWORD     pwName,
                              SC_HANDLE hManager)
    {
    SC_HANDLE hManager1 = hManager;
    BOOL      fOk       = FALSE;

    if (pwName != NULL)
        {
        if (hManager1 == NULL)
            {
            hManager1 = w2kServiceConnect ();
            }
        if (hManager1 != NULL)
            {
            w2kServiceStop (hManager1, pwName);
            fOk = w2kServiceRemove (hManager1, pwName);
            }
        }
    w2kServiceDisconnect (hManager1);
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kServiceUnloadEx (PWORD     pwPath,
                                SC_HANDLE hManager)
    {
    DWORD dName, dExtension;
    WORD  awName [MAX_PATH];
    PWORD pwName = NULL;

    if (pwPath != NULL)
        {
        dName = w2kPathName (pwPath, &dExtension);

        lstrcpyn (pwName = awName, pwPath + dName,
                  min (MAX_PATH, dExtension - dName + 1));
        }
    return w2kServiceUnload (pwName, hManager);
    }

// -----------------------------------------------------------------

PW2K_SERVICES WINAPI w2kServiceList (BOOL fDriver,
                                     BOOL fWin32,
                                     BOOL fActive,
                                     BOOL fInactive)
    {
    SC_HANDLE     hManager;
    DWORD         dType, dState, dBytes, dResume, dName, i;
    PW2K_SERVICES pws = NULL;

    if ((pws = w2kMemoryCreate (W2K_SERVICES_)) != NULL)
        {
        pws->dEntries     = 0;
        pws->dBytes       = 0;
        pws->dDisplayName = 0;
        pws->dServiceName = 0;

        if ((fDriver || fWin32) && (fActive || fInactive))
            {
            if ((hManager = w2kServiceConnect ()) != NULL)
                {
                dType   = (fDriver ? SERVICE_DRIVER : 0) |
                          (fWin32  ? SERVICE_WIN32  : 0);

                dState  = (fActive && fInactive
                           ? SERVICE_STATE_ALL
                           : (fActive
                              ? SERVICE_ACTIVE
                              : SERVICE_INACTIVE));

                dBytes  = pws->dBytes;

                while (pws != NULL)
                    {
                    pws->dEntries     = 0;
                    pws->dBytes       = dBytes;
                    pws->dDisplayName = 0;
                    pws->dServiceName = 0;

                    dResume = 0;

                    if (EnumServicesStatus (hManager, dType, dState,
                                            pws->aess, pws->dBytes,
                                            &dBytes, &pws->dEntries,
                                            &dResume))
                        break;

                    dBytes += pws->dBytes;
                    pws     = w2kMemoryDestroy (pws);

                    if (GetLastError () != ERROR_MORE_DATA) break;

                    pws = w2kMemoryCreate (W2K_SERVICES_ + dBytes);
                    }
                w2kServiceDisconnect (hManager);
                }
            else
                {
                pws = w2kMemoryDestroy (pws);
                }
            }
        if (pws != NULL)
            {
            for (i = 0; i < pws->dEntries; i++)
                {
                dName = lstrlen (pws->aess [i].lpDisplayName);
                pws->dDisplayName = max (pws->dDisplayName, dName);

                dName = lstrlen (pws->aess [i].lpServiceName);
                pws->dServiceName = max (pws->dServiceName, dName);
                }
            }
        }
    return pws;
    }

// =================================================================
// LINKED LIST MANAGEMENT
// =================================================================

void WINAPI w2kChainLock (void)
    {
    EnterCriticalSection (&gcsChain);
    return;
    }

// -----------------------------------------------------------------

void WINAPI w2kChainUnlock (void)
    {
    LeaveCriticalSection (&gcsChain);
    return;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kChainThis (PW2K_NODE pwn)
    {
    return (pwn != NULL ? pwn->pThis : NULL);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kChainAdd (PW2K_NODE pwn,
                         PW2K_NODE pwnBack,
                         PVOID     pThis,
                         DWORD     dType)
    {
    BOOL fOk = FALSE;

    if (pwn != NULL)
        {
        w2kChainLock ();

        if (pwnBack != NULL)
            {
            pwn->pwnBack = pwnBack;

            if ((pwn->pwnNext = pwnBack->pwnNext) != NULL)
                {
                pwnBack->pwnNext->pwnBack = pwn;
                }
            pwnBack->pwnNext = pwn;
            }
        else
            {
            pwn->pwnBack = NULL;
            pwn->pwnNext = NULL;
            }
        pwn->pThis = pThis;
        pwn->dType = dType;

        w2kChainUnlock ();

        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kChainRemove (PW2K_NODE pwn)
    {
    return w2kChainRemoveEx (pwn, NULL, NULL);
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kChainRemoveEx (PW2K_NODE pwn,
                              W2K_WALK  Handler,
                              PVOID     pData)
    {
    BOOL fOk = FALSE;

    if (pwn != NULL)
        {
        w2kChainLock ();

        fOk = (Handler == NULL) || Handler (pwn->pThis, pData);

        if (pwn->pwnBack != NULL)
            {
            pwn->pwnBack->pwnNext = pwn->pwnNext;
            }
        if (pwn->pwnNext != NULL)
            {
            pwn->pwnNext->pwnBack = pwn->pwnBack;
            }
        pwn->pwnBack = NULL;
        pwn->pwnNext = NULL;

        w2kChainUnlock ();
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kChainMove (PW2K_NODE pwn,
                          PW2K_NODE pwnBack)
    {
    BOOL fOk = FALSE;

    if (pwn != NULL)
        {
        fOk = w2kChainRemove (pwn) &&
              w2kChainAdd    (pwn, pwnBack, pwn->pThis, pwn->dType);
        }
    return fOk;
    }

// -----------------------------------------------------------------

PW2K_NODE WINAPI w2kChainBack (PW2K_NODE pwn)
    {
    PW2K_NODE pwn1 = pwn;

    if (pwn1 != NULL)
        {
        w2kChainLock ();
        pwn1 = pwn1->pwnBack;
        w2kChainUnlock ();
        }
    return pwn1;
    }

// -----------------------------------------------------------------

PW2K_NODE WINAPI w2kChainNext (PW2K_NODE pwn)
    {
    PW2K_NODE pwn1 = pwn;

    if (pwn1 != NULL)
        {
        w2kChainLock ();
        pwn1 = pwn1->pwnNext;
        w2kChainUnlock ();
        }
    return pwn1;
    }

// -----------------------------------------------------------------

PW2K_NODE WINAPI w2kChainFirst (PW2K_NODE pwn)
    {
    PW2K_NODE pwn1 = pwn;

    if (pwn1 != NULL)
        {
        w2kChainLock ();
        while (pwn1->pwnBack != NULL) pwn1 = pwn1->pwnBack;
        w2kChainUnlock ();
        }
    return pwn1;
    }

// -----------------------------------------------------------------

PW2K_NODE WINAPI w2kChainLast (PW2K_NODE pwn)
    {
    PW2K_NODE pwn1 = pwn;

    if (pwn1 != NULL)
        {
        w2kChainLock ();
        while (pwn1->pwnNext != NULL) pwn1 = pwn1->pwnNext;
        w2kChainUnlock ();
        }
    return pwn1;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kChainWalk (PW2K_NODE pwn,
                           BOOL      fComplete,
                           BOOL      fBack,
                           W2K_WALK  Handler,
                           PVOID     pData)
    {
    PW2K_NODE pwn1 = pwn;
    DWORD     n    = 0;

    if (pwn1 != NULL)
        {
        w2kChainLock ();

        if (fComplete)
            {
            if (fBack)
                {
                while (pwn1->pwnNext != NULL) pwn1 = pwn1->pwnNext;
                }
            else
                {
                while (pwn1->pwnBack != NULL) pwn1 = pwn1->pwnBack;
                }
            }
        while (pwn1 != NULL)
            {
            if ((Handler != NULL) &&
                (!Handler (pwn1->pThis, pData))) break;
            n++;

            if (fBack)
                {
                pwn1 = pwn1->pwnBack;
                }
            else
                {
                pwn1 = pwn1->pwnNext;
                }
            }
        w2kChainUnlock ();
        }
    return n;
    }

// =================================================================
// DLL MANAGEMENT
// =================================================================

BOOL WINAPI DllMain (HINSTANCE hInstance,
                     DWORD     dReason,
                     PVOID     pReserved)
    {
    BOOL fOk = TRUE;

    if (dReason == DLL_PROCESS_ATTACH)
        {
        ghInstance = hInstance;

        InitializeCriticalSection (&gcsMemory);
        InitializeCriticalSection (&gcsChain);
        }
    if (dReason == DLL_PROCESS_DETACH)
        {
        w2kConsoleClose ();

        DeleteCriticalSection (&gcsMemory);
        DeleteCriticalSection (&gcsChain);
        }
    return fOk;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
