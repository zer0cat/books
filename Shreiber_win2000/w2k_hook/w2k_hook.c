
// __________________________________________________________
//
//                         w2k_hook.c
//           SBS Windows 2000 API Hook Viewer V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_hook.h"

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
// GLOBAL VARIABLES
// =================================================================

BOOL gfSpyUnload = FALSE;

// =================================================================
// GLOBAL STRINGS
// =================================================================

WORD awSpyFile      [] = SW(DRV_FILENAME);
WORD awSpyDevice    [] = SW(DRV_MODULE);
WORD awSpyDisplay   [] = SW(DRV_NAME);
WORD awSpyPath      [] = SW(DRV_PATH);

// -----------------------------------------------------------------

WORD awArguments    [] = L"<pattern #1> ... <pattern #N>\r\n\r\n"
                         L"Examples: " SW(MAIN_MODULE) L" *\r\n"
                         L"          " SW(MAIN_MODULE) L" *key\r\n"
                         L"          " SW(MAIN_MODULE)
                         L" ntopenfile ntcreatefile"
                         L" nt*informationfile";

// -----------------------------------------------------------------

BYTE abPauseOff     [] = "---------- PROTOCOL RESUMED ----------\n";
BYTE abPauseOn      [] = "---------- PROTOCOL PAUSED -----------\n";

BYTE abFilterOff    [] = "---------- FILTER DISABLED -----------\n";
BYTE abFilterOn     [] = "---------- FILTER ENABLED ------------\n";

BYTE abReset        [] = "---------- PROTOCOL RESET ------------\n";
BYTE abExit         [] = "---------- PROTOCOL STOPPED ----------";

// =================================================================
// CONSOLE I/O
// =================================================================

DWORD KeyboardData (void)
    {
    INPUT_RECORD InputRecord;
    DWORD        dCount;
    DWORD        dKeyCode = 0;

    GetNumberOfConsoleInputEvents (ghStdInput, &dCount);

    while (dCount &&
           ReadConsoleInput (ghStdInput, &InputRecord, 1, &dCount))
        {
        if ((InputRecord.EventType == KEY_EVENT) &&
            (InputRecord.Event.KeyEvent.bKeyDown))
            {
            dKeyCode = InputRecord.Event.KeyEvent.wVirtualKeyCode;
            break;
            }
        GetNumberOfConsoleInputEvents (ghStdInput, &dCount);
        }
    return dKeyCode;
    }

// =================================================================
// PATTERN MATCHER
// =================================================================

BOOL WINAPI PatternMatcher (PWORD pwFilter,
                            PWORD pwData)
    {
    DWORD i, j;

    i = j = 0;
    while (pwFilter [i] && pwData [j])
        {
        if (pwFilter [i] != '?')
            {
            if (pwFilter [i] == '*')
                {
                i++;
                if ((pwFilter [i] != '*') && (pwFilter [i] != '?'))
                    {
                    if (pwFilter [i])
                        {
                        while (pwData [j] &&
                               (!PatternMatcher (pwFilter + i,
                                                 pwData   + j)))
                            {
                            j++;
                            }
                        }
                    return (pwData [j]);
                    }
                }
            if ((WORD) CharUpperW ((PWORD) (pwFilter [i])) !=
                (WORD) CharUpperW ((PWORD) (pwData   [j])))
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
// SPY DEVICE I/O
// =================================================================

BOOL WINAPI SpyIoControl (HANDLE hDevice,
                          DWORD  dCode,
                          PVOID  pInput,
                          DWORD  dInput,
                          PVOID  pOutput,
                          DWORD  dOutput)
    {
    DWORD dInfo = 0;

    return DeviceIoControl (hDevice, dCode,
                            pInput,  dInput,
                            pOutput, dOutput,
                            &dInfo,  NULL)
           &&
           (dInfo == dOutput);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyVersionInfo (HANDLE            hDevice,
                            PSPY_VERSION_INFO psvi)
    {
    return SpyIoControl (hDevice, SPY_IO_VERSION_INFO,
                         NULL, 0,
                         psvi, SPY_VERSION_INFO_);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookInfo (HANDLE         hDevice,
                         PSPY_HOOK_INFO pshi)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_INFO,
                         NULL, 0,
                         pshi, SPY_HOOK_INFO_);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookInstall (HANDLE hDevice,
                            BOOL   fReset,
                            PDWORD pdCount)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_INSTALL,
                         &fReset, BOOL_,
                         pdCount, DWORD_);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookRemove (HANDLE hDevice,
                           BOOL   fReset,
                           PDWORD pdCount)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_REMOVE,
                         &fReset, BOOL_,
                         pdCount, DWORD_);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookPause (HANDLE hDevice,
                          BOOL   fPause,
                          PBOOL  pfPause)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_PAUSE,
                         &fPause, BOOL_,
                         pfPause, BOOL_);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookFilter (HANDLE hDevice,
                           BOOL   fFilter,
                           PBOOL  pfFilter)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_FILTER,
                         &fFilter, BOOL_,
                         pfFilter, BOOL_);
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookReset (HANDLE hDevice)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_RESET,
                         NULL, 0,
                         NULL, 0);
    }

// -----------------------------------------------------------------

DWORD WINAPI SpyHookRead (HANDLE hDevice,
                          BOOL   fLine,
                          PBYTE  pbData,
                          DWORD  dData)
    {
    DWORD dInfo;

    if (!DeviceIoControl (hDevice, SPY_IO_HOOK_READ,
                          &fLine, BOOL_,
                          pbData, dData,
                          &dInfo, NULL))
        {
        dInfo = 0;
        }
    return dInfo;
    }

// -----------------------------------------------------------------

BOOL WINAPI SpyHookWrite (HANDLE hDevice,
                          PBYTE  pbData)
    {
    return SpyIoControl (hDevice, SPY_IO_HOOK_WRITE,
                         pbData, lstrlenA (pbData),
                         NULL,   0);
    }

// =================================================================
// SPY DEVICE MANAGEMENT
// =================================================================

void WINAPI Execute (PPWORD ppwFilters,
                     DWORD  dFilters)
    {
    SPY_VERSION_INFO svi;
    SPY_HOOK_INFO    shi;
    DWORD            dCount, i, j, k, n;
    BOOL             fPause, fFilter, fRepeat;
    BYTE             abData [HOOK_MAX_DATA];
    WORD             awData [HOOK_MAX_DATA];
    WORD             awPath [MAX_PATH] = L"?";
    SC_HANDLE        hControl          = NULL;
    HANDLE           hDevice           = INVALID_HANDLE_VALUE;

    _printf (L"\r\nLoading \"%s\" (%s) ...\r\n",
             awSpyDisplay, awSpyDevice);

    if (w2kFilePath (NULL, awSpyFile, awPath, MAX_PATH))
        {
        _printf (L"Driver: \"%s\"\r\n",
                 awPath);

        hControl = w2kServiceLoad (awSpyDevice, awSpyDisplay,
                                   awPath, TRUE);
        }
    if (hControl != NULL)
        {
        _printf (L"Opening \"%s\" ...\r\n",
                 awSpyPath);

        hDevice = CreateFile (awSpyPath,
                              GENERIC_READ    | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);
        }
    else
        {
        _printf (L"Unable to load the spy device driver.\r\n");
        }
    if (hDevice != INVALID_HANDLE_VALUE)
        {
        if (SpyVersionInfo (hDevice, &svi))
            {
            _printf (L"\r\n"
                     L"%s V%lu.%02lu ready\r\n",
                     svi.awName,
                     svi.dVersion / 100, svi.dVersion % 100);
            }
        if (SpyHookInfo (hDevice, &shi))
            {
            _printf (L"\r\n"
                     L"API hook parameters:       0x%08lX\r\n"
                     L"SPY_PROTOCOL structure:    0x%08lX\r\n"
                     L"SPY_PROTOCOL data buffer:  0x%08lX\r\n"
                     L"KeServiceDescriptorTable:  0x%08lX\r\n"
                     L"KiServiceTable:            0x%08lX\r\n"
                     L"KiArgumentTable:           0x%08lX\r\n"
                     L"Service table size:        0x%lX (%lu)\r\n",
                     shi.psc,
                     shi.psp,
                     shi.psp->abData,
                     shi.psdt,
                     shi.sdt.ntoskrnl.ServiceTable,
                     shi.sdt.ntoskrnl.ArgumentTable,
                     shi.ServiceLimit, shi.ServiceLimit);
            }
        SpyHookPause  (hDevice, TRUE, &fPause ); fPause  = FALSE;
        SpyHookFilter (hDevice, TRUE, &fFilter); fFilter = FALSE;

        if (SpyHookInstall (hDevice, TRUE, &dCount))
            {
            _printf (L"\r\n"
                     L"Installed %lu API hooks\r\n",
                     dCount);
            }
        _printf (L"\r\n"
                 L"Protocol control keys:\r\n"
                 L"\r\n"
                 L"P    -  pause  ON/off\r\n"
                 L"F    -  filter ON/off\r\n"
                 L"R    -  reset protocol\r\n"
                 L"ESC  -  exit\r\n"
                 L"\r\n");

        for (fRepeat = TRUE; fRepeat;)
            {
            if (n = SpyHookRead (hDevice, TRUE,
                                 abData, HOOK_MAX_DATA))
                {
                if (abData [0] == '-')
                    {
                    n = 0;
                    }
                else
                    {
                    i = 0;
                    while (abData [i] && (abData [i++] != '='));

                    j = i;
                    while (abData [j] && (abData [j] != '(')) j++;

                    k = 0;
                    while (i < j) awData [k++] = abData [i++];

                    awData [k] = 0;

                    for (i = 0; i < dFilters; i++)
                        {
                        if (PatternMatcher (ppwFilters [i], awData))
                            {
                            n = 0;
                            break;
                            }
                        }
                    }
                if (!n) _printf (L"%hs\r\n", abData);
                Sleep (0);
                }
            else
                {
                Sleep (HOOK_IOCTL_DELAY);
                }
            switch (KeyboardData ())
                {
                case 'P':
                    {
                    SpyHookPause (hDevice, fPause, &fPause);
                    SpyHookWrite (hDevice, (fPause ? abPauseOff
                                                   : abPauseOn));
                    break;
                    }
                case 'F':
                    {
                    SpyHookFilter (hDevice, fFilter, &fFilter);
                    SpyHookWrite  (hDevice, (fFilter ? abFilterOff
                                                     : abFilterOn));
                    break;
                    }
                case 'R':
                    {
                    SpyHookReset (hDevice);
                    SpyHookWrite (hDevice, abReset);
                    break;
                    }
                case VK_ESCAPE:
                    {
                    _printf (L"%hs\r\n", abExit);
                    fRepeat = FALSE;
                    break;
                    }
                }
            }
        if (SpyHookRemove (hDevice, FALSE, &dCount))
            {
            _printf (L"\r\n"
                     L"Removed %lu API hooks\r\n",
                     dCount);
            }
        _printf (L"\r\nClosing the spy device ...\r\n");
        CloseHandle (hDevice);
        }
    else
        {
        _printf (L"Unable to open the spy device.\r\n");
        }
    if ((hControl != NULL) && gfSpyUnload)
        {
        _printf (L"Unloading the spy device ...\r\n");
        w2kServiceUnload (awSpyDevice, hControl);
        }
    return;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    _printf (atAbout);

    if (argc < 2)
        {
        _printf (atUsage, awArguments);
        }
    else
        {
        Execute (argv+1, argc-1);
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
