
// __________________________________________________________
//
//                         w2k_svc.c
//            SBS Windows 2000 Service List V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_svc.h"

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
// GLOBAL STRINGS
// =================================================================

WORD awArguments [] =
    L"<type> <state>\r\n\r\n"
    L"       <type>  = /drivers or /processes or /any\r\n"
    L"       <state> = /active  or /inactive  or /all";

WORD awNewLine   [] = L"\r\n";
WORD awNull      [] = L"";

// =================================================================
// DISPLAY ROUTINES
// =================================================================

BOOL WINAPI DisplayServices (BOOL fWin32,
                             BOOL fDriver,
                             BOOL fActive,
                             BOOL fInactive)
    {
    PW2K_SERVICES pws;
    DWORD         i, n;
    BOOL          fOk = FALSE;

    if ((pws = w2kServiceList (fDriver, fWin32, fActive, fInactive))
        != NULL)
        {
        _printf (L"\r\nFound %lu%s%s%s%s:\r\n",
                 pws->dEntries,
                 (fActive && (!fInactive)
                  ? L" active"
                  : (fInactive && (!fActive)
                     ? L" inactive"
                     : awNull)),
                 (fDriver           ? L" drivers"   : awNull),
                 (fWin32 && fDriver ? L" and"       : awNull),
                 (fWin32            ? L" processes" : awNull));

        for (i = 0; i < pws->dEntries; i++)
            {
            _printf (L"%s%5lu. ", (i ? awNull : awNewLine), i+1);
            n = _printf (L"%s", pws->aess [i].lpDisplayName);
            if (n & 1) n += _printf (L" ");
            while (n < pws->dDisplayName + 2) n += _printf (L" .");
            _printf (L" %s\r\n", pws->aess [i].lpServiceName);
            }
        w2kMemoryDestroy (pws);
        }
    return fOk;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PWORD *argv, PWORD *argp)
    {
    DWORD i, n1, n2;
    BOOL  fWin32    = FALSE;
    BOOL  fDriver   = FALSE;
    BOOL  fActive   = FALSE;
    BOOL  fInactive = FALSE;

    _printf (atAbout);

    for (n1 = n2 = 0, i = 1; i < argc; i++)
        {
        if      (!lstrcmpi (argv [i], L"/processes"))
            {
            fWin32    = TRUE;
            n1++;
            }
        else if (!lstrcmpi (argv [i], L"/drivers"))
            {
            fDriver   = TRUE;
            n1++;
            }
        else if (!lstrcmpi (argv [i], L"/any"))
            {
            fWin32    = TRUE;
            fDriver   = TRUE;
            n1++;
            }
        else if (!lstrcmpi (argv [i], L"/active"))
            {
            fActive   = TRUE;
            n2++;
            }
        else if (!lstrcmpi (argv [i], L"/inactive"))
            {
            fInactive = TRUE;
            n2++;
            }
        else if (!lstrcmpi (argv [i], L"/all"))
            {
            fActive   = TRUE;
            fInactive = TRUE;
            n2++;
            }
        }
    if (n1 && n2 && (1 + n1 + n2 == argc))
        {
        DisplayServices (fWin32, fDriver, fActive, fInactive);
        }
    else
        {
        _printf (atUsage, awArguments);
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
