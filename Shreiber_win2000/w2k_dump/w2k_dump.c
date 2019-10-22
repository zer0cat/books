
// __________________________________________________________
//
//                         w2k_dump.c
//          SBS Windows 2000 Hex Dump Utility V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_dump.h"

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

WORD awArguments      [] = L"{ [+-bwdqp] <path> }";

WORD awMoreInfo       [] = L"\r\n"
                           L"       +   enable  option\r\n"
                           L"       -   disable option\r\n"
                           L"       b   display BYTEs\r\n"
                           L"       w   display WORDs\r\n"
                           L"       d   display DWORDs\r\n"
                           L"       q   display QWORDs\r\n"
                           L"       p   display PDB streams\r\n";

WORD awFileOk         [] = L"\r\nFile: %s\r\n"
                           L"Size: 0x%08lX (%lu)\r\n";

WORD awFileError      [] = L"\r\nFile: %s\r\n"
                           L"Unable to load this file\r\n";

WORD awFileType       [] = L"Type: %hs";

WORD awPdbStream      [] = L"\r\nPDB stream #%lu (range 0..%hu)\r\n"
                           L"Size: 0x%08lX (%lu)\r\n";

// -----------------------------------------------------------------

WORD awTableAddress   [] = L"Address "; // 8 characters

WORD awTableSpace1    [] = L" ";
WORD awTableSpace2    [] = L"  ";

WORD awTableHex1      [] = L"%01lX";
WORD awTableHex2      [] = L"%02lX";
WORD awTableHex8      [] = L"%08lX";

WORD awByte           [] = L"byte";
WORD awBytes          [] = L"bytes";
WORD awNot            [] = L"not ";
WORD awUndefined      [] = L"???";
WORD awNewLine        [] = L"\r\n";
WORD awString         [] = L"%s";
WORD awNull           [] = L"";

// -----------------------------------------------------------------

WORD awTableDataByte  [] =
    L"\r\n%s | "
    L"%s %s %s %s-%s %s %s %s : %s %s %s %s-%s %s %s %s | "
    L"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s";

WORD awTableBarByte   [] =
    L"\r\n---------|-"
    L"------------------------:-------------------------|-"
    L"----------------";

WORD awTableDataWord  [] =
    L"\r\n%s | "
    L"%s%s %s%s-%s%s %s%s : %s%s %s%s-%s%s %s%s | "
    L"%s%s %s%s %s%s %s%s %s%s %s%s %s%s %s%s";

WORD awTableBarWord   [] =
    L"\r\n---------|-"
    L"--------------------:---------------------|-"
    L"-----------------------";

WORD awTableDataDword [] =
    L"\r\n%s | "
    L"%s%s%s%s - %s%s%s%s : %s%s%s%s - %s%s%s%s | "
    L"%s%s%s%s %s%s%s%s %s%s%s%s %s%s%s%s";

WORD awTableBarDword  [] =
    L"\r\n---------|-"
    L"--------------------:---------------------|-"
    L"-------------------";

WORD awTableDataQword [] =
    L"\r\n%s | "
    L"%s%s%s%s-%s%s%s%s : %s%s%s%s-%s%s%s%s | "
    L"%s%s%s%s%s%s%s%s %s%s%s%s%s%s%s%s";

WORD awTableBarQword  [] =
    L"\r\n---------|-"
    L"------------------:-------------------|-"
    L"-----------------";

// =================================================================
// DISPLAY ROUTINES
// =================================================================

VOID WINAPI DisplayMemory (PBYTE pbData,
                           DWORD dData,
                           PVOID pBase,
                           DWORD dOptions)
    {
    DATA_ROW dr;
    PWORD    pwTableData, pwTableBar;
    DWORD    dGroup, dValue, i, j, k;

    if (dData)
        {
        switch (dOptions & DISPLAY_OPTION_FORMAT)
            {
            default:
            case DISPLAY_OPTION_BYTE:
                {
                pwTableData = awTableDataByte;
                pwTableBar  = awTableBarByte;
                dGroup      = sizeof (BYTE);
                break;
                }
            case DISPLAY_OPTION_WORD:
                {
                pwTableData = awTableDataWord;
                pwTableBar  = awTableBarWord;
                dGroup      = sizeof (WORD);
                break;
                }
            case DISPLAY_OPTION_DWORD:
                {
                pwTableData = awTableDataDword;
                pwTableBar  = awTableBarDword;
                dGroup      = sizeof (DWORD);
                break;
                }
            case DISPLAY_OPTION_QWORD:
                {
                pwTableData = awTableDataQword;
                pwTableBar  = awTableBarQword;
                dGroup      = sizeof (QWORD);
                break;
                }
            }
        dr.pArguments = &dr.pwAddress;
        dr.pwAddress  = dr.awAddress;

        for (j = 0; j < 16; j++)
            {
            dr.apwHex  [j] = dr.awHex  + (j * (2+1));
            dr.apwText [j] = dr.awText + (j * (1+1));
            }
        lstrcpy (dr.pwAddress, awTableAddress);

        for (j = 0; j < 16; j++)
            {
            k = (((j / dGroup) + 1) * dGroup) - ((j % dGroup) + 1);

            dValue = (k % dGroup == 0
                      ? ((DWORD_PTR) pBase & 0x0F) + k
                      : 0);

            sprintf (dr.apwHex  [j], awTableHex2, dValue);

            dValue = (k % dGroup == 0
                      ? ((DWORD_PTR) pBase + k) & 0x0F
                      : (k % dGroup == 1
                         ? (((DWORD_PTR) pBase & 0x0F) + k - 1) >> 4
                         : 0));

            sprintf (dr.apwText [j], awTableHex1, dValue);
            }
        vsprintf (dr.awBuffer, pwTableData, dr.pArguments);
        printf   (dr.awBuffer);
        printf   (pwTableBar);
        }
    for (i = 0; i < dData; i += j)
        {
        sprintf (dr.pwAddress, awTableHex8, (PBYTE) pBase + i);

        for (j = 0; j < 16; j++)
            {
            k = (((j / dGroup) + 1) * dGroup) - ((j % dGroup) + 1);

            if (i+k < dData)
                {
                dValue = pbData [i+k];
                sprintf (dr.apwHex [j], awTableHex2, dValue);

                if ((dValue < 0x20) ||
                    (dValue == 0x7F)) dValue = '.';

                dr.apwText [j] [0] = (WORD) dValue;
                dr.apwText [j] [1] = 0;
                }
            else
                {
                lstrcpy (dr.apwHex  [j], awTableSpace2);
                lstrcpy (dr.apwText [j], awTableSpace1);
                }
            }
        vsprintf (dr.awBuffer, pwTableData, dr.pArguments);
        printf   (awString, dr.awBuffer);
        }
    if (dData) printf (awNewLine);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayPdb (PIMG_PDB pip,
                        PVOID    pBase,
                        DWORD    dOptions)
    {
    DWORD     dStart, dStop, i;
    PPDB_ROOT ppr;
    PVOID     pData;
    DWORD     dData;

    if ((ppr = imgPdbRoot (pip)) != NULL)
        {
        if (dOptions & DISPLAY_OPTION_STREAM)
            {
            dStart = dOptions & DISPLAY_OPTION_STREAM_MASK;
            dStop  = min (dStart + 1, (DWORD) ppr->wCount);
            }
        else
            {
            dStart = 0;
            dStop  = ppr->wCount;
            }
        for (i = dStart; i < dStop; i++)
            {
            if ((pData = imgPdbStream (pip, i, &dData)) != NULL)
                {
                printf (awPdbStream,
                        i, ppr->wCount-1,
                        dData, dData);

                DisplayMemory (pData, dData, pBase, dOptions);

                imgMemoryDestroy (pData);
                }
            }
        imgMemoryDestroy (ppr);
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayFile (PWORD pwPath,
                         PVOID pBase,
                         DWORD dOptions)
    {
    BYTE     abType [PDB_SIGNATURE_TEXT];
    PVOID    pData;
    DWORD    dData = 0;
    PIMG_PDB pip   = NULL;

    if ((pData = imgFileLoad (pwPath, &dData)) != NULL)
        {
        printf (awFileOk, pwPath, dData, dData);

        if (imgPdbVerify (pData, dData))
            {
            pip = (PIMG_PDB) pData;

            lstrcpynA (abType, pip->Header.Signature.abSignature,
                       PDB_SIGNATURE_TEXT);

            printf (awFileType, abType);
            }
        if ((pip != NULL) && (dOptions & DISPLAY_OPTION_PDB))
            {
            DisplayPdb (pip, pBase, dOptions);
            }
        else
            {
            DisplayMemory (pData, dData, pBase, dOptions);
            }
        imgMemoryDestroy (pData);
        }
    else
        {
        printf (awFileError, pwPath);
        }
    return;
    }

// =================================================================
// OPTION MANAGEMENT
// =================================================================

VOID WINAPI OptionSet (BOOL   fPlus,
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

// -----------------------------------------------------------------

DWORD WINAPI OptionNumber (PWORD  pwNumber,
                           PDWORD pdDigits)
    {
    DWORD i;
    DWORD dNumber = 0;

    *pdDigits = 0;

    for (i = 0; (pwNumber [i] >= '0') && (pwNumber [i] <= '9'); i++)
        {
        dNumber *= 10;
        dNumber += (pwNumber [i] - '0');

        (*pdDigits)++;
        }
    return dNumber;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    DWORD dOptions, dStream, dDigits, i, j;
    BOOL  fPlus;

    printf (atAbout);

    if (argc < 2)
        {
        printf (atUsage, awArguments);
        printf (awMoreInfo);
        }
    else
        {
        dOptions = DISPLAY_OPTION_DEFAULT;

        for (i = 1; i < argc; i++)
            {
            if ((argv [i] [0] == '+') || (argv [i] [0] == '-'))
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
                        case 'b':
                        case 'B':
                            {
                            OptionSet (fPlus, &dOptions,
                                       DISPLAY_OPTION_FORMAT,
                                       DISPLAY_OPTION_BYTE,
                                       DISPLAY_OPTION_DEFAULT);
                            break;
                            }
                        case 'w':
                        case 'W':
                            {
                            OptionSet (fPlus, &dOptions,
                                       DISPLAY_OPTION_FORMAT,
                                       DISPLAY_OPTION_WORD,
                                       DISPLAY_OPTION_DEFAULT);
                            break;
                            }
                        case 'd':
                        case 'D':
                            {
                            OptionSet (fPlus, &dOptions,
                                       DISPLAY_OPTION_FORMAT,
                                       DISPLAY_OPTION_DWORD,
                                       DISPLAY_OPTION_DEFAULT);
                            break;
                            }
                        case 'q':
                        case 'Q':
                            {
                            OptionSet (fPlus, &dOptions,
                                       DISPLAY_OPTION_FORMAT,
                                       DISPLAY_OPTION_QWORD,
                                       DISPLAY_OPTION_DEFAULT);
                            break;
                            }
                        case 'p':
                        case 'P':
                            {
                            OptionSet (fPlus, &dOptions,
                                       DISPLAY_OPTION_PDB,
                                       DISPLAY_OPTION_PDB,
                                       DISPLAY_OPTION_DEFAULT);
                            break;
                            }
                        }
                    }
                }
            else
                {
                if (argv [i] [0] == '/')
                    {
                    dStream = OptionNumber (argv [i] + 1, &dDigits)
                              & DISPLAY_OPTION_STREAM_MASK;

                    if (dDigits)
                        {
                        dOptions &= ~DISPLAY_OPTION_STREAM_MASK;
                        dOptions |=  dStream;
                        dOptions |=  DISPLAY_OPTION_STREAM;
                        }
                    else
                        {
                        dOptions ^=  DISPLAY_OPTION_STREAM;
                        }
                    }
                else
                    {
                    DisplayFile (argv [i], (PVOID) 0, dOptions);
                    }
                }
            }
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
