
// __________________________________________________________
//
//                         w2k_dbg.c
//          SBS Windows 2000 Debugging Library V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#define  _W2K_DBG_DLL_
#include "w2k_dbg.h"

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

08-27-2000 V1.00 Original version (SBS).

*/

// =================================================================
// GLOBAL VARIABLES
// =================================================================

LONG  glMemorySign = 0;
DWORD gdMemoryNow  = 0;
DWORD gdMemoryMax  = 0;

CRITICAL_SECTION gcsMemory;

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
// CRC32 COMPUTATION
// =================================================================

DWORD WINAPI dbgCrc32Start (PDWORD pdCrc32)
    {
    *pdCrc32 = 0xFFFFFFFF;
    return *pdCrc32;
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgCrc32Stop (PDWORD pdCrc32)
    {
    *pdCrc32 = ~*pdCrc32;
    return (*pdCrc32 == 0xDEBB20E3);
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgCrc32Byte (PDWORD pdCrc32,
                           BYTE   bData)
    {
    *pdCrc32 = (*pdCrc32 >> 8) ^ adCrc32 [(BYTE) *pdCrc32 ^ bData];
    return *pdCrc32;
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgCrc32Block (PDWORD pdCrc32,
                            PVOID  pData,
                            DWORD  dData)
    {
    DWORD dCrc32, i;

    if (pdCrc32 != NULL) dCrc32 = *pdCrc32;
    else dbgCrc32Start (&dCrc32);

    if (pData != NULL)
        {
        for (i = 0; i < dData; i++)
            {
            dbgCrc32Byte (&dCrc32, ((PBYTE) pData) [i]);
            }
        }
    if (pdCrc32 != NULL) *pdCrc32 = dCrc32;
    else dbgCrc32Stop (&dCrc32);

    return dCrc32;
    }

// =================================================================
// PRIVILEGE MANAGEMENT
// =================================================================

BOOL WINAPI dbgPrivilegeSet (PWORD pwName)
    {
    HANDLE           hToken;
    TOKEN_PRIVILEGES tp;
    BOOL             fOk = FALSE;

    if ((pwName != NULL)
        &&
        OpenProcessToken (GetCurrentProcess (),
                          TOKEN_ADJUST_PRIVILEGES,
                          &hToken))
        {
        if (LookupPrivilegeValue (NULL, pwName,
                                  &tp.Privileges->Luid))
            {
            tp.Privileges->Attributes = SE_PRIVILEGE_ENABLED;
            tp.PrivilegeCount         = 1;

            fOk = AdjustTokenPrivileges (hToken, FALSE, &tp,
                                         0, NULL, NULL)
                  &&
                  (GetLastError () == ERROR_SUCCESS);
            }
        CloseHandle (hToken);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgPrivilegeDebug (void)
    {
    return dbgPrivilegeSet (SE_DEBUG_NAME);
    }

// =================================================================
// MEMORY MANAGEMENT
// =================================================================

PVOID WINAPI dbgMemoryCreate (DWORD dSize)
    {
    return dbgMemoryCreateEx (dSize, DBG_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PVOID WINAPI dbgMemoryCreateEx (DWORD dSize,
                                DWORD dTag)
    {
    PDBG_MEMORY pwm = NULL;

    if ((DBG_MEMORY_ + dSize >= DBG_MEMORY_)
        &&
        ((pwm = LocalAlloc (LMEM_FIXED, DBG_MEMORY_ + dSize))
         != NULL))
        {
        pwm->dTag  = dTag;
        pwm->dSize = dSize;

        dbgMemoryTrack (pwm->dSize, TRUE);
        }
    return (pwm != NULL ? pwm->abData : NULL);
    }

// -----------------------------------------------------------------

PDBG_MEMORY WINAPI dbgMemoryBase (PVOID pData)
    {
    return dbgMemoryBaseEx (pData, DBG_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PDBG_MEMORY WINAPI dbgMemoryBaseEx (PVOID pData,
                                    DWORD dTag)
    {
    PDBG_MEMORY pwm = (PDBG_MEMORY) ((PBYTE) pData - DBG_MEMORY_);

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

PVOID WINAPI dbgMemoryResize (PVOID pData,
                              DWORD dSize,
                              PBOOL pfOk)
    {
    return dbgMemoryResizeEx (pData, dSize, pfOk, DBG_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PVOID WINAPI dbgMemoryResizeEx (PVOID pData,
                                DWORD dSize,
                                PBOOL pfOk,
                                DWORD dTag)
    {
    PDBG_MEMORY pwmNew;
    BOOL        fOk = FALSE;
    PDBG_MEMORY pwm = dbgMemoryBaseEx (pData, dTag);

    if (pwm != NULL)
        {
        fOk = TRUE;

        if (pwm->dSize != dSize)
            {
            if ((pwmNew = LocalReAlloc (pwm, DBG_MEMORY_ + dSize,
                                        LMEM_MOVEABLE))
                != NULL)
                {
                pwm = pwmNew;

                if (dSize >= pwm->dSize)
                    {
                    dbgMemoryTrack (dSize - pwm->dSize, TRUE);
                    }
                else
                    {
                    dbgMemoryTrack (pwm->dSize - dSize, FALSE);
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
                    dbgMemoryTrack (pwm->dSize, FALSE);

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

PVOID WINAPI dbgMemoryDestroy (PVOID pData)
    {
    return dbgMemoryDestroyEx (pData, DBG_MEMORY_TAG);
    }

// -----------------------------------------------------------------

PVOID WINAPI dbgMemoryDestroyEx (PVOID pData,
                                 DWORD dTag)
    {
    PDBG_MEMORY pwm;

    if ((pwm = dbgMemoryBaseEx (pData, dTag)) != NULL)
        {
        dbgMemoryTrack (pwm->dSize, FALSE);

        pwm->dSize = MAXDWORD;
        LocalFree (pwm);
        }
    return NULL;
    }

// -----------------------------------------------------------------

void WINAPI dbgMemoryReset (void)
    {
    EnterCriticalSection (&gcsMemory);

    glMemorySign = 0;
    gdMemoryNow  = 0;
    gdMemoryMax  = 0;

    LeaveCriticalSection (&gcsMemory);
    return;
    }

// -----------------------------------------------------------------

void WINAPI dbgMemoryTrack (DWORD dSize,
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

BOOL WINAPI dbgMemoryStatus (PDWORD pdMemoryNow,
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

DWORD WINAPI dbgMemoryAlign (DWORD dSize)
    {
    return (((dSize - 1) >> SIZE_ALIGNMENT) + 1) << SIZE_ALIGNMENT;
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgMemoryAlignEx (DWORD dFixed,
                               DWORD dText)
    {
    return dbgMemoryAlign (dFixed + (dText * sizeof (WORD)));
    }

// =================================================================
// FILE MANAGEMENT
// =================================================================

DWORD WINAPI dbgFileRoot (PWORD pwPath)
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

HANDLE WINAPI dbgFileOpen (PWORD pwPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pwPath != NULL) && pwPath [0])
        {
        hf = CreateFile (pwPath, GENERIC_READ,
                         FILE_SHARE_READ, NULL, OPEN_EXISTING,
                         FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI dbgFileNew (PWORD pwPath)
    {
    DWORD  i;
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pwPath != NULL) && pwPath [0])
        {
        i = dbgFileRoot (pwPath);

        while (pwPath [i])
            {
            while (pwPath [i] && (pwPath [++i] != '\\'));

            if (pwPath [i] == '\\')
                {
                pwPath [i] = 0;
                CreateDirectory (pwPath, NULL);
                pwPath [i] = '\\';
                }
            }
        hf = CreateFile (pwPath, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                         FILE_FLAG_SEQUENTIAL_SCAN |
                         FILE_ATTRIBUTE_NORMAL, NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI dbgFileClose (HANDLE hf)
    {
    if (hf != INVALID_HANDLE_VALUE) CloseHandle (hf);
    return INVALID_HANDLE_VALUE;
    }

// -----------------------------------------------------------------

PVOID WINAPI dbgFileUnload (PVOID pData)
    {
    return dbgMemoryDestroy (pData);
    }

// -----------------------------------------------------------------

PVOID WINAPI dbgFileLoad (PWORD  pwPath,
                          PDWORD pdData)
    {
    HANDLE hf;
    DWORD  n;
    DWORD  dData  = 0;
    PBYTE  pbData = NULL;

    if ((hf = dbgFileOpen (pwPath)) != INVALID_HANDLE_VALUE)
        {
        dData = GetFileSize (hf, NULL);

        if ((dData != INVALID_FILE_SIZE) && (dData < MAXDWORD) &&
            ((pbData = dbgMemoryCreate (dData+1)) != NULL))
            {
            if (ReadFile (hf, pbData, dData, &n, NULL) &&
                (dData == n))
                {
                pbData [dData] = 0;
                }
            else
                {
                pbData = dbgFileUnload (pbData);
                }
            }
        dbgFileClose (hf);

        if (pbData == NULL) dData = 0;
        }
    if (pdData != NULL) *pdData = dData;
    return pbData;
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgFileSave (PWORD pwPath,
                         PVOID pData,
                         DWORD dData)
    {
    HANDLE hf;
    DWORD  dData1, n;
    BOOL   fOk = FALSE;

    if ((pData != NULL) &&
        ((hf = dbgFileNew (pwPath)) != INVALID_HANDLE_VALUE))
        {
        dData1 = (dData != MAXDWORD ? dData : lstrlenA (pData));

        fOk = WriteFile (hf, pData, dData1, &n, NULL) &&
              (dData1 == n);

        dbgFileClose (hf);
        }
    return fOk;
    }

// =================================================================
// SIZE CONVERSION
// =================================================================

DWORD WINAPI dbgSizeDivide (DWORD dShift,
                            DWORD dSize,
                            BOOL  fRoundUp,
                            BOOL  fRoundDown)
    {
    DWORD dFactor;
    DWORD dSize1 = (dShift ? 0 : dSize);

    if (dShift && (dShift < 32))
        {
        dFactor = 1     << dShift;
        dSize1  = dSize >> dShift;

        if ((dSize & (dFactor-1)) && (fRoundUp || !fRoundDown))
            {
            if ((fRoundUp && !fRoundDown) ||
                (dSize & (dFactor >> 1))) dSize1++;
            }
        }
    return dSize1;
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgSizeKB (DWORD dBytes,
                        BOOL  fRoundUp,
                        BOOL  fRoundDown)
    {
    return dbgSizeDivide (10, dBytes, fRoundUp, fRoundDown);
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgSizeMB (DWORD dBytes,
                        BOOL  fRoundUp,
                        BOOL  fRoundDown)
    {
    return dbgSizeDivide (20, dBytes, fRoundUp, fRoundDown);
    }

// =================================================================
// STRING OPERATIONS
// =================================================================

PBYTE WINAPI dbgStringAnsi (PWORD pwData,
                            PBYTE pbData)
    {
    DWORD n;
    PBYTE pbData1 = NULL;

    if (pwData != NULL)
        {
        for (n = 0; pwData [n]; n++);

        if ((pbData1 = (pbData != NULL ? pbData
                                       : dbgMemoryCreate (n+1)))
            != NULL)
            {
            if (!WideCharToMultiByte
                     (CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
                      pwData, -1, pbData1, n+1, NULL, NULL))
                {
                do  {
                    pbData1 [n] = (pwData [n] < 0x0100
                                   ? (BYTE) pwData [n]
                                   : UNICODE_UNMAPPED);
                    }
                while (n--);
                }
            }
        }
    return pbData1;
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgStringMatch (PWORD pwFilter,
                            PWORD pwData,
                            BOOL  fCase)
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
                               (!dbgStringMatch (pwFilter + i,
                                                 pwData   + j,
                                                 fCase)))
                            {
                            j++;
                            }
                        }
                    return pwData [j] != 0;
                    }
                }
            if (fCase ?        pwFilter [i]  !=        pwData [j]
                      : LCASE (pwFilter [i]) != LCASE (pwData [j]))
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

// -----------------------------------------------------------------

PWORD WINAPI dbgStringDay (DWORD dDay)
    {
    PWORD pwDay = L"";

    switch (dDay % 7)
        {
        case 0: pwDay = L"Sunday";    break;
        case 1: pwDay = L"Monday";    break;
        case 2: pwDay = L"Tuesday";   break;
        case 3: pwDay = L"Wednesday"; break;
        case 4: pwDay = L"Thursday";  break;
        case 5: pwDay = L"Friday";    break;
        case 6: pwDay = L"Saturday";  break;
        }
    return pwDay;
    }

// =================================================================
// PATH MANAGEMENT
// =================================================================

DWORD WINAPI dbgPathFile (PWORD pwPath)
    {
    DWORD dFile = (pwPath != NULL ? lstrlen (pwPath) : 0);

    while (dFile && (pwPath [dFile-1] != '\\')
                 && (pwPath [dFile-1] != ':' )) dFile--;

    return dFile;
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgPathDriver (PWORD pwFile,
                            PWORD pwBuffer,
                            DWORD dBuffer)
    {
    WORD  awPath [MAX_PATH] = L"drivers\\";
    DWORD n1, n2, i;
    DWORD n = 0;

    if (pwFile != NULL)
        {
        i = n2 = lstrlen (pwFile);

        while (i && (pwFile [i-1] != '\\')
                 && (pwFile [i-1] != ':' )) i--;

        if ((!i) && ((n1 = lstrlen (awPath)) + n2 < MAX_PATH))
            {
            lstrcpy (awPath + n1, pwFile);
            n = n1 + n2 + 1;
            }
        }
    if (!n) awPath [0] = 0;

    if (dBuffer)
        {
        lstrcpyn ((pwBuffer != NULL ? pwBuffer : pwFile),
                  awPath, dBuffer);
        }
    return n;
    }

// =================================================================
// LIST MANAGEMENT
// =================================================================

PDBG_LIST WINAPI dbgListCreate (void)
    {
    return dbgListCreateEx (0, 0, 0, NULL, NULL);
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgListCreateEx (DWORD       dData,
                                  DWORD       dEntries,
                                  DWORD       dContext,
                                  PVOID       pContext,
                                  PSYSTEMTIME pst)
    {
    DWORD     dSize = dbgMemoryAlign  (DBG_LIST_);
    PDBG_LIST pdl   = dbgMemoryCreate (dSize + dData);

    if (pdl != NULL)
        {
        ZeroMemory (pdl, dSize + dData);

        pdl->dTag     = DBG_LIST_TAG;
        pdl->dFirst   = dSize - DBG_LIST_;
        pdl->dMemory  = pdl->dFirst + dData;
        pdl->dOffset  = pdl->dFirst;
        pdl->dEntries = dEntries;
        pdl->dCrc32   = 0;
        pdl->dContext = dContext;
        pdl->pContext = pContext;

        if (pst != NULL)
            {
            pdl->st = *pst;
            }
        else
            {
            GetSystemTime (&pdl->st);
            }
        }
    return pdl;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgListDestroy (PDBG_LIST pdl)
    {
    return dbgMemoryDestroy (pdl);
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgListResize (PDBG_LIST pdl,
                                DWORD     dData)
    {
    PDBG_LIST pdl1 = pdl;

    if (dData)
        {
        if (pdl != NULL)
            {
            if (pdl->dMemory - pdl->dOffset < dData)
                {
                pdl->dMemory += max (SIZE_INCREMENT, dData);

                pdl1 = dbgMemoryResize (pdl,
                                        DBG_LIST_ + pdl->dMemory,
                                        NULL);
                }
            }
        if (pdl1 != NULL)
            {
            ZeroMemory (pdl1->abData + pdl1->dOffset, dData);
            }
        }
    return pdl1;
    }

// -----------------------------------------------------------------

DWORD WINAPI dbgListNext (PDBG_LIST pdl,
                          DWORD     dData,
                          BOOL      fCount)
    {
    DWORD dEntries = 0;

    if (pdl != NULL)
        {
        pdl->dOffset += dData;
        if (fCount) pdl->dEntries++;
        dEntries =  pdl->dEntries;
        }
    return dEntries;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgListFinish (PDBG_LIST pdl)
    {
    PDBG_LIST pdl1 = pdl;

    if ((pdl1 != NULL) && (pdl1->dMemory != pdl1->dOffset))
        {
        pdl1->dMemory = pdl1->dOffset;

        pdl1 = dbgMemoryResize (pdl1,
                                DBG_LIST_ + pdl1->dOffset,
                                NULL);
        }
    if (pdl1 != NULL)
        {
        pdl1->dCrc32 = dbgCrc32Block (NULL, pdl1->abData,
                                            pdl1->dMemory);
        }
    return pdl1;
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgListIndex (PDBG_LIST pdl,
                                DWORD     dMemberNext)
    {
    DWORD      dOffset, dNext, i;
    PDBG_INDEX pdi = NULL;

    if ((pdl != NULL)
        &&
        ((pdi = dbgMemoryCreate (DBG_INDEX__ (pdl->dEntries)))
         != NULL))
        {
        pdi->ppbData     = pdi->apbData;
        pdi->pdl         = pdl;
        pdi->pdlDestroy  = NULL;
        pdi->dMemberNext = dMemberNext;
        pdi->dData       = 0;
        pdi->dEntries    = pdl->dEntries;
        pdi->dContext    = pdl->dContext;
        pdi->pContext    = pdl->pContext;

        dOffset = pdl->dFirst;

        for (i = 0; i < pdl->dEntries; i++)
            {
            pdi->apbData [i] = pdl->abData + dOffset;
            dNext = *(PDWORD) (pdl->abData + dOffset + dMemberNext);

            pdi->dData += dNext;
            dOffset    += dNext;
            }
        }
    return pdi;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgListLoad (PWORD pwPath)
    {
    DWORD     dData;
    PDBG_LIST pdl = dbgFileLoad (pwPath, &dData);

    if ((pdl != NULL)
        &&
        ((dData       <  DBG_LIST_               ) ||
         (pdl->dTag   != DBG_LIST_TAG            ) ||
         (dData       != DBG_LIST_ + pdl->dMemory) ||
         (pdl->dCrc32 != dbgCrc32Block (NULL, pdl->abData,
                                              pdl->dMemory))))
        {
        pdl = dbgFileUnload (pdl);
        }
    return pdl;
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgListSave (PWORD     pwPath,
                         PDBG_LIST pdl)
    {
    DWORD dContext;
    PVOID pContext;
    BOOL  fOk = FALSE;

    if (pdl != NULL)
        {
        dContext = pdl->dContext;
        pContext = pdl->pContext;

        pdl->dContext = 0;
        pdl->pContext = NULL;

        fOk = dbgFileSave (pwPath, pdl, DBG_LIST_ + pdl->dMemory);

        pdl->dContext = dContext;
        pdl->pContext = pContext;
        }
    return fOk;
    }

// =================================================================
// LIST INDEX MANAGEMENT
// =================================================================

PDBG_INDEX WINAPI dbgIndexCreate (PDBG_LIST pdl,
                                  DWORD     dMemberNext)
    {
    PDBG_INDEX pdi = dbgListIndex (pdl, dMemberNext);

    if (pdi != NULL) pdi->pdlDestroy = pdl;
    return pdi;
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgIndexCreateEx (PDBG_LIST pdl,
                                    DWORD     dMemberNext,
                                    DWORD     dMemberAddress,
                                    DWORD     dMemberSize,
                                    DWORD     dMemberId,
                                    DWORD     dMemberNameData,
                                    DWORD     dMemberNameOffset,
                                    DWORD     dSort,
                                    BOOL      fReverse)
    {
    DWORD      dMemberData, dMemberOffset, dControl;
    PDBG_INDEX pdi = dbgIndexCreate (pdl, dMemberNext);

    if (pdi != NULL)
        {
        dMemberData   = MAXDWORD;
        dMemberOffset = MAXDWORD;
        dControl      = (fReverse ? DBG_SORT_REVERSE : 0);

        switch (dSort)
            {
            case DBG_SORT_BY_ADDRESS:
                {
                dMemberData    = dMemberAddress;
                dControl      |= DBG_SORT_PVOID;
                break;
                }
            case DBG_SORT_BY_SIZE:
                {
                dMemberData    = dMemberSize;
                dControl      |= DBG_SORT_DWORD;
                break;
                }
            case DBG_SORT_BY_ID:
                {
                dMemberData    = dMemberId;
                dControl      |= DBG_SORT_DWORD;
                break;
                }
            case DBG_SORT_BY_NAME_CS:
                {
                dControl      |= DBG_SORT_CASE;
                }
            case DBG_SORT_BY_NAME:
                {
                dMemberData    = dMemberNameData;
                dMemberOffset  = dMemberNameOffset;
                dControl      |= DBG_SORT_STRING;
                break;
                }
            default:
                {
                dControl      |= DBG_SORT_NONE;
                break;
                }
            }
        dbgIndexSort (pdi, dMemberData, dMemberOffset, dControl);
        }
    return pdi;
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgIndexDestroy (PDBG_INDEX pdi)
    {
    if (pdi != NULL) dbgListDestroy (pdi->pdlDestroy);
    return dbgMemoryDestroy (pdi);
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgIndexDestroyEx (PDBG_INDEX pdi)
    {
    DWORD i;

    if (pdi != NULL)
        {
        for (i = 0; i < pdi->dEntries; i++)
            {
            dbgIndexDestroy (pdi->ppdi [i]);
            }
        }
    return dbgIndexDestroy (pdi);
    }

// -----------------------------------------------------------------

void WINAPI dbgIndexReverse (PDBG_INDEX pdi)
    {
    DWORD i, j;
    PBYTE pbData;

    if ((pdi != NULL) && (pdi->dEntries > 1))
        {
        for (i = 0, j = pdi->dEntries-1; i < j; i++, j--)
            {
            pbData           = pdi->apbData [i];
            pdi->apbData [i] = pdi->apbData [j];
            pdi->apbData [j] = pbData;
            }
        }
    return;
    }

// -----------------------------------------------------------------

INT WINAPI dbgIndexCompare (PVOID pData1,
                            PVOID pData2,
                            DWORD dControl)
    {
    UNICODE_STRING us1, us2;
    BOOL           fOk = FALSE;
    INT            i   = -1;

    if ((pData1 != NULL) && (pData2 != NULL))
        {
        fOk = TRUE;

        switch (dControl & DBG_SORT_TYPE)
            {
            case DBG_SORT_RESTORE:
                {
                i = ((DWORD_PTR) pData1 < (DWORD_PTR) pData2
                     ? -1
                     : ((DWORD_PTR) pData1 > (DWORD_PTR) pData2
                        ? 1 : 0));
                break;
                }
            case DBG_SORT_DWORD:
                {
                i = (*(PDWORD) pData1 < *(PDWORD) pData2
                     ? -1
                     : (*(PDWORD) pData1 > *(PDWORD) pData2
                        ? 1 : 0));
                break;
                }
            case DBG_SORT_PVOID:
                {
                i = (*(PDWORD_PTR) pData1 < *(PDWORD_PTR) pData2
                     ? -1
                     : (*(PDWORD_PTR) pData1 > *(PDWORD_PTR) pData2
                        ? 1 : 0));
                break;
                }
            case DBG_SORT_STRING:
                {
                RtlInitUnicodeString (&us1, pData1);
                RtlInitUnicodeString (&us2, pData2);

                i = RtlCompareUnicodeString
                        (&us1, &us2,
                         (BOOLEAN) !(dControl & DBG_SORT_CASE));
                break;
                }
            default:
                {
                fOk = FALSE;
                break;
                }
            }
        }
    if (fOk && (dControl & DBG_SORT_REVERSE)) i = 0 - i;
    return i;
    }

// -----------------------------------------------------------------

void WINAPI dbgIndexSort (PDBG_INDEX pdi,
                          DWORD      dMemberData,
                          DWORD      dMemberOffset,
                          DWORD      dControl)
    {
    INT   i;
    DWORD dControl1, dCount, dBase, dDelta, dTrans, n1, n2;
    PBYTE pbData1, pbData2;

    dControl1 = (dControl & ~DBG_SORT_TYPE) | DBG_SORT_RESTORE;

    if ((dControl & DBG_SORT_TYPE) == DBG_SORT_NONE)
        {
        if (dControl & DBG_SORT_REVERSE) dbgIndexReverse (pdi);
        }
    else
        {
        if ((pdi != NULL) && (pdi->dEntries > 1) &&
            (dMemberData != MAXDWORD))
            {
            dTrans = 1;
            dDelta = dCount = pdi->dEntries;

            while ((dDelta > 1) || dTrans)
                {
                dTrans  = 0;
                dDelta -= (dDelta > 3 ? dDelta >> 2
                                      : (dDelta > 1 ? 1 : 0));

                for (dBase = 0; dBase < dCount - dDelta; dBase++)
                    {
                    pbData1 = pdi->apbData [dBase];
                    pbData2 = pdi->apbData [dBase + dDelta];

                    n1 = n2 = dMemberData;
                    
                    if (dMemberOffset != MAXDWORD)
                        {
                        n1 += *(PDWORD) (pbData1 + dMemberOffset) *
                              sizeof (WORD);

                        n2 += *(PDWORD) (pbData2 + dMemberOffset) *
                              sizeof (WORD);
                        }
                    if (!(i = dbgIndexCompare (pbData1 + n1,
                                               pbData2 + n2,
                                               dControl)))
                        {
                        i = dbgIndexCompare (pbData1,
                                             pbData2,
                                             dControl1);
                        }
                    if (i > 0)
                        {
                        pdi->apbData [dBase]          = pbData2;
                        pdi->apbData [dBase + dDelta] = pbData1;

                        dTrans++;
                        }
                    }
                }
            }
        }
    return;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgIndexList (PDBG_INDEX pdi)
    {
    DWORD     dNext, i;
    PDBG_LIST pdl = NULL;

    if ((pdi != NULL)
        &&
        ((pdl = dbgListCreateEx (pdi->dData,    pdi->dEntries,
                                 pdi->dContext, pdi->pContext,
                                &pdi->pdl->st))
         != NULL))
        {
        for (i = 0; i < pdi->dEntries; i++)
            {
            dNext = *(PDWORD) (pdi->apbData [i] +
                               pdi->dMemberNext);

            CopyMemory (pdl->abData + pdl->dOffset,
                        pdi->apbData [i], dNext);

            pdl->dOffset += dNext;
            }
        }
    return dbgListFinish (pdl);
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgIndexListEx (PDBG_INDEX pdi)
    {
    DWORD      dNext, i, j;
    PDBG_INDEX pdi1;
    PDBG_LIST  pdl = NULL;

    if ((pdi != NULL)
        &&
        ((pdl = dbgListCreateEx (pdi->dData,    pdi->dEntries,
                                 pdi->dContext, pdi->pContext,
                                &pdi->pdl->st))
         != NULL))
        {
        for (i = 0; i < pdi->dEntries; i++)
            {
            pdi1 = pdi->ppdi [i];

            CopyMemory (pdl->abData + pdl->dOffset,
                        pdi1->pContext, pdi1->dContext);

            pdl->dOffset += pdi1->dContext;

            for (j = 0; j < pdi1->dEntries; j++)
                {
                dNext = *(PDWORD) (pdi1->apbData [j] +
                                   pdi1->dMemberNext);

                CopyMemory (pdl->abData + pdl->dOffset,
                            pdi1->apbData [j], dNext);

                pdl->dOffset += dNext;
                }
            }
        }
    return dbgListFinish (pdl);
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgIndexSave (PWORD      pwPath,
                          PDBG_INDEX pdi)
    {
    PDBG_LIST pdl;
    BOOL      fOk = FALSE;

    if ((pdl = dbgIndexList (pdi)) != NULL)
        {
        fOk = dbgListSave (pwPath, pdl);
        dbgListDestroy (pdl);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI dbgIndexSaveEx (PWORD      pwPath,
                            PDBG_INDEX pdi)
    {
    PDBG_LIST pdl;
    BOOL      fOk = FALSE;

    if ((pdl = dbgIndexListEx (pdi)) != NULL)
        {
        fOk = dbgListSave (pwPath, pdl);
        dbgListDestroy (pdl);
        }
    return fOk;
    }

// =================================================================
// PROCESS MANAGEMENT
// =================================================================

PDWORD WINAPI dbgProcessIds (PDWORD pdCount)
    {
    DWORD  dSize;
    DWORD  dCount = 0;
    PDWORD pdList = NULL;

    dSize = SIZE_MINIMUM * sizeof (DWORD);

    while ((pdList = dbgMemoryCreate (dSize)) != NULL)
        {
        if (EnumProcesses (pdList, dSize, &dCount) &&
            (dCount < dSize))
            {
            dCount /= sizeof (DWORD);
            break;
            }
        dCount = 0;
        pdList = dbgMemoryDestroy (pdList);
        if ((dSize <<= 1) > (SIZE_MAXIMUM * sizeof (DWORD))) break;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pdList;
    }

// -----------------------------------------------------------------

PHMODULE WINAPI dbgProcessModules (HANDLE hProcess,
                                   PDWORD pdCount)
    {
    DWORD    dSize;
    DWORD    dCount = 0;
    PHMODULE phList = NULL;

    if (hProcess != NULL)
        {
        dSize = SIZE_MINIMUM * sizeof (HMODULE);

        while ((phList = dbgMemoryCreate (dSize)) != NULL)
            {
            if (EnumProcessModules (hProcess, phList, dSize,
                                    &dCount))
                {
                if (dCount <= dSize)
                    {
                    dCount /= sizeof (HMODULE);
                    break;
                    }
                }
            else
                {
                dCount = 0;
                }
            phList = dbgMemoryDestroy (phList);
            if (!(dSize = dCount)) break;
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    return phList;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgProcessAdd (PDBG_LIST pdl,
                                DWORD     dId)
    {
    PDBG_PROCESS pdp;
    PDBG_MODULE  pdm;
    MODULEINFO   mi;
    WORD         awPath [MAX_PATH];
    HANDLE       hProcess;
    PHMODULE     phModules;
    DWORD        dModules, dPath, dFile, dData, dOffset, i;
    PDBG_LIST    pdl1  = pdl;

    if (pdl1 != NULL)
        {
        dOffset = pdl1->dOffset;

        if ((hProcess = OpenProcess (PROCESS_ALL_ACCESS,
                                     FALSE, dId))
            != NULL)
            {
            if ((phModules = dbgProcessModules (hProcess,
                                                &dModules))
                != NULL)
                {
                for (i = 0; (pdl1 != NULL) && (i < dModules); i++)
                    {
                    if (!GetModuleInformation (hProcess,
                                               phModules [i],
                                               &mi, sizeof (mi)))
                        {
                        mi.SizeOfImage = 0;
                        mi.lpBaseOfDll = NULL;
                        mi.EntryPoint  = NULL;
                        }
                    dPath = GetModuleFileNameEx (hProcess,
                                                 phModules [i],
                                                 awPath, MAX_PATH);
                    awPath [dPath++] = 0;
                    dFile = dbgPathFile (awPath);

                    if (!i)
                        {
                        dData = dbgMemoryAlignEx (DBG_PROCESS_,
                                                  dPath);

                        if ((pdl1 = dbgListResize (pdl1, dData))
                            != NULL)
                            {
                            pdp = (PDBG_PROCESS) (pdl1->abData +
                                                  pdl1->dOffset);
                            pdp->dNext    = 0;
                            pdp->dSize    = mi.SizeOfImage;
                            pdp->pBase    = mi.lpBaseOfDll;
                            pdp->pStart   = mi.EntryPoint;
                            pdp->dDown    = dData;
                            pdp->dId      = dId;
                            pdp->dModules = dModules;
                            pdp->dFile    = dFile;

                            lstrcpy (pdp->awPath, awPath);

                            dbgListNext (pdl1, dData, TRUE);
                            }
                        }
                    dData = dbgMemoryAlignEx (DBG_MODULE_, dPath);

                    if ((pdl1 = dbgListResize (pdl1, dData))
                        != NULL)
                        {
                        pdm = (PDBG_MODULE) (pdl1->abData +
                                             pdl1->dOffset);
                        pdm->dNext  = dData;
                        pdm->dSize  = mi.SizeOfImage;
                        pdm->pBase  = mi.lpBaseOfDll;
                        pdm->pStart = mi.EntryPoint;
                        pdm->dFile  = dFile;

                        lstrcpy (pdm->awPath, awPath);

                        dbgListNext (pdl1, dData, FALSE);
                        }
                    }
                dbgMemoryDestroy (phModules);
                }
            CloseHandle (hProcess);
            }
        if (pdl1 != NULL)
            {
            if (dOffset != pdl1->dOffset)
                {
                pdp = (PDBG_PROCESS) (pdl1->abData + dOffset);

                pdp->dNext = pdl1->dOffset - dOffset;
                }
            else
                {
                dData = dbgMemoryAlignEx (DBG_PROCESS_, 1);

                if ((pdl1 = dbgListResize (pdl1, dData)) != NULL)
                    {
                    pdp = (PDBG_PROCESS) (pdl1->abData + dOffset);

                    pdp->dNext      = dData;
                    pdp->dSize      = 0;
                    pdp->pBase      = NULL;
                    pdp->pStart     = NULL;
                    pdp->dDown      = dData;
                    pdp->dId        = dId;
                    pdp->dModules   = 0;
                    pdp->dFile      = 0;
                    pdp->awPath [0] = 0;

                    dbgListNext (pdl1, dData, TRUE);
                    }
                }
            }
        }
    return pdl1;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgProcessList (void)
    {
    PDWORD    pdProcesses;
    DWORD     dProcesses, i;
    PDBG_LIST pdl = NULL;

    if ((pdProcesses = dbgProcessIds (&dProcesses)) != NULL)
        {
        pdl = dbgListCreate ();

        for (i = 0; (pdl != NULL) && (i < dProcesses); i++)
            {
            pdl = dbgProcessAdd (pdl, pdProcesses [i]);
            }
        dbgMemoryDestroy (pdProcesses);
        }
    return dbgListFinish (pdl);
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgProcessIndex (PWORD pwImage,
                                   DWORD dSort,
                                   BOOL  fReverse)
    {
    return dbgIndexCreateEx ((pwImage != NULL
                              ? dbgListLoad (pwImage)
                              : dbgProcessList ()),
                             OFFSET (DBG_PROCESS, dNext ),
                             OFFSET (DBG_PROCESS, pBase ),
                             OFFSET (DBG_PROCESS, dSize ),
                             OFFSET (DBG_PROCESS, dId   ),
                             OFFSET (DBG_PROCESS, awPath),
                             OFFSET (DBG_PROCESS, dFile ),
                             dSort, fReverse);
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgProcessIndexEx (PWORD pwImage,
                                     DWORD dSort,
                                     BOOL  fReverse)
    {
    DWORD      i, n;
    PDBG_INDEX pdi = dbgProcessIndex (pwImage, dSort, fReverse);

    if (pdi != NULL)
        {
        for (i = n = 0; i < pdi->dEntries; i++)
            {
            if ((pdi->ppdi [i] = dbgModuleIndex (pdi->ppdp [i],
                                                 dSort, fReverse))
                != NULL) n++;
            }
        if (n < pdi->dEntries) pdi = dbgIndexDestroyEx (pdi);
        }
    return pdi;
    }

// -----------------------------------------------------------------

PWORD WINAPI dbgProcessGuess (DWORD dIndex)
    {
    PWORD pwName = L"<unknown>";

    switch (dIndex)
        {
        case 0: pwName = L"<Idle>";   break;
        case 1: pwName = L"<System>"; break;
        }
    return pwName;
    }

// =================================================================
// MODULE MANAGEMENT
// =================================================================

PDBG_LIST WINAPI dbgModuleList (PDBG_PROCESS pdp)
    {
    DWORD     dData;
    PDBG_LIST pdl = NULL;

    if (pdp != NULL)
        {
        dData = pdp->dNext - pdp->dDown;

        if ((pdl = dbgListCreateEx (dData, pdp->dModules,
                                    pdp->dDown, pdp, NULL))
            != NULL)
            {
            CopyMemory (pdl->abData + pdl->dOffset,
                        (PBYTE) pdp + pdp->dDown, dData);

            pdl->dOffset += dData;
            }
        }
    return dbgListFinish (pdl);
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgModuleIndex (PDBG_PROCESS pdp,
                                  DWORD        dSort,
                                  BOOL         fReverse)
    {
    return dbgIndexCreateEx (dbgModuleList (pdp),
                             OFFSET (DBG_MODULE, dNext ),
                             OFFSET (DBG_MODULE, pBase ),
                             OFFSET (DBG_MODULE, dSize ),
                             MAXDWORD,
                             OFFSET (DBG_MODULE, awPath),
                             OFFSET (DBG_MODULE, dFile ),
                             dSort, fReverse);
    }

// =================================================================
// DRIVER MANAGEMENT
// =================================================================

PPVOID WINAPI dbgDriverAddresses (PDWORD pdCount)
    {
    DWORD  dSize;
    DWORD  dCount = 0;
    PPVOID ppList = NULL;

    dSize = SIZE_MINIMUM * sizeof (PVOID);

    while ((ppList = dbgMemoryCreate (dSize)) != NULL)
        {
        if (EnumDeviceDrivers (ppList, dSize, &dCount) &&
            (dCount < dSize))
            {
            dCount /= sizeof (PVOID);
            break;
            }
        dCount = 0;
        ppList = dbgMemoryDestroy (ppList);
        if ((dSize <<= 1) > (SIZE_MAXIMUM * sizeof (PVOID))) break;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return ppList;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgDriverAdd (PDBG_LIST pdl,
                               PVOID     pBase)
    {
    PDBG_DRIVER pdd;
    WORD        awPath [MAX_PATH];
    DWORD       dPath, dFile, dData;
    PDBG_LIST   pdl1 = pdl;

    if (pBase != NULL)
        {
        dPath = GetDeviceDriverFileName (pBase, awPath, MAX_PATH);
        }
    else
        {
        dData = 0;
        }
    awPath [dPath++] = 0;
    dFile = dbgPathFile (awPath);
    dData = dbgMemoryAlignEx (DBG_DRIVER_, dPath);

    if ((pdl1 = dbgListResize (pdl, dData)) != NULL)
        {
        pdd = (PDBG_DRIVER) (pdl1->abData + pdl1->dOffset);

        pdd->dNext = dData;
        pdd->pBase = pBase;
        pdd->dFile = dFile;

        lstrcpy (pdd->awPath, awPath);

        if (dbgBaseModule (awPath + dFile, &pdd->dSize) == NULL)
            {
            dPath = dbgPathDriver (pdd->awPath + pdd->dFile,
                                   awPath, MAX_PATH);

            if (dPath && (dPath < MAX_PATH))
                {
                dbgBaseModule (awPath, &pdd->dSize);
                }
            }
        dbgListNext (pdl1, dData, TRUE);
        }
    return pdl1;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgDriverList (void)
    {
    PPVOID    ppDrivers;
    DWORD     dDrivers, i;
    PDBG_LIST pdl = NULL;

    if ((ppDrivers = dbgDriverAddresses (&dDrivers)) != NULL)
        {
        pdl = dbgListCreate ();

        for (i = 0; (pdl != NULL) && (i < dDrivers); i++)
            {
            pdl = dbgDriverAdd (pdl, ppDrivers [i]);
            }
        dbgMemoryDestroy (ppDrivers);
        }
    return dbgListFinish (pdl);
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgDriverIndex (PWORD pwImage,
                                  DWORD dSort,
                                  BOOL  fReverse)
    {
    return dbgIndexCreateEx ((pwImage != NULL
                              ? dbgListLoad (pwImage)
                              : dbgDriverList ()),
                             OFFSET (DBG_DRIVER, dNext ),
                             OFFSET (DBG_DRIVER, pBase ),
                             OFFSET (DBG_DRIVER, dSize ),
                             MAXDWORD,
                             OFFSET (DBG_DRIVER, awPath),
                             OFFSET (DBG_DRIVER, dFile ),
                             dSort, fReverse);
    }

// =================================================================
// SYMBOL MANAGEMENT
// =================================================================

BOOL CALLBACK dbgSymbolCallback (PSTR       psSymbolName,
                                 DWORD      dSymbolAddress,
                                 DWORD      dSymbolSize,
                                 PPDBG_LIST ppdl)
    {
    PDBG_LIST   pdl;
    PDBG_SYMBOL pds;
    DWORD       dName, dData, i;
    BOOL        fOk = FALSE;

    if (ppdl != NULL)
        {
        pdl   = *ppdl;
        dName = lstrlenA (psSymbolName) + 1;
        dData = dbgMemoryAlignEx (DBG_SYMBOL_, dName);

        if ((pdl = dbgListResize (pdl, dData)) != NULL)
            {
            pds = (PDBG_SYMBOL) (pdl->abData + pdl->dOffset);

            pds->dNext =         dData;
            pds->dSize =         dSymbolSize;
            pds->pBase = (PVOID) dSymbolAddress;

            for (i = 0; i < dName; i++)
                {
                pds->awName [i] = psSymbolName [i];
                }
            dbgListNext (pdl, dData, TRUE);

            fOk = TRUE;
            }
        *ppdl = pdl;
        }
    return fOk;
    }

// -----------------------------------------------------------------

PLOADED_IMAGE WINAPI dbgSymbolLoad (PWORD  pwPath,
                                    PVOID  pBase,
                                    HANDLE hProcess)
    {
    WORD          awPath [MAX_PATH];
    PBYTE         pbPath;
    DWORD         dPath;
    PLOADED_IMAGE pli = NULL;

    if ((pbPath = dbgStringAnsi (pwPath, NULL)) != NULL)
        {
        if (((pli = ImageLoad (pbPath, NULL)) == NULL)         &&
            (dPath = dbgPathDriver (pwPath, awPath, MAX_PATH)) &&
            (dPath < MAX_PATH))
            {
            dbgMemoryDestroy (pbPath);

            if ((pbPath = dbgStringAnsi (awPath, NULL)) != NULL)
                {
                pli = ImageLoad (pbPath, NULL);
                }
            }
        if ((pli != NULL)
            &&
            (!SymLoadModule (hProcess, pli->hFile, pbPath, NULL,
                             (DWORD_PTR) pBase, pli->SizeOfImage)))
            {
            ImageUnload (pli);
            pli = NULL;
            }
        dbgMemoryDestroy (pbPath);
        }
    return pli;
    }

// -----------------------------------------------------------------

PLOADED_IMAGE WINAPI dbgSymbolUnload (PLOADED_IMAGE pli,
                                      PVOID         pBase,
                                      HANDLE        hProcess)
    {
    if (pli != NULL)
        {
        SymUnloadModule (hProcess, (DWORD_PTR) pBase);
        ImageUnload     (pli);
        }
    return NULL;
    }

// -----------------------------------------------------------------

PDBG_LIST WINAPI dbgSymbolList (PWORD pwPath,
                                PVOID pBase)
    {
    PLOADED_IMAGE pli;
    HANDLE        hProcess = GetCurrentProcess ();
    PDBG_LIST     pdl      = NULL;

    if ((pwPath != NULL) &&
        SymInitialize (hProcess, NULL, FALSE))
        {
        if ((pli = dbgSymbolLoad (pwPath, pBase, hProcess)) != NULL)
            {
            if ((pdl = dbgListCreate ()) != NULL)
                {
                SymEnumerateSymbols (hProcess, (DWORD_PTR) pBase,
                                     dbgSymbolCallback, &pdl);
                }
            dbgSymbolUnload (pli, pBase, hProcess);
            }
        SymCleanup (hProcess);
        }
    return dbgListFinish (pdl);
    }

// -----------------------------------------------------------------

PDBG_INDEX WINAPI dbgSymbolIndex (PWORD pwPath,
                                  PVOID pBase,
                                  PWORD pwImage,
                                  DWORD dSort,
                                  BOOL  fReverse)
    {
    return dbgIndexCreateEx ((pwImage != NULL
                              ? dbgListLoad (pwImage)
                              : dbgSymbolList (pwPath, pBase)),
                             OFFSET (DBG_SYMBOL, dNext ),
                             OFFSET (DBG_SYMBOL, pBase ),
                             OFFSET (DBG_SYMBOL, dSize ),
                             MAXDWORD,
                             OFFSET (DBG_SYMBOL, awName),
                             MAXDWORD,
                             dSort, fReverse);
    }

// -----------------------------------------------------------------

PDBG_SYMBOL WINAPI dbgSymbolLookup (PDBG_INDEX pdi,
                                    PVOID      pAddress,
                                    PDWORD     pdOffset)
    {
    DWORD       dAddress, dBase, i, j, k;
    DWORD       dOffset = 0;
    PDBG_SYMBOL pds     = NULL;

    if (pdi != NULL)
        {
        dAddress = (DWORD_PTR) pAddress;

        for (i = 0, j = k = MAXDWORD; i < pdi->dEntries; i++)
            {
            dBase = (DWORD_PTR) pdi->ppds [i]->pBase;

            if (dBase && (dBase + pdi->ppds [i]->dSize) &&
                (dAddress >= dBase) && (dAddress - dBase < k))
                {
                j = i;
                k = dAddress - dBase;
                }
            }
        if (j != MAXDWORD)
            {
            dOffset = k;
            pds     = pdi->ppds [j];
            }
        }
    if (pdOffset != NULL) *pdOffset = dOffset;
    return pds;
    }

// =================================================================
// BASE ADDRESS INQUIRY
// =================================================================

PVOID WINAPI dbgBaseModule (PWORD  pwPath,
                            PDWORD pdSize)
    {
    MODULEINFO mi;
    HMODULE    hModule;
    DWORD      dSize = 0;
    PVOID      pBase = NULL;

    if ((pwPath != NULL)
        &&
        ((hModule = LoadLibraryEx (pwPath, NULL,
                                   DONT_RESOLVE_DLL_REFERENCES))
         != NULL))
        {
        if (GetModuleInformation (GetCurrentProcess (), hModule,
                                  &mi, sizeof (mi)))
            {
            pBase = mi.lpBaseOfDll;
            dSize = mi.SizeOfImage;
            }
        FreeLibrary (hModule);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pBase;
    }

// -----------------------------------------------------------------

PVOID WINAPI dbgBaseDriver (PWORD  pwPath,
                            PDWORD pdSize)
    {
    PDBG_LIST   pdl;
    PDBG_DRIVER pdd;
    PWORD       pwName;
    DWORD       i;
    DWORD       dSize = 0;
    PVOID       pBase = NULL;

    if (pwPath != NULL)
        {
        pwName = pwPath + dbgPathFile (pwPath);

        if ((pdl = dbgDriverList ()) != NULL)
            {
            pdd = (PDBG_DRIVER) (pdl->abData + pdl->dFirst);

            for (i = 0; i < pdl->dEntries; i++)
                {
                if (!lstrcmpi (pwName, pdd->awPath + pdd->dFile))
                    {
                    pBase = pdd->pBase;
                    dSize = pdd->dSize;
                    break;
                    }
                pdd = (PDBG_DRIVER) ((PBYTE) pdd + pdd->dNext);
                }
            dbgListDestroy (pdl);
            }
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pBase;
    }

// =================================================================
// DLL MANAGEMENT
// =================================================================

BOOL WINAPI DllMain (HINSTANCE hInstance,
                     DWORD     dReason,
                     PVOID     pReserved)
    {
    BOOL fOk = TRUE;

    switch (dReason)
        {
        case DLL_PROCESS_ATTACH:
            {
            InitializeCriticalSection (&gcsMemory);
            break;
            }
        case DLL_PROCESS_DETACH:
            {
            DeleteCriticalSection (&gcsMemory);
            break;
            }
        }
    return fOk;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
