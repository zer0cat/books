
// __________________________________________________________
//
//                         w2k_mem.c
//             SBS Windows 2000 Memory Spy V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_mem.h"

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

WORD awArguments    [] =
    L"{ { [+option|-option] [/<path>] } "
    L"[#[[0]x]<size>] [[0]x]<base> }";

WORD awMoreInfo     [] =
    L"\r\n"
    L"<path> specifies a module to be loaded into memory.\r\n"
    L"Use the +x/-x switch to enable/disable its startup code.\r\n"
    L"If <size> is missing, the default size is %lu bytes.\r\n";

WORD awOptions      [] =
    L"\r\n"
    L"Display address options (mutually exclusive):\r\n"
    L"\r\n"
    L"   +z -z   zero-based display         on / OFF\r\n"
    L"   +r -r   physical RAM addresses     on / OFF\r\n"
    L"\r\n"
    L"Display mode options (mutually exclusive):\r\n"
    L"\r\n"
    L"   +w -w   WORD  data formatting      on / OFF\r\n"
    L"   +d -d   DWORD data formatting      on / OFF\r\n"
    L"   +q -q   QWORD data formatting      on / OFF\r\n"
    L"\r\n"
    L"Addressing options (mutually exclusive):\r\n"
    L"\r\n"
    L"   +t -t   TEB-relative addressing    on / OFF\r\n"
    L"   +f -f   FS-relative  addressing    on / OFF\r\n"
    L"   +u -u   user-mode   FS:[<base>]    on / OFF\r\n"
    L"   +k -k   kernel-mode FS:[<base>]    on / OFF\r\n"
    L"   +h -h   handle/object resolution   on / OFF\r\n"
    L"   +a -a   add bias  to  last base    on / OFF\r\n"
    L"   +s -s   sub bias from last base    on / OFF\r\n"
    L"   +p -p   pointer  from last block   on / OFF\r\n"
    L"\r\n"
    L"System status options (cumulative):\r\n"
    L"\r\n"
    L"   +o -o   display OS  information    on / OFF\r\n"
    L"   +c -c   display CPU information    on / OFF\r\n"
    L"   +g -g   display GDT information    on / OFF\r\n"
    L"   +i -i   display IDT information    on / OFF\r\n"
    L"   +b -b   display contiguous blocks  on / OFF\r\n"
    L"\r\n"
    L"Other options (cumulative):\r\n"
    L"\r\n"
    L"   +x -x   execute DLL startup code   on / OFF\r\n";

WORD awExamples     [] =
    L"\r\n"
    L"Example: The following command displays the first 64\r\n"
    L"bytes of the current Process Environment Block (PEB)\r\n"
    L"in zero-based DWORD format, assuming that a pointer to\r\n"
    L"the PEB is located at offset 0x30 inside the current\r\n"
    L"Thread Environment Block (TEB):\r\n"
    L"\r\n"
    L"   " SW(MAIN_MODULE) L" +t #0 0 +pzd #64 0x30\r\n"
    L"\r\n"
    L"Note: Specifying #0 after +t causes the TEB to be\r\n"
    L"addressed without displaying its contents.\r\n";

WORD awSummary           [] = L"\r\n"
                              L"%10lu %s requested\r\n"
                              L"%10lu %s received\r\n";

WORD awInvalidCommand    [] = L"%sYou didn't request any data!\r\n";
WORD awInvalidArgument   [] = L"%sInvalid argument: \"%s\"\r\n";
WORD awInvalidOption     [] = L"%sInvalid option: \"%c%c\"\r\n";
WORD awInvalidModule     [] = L"%sLoad error 0x%08lX: \"%s\"\r\n";

WORD awLoadLibrary       [] = L"\r\nLoadLibrary (%s) = 0x%08lX\r\n";

// -----------------------------------------------------------------

WORD awOsInfoCaption     [] =
    L"\r\n"
    L"OS information:\r\n"
    L"---------------\r\n";

// -----------------------------------------------------------------

WORD awCpuInfoCaption    [] =
    L"\r\n"
    L"CPU information:\r\n"
    L"----------------\r\n";

WORD awCpuInfoUser       [] =
    L"\r\n"
    L"User mode segments:\r\n"
    L"\r\n";

WORD awCpuInfoKernel     [] =
    L"\r\n"
    L"Kernel mode segments:\r\n"
    L"\r\n";

WORD awCpuInfoOther      [] =
    L"\r\n"
    L"IDT : Limit    = %04X, Base = %08lX\r\n"
    L"GDT : Limit    = %04X, Base = %08lX\r\n"
    L"LDT : Selector = %04X\r\n"
    L"\r\n"
    L"CR0 : Contents = %08lX\r\n"
    L"CR2 : Contents = %08lX\r\n"
    L"CR3 : Contents = %08lX\r\n";

// -----------------------------------------------------------------

WORD awGdtInfoCaption    [] =
    L"\r\n"
    L"GDT information:\r\n"
    L"----------------\r\n"
    L"\r\n";

WORD awGdtInfoSegment    [] =
    L"%s : Selector = %04X, "
    L"Base = %08lX, Limit = %08lX, DPL%u, Type = %s\r\n";

// -----------------------------------------------------------------

WORD awIdtInfoCaption    [] =
    L"\r\n"
    L"IDT information:\r\n"
    L"----------------\r\n"
    L"\r\n";

WORD awIdtInfoPointer    [] =
    L"%02lX : Pointer = %04X:%08lX, "
    L"Base = %08lX, Limit = %08lX, Type = %s\r\n";

WORD awIdtInfoSegment    [] =
    L"%02lX : TSS     = %04X,          "
    L"Base = %08lX, Limit = %08lX, Type = %s\r\n";

// -----------------------------------------------------------------

#define N_SEGMENT N_HEX32

WORD awSegment           [] = L"%03lX";
WORD awSegmentCS         [] = L"CS ";
WORD awSegmentDS         [] = L"DS ";
WORD awSegmentES         [] = L"ES ";
WORD awSegmentFS         [] = L"FS ";
WORD awSegmentGS         [] = L"GS ";
WORD awSegmentSS         [] = L"SS ";
WORD awSegmentTSS        [] = L"TSS";

// -----------------------------------------------------------------

PWORD apwTypeApplication [] =
    {
    L"DATA ---", L"DATA --a", L"DATA -w-", L"DATA -wa",
    L"DATA e--", L"DATA e-a", L"DATA ew-", L"DATA ewa",
    L"CODE ---", L"CODE --a", L"CODE -r-", L"CODE -ra",
    L"CODE c--", L"CODE c-a", L"CODE cr-", L"CODE cra",
    };

PWORD apwTypeSystem      [] =
    {
    NULL,        L"TSS16 a",  L"LDT",      L"TSS16 b",
    L"CALL16",   L"TASK",     L"INT16",    L"TRAP16",
    NULL,        L"TSS32 a",  NULL,        L"TSS32 b",
    L"CALL32",   NULL,        L"INT32",    L"TRAP32",
    };

// -----------------------------------------------------------------

WORD awTableCaption [] = L"\r\n%08lX..%08lX: %lu valid %s\r\n";
WORD awTableNoData  [] = L"\r\n%08lX: No data\r\n";

WORD awTableAddress [] = L"Address "; // 8 characters

WORD awTableSpace1  [] = L" ";
WORD awTableSpace2  [] = L"  ";

WORD awTableHex1    [] = L"%01lX";
WORD awTableHex2    [] = L"%02lX";
WORD awTableHex8    [] = L"%08lX";

WORD awByte         [] = L"byte";
WORD awBytes        [] = L"bytes";
WORD awNot          [] = L"not ";
WORD awUndefined    [] = L"???";
WORD awNewLine      [] = L"\r\n";
WORD awString       [] = L"%s";
WORD awNull         [] = L"";

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
// SPY DEVICE I/O
// =================================================================

BOOL WINAPI IoControl (HANDLE hDevice,
                       DWORD  dCode,
                       PVOID  pInput,
                       DWORD  dInput,
                       PVOID  pOutput,
                       DWORD  dOutput)
    {
    DWORD dData = 0;

    return DeviceIoControl (hDevice, dCode,
                            pInput,  dInput,
                            pOutput, dOutput,
                            &dData,  NULL)
           &&
           (dData == dOutput);
    }

// -----------------------------------------------------------------

BOOL WINAPI ReadBinary (HANDLE hDevice,
                        DWORD  dCode,
                        PVOID  pOutput,
                        DWORD  dOutput)
    {
    return IoControl (hDevice, dCode, NULL, 0, pOutput, dOutput);
    }

// -----------------------------------------------------------------

BOOL WINAPI ReadCpuInfo (HANDLE        hDevice,
                         PSPY_CPU_INFO psci)
    {
    return IoControl (hDevice, SPY_IO_CPU_INFO,
                      NULL,    0,
                      psci,    SPY_CPU_INFO_);
    }

// -----------------------------------------------------------------

BOOL WINAPI ReadSegment (HANDLE       hDevice,
                         DWORD        dSelector,
                         PSPY_SEGMENT pss)
    {
    return IoControl (hDevice,    SPY_IO_SEGMENT,
                      &dSelector, DWORD_,
                      pss,        SPY_SEGMENT_);
    }

// -----------------------------------------------------------------

BOOL WINAPI ReadPhysical (HANDLE            hDevice,
                          PVOID             pLinear,
                          PPHYSICAL_ADDRESS ppa)
    {
    return IoControl (hDevice,  SPY_IO_PHYSICAL,
                      &pLinear, PVOID_,
                      ppa,      PHYSICAL_ADDRESS_)
           &&
           (ppa->LowPart || ppa->HighPart);
    }

// =================================================================
// MEMORY MANAGEMENT
// =================================================================

PSPY_MEMORY_DATA WINAPI MemoryRelease (PSPY_MEMORY_DATA psmd)
    {
    if (psmd != NULL) LocalFree (psmd);
    return NULL;
    }

// -----------------------------------------------------------------

PSPY_MEMORY_DATA WINAPI MemoryRead (HANDLE hDevice,
                                    PVOID  pAddress,
                                    DWORD  dBytes)
    {
    SPY_MEMORY_BLOCK smb;
    DWORD            dData = SPY_MEMORY_DATA__ (dBytes);
    PSPY_MEMORY_DATA psmd  = NULL;

    if ((psmd = LocalAlloc (LMEM_FIXED, dData)) != NULL)
        {
        smb.pAddress = pAddress;
        smb.dBytes   = dBytes;

        if (!IoControl (hDevice, SPY_IO_MEMORY_DATA,
                        &smb,    SPY_MEMORY_BLOCK_,
                        psmd,    dData))
            {
            psmd = MemoryRelease (psmd);
            }
        }
    return psmd;
    }

// -----------------------------------------------------------------

BOOL WINAPI MemoryPointer (HANDLE hDevice,
                           PVOID  pAddress,
                           PPVOID ppData)
    {
    PSPY_MEMORY_DATA psmd;
    DWORD            i, j;
    PBYTE            pbData = 0;
    BOOL             fOk    = FALSE;

    if ((psmd = MemoryRead (hDevice, pAddress, PVOID_)) != NULL)
        {
        for (i = j = 0; i < psmd->smb.dBytes; i++, j += 8)
            {
            if (!(psmd->awData [i] & SPY_MEMORY_DATA_VALID)) break;
            pbData += (psmd->awData [i] & SPY_MEMORY_DATA_BYTE) <<j;
            }
        if (i == psmd->smb.dBytes)
            {
            *ppData = pbData;
            fOk     = TRUE;
            }
        MemoryRelease (psmd);
        }
    return fOk;
    }

// -----------------------------------------------------------------

DWORD WINAPI MemoryDisplay (HANDLE           hDevice,
                            PSPY_MEMORY_DATA psmd,
                            DWORD            dOptions)
    {
    PBYTE            pbDisplay;
    PHYSICAL_ADDRESS pa;
    DATA_ROW         dr;
    PWORD            pwTableData, pwTableBar;
    DWORD            dData, dGroup, i, j, k;
    DWORD            n = 0;

    switch (dOptions & COMMAND_OPTION_ADDRESS)
        {
        default:
            {
            pbDisplay = psmd->smb.pAddress;
            break;
            }
        case COMMAND_OPTION_ZERO:
            {
            pbDisplay = 0;
            break;
            }
        case COMMAND_OPTION_RAM:
            {
            pbDisplay = psmd->smb.pAddress;

            if (ReadPhysical (hDevice, pbDisplay, &pa))
                {
                pbDisplay = (PBYTE) pa.LowPart;
                }
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
    if (!psmd->smb.dBytes)
        {
        _printf (awTableNoData, psmd->smb.pAddress);
        }
    else
        {
        switch (dOptions & COMMAND_OPTION_MODE)
            {
            default:
                {
                pwTableData = awTableDataByte;
                pwTableBar  = awTableBarByte;
                dGroup      = BYTE_;
                break;
                }
            case COMMAND_OPTION_WORD:
                {
                pwTableData = awTableDataWord;
                pwTableBar  = awTableBarWord;
                dGroup      = WORD_;
                break;
                }
            case COMMAND_OPTION_DWORD:
                {
                pwTableData = awTableDataDword;
                pwTableBar  = awTableBarDword;
                dGroup      = DWORD_;
                break;
                }
            case COMMAND_OPTION_QWORD:
                {
                pwTableData = awTableDataQword;
                pwTableBar  = awTableBarQword;
                dGroup      = QWORD_;
                break;
                }
            }
        for (i = 0; i < psmd->smb.dBytes; i++)
            {
            if (psmd->awData [i] & SPY_MEMORY_DATA_VALID) n++;
            }
        _printf (awTableCaption,
                 psmd->smb.pbAddress,
                 psmd->smb.pbAddress + psmd->smb.dBytes - 1,
                 n, (n == 1 ? awByte : awBytes));

        lstrcpy (dr.pwAddress, awTableAddress);

        for (j = 0; j < 16; j++)
            {
            k = (((j / dGroup) + 1) * dGroup) - ((j % dGroup) + 1);

            dData = (k % dGroup == 0
                     ? ((DWORD) pbDisplay & 0x0F) + k
                     : 0);

            _sprintf (dr.apwHex  [j], awTableHex2, dData);

            dData = (k % dGroup == 0
                     ? ((DWORD) pbDisplay + k) & 0x0F
                     : (k % dGroup == 1
                        ? (((DWORD) pbDisplay & 0x0F) + k - 1) >> 4
                        : 0));

            _sprintf (dr.apwText [j], awTableHex1, dData);
            }
        _vsprintf (dr.awBuffer, pwTableData, dr.pArguments);
        _printf (dr.awBuffer);
        _printf (pwTableBar);
        }
    for (i = 0; i < psmd->smb.dBytes; i += j)
        {
        _sprintf (dr.pwAddress, awTableHex8, pbDisplay + i);

        for (j = 0; j < 16; j++)
            {
            k = (((j / dGroup) + 1) * dGroup) - ((j % dGroup) + 1);

            if ((i+k < psmd->smb.dBytes) &&
                (psmd->awData [i+k] & SPY_MEMORY_DATA_VALID))
                {
                dData = psmd->awData [i+k] & SPY_MEMORY_DATA_BYTE;
                _sprintf (dr.apwHex  [j], awTableHex2, dData);

                if ((dData < 0x20) || (dData == 0x7F)) dData = '.';
                dr.apwText [j] [0] = (WORD) dData;
                dr.apwText [j] [1] = 0;
                }
            else
                {
                lstrcpy (dr.apwHex  [j], awTableSpace2);
                lstrcpy (dr.apwText [j], awTableSpace1);
                }
            }
        _vsprintf (dr.awBuffer, pwTableData, dr.pArguments);
        _printf   (awString, dr.awBuffer);

        if ((dOptions & COMMAND_OPTION_RAM)
            &&
            (((DWORD) (psmd->smb.pbAddress+i  ) & X86_PAGE_MASK) != 
             ((DWORD) (psmd->smb.pbAddress+i+j) & X86_PAGE_MASK)))
            {
            if (ReadPhysical (hDevice, psmd->smb.pbAddress+i+j,
                              &pa))
                {
                pbDisplay = (PBYTE) pa.LowPart - (i+j);
                }
            else
                {
                pbDisplay = psmd->smb.pAddress;
                }
            }
        }
    if (psmd->smb.dBytes) _printf (awNewLine);
    return n;
    }

// =================================================================
// DISPLAY OS INFO
// =================================================================

BOOL WINAPI DisplayOsInfo (HANDLE hDevice)
    {
    SPY_OS_INFO soi;
    PWORD       pwProductType;
    BOOL        fOk = FALSE;

    if (ReadBinary (hDevice, SPY_IO_OS_INFO,
                    &soi, SPY_OS_INFO_))
        {
        switch (soi.dProductType)
            {
            case NtProductInvalid:
                {
                pwProductType = L"<invalid>";
                break;
                }
            case NtProductWinNt:
                {
                pwProductType = L"Windows NT Workstation";
                break;
                }
            case NtProductLanManNt:
                {
                pwProductType = L"Windows NT Domain Controller";
                break;
                }
            case NtProductServer:
                {
                pwProductType = L"Windows NT Server";
                break;
                }
            default:
                {
                pwProductType = L"<unknown>";
                break;
                }
            }
        _printf (L"%s\r\n"
                 L"Memory page size         : %lu bytes\r\n"
                 L"Memory page shift        : %lu bits\r\n"
                 L"Memory PTI  shift        : %lu bits\r\n"
                 L"Memory PDI  shift        : %lu bits\r\n"
                 L"Memory page mask         : 0x%08lX\r\n"
                 L"Memory PTI  mask         : 0x%08lX\r\n"
                 L"Memory PDI  mask         : 0x%08lX\r\n"
                 L"Memory PTE  array        : 0x%08lX\r\n"
                 L"Memory PDE  array        : 0x%08lX\r\n"
                 L"\r\n"
                 L"Lowest user address      : 0x%08lX\r\n"
                 L"Thread environment block : 0x%08lX\r\n"
                 L"Highest user address     : 0x%08lX\r\n"
                 L"User probe address       : 0x%08lX\r\n"
                 L"System range start       : 0x%08lX\r\n"
                 L"Lowest system address    : 0x%08lX\r\n"
                 L"Shared user data         : 0x%08lX\r\n"
                 L"Processor control region : 0x%08lX\r\n"
                 L"Processor control block  : 0x%08lX\r\n"
                 L"\r\n"
                 L"Global flag              : 0x%08lX\r\n"
                 L"i386 machine type        : %lu\r\n"
                 L"Number of processors     : %lu\r\n"
                 L"Product type             : %s (%lu)\r\n"
                 L"Version & Build number   : %lu.%02lu.%lu\r\n"
                 L"System root              : \"%s\"\r\n",
                 awOsInfoCaption,
                 soi.dPageSize,
                 soi.dPageShift, soi.dPtiShift, soi.dPdiShift,
                 soi.dPageMask,  soi.dPtiMask,  soi.dPdiMask,
                 soi.PteArray,   soi.PdeArray,
                 soi.pLowestUserAddress,
                 soi.pThreadEnvironmentBlock,
                 soi.pHighestUserAddress,
                 soi.pUserProbeAddress,
                 soi.pSystemRangeStart,
                 soi.pLowestSystemAddress,
                 soi.pSharedUserData,
                 soi.pProcessorControlRegion,
                 soi.pProcessorControlBlock,
                 soi.dGlobalFlag,
                 soi.dI386MachineType,
                 soi.dNumberProcessors,
                 pwProductType, soi.dProductType,
                 soi.dNtMajorVersion, soi.dNtMinorVersion,
                 soi.dBuildNumber,
                 soi.awNtSystemRoot);

        fOk = TRUE;
        }
    return fOk;
    }

// =================================================================
// DISPLAY CPU INFO
// =================================================================

BOOL WINAPI DisplaySegmentInfo (PSPY_SEGMENT pss,
                                PWORD        pwSegment)
    {
    PWORD pwType;
    BOOL  fOk = FALSE;

    if (pss->fOk && pss->Descriptor.P
        &&
        ((pwType = (pss->Descriptor.S
                    ? apwTypeApplication [pss->Descriptor.Type]
                    : apwTypeSystem      [pss->Descriptor.Type]))
         != NULL))
        {
        _printf (awGdtInfoSegment, pwSegment,
                 pss->Selector.wValue,
                 pss->pBase, pss->dLimit,
                 pss->Descriptor.DPL,
                 (pwType != NULL ? pwType : awUndefined));

        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI DisplaySelectorInfo (HANDLE hDevice,
                                 DWORD  dSelector,
                                 PWORD  pwSegment)
    {
    SPY_SEGMENT ss;
    WORD        awId [N_SEGMENT];
    BOOL        fOk = FALSE;

    if (ReadSegment (hDevice, dSelector, &ss))
        {
        if (pwSegment != NULL)
            {
            lstrcpyn (awId, pwSegment, N_SEGMENT);
            }
        else
            {
            _sprintf (awId, awSegment,
                      dSelector >> X86_SELECTOR_SHIFT);
            }
        fOk = DisplaySegmentInfo (&ss, awId);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI DisplayInterruptInfo (HANDLE hDevice,
                                  DWORD  dInterrupt)
    {
    SPY_INTERRUPT si;
    PWORD         pwType;
    BOOL          fOk = FALSE;

    if (IoControl (hDevice,     SPY_IO_INTERRUPT,
                   &dInterrupt, DWORD_,
                   &si,         SPY_INTERRUPT_)
        &&
        si.fOk && si.Gate.P && (!si.Gate.S)
        &&
        ((pwType = apwTypeSystem [si.Gate.Type]) != NULL))
        {
        if (si.Gate.Type == X86_DESCRIPTOR_SYS_TASK)
            {
            _printf (awIdtInfoSegment, dInterrupt,
                     si.Segment.Selector.wValue,
                     si.Segment.pBase, si.Segment.dLimit,
                     pwType);
            }
        else
            {
            _printf (awIdtInfoPointer, dInterrupt,
                     si.Segment.Selector.wValue, si.pOffset,
                     si.Segment.pBase, si.Segment.dLimit,
                     pwType);
            }
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI DisplayCpuInfo (HANDLE hDevice)
    {
    SPY_CPU_INFO sci;
    WORD         wSelector;
    BOOL         fOk = FALSE;

    if (ReadCpuInfo (hDevice, &sci))
        {
        _printf (awCpuInfoCaption);
        _printf (awCpuInfoUser);

        __asm mov wSelector, cs
        DisplaySelectorInfo (hDevice, wSelector, awSegmentCS);

        __asm mov wSelector, ds
        DisplaySelectorInfo (hDevice, wSelector, awSegmentDS);

        __asm mov wSelector, es
        DisplaySelectorInfo (hDevice, wSelector, awSegmentES);

        __asm mov wSelector, fs
        DisplaySelectorInfo (hDevice, wSelector, awSegmentFS);

        __asm mov wSelector, gs
        DisplaySelectorInfo (hDevice, wSelector, awSegmentGS);

        __asm mov wSelector, ss
        DisplaySelectorInfo (hDevice, wSelector, awSegmentSS);

        __asm str wSelector
        DisplaySelectorInfo (hDevice, wSelector, awSegmentTSS);

        _printf (awCpuInfoKernel);

        DisplaySegmentInfo (&sci.cs,  awSegmentCS);
        DisplaySegmentInfo (&sci.ds,  awSegmentDS);
        DisplaySegmentInfo (&sci.es,  awSegmentES);
        DisplaySegmentInfo (&sci.fs,  awSegmentFS);
        DisplaySegmentInfo (&sci.gs,  awSegmentGS);
        DisplaySegmentInfo (&sci.ss,  awSegmentSS);
        DisplaySegmentInfo (&sci.tss, awSegmentTSS);

        _printf (awCpuInfoOther,
                 sci.idt.wLimit, sci.idt.pDescriptors,
                 sci.gdt.wLimit, sci.gdt.pDescriptors,
                 sci.ldt.wValue,
                 sci.cr0, sci.cr2, sci.cr3);

        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayGdtInfo (HANDLE hDevice)
    {
    SPY_CPU_INFO sci;
    DWORD        dSelector;
    DWORD        n = 0;

    if (ReadCpuInfo (hDevice, &sci))
        {
        _printf (awGdtInfoCaption);

        dSelector = 0;

        while (dSelector <= sci.gdt.wLimit)
            {
            if (DisplaySelectorInfo (hDevice, dSelector, NULL)) n++;
            dSelector += (1 << X86_SELECTOR_SHIFT);
            }
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayIdtInfo (HANDLE hDevice)
    {
    SPY_CPU_INFO sci;
    DWORD        dInterrupt;
    DWORD        n = 0;

    if (ReadCpuInfo (hDevice, &sci))
        {
        _printf (awIdtInfoCaption);

        dInterrupt = 0;

        while (dInterrupt << X86_SELECTOR_SHIFT <= sci.idt.wLimit)
            {
            if (DisplayInterruptInfo (hDevice, dInterrupt++)) n++;
            }
        }
    return n;
    }

// =================================================================
// DISPLAY MEMORY INFO
// =================================================================

DWORD WINAPI DisplayMemoryBlocks (HANDLE hDevice)
    {
    SPY_PAGE_ENTRY spe;
    PBYTE          pbPage, pbBase;
    DWORD          dBlock, dPresent, dTotal;
    DWORD          n = 0;

    pbPage   = 0;
    pbBase   = INVALID_ADDRESS;
    dBlock   = 0;
    dPresent = 0;
    dTotal   = 0;

    n += _printf (L"\r\nContiguous memory blocks:"
                  L"\r\n-------------------------\r\n\r\n");

    do  {
        if (!IoControl (hDevice, SPY_IO_PAGE_ENTRY,
                        &pbPage, PVOID_,
                        &spe,    SPY_PAGE_ENTRY_))
            {
            n += _printf (L" !!! Device I/O error !!!\r\n");
            break;
            }
        if (spe.fPresent)
            {
            dPresent += spe.dSize;
            }
        if (spe.pe.dValue)
            {
            dTotal += spe.dSize;

            if (pbBase == INVALID_ADDRESS)
                {
                n += _printf (L"%5lu : 0x%08lX ->",
                              ++dBlock, pbPage);

                pbBase = pbPage;
                }
            }
        else
            {
            if (pbBase != INVALID_ADDRESS)
                {
                n += _printf (L" 0x%08lX (0x%08lX bytes)\r\n",
                              pbPage-1, pbPage-pbBase);

                pbBase = INVALID_ADDRESS;
                }
            }
        }
    while (pbPage += spe.dSize);

    if (pbBase != INVALID_ADDRESS)
        {
        n += _printf (L"0x%08lX\r\n", pbPage-1);
        }
    n += _printf (L"\r\n"
                  L" Present bytes: 0x%08lX\r\n"
                  L" Total   bytes: 0x%08lX\r\n",
                  dPresent, dTotal);
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI DisplayMemoryData (HANDLE hDevice,
                                PVOID  pAddress,
                                DWORD  dBytes,
                                DWORD  dOptions)
    {
    PSPY_MEMORY_DATA psmd;
    DWORD            n = 0;

    if ((psmd = MemoryRead (hDevice, pAddress, dBytes)) != NULL)
        {
        n = MemoryDisplay (hDevice, psmd, dOptions);
        MemoryRelease (psmd);
        }
    return n;
    }

// =================================================================
// COMMAND PARSER
// =================================================================

BOOL WINAPI CommandNumber (PWORD pwData,
                           PVOID pData)
    {
    DWORD dBase, dData, dBackup, i;
    WORD  wData = 0;

    for (dData = i = 0; pwData [i] == '0'; i++);

    if (CHAR_LOWER (pwData [i]) == 'x')
        {
        dBase = 16;
        while (pwData [++i] == '0');
        }
    else
        {
        dBase = 10;
        }
    while (wData = CHAR_LOWER (pwData [i++]))
        {
        dBackup = dData;

        if ((wData >= '0') && (wData <= '9'))
            {
            dData *= dBase;
            dData += (wData - '0');
            }
        else
            {
            if ((dBase > 10) &&
                (wData >= 'a') && (wData <= 'a' + (dBase-10-1)))
                {
                dData *= dBase;
                dData += (wData - 'a' + 10);
                }
            else
                {
                break;
                }
            }
        if (dData <= dBackup)
            {
            dData = dBackup;
            break;
            }
        }
    if (pData != NULL) *(PDWORD) pData = dData;
    return (!wData);
    }

// -----------------------------------------------------------------

BOOL WINAPI CommandOption (WORD   wOptionId,
                           BOOL   fOptionStatus,
                           PDWORD pdOptions)
    {
    DWORD dMask, dOption;
    BOOL  fOk = TRUE;

    switch (wOptionId)
        {
        case 'z':
            {
            dMask   = COMMAND_OPTION_ADDRESS;
            dOption = COMMAND_OPTION_ZERO;
            break;
            }
        case 'r':
            {
            dMask   = COMMAND_OPTION_ADDRESS;
            dOption = COMMAND_OPTION_RAM;
            break;
            }
        case 'w':
            {
            dMask   = COMMAND_OPTION_MODE;
            dOption = COMMAND_OPTION_WORD;
            break;
            }
        case 'd':
            {
            dMask   = COMMAND_OPTION_MODE;
            dOption = COMMAND_OPTION_DWORD;
            break;
            }
        case 'q':
            {
            dMask   = COMMAND_OPTION_MODE;
            dOption = COMMAND_OPTION_QWORD;
            break;
            }
        case 't':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_TEB;
            break;
            }
        case 'f':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_FS;
            break;
            }
        case 'u':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_USER;
            break;
            }
        case 'k':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_KERNEL;
            break;
            }
        case 'h':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_HANDLE;
            break;
            }
        case 'a':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_ADD;
            break;
            }
        case 's':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_SUBTRACT;
            break;
            }
        case 'p':
            {
            dMask   = COMMAND_OPTION_BASE;
            dOption = COMMAND_OPTION_POINTER;
            break;
            }
        case 'o':
            {
            dMask   = COMMAND_OPTION_OS;
            dOption = COMMAND_OPTION_OS;
            break;
            }
        case 'c':
            {
            dMask   = COMMAND_OPTION_CPU;
            dOption = COMMAND_OPTION_CPU;
            break;
            }
        case 'g':
            {
            dMask   = COMMAND_OPTION_GDT;
            dOption = COMMAND_OPTION_GDT;
            break;
            }
        case 'i':
            {
            dMask   = COMMAND_OPTION_IDT;
            dOption = COMMAND_OPTION_IDT;
            break;
            }
        case 'b':
            {
            dMask   = COMMAND_OPTION_BLOCKS;
            dOption = COMMAND_OPTION_BLOCKS;
            break;
            }
        case 'x':
            {
            dMask   = COMMAND_OPTION_EXECUTE;
            dOption = COMMAND_OPTION_EXECUTE;
            break;
            }
        default:
            {
            fOk = FALSE;
            break;
            }
        }
    if (fOk)
        {
        *pdOptions &= ~dMask;
        *pdOptions |= (fOptionStatus ? dOption
                                     : COMMAND_OPTION_NONE);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI CommandBase (HANDLE hDevice,
                         PPVOID ppBase,
                         DWORD  dOffset,
                         DWORD  dOptions)
    {
    SPY_SEGMENT     ss;
    SPY_CPU_INFO    sci;
    SPY_HANDLE_INFO shi;
    WORD            wSelector;
    HANDLE          hObject;
    PBYTE           pbData = *ppBase;
    BOOL            fOk    = FALSE;

    switch (dOptions & COMMAND_OPTION_BASE)
        {
        case COMMAND_OPTION_TEB:
            {
            __asm mov wSelector, fs

            if (ReadSegment (hDevice, wSelector, &ss))
                {
                pbData = (PBYTE) ss.pBase + dOffset;
                fOk    = TRUE;
                }
            break;
            }
        case COMMAND_OPTION_FS:
            {
            if (ReadCpuInfo (hDevice, &sci) &&
                ReadSegment (hDevice, sci.fs.Selector.wValue, &ss))
                {
                pbData = (PBYTE) ss.pBase + dOffset;
                fOk    = TRUE;
                }
            break;
            }
        case COMMAND_OPTION_USER:
            {
            __asm mov wSelector, fs

            if (ReadSegment (hDevice, wSelector, &ss))
                {
                fOk = MemoryPointer (hDevice,
                                     (PBYTE) ss.pBase + dOffset,
                                     &pbData);
                }
            break;
            }
        case COMMAND_OPTION_KERNEL:
            {
            if (ReadCpuInfo (hDevice, &sci) &&
                ReadSegment (hDevice, sci.fs.Selector.wValue, &ss))
                {
                fOk = MemoryPointer (hDevice,
                                     (PBYTE) ss.pBase + dOffset,
                                     &pbData);
                }
            break;
            }
        case COMMAND_OPTION_HANDLE:
            {
            hObject = (HANDLE) dOffset;

            if (IoControl (hDevice,  SPY_IO_HANDLE_INFO,
                           &hObject, HANDLE_,
                           &shi,     SPY_HANDLE_INFO_))
                {
                pbData = shi.pObjectBody;
                fOk    = TRUE;
                }
            break;
            }
        case COMMAND_OPTION_ADD:
            {
            pbData += dOffset;
            fOk     = TRUE;
            break;
            }
        case COMMAND_OPTION_SUBTRACT:
            {
            pbData -= dOffset;
            fOk     = TRUE;
            break;
            }
        case COMMAND_OPTION_POINTER:
            {
            fOk = MemoryPointer (hDevice,
                                 pbData + dOffset,
                                 &pbData);
            break;
            }
        default:
            {
            pbData = (PBYTE) dOffset;
            fOk    = TRUE;
            break;
            }
        }
    *ppBase = pbData;
    return fOk;
    }

// -----------------------------------------------------------------

DWORD WINAPI CommandExecute (HANDLE hDevice,
                             PPVOID ppBase,
                             DWORD  dOffset,
                             DWORD  dOptions,
                             DWORD  dBytes)
    {
    DWORD n = 0;

    if (dOptions & COMMAND_OPTION_OS)       // +o
        DisplayOsInfo (hDevice);

    if (dOptions & COMMAND_OPTION_CPU)      // +c
        DisplayCpuInfo (hDevice);

    if (dOptions & COMMAND_OPTION_GDT)      // +g
        DisplayGdtInfo (hDevice);

    if (dOptions & COMMAND_OPTION_IDT)      // +i
        DisplayIdtInfo (hDevice);

    if (dOptions & COMMAND_OPTION_BLOCKS)   // +b
        DisplayMemoryBlocks (hDevice);

    if (CommandBase (hDevice, ppBase, dOffset, dOptions) && dBytes)
        {
        n = DisplayMemoryData (hDevice, *ppBase, dBytes, dOptions);
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI CommandParse (HANDLE hDevice,
                           PPWORD ppwItems,
                           DWORD  dItems,
                           BOOL   fTest,
                           PDWORD pdOptions)
    {
    BOOL    fDump, fOptionOn, fOptionOff, fOptionStatus;
    PWORD   pwPrefix;
    PVOID   pBase;
    HMODULE hm;
    DWORD   dOffset, dData, dBackup, i, j;
    DWORD   n = 0;

    fDump    = FALSE;
    pwPrefix = awNewLine;
    pBase    = NULL;
    dOffset  = 0;
    dData    = DISPLAY_SIZE_DEFAULT;

    for (i = 0; i < dItems; i++)
        {
        fOptionOn  = FALSE;
        fOptionOff = FALSE;

        switch (ppwItems [i] [0])
            {
            case COMMAND_OPTION_ON:
                {
                fOptionOn     = TRUE;
                fOptionStatus = TRUE;
                break;
                }
            case COMMAND_OPTION_OFF:
                {
                fOptionOff    = TRUE;
                fOptionStatus = FALSE;
                break;
                }
            case COMMAND_OPTION_NUMBER:
                {
                dBackup = dData;

                if (!CommandNumber (ppwItems [i] + 1, &dData))
                    {
                    if (fTest)
                        {
                        _printf (awInvalidArgument,
                                 pwPrefix,
                                 ppwItems [i]);

                        pwPrefix = awNull;
                        }
                    dData = dBackup;
                    }
                break;
                }
            case COMMAND_OPTION_LOAD:
                {
                hm = LoadLibraryEx
                         (ppwItems [i] + 1, NULL,
                          (*pdOptions & COMMAND_OPTION_EXECUTE
                           ? 0 : DONT_RESOLVE_DLL_REFERENCES));
                if (fTest)
                    {
                    if (hm == NULL)
                        {
                        _printf (awInvalidModule,
                                 pwPrefix,
                                 GetLastError (),
                                 ppwItems [i] + 1);
                        }
                    else
                        {
                        FreeLibrary (hm);
                        }
                    }
                else
                    {
                    if (hm != NULL)
                        {
                        _printf (awLoadLibrary,
                                 ppwItems [i] + 1, hm);
                        }
                    }
                break;
                }
            default:
                {
                if (!CommandNumber (ppwItems [i], &dOffset))
                    {
                    if (fTest)
                        {
                        _printf (awInvalidArgument,
                                 pwPrefix,
                                 ppwItems [i]);

                        pwPrefix = awNull;
                        }
                    }
                else
                    {
                    if (fTest)
                        {
                        n += dData;
                        }
                    else
                        {
                        n += CommandExecute (hDevice, &pBase,
                                             dOffset, *pdOptions,
                                             dData);
                        fDump = TRUE;
                        }
                    }
                break;
                }
            }
        if (fOptionOn || fOptionOff)
            {
            for (j = 1; ppwItems [i] [j]; j++)
                {
                if (!CommandOption
                         (CHAR_LOWER (ppwItems [i] [j]),
                          fOptionStatus, pdOptions))
                    {
                    if (fTest)
                        {
                        _printf (awInvalidOption,
                                 pwPrefix,
                                 ppwItems [i] [0],
                                 ppwItems [i] [j]);

                        pwPrefix = awNull;
                        }
                    }
                }
            }
        }
    if (fTest)
        {
        if (!((*pdOptions & COMMAND_OPTION_INFO) || n))
            {
            _printf (awInvalidCommand, pwPrefix);
            }
        }
    else
        {
        if ((*pdOptions & COMMAND_OPTION_INFO) && (!fDump))
            {
            n += CommandExecute (hDevice, &pBase, 0, *pdOptions, 0);
            }
        }
    return n;
    }

// =================================================================
// SPY DEVICE MANAGEMENT
// =================================================================

void WINAPI Execute (PPWORD ppwArguments,
                     DWORD  dArguments)
    {
    SPY_VERSION_INFO svi;
    DWORD            dOptions, dRequest, dReceive;
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

        hDevice = CreateFile (awSpyPath, GENERIC_READ,
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
        if (ReadBinary (hDevice, SPY_IO_VERSION_INFO,
                        &svi, SPY_VERSION_INFO_))
            {
            _printf (L"\r\n%s V%lu.%02lu ready\r\n",
                     svi.awName,
                     svi.dVersion / 100, svi.dVersion % 100);
            }
        dOptions = COMMAND_OPTION_NONE;
        dRequest = CommandParse (hDevice, ppwArguments, dArguments,
                                 TRUE, &dOptions);

        dOptions = COMMAND_OPTION_NONE;
        dReceive = CommandParse (hDevice, ppwArguments, dArguments,
                                 FALSE, &dOptions);
        if (dRequest)
            {
            _printf (awSummary,
                     dRequest, (dRequest == 1 ? awByte : awBytes),
                     dReceive, (dReceive == 1 ? awByte : awBytes));
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

DWORD Main (DWORD argc, PWORD *argv, PWORD *argp)
    {
    _printf (atAbout);

    if (argc < 2)
        {
        _printf (atUsage,    awArguments);
        _printf (awMoreInfo, DISPLAY_SIZE_DEFAULT);
        _printf (awOptions);
        _printf (awExamples);
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
