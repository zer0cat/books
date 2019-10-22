
// __________________________________________________________
//
//                         w2k_obj.c
//           SBS Windows 2000 Object Browser V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_obj.h"

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

BOOL gfMemoryStatus = FALSE;

// =================================================================
// GLOBAL STRINGS
// =================================================================

WORD awArguments      [] =
    L"[+-atf] [<type>] [<#>|-1] [/root] [/types]";

WORD awMoreInfo       [] =
    L"\r\n"
    L"       +a -a  : show/hide object addresses  (default: -a)\r\n"
    L"       +t -t  : show/hide object type names (default: -t)\r\n"
    L"       +f -f  : show/hide object flags      (default: -f)\r\n"
    L"       <type> : show <type> objects only    (default:  *)\r\n"
    L"       <#>    : show <#> directory levels   (default: -1)\r\n"
    L"       -1     : show all directory levels\r\n"
    L"       /root  : show ObpRootDirectoryObject tree\r\n"
    L"       /types : show ObpTypeDirectoryObject tree\r\n"
    L"\r\n"
    L"Example: " SW(MAIN_MODULE) L" +atf *port 2 /root\r\n"
    L"\r\n"
    L"This command displays all Port and WaitablePort objects,\r\n"
    L"starting in the root and scanning two directory levels.\r\n"
    L"Each line includes address, type, and flag information.\r\n";

WORD awModuleNotFound [] =
    L"\r\n"
    L"Unable to locate the module ntoskrnl.exe\r\n";

WORD awLoadError      [] =
    L"\r\n"
    L"Unable to load the symbol table\r\n";

WORD awChecksumError  [] =
    L"\r\n"
    L"The symbol files don't match your installed system\r\n";

WORD awUnknownError   [] =
    L"\r\n"
    L"Unknown symbol table initialization error\r\n";

// =================================================================
// OBJECT BROWSER
// =================================================================

DWORD WINAPI DisplayDirectory (POBJECT_DIRECTORY pDirectory,
                               DWORD             dOptions,
                               PWORD             pwType,
                               DWORD             dLevel,
                               DWORD             dMaxLevel,
                               PDIR_LEVEL        pLevels)
    {
    POBJECT_DIRECTORY       pDirectory1;
    POBJECT_DIRECTORY_ENTRY pEntry;
    DWORD                   dSize, dIndex, i, j;
    DWORD                   n = 0;

    if ((dLevel < dMaxLevel) &&
        ((pDirectory1 = w2kDirectoryOpen (pDirectory)) != NULL))
        {
        dSize  = w2kDirectorySize (pDirectory1, pwType);
        dIndex = 0;

        for (i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
            {
            for (pEntry  = pDirectory1->HashTable [i];
                 pEntry != NULL;
                 pEntry  = pEntry->NextEntry)
                {
                pLevels [dLevel].fLastEntry = (dIndex+1 == dSize);

                if (j = DisplayObject (pEntry->Object, dOptions,
                                       pwType, dLevel+1, dMaxLevel,
                                       pLevels, FALSE))
                    {
                    n += j;
                    dIndex++;
                    }
                }
            }
        for (i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
            {
            for (pEntry  = pDirectory1->HashTable [i];
                 pEntry != NULL;
                 pEntry  = pEntry->NextEntry)
                {
                pLevels [dLevel].fLastEntry = (dIndex+1 == dSize);

                if (j = DisplayObject (pEntry->Object, dOptions,
                                       pwType, dLevel+1, dMaxLevel,
                                       pLevels, TRUE))
                    {
                    n += j;
                    dIndex++;
                    }
                }
            }
        w2kDirectoryClose (pDirectory1);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayObject (PW2K_OBJECT pObject,
                            DWORD       dOptions,
                            PWORD       pwType,
                            DWORD       dLevel,
                            DWORD       dMaxLevel,
                            PDIR_LEVEL  pLevels,
                            BOOL        fDirectories)
    {
    BOOL  fDirectory, fMatch;
    DWORD dBytes, i;
    DWORD n = 0;

    dBytes = OBJECT_NAME_INFORMATION_ + (1000 * WORD_);

    if (pObject != NULL)
        {
        fDirectory = !lstrcmp (pObject->pwType, L"Directory");
        fMatch = w2kStringFilter (pwType, pObject->pwType, TRUE);

        if ((fDirectory && fDirectories)  ||
            (!(fDirectory || fDirectories || (!fMatch))))
            {
            _printf (lstrcmp (pwType, L"*") && fMatch
                     ? L">"
                     : L" ");

            for (i = 0; i < dLevel; i++)
                {
                printf (i+1 == dLevel
                        ? (pLevels [i].fLastEntry
                           ? L" \\_ "
                           : L" |_ ")
                        : (pLevels [i].fLastEntry
                           ? L"    "
                           : L" |  "));
                }
            if (dOptions & OPTION_ADDRESS)
                {
                _printf (L"%08lX ",
                         pObject->pObject);
                }
            if (dOptions & OPTION_TYPE)
                {
                _printf (L"%+_-14s ",
                         pObject->pwType);
                }
            if (dOptions & OPTION_FLAGS)
                {
                _printf (L"<%02lX> ",
                         (DWORD) pObject->pHeader->ObjectFlags);
                }
            if (fDirectory)
                {
                pLevels [dLevel].pwName = pObject->pwName;

                for (i = 0; i <= dLevel; i++)
                    {
                    _printf (L"%s%s",
                             pLevels [i].pwName,
                             (i && (i < dLevel) ? L"\\" : L""));
                    }
                printf (L"\r\n");

                n += DisplayDirectory (pObject->pObject, dOptions,
                                       pwType, dLevel, dMaxLevel,
                                       pLevels);
                }
            else
                {
                _printf (L"%s\r\n", pObject->pwName);
                }
            n++;
            }
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayObjects (DWORD dOptions,
                             PWORD pwType,
                             DWORD dMaxLevel,
                             BOOL  fTypeDirectory)
    {
    PW2K_OBJECT pObject;
    WORD        awMaxLevel [N_DECIMAL32];
    DIR_LEVEL   aLevels    [256];
    DWORD       n = 0;

    if (dMaxLevel != MAXDWORD)
        {
        _sprintf (awMaxLevel, L"%lu", dMaxLevel);
        }
    else
        {
        lstrcpyn (awMaxLevel, L"all", N_DECIMAL32);
        }
    _printf (L"\r\n"
             L"%s directory contents: (%s level%s shown)\r\n"
             L"------------------------\r\n"
             L"\r\n",
             (fTypeDirectory ? L"Type" : L"Root"),
             awMaxLevel, (dMaxLevel == 1 ? L"" : L"s"));

    if ((pObject = w2kObjectOpen (fTypeDirectory
                                  ? __ObpTypeDirectoryObject ()
                                  : __ObpRootDirectoryObject ()))
        != NULL)
        {
        n += DisplayObject (pObject, dOptions,
                            pwType, 0, dMaxLevel,
                            aLevels, TRUE);

        w2kObjectClose (pObject);
        }
    _printf (L"\r\n%lu object%s\r\n",
             n, (n == 1 ? L"" : L"s"));
    return n;
    }

// =================================================================
// POOR MAN'S OBJECT BROWSER
// =================================================================

VOID WINAPI _DisplayObject (PW2K_OBJECT pObject,
                            DWORD       dLevel)
    {
    POBJECT_DIRECTORY       pDir;
    POBJECT_DIRECTORY_ENTRY pEntry;
    DWORD                   i;

    for (i = 0; i < dLevel; i++) printf (L"   ");

    _printf (L"%+.-16s%s\r\n", pObject->pwType, pObject->pwName);

    if ((!lstrcmp (pObject->pwType, L"Directory")) &&
        ((pDir = w2kDirectoryOpen (pObject->pObject)) != NULL))
        {
        for (i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
            {
            for (pEntry  = pDir->HashTable [i];
                 pEntry != NULL;
                 pEntry  = pEntry->NextEntry)
                {
                _DisplayObject (pEntry->Object, dLevel+1);
                }
            }
        w2kDirectoryClose (pDir);
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI _DisplayObjects (VOID)
    {
    PW2K_OBJECT pObject;

    if ((pObject = w2kObjectOpen (__ObpRootDirectoryObject ()))
        != NULL)
        {
        _DisplayObject (pObject, 0);
        w2kObjectClose (pObject);
        }
    return;
    }

// =================================================================
// OPTION MANAGEMENT
// =================================================================

BOOL WINAPI OptionNumeric (PWORD  pwData,
                           PDWORD pdData)
    {
    BOOL  fMinus;
    DWORD i, j, n;
    DWORD dData = (pdData != NULL ? *pdData : 0);
    BOOL  fOk   = FALSE;

    i = n = 0;

    switch (pwData [i])
        {
        case '-': fMinus = TRUE;  i++; break;
        case '+': fMinus = FALSE; i++; break;
        default:  fMinus = FALSE;      break;
        }
    while ((pwData [i] >= '0') && (pwData [i] <= '9'))
        {
        if ((j = (n * 10)) / 10 != n) break;
        n = j;

        if ((j = n + (pwData [i] - '0')) < n) break;
        n = j;

        i++;
        }
    if (i && (!pwData [i]))
        {
        dData = (fMinus ? 0 - n : n);
        fOk   = TRUE;
        }
    if (pdData != NULL) *pdData = dData;
    return fOk;
    }

// -----------------------------------------------------------------

VOID WINAPI OptionDispatcher (PPWORD ppwItems,
                              DWORD  dCount)
    {
    BOOL  fPlus;
    DWORD dOption, i, j;
    DWORD dOptions = OPTION_DEFAULT;
    PWORD pwType   = L"*";
    DWORD dLevel   = MAXDWORD;

    for (i = 0; i < dCount; i++)
        {
        if (!lstrcmpi (ppwItems [i], L"/root"))
            {
            DisplayObjects (dOptions, pwType, dLevel, FALSE);
            }
        else if (!lstrcmpi (ppwItems [i], L"/types"))
            {
            DisplayObjects (dOptions, pwType, dLevel, TRUE);
            }
        else if (!OptionNumeric (ppwItems [i], &dLevel))
            {
            switch (ppwItems [i] [0])
                {
                default:
                    {
                    pwType = ppwItems [i];
                    break;
                    }
                case '+':
                case '-':
                    {
                    for (j = 0; ppwItems [i] [j]; j++)
                        {
                        dOption = 0;

                        switch (ppwItems [i] [j])
                            {
                            case '+':
                                {
                                fPlus = TRUE;
                                break;
                                }
                            case '-':
                                {
                                fPlus = FALSE;
                                break;
                                }
                            case 'a':
                            case 'A':
                                {
                                dOption = OPTION_ADDRESS;
                                break;
                                }
                            case 't':
                            case 'T':
                                {
                                dOption = OPTION_TYPE;
                                break;
                                }
                            case 'f':
                            case 'F':
                                {
                                dOption = OPTION_FLAGS;
                                break;
                                }
                            }
                        if (dOption)
                            {
                            if (fPlus) dOptions |=  dOption;
                            else       dOptions &= ~dOption;
                            }
                        }
                    }
                }
            }
        }
    return;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    PWORD pwStatus;
    DWORD dStatus, dMemoryNow, dMemoryMax;
    BOOL  fMemorySign;

    _printf (atAbout);

    if (argc < 2)
        {
        _printf (atUsage, awArguments);
        _printf (awMoreInfo);
        }
    else
        {
        if ((dStatus = w2kSymbolsStatus ()) == W2K_SYMBOLS_OK)
            {
            OptionDispatcher (argv + 1, argc - 1);
            }
        else
            {
            switch (dStatus)
                {
                case W2K_SYMBOLS_MODULE_ERROR:
                    {
                    pwStatus = awModuleNotFound;
                    break;
                    }
                case W2K_SYMBOLS_LOAD_ERROR:
                    {
                    pwStatus = awLoadError;
                    break;
                    }
                case W2K_SYMBOLS_VERSION_ERROR:
                    {
                    pwStatus = awChecksumError;
                    break;
                    }
                default:
                    {
                    pwStatus = awUnknownError;
                    break;
                    }
                }
            _printf (L"%s", pwStatus);
            }
        }
    if (gfMemoryStatus)
        {
        fMemorySign = w2kMemoryStatus (&dMemoryNow, &dMemoryMax);

        _printf (L"\r\nCurrent memory usage: %c%lu"
                 L"\r\nMaximum memory usage:  %lu\r\n",
                 (fMemorySign ? '-' : ' '), dMemoryNow, dMemoryMax);
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
