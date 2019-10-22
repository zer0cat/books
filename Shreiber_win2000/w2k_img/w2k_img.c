
// __________________________________________________________
//
//                         w2k_img.c
//            SBS Windows 2000 Image Library V1.01
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#define  _W2K_IMG_DLL_
#include "w2k_img.h"

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

08-09-2000 V1.00 Original version (SBS).

08-27-2000 V1.01 Upgrade (SBS).

    Added imgAnsiMatch(), imgTimeDay(), imgSymbolTable(),
    imgSymbolCompare(), imgSymbolSort(), and the complete
    imgTable*() function set. Revised the symbol file finder
    algorithm. Optimized OMAP address lookup. Added detection
    of calling conventions.

*/

// =================================================================
// GLOBAL VARIABLES
// =================================================================

HINSTANCE ghInstance = NULL;

// =================================================================
// GLOBAL STRINGS
// =================================================================

BYTE gabSymbolPath1 [] =  "_NT_SYMBOL_PATH";
BYTE gabSymbolPath2 [] =  "_NT_ALT_SYMBOL_PATH";
BYTE gabSymbols     [] =  "Symbols\\";
BYTE gabKernel      [] =  "ntoskrnl.exe";
BYTE gabDbg         [] =  ".dbg";
BYTE gabPdb         [] =  ".pdb";
BYTE gabNull        [] =  "";

WORD gawSymbolPath1 [] = L"_NT_SYMBOL_PATH";
WORD gawSymbolPath2 [] = L"_NT_ALT_SYMBOL_PATH";
WORD gawSymbols     [] = L"Symbols\\";
WORD gawKernel      [] = L"ntoskrnl.exe";
WORD gawDbg         [] = L".dbg";
WORD gawPdb         [] = L".pdb";
WORD gawNull        [] = L"";

// =================================================================
// LOWER-CASE CHARACTER TABLE
// =================================================================

BYTE LCase [] =
    {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
     64, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
    112,113,114,115,116,117,118,119,120,121,122, 91, 92, 93, 94, 95,
     96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,255,
    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,215,248,249,250,251,252,253,222,223,
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,222,255
    };

// =================================================================
// OUTPUT FUNCTIONS
// =================================================================

INT WINAPI imgBoxA (HWND  hWnd,
                    UINT  uiType,
                    PBYTE pbCaption,
                    PBYTE pbFormat,
                    ...)
    {
    BYTE abBuffer [1024];

    wvsprintf (abBuffer, pbFormat, (PVOID) (&pbFormat+1));
    return MessageBoxA (hWnd, abBuffer, pbCaption, uiType);
    }

// -----------------------------------------------------------------

INT WINAPI imgBoxW (HWND  hWnd,
                    UINT  uiType,
                    PWORD pwCaption,
                    PWORD pwFormat,
                    ...)
    {
    WORD awBuffer [1024];

    wvsprintfW (awBuffer, pwFormat, (PVOID) (&pwFormat+1));
    return MessageBoxW (hWnd, awBuffer, pwCaption, uiType);
    }

// =================================================================
// MEMORY MANAGEMENT
// =================================================================

PVOID WINAPI imgMemoryCreate (DWORD dBytes)
    {
    return LocalAlloc (LMEM_FIXED, max (dBytes, 1));
    }

// -----------------------------------------------------------------

PVOID WINAPI imgMemoryResize (PVOID pData,
                              DWORD dBytes)
    {
    DWORD dBytes1, dBytes2;
    PVOID pData1 = NULL;

    if (pData != NULL)
        {
        if (dBytes1 = LocalSize (pData))
            {
            if ((dBytes2 = max (dBytes, 1)) != dBytes1)
                {
                if ((pData1 = LocalReAlloc (pData, dBytes2,
                                            LMEM_MOVEABLE))
                    == NULL)
                    {
                    LocalFree (pData);
                    }
                }
            else
                {
                pData1 = pData;
                }
            }
        else
            {
            LocalFree (pData);
            }
        }
    return pData1;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgMemoryDestroy (PVOID pData)
    {
    if (pData != NULL)
        {
        LocalFree (pData);
        }
    return NULL;
    }

// =================================================================
// STRING PATTERN MATCHER
// =================================================================

BOOL WINAPI imgAnsiMatchA (PBYTE pbFilter,
                           PBYTE pbData,
                           BOOL  fIgnoreCase)
    {
    DWORD i = 0;
    DWORD j = 0;

    if (pbData   == NULL) return FALSE;
    if (pbFilter == NULL) return TRUE;

    while (pbFilter [i] && pbData [j])
        {
        if (pbFilter [i] != '?')
            {
            if (pbFilter [i] == '*')
                {
                i++;

                if ((pbFilter [i] != '*') &&
                    (pbFilter [i] != '?'))
                    {
                    if (pbFilter [i])
                        {
                        while (pbData [j]
                               &&
                               (!imgAnsiMatchA (pbFilter + i,
                                                pbData   + j,
                                                fIgnoreCase)))
                            {
                            j++;
                            }
                        }
                    return pbData [j] != 0;
                    }
                }
            if (fIgnoreCase
                ? LCASEA (pbFilter [i]) != LCASEA (pbData [j])
                :         pbFilter [i]  !=         pbData [j] )
                {
                return FALSE;
                }
            }
        i++;
        j++;
        }
    if (pbFilter [i] == '*') i++;
    return !(pbFilter [i] || pbData [j]);
    }

// -----------------------------------------------------------------

BOOL WINAPI imgAnsiMatchW (PWORD pwFilter,
                           PBYTE pbData,
                           BOOL  fIgnoreCase)
    {
    DWORD i = 0;
    DWORD j = 0;

    if (pbData   == NULL) return FALSE;
    if (pwFilter == NULL) return TRUE;

    while (pwFilter [i] && pbData [j])
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
                        while (pbData [j]
                               &&
                               (!imgAnsiMatchW (pwFilter + i,
                                                pbData   + j,
                                                fIgnoreCase)))
                            {
                            j++;
                            }
                        }
                    return pbData [j] != 0;
                    }
                }
            if (fIgnoreCase
                ? LCASEW (pwFilter [i]) != LCASEA (pbData [j])
                :         pwFilter [i]  !=         pbData [j] )
                {
                return FALSE;
                }
            }
        i++;
        j++;
        }
    if (pwFilter [i] == '*') i++;
    return !(pwFilter [i] || pbData [j]);
    }

// =================================================================
// DATE/TIME CONVERSION
// =================================================================

DWORD adDaysPerMonth [] = {31,28,31,30,31,30,31,31,30,31,30,31};

// -----------------------------------------------------------------

IMG_TIME WINAPI imgTimeNow (BOOL fLocal)
    {
    SYSTEMTIME st;
    IMG_TIME   it;

    if (fLocal) GetLocalTime  (&st);
    else        GetSystemTime (&st);

    it.wYear      = (WORD) st.wYear;
    it.bMonth     = (BYTE) st.wMonth;
    it.bDay       = (BYTE) st.wDay;
    it.bHour      = (BYTE) st.wHour;
    it.bMinute    = (BYTE) st.wMinute;
    it.bSecond    = (BYTE) st.wSecond;
    it.bDayOfWeek =
        (BYTE) (((((imgTimePack (it) / 60) / 60) / 24) + 4) % 7);

    return it;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgTimePack (IMG_TIME it)
    {
    DWORD dYear, dMonth, dDay, i;
    DWORD dTime = 0;

    if (it.wYear >= 1970)
        {
        dMonth = (it.bMonth ? it.bMonth-1 : 0);

        dYear  = (DWORD) it.wYear - 1600 + (dMonth / 12);
        dMonth = dMonth % 12;
        dDay   = (dYear / 400) * DAYS_PER_400_YEARS;

        for (i = 0; i < dMonth; i++)
            {
            dDay += adDaysPerMonth [i];
            }
        if ((dMonth > 1) && (!(dYear % 4)) &&
            ((dYear % 100) || (!(dYear % 400))))
            {
            dDay += 1;
            }
        if (it.bDay)
            {
            dDay += it.bDay - 1;
            }
        if (dYear = dYear % 400)
            {
            dDay += ((dYear / 100) * (DAYS_PER_100_YEARS-1)) + 1;

            if (dYear = dYear % 100)
                {
                dDay += ((dYear / 4) * DAYS_PER_4_YEARS) - 1;

                if (dYear = dYear % 4)
                    {
                    dDay += (dYear * DAYS_PER_YEAR) + 1;
                    }
                }
            }
        dDay -= DAYS_1600_TO_1970;

        dTime  = (dDay               * 60 * 60 * 24) +
                 ((DWORD) it.bHour   * 60 * 60     ) +
                 ((DWORD) it.bMinute * 60          ) +
                 ((DWORD) it.bSecond               );
        }
    return dTime;
    }

// -----------------------------------------------------------------

IMG_TIME WINAPI imgTimeUnpack (DWORD dTime)
    {
    IMG_TIME it;

    DWORD dDaySince1600, dDayIn100Years, dDayIn4Years;
    DWORD d400Years, d100In400Years, d4In100Years, d1In4Years;
    DWORD dDay, dMonth, dYear, dDaysPerMonth;
    BOOL  fLeap;

    it.bSecond    = (BYTE) ((dTime        ) % 60);
    it.bMinute    = (BYTE) ((dTime /    60) % 60);
    it.bHour      = (BYTE) ((dTime /  3600) % 24);
    dDaySince1600 =         (dTime / 86400) + DAYS_1600_TO_1970;

    dDayIn100Years = dDaySince1600 % DAYS_PER_400_YEARS;
    d400Years      = dDaySince1600 / DAYS_PER_400_YEARS;
    d100In400Years = 0;

    while (dDayIn100Years >= DAYS_PER_100_YEARS)
        {
        dDayIn100Years -= (DAYS_PER_100_YEARS-1);
        d100In400Years++;
        }
    dDayIn4Years = dDayIn100Years % DAYS_PER_4_YEARS;
    d4In100Years = dDayIn100Years / DAYS_PER_4_YEARS;

    if (dDayIn4Years)
        {
        dDay       = (dDayIn4Years-1) % DAYS_PER_YEAR;
        d1In4Years = (dDayIn4Years-1) / DAYS_PER_YEAR;
        if (!d1In4Years) dDay++;
        }
    else
        {
        dDay       = 0;
        d1In4Years = 0;
        }
    fLeap = (d1In4Years ? FALSE
                        : (d4In100Years ? TRUE
                                        : !d100In400Years));

    for (dMonth = 0; dMonth < 12; dMonth++)
        {
        dDaysPerMonth = adDaysPerMonth [dMonth];
        if ((dMonth == 2) && fLeap) dDaysPerMonth++;
        if (dDay < dDaysPerMonth) break;
        dDay -= dDaysPerMonth;
        }
    dYear = (d1In4Years          ) +
            (d4In100Years   *   4) +
            (d100In400Years * 100) +
            (d400Years      * 400) + 1600;

    it.bDay       = (BYTE) (dDay + 1);
    it.bMonth     = (BYTE) (dMonth + 1);
    it.wYear      = (WORD) dYear;
    it.bDayOfWeek = (BYTE) ((dDaySince1600 + 6) % 7);
    return it;
    }

// -----------------------------------------------------------------

PBYTE WINAPI imgTimeDayA (IMG_TIME it)
    {
    PBYTE pbDay = NULL;

    switch (it.bDayOfWeek % 7)
        {
        case 0: pbDay = "Sunday";    break;
        case 1: pbDay = "Monday";    break;
        case 2: pbDay = "Tuesday";   break;
        case 3: pbDay = "Wednesday"; break;
        case 4: pbDay = "Thursday";  break;
        case 5: pbDay = "Friday";    break;
        case 6: pbDay = "Saturday";  break;
        }
    return pbDay;
    }

// -----------------------------------------------------------------

PWORD WINAPI imgTimeDayW (IMG_TIME it)
    {
    PWORD pwDay = NULL;

    switch (it.bDayOfWeek % 7)
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
// FILE PATH MANAGEMENT
// =================================================================

DWORD WINAPI imgPathRootA (PBYTE pbPath)
    {
    DWORD dRoot = 0;

    if ((pbPath != NULL) && pbPath [0])
        {
        if (pbPath [0] == '\\')
            {
            if (pbPath [1] == '\\')
                {
                for (dRoot = 2;
                     pbPath [dRoot] && (pbPath [dRoot] != '\\');
                     dRoot++);
                }
            }
        else
            {
            if (pbPath [1] == ':') dRoot = 2;
            }
        }
    return dRoot;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathRootW (PWORD pwPath)
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

DWORD WINAPI imgPathNameA (PBYTE  pbPath,
                           PDWORD pdExtension)
    {
    DWORD dRoot, dEnd;
    DWORD dExtension = 0;
    DWORD dName      = 0;

    if (pbPath != NULL)
        {
        dRoot = imgPathRootA (pbPath);
        dEnd  = dRoot + lstrlenA (pbPath + dRoot);

        for (dName = dEnd;
             (dName > dRoot) && (pbPath [dName-1] != '\\')
                             && (pbPath [dName-1] != ':' );
             dName--);

        for (dExtension = dEnd;
             (dExtension > dName) && (pbPath [dExtension-1] != '.');
             dExtension--);

        dExtension = (dExtension == dName ? dEnd : dExtension - 1);
        }
    if (pdExtension != NULL) *pdExtension = dExtension;
    return dName;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathNameW (PWORD  pwPath,
                           PDWORD pdExtension)
    {
    DWORD dRoot, dEnd;
    DWORD dExtension = 0;
    DWORD dName      = 0;

    if (pwPath != NULL)
        {
        dRoot = imgPathRootW (pwPath);
        dEnd  = dRoot + lstrlenW (pwPath + dRoot);

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

// -----------------------------------------------------------------

DWORD WINAPI imgPathCanonicalA (PBYTE pbPath,   // NULL: current dir
                                PBYTE pbBuffer, // can be == pbPath
                                DWORD dBuffer)
    {
    BYTE  abBuffer [MAX_PATH];
    PBYTE pb;
    DWORD n = 0;

    if ((pbBuffer != NULL) && dBuffer)
        {
        n = (pbPath == NULL
             ? GetCurrentDirectoryA (MAX_PATH, abBuffer)
             : (pbPath [0]
                ? GetFullPathNameA (pbPath, MAX_PATH, abBuffer, &pb)
                : 0));

        if (n && (n < MAX_PATH))
            {
            if (abBuffer [n-1] != '\\')
                {
                if (n+1 < MAX_PATH)
                    {
                    abBuffer [n++] = '\\';
                    }
                else
                    {
                    n = 0;
                    }
                }
            }
        else
            {
            n = 0;
            }
        if (n >= dBuffer) n = 0;
        abBuffer [n] = 0;
        lstrcpyA (pbBuffer, abBuffer);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathCanonicalW (PWORD pwPath,   // NULL: current dir
                                PWORD pwBuffer, // can be == pwPath
                                DWORD dBuffer)
    {
    WORD  awBuffer [MAX_PATH];
    PWORD pw;
    DWORD n = 0;

    if ((pwBuffer != NULL) && dBuffer)
        {
        n = (pwPath == NULL
             ? GetCurrentDirectoryW (MAX_PATH, awBuffer)
             : (pwPath [0]
                ? GetFullPathNameW (pwPath, MAX_PATH, awBuffer, &pw)
                : 0));

        if (n && (n < MAX_PATH))
            {
            if (awBuffer [n-1] != '\\')
                {
                if (n+1 < MAX_PATH)
                    {
                    awBuffer [n++] = '\\';
                    }
                else
                    {
                    n = 0;
                    }
                }
            }
        else
            {
            n = 0;
            }
        if (n >= dBuffer) n = 0;
        awBuffer [n] = 0;
        lstrcpyW (pwBuffer, awBuffer);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathCurrentA (PBYTE pbBuffer,
                              DWORD dBuffer)
    {
    return imgPathCanonicalA (NULL, pbBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathCurrentW (PWORD pwBuffer,
                              DWORD dBuffer)
    {
    return imgPathCanonicalW (NULL, pwBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathWindowsA (PBYTE pbBuffer,
                              DWORD dBuffer)
    {
    BYTE  abBuffer [MAX_PATH];
    DWORD n;

    n = GetWindowsDirectoryA (abBuffer, MAX_PATH);
    if (n >= MAX_PATH) n = 0;
    abBuffer [n] = 0;
    return imgPathCanonicalA (abBuffer, pbBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathWindowsW (PWORD pwBuffer,
                              DWORD dBuffer)
    {
    WORD  awBuffer [MAX_PATH];
    DWORD n;

    n = GetWindowsDirectoryW (awBuffer, MAX_PATH);
    if (n >= MAX_PATH) n = 0;
    awBuffer [n] = 0;
    return imgPathCanonicalW (awBuffer, pwBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathVariableA (PBYTE pbVariable,
                               PBYTE pbBuffer,
                               DWORD dBuffer,
                               DWORD dIndex)
    {
    BYTE  abBuffer [MAX_PATH_EX];
    DWORD i, j, k;
    DWORD n = 0;

    n = GetEnvironmentVariableA (pbVariable, abBuffer, MAX_PATH_EX);
    if (n >= MAX_PATH_EX) n = 0;

    for (i = j = 0; (i < dIndex) && (j < n); i++)
        {
        while ((j < n) && (abBuffer [j++] != ';'));
        }
    while ((j < n) && (abBuffer [j] == ' ')) j++;
    for (k = j; (k < n) && (abBuffer [k] != ';'); k++);
    while ((k > j) && (abBuffer [k-1] == ' ')) k--;

    if (j < n)
        {
        i = 0;

        if (k - j < MAX_PATH)
            {
            if (j)
                {
                while (j < k) abBuffer [i++] = abBuffer [j++];
                }
            else
                {
                i = k - j;
                }
            }
        abBuffer [i] = 0;

        n = imgPathCanonicalA (abBuffer, pbBuffer, dBuffer);
        }
    else
        {
        if ((pbBuffer != NULL) && dBuffer) pbBuffer [0] = 0;
        n = -1;
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathVariableW (PWORD pwVariable,
                               PWORD pwBuffer,
                               DWORD dBuffer,
                               DWORD dIndex)
    {
    WORD  awBuffer [MAX_PATH_EX];
    DWORD i, j, k;
    DWORD n = 0;

    n = GetEnvironmentVariableW (pwVariable, awBuffer, MAX_PATH_EX);
    if (n >= MAX_PATH_EX) n = 0;

    for (i = j = 0; (i < dIndex) && (j < n); i++)
        {
        while ((j < n) && (awBuffer [j++] != ';'));
        }
    while ((j < n) && (awBuffer [j] == ' ')) j++;
    for (k = j; (k < n) && (awBuffer [k] != ';'); k++);
    while ((k > j) && (awBuffer [k-1] == ' ')) k--;

    if (j < n)
        {
        i = 0;

        if (k - j < MAX_PATH)
            {
            if (j)
                {
                while (j < k) awBuffer [i++] = awBuffer [j++];
                }
            else
                {
                i = k - j;
                }
            }
        awBuffer [i] = 0;

        n = imgPathCanonicalW (awBuffer, pwBuffer, dBuffer);
        }
    else
        {
        if ((pwBuffer != NULL) && dBuffer) pwBuffer [0] = 0;
        n = -1;
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathEnumerateA (IMG_CALLBACKA CallbackA,
                                PBYTE         pbModule,
                                PVOID         pContext)
    {
    BYTE  abPath [MAX_PATH];
    DWORD i, j;
    DWORD n = 0;

    if (CallbackA != NULL)
        {
        if ((pbModule != NULL) &&
            ((i = imgPathNameA (pbModule, NULL)) < MAX_PATH))
            {
            lstrcpynA (abPath, pbModule, i+1);

            if (j = imgPathCanonicalA (abPath, abPath, MAX_PATH))
                {
                if (n = CallbackA (pbModule, abPath, j, pContext))
                    return n;
                }
            }
        if (j = imgPathCurrentA (abPath, MAX_PATH))
            {
            if (n = CallbackA (pbModule, abPath, j, pContext))
                return n;
            }
        i = 0;
        while ((j = imgPathVariableA (gabSymbolPath1,
                                      abPath, MAX_PATH, i++))
               != -1)
            {
            if (j)
                {
                if (n = CallbackA (pbModule, abPath, j, pContext))
                    return n;
                }
            }
        i = 0;
        while ((j = imgPathVariableA (gabSymbolPath2,
                                      abPath, MAX_PATH, i++))
               != -1)
            {
            if (j)
                {
                if (n = CallbackA (pbModule, abPath, j, pContext))
                    return n;
                }
            }
        if (j = imgPathWindowsA (abPath, MAX_PATH))
            {
            if (n = CallbackA (pbModule, abPath, j, pContext))
                return n;
            }
        n = CallbackA (pbModule, NULL, 0, pContext);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathEnumerateW (IMG_CALLBACKW CallbackW,
                                PWORD         pwModule,
                                PVOID         pContext)
    {
    WORD  awPath [MAX_PATH];
    DWORD i, j;
    DWORD n = 0;

    if (CallbackW != NULL)
        {
        if ((pwModule != NULL) &&
            ((i = imgPathNameW (pwModule, NULL)) < MAX_PATH))
            {
            lstrcpynW (awPath, pwModule, i+1);

            if (j = imgPathCanonicalW (awPath, awPath, MAX_PATH))
                {
                if (n = CallbackW (pwModule, awPath, j, pContext))
                    return n;
                }
            }
        if (j = imgPathCurrentW (awPath, MAX_PATH))
            {
            if (n = CallbackW (pwModule, awPath, j, pContext))
                return n;
            }
        i = 0;
        while ((j = imgPathVariableW (gawSymbolPath1,
                                      awPath, MAX_PATH, i++))
               != -1)
            {
            if (j)
                {
                if (n = CallbackW (pwModule, awPath, j, pContext))
                    return n;
                }
            }
        i = 0;
        while ((j = imgPathVariableW (gawSymbolPath2,
                                      awPath, MAX_PATH, i++))
               != -1)
            {
            if (j)
                {
                if (n = CallbackW (pwModule, awPath, j, pContext))
                    return n;
                }
            }
        if (j = imgPathWindowsW (awPath, MAX_PATH))
            {
            if (n = CallbackW (pwModule, awPath, j, pContext))
                return n;
            }
        n = CallbackW (pwModule, NULL, 0, pContext);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathSymbolsA (PBYTE pbModule,   // NULL: ntoskrnl
                              PBYTE pbExtension,// NULL: .dbg
                              PBYTE pbPath,     // NULL: current dir
                              PBYTE pbBuffer,   // can be == pbPath
                              DWORD dBuffer,
                              BOOL  fSymbols)
    {
    BYTE  abBuffer [MAX_PATH];
    PBYTE pbModule1, pbExtension1, pbSymbols;
    DWORD dNext, dName, dExtension, n1, n2, n3, n4;
    DWORD n = 0;

    pbModule1    = (pbModule    != NULL ? pbModule    : gabKernel);
    pbExtension1 = (pbExtension != NULL ? pbExtension : gabDbg);
    pbSymbols    = (fSymbols            ? gabSymbols  : gabNull);

    dNext = imgPathCanonicalA (pbPath, abBuffer, MAX_PATH);
    dName = imgPathNameA (pbModule1, &dExtension);

    if (dNext && (pbModule1 [dExtension  ] == '.')
              &&  pbModule1 [dExtension+1]
        &&
        (dNext + (n1 = lstrlenA (pbSymbols))
               + (n2 = lstrlenA (pbModule1 + (dExtension+1))) + 1
               + (n3 = dExtension - dName)
               + (n4 = lstrlenA (pbExtension1))
         < MAX_PATH))
        {
        n = dNext;

        lstrcpyA (abBuffer + n, pbSymbols);
        n += n1;

        lstrcpyA (abBuffer + n, pbModule1 + (dExtension+1));
        n += n2;
        abBuffer [n++] = '\\';

        lstrcpynA (abBuffer + n, pbModule1 + dName, n3 + 1);
        n += n3;

        lstrcpyA (abBuffer + n, pbExtension1);
        n += n4;
        }
    if ((pbBuffer != NULL) && dBuffer)
        {
        if (n >= dBuffer) n = 0;
        abBuffer [n] = 0;
        lstrcpyA (pbBuffer, abBuffer);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathSymbolsW (PWORD pwModule,   // NULL: ntoskrnl
                              PWORD pwExtension,// NULL: .dbg
                              PWORD pwPath,     // NULL: current dir
                              PWORD pwBuffer,   // can be == pwPath
                              DWORD dBuffer,
                              BOOL  fSymbols)
    {
    WORD  awBuffer [MAX_PATH];
    PWORD pwModule1, pwExtension1, pwSymbols;
    DWORD dNext, dName, dExtension, n1, n2, n3, n4;
    DWORD n = 0;

    pwModule1    = (pwModule    != NULL ? pwModule    : gawKernel);
    pwExtension1 = (pwExtension != NULL ? pwExtension : gawDbg);
    pwSymbols    = (fSymbols            ? gawSymbols  : gawNull);

    dNext = imgPathCanonicalW (pwPath, awBuffer, MAX_PATH);
    dName = imgPathNameW (pwModule1, &dExtension);

    if (dNext && (pwModule1 [dExtension  ] == '.')
              &&  pwModule1 [dExtension+1]
        &&
        (dNext + (n1 = lstrlenW (pwSymbols))
               + (n2 = lstrlenW (pwModule1 + (dExtension+1))) + 1
               + (n3 = dExtension - dName)
               + (n4 = lstrlenW (pwExtension1))
         < MAX_PATH))
        {
        n = dNext;

        lstrcpyW (awBuffer + n, pwSymbols);
        n += n1;

        lstrcpyW (awBuffer + n, pwModule1 + (dExtension+1));
        n += n2;
        awBuffer [n++] = '\\';

        lstrcpynW (awBuffer + n, pwModule1 + dName, n3 + 1);
        n += n3;

        lstrcpyW (awBuffer + n, pwExtension1);
        n += n4;
        }
    if ((pwBuffer != NULL) && dBuffer)
        {
        if (n >= dBuffer) n = 0;
        awBuffer [n] = 0;
        lstrcpyW (pwBuffer, awBuffer);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD CALLBACK imgPathCallbackA (PBYTE        pbModule,
                                 PBYTE        pbPath,
                                 DWORD        dPath,
                                 PIMG_CONTEXT pic)
    {
    BYTE  abBuffer [MAX_PATH];
    PBYTE pbBuffer, pbExtension;
    DWORD dBuffer;
    DWORD n = 0;

    if (pic != NULL)
        {
        pbExtension = pic->pbExtension;
        pbBuffer    = pic->pbBuffer;
        dBuffer     = pic->dBuffer;
        }
    else
        {
        pbExtension = NULL;
        pbBuffer    = NULL;
        dBuffer     = 0;
        }
    if (pbPath != NULL)
        {
        if (((n = imgPathSymbolsA (pbModule, pbExtension,
                                   pbPath, abBuffer, MAX_PATH,
                                   TRUE))
             &&
             imgFileTestA (abBuffer))
            ||
            ((n = imgPathSymbolsA (pbModule, pbExtension,
                                   pbPath, abBuffer, MAX_PATH,
                                   FALSE))
             &&
             imgFileTestA (abBuffer)))
            {
            if ((pbBuffer != NULL) && dBuffer)
                {
                if (n >= dBuffer)
                    {
                    abBuffer [0] = 0;
                    n = -1;
                    }
                lstrcpyA (pbBuffer, abBuffer);
                }
            }
        else
            {
            n = 0;
            }
        }
    else
        {
        if ((pbBuffer != NULL) && dBuffer) pbBuffer [0] = 0;
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD CALLBACK imgPathCallbackW (PWORD        pwModule,
                                 PWORD        pwPath,
                                 DWORD        dPath,
                                 PIMG_CONTEXT pic)
    {
    WORD  awBuffer [MAX_PATH];
    PWORD pwBuffer, pwExtension;
    DWORD dBuffer;
    DWORD n = 0;

    if (pic != NULL)
        {
        pwExtension = pic->pwExtension;
        pwBuffer    = pic->pwBuffer;
        dBuffer     = pic->dBuffer;
        }
    else
        {
        pwExtension = NULL;
        pwBuffer    = NULL;
        dBuffer     = 0;
        }
    if (pwPath != NULL)
        {
        if (((n = imgPathSymbolsW (pwModule, pwExtension,
                                   pwPath, awBuffer, MAX_PATH,
                                   TRUE))
             &&
             imgFileTestW (awBuffer))
            ||
            ((n = imgPathSymbolsW (pwModule, pwExtension,
                                   pwPath, awBuffer, MAX_PATH,
                                   FALSE))
             &&
             imgFileTestW (awBuffer)))
            {
            if ((pwBuffer != NULL) && dBuffer)
                {
                if (n >= dBuffer)
                    {
                    awBuffer [0] = 0;
                    n = -1;
                    }
                lstrcpyW (pwBuffer, awBuffer);
                }
            }
        else
            {
            n = 0;
            }
        }
    else
        {
        if ((pwBuffer != NULL) && dBuffer) pwBuffer [0] = 0;
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathSymbolsExA (PBYTE pbModule,
                                PBYTE pbExtension,
                                PBYTE pbBuffer,
                                DWORD dBuffer)
    {
    IMG_CONTEXT ic;
    DWORD       n = 0;

    ic.pbExtension = pbExtension;
    ic.pbBuffer    = pbBuffer;
    ic.dBuffer     = dBuffer;

    n = imgPathEnumerateA (imgPathCallbackA, pbModule, &ic);
    return (n != -1 ? n : 0);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathSymbolsExW (PWORD pwModule,
                                PWORD pwExtension,
                                PWORD pwBuffer,
                                DWORD dBuffer)
    {
    IMG_CONTEXT ic;
    DWORD       n = 0;

    ic.pwExtension = pwExtension;
    ic.pwBuffer    = pwBuffer;
    ic.dBuffer     = dBuffer;

    n = imgPathEnumerateW (imgPathCallbackW, pwModule, &ic);
    return (n != -1 ? n : 0);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathDbgA (PBYTE pbModule,
                          PBYTE pbBuffer,
                          DWORD dBuffer)
    {
    return imgPathSymbolsExA (pbModule, gabDbg, pbBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathDbgW (PWORD pwModule,
                          PWORD pwBuffer,
                          DWORD dBuffer)
    {
    return imgPathSymbolsExW (pwModule, gawDbg, pwBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathPdbA (PBYTE pbModule,
                          PBYTE pbBuffer,
                          DWORD dBuffer)
    {
    return imgPathSymbolsExA (pbModule, gabPdb, pbBuffer, dBuffer);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPathPdbW (PWORD pwModule,
                          PWORD pwBuffer,
                          DWORD dBuffer)
    {
    return imgPathSymbolsExW (pwModule, gawPdb, pwBuffer, dBuffer);
    }

// =================================================================
// FILE MANAGEMENT
// =================================================================

HANDLE WINAPI imgFileClose (HANDLE hf)
    {
    if (hf != INVALID_HANDLE_VALUE) CloseHandle (hf);
    return INVALID_HANDLE_VALUE;
    }

// -----------------------------------------------------------------

HANDLE WINAPI imgFileOpenA (PBYTE pbPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pbPath != NULL) && pbPath [0])
        {
        hf = CreateFileA (pbPath, GENERIC_READ,
                          FILE_SHARE_READ, NULL, OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI imgFileOpenW (PWORD pwPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pwPath != NULL) && pwPath [0])
        {
        hf = CreateFileW (pwPath, GENERIC_READ,
                          FILE_SHARE_READ, NULL, OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI imgFileNewA (PBYTE pbPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pbPath != NULL) && pbPath [0])
        {
        hf = CreateFileA (pbPath, GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                          FILE_FLAG_SEQUENTIAL_SCAN |
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI imgFileNewW (PWORD pwPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((pwPath != NULL) && pwPath [0])
        {
        hf = CreateFileW (pwPath, GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                          FILE_FLAG_SEQUENTIAL_SCAN |
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

BOOL WINAPI imgFileTestA (PBYTE pbPath)
    {
    HANDLE           hff;
    WIN32_FIND_DATAA wfd;
    BOOL             fOk = FALSE;

    if ((hff = FindFirstFileA (pbPath, &wfd))
        != INVALID_HANDLE_VALUE)
        {
        fOk = !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        FindClose (hff);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI imgFileTestW (PWORD pwPath)
    {
    HANDLE           hff;
    WIN32_FIND_DATAW wfd;
    BOOL             fOk = FALSE;

    if ((hff = FindFirstFileW (pwPath, &wfd))
        != INVALID_HANDLE_VALUE)
        {
        fOk = !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        FindClose (hff);
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgFileLoadA (PBYTE  pbPath,
                           PDWORD pdSize)
    {
    HANDLE hf;
    DWORD  n;
    DWORD  dOffset = (pdSize != NULL ? *pdSize : 0);
    DWORD  dSize   = 0;
    PBYTE  pbData  = NULL;

    if ((hf = imgFileOpenA (pbPath)) != INVALID_HANDLE_VALUE)
        {
        if ((dSize = GetFileSize (hf, NULL)) != INVALID_FILE_SIZE)
            {
            if ((dOffset + dSize + 1 > dSize)
                &&
                ((pbData = imgMemoryCreate (dOffset + dSize + 1))
                 != NULL))
                {
                n = 0;

                if ((!dSize)
                    ||
                    (ReadFile (hf, pbData + dOffset, dSize,
                               &n, NULL) &&
                     (dSize == n)))
                    {
                    if (dOffset) ZeroMemory (pbData, dOffset);
                    pbData [dOffset + dSize] = 0;
                    }
                else
                    {
                    pbData = imgMemoryDestroy (pbData);
                    }
                }
            }
        else
            {
            dSize = 0;
            }
        imgFileClose (hf);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pbData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgFileLoadW (PWORD  pwPath,
                           PDWORD pdSize)
    {
    HANDLE hf;
    DWORD  n;
    DWORD  dOffset = (pdSize != NULL ? *pdSize : 0);
    DWORD  dSize   = 0;
    PBYTE  pbData  = NULL;

    if ((hf = imgFileOpenW (pwPath)) != INVALID_HANDLE_VALUE)
        {
        if ((dSize = GetFileSize (hf, NULL)) != INVALID_FILE_SIZE)
            {
            if ((dOffset + dSize + 1 > dSize)
                &&
                ((pbData = imgMemoryCreate (dOffset + dSize + 1))
                 != NULL))
                {
                n = 0;

                if ((!dSize)
                    ||
                    (ReadFile (hf, pbData + dOffset, dSize,
                               &n, NULL) &&
                     (dSize == n)))
                    {
                    if (dOffset) ZeroMemory (pbData, dOffset);
                    pbData [dOffset + dSize] = 0;
                    }
                else
                    {
                    pbData = imgMemoryDestroy (pbData);
                    }
                }
            }
        else
            {
            dSize = 0;
            }
        imgFileClose (hf);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pbData;
    }

// -----------------------------------------------------------------

BOOL WINAPI imgFileSaveA (PBYTE pbPath,
                          PVOID pData,
                          DWORD dData)
    {
    HANDLE hf;
    DWORD  n;
    BOOL   fOk = FALSE;

    if ((hf = imgFileNewA (pbPath)) != INVALID_HANDLE_VALUE)
        {
        n = 0;
        if ((pData != NULL) && dData)
            {
            if (!WriteFile (hf, pData, dData, &n, NULL)) n = 0;
            }
        fOk = (n == dData) && SetEndOfFile (hf);
        imgFileClose (hf);

        if (!fOk) DeleteFileA (pbPath);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI imgFileSaveW (PWORD pwPath,
                          PVOID pData,
                          DWORD dData)
    {
    HANDLE hf;
    DWORD  n;
    BOOL   fOk = FALSE;

    if ((hf = imgFileNewW (pwPath)) != INVALID_HANDLE_VALUE)
        {
        n = 0;
        if ((pData != NULL) && dData)
            {
            if (!WriteFile (hf, pData, dData, &n, NULL)) n = 0;
            }
        fOk = (n == dData) && SetEndOfFile (hf);
        imgFileClose (hf);

        if (!fOk) DeleteFileW (pwPath);
        }
    return fOk;
    }

// =================================================================
// CodeView DATA MANAGEMENT
// =================================================================

PCV_ENTRY WINAPI imgCvEntry (PCV_NB09 pc09,
                             DWORD    dType,
                             DWORD    dIndex)
    {
    DWORD     i, j;
    PCV_ENTRY pce = NULL;

    if ((pc09 != NULL) &&
        (pc09->Header.Signature.dVersion == CV_SIGNATURE_NB09))
        {
        for (i = j = 0; i < pc09->Directory.dEntries; i++)
            {
            if ((pc09->Entries [i].wSubSectionType == dType) &&
                (j++ == dIndex))
                {
                pce = pc09->Entries + i;
                break;
                }
            }
        }
    return pce;
    }

// -----------------------------------------------------------------

PCV_MODULE WINAPI imgCvModule (PCV_NB09 pc09,
                               DWORD    dIndex,
                               PDWORD   pdSize)
    {
    PCV_ENTRY  pce;
    DWORD      dSize = 0;
    PCV_MODULE pcm   = NULL;

    if ((pce = imgCvEntry (pc09, sstModule, dIndex)) != NULL)
        {
        pcm = (PCV_MODULE)
              ((PBYTE) pc09 + pce->lSubSectionOffset);

        dSize = pce->dSubSectionSize;
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pcm;
    }

// -----------------------------------------------------------------

PCV_PUBSYM WINAPI imgCvSymbols (PCV_NB09 pc09,
                                PDWORD   pdCount,
                                PDWORD   pdSize)
    {
    PCV_ENTRY  pce;
    PCV_PUBSYM pcp1;
    DWORD      i;
    DWORD      dCount = 0;
    DWORD      dSize  = 0;
    PCV_PUBSYM pcp    = NULL;

    if ((pce = imgCvEntry (pc09, sstGlobalPub, 0)) != NULL)
        {
        pcp = CV_PUBSYM_DATA ((PBYTE) pc09
                              + pce->lSubSectionOffset);

        dSize = pce->dSubSectionSize;

        for (i  = 0; dSize - i >= CV_PUBSYM_;
             i += CV_PUBSYM_SIZE (pcp1))
            {
            pcp1 = (PCV_PUBSYM) ((PBYTE) pcp + i);
            if (dSize - i < CV_PUBSYM_SIZE (pcp1)) break;
            if (pcp1->Header.wRecordType == CV_PUB32) dCount++;
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    if (pdSize  != NULL) *pdSize  = dSize;
    return pcp;
    }

// -----------------------------------------------------------------

PCV_SEGMAP WINAPI imgCvSegments (PCV_NB09 pc09,
                                 PDWORD   pdCount)
    {
    PCV_ENTRY  pce;
    DWORD      dCount = 0;
    PCV_SEGMAP pcs    = NULL;

    if ((pce = imgCvEntry (pc09, sstSegMap, 0)) != NULL)
        {
        pcs = (PCV_SEGMAP)
              ((PBYTE) pc09 + pce->lSubSectionOffset);

        dCount = pcs->wTotal;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pcs;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgCvPdbA (PCV_NB10 pc10,
                        PBYTE    pbPath,
                        PDWORD   pdSize)
    {
    BYTE  abPath [MAX_PATH];
    DWORD dName, i, n;
    DWORD dSize = 0;
    PVOID pData = NULL;

    if ((pc10 != NULL) &&
        (pc10->Header.Signature.dVersion == CV_SIGNATURE_NB10))
        {
        dName = imgPathNameA (pbPath, NULL);

        if (dName + lstrlenA (pc10->abPdbName) < MAX_PATH)
            {
            for (i = n = 0; i < dName; i++)
                {
                abPath [n++] = pbPath [i];
                }
            for (i = 0; pc10->abPdbName [i]; i++)
                {
                abPath [n++] = pc10->abPdbName [i];
                }
            abPath [n] = 0;

            dSize = (pdSize != NULL ? *pdSize : 0);
            pData = imgPdbLoadA (abPath, &dSize);
            }
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgCvPdbW (PCV_NB10 pc10,
                        PWORD    pwPath,
                        PDWORD   pdSize)
    {
    WORD  awPath [MAX_PATH];
    DWORD dName, i, n;
    DWORD dSize = 0;
    PVOID pData = NULL;

    if ((pc10 != NULL) &&
        (pc10->Header.Signature.dVersion == CV_SIGNATURE_NB10))
        {
        dName = imgPathNameW (pwPath, NULL);

        if (dName + lstrlenA (pc10->abPdbName) < MAX_PATH)
            {
            for (i = n = 0; i < dName; i++)
                {
                awPath [n++] = pwPath [i];
                }
            for (i = 0; pc10->abPdbName [i]; i++)
                {
                awPath [n++] = pc10->abPdbName [i];
                }
            awPath [n] = 0;

            dSize = (pdSize != NULL ? *pdSize : 0);
            pData = imgPdbLoadW (awPath, &dSize);
            }
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pData;
    }

// =================================================================
// DBG FILE MANAGEMENT
// =================================================================

BOOL WINAPI imgDbgVerify (PIMG_DBG pid,
                          DWORD    dSize)
    {
    PIMAGE_DEBUG_DIRECTORY pidd;
    DWORD                  i, n, n1, n2;
    BOOL                   fOk = FALSE;

    if ((pid != NULL) && (dSize >= IMG_DBG_)
        &&
        (pid->Header.Signature == IMAGE_SEPARATE_DEBUG_SIGNATURE)
        &&
        (dSize >= IMG_DBG__ (pid->Header.NumberOfSections) +
                  pid->Header.ExportedNamesSize +
                  pid->Header.DebugDirectorySize)
        &&
        (!(pid->Header.DebugDirectorySize % IMAGE_DEBUG_DIRECTORY_))
        &&
        ((pidd = imgDbgDirectories (pid, &n)) != NULL))
        {
        fOk = TRUE;

        for (i = 0; i < n; i++)
            {
            n1 = pidd [i].PointerToRawData;
            n2 = pidd [i].SizeOfData;

            if ((n1 + n2 < n1) || (n1 + n2 > dSize))
                {
                fOk = FALSE;
                break;
                }
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgDbgLoadA (PBYTE  pbPath,
                          PDWORD pdSize)
    {
    DWORD dOffset = (pdSize != NULL ? *pdSize : 0);
    DWORD dSize   = dOffset;
    PBYTE pbData  = imgFileLoadA (pbPath, &dSize);

    if ((pbData != NULL) &&
        (!imgDbgVerify ((PIMG_DBG) (pbData + dOffset), dSize)))
        {
        pbData = imgMemoryDestroy (pbData);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pbData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgDbgLoadW (PWORD  pwPath,
                          PDWORD pdSize)
    {
    DWORD dOffset = (pdSize != NULL ? *pdSize : 0);
    DWORD dSize   = dOffset;
    PBYTE pbData  = imgFileLoadW (pwPath, &dSize);

    if ((pbData != NULL) &&
        (!imgDbgVerify ((PIMG_DBG) (pbData + dOffset), dSize)))
        {
        pbData = imgMemoryDestroy (pbData);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pbData;
    }

// -----------------------------------------------------------------

PBYTE WINAPI imgDbgExports (PIMG_DBG pid,
                            PDWORD   pdCount)
    {
    DWORD i, j;
    DWORD dCount    = 0;
    PBYTE pbExports = NULL;

    if (pid != NULL)
        {
        pbExports = (PBYTE) pid->aSections
                    + (pid->Header.NumberOfSections
                       * IMAGE_SECTION_HEADER_);

        for (i = 0; i < pid->Header.ExportedNamesSize; i = j)
            {
            if (!pbExports [j = i]) break;

            while ((j < pid->Header.ExportedNamesSize) &&
                   pbExports [j++]);

            if ((j > i) && (!pbExports [j-1])) dCount++;
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pbExports;
    }

// -----------------------------------------------------------------

PIMAGE_DEBUG_DIRECTORY WINAPI imgDbgDirectories (PIMG_DBG pid,
                                                 PDWORD   pdCount)
    {
    DWORD                  dCount = 0;
    PIMAGE_DEBUG_DIRECTORY pidd   = NULL;

    if (pid != NULL)
        {
        pidd   = (PIMAGE_DEBUG_DIRECTORY)
                 ((PBYTE) pid
                  + IMG_DBG__ (pid->Header.NumberOfSections)
                  + pid->Header.ExportedNamesSize);

        dCount = pid->Header.DebugDirectorySize
                 / IMAGE_DEBUG_DIRECTORY_;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pidd;
    }

// -----------------------------------------------------------------

PIMAGE_DEBUG_DIRECTORY WINAPI imgDbgDirectory (PIMG_DBG pid,
                                               DWORD    dType)
    {
    DWORD                  dCount, i;
    PIMAGE_DEBUG_DIRECTORY pidd = NULL;

    if ((pidd = imgDbgDirectories (pid, &dCount)) != NULL)
        {
        for (i = 0; i < dCount; i++, pidd++)
            {
            if (pidd->Type == dType) break;
            }
        if (i == dCount) pidd = NULL;
        }
    return pidd;
    }

// -----------------------------------------------------------------

PCV_DATA WINAPI imgDbgCv (PIMG_DBG pid,
                          PDWORD   pdSize)
    {
    PIMAGE_DEBUG_DIRECTORY pidd;
    DWORD                  dSize = 0;
    PCV_DATA               pcd   = NULL;

    if ((pidd = imgDbgDirectory (pid, IMAGE_DEBUG_TYPE_CODEVIEW))
        != NULL)
        {
        pcd   = IMG_DBG_DATA (pid, pidd);
        dSize = pidd->SizeOfData;
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pcd;
    }

// -----------------------------------------------------------------

PFPO_DATA WINAPI imgDbgFpo (PIMG_DBG pid,
                            PDWORD   pdCount)
    {
    PIMAGE_DEBUG_DIRECTORY pidd;
    DWORD                  dCount = 0;
    PFPO_DATA              pfd    = NULL;

    if ((pidd = imgDbgDirectory (pid, IMAGE_DEBUG_TYPE_FPO))
        != NULL)
        {
        pfd    = IMG_DBG_DATA (pid, pidd);
        dCount = pidd->SizeOfData / FPO_DATA_;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pfd;
    }

// -----------------------------------------------------------------

PIMAGE_DEBUG_MISC WINAPI imgDbgMisc (PIMG_DBG pid,
                                     PDWORD   pdCount)
    {
    PIMAGE_DEBUG_DIRECTORY pidd;
    PIMAGE_DEBUG_MISC      pidm1;
    DWORD                  i;
    DWORD                  dCount = 0;
    PIMAGE_DEBUG_MISC      pidm   = NULL;

    if ((pidd = imgDbgDirectory (pid, IMAGE_DEBUG_TYPE_MISC))
        != NULL)
        {
        pidm = IMG_DBG_DATA (pid, pidd);

        for (i = 0; i < pidd->SizeOfData; i += pidm1->Length)
            {
            pidm1 = (PIMAGE_DEBUG_MISC) ((PBYTE) pidm + i);

            if ((pidd->SizeOfData - i < IMAGE_DEBUG_MISC_) ||
                (pidd->SizeOfData - i < pidm1->Length    )) break;

            dCount++;
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pidm;
    }

// -----------------------------------------------------------------

POMAP_TO_SRC WINAPI imgDbgOmapToSrc (PIMG_DBG pid,
                                     PDWORD   pdCount)
    {
    PIMAGE_DEBUG_DIRECTORY pidd;
    DWORD                  dCount = 0;
    POMAP_TO_SRC           pots   = NULL;

    if ((pidd = imgDbgDirectory (pid,
                                 IMAGE_DEBUG_TYPE_OMAP_TO_SRC))
        != NULL)
        {
        pots   = IMG_DBG_DATA (pid, pidd);
        dCount = pidd->SizeOfData / OMAP_TO_SRC_;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pots;
    }

// -----------------------------------------------------------------

POMAP_FROM_SRC WINAPI imgDbgOmapFromSrc (PIMG_DBG pid,
                                         PDWORD   pdCount)
    {
    PIMAGE_DEBUG_DIRECTORY pidd;
    DWORD                  dCount = 0;
    POMAP_FROM_SRC         pofs   = NULL;

    if ((pidd = imgDbgDirectory (pid,
                                 IMAGE_DEBUG_TYPE_OMAP_FROM_SRC))
        != NULL)
        {
        pofs   = IMG_DBG_DATA (pid, pidd);
        dCount = pidd->SizeOfData / OMAP_FROM_SRC_;
        }
    if (pdCount != NULL) *pdCount = dCount;
    return pofs;
    }

// =================================================================
// PDB FILE MANAGEMENT
// =================================================================

BOOL WINAPI imgPdbVerify (PIMG_PDB pip,
                          DWORD    dSize)
    {
    DWORD i, n;
    BOOL  fOk = FALSE;

    if ((pip != NULL) && (dSize >= IMG_PDB_)
        &&
        (!lstrcmpA (pip->Header.Signature.abSignature,
                    PDB_SIGNATURE_200))
        &&
        pip->Header.dPageSize
        &&
        (dSize >= pip->Header.wFilePages * pip->Header.dPageSize))
        {
        fOk = TRUE;

        n = imgPdbPages (pip, pip->Header.RootStream.dStreamSize);

        for (i = 0; i < n; i++)
            {
            if (pip->Header.awRootPages [i] >=
                pip->Header.wFilePages)
                {
                fOk = FALSE;
                break;
                }
            }
        if (fOk)
            {
            pip->Header.RootStream.pwStreamPages =
                pip->Header.awRootPages;
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbLoadA (PBYTE  pbPath,
                          PDWORD pdSize)
    {
    DWORD dOffset = (pdSize != NULL ? *pdSize : 0);
    DWORD dSize   = dOffset;
    PBYTE pbData  = imgFileLoadA (pbPath, &dSize);

    if ((pbData != NULL) &&
        (!imgPdbVerify ((PIMG_PDB) (pbData + dOffset), dSize)))
        {
        pbData = imgMemoryDestroy (pbData);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pbData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbLoadW (PWORD  pwPath,
                          PDWORD pdSize)
    {
    DWORD dOffset = (pdSize != NULL ? *pdSize : 0);
    DWORD dSize   = dOffset;
    PBYTE pbData  = imgFileLoadW (pwPath, &dSize);

    if ((pbData != NULL) &&
        (!imgPdbVerify ((PIMG_PDB) (pbData + dOffset), dSize)))
        {
        pbData = imgMemoryDestroy (pbData);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pbData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbPage (PIMG_PDB pip,
                         DWORD    dPage)
    {
    PVOID pPage = NULL;

    if ((pip != NULL) && (dPage < pip->Header.wFilePages))
        {
        pPage = (PBYTE) pip + (dPage * pip->Header.dPageSize);
        }
    return pPage;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgPdbPages (PIMG_PDB pip,
                          DWORD    dBytes)
    {
    DWORD dPages = 0;

    if ((pip != NULL) && dBytes)
        {
        dPages = ((dBytes - 1) / pip->Header.dPageSize) + 1;
        }
    return dPages;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbRead (PIMG_PDB    pip,
                         PPDB_STREAM pps)
    {
    DWORD i, j, n;
    PVOID pPage;
    PBYTE pbData = NULL;

    if ((pip != NULL) && (pps != NULL) &&
        ((pbData = imgMemoryCreate (pps->dStreamSize)) != NULL))
        {
        for (i = j = 0; i < pps->dStreamSize; i += n)
            {
            if ((pPage = imgPdbPage (pip, pps->pwStreamPages [j++]))
                == NULL)
                {
                pbData = imgMemoryDestroy (pbData);
                break;
                }
            n = min (pps->dStreamSize - i, pip->Header.dPageSize);
            CopyMemory (pbData + i, pPage, n);
            }
        }
    return pbData;
    }

// -----------------------------------------------------------------

PPDB_ROOT WINAPI imgPdbRoot (PIMG_PDB pip)
    {
    DWORD     i;
    PWORD     pwPages;
    PPDB_ROOT ppr = NULL;

    if ((pip != NULL) &&
        ((ppr = imgPdbRead (pip, &pip->Header.RootStream)) != NULL))
        {
        pwPages = PDB_PAGES (ppr);

        for (i = 0; i < ppr->wCount; i++)
            {
            if (ppr->aStreams [i].dStreamSize != PDB_STREAM_FREE)
                {
                ppr->aStreams [i].pwStreamPages = pwPages;

                pwPages +=
                   imgPdbPages (pip, ppr->aStreams [i].dStreamSize);
                }
            else
                {
                ppr->aStreams [i].pwStreamPages = NULL;
                }
            }
        }
    return ppr;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbStream (PIMG_PDB pip,
                           DWORD    dStream,
                           PDWORD   pdSize)
    {
    PPDB_ROOT ppr;
    DWORD     dSize = 0;
    PVOID     pData = NULL;

    if ((ppr = imgPdbRoot (pip)) != NULL)
        {
        if (dStream < ppr->wCount)
            {
            pData = imgPdbRead (pip, ppr->aStreams + dStream);
            dSize = ppr->aStreams [dStream].dStreamSize;
            }
        imgMemoryDestroy (ppr);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbStreamExA (PBYTE  pbPath,
                              DWORD  dStream,
                              PDWORD pdSize)
    {
    PIMG_PDB pip;
    DWORD    dSize = 0;
    PVOID    pData = NULL;

    if ((pip = imgPdbLoadA (pbPath, NULL)) != NULL)
        {
        pData = imgPdbStream (pip, dStream, &dSize);
        imgMemoryDestroy (pip);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pData;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgPdbStreamExW (PWORD  pwPath,
                              DWORD  dStream,
                              PDWORD pdSize)
    {
    PIMG_PDB pip;
    DWORD    dSize = 0;
    PVOID    pData = NULL;

    if ((pip = imgPdbLoadW (pwPath, NULL)) != NULL)
        {
        pData = imgPdbStream (pip, dStream, &dSize);
        imgMemoryDestroy (pip);
        }
    if (pdSize != NULL) *pdSize = dSize;
    return pData;
    }

// -----------------------------------------------------------------

PPDB_PUBSYM WINAPI imgPdbSymbols (PIMG_PDB pip,
                                  PDWORD   pdCount,
                                  PDWORD   pdSize)
    {
    PPDB_PUBSYM ppp1;
    DWORD       i;
    DWORD       dCount = 0;
    DWORD       dSize  = 0;
    PPDB_PUBSYM ppp    = NULL;

    if ((ppp = imgPdbStream (pip, PDB_STREAM_PUBSYM, &dSize))
        != NULL)
        {
        for (i  = 0; dSize - i >= PDB_PUBSYM_;
             i += PDB_PUBSYM_SIZE (ppp1))
            {
            ppp1 = (PPDB_PUBSYM) ((PBYTE) ppp + i);
            if (dSize - i < PDB_PUBSYM_SIZE (ppp1)) break;
            if (ppp1->Header.wRecordType == PDB_PUB32) dCount++;
            }
        }
    if (pdCount != NULL) *pdCount = dCount;
    if (pdSize  != NULL) *pdSize  = dSize;
    return ppp;
    }

// =================================================================
// MODULE INFO MANAGEMENT
// =================================================================

BOOL WINAPI imgInfoInitialize (PIMG_INFO pii,
                               PVOID     pBase,
                               DWORD     dSize)
    {
    PIMG_DBG pid;
    DWORD    i;
    BOOL     fOk = FALSE;

    if (pii != NULL)
        {
        pid = &pii->DbgFile;

        pii->pBase        =  (pBase != NULL
                              ? pBase
                              : (PVOID) pid->Header.ImageBase);

        pii->pHeader      = &pid->Header;
        pii->pSections    =  pid->aSections;

        pii->pbExports    = imgDbgExports
                                (pid, &pii->dExports);

        pii->pDirectories = imgDbgDirectories
                                (pid, &pii->dDirectories);

        pii->pCvData      = imgDbgCv
                                (pid, &pii->dCvData);

        pii->pFpoEntries  = imgDbgFpo
                                (pid, &pii->dFpoEntries);

        pii->pMiscEntries = imgDbgMisc
                                (pid, &pii->dMiscEntries);

        pii->pOmapToSrc   = imgDbgOmapToSrc
                                (pid, &pii->dOmapToSrc);

        pii->pOmapFromSrc = imgDbgOmapFromSrc
                                (pid, &pii->dOmapFromSrc);

        pii->dSize        = dSize;
        pii->dSections    = pii->pHeader->NumberOfSections;

        for (i = 0; i < pii->dSections; i++)
            {
            pii->pSections [i].PointerToRawData =
                imgInfoOmapSection (pii, i+1);
            }
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

PIMG_INFO WINAPI imgInfoLoadA (PBYTE pbPath,
                               PVOID pBase)
    {
    DWORD     dSize = IMG_INFO_PREFIX;
    PIMG_INFO pii   = NULL;

    if (((pii = imgDbgLoadA (pbPath, &dSize)) != NULL) &&
        imgInfoInitialize (pii, pBase, dSize) &&
        (lstrlenA (pbPath) < MAX_PATH))
        {
        pii->fUnicode = FALSE;
        lstrcpyA (pii->abPath, pbPath);
        }
    else
        {
        pii = imgMemoryDestroy (pii);
        }
    return pii;
    }

// -----------------------------------------------------------------

PIMG_INFO WINAPI imgInfoLoadW (PWORD pwPath,
                               PVOID pBase)
    {
    DWORD     dSize = IMG_INFO_PREFIX;
    PIMG_INFO pii   = NULL;

    if (((pii = imgDbgLoadW (pwPath, &dSize)) != NULL) &&
        imgInfoInitialize (pii, pBase, dSize)          &&
        (lstrlenW (pwPath) < MAX_PATH))
        {
        pii->fUnicode = TRUE;
        lstrcpyW (pii->awPath, pwPath);
        }
    else
        {
        pii = imgMemoryDestroy (pii);
        }
    return pii;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgInfoOmapToSrc (PIMG_INFO pii,
                               DWORD     dTarget,
                               PDWORD    pdOffset)
    {
    POMAP_TO_SRC potc;
    DWORD        i, n;
    DWORD        dOffset = 0;
    DWORD        dSource = 0;

    if (pii != NULL)
        {
        if (n = pii->dOmapToSrc)
            {
            i = (n /= 2);

            while (TRUE)
                {
                potc = NULL;
                if (i >= pii->dOmapToSrc) break;

                potc = pii->pOmapToSrc + i;
                if (dTarget == potc->dTarget) break;

                if (n > 1) n /= 2;

                if (dTarget < potc->dTarget)
                    {
                    i -= n;
                    }
                else
                    {
                    if ((i + 1 == pii->dOmapToSrc) ||
                        (dTarget < (potc+1)->dTarget)) break;

                    i += n;
                    }
                }
            if (potc != NULL)
                {
                dSource = potc->dSource;
                dOffset = dTarget - potc->dTarget;
                }
            }
        else
            {
            dSource = dTarget;
            }
        }
    if (pdOffset != NULL) *pdOffset = dOffset;
    return dSource;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgInfoOmapFromSrc (PIMG_INFO pii,
                                 DWORD     dSource,
                                 PDWORD    pdOffset)
    {
    POMAP_FROM_SRC pofs;
    DWORD          i, n;
    DWORD          dOffset = 0;
    DWORD          dTarget = 0;

    if (pii != NULL)
        {
        if (n = pii->dOmapFromSrc)
            {
            i = (n /= 2);

            while (TRUE)
                {
                pofs = NULL;
                if (i >= pii->dOmapFromSrc) break;

                pofs = pii->pOmapFromSrc + i;
                if (dSource == pofs->dSource) break;

                if (n > 1) n /= 2;

                if (dSource < pofs->dSource)
                    {
                    i -= n;
                    }
                else
                    {
                    if ((i + 1 == pii->dOmapFromSrc) ||
                        (dSource < (pofs+1)->dSource)) break;

                    i += n;
                    }
                }
            if (pofs != NULL)
                {
                dTarget = pofs->dTarget;
                dOffset = dSource - pofs->dSource;
                }
            }
        else
            {
            dTarget = dSource;
            }
        }
    if (pdOffset != NULL) *pdOffset = dOffset;
    return dTarget;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgInfoOmapSection (PIMG_INFO pii,
                                 DWORD     dSection)
    {
    POMAP_TO_SRC          potc;
    PIMAGE_SECTION_HEADER pish;
    DWORD                 i;
    DWORD                 dBase = 0;

    if ((pii != NULL) &&
        dSection && (dSection <= pii->dSections))
        {
        pish = pii->pSections + (dSection-1);

        if (pii->pOmapToSrc)
            {
            for (i = 0; i < pii->dOmapToSrc; i++)
                {
                potc = pii->pOmapToSrc + i;

                if (potc->dTarget >= pish->VirtualAddress +
                                     pish->Misc.VirtualSize) break;

                if ((potc->dTarget >= pish->VirtualAddress) &&
                    potc->dSource)
                    {
                    dBase = (dBase ? min (dBase, potc->dSource)
                                   : potc->dSource);
                    }
                }
            }
        else
            {
            dBase = pish->VirtualAddress;
            }
        }
    return dBase;
    }

// -----------------------------------------------------------------

PIMG_PUBSYM WINAPI imgInfoSymbols (PIMG_INFO pii,
                                   PDWORD    pdCount,
                                   PDWORD    pdSize,
                                   PDWORD    pdVersion)
    {
    PCV_PUBSYM  pcp;
    PIMG_PDB    pPdbFile;
    DWORD       dCount   = 0;
    DWORD       dSize    = 0;
    DWORD       dVersion = 0;
    PIMG_PUBSYM pip      = NULL;

    if ((pii != NULL) && (pii->pCvData != NULL))
        {
        switch (dVersion = pii->pCvData->Header.Signature.dVersion)
            {
            case CV_SIGNATURE_NB09:
                {
                if ((pcp = imgCvSymbols (&pii->pCvData->NB09,
                                         &dCount, &dSize))
                    != NULL)
                    {
                    if (((pip = imgMemoryCreate (dSize)) != NULL) &&
                        dSize)
                        {
                        CopyMemory (pip, pcp, dSize);
                        }
                    }
                break;
                }
            case CV_SIGNATURE_NB10:
                {
                if ((pPdbFile = (pii->fUnicode

                                 ? imgCvPdbW (&pii->pCvData->NB10,
                                              pii->awPath, NULL)

                                 : imgCvPdbA (&pii->pCvData->NB10,
                                              pii->abPath, NULL)))
                    != NULL)
                    {
                    pip = (PIMG_PUBSYM)
                          imgPdbSymbols (pPdbFile, &dCount, &dSize);

                    imgMemoryDestroy (pPdbFile);
                    }
                break;
                }
            }
        }
    if (pdCount   != NULL) *pdCount   = dCount;
    if (pdSize    != NULL) *pdSize    = dSize;
    if (pdVersion != NULL) *pdVersion = dVersion;
    return pip;
    }

// -----------------------------------------------------------------

PBYTE WINAPI imgInfoTypeA (PIMG_INFO pii,
                           DWORD     dType)
    {
    PBYTE pbType = "";

    if ((pii != NULL) && (dType < pii->dDirectories))
        {
        switch (pii->pDirectories [dType].Type)
            {
            case IMAGE_DEBUG_TYPE_UNKNOWN:
                {
                pbType = "Unknown";
                break;
                }
            case IMAGE_DEBUG_TYPE_COFF:
                {
                pbType = "COFF";
                break;
                }
            case IMAGE_DEBUG_TYPE_CODEVIEW:
                {
                pbType = "CodeView";
                break;
                }
            case IMAGE_DEBUG_TYPE_FPO:
                {
                pbType = "Frame Pointer Omission (FPO)";
                break;
                }
            case IMAGE_DEBUG_TYPE_MISC:
                {
                pbType = "Miscellaneous Information";
                break;
                }
            case IMAGE_DEBUG_TYPE_EXCEPTION:
                {
                pbType = "Exception Information";
                break;
                }
            case IMAGE_DEBUG_TYPE_FIXUP:
                {
                pbType = "Fixup Information";
                break;
                }
            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                {
                pbType = "OMAP_TO_SRC";
                break;
                }
            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                {
                pbType = "OMAP_FROM_SRC";
                break;
                }
            case IMAGE_DEBUG_TYPE_BORLAND:
                {
                pbType = "Borland debugging information";
                break;
                }
            case IMAGE_DEBUG_TYPE_RESERVED10:
                {
                pbType = "Reserved #10";
                break;
                }
            case IMAGE_DEBUG_TYPE_CLSID:
                {
                pbType = "Class ID (CLSID)";
                break;
                }
            default:
                {
                pbType = "Undefined";
                break;
                }
            }
        }
    return pbType;
    }

// -----------------------------------------------------------------

PWORD WINAPI imgInfoTypeW (PIMG_INFO pii,
                           DWORD     dType)
    {
    PWORD pwType = L"";

    if ((pii != NULL) && (dType < pii->dDirectories))
        {
        switch (pii->pDirectories [dType].Type)
            {
            case IMAGE_DEBUG_TYPE_UNKNOWN:
                {
                pwType = L"Unknown";
                break;
                }
            case IMAGE_DEBUG_TYPE_COFF:
                {
                pwType = L"COFF";
                break;
                }
            case IMAGE_DEBUG_TYPE_CODEVIEW:
                {
                pwType = L"CodeView";
                break;
                }
            case IMAGE_DEBUG_TYPE_FPO:
                {
                pwType = L"Frame Pointer Omission (FPO)";
                break;
                }
            case IMAGE_DEBUG_TYPE_MISC:
                {
                pwType = L"Miscellaneous Information";
                break;
                }
            case IMAGE_DEBUG_TYPE_EXCEPTION:
                {
                pwType = L"Exception Information";
                break;
                }
            case IMAGE_DEBUG_TYPE_FIXUP:
                {
                pwType = L"Fixup Information";
                break;
                }
            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                {
                pwType = L"OMAP_TO_SRC";
                break;
                }
            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                {
                pwType = L"OMAP_FROM_SRC";
                break;
                }
            case IMAGE_DEBUG_TYPE_BORLAND:
                {
                pwType = L"Borland debugging information";
                break;
                }
            case IMAGE_DEBUG_TYPE_RESERVED10:
                {
                pwType = L"Reserved #10";
                break;
                }
            case IMAGE_DEBUG_TYPE_CLSID:
                {
                pwType = L"Class ID (CLSID)";
                break;
                }
            default:
                {
                pwType = L"Undefined";
                break;
                }
            }
        }
    return pwType;
    }

// =================================================================
// SYMBOL MANAGEMENT
// =================================================================

PIMG_PUBSYM WINAPI imgSymbolNext (PIMG_PUBSYM pip)
    {
    return (pip != NULL ? IMG_PUBSYM_NEXT (pip) : NULL);
    }

// -----------------------------------------------------------------

DWORD WINAPI imgSymbolTest (PIMG_PUBSYM pip)
    {
    return (pip != NULL)
           && 
           ((pip->Header.wRecordType == CV_PUB32 ) ||
            (pip->Header.wRecordType == PDB_PUB32));
    }

// -----------------------------------------------------------------

DWORD WINAPI imgSymbolUndecorate (PBYTE  pbSymbol,
                                  PBYTE  pbBuffer,
                                  PDWORD pdConvention)
    {
    PBYTE apbPrefixes [] = {"__imp__", "__imp_@", "__imp_",
                            "_", "@", "\x7F",
                            NULL};

    BYTE  abBuffer [256] = "";
    DWORD i, j, k, l;
    DWORD dConvention = IMG_CONVENTION_UNDEFINED;
    DWORD dStack      = -1;

    if (pbSymbol != NULL)
        {
        // skip common prefixes
        for (i = j = 0; apbPrefixes [i] != NULL; i++)
            {
            for (j = 0; apbPrefixes [i] [j]; j++)
                {
                if (apbPrefixes [i] [j] != pbSymbol [j]) break;
                }
            if (!apbPrefixes [i] [j]) break;
            j = 0;
            }
        // test for multiple '@'
        for (k = j, l = 0; (l < 2) && pbSymbol [k]; k++)
            {
            if (pbSymbol [k] == '@') l++;
            }
        // don't undecorate if multiple '@', or C++ symbol
        if ((l == 2) || (pbSymbol [0] == '?'))
            {
            j = 0;        // keep prefix
            k = MAXDWORD; // keep length
            }
        else
            {
            // search for next '@'
            for (k = j; pbSymbol [k] && (pbSymbol [k] != '@'); k++);

            // read number of argument stack bytes if '@' found
            if (pbSymbol [k] == '@')
                {
                dStack = 0;

                for (l = k + 1; (pbSymbol [l] >= '0') &&
                                (pbSymbol [l] <= '9'); l++)
                    {
                    dStack *= 10;
                    dStack += pbSymbol [l] - '0';
                    }
                // don't undecorate if non-numeric or empty trailer
                if (pbSymbol [l] || (l == k + 1))
                    {
                    dStack = -1;  // no stack size info

                    j = 0;        // keep prefix
                    k = MAXDWORD; // keep length
                    }
                }
            }
        // determine calling convention if single-char prefix
        if (j == 1)
            {
            switch (pbSymbol [0])
                {
                case '@':
                    {
                    dConvention = IMG_CONVENTION_FASTCALL;
                    break;
                    }
                case '_':
                    {
                    dConvention = (dStack != -1
                                   ? IMG_CONVENTION_STDCALL
                                   : IMG_CONVENTION_CDECL);
                    break;
                    }
                }
            }
        // copy selected name portion
        k = min (k - j, sizeof (abBuffer) - 1);
        lstrcpynA (abBuffer, pbSymbol + j, k + 1);
        }
    if (pbBuffer != NULL)
        {
        lstrcpyA (pbBuffer, abBuffer);
        }
    if (pdConvention != NULL) *pdConvention = dConvention;
    return dStack;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgSymbolExported (PIMG_PUBSYM pip,
                                PIMG_INFO   pii)
    {
    POMF_NAME pon;
    BYTE      abName [256];
    DWORD     i, j;
    DWORD     dId = 0;

    if ((pip != NULL) && (pii != NULL) && pii->dExports)
        {
        switch (pip->Header.wRecordType)
            {
            case CV_PUB32:
                {
                pon = &pip->CvPubSym.Name;
                break;
                }
            case PDB_PUB32:
                {
                pon = &pip->PdbPubSym.Name;
                break;
                }
            default:
                {
                pon = NULL;
                break;
                }
            }
        if (pon != NULL)
            {
            for (i = 0; i < pon->bLength; i++)
                {
                abName [i] = pon->abName [i];
                }
            abName [i] = 0;

            imgSymbolUndecorate (abName, abName, NULL);

            for (i = j = 0; i < pii->dExports; i++)
                {
                if (!lstrcmpA (abName, pii->pbExports + j))
                    {
                    dId = i + 1;
                    break;
                    }
                j += (lstrlenA (pii->pbExports + j) + 1);
                }
            }
        }
    return dId;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgSymbolInfo (PIMG_PUBSYM pip,
                            PIMG_INFO   pii,
                            PIMG_SYMBOL pis,
                            BOOL        fUndecorate)
    {
    DWORD      i;
    IMG_SYMBOL is;
    POMF_NAME  pon = NULL;

    is.pip        = pip;
    is.dSection   = 0;
    is.dRaw       = 0;
    is.dBias      = 0;
    is.dSource    = 0;
    is.dTarget    = 0;
    is.dOffset    = 0;
    is.pBase      = NULL;
    is.pAddress   = NULL;
    is.dStack     = -1;
    is.abName [0] = 0;

    if (pip != NULL)
        {
        switch (pip->Header.wRecordType)
            {
            case CV_PUB32:
                {
                is.dSection =  pip->CvPubSym.wSegment;
                is.dRaw     =  pip->CvPubSym.dOffset;
                pon         = &pip->CvPubSym.Name;
                break;
                }
            case PDB_PUB32:
                {
                is.dSection =  pip->PdbPubSym.wSegment;
                is.dRaw     =  pip->PdbPubSym.dOffset;
                pon         = &pip->PdbPubSym.Name;
                break;
                }
            }
        }
    if ((pii != NULL) &&
        is.dSection && (is.dSection <= pii->dSections) &&
        (is.dBias = pii->pSections[is.dSection-1].PointerToRawData))
        {
        is.dSource   = is.dBias + is.dRaw;
        is.dTarget   = imgInfoOmapFromSrc (pii, is.dSource,
                                           &is.dOffset);

        is.dRelative = (is.dTarget ? is.dTarget + is.dOffset : 0);
        is.pBase     = pii->pBase;
        is.pAddress  = (is.dRelative
                        ? (PBYTE) pii->pBase + is.dRelative
                        : NULL);
        }
    if (pon != NULL)
        {
        for (i = 0; i < pon->bLength; i++)
            {
            is.abName [i] = pon->abName [i];
            }
        is.abName [i] = 0;

        is.dStack = imgSymbolUndecorate
                        (is.abName,
                         (fUndecorate ? is.abName : NULL),
                         &is.dConvention);
        }
    if (pis != NULL) *pis = is;
    return is.pAddress;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgSymbolLookup (PVOID       pAddress,
                              DWORD       dCount,
                              PIMG_PUBSYM pip,
                              PIMG_INFO   pii,
                              PIMG_SYMBOL pis,
                              BOOL        fUndecorate)
    {
    IMG_SYMBOL  is, is1;
    PIMG_PUBSYM pip1;
    DWORD       dOffset1, dIndex;
    DWORD       dOffset = MAXDWORD;

    imgSymbolInfo (NULL, NULL, &is, FALSE);

    if ((pAddress != NULL) && (pip != NULL) && (pii != NULL))
        {
        pip1   = pip;
        dIndex = 0;

        while (dIndex < dCount)
            {
            if (imgSymbolTest (pip1))
                {
                if ((imgSymbolInfo (pip1, pii, &is1, fUndecorate)
                     != NULL)
                    &&
                    ((DWORD_PTR) pAddress
                     >= (DWORD_PTR) is1.pAddress))
                    {
                    dOffset1 = (DWORD_PTR)     pAddress -
                               (DWORD_PTR) is1.pAddress;

                    if ((is.pAddress == NULL) ||
                        (dOffset > dOffset1))
                        {
                        dOffset = dOffset1;
                        is      = is1;
                        }
                    }
                dIndex++;
                }
            pip1 = imgSymbolNext (pip1);
            }
        }
    if (pis != NULL) *pis = is;
    return dOffset;
    }

// -----------------------------------------------------------------

PIMG_TABLE WINAPI imgSymbolTable (DWORD       dCount,
                                  PIMG_PUBSYM pip,
                                  PIMG_INFO   pii)
    {
    DWORD       dSize, i, j;
    IMG_SYMBOL  is;
    PIMG_PUBSYM pip1;
    PIMG_ENTRY  pie;
    PIMG_TABLE  pit = NULL;

    dSize = IMG_TABLE__ (dCount);

    if ((pip != NULL) && (pii != NULL) &&
        ((pit = imgMemoryCreate (dSize)) != NULL))
        {
        pit->dSize        = dSize;
        pit->dSections    = pii->dSections;
        pit->dSymbols     = 0;
        pit->dTimeStamp   = pii->DbgFile.Header.TimeDateStamp;
        pit->dCheckSum    = pii->DbgFile.Header.CheckSum;
        pit->pBase        = pii->pBase;

        pit->piiAddress   = (PIMG_INDEX)
                            (pit->aEntries              + dCount);

        pit->piiName      = (PIMG_INDEX)
                            (pit->piiAddress->apEntries + dCount);

        pit->piiNameIC    = (PIMG_INDEX)
                            (pit->piiName   ->apEntries + dCount);

        if (pit->fUnicode = pii->fUnicode)
            {
            lstrcpyW (pit->awPath, pii->awPath);
            }
        else
            {
            lstrcpyA (pit->abPath, pii->abPath);
            }
        pip1 = pip;

        while (pit->dSymbols < dCount)
            {
            if (imgSymbolTest (pip1))
                {
                imgSymbolInfo (pip1, pii, &is, FALSE);

                pie = pit->aEntries + pit->dSymbols;

                pie->dSection    = is.dSection;
                pie->pAddress    = is.pAddress;
                pie->dConvention = is.dConvention;
                pie->dStack      = is.dStack;
                pie->fExported   = FALSE;

                ZeroMemory (pie->abSection,
                            sizeof (pie->abSection));

                if (is.dSection && (is.dSection <= pii->dSections))
                    {
                    CopyMemory (pie->abSection,
                                pii->pSections [is.dSection-1].Name,
                                IMAGE_SIZEOF_SHORT_NAME);
                    }
                ZeroMemory (pie->abSymbol,
                            sizeof (pie->abDecorated));

                lstrcpyA (pie->abDecorated, is.abName);

                ZeroMemory (pie->abSymbol,
                            sizeof (pie->abSymbol));

                imgSymbolUndecorate (pie->abDecorated,
                                     pie->abSymbol, NULL);

                if (!(pie->fSpecial = (pie->abSymbol [0] == '?')))
                    {
                    for (i = 0; pie->abSymbol [i]; i++)
                        {
                        if (pie->abSymbol [i] == '@')
                            {
                            pie->fSpecial = TRUE;
                            break;
                            }
                        }
                    }
                pit->piiAddress->apEntries [pit->dSymbols] =
                pit->piiName   ->apEntries [pit->dSymbols] =
                pit->piiNameIC ->apEntries [pit->dSymbols] = pie;

                pit->dSymbols++;
                }
            pip1 = imgSymbolNext (pip1);
            }
        if (pit->dSymbols)
            {
            imgSymbolSort (pit->dSymbols,
                           pit->piiAddress, FALSE, FALSE);

            CopyMemory (pit->piiName, pit->piiAddress,
                        IMG_INDEX__ (pit->dSymbols));

            imgSymbolSort (pit->dSymbols,
                           pit->piiName, TRUE, FALSE);

            CopyMemory (pit->piiNameIC, pit->piiName,
                        IMG_INDEX__ (pit->dSymbols));

            imgSymbolSort (pit->dSymbols,
                           pit->piiNameIC, TRUE, TRUE);
            }
        for (i = j = 0; i < pii->dExports; i++)
            {
            if ((pie = imgTableResolve (pit, pii->pbExports + j))
                != NULL)
                {
                pie->fExported = TRUE;
                }
            j += (lstrlenA (pii->pbExports + j) + 1);
            }
        }
    return pit;
    }

// -----------------------------------------------------------------

INT WINAPI imgSymbolCompare (PIMG_ENTRY pie1,
                             PIMG_ENTRY pie2,
                             BOOL       fCompareNames,
                             BOOL       fIgnoreCase)
    {
    DWORD i;
    BYTE  b1, b2;
    INT   iCompare = 0;

    if (pie1 != NULL)
        {
        if (pie2 != NULL)
            {
            if (fCompareNames)
                {
                for (i = 0; !iCompare; i++)
                    {
                    b1 = pie1->abSymbol [i];
                    b2 = pie2->abSymbol [i];

                    if (fIgnoreCase)
                        {
                        b1 = LCase [b1];
                        b2 = LCase [b2];
                        }
                    iCompare = (b1 < b2 ? -1 : (b1 > b2 ? 1 : 0));
                    if ((!b1) || (!b2)) break;
                    }
                }
            else
                {
                iCompare = ((DWORD_PTR) pie1->pAddress <
                            (DWORD_PTR) pie2->pAddress
                            ? -1
                            : ((DWORD_PTR) pie1->pAddress >
                               (DWORD_PTR) pie2->pAddress
                               ? 1
                               : 0));
                }
            if (!iCompare)
                {
                iCompare = (pie1->dConvention <
                            pie2->dConvention
                            ? -1
                            : (pie1->dConvention >
                               pie2->dConvention
                               ? 1
                               : ((DWORD_PTR) pie1 <
                                  (DWORD_PTR) pie2
                                  ? -1
                                  : ((DWORD_PTR) pie1 >
                                     (DWORD_PTR) pie2
                                     ? 1
                                     : 0))));
                }
            }
        else
            {
            iCompare = 1;
            }
        }
    else
        {
        if (pie2 != NULL) iCompare = -1;
        }
    return iCompare;
    }

// -----------------------------------------------------------------

DWORD WINAPI imgSymbolSort (DWORD      dCount,
                            PIMG_INDEX pii,
                            BOOL       fSortByName,
                            BOOL       fIgnoreCase)
    {
    PIMG_ENTRY pie1, pie2;
    DWORD      dDelta, dTrans, i;
    DWORD      n = 0;

    if (pii != NULL)
        {
        if ((dDelta = dCount) > 1)
            {
            dTrans = 1;

            while ((dDelta > 1) || dTrans)
                {
                dTrans  = 0;
                dDelta -= (dDelta > 3 ? dDelta >> 2
                                      : (dDelta > 1 ? 1 : 0));

                for (i = 0; i < dCount - dDelta; i++)
                    {
                    pie1 = pii->apEntries [i       ];
                    pie2 = pii->apEntries [i+dDelta];

                    if (imgSymbolCompare (pie1, pie2,
                                          fSortByName,
                                          fIgnoreCase)
                        > 0)
                        {
                        pii->apEntries [i       ] = pie2;
                        pii->apEntries [i+dDelta] = pie1;

                        dTrans++;
                        }
                    }
                n += dTrans;
                }
            }
        }
    return n;
    }

// =================================================================
// SYMBOL TABLE MANAGEMENT
// =================================================================

PIMG_TABLE WINAPI imgTableLoadA (PBYTE pbPath,
                                 PVOID pBase)
    {
    BYTE        abPath [MAX_PATH];
    DWORD       dCount;
    PIMG_INFO   pii;
    PIMG_PUBSYM pip;
    PIMG_TABLE  pit = NULL;

    if (imgPathDbgA (pbPath, abPath, MAX_PATH) &&
        ((pii = imgInfoLoadA (abPath, pBase)) != NULL))
        {
        if ((pip = imgInfoSymbols (pii, &dCount, NULL, NULL))
            != NULL)
            {
            pit = imgSymbolTable (dCount, pip, pii);
            imgMemoryDestroy (pip);
            }
        imgMemoryDestroy (pii);
        }
    return pit;
    }

// -----------------------------------------------------------------

PIMG_TABLE WINAPI imgTableLoadW (PWORD pwPath,
                                 PVOID pBase)
    {
    WORD        awPath [MAX_PATH];
    DWORD       dCount;
    PIMG_INFO   pii;
    PIMG_PUBSYM pip;
    PIMG_TABLE  pit = NULL;

    if (imgPathDbgW (pwPath, awPath, MAX_PATH) &&
        ((pii = imgInfoLoadW (awPath, pBase)) != NULL))
        {
        if ((pip = imgInfoSymbols (pii, &dCount, NULL, NULL))
            != NULL)
            {
            pit = imgSymbolTable (dCount, pip, pii);
            imgMemoryDestroy (pip);
            }
        imgMemoryDestroy (pii);
        }
    return pit;
    }

// -----------------------------------------------------------------

PIMG_ENTRY WINAPI imgTableLookup (PIMG_TABLE pit,
                                  PVOID      pAddress,
                                  PDWORD     pdOffset)
    {
    PPIMG_ENTRY ppie;
    DWORD       i, n;
    DWORD       dOffset = 0;
    PIMG_ENTRY  pie     = NULL;

    if ((pit != NULL) && (pAddress != NULL) && (n = pit->dSymbols))
        {
        i = (n /= 2);

        while (TRUE)
            {
            pie = NULL;

            if (i >= pit->dSymbols) break;

            ppie =  pit->piiAddress->apEntries + i;
            pie  = *(ppie++);

            if (n > 1) n /= 2;

            if ((DWORD_PTR) pAddress <
                (DWORD_PTR) pie->pAddress)
                {
                i -= n;
                }
            else
                {
                if ((i + 1 == pit->dSymbols) ||
                    ((DWORD_PTR) pAddress <
                     (DWORD_PTR) (*ppie)->pAddress))
                    {
                    if (pie->pAddress == NULL) pie = NULL;
                    break;
                    }
                i += n;
                }
            }
        if (pie != NULL)
            {
            dOffset = (DWORD_PTR) pAddress -
                      (DWORD_PTR) pie->pAddress;
            }
        }
    if (pdOffset != NULL) *pdOffset = dOffset;
    return pie;
    }

// -----------------------------------------------------------------

PIMG_ENTRY WINAPI imgTableResolve (PIMG_TABLE pit,
                                   PBYTE      pbSymbol)
    {
    DWORD       i, j, k, l, n;
    PIMG_ENTRY  pie = NULL;

    if ((pit != NULL) && (pbSymbol != NULL) && (n = pit->dSymbols))
        {
        i = (n /= 2); // current index
        j = 0;        // previous distance
        k = 1;        // loop detector

        while (TRUE)
            {
            pie = NULL;

            if ((i >= pit->dSymbols) || (!k)) break;

            pie = pit->piiName->apEntries [i];

            for (l = 0; pbSymbol [l]; l++)
                {
                if (pbSymbol [l] != pie->abSymbol [l]) break;
                }
            // OK if symbol with defined calling convention
            if ((pbSymbol [l] == pie->abSymbol [l]) &&
                (pie->dConvention != IMG_CONVENTION_UNDEFINED))
                break;

            if (n > 1) n /= 2;

            if (pbSymbol [l] < pie->abSymbol [l])
                {
                k = j - n; // if 0, the sequence is stuck in a loop
                j = 0 - n;
                }
            else
                {
                k = j + n; // if 0, the sequence is stuck in a loop
                j =     n;
                }
            i += j;
            }
        }
    return pie;
    }

// =================================================================
// MODULE MANAGEMENT
// =================================================================

PVOID WINAPI imgModuleExport (PBYTE pbModule,
                              PBYTE pbExport)
    {
    PBYTE   pbModule1;
    HMODULE hModule;
    PVOID   pExport = NULL;

    pbModule1 = (pbModule != NULL ? pbModule : "ntdll.dll");

    if ((hModule = GetModuleHandleA (pbModule1)) != NULL)
        {
        pExport = GetProcAddress (hModule, pbExport);
        }
    return pExport;
    }

// -----------------------------------------------------------------

PMODULE_LIST WINAPI imgModuleList (PDWORD pdData)
    {
    NTPROC       _NtQuerySystemInformation;
    NTSTATUS     ns;
    DWORD        dSize;
    DWORD        dData = 0;
    PMODULE_LIST pml   = NULL;

    if ((_NtQuerySystemInformation =
             imgModuleExport (NULL, "NtQuerySystemInformation"))
        != NULL)
        {
        for (dSize = PAGE_SIZE; (pml == NULL) && dSize; dSize <<= 1)
            {
            if ((pml = imgMemoryCreate (dSize)) == NULL) break;

            ns = _NtQuerySystemInformation (SystemModuleInformation,
                                            pml, dSize, &dData);
            if (ns != STATUS_SUCCESS)
                {
                pml   = imgMemoryDestroy (pml);
                dData = 0;

                if (ns != STATUS_INFO_LENGTH_MISMATCH) break;
                }
            }
        }
    if (pdData != NULL) *pdData = dData;
    return pml;
    }

// -----------------------------------------------------------------

PMODULE_LIST WINAPI imgModuleFindA (PBYTE  pbModule,
                                    PDWORD pdIndex)
    {
    PBYTE        pbModule1;
    DWORD        dName, i;
    DWORD        dIndex = -1;
    PMODULE_LIST pml    = NULL;

    pbModule1 = (pbModule != NULL ? pbModule : gabKernel);

    if ((pml = imgModuleList (NULL)) != NULL)
        {
        dName = imgPathNameA (pbModule1, NULL);

        for (i = 0; i < pml->dModules; i++)
            {
            if (!lstrcmpiA (pml->aModules [i].abPath +
                            pml->aModules [i].wNameOffset,
                            pbModule1 + dName))
                {
                dIndex = i;
                break;
                }
            }
        if (dIndex == -1) pml = imgMemoryDestroy (pml);
        }
    if (pdIndex != NULL) *pdIndex = dIndex;
    return pml;
    }

// -----------------------------------------------------------------

PMODULE_LIST WINAPI imgModuleFindW (PWORD  pwModule,
                                    PDWORD pdIndex)
    {
    PWORD        pwModule1;
    WORD         awModule [MAXIMUM_FILENAME_LENGTH];
    DWORD        dName, i, j, k;
    DWORD        dIndex = -1;
    PMODULE_LIST pml    = NULL;

    pwModule1 = (pwModule != NULL ? pwModule : gawKernel);

    if ((pml = imgModuleList (NULL)) != NULL)
        {
        dName = imgPathNameW (pwModule1, NULL);

        for (i = 0; i < pml->dModules; i++)
            {
            j = 0;
            k = pml->aModules [i].wNameOffset;

            do  {
                awModule [j++] = pml->aModules [i].abPath [k++];
                }
            while (awModule [j-1]);

            if (!lstrcmpiW (awModule, pwModule1 + dName))
                {
                dIndex = i;
                break;
                }
            }
        if (dIndex == -1) pml = imgMemoryDestroy (pml);
        }
    if (pdIndex != NULL) *pdIndex = dIndex;
    return pml;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgModuleBaseA (PBYTE pbModule)
    {
    PMODULE_LIST pml;
    DWORD        dIndex;
    PVOID        pBase = NULL;

    if ((pml = imgModuleFindA (pbModule, &dIndex)) != NULL)
        {
        pBase = pml->aModules [dIndex].pBase;
        imgMemoryDestroy (pml);
        }
    return pBase;
    }

// -----------------------------------------------------------------

PVOID WINAPI imgModuleBaseW (PWORD pwModule)
    {
    PMODULE_LIST pml;
    DWORD        dIndex;
    PVOID        pBase = NULL;

    if ((pml = imgModuleFindW (pwModule, &dIndex)) != NULL)
        {
        pBase = pml->aModules [dIndex].pBase;
        imgMemoryDestroy (pml);
        }
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

    if (dReason == DLL_PROCESS_ATTACH)
        {
        ghInstance = hInstance;
        }
    return fOk;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
