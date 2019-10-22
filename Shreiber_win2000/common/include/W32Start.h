    
// __________________________________________________________
//
//                         W32Start.h
//          Startup Code for Win32 Applications V1.02
//                08-23-2000 Sven B. Schreiber
//                        sbs@orgon.com
// __________________________________________________________
    
#ifndef  _W32START_H_
#define  _W32START_H_
#include <proginfo.h>

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

06-25-1999 V1.00 Original version (SBS).

    This is a replacement for ConStart.h and GuiStart.h, containing
    common startup and library code for both Win32 console and GUI
    applications. Differences are accounted for by checking the
    existence of the _CONSOLE symbol in conditional compilation
    blocks. New enhanced printf(), fprintf(), vfprintf(), sprintf(),
    and vsprintf() implementations for Unicode and ANSI. mprintf()
    and vmprintf() are derivatives of fprintf() and vfprintf(),
    displaying the data in a MessageBox() instead of writing it to
    a stream. All *printf() functions impose no restrictions on the
    size of the data and have various new formatting features. If
    sprintf() and vsprintf() receive a buffer pointer of NULL, the
    required buffer size is computed only. GUI applications are
    allowed to call printf(), which will automatically create a
    console window on first invocation. The console status can also
    be controlled explicitly using the ConsoleCreate() and
    ConsoleDestroy() run-time functions. The console line length
    computation has moved from Main() to the startup code. New
    command line parser, featuring an argp[] array of pointers to
    the arguments in the raw command line. Main() and WinMain()
    prototypes slightly revised for the new parser.

07-17-1999 V1.01 Upgrade (SBS).

    Added vprintfA(), vprintfW(), and the corresponding ANSI/Unicode
    macro vprintf. Changed printf() to use vprintf() now instead of
    vfprintf().

08-23-2000 V1.02 Upgrade (SBS).

    Moved the WriteFile() calls into a loop that outputs the data
    in 16-KB blocks. Moved the ConsoleCreate() call inside printf()
    to vprintf(). Added eprintf() and veprintf(), which write to
    STDERR instead of STDOUT. Added ConsoleTest() and redesigned
    ConsoleCreate() and ConsoleDestroy(). Renamed ConsoleCreate() to
    ConsoleOpen() and ConsoleDestroy() to ConsoleClose().

*/

////////////////////////////////////////////////////////////////////
#ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

// =================================================================
// MACROS
// =================================================================

#define ALLOC_ANSI(_n) \
    ((PBYTE) LocalAlloc (LMEM_FIXED, (_n)))

#define ALLOC_UNICODE(_n) \
    ((PWORD) LocalAlloc (LMEM_FIXED, (_n) * sizeof (WORD)))

// =================================================================
// CONSTANTS
// =================================================================

#define UNICODE_UNMAPPED 0x7F
#define WRITE_FILE_BLOCK 0x4000
#define CONSOLE_TITLE    0x1000

// =================================================================
// STRUCTURES
// =================================================================

typedef struct _WIN_CMD
    {
    DWORD   argc;
    PTBYTE *argv;
    PTBYTE *argp;
    PTBYTE  ptRaw;
    PTBYTE  ptCooked;
    PTBYTE  ptTail;
    BYTE    abData [];
    }
    WIN_CMD, *PWIN_CMD, **PPWIN_CMD;

#define WIN_CMD_ sizeof (WIN_CMD)

// =================================================================
// GLOBAL VARIABLES
// =================================================================

HANDLE ghStdInput     = INVALID_HANDLE_VALUE;
HANDLE ghStdOutput    = INVALID_HANDLE_VALUE;
HANDLE ghStdError     = INVALID_HANDLE_VALUE;

BOOL   gfStdHandles   = FALSE;
BOOL   gfStdFailure   = FALSE;
BOOL   gfStdTransient = FALSE;

DWORD  gdLine         = 80;
INT    giNetwork      = 0;
INT    giDebug        = 0;
WORD   gwFill         = ' ';

// =================================================================
// GLOBAL STRINGS
// =================================================================

TBYTE atUsage [] = T("\r\n")
                   T("Usage: ") S(MAIN_MODULE) T(" %s\r\n");

TBYTE atAbout [] = T("\r\n")
                   T("// ") S(ABOUT_TEXT1) T("\r\n")
                   T("// ") S(ABOUT_TEXT2) T("\r\n")
                   T("// ") S(ABOUT_TEXT3) T("\r\n")
                   T("// ") S(ABOUT_TEXT4) T("\r\n");

// =================================================================
// CONDITIONAL ANSI/UNICODE NAMES
// =================================================================

#ifdef UNICODE

#define ConsoleTest ConsoleTestW

#define vsprintf    vsprintfW
#define vmprintf    vmprintfW
#define vfprintf    vfprintfW
#define veprintf    veprintfW
#define vprintf     vprintfW

#define sprintf     sprintfW
#define mprintf     mprintfW
#define fprintf     fprintfW
#define eprintf     eprintfW
#define printf      printfW

#else

#define ConsoleTest ConsoleTestA

#define vsprintf    vsprintfA
#define vmprintf    vmprintfA
#define vfprintf    vfprintfA
#define veprintf    veprintfA
#define vprintf     vprintfA

#define sprintf     sprintfA
#define mprintf     mprintfA
#define fprintf     fprintfA
#define eprintf     eprintfA
#define printf      printfA

#endif

// =================================================================
// CONSOLE MANAGEMENT
// =================================================================

BOOL WINAPI ConsoleTestA (void)
    {
    BYTE abTitle [CONSOLE_TITLE];

    return GetConsoleTitleA (abTitle, CONSOLE_TITLE) != 0;
    }

// -----------------------------------------------------------------

BOOL WINAPI ConsoleTestW (void)
    {
    WORD awTitle [CONSOLE_TITLE];

    return GetConsoleTitleW (awTitle, CONSOLE_TITLE) != 0;
    }

// -----------------------------------------------------------------

BOOL WINAPI ConsoleOpen (void)
    {
    if ((!gfStdHandles) && (!gfStdFailure))
        {
        gfStdTransient = (!ConsoleTest ()) && AllocConsole ();

        if (ConsoleTest ())
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

BOOL WINAPI ConsoleClose (void)
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
        if (ConsoleTest ()) FreeConsole ();
        gfStdTransient = FALSE;
        }
    gfStdFailure = FALSE;
    return fOk;
    }

// =================================================================
// TEXT CONVERSION ROUTINES
// =================================================================

PWORD WINAPI ConvertAnsiToUnicode (PBYTE pbData,
                                   PWORD pwData)
    {
    DWORD n;
    PWORD pwData1 = NULL;

    if (pbData != NULL)
        {
        for (n = 0; pbData [n]; n++);

        if ((pwData1 = (pwData != NULL ? pwData
                                       : ALLOC_UNICODE (n+1)))
            != NULL)
            {
            do  {
                pwData1 [n] = pbData [n];
                }
            while (n--);
            }
        }
    return pwData1;
    }

// -----------------------------------------------------------------

PBYTE WINAPI ConvertUnicodeToAnsi (PWORD pwData,
                                   PBYTE pbData)
    {
    DWORD n;
    PBYTE pbData1 = NULL;

    if (pwData != NULL)
        {
        for (n = 0; pwData [n]; n++);

        if ((pbData1 = (pbData != NULL ? pbData
                                       : ALLOC_ANSI (n+1)))
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

// =================================================================
// printf() PRIMITIVES
// =================================================================

DWORD WINAPI FormatAnsiW (PWORD pwBuffer,
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

DWORD WINAPI FormatUnicodeW (PWORD pwBuffer,
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

DWORD WINAPI FormatDecimalW (PWORD pwBuffer,
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

DWORD WINAPI FormatHexW (PWORD pwBuffer,
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

DWORD WINAPI FormatSingleW (PWORD  pwBuffer,
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
                        n = FormatAnsiW
                                (pwBuffer, dOffset,
                                 pText, -1,
                                 dWidth, dPrecision, wFill1,
                                 fRight, fZero);
                        }
                    else
                        {
                        n = FormatUnicodeW
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
                    n = FormatUnicodeW (pwBuffer, dOffset,
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

                    n = FormatDecimalW (pwBuffer, dOffset, dData,
                                        dWidth, dPrecision, wFill1,
                                        fRight, fZero, fPrefix,
                                        fSigned);

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

                    n = FormatHexW (pwBuffer, dOffset, dData,
                                    dWidth, dPrecision, wFill1,
                                    fRight, fZero, fPrefix,
                                    fLower);

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

DWORD WINAPI FormatMultiW (PWORD pwBuffer,
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

                n += FormatUnicodeW (pwBuffer, dOffset+n,
                                     pwFormat+i, j-i,
                                     0, -1, wFill,
                                     TRUE, FALSE);

                if (pwFormat [j] == '%')
                    {
                    j++;

                    n += FormatSingleW (pwBuffer, dOffset+n,
                                        pwFormat, &j, &pData,
                                        wFill, fUnicode);
                    }
                }
            }
        else
            {
            n = FormatUnicodeW (pwBuffer, dOffset,
                                pwFormat, -1,
                                0, -1, wFill,
                                TRUE, FALSE);
            }
        }
    if (pwBuffer != NULL) pwBuffer [dOffset+n] = 0;
    return n;
    }

// =================================================================
// vprintf() ROUTINES (UNICODE)
// =================================================================

DWORD WINAPI vsprintfW (PWORD pwBuffer,
                        PWORD pwFormat,
                        PVOID pArguments)
    {
    return FormatMultiW (pwBuffer, 0, pwFormat, pArguments,
                         gwFill, TRUE);
    }

// -----------------------------------------------------------------

INT WINAPI vmprintfW (HWND  hWnd,
                      UINT  uiType,
                      PWORD pwCaption,
                      PWORD pwFormat,
                      PVOID pArguments)
    {
    PWORD pwBuffer;
    DWORD dBuffer;
    INT   iId = IDABORT;

    dBuffer = vsprintfW (NULL, pwFormat, pArguments);

    if ((pwBuffer = ALLOC_UNICODE (dBuffer+1))
        != NULL)
        {
        vsprintfW (pwBuffer, pwFormat, pArguments);

        iId = MessageBoxW (hWnd, pwBuffer,
                           (pwCaption != NULL
                            ? pwCaption
                            : SW(MAIN_CAPTION)),
                           uiType);

        LocalFree (pwBuffer);
        }
    return iId;
    }

// -----------------------------------------------------------------

DWORD WINAPI vfprintfW (HANDLE hFile,
                        PWORD  pwFormat,
                        PVOID  pArguments)
    {
    PWORD pwBuffer;
    PBYTE pbBuffer;
    DWORD dBuffer, n1, n2;
    DWORD n = 0;

    dBuffer = vsprintfW (NULL, pwFormat, pArguments);

    if ((pwBuffer = ALLOC_UNICODE (dBuffer+1))
        != NULL)
        {
        vsprintfW (pwBuffer, pwFormat, pArguments);

        if ((pbBuffer = ConvertUnicodeToAnsi (pwBuffer, NULL))
            != NULL)
            {
            while (dBuffer > n)
                {
                n1 = min (dBuffer-n, WRITE_FILE_BLOCK);
                n2 = 0;

                if (WriteFile (hFile, pbBuffer+n, n1, &n2, NULL)
                    && n2)
                    {
                    n += n2;
                    }
                else break;
                }
            LocalFree (pbBuffer);
            }
        LocalFree (pwBuffer);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI veprintfW (PWORD pwFormat,
                        PVOID pArguments)
    {
    ConsoleOpen ();
    return vfprintfW (ghStdError, pwFormat, pArguments);
    }

// -----------------------------------------------------------------

DWORD WINAPI vprintfW (PWORD pwFormat,
                       PVOID pArguments)
    {
    ConsoleOpen ();
    return vfprintfW (ghStdOutput, pwFormat, pArguments);
    }

// =================================================================
// printf() ROUTINES (UNICODE)
// =================================================================

DWORD WINAPI sprintfW (PWORD pwBuffer,
                       PWORD pwFormat,
                       ...)
    {
    return vsprintfW (pwBuffer, pwFormat, (&pwFormat)+1);
    }

// -----------------------------------------------------------------

INT WINAPI mprintfW (HWND  hWnd,
                     UINT  uiType,
                     PWORD pwCaption,
                     PWORD pwFormat,
                     ...)
    {
    return vmprintfW (hWnd, uiType, pwCaption, pwFormat,
                      (&pwFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI fprintfW (HANDLE hFile,
                       PWORD  pwFormat,
                       ...)
    {
    return vfprintfW (hFile, pwFormat, (&pwFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI eprintfW (PWORD pwFormat,
                       ...)
    {
    return veprintfW (pwFormat, (&pwFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI printfW (PWORD pwFormat,
                      ...)
    {
    return vprintfW (pwFormat, (&pwFormat)+1);
    }

// =================================================================
// vprintf() ROUTINES (ANSI)
// =================================================================

DWORD WINAPI vsprintfA (PBYTE pbBuffer,
                        PBYTE pbFormat,
                        PVOID pArguments)
    {
    PWORD pwBuffer, pwFormat;
    DWORD n = 0;

    if ((pwFormat = ConvertAnsiToUnicode (pbFormat, NULL))
        != NULL)
        {
        n = FormatMultiW (NULL, 0, pwFormat, pArguments,
                          gwFill, FALSE);

        if (pbBuffer != NULL)
            {
            if ((pwBuffer = ALLOC_UNICODE (n+1))
                != NULL)
                {
                FormatMultiW (pwBuffer, 0, pwFormat, pArguments,
                              gwFill, FALSE);

                ConvertUnicodeToAnsi (pwBuffer, pbBuffer);
                LocalFree (pwBuffer);
                }
            else
                {
                pbBuffer [n = 0] = 0;
                }
            }
        LocalFree (pwFormat);
        }
    return n;
    }

// -----------------------------------------------------------------

INT WINAPI vmprintfA (HWND  hWnd,
                      UINT  uiType,
                      PBYTE pbCaption,
                      PBYTE pbFormat,
                      PVOID pArguments)
    {
    PWORD pwFormat;
    PWORD pwBuffer;
    PBYTE pbBuffer;
    DWORD dBuffer;
    INT   iId = IDABORT;

    if ((pwFormat = ConvertAnsiToUnicode (pbFormat, NULL))
        != NULL)
        {
        dBuffer = FormatMultiW (NULL, 0, pwFormat, pArguments,
                                gwFill, FALSE);

        if ((pwBuffer = ALLOC_UNICODE (dBuffer+1))
            != NULL)
            {
            FormatMultiW (pwBuffer, 0, pwFormat, pArguments,
                          gwFill, FALSE);

            if ((pbBuffer = ConvertUnicodeToAnsi (pwBuffer, NULL))
                != NULL)
                {
                iId = MessageBoxA (hWnd, pbBuffer,
                                   (pbCaption != NULL
                                    ? pbCaption
                                    : SA(MAIN_CAPTION)),
                                   uiType);

                LocalFree (pbBuffer);
                }
            LocalFree (pwBuffer);
            }
        LocalFree (pwFormat);
        }
    return iId;
    }

// -----------------------------------------------------------------

DWORD WINAPI vfprintfA (HANDLE hFile,
                        PBYTE  pbFormat,
                        PVOID  pArguments)
    {
    PWORD pwFormat;
    PWORD pwBuffer;
    PBYTE pbBuffer;
    DWORD dBuffer, n1, n2;
    DWORD n = 0;

    if ((pwFormat = ConvertAnsiToUnicode (pbFormat, NULL))
        != NULL)
        {
        dBuffer = FormatMultiW (NULL, 0, pwFormat, pArguments,
                                gwFill, FALSE);

        if ((pwBuffer = ALLOC_UNICODE (dBuffer+1))
            != NULL)
            {
            FormatMultiW (pwBuffer, 0, pwFormat, pArguments,
                          gwFill, FALSE);

            if ((pbBuffer = ConvertUnicodeToAnsi (pwBuffer, NULL))
                != NULL)
                {
                while (dBuffer > n)
                    {
                    n1 = min (dBuffer-n, WRITE_FILE_BLOCK);
                    n2 = 0;

                    if (WriteFile (hFile, pbBuffer+n, n1, &n2, NULL)
                        && n2)
                        {
                        n += n2;
                        }
                    else break;
                    }
                LocalFree (pbBuffer);
                }
            LocalFree (pwBuffer);
            }
        LocalFree (pwFormat);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI veprintfA (PBYTE pbFormat,
                        PVOID pArguments)
    {
    ConsoleOpen ();
    return vfprintfA (ghStdError, pbFormat, pArguments);
    }

// -----------------------------------------------------------------

DWORD WINAPI vprintfA (PBYTE pbFormat,
                       PVOID pArguments)
    {
    ConsoleOpen ();
    return vfprintfA (ghStdOutput, pbFormat, pArguments);
    }

// =================================================================
// printf() ROUTINES (ANSI)
// =================================================================

DWORD WINAPI sprintfA (PBYTE pbBuffer,
                       PBYTE pbFormat,
                       ...)
    {
    return vsprintfA (pbBuffer, pbFormat, (&pbFormat)+1);
    }

// -----------------------------------------------------------------

INT WINAPI mprintfA (HWND  hWnd,
                     UINT  uiType,
                     PBYTE pbCaption,
                     PBYTE pbFormat,
                     ...)
    {
    return vmprintfA (hWnd, uiType, pbCaption,
                      pbFormat, (&pbFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI fprintfA (HANDLE hFile,
                       PBYTE  pbFormat,
                       ...)
    {
    return vfprintfA (hFile, pbFormat, (&pbFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI eprintfA (PBYTE pbFormat,
                       ...)
    {
    return veprintfA (pbFormat, (&pbFormat)+1);
    }

// -----------------------------------------------------------------

DWORD WINAPI printfA (PBYTE pbFormat,
                      ...)
    {
    return vprintfA (pbFormat, (&pbFormat)+1);
    }

// =================================================================
// PROTOTYPES
// =================================================================

#ifdef _CONSOLE

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp);

#else

#pragma warning (disable: 4028)

int WINAPI WinMain (HINSTANCE hInstance,
                    PWIN_CMD  pwc,
                    PTBYTE    ptCmdLine,
                    int       iShowCmd);

#pragma warning (default: 4028)

#endif

// =================================================================
// COMMAND PARSER
// =================================================================

PWIN_CMD W32Command (void)
    {
    PTBYTE   ptRaw;
    DWORD    dCount, dRaw, dCooked;
    PWIN_CMD pwc = NULL;

    if ((ptRaw = GetCommandLine ()) != NULL)
        {
        for (dCount = dRaw = dCooked = 0; ptRaw [dRaw];
             dCount++)
            {
            while (ptRaw [dRaw] == ' ') dRaw++;

            if (!ptRaw [dRaw]) break;

            while ( ptRaw [dRaw] &&
                   (ptRaw [dRaw] != ' '))
                {
                if (ptRaw [dRaw++] != '"')
                    {
                    dCooked++;
                    }
                else
                    {
                    while ( ptRaw [dRaw  ] &&
                           (ptRaw [dRaw++] != '"'))
                        {
                        dCooked++;
                        }
                    }
                }
            dCooked++;
            }
        dRaw++;

        if ((pwc = LocalAlloc (LMEM_FIXED,
                               WIN_CMD_                       +
                               (dCount * 2 * sizeof (PTBYTE)) +
                               (dRaw       * sizeof ( TBYTE)) +
                               (dCooked    * sizeof ( TBYTE))))
            != NULL)
            {
            pwc->argc     =             dCount;
            pwc->argv     = (PTBYTE *)  pwc->abData;
            pwc->argp     =             pwc->argv  + dCount;
            pwc->ptRaw    = (PTBYTE  ) (pwc->argp  + dCount);
            pwc->ptCooked =             pwc->ptRaw + dRaw;
            pwc->ptTail   =             NULL;

            CopyMemory (pwc->ptRaw, ptRaw, dRaw * sizeof ( TBYTE));

            for (dCount = dRaw = dCooked = 0; pwc->ptRaw [dRaw];
                 dCount++)
                {
                while (pwc->ptRaw [dRaw] == ' ') dRaw++;

                if (!pwc->ptRaw [dRaw]) break;

                pwc->argp [dCount] = pwc->ptRaw    + dRaw;
                pwc->argv [dCount] = pwc->ptCooked + dCooked;

                while ( pwc->ptRaw [dRaw] &&
                       (pwc->ptRaw [dRaw] != ' '))
                    {
                    if (pwc->ptRaw [dRaw++] != '"')
                        {
                        pwc->ptCooked [dCooked++] =
                        pwc->ptRaw    [dRaw-1];
                        }
                    else
                        {
                        while ( pwc->ptRaw [dRaw  ] &&
                               (pwc->ptRaw [dRaw++] != '"'))
                            {
                            pwc->ptCooked [dCooked++] =
                            pwc->ptRaw    [dRaw-1];
                            }
                        }
                    }
                pwc->ptCooked [dCooked++] = 0;
                }
            pwc->ptTail = (pwc->argc > 1 ? pwc->argp [1]
                                         : pwc->ptRaw + dRaw);
            }
        }
    return pwc;
    }

// =================================================================
// STARTUP CODE
// =================================================================

void W32Start (void)
    {

#ifdef _CONSOLE

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD                      dCodePage;

#else

    STARTUPINFO                si;
    INT                        iShowCmd;

#endif

    PWIN_CMD                   pwc;
    DWORD                      dStatus = 0;

    if ((pwc = W32Command ()) != NULL)
        {
        giNetwork = GetSystemMetrics (SM_NETWORK);
        giDebug   = GetSystemMetrics (SM_DEBUG  );

#ifdef _CONSOLE
// -----------------------------------------------------------------

        if (ConsoleOpen ())
            {
            dCodePage = GetConsoleOutputCP ();
            SetConsoleOutputCP (GetACP ());
            GetConsoleScreenBufferInfo (ghStdOutput, &csbi);
            gdLine = csbi.dwSize.X;

            dStatus = Main (pwc->argc, pwc->argv, pwc->argp);

            SetConsoleOutputCP (dCodePage);
            }
#else
// -----------------------------------------------------------------

        GetStartupInfo (&si);

        iShowCmd = (si.dwFlags & STARTF_USESHOWWINDOW
                    ? (INT) si.wShowWindow
                    : SW_SHOWDEFAULT);

        dStatus = WinMain (GetModuleHandle (NULL),
                           pwc, pwc->ptTail,  iShowCmd);

#endif
// -----------------------------------------------------------------

        ConsoleClose ();
        LocalFree (pwc);
        }
    ExitProcess (dStatus);
    return;
    }

// =================================================================
// LINKER CONTROL
// =================================================================

#pragma comment (linker, "/entry:\"W32Start\"")

////////////////////////////////////////////////////////////////////
#endif // #ifndef _RC_PASS_
////////////////////////////////////////////////////////////////////

#endif // #ifndef _W32START_H_

// =================================================================
// END OF FILE
// =================================================================
