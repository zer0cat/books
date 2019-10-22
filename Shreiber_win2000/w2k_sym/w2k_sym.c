
// __________________________________________________________
//
//                         w2k_sym.c
//           SBS Windows 2000 Symbol Browser V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_sym.h"

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

WORD gawArguments [] =
    L"{ <mode> [ /f | /F <filter> ] <operation> }";

WORD gawOptions   [] =
    L"\r\n"
    L"<mode> is a series of options for the next <operation>:\r\n"
    L"\r\n"
    L"        /a : sort by address\r\n"
    L"        /s : sort by size\r\n"
    L"        /i : sort by ID (process/module lists only)\r\n"
    L"        /n : sort by name\r\n"
    L"        /c : sort by name (case-sensitive)\r\n"
    L"        /r : reverse order\r\n"
    L"        /l : load  checkpoint file (see below)\r\n"
    L"        /w : write checkpoint file (see below)\r\n"
    L"        /e : display end address instead of size\r\n"
    L"        /v : verbose mode\r\n"
    L"\r\n"
    L"/f <filter> applies a case-insensitive search pattern.\r\n"
    L"/F <filter> works analogous, but case-sensitive.\r\n"
    L"In <filter>, the wildcards * and ? are allowed.\r\n"
    L"\r\n"
    L"<operation> is one of the following:\r\n"
    L"\r\n"
    L"        /p : display processes      - checkpoint: %s\r\n"
    L"        /m : display modules        - checkpoint: %s\r\n"
    L"        /d : display drivers        - checkpoint: %s\r\n"
    L"    <file> : display <file> symbols - checkpoint: %s\r\n"
    L"\r\n"
    L"<file> is a file name, a relative path, or a fully qualified "
    L"path.\r\n"
    L"Checkpoint files are loaded from and written to the current "
    L"directory.\r\n"
    L"A checkpoint is an on-disk image of a DBG_LIST structure "
    L"(see w2k_dbg.h).\r\n";

// -----------------------------------------------------------------

WORD gawTitleProcesses [] =
    L"\r\n"
    L"PROCESSES\r\n"
    L"=========\r\n";

WORD gawTableProcesses [] =
    L"\r\n"
    L"    #  ADDRESS  %s   PID   MOD NAME\r\n"
    L"----------------------------------------"
    L"---------------------------------------\r\n";

WORD gawTotalProcesses [] =
    L"----------------------------------------"
    L"---------------------------------------\r\n"
    L"    TOTAL SIZE: %8lX (%lu bytes / %lu KB / %lu MB)\r\n";

WORD gawFileProcesses  [] = L"processes.dbgl";

// -----------------------------------------------------------------

WORD gawTitleModules [] =
    L"\r\n"
    L"MODULES\r\n"
    L"=======\r\n";

WORD gawTableModules [] =
    L"\r\n"
    L"    #  ADDRESS  %s NAME\r\n"
    L"----------------------------------------"
    L"---------------------------------------\r\n";

WORD gawTotalModules [] =
    L"----------------------------------------"
    L"---------------------------------------\r\n"
    L"    TOTAL SIZE: %8lX (%lu bytes / %lu KB / %lu MB)\r\n";

WORD gawTotalSystem  [] =
    L"\r\n"
    L"  OVERALL SIZE: %8lX (%lu bytes / %lu KB / %lu MB)\r\n";

WORD gawFileModules  [] = L"modules.dbgl";

// -----------------------------------------------------------------

WORD gawTitleDrivers [] =
    L"\r\n"
    L"DRIVERS\r\n"
    L"=======\r\n";

WORD gawTableDrivers [] =
    L"\r\n"
    L"    #  ADDRESS  %s NAME\r\n"
    L"----------------------------------------"
    L"---------------------------------------\r\n";

WORD gawTotalDrivers [] =
    L"----------------------------------------"
    L"---------------------------------------\r\n"
    L"    TOTAL SIZE: %8lX (%lu bytes / %lu KB / %lu MB)\r\n";

WORD gawFileDrivers  [] = L"drivers.dbgl";

// -----------------------------------------------------------------

WORD gawTitleSymbols [] =
    L"\r\n"
    L"SYMBOLS\r\n"
    L"=======\r\n";

WORD gawTableSymbols [] =
    L"\r\n"
    L"    #  ADDRESS  %s NAME\r\n"
    L"----------------------------------------"
    L"---------------------------------------\r\n";

WORD gawTotalSymbols [] =
    L"----------------------------------------"
    L"---------------------------------------\r\n"
    L"    TOTAL SIZE: %8lX (%lu bytes / %lu KB / %lu MB)\r\n";

WORD gawFileSymbols  [] = L"symbols.dbgl";

// -----------------------------------------------------------------

WORD gawSize [] = L"    SIZE";
WORD gawEnd  [] = L"END     ";
WORD gawNull [] = L"";

// =================================================================
// INFO DISPLAY ROUTINES
// =================================================================

DWORD WINAPI DisplayTime (PSYSTEMTIME pst)
    {
    SYSTEMTIME st;
    DWORD n = 0;

    if (SystemTimeToTzSpecificLocalTime (NULL, pst, &st))
        {
        n = printf (L"%s, %02hd-%02hd-%04hd, %02hd:%02hd:%02hd\r\n",
                    dbgStringDay (st.wDayOfWeek),
                    st.wMonth, st.wDay,    st.wYear,
                    st.wHour,  st.wMinute, st.wSecond);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayCount (PWORD pwSingular,
                           PWORD pwPlural,
                           DWORD dCount,
                           PWORD pwInfo)
    {
    return printf (L"\r\n%lu %s%s\r\n", dCount,
                   (dCount == 1    ? pwSingular : pwPlural),
                   (pwInfo != NULL ? pwInfo     : gawNull ));
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayFilter (PWORD pwFilter)
    {
    return (pwFilter != NULL
            ? printf (L"Filter: <%s>\r\n", pwFilter)
            : 0);
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayTotal (PWORD pwFormat,
                           DWORD dTotal)
    {
    return printf (pwFormat, dTotal, dTotal,
                   dbgSizeKB (dTotal, FALSE, FALSE),
                   dbgSizeMB (dTotal, FALSE, FALSE));
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayMatches (DWORD dMatches)
    {
    return (dMatches

            ? DisplayCount (L"matching entry", L"matching entries",
                            dMatches, NULL)

            : printf (L"No matching entries\r\n"));
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayError (PWORD pwType)
    {
    return printf (L"\r\nUnable to obtain the %s list\r\n", pwType);
    }

// =================================================================
// LIST DISPLAY ROUTINES
// =================================================================

void WINAPI DisplayProcesses (PWORD pwFilter,
                              DWORD dSort,
                              BOOL  fReverse,
                              BOOL  fLoad,
                              BOOL  fWrite,
                              BOOL  fEnd,
                              BOOL  fVerbose,
                              BOOL  fCase)
    {
    PDBG_INDEX   pdi;
    PDBG_PROCESS pdp;
    PWORD        pwPath;
    DWORD        dEnd, dTotal, i, n;

    printf (gawTitleProcesses);

    if ((pdi = dbgProcessIndex ((fLoad ? gawFileProcesses : NULL),
                                dSort, fReverse))
        != NULL)
        {
        DisplayCount (L"process", L"processes", pdi->dEntries,
                      (!fVerbose
                       ? L" - use the /v switch to view full paths"
                       : NULL));

        DisplayTime   (&pdi->pdl->st);
        DisplayFilter (pwFilter);

        for (i = n = dTotal = 0; i < pdi->dEntries; i++)
            {
            pdp = pdi->ppdp [i];

            pwPath = (pdp->awPath [0] 
                      ? pdp->awPath + (fVerbose ? 0 : pdp->dFile)
                      : dbgProcessGuess (i));

            if (dbgStringMatch (pwFilter, pwPath, fCase))
                {
                if (fEnd)
                    {
                    if (!n++) printf (gawTableProcesses, gawEnd);

                    dEnd = (DWORD_PTR) pdp->pBase + pdp->dSize;

                    printf (L"%5lu: %08lX %08lX %5lu %5lu %s\r\n",
                            i+1, pdp->pBase, dEnd,
                            pdp->dId, pdp->dModules, pwPath);
                    }
                else
                    {
                    if (!n++) printf (gawTableProcesses, gawSize);

                    printf (L"%5lu: %08lX %8lX %5lu %5lu %s\r\n",
                            i+1, pdp->pBase, pdp->dSize,
                            pdp->dId, pdp->dModules, pwPath);
                    }
                dTotal += pdp->dSize;
                }
            }
        if (n) DisplayTotal (gawTotalProcesses, dTotal);
        if (pwFilter != NULL) DisplayMatches (n);

        if (fWrite) dbgIndexSave (gawFileProcesses, pdi);
        dbgIndexDestroy (pdi);
        }
    else
        {
        DisplayError (L"process");
        }
    return;
    }

// -----------------------------------------------------------------

void WINAPI DisplayModules (PWORD pwFilter,
                            DWORD dSort,
                            BOOL  fReverse,
                            BOOL  fLoad,
                            BOOL  fWrite,
                            BOOL  fEnd,
                            BOOL  fVerbose,
                            BOOL  fCase)
    {
    PDBG_INDEX   pdi, pdi1;
    PDBG_PROCESS pdp;
    PDBG_MODULE  pdm;
    PWORD        pwPath1, pwPath2;
    DWORD        dEnd, dTotal1, dTotal2, i, j, n;

    printf (gawTitleModules);

    if ((pdi = dbgProcessIndexEx ((fLoad ? gawFileModules : NULL),
                                  dSort, fReverse))
        != NULL)
        {
        DisplayCount (L"process", L"processes", pdi->dEntries,
                      (!fVerbose
                       ? L" - use the /v switch to view full paths"
                       : NULL));

        DisplayTime   (&pdi->pdl->st);
        DisplayFilter (pwFilter);

        for (i = dTotal1 = 0; i < pdi->dEntries; i++)
            {
            pdi1 = pdi->ppdi [i];
            pdp  = pdi1->pContext;

            pwPath1 = (pdp->awPath [0]
                       ? pdp->awPath + (fVerbose ? 0 : pdp->dFile)
                       : dbgProcessGuess (i));

            for (j = n = dTotal2 = 0; j < pdi1->dEntries; j++)
                {
                pdm     = pdi1->ppdm [j];
                pwPath2 = pdm->awPath + (fVerbose ? 0 : pdm->dFile);

                if (dbgStringMatch (pwFilter, pwPath2, fCase))
                    {
                    if (!n)
                        {
                        printf (L"\r\nPROCESS #%lu (%lu/%lu): "
                                L"%s (%lu MODULES)\r\n",
                                pdp->dId, i+1, pdi->dEntries,
                                pwPath1, pdp->dModules);
                        }
                    if (fEnd)
                        {
                        if (!n++) printf (gawTableModules, gawEnd);

                        dEnd = (DWORD_PTR) pdm->pBase + pdm->dSize;

                        printf (L"%5lu: %08lX %08lX %s\r\n",
                                j+1, pdm->pBase, dEnd,
                                pwPath2);
                        }
                    else
                        {
                        if (!n++) printf (gawTableModules, gawSize);

                        printf (L"%5lu: %08lX %8lX %s\r\n",
                                j+1, pdm->pBase, pdm->dSize,
                                pwPath2);
                        }
                    dTotal2 += pdm->dSize;
                    }
                }
            dTotal1 += dTotal2;

            if (n) DisplayTotal (gawTotalModules, dTotal2);
            if ((pwFilter != NULL) && n) DisplayMatches (n);
            }
        if (dTotal1) DisplayTotal (gawTotalSystem, dTotal1);

        if (fWrite) dbgIndexSaveEx (gawFileModules, pdi);
        dbgIndexDestroyEx (pdi);
        }
    else
        {
        DisplayError (L"module");
        }
    return;
    }

// -----------------------------------------------------------------

void WINAPI DisplayDrivers (PWORD pwFilter,
                            DWORD dSort,
                            BOOL  fReverse,
                            BOOL  fLoad,
                            BOOL  fWrite,
                            BOOL  fEnd,
                            BOOL  fVerbose,
                            BOOL  fCase)
    {
    PDBG_INDEX  pdi;
    PDBG_DRIVER pdd;
    PWORD       pwPath;
    DWORD       dEnd, dTotal, i, n;

    printf (gawTitleDrivers);

    if ((pdi = dbgDriverIndex ((fLoad ? gawFileDrivers : NULL),
                               dSort, fReverse))
        != NULL)
        {
        DisplayCount (L"driver", L"drivers", pdi->dEntries,
                      (!fVerbose
                       ? L" - use the /v switch to view full paths"
                       : NULL));

        DisplayTime   (&pdi->pdl->st);
        DisplayFilter (pwFilter);

        for (i = n = dTotal = 0; i < pdi->dEntries; i++)
            {
            pdd    = pdi->ppdd [i];
            pwPath = pdd->awPath + (fVerbose ? 0 : pdd->dFile);

            if (dbgStringMatch (pwFilter, pwPath, fCase))
                {
                if (fEnd)
                    {
                    if (!n++) printf (gawTableDrivers, gawEnd);

                    dEnd = (DWORD_PTR) pdd->pBase + pdd->dSize;

                    printf (L"%5lu: %08lX %08lX %s\r\n",
                            i+1, pdd->pBase, dEnd,
                            pwPath);
                    }
                else
                    {
                    if (!n++) printf (gawTableDrivers, gawSize);

                    printf (L"%5lu: %08lX %8lX %s\r\n",
                            i+1, pdd->pBase, pdd->dSize,
                            pwPath);
                    }
                dTotal += pdd->dSize;
                }
            }
        if (n) DisplayTotal (gawTotalDrivers, dTotal);
        if (pwFilter != NULL) DisplayMatches (n);

        if (fWrite) dbgIndexSave (gawFileDrivers, pdi);
        dbgIndexDestroy (pdi);
        }
    else
        {
        DisplayError (L"driver");
        }
    return;
    }

// -----------------------------------------------------------------

void WINAPI DisplaySymbols (PWORD pwPath,
                            PWORD pwFilter,
                            DWORD dSort,
                            BOOL  fReverse,
                            BOOL  fLoad,
                            BOOL  fWrite,
                            BOOL  fEnd,
                            BOOL  fVerbose,
                            BOOL  fCase)
    {
    PDBG_INDEX  pdi;
    PDBG_SYMBOL pds;
    PVOID       pBase;
    DWORD       dSize, dEnd, dTotal, i, n;

    printf (gawTitleSymbols);

    if (((pBase = dbgBaseDriver (pwPath, &dSize)) != NULL) ||
        ((pBase = dbgBaseModule (pwPath, &dSize)) != NULL))
        {
        if (dSize)
            {
            printf (L"\r\nModule %s at %08lX (%lX bytes)\r\n",
                    pwPath, pBase, dSize);
            }
        else
            {
            printf (L"\r\nModule %s at %08lX (unknown size)\r\n",
                    pwPath, pBase);
            }
        }
    if ((pdi = dbgSymbolIndex (pwPath, pBase,
                               (fLoad ? gawFileSymbols : NULL),
                               dSort, fReverse))
        != NULL)
        {
        DisplayCount (L"symbol", L"symbols", pdi->dEntries,
                      (!fVerbose
                       ? L" - use the /v switch to view all symbols"
                       : NULL));

        DisplayTime (&pdi->pdl->st);

        if (fVerbose)
            {
            DisplayFilter (pwFilter);

            for (i = n = dTotal = 0; i < pdi->dEntries; i++)
                {
                pds = pdi->ppds [i];

                if (dbgStringMatch (pwFilter, pds->awName, fCase))
                    {
                    if (fEnd)
                        {
                        if (!n++) printf (gawTableSymbols, gawEnd);

                        dEnd = (DWORD_PTR) pds->pBase + pds->dSize;

                        printf (L"%5lu: %08lX %08lX %s\r\n",
                                i+1, pds->pBase, dEnd,
                                pds->awName);
                        }
                    else
                        {
                        if (!n++) printf (gawTableSymbols, gawSize);

                        printf (L"%5lu: %08lX %8lX %s\r\n",
                                i+1, pds->pBase, pds->dSize,
                                pds->awName);
                        }
                    if (pds->pBase &&
                        ((DWORD_PTR) pds->pBase + pds->dSize))
                        {
                        dTotal += pds->dSize;
                        }
                    }
                }
            if (n) DisplayTotal (gawTotalSymbols, dTotal);
            if (pwFilter != NULL) DisplayMatches (n);
            }
        if (fWrite) dbgIndexSave (gawFileSymbols, pdi);
        dbgIndexDestroy (pdi);
        }
    else
        {
        DisplayError (L"symbol");
        }
    return;
    }

// =================================================================
// COMMAND DISPATCHER
// =================================================================

void WINAPI Dispatch (PWORD *ppwArguments,
                      DWORD  dArguments)
    {
    DWORD i, j;
    WORD  wOption;
    BOOL  fReset;
    BOOL  fReverse = FALSE;
    BOOL  fLoad    = FALSE;
    BOOL  fWrite   = FALSE;
    BOOL  fEnd     = FALSE;
    BOOL  fVerbose = FALSE;
    BOOL  fCase    = FALSE;
    BOOL  fFilter  = FALSE;
    PWORD pwFilter = NULL;
    DWORD dSort    = DBG_UNSORTED;

    printf (L"\r\nRequesting the debugging privilege ... %s\r\n",
            (dbgPrivilegeDebug () ? L"OK" : L"ERROR"));

    for (i = 0; i < dArguments; i++)
        {
        for (j = 0; ppwArguments [i] [j];)
            {
            if (fFilter)
                {
                fFilter  = FALSE;
                pwFilter = ppwArguments [i] + j;

                j += lstrlen (ppwArguments [i] + j);
                }
            else
                {
                fReset = FALSE;

                if (ppwArguments [i] [j] == '/')
                    {
                    wOption = ppwArguments [i] [++j];

                    switch (LCASE (wOption))
                        {
                        case 'a':
                            {
                            dSort = DBG_SORT_BY_ADDRESS;
                            break;
                            }
                        case 's':
                            {
                            dSort = DBG_SORT_BY_SIZE;
                            break;
                            }
                        case 'i':
                            {
                            dSort = DBG_SORT_BY_ID;
                            break;
                            }
                        case 'n':
                            {
                            dSort = DBG_SORT_BY_NAME;
                            break;
                            }
                        case 'c':
                            {
                            dSort = DBG_SORT_BY_NAME_CS;
                            break;
                            }
                        case 'r':
                            {
                            fReverse = TRUE;
                            break;
                            }
                        case 'l':
                            {
                            fLoad    = TRUE;
                            break;
                            }
                        case 'w':
                            {
                            fWrite   = TRUE;
                            break;
                            }
                        case 'e':
                            {
                            fEnd     = TRUE;
                            break;
                            }
                        case 'v':
                            {
                            fVerbose = TRUE;
                            break;
                            }
                        case 'f':
                            {
                            fCase    = (wOption == 'F');
                            fFilter  = TRUE;
                            break;
                            }
                        case 'p':
                            {
                            DisplayProcesses (pwFilter, dSort,
                                              fReverse, fLoad,
                                              fWrite,   fEnd,
                                              fVerbose, fCase);
                            fReset = TRUE;
                            break;
                            }
                        case 'm':
                            {
                            DisplayModules (pwFilter, dSort,
                                            fReverse, fLoad,
                                            fWrite,   fEnd,
                                            fVerbose, fCase);
                            fReset = TRUE;
                            break;
                            }
                        case 'd':
                            {
                            DisplayDrivers (pwFilter, dSort,
                                            fReverse, fLoad,
                                            fWrite,   fEnd,
                                            fVerbose, fCase);
                            fReset = TRUE;
                            break;
                            }
                        }
                    if (wOption) j++;
                    }
                else
                    {
                    DisplaySymbols (ppwArguments [i] + j,
                                    pwFilter, dSort, 
                                    fReverse, fLoad,
                                    fWrite,   fEnd,
                                    fVerbose, fCase);

                    j += lstrlen (ppwArguments [i] + j);

                    fReset = TRUE;
                    }
                if (fReset)
                    {
                    fReverse = FALSE;
                    fLoad    = FALSE;
                    fWrite   = FALSE;
                    fEnd     = FALSE;
                    fVerbose = FALSE;
                    fCase    = FALSE;
                    fFilter  = FALSE;
                    pwFilter = NULL;
                    dSort    = DBG_UNSORTED;
                    }
                }
            }
        }
    if (fLoad)
        {
        DisplaySymbols (NULL,
                        pwFilter, dSort, 
                        fReverse, fLoad,
                        fWrite,   fEnd,
                        fVerbose, fCase);
        }
    return;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    DWORD dMemoryNow, dMemoryMax;
    BOOL  fMemorySign;

    printf (atAbout);

    if (argc < 2)
        {
        printf (atUsage, gawArguments);

        printf (gawOptions, gawFileProcesses, gawFileModules,
                            gawFileDrivers,   gawFileSymbols);
        }
    else
        {
        Dispatch (argv+1, argc-1);

        fMemorySign = dbgMemoryStatus (&dMemoryNow, &dMemoryMax);
        printf (L"\r\nMemory usage: %lu bytes\r\n", dMemoryMax);

#ifdef _MORE_INFO_

        printf (L"Memory leak: %s%lu bytes\r\n",
                (fMemorySign ? L"-" : L" "), dMemoryNow);

#endif // #ifdef _MORE_INFO_

        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
