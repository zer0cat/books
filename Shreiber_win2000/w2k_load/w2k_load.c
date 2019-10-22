
// __________________________________________________________
//
//                         w2k_load.c
//            SBS Windows 2000 Driver Loader V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_load.h"

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
// GLOBAL STRINGS
// =================================================================

WORD awUsage  [] =
    L"\r\n"
    L"Usage: " SW(MAIN_MODULE) L" <driver path>\r\n"
    L"       " SW(MAIN_MODULE) L" <driver path> %s\r\n"
    L"       " SW(MAIN_MODULE) L" <driver name> %s\r\n";

WORD awUnload [] = L"/unload";

WORD awOk     [] = L"OK\r\n";
WORD awError  [] = L"ERROR\r\n";

// =================================================================
// COMMAND HANDLERS
// =================================================================

BOOL WINAPI DriverLoad (PWORD pwPath)
    {
    SC_HANDLE hManager;
    BOOL      fOk = FALSE;

    _printf (L"\r\nLoading \"%s\" ... ", pwPath);

    if ((hManager = w2kServiceLoadEx (pwPath, TRUE)) != NULL)
        {
        w2kServiceDisconnect (hManager);
        fOk = TRUE;
        }
    _printf (fOk ? awOk : awError);
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI DriverUnload (PWORD pwPath)
    {
    BOOL fOk = FALSE;

    _printf (L"\r\nUnloading \"%s\" ... ", pwPath);

    fOk = w2kServiceUnloadEx (pwPath, NULL);

    _printf (fOk ? awOk : awError);
    return fOk;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    _printf (atAbout);

    if (argc == 2)
        {
        DriverLoad (argv [1]);
        }
    else
        {
        if ((argc == 3) && (!lstrcmpi (argv [2], awUnload)))
            {
            DriverUnload (argv [1]);
            }
        else
            {
            _printf (awUsage, awUnload, awUnload);
            }
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
