
// __________________________________________________________
//
//                         w2k_sym2.c
//           SBS Windows 2000 Symbol Browser V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_sym2.h"

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

TBYTE atArguments [] =
 T("{ [+-anNiprdxusz] [:<sections>] [/<symbols>] <module> }");

TBYTE atMoreInfo  [] =
 T("\r\n")
 T("       +   enable subsequent options\r\n")
 T("       -   disable subsequent options\r\n")
 T("       a   sort by address\r\n")
 T("       n   sort by name\r\n")
 T("       N   sort by name (case sensitive)\r\n")
 T("       i   ignore case in filter strings\r\n")
 T("       p   force preferred load address\r\n")
 T("       r   display relative addresses\r\n")
 T("       d   display decorated symbols\r\n")
 T("       x   display exported symbols only\r\n")
 T("       u   include symbols with unknown calling convention\r\n")
 T("       s   include special symbols\r\n")
 T("       z   include zero-address symbols\r\n")
 T("\r\n")
 T("<sections> and <symbols> are filter expressions,\r\n")
 T("optionally containing the wildcards * and ?.\r\n");

TBYTE atMatchAll  [] = T("*");

// -----------------------------------------------------------------

TBYTE atError     [] =
 T("\r\nNo symbols available for %s\r\n");

TBYTE atFile      [] =
 T("\r\n")
 T("Module name:    %s\r\n")
 T("Time stamp:     %s, %02lu-%02lu-%04lu, %02lu:%02lu:%02lu\r\n")
 T("Check sum:      0x%08lX\r\n")
 T("Base address:   0x%08lX%s\r\n")
 T("Symbol file:    %s\r\n")
 T("Symbol table:   %lu bytes\r\n")
 T("Symbol filter:  %s\r\n")
 T("Section%s       %s\r\n");

TBYTE atNotLoaded [] =
 T(" (not resident)");

TBYTE atStart     [] =
 T("\r\n")
 T("    # INDEX ADDRESS  SECTION     ARGUMENTS   X NAME \r\n")
 T("----------------------------------------")
 T("---------------------------------------\r\n");

TBYTE atLine      [] =
 T("%5lu %5lu %08lX %2ld %-8hs %s %s %c %hs\r\n");

TBYTE atStop      [] =
 T("----------------------------------------")
 T("---------------------------------------\r\n")
 T("%5lu non-NULL symbol%s\r\n")
 T("%5lu exported symbol%s\r\n");

// =================================================================
// DISPLAY ROUTINES
// =================================================================

VOID WINAPI DisplaySymbols (PTBYTE ptModule,
                            PTBYTE ptSections,
                            PTBYTE ptSymbols,
                            DWORD  dOptions)
    {
    IMG_TIME   it;
    PIMG_TABLE pit;
    PIMG_ENTRY pie;
    PVOID      pBase;
    PVOID      pAddress;
    PBYTE      pbSymbol;
    TBYTE      atStack [8+1];
    PTBYTE     ptConvention;
    DWORD      dNotNull, dExported, i, j;

    pBase = (dOptions & DISPLAY_OPTION_PREFERRED
             ? NULL
             : imgModuleBase (ptModule));

    if ((pit = imgTableLoad (ptModule, pBase)) != NULL)
        {
        it = imgTimeUnpack (pit->dTimeStamp);

        for (i = 0;
             ptSections [i] && (ptSections [i] != '*')
                            && (ptSections [i] != '?');
             i++);

        printf (atFile,
                ptModule + imgPathName (ptModule, NULL),
                imgTimeDay (it), it.bMonth, it.bDay,    it.wYear,
                                 it.bHour,  it.bMinute, it.bSecond,
                pit->dCheckSum, pit->pBase,
                (pBase != NULL ? T("") : atNotLoaded),
                pit->atPath, pit->dSize, ptSymbols,
                (ptSections [i] ? T("s:") : T(": ")), ptSections);

        dNotNull = dExported = 0;

        for (i = j = 0; i < pit->dSymbols; i++)
            {
            switch (dOptions & DISPLAY_OPTION_SORT)
                {
                case DISPLAY_OPTION_ADDRESS:
                    {
                    pie = pit->piiAddress->apEntries [i];
                    break;
                    }
                case DISPLAY_OPTION_NAME_IC:
                    {
                    pie = pit->piiNameIC->apEntries [i];
                    break;
                    }
                case DISPLAY_OPTION_NAME:
                    {
                    pie = pit->piiName->apEntries [i];
                    break;
                    }
                default:
                    {
                    pie = pit->aEntries + i;
                    break;
                    }
                }
            pAddress = ((pie->pAddress != NULL) &&
                        (dOptions & DISPLAY_OPTION_RELATIVE)
                        ? (PBYTE)     pie->pAddress -
                          (DWORD_PTR) pit->pBase
                        : pie->pAddress);

            pbSymbol = (dOptions & DISPLAY_OPTION_DECORATED
                        ? pie->abDecorated
                        : pie->abSymbol);

            if (((!(dOptions & DISPLAY_OPTION_EXPORTED)) ||
                 pie->fExported)
                &&
                ((dOptions & DISPLAY_OPTION_UNDEFINED) ||
                 (pie->dConvention != IMG_CONVENTION_UNDEFINED) ||
                 pie->fSpecial)
                &&
                ((dOptions & DISPLAY_OPTION_SPECIAL) ||
                 (!pie->fSpecial))
                &&
                ((dOptions & DISPLAY_OPTION_ZERO) ||
                 (pie->pAddress != NULL))
                &&
                imgAnsiMatch (ptSections, pie->abSection,
                              dOptions & DISPLAY_OPTION_IGNORECASE)
                &&
                imgAnsiMatch (ptSymbols, pbSymbol,
                              dOptions & DISPLAY_OPTION_IGNORECASE))
                {
                if (!j++) printf (atStart);

                if (pie->dStack != -1)
                    {
                    sprintf (atStack, T("%2lX"), pie->dStack);
                    }
                else
                    {
                    sprintf (atStack, T("  "));
                    }
                switch (pie->dConvention)
                    {
                    case IMG_CONVENTION_UNDEFINED:
                        {
                        ptConvention = T("        ");
                        break;
                        }
                    case IMG_CONVENTION_STDCALL:
                        {
                        ptConvention = T("STDCALL ");
                        break;
                        }
                    case IMG_CONVENTION_CDECL:
                        {
                        ptConvention = T("CDECL   ");
                        break;
                        }
                    case IMG_CONVENTION_FASTCALL:
                        {
                        ptConvention = T("FASTCALL");
                        break;
                        }
                    default:
                        {
                        ptConvention = T("???     ");
                        break;
                        }
                    }
                printf (atLine,
                        j, i, pAddress,
                        pie->dSection, pie->abSection,
                        atStack, ptConvention,
                        (pie->fExported ? '*' : ' '), pbSymbol);

                if (pie->pAddress != NULL) dNotNull++;
                if (pie->fExported       ) dExported++;
                }
            }
        if (pit->dSymbols)
            {
            printf (atStop,
                    dNotNull,  (dNotNull  == 1 ? T("") : T("s")),
                    dExported, (dExported == 1 ? T("") : T("s")));
            }
        imgMemoryDestroy (pit);
        }
    else
        {
        printf (atError, ptModule);
        }
    return;
    }

// =================================================================
// OPTION MANAGEMENT
// =================================================================

VOID WINAPI Option (BOOL   fPlus,
                    PDWORD pdOptions,
                    DWORD  dMask,
                    DWORD  dValue,
                    DWORD  dDefault)
    {
    DWORD dNewValue = *pdOptions & dMask;

    *pdOptions &= ~dMask;

    if (fPlus)
        {
        dNewValue = dValue & dMask;
        }
    else
        {
        if (dNewValue == (dValue & dMask))
            {
            dNewValue = dDefault & dMask;
            }
        }
    *pdOptions |= dNewValue;
    return;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    PTBYTE ptSections = atMatchAll;
    PTBYTE ptSymbols  = atMatchAll;
    DWORD  dOptions, i, j;
    BOOL   fPlus;

    printf (atAbout);

    if (argc < 2)
        {
        printf (atUsage, atArguments);
        printf (atMoreInfo);
        }
    else
        {
        dOptions = DISPLAY_OPTION_DEFAULT;

        for (i = 1; i < argc; i++)
            {
            switch (argv [i] [0])
                {
                default:
                    {
                    DisplaySymbols (argv [i], ptSections, ptSymbols,
                                    dOptions);
                    break;
                    }
                case ':':
                    {
                    ptSections = argv [i] + 1;
                    break;
                    }
                case '/':
                    {
                    ptSymbols  = argv [i] + 1;
                    break;
                    }
                case '+':
                case '-':
                    {
                    for (j = 0; argv [i] [j]; j++)
                        {
                        switch (argv [i] [j])
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
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_SORT,
                                        DISPLAY_OPTION_ADDRESS,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'n':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_SORT,
                                        DISPLAY_OPTION_NAME_IC,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'N':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_SORT,
                                        DISPLAY_OPTION_NAME,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'i':
                            case 'I':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_IGNORECASE,
                                        DISPLAY_OPTION_IGNORECASE,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'p':
                            case 'P':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_PREFERRED,
                                        DISPLAY_OPTION_PREFERRED,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'r':
                            case 'R':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_RELATIVE,
                                        DISPLAY_OPTION_RELATIVE,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'd':
                            case 'D':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_DECORATED,
                                        DISPLAY_OPTION_DECORATED,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'x':
                            case 'X':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_EXPORTED,
                                        DISPLAY_OPTION_EXPORTED,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'u':
                            case 'U':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_UNDEFINED,
                                        DISPLAY_OPTION_UNDEFINED,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 's':
                            case 'S':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_SPECIAL,
                                        DISPLAY_OPTION_SPECIAL,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            case 'z':
                            case 'Z':
                                {
                                Option (fPlus, &dOptions,
                                        DISPLAY_OPTION_ZERO,
                                        DISPLAY_OPTION_ZERO,
                                        DISPLAY_OPTION_DEFAULT);
                                break;
                                }
                            }
                        }
                    break;
                    }
                }
            }
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
