
// __________________________________________________________
//
//                         w2k_cv.c
//         SBS Windows 2000 CodeView Decompiler V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_cv.h"

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

TBYTE atArguments   [] =
 T("{ [+-hedcpPfmstx] <module> }");

TBYTE atMoreInfo    [] =
 T("\r\n")
 T("       +   enable subsequent options\r\n")
 T("       -   disable subsequent options\r\n")
 T("       h   display PE section headers\r\n")
 T("       e   display exported symbols\r\n")
 T("       d   display debug directories\r\n")
 T("       c   display CodeView data\r\n")
 T("       p   display public symbols\r\n")
 T("       P   display public symbols (extended information)\r\n")
 T("       f   display frame pointer omission (FPO) data\r\n")
 T("       m   display miscellaneous data\r\n")
 T("       s   display OMAP source table\r\n")
 T("       t   display OMAP target table\r\n")
 T("       x   display all of the above\r\n");

// -----------------------------------------------------------------

TBYTE atSeparator   [] =
 T("----------------------------------------")
 T("---------------------------------------\r\n");

TBYTE atSections    [] =
 T("\r\n%lu PE file section%s\r\n\r\n")
 T("     #  Name     Source   Target   Size     Raw Size\r\n")
 T("\0\0s\0\r\nNo PE file sections\r\n");

TBYTE atExports     [] =
 T("\r\n%lu exported symbol%s\r\n\r\n")
 T("     #  Symbol\r\n")
 T("\0\0s\0\r\nNo exported symbols\r\n");

TBYTE atDirectories [] =
 T("\r\n%lu debug director%s\r\n\r\n")
 T("     #  Address  Size     Type     Description\r\n")
 T("\0y\0ies\0\r\nNo debug directories\r\n");

TBYTE atSegments    [] =
 T("\r\n%lu CodeView NB09 segment%s (%hu logical)\r\n\r\n")
 T("     # Frame Offset   Size     Flags\r\n")
 T("\0\0s\0\r\nNo NB09 segments\r\n");

TBYTE atCodeView09  [] =
 T("\r\n%lu CodeView NB09 directory entr%s\r\n\r\n")
 T("     #  Offset   Size     Type Index Name\r\n")
 T("\0y\0ies\0\r\nNo CodeView NB09 directory entries\r\n");

TBYTE atCodeView10  [] =
 T("\r\n%lu CodeView NB10 byte%s\r\n\r\n")
 T("     #  Member     Value\r\n")
 T("\0\0s\0");

TBYTE atCodeViewXX  [] =
 T("\r\n%lu CodeView byte%s (%hc%hc%hc%hc)\r\n\r\n")
 T("Offset  ")
 T(" | 00 01 02 03 04 05 06 07 : 08 09 0A 0B 0C 0D 0E 0F")
 T(" | 0123456789ABCDEF\r\n")
 T("\0\0s\0\r\nNo CodeView data\r\n");

TBYTE atSymbols     [] =
 T("\r\n%lu symbol%s (%hc%hc%hc%hc)\r\n\r\n")
 T("     #  Section  Address  Symbol\r\n")
 T("\0\0s\0\r\nNo symbols\r\n");

TBYTE atSymbolsEx   [] =
 T("\r\n%lu symbol%s (%hc%hc%hc%hc)\r\n\r\n")
 T("     #  Section  Raw      Source   Target   Address  ")
 T("  Offset    Stack X Symbol\r\n")
 T("\0\0s\0\r\nNo symbols\r\n");

TBYTE atFpoEntries   [] =
 T("\r\n%lu FPO entr%s\r\n\r\n")
 T("     #  Address  Size     Locals   Args\r\n")
 T("\0y\0ies\0\r\nNo FPO entries\r\n");

TBYTE atMiscEntries  [] =
 T("\r\n%lu miscellaneous entr%s\r\n\r\n")
 T("     #  Type     Size     Data\r\n")
 T("\0y\0ies\0\r\nNo miscellaneous entries\r\n");

TBYTE atOmapFromSrc  [] =
 T("\r\n%lu OMAP source entr%s\r\n\r\n")
 T("     #  Source   Target\r\n")
 T("\0y\0ies\0\r\nNo OMAP source entries\r\n");

TBYTE atOmapToSrc    [] =
 T("\r\n%lu OMAP target entr%s\r\n\r\n")
 T("     #  Target   Source\r\n")
 T("\0y\0ies\0\r\nNo OMAP target entries\r\n");

// =================================================================
// PRIMITIVE DISPLAY ROUTINES
// =================================================================

VOID DisplaySeparator (VOID)
    {
    printf  (atSeparator);
    return;
    }

// -----------------------------------------------------------------

VOID DisplayTop (PTBYTE ptFormat,
                 DWORD  dCount,
                 PTBYTE ptSuffix,
                 ...)
    {
    PVOID  pArguments = &dCount;
    PTBYTE ptText     = ptFormat;

    ptText += lstrlen (ptText) + 1;

    if (dCount != 1)
        {
        ptText += lstrlen (ptText) + 1;
        }
    if (dCount == -1)
        {
        ptText += lstrlen (ptText) + 1;
        printf (ptText);
        }
    else
        {
        ((PTBYTE *) pArguments) [1] = ptText;
        vprintf (ptFormat, pArguments);

        DisplaySeparator ();
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayTime (PTBYTE ptFormat,
                         DWORD  dTime)
    {
    TBYTE    atTime [] = T("mm-dd-yyyy HH:MM:SS");
    IMG_TIME it        = imgTimeUnpack (dTime);

    sprintf (atTime, T("%02lu-%02lu-%04lu %02lu:%02lu:%02lu"),
            (DWORD) it.bMonth,
            (DWORD) it.bDay,
            (DWORD) it.wYear,
            (DWORD) it.bHour,
            (DWORD) it.bMinute,
            (DWORD) it.bSecond);

    printf (ptFormat, atTime);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayOmfName (POMF_NAME pon)
    {
    DWORD i;

    for (i = 0; i < pon->bLength; i++)
        {
        printf (T("%hc"),
                pon->abName [i]);
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplaySectionName (PIMG_INFO pii,
                                DWORD     dSection)
    {
    DWORD                 i, j;
    PIMAGE_SECTION_HEADER pish = pii->pSections + (dSection-1);

    for (i = j = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++)
        {
        printf (T("%hc"),
                (pish->Name [j] ? pish->Name [j++] : ' '));
        }
    return;
    }

// =================================================================
// COMPLEX DISPLAY ROUTINES
// =================================================================

VOID WINAPI DisplaySections (PIMG_INFO pii)
    {
    DWORD i;

    if (pii->dSections)
        {
        DisplayTop (atSections, pii->dSections, NULL);

        for (i = 0; i < pii->dSections; i++)
            {
            printf (T("%6lu: "), i+1);

            DisplaySectionName (pii, i+1);

            printf (T(" %08lX %08lX %08lX %08lX\r\n"),
                    pii->pSections [i].PointerToRawData,
                    pii->pSections [i].VirtualAddress,
                    pii->pSections [i].Misc.VirtualSize,
                    pii->pSections [i].SizeOfRawData);
            }
        }
    else
        {
        DisplayTop (atSections, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayExports (PIMG_INFO pii)
    {
    DWORD i, j;

    if (pii->dExports)
        {
        DisplayTop (atExports, pii->dExports, NULL);

        for (i = j = 0; i < pii->dExports; i++)
            {
            printf (T("%6lu: %hs\r\n"),
                    i+1,
                    pii->pbExports + j);

            j += (lstrlenA (pii->pbExports + j) + 1);
            }
        }
    else
        {
        DisplayTop (atExports, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayDirectories (PIMG_INFO pii)
    {
    DWORD i;

    if (pii->dDirectories)
        {
        DisplayTop (atDirectories, pii->dDirectories, NULL);

        for (i = 0; i < pii->dDirectories; i++)
            {
            printf (T("%6lu: %08lX %08lX %08lX %s\r\n"),
                    i+1,
                    pii->pDirectories [i].PointerToRawData,
                    pii->pDirectories [i].SizeOfData,
                    pii->pDirectories [i].Type,
                    imgInfoType (pii, i));
            }
        }
    else
        {
        DisplayTop (atDirectories, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayNB09Segments (PIMG_INFO pii,
                                 PCV_NB09  pc09)
    {
    PCV_SEGMAP pcs;
    DWORD      dCount, i;

    if ((pcs = imgCvSegments (pc09, &dCount)) != NULL)
        {
        DisplayTop (atSegments, dCount, NULL, pcs->wLogical);

        for (i = 0; i < dCount; i++)
            {
            printf (T("%6lu: %04hX %08lX %08lX %04hX\r\n"),
                    i+1,
                    pcs->Descriptors [i].wFrame,
                    pcs->Descriptors [i].dOffset,
                    pcs->Descriptors [i].dSize,
                    pcs->Descriptors [i].wFlags);
            }
        }
    else
        {
        DisplayTop (atSegments, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }


// -----------------------------------------------------------------

VOID WINAPI DisplayNB09Data (PIMG_INFO pii,
                             PCV_NB09  pc09,
                             DWORD     dSize)
    {
    DWORD      i, j;
    PCV_MODULE pcm;

    if (pc09->Directory.dEntries)
        {
        DisplayTop (atCodeView09, pc09->Directory.dEntries, NULL);

        for (i = j = 0; i < pc09->Directory.dEntries; i++)
            {
            printf (T("%6lu: %08lX %08lX %04hX %04hX "),
                    i+1,
                    pc09->Entries [i].lSubSectionOffset,
                    pc09->Entries [i].dSubSectionSize,
                    pc09->Entries [i].wSubSectionType,
                    pc09->Entries [i].wModuleIndex);

            if ((pc09->Entries [i].wSubSectionType == sstModule) &&
                ((pcm = imgCvModule (pc09, j++, NULL)) != NULL))
                {
                printf (T(" "));
                DisplayOmfName (CV_MODULE_NAME (pcm));
                }
            printf (T("\r\n"));
            }
        DisplaySeparator ();
        DisplayNB09Segments (pii, pc09);
        }
    else
        {
        DisplayTop (atCodeView09, -1, NULL);
        DisplaySeparator ();
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayNB10Data (PIMG_INFO pii,
                             PCV_NB10  pc10,
                             DWORD     dSize)
    {
    DisplayTop (atCodeView10, dSize, NULL);

    DisplayTime (T("     1: Signature  %s\r\n"),
                 pc10->dSignature);

    printf      (T("     2: Age        %08lX\r\n"),
                 pc10->dAge);

    printf      (T("     3: PDB name   %hs\r\n"),
                 pc10->abPdbName);

    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayNBXXData (PIMG_INFO  pii,
                             PCV_HEADER pch,
                             DWORD      dSize)
    {
    DWORD i, j, k, n;

    DisplayTop (atCodeViewXX, dSize, NULL,
                pch->Signature.abText [0],
                pch->Signature.abText [1],
                pch->Signature.abText [2],
                pch->Signature.abText [3]);

    n = (((dSize - 1) / 16) + 1) * 16;

    for (i = 0; i < n; i += 16)
        {
        printf (T("%08lX | "), i);

        for (j = i; j < i + 16; j++)
            {
            if ((j & 0xF) == 8)
                {
                printf (T(": "));
                }
            if (j < dSize)
                {
                k = ((PBYTE) pch) [j];
                printf (T("%02lX "), k);
                }
            else
                {
                printf (T("   "));
                }
            }
        printf (T("| "));

        for (j = i; j < i + 16; j++)
            {
            if (j < dSize)
                {
                k = ((PBYTE) pch) [j];

                if ((k >= 0x20) && (k != 0x7F))
                    {
                    printf (T("%c"), k);
                    }
                else
                    {
                    printf (T("."));
                    }
                }
            }
        printf (T("\r\n"));
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayCvData (PIMG_INFO pii)
    {
    if (pii->dCvData >= CV_HEADER_)
        {
        switch (pii->pCvData->Header.Signature.dVersion)
            {
            case CV_SIGNATURE_NB09:
                {
                DisplayNB09Data (pii, &pii->pCvData->NB09,
                                 pii->dCvData);
                break;
                }
            case CV_SIGNATURE_NB10:
                {
                DisplayNB10Data (pii, &pii->pCvData->NB10,
                                 pii->dCvData);
                break;
                }
            default:
                {
                DisplayNBXXData (pii, &pii->pCvData->Header,
                                 pii->dCvData);
                break;
                }
            }
        }
    else
        {
        DisplayTop (atCodeViewXX, -1, NULL);
        DisplaySeparator ();
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplaySymbols (PIMG_INFO pii)
    {
    PIMG_PUBSYM pip, pip1;
    IMG_SYMBOL  is;
    DWORD       dCount, dSize, dVersion, dIndex;

    if ((pip = imgInfoSymbols (pii, &dCount, &dSize, &dVersion))
        != NULL)
        {
        DisplayTop (atSymbols, dCount, NULL,
                    ((PCV_SIGNATURE) &dVersion)->abText [0],
                    ((PCV_SIGNATURE) &dVersion)->abText [1],
                    ((PCV_SIGNATURE) &dVersion)->abText [2],
                    ((PCV_SIGNATURE) &dVersion)->abText [3]);

        dIndex = 0;
        pip1   = pip;

        while (dIndex < dCount)
            {
            if (imgSymbolTest (pip1))
                {
                imgSymbolInfo (pip1, pii, &is, TRUE);

                printf (T("%6lu: "), dIndex+1);

                if (is.dSection &&
                    (is.dSection <= pii->dSections))
                    {
                    DisplaySectionName (pii, is.dSection);
                    }
                else
                    {
                    printf (T("[#%04hX] "), is.dSection);
                    }
                printf (T(" %08lX %hs\r\n"),
                        is.pAddress, is.abName);

                dIndex++;
                }
            pip1 = imgSymbolNext (pip1);
            }
        imgMemoryDestroy (pip);
        }
    else
        {
        DisplayTop (atSymbols, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplaySymbolsEx (PIMG_INFO pii)
    {
    PIMG_PUBSYM pip, pip1;
    IMG_SYMBOL  is;
    DWORD       dCount, dSize, dVersion, dIndex;

    if ((pip = imgInfoSymbols (pii, &dCount, &dSize, &dVersion))
        != NULL)
        {
        DisplayTop (atSymbolsEx, dCount, NULL,
                    ((PCV_SIGNATURE) &dVersion)->abText [0],
                    ((PCV_SIGNATURE) &dVersion)->abText [1],
                    ((PCV_SIGNATURE) &dVersion)->abText [2],
                    ((PCV_SIGNATURE) &dVersion)->abText [3]);

        dIndex = 0;
        pip1   = pip;

        while (dIndex < dCount)
            {
            if (imgSymbolTest (pip1))
                {
                imgSymbolInfo (pip1, pii, &is, TRUE);

                printf (T("%6lu: "), dIndex+1);

                if (is.dSection &&
                    (is.dSection <= pii->dSections))
                    {
                    DisplaySectionName (pii, is.dSection);
                    }
                else
                    {
                    printf (T("[#%04hX] "), is.dSection);
                    }
                printf (T(" %08lX %08lX %08lX %08lX %8lX "),
                        is.dRaw,     is.dSource, is.dTarget,
                        is.pAddress, is.dOffset);

                if (is.dStack != -1)
                    {
                    printf (T("%8lX "), is.dStack);
                    }
                else
                    {
                    printf (T("         "));
                    }
                printf (T("%c %hs\r\n"),
                        (imgSymbolExported (pip1, pii) ? '*' : ' '),
                        is.abName);

                dIndex++;
                }
            pip1 = imgSymbolNext (pip1);
            }
        imgMemoryDestroy (pip);
        }
    else
        {
        DisplayTop (atSymbols, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayFpoEntries (PIMG_INFO pii)
    {
    DWORD i;

    if (pii->dFpoEntries)
        {
        DisplayTop (atFpoEntries, pii->dFpoEntries, NULL);

        for (i = 0; i < pii->dFpoEntries; i++)
            {
            printf (T("%6lu: %08lX %08lX %08lX %04hX\r\n"),
                    i+1,
                    pii->pFpoEntries [i].ulOffStart,
                    pii->pFpoEntries [i].cbProcSize,
                    pii->pFpoEntries [i].cdwLocals,
                    pii->pFpoEntries [i].cdwParams);
            }
        }
    else
        {
        DisplayTop (atFpoEntries, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayMiscEntries (PIMG_INFO pii)
    {
    DWORD i;

    if (pii->dMiscEntries)
        {
        DisplayTop (atMiscEntries, pii->dMiscEntries, NULL);

        for (i = 0; i < pii->dMiscEntries; i++)
            {
            printf ((pii->pMiscEntries [i].Unicode
                     ? T("%6lu: %08lX %08lX %ls\r\n")
                     : T("%6lu: %08lX %08lX %hs\r\n")),
                    i+1,
                    pii->pMiscEntries [i].DataType,
                    pii->pMiscEntries [i].Length,
                    pii->pMiscEntries [i].Data);
            }
        }
    else
        {
        DisplayTop (atMiscEntries, -1, NULL);
        }
    DisplaySeparator ();
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayOmapFromSrc (PIMG_INFO pii)
    {
    DWORD i, j;

    if (pii->dOmapFromSrc)
        {
        DisplayTop (atOmapFromSrc, pii->dOmapFromSrc, NULL);

        for (i = j = 0; i < pii->dOmapFromSrc; i++)
            {
            printf (T("%6lu: %08lX %08lX\r\n"),
                    i+1,
                    pii->pOmapFromSrc [i].dSource,
                    pii->pOmapFromSrc [i].dTarget);

            if (!pii->pOmapFromSrc [i].dTarget) j++;
            }
        DisplaySeparator ();

        printf (T("%6lu entr%s mapped to zero\r\n"),
                j, (j == 1 ? T("y") : T("ies")));
        }
    else
        {
        DisplayTop (atOmapFromSrc, -1, NULL);
        DisplaySeparator ();
        }
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI DisplayOmapToSrc (PIMG_INFO pii)
    {
    DWORD i, j;

    if (pii->dOmapToSrc)
        {
        DisplayTop (atOmapToSrc, pii->dOmapToSrc, NULL);

        for (i = j = 0; i < pii->dOmapToSrc; i++)
            {
            printf (T("%6lu: %08lX %08lX\r\n"),
                    i+1,
                    pii->pOmapToSrc [i].dTarget,
                    pii->pOmapToSrc [i].dSource);

            if (!pii->pOmapToSrc [i].dSource) j++;
            }
        DisplaySeparator ();

        printf (T("%6lu entr%s mapped to zero\r\n"),
                j, (j == 1 ? T("y") : T("ies")));
        }
    else
        {
        DisplayTop (atOmapToSrc, -1, NULL);
        DisplaySeparator ();
        }
    return;
    }

// =================================================================
// MODULE HANDLER
// =================================================================

void WINAPI DisplayModule (PTBYTE ptModule,
                           DWORD  dOptions)
    {
    TBYTE     atPath [MAX_PATH];
    PVOID     pBase;
    PIMG_INFO pii;

    pBase = imgModuleBase (ptModule);

    if (imgPathDbg (ptModule, atPath, MAX_PATH) &&
        ((pii = imgInfoLoad (atPath, pBase)) != NULL))
        {
        printf (T("\r\nModule name:  %s")
                T("\r\nModule base:  %08lX%s")
                T("\r\nSymbol file:  %s\r\n"),
                ptModule, pii->pBase,
                (pBase != NULL ? T("") : T(" (not resident)")),
                pii->atPath);

        if (dOptions & OPTION_SECTIONS   ) DisplaySections    (pii);
        if (dOptions & OPTION_EXPORTS    ) DisplayExports     (pii);
        if (dOptions & OPTION_DIRECTORIES) DisplayDirectories (pii);
        if (dOptions & OPTION_CODEVIEW   ) DisplayCvData      (pii);
        if (dOptions & OPTION_PUBSYM     ) DisplaySymbols     (pii);
        if (dOptions & OPTION_PUBSYM_EX  ) DisplaySymbolsEx   (pii);
        if (dOptions & OPTION_FPO        ) DisplayFpoEntries  (pii);
        if (dOptions & OPTION_MISC       ) DisplayMiscEntries (pii);
        if (dOptions & OPTION_SOURCE     ) DisplayOmapFromSrc (pii);
        if (dOptions & OPTION_TARGET     ) DisplayOmapToSrc   (pii);

        imgMemoryDestroy   (pii);
        }
    else
        {
        printf (T("\r\nNo symbol file available for %s\r\n"),
                ptModule);
        }
    return;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    DWORD dOption, dOptions, i, j;
    BOOL  fPlus;

    printf (atAbout);

    if (argc < 2)
        {
        printf (atUsage, atArguments);
        printf (atMoreInfo);
        }
    else
        {
        dOptions = OPTION_DEFAULT;

        for (i = 1; i < argc; i++)
            {
            switch (argv [i] [0])
                {
                default:
                    {
                    DisplayModule (argv [i], dOptions);
                    break;
                    }
                case '+':
                case '-':
                    {
                    for (j = 0; argv [i] [j]; j++)
                        {
                        dOption = 0;

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
                            case 'h':
                            case 'H':
                                {
                                dOption = OPTION_SECTIONS;
                                break;
                                }
                            case 'e':
                            case 'E':
                                {
                                dOption = OPTION_EXPORTS;
                                break;
                                }
                            case 'd':
                            case 'D':
                                {
                                dOption = OPTION_DIRECTORIES;
                                break;
                                }
                            case 'c':
                            case 'C':
                                {
                                dOption = OPTION_CODEVIEW;
                                break;
                                }
                            case 'p':
                                {
                                dOption = OPTION_PUBSYM;
                                break;
                                }
                            case 'P':
                                {
                                dOption = OPTION_PUBSYM_EX;
                                break;
                                }
                            case 'f':
                            case 'F':
                                {
                                dOption = OPTION_FPO;
                                break;
                                }
                            case 'm':
                            case 'M':
                                {
                                dOption = OPTION_MISC;
                                break;
                                }
                            case 's':
                            case 'S':
                                {
                                dOption = OPTION_SOURCE;
                                break;
                                }
                            case 't':
                            case 'T':
                                {
                                dOption = OPTION_TARGET;
                                break;
                                }
                            case 'x':
                            case 'X':
                                {
                                dOption = OPTION_ALL;
                                break;
                                }
                            }
                        if (dOption)
                            {
                            if (fPlus) dOptions |=  dOption;
                            else       dOptions &= ~dOption;
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
