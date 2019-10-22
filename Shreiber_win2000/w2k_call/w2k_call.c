
// __________________________________________________________
//
//                         w2k_call.c
//           SBS Windows 2000 Call Interface V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#define  _W2K_CALL_DLL_
#include "w2k_call.h"

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
// GLOBAL VARIABLES
// =================================================================

HINSTANCE               ghInstance          = NULL;
HANDLE                  ghDevice            = INVALID_HANDLE_VALUE;
WORD                    awDriver [MAX_PATH] = L"";
DWORD                   gdStatus            = W2K_SYMBOLS_OK;
PIMG_TABLE              gpit                = NULL;
PCRITICAL_SECTION       gpcs                = NULL;
CRITICAL_SECTION        gcs;

// =================================================================
// GLOBAL STRINGS
// =================================================================

BYTE abKernel     [] = "ntoskrnl.exe";

WORD awSpyFile    [] = SW(DRV_FILENAME);
WORD awSpyDevice  [] = SW(DRV_MODULE);
WORD awSpyDisplay [] = SW(DRV_NAME);
WORD awSpyPath    [] = SW(DRV_PATH);

// =================================================================
// PE FILE PARSER
// =================================================================

PSPY_MODULE_INFO WINAPI w2kPeInfo (PBYTE pbModule)
    {
    PBYTE            pbModule1;
    PSPY_MODULE_INFO psmi = NULL;

    pbModule1 = (pbModule != NULL ? pbModule : abKernel);

    if (((psmi = w2kMemoryCreate (SPY_MODULE_INFO_)) != NULL)
        &&
        (!w2kSpyControl (SPY_IO_MODULE_INFO,
                         &pbModule1, PVOID_,
                         psmi,       SPY_MODULE_INFO_)))
        {
        psmi = w2kMemoryDestroy (psmi);
        }
    return psmi;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kPeBase (PBYTE pbModule)
    {
    PSPY_MODULE_INFO psmi;
    PVOID            pBase = NULL;

    if ((psmi = w2kPeInfo (pbModule)) != NULL)
        {
        pBase = psmi->pBase;
        w2kMemoryDestroy (psmi);
        }
    return pBase;
    }

// -----------------------------------------------------------------

PIMAGE_NT_HEADERS WINAPI w2kPeHeader (PBYTE  pbModule,
                                      PPVOID ppBase)
    {
    PVOID             pBase = NULL;
    PIMAGE_NT_HEADERS pinh  = NULL;

    if (((pBase = w2kPeBase       (pbModule         )) != NULL) &&
        ((pinh  = w2kMemoryCreate (IMAGE_NT_HEADERS_)) != NULL)
        &&
        (!w2kSpyControl (SPY_IO_PE_HEADER,
                         &pBase, PVOID_,
                         pinh,   IMAGE_NT_HEADERS_)))
        {
        pinh = w2kMemoryDestroy (pinh);
        }
    if (ppBase != NULL) *ppBase = pBase;
    return pinh;
    }

// -----------------------------------------------------------------

PIMAGE_EXPORT_DIRECTORY WINAPI w2kPeExport (PBYTE  pbModule,
                                            PPVOID ppBase)
    {
    PIMAGE_NT_HEADERS       pinh;
    DWORD                   dSize;
    PVOID                   pBase = NULL;
    PIMAGE_EXPORT_DIRECTORY pied  = NULL;

    if ((pinh = w2kPeHeader (pbModule, &pBase)) != NULL)
        {
        dSize = pinh->OptionalHeader
                .DataDirectory [IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

        pinh  = w2kMemoryDestroy (pinh);

        if ((dSize >= IMAGE_EXPORT_DIRECTORY_) &&
            ((pied  = w2kMemoryCreate (dSize)) != NULL)
            &&
            (!w2kSpyControl (SPY_IO_PE_EXPORT,
                             &pBase, PVOID_,
                             pied,   dSize)))
            {
            pied = w2kMemoryDestroy (pied);
            }
        }
    if (ppBase != NULL) *ppBase = pBase;
    return pied;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kPeCheck (PBYTE pbModule,
                        DWORD dTimeStamp,
                        DWORD dCheckSum)
    {
    PIMAGE_NT_HEADERS pinh;
    BOOL              fOk = FALSE;

    if ((pinh = w2kPeHeader (pbModule, NULL)) != NULL)
        {
        fOk = (pinh->FileHeader.TimeDateStamp == dTimeStamp) &&
              (pinh->OptionalHeader.CheckSum  == dCheckSum);

        w2kMemoryDestroy (pinh);
        }
    return fOk;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kPeEnumerate (PBYTE            pbModule,
                             PPVOID           ppBase,
                             W2K_PE_ENUMERATE Callback)
    {
    PIMAGE_EXPORT_DIRECTORY pied;
    PVOID                   pAddress;
    PDWORD                  pdNames, pdFunctions;
    PWORD                   pwOrdinals;
    DWORD                   dOrdinal;
    PVOID                   pBase = NULL;
    DWORD                   n     = 0;

    if ((pied = w2kPeExport (pbModule, &pBase)) != NULL)
        {
        pdNames     = PTR_ADD (pied, pied->AddressOfNames);
        pdFunctions = PTR_ADD (pied, pied->AddressOfFunctions);
        pwOrdinals  = PTR_ADD (pied, pied->AddressOfNameOrdinals);

        while (n < pied->NumberOfNames)
            {
            pAddress = (pwOrdinals [n] < pied->NumberOfFunctions
                        ? (PVOID) pdFunctions [pwOrdinals [n]]
                        : NULL);

            dOrdinal = pied->Base + pwOrdinals [n];

            if ((Callback != NULL)
                &&
                (!Callback (PTR_ADD (pied, pied->Name),  pBase,
                            PTR_ADD (pied, pdNames [n]), pAddress,
                            dOrdinal, n, pied->NumberOfNames)))
                {
                break;
                }
            n++;
            }
        w2kMemoryDestroy (pied);
        }
    if (ppBase != NULL) *ppBase = pBase;
    return n;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kPeSymbol (PBYTE pbSymbol)
    {
    PVOID pAddress = NULL;

    if ((pbSymbol != NULL)
        &&
        (!w2kSpyControl (SPY_IO_PE_SYMBOL,
                         &pbSymbol, PVOID_,
                         &pAddress, PVOID_)))
        {
        pAddress = NULL;
        }
    return pAddress;
    }

// =================================================================
// SYMBOL TABLE MANAGEMENT
// =================================================================

PIMG_TABLE WINAPI w2kSymbolsLoad (PBYTE  pbModule,
                                  PDWORD pdStatus)
    {
    PVOID      pBase;
    DWORD      dStatus = W2K_SYMBOLS_UNDEFINED;
    PIMG_TABLE pit     = NULL;

    if ((pBase = imgModuleBaseA (pbModule)) == NULL)
        {
        dStatus = W2K_SYMBOLS_MODULE_ERROR;
        }
    else
        {
        if ((pit = imgTableLoadA (pbModule, pBase)) == NULL)
            {
            dStatus = W2K_SYMBOLS_LOAD_ERROR;
            }
        else
            {
            if (!w2kPeCheck (pbModule, pit->dTimeStamp,
                                       pit->dCheckSum))
                {
                dStatus = W2K_SYMBOLS_VERSION_ERROR;
                pit     = imgMemoryDestroy (pit);
                }
            else
                {
                dStatus = W2K_SYMBOLS_OK;
                }
            }
        }
    if (pdStatus != NULL) *pdStatus = dStatus;
    return pit;
    }

// -----------------------------------------------------------------

PIMG_TABLE WINAPI w2kSymbolsGlobal (PDWORD pdStatus)
    {
    DWORD      dStatus = W2K_SYMBOLS_UNDEFINED;
    PIMG_TABLE pit     = NULL;

    w2kSpyLock ();

    if ((gdStatus == W2K_SYMBOLS_OK) && (gpit == NULL))
        {
        gpit = w2kSymbolsLoad (NULL, &gdStatus);
        }
    dStatus = gdStatus;
    pit     = gpit;

    w2kSpyUnlock ();

    if (pdStatus != NULL) *pdStatus = dStatus;
    return pit;
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kSymbolsStatus (VOID)
    {
    DWORD dStatus = W2K_SYMBOLS_UNDEFINED;

    w2kSymbolsGlobal (&dStatus);
    return dStatus;
    }

// -----------------------------------------------------------------

VOID WINAPI w2kSymbolsReset (VOID)
    {
    w2kSpyLock ();

    gpit     = imgMemoryDestroy (gpit);
    gdStatus = W2K_SYMBOLS_OK;

    w2kSpyUnlock ();
    return;
    }

// =================================================================
// SPY DEVICE MANAGEMENT
// =================================================================

BOOL WINAPI w2kSpyLock (VOID)
    {
    BOOL fOk = FALSE;

    if (gpcs != NULL)
        {
        EnterCriticalSection (gpcs);
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kSpyUnlock (VOID)
    {
    BOOL fOk = FALSE;

    if (gpcs != NULL)
        {
        LeaveCriticalSection (gpcs);
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kSpyStartup (BOOL      fUnload,
                           HINSTANCE hInstance)
    {
    HINSTANCE hInstance1;
    SC_HANDLE hControl;
    BOOL      fOk = FALSE;

    w2kSpyLock ();

    hInstance1 = (hInstance != NULL ? hInstance : ghInstance);

    if ((ghDevice == INVALID_HANDLE_VALUE) &&
        w2kFilePath (hInstance1, awSpyFile, awDriver, MAX_PATH)
        &&
        ((hControl = w2kServiceLoad (awSpyDevice, awSpyDisplay,
                                     awDriver, TRUE))
         != NULL))
        {
        ghDevice = CreateFile (awSpyPath,
                               GENERIC_READ    | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);

        if ((ghDevice == INVALID_HANDLE_VALUE) && fUnload)
            {
            w2kServiceUnload (awSpyDevice, hControl);
            }
        else
            {
            w2kServiceDisconnect (hControl);
            }
        }
    fOk = (ghDevice != INVALID_HANDLE_VALUE);

    w2kSpyUnlock ();
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kSpyCleanup (BOOL fUnload)
    {
    BOOL fOk = FALSE;

    w2kSpyLock ();

    if (ghDevice != INVALID_HANDLE_VALUE)
        {
        CloseHandle (ghDevice);
        ghDevice = INVALID_HANDLE_VALUE;
        }
    if (fUnload)
        {
        w2kServiceUnload (awSpyDevice, NULL);
        }
    w2kSpyUnlock ();
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kSpyControl (DWORD dCode,
                           PVOID pInput,
                           DWORD dInput,
                           PVOID pOutput,
                           DWORD dOutput)
    {
    DWORD dInfo = 0;
    BOOL  fOk   = FALSE;

    SetLastError (ERROR_INVALID_PARAMETER);

    if (((pInput  != NULL) || (!dInput )) &&
        ((pOutput != NULL) || (!dOutput)))
        {
        if (w2kSpyStartup (FALSE, NULL))
            {
            if (DeviceIoControl (ghDevice, dCode,
                                 pInput,   dInput,
                                 pOutput,  dOutput,
                                 &dInfo,   NULL))
                {
                if (dInfo == dOutput)
                    {
                    SetLastError (ERROR_SUCCESS);
                    fOk = TRUE;
                    }
                else
                    {
                    SetLastError (ERROR_DATATYPE_MISMATCH);
                    }
                }
            }
        else
            {
            SetLastError (ERROR_GEN_FAILURE);
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kSpyRead (PVOID pBuffer,
                        PVOID pAddress,
                        DWORD dBytes)
    {
    SPY_MEMORY_BLOCK smb;
    BOOL             fOk = FALSE;

    if ((pBuffer != NULL) && (pAddress != NULL) && dBytes)
        {
        ZeroMemory (pBuffer, dBytes);

        smb.pAddress = pAddress;
        smb.dBytes   = dBytes;

        fOk = w2kSpyControl (SPY_IO_MEMORY_BLOCK,
                             &smb,    SPY_MEMORY_BLOCK_,
                             pBuffer, dBytes);
        }
    return fOk;
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kSpyClone (PVOID pAddress,
                          DWORD dBytes)
    {
    PVOID pBuffer = NULL;

    if ((pAddress != NULL) && dBytes &&
        ((pBuffer = w2kMemoryCreate (dBytes)) != NULL) &&
        (!w2kSpyRead (pBuffer, pAddress, dBytes)))
        {
        pBuffer = w2kMemoryDestroy (pBuffer);
        }
    return pBuffer;
    }

// =================================================================
// UNICODE STRING MANAGEMENT
// =================================================================

PANSI_STRING WINAPI w2kStringAnsi (DWORD dSize)
    {
    PANSI_STRING pasData = NULL;

    if ((pasData = w2kMemoryCreate (ANSI_STRING_ + dSize))
        != NULL)
        {
        pasData->Length        = 0;
        pasData->MaximumLength = (WORD) dSize;
        pasData->Buffer        = PTR_ADD (pasData, ANSI_STRING_);

        if (dSize) pasData->Buffer [0] = 0;
        }
    return pasData;
    }

// -----------------------------------------------------------------

PUNICODE_STRING WINAPI w2kStringUnicode (DWORD dSize)
    {
    DWORD           dSize1  = dSize * WORD_;
    PUNICODE_STRING pusData = NULL;

    if ((pusData = w2kMemoryCreate (UNICODE_STRING_ + dSize1))
        != NULL)
        {
        pusData->Length        = 0;
        pusData->MaximumLength = (WORD) dSize1;
        pusData->Buffer        = PTR_ADD (pusData, UNICODE_STRING_);

        if (dSize) pusData->Buffer [0] = 0;
        }
    return pusData;
    }

// -----------------------------------------------------------------

PUNICODE_STRING WINAPI w2kStringClone (PUNICODE_STRING pusSource)
    {
    DWORD           dSize;
    UNICODE_STRING  usCopy;
    PUNICODE_STRING pusData = NULL;

    if (w2kSpyRead (&usCopy, pusSource, UNICODE_STRING_))
        {
        dSize = max (usCopy.Length + WORD_,
                     usCopy.MaximumLength) / WORD_;

        if (((pusData = w2kStringUnicode (dSize)) != NULL) &&
            usCopy.Length && (usCopy.Buffer != NULL))
            {
            if (w2kSpyRead (pusData->Buffer, usCopy.Buffer,
                                             usCopy.Length))
                {
                pusData->Length = usCopy.Length;
                pusData->Buffer  [usCopy.Length / WORD_] = 0;
                }
            else
                {
                pusData = w2kMemoryDestroy (pusData);
                }
            }
        }
    return pusData;
    }

// -----------------------------------------------------------------

PWORD WINAPI w2kStringCopy (PUNICODE_STRING pusSource,
                            PUNICODE_STRING pusTarget,
                            PWORD           pwBuffer)
    {
    DWORD dLength = 0;
    PWORD pwNext  = NULL;

    if (pwBuffer != NULL)
        {
        if ((pusSource != NULL) && (pusSource->Buffer != NULL) &&
            (dLength = pusSource->Length) &&
            (!w2kSpyRead (pwBuffer, pusSource->Buffer, dLength)))
            {
            dLength = 0;
            }
        pwNext      = pwBuffer + (dLength / WORD_) + 1;
        pwNext [-1] = 0;
        }
    if (pusTarget != NULL)
        {
        if (pwBuffer != NULL)
            {
            pusTarget->Length        = (WORD) dLength;
            pusTarget->MaximumLength = (WORD) dLength + WORD_;
            pusTarget->Buffer        = pwBuffer;
            }
        else
            {
            pusTarget->Length        = 0;
            pusTarget->MaximumLength = 0;
            pusTarget->Buffer        = L"";
            }
        }
    return pwNext;
    }

// =================================================================
// KERNEL CALL INTERFACE
// =================================================================

BOOL WINAPI w2kCallInfo (PW2K_CALL_INFO pwci)
    {
    BOOL fOk = FALSE;

    if (pwci != NULL)
        {
        w2kSpyLock ();

        pwci->hDevice  = ghDevice;
        pwci->pwDevice = awSpyDisplay;

        lstrcpyn (pwci->awDriver, awDriver, MAX_PATH);

        w2kSpyUnlock ();
        fOk = TRUE;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kCallExecute (PSPY_CALL_INPUT  psci,
                            PSPY_CALL_OUTPUT psco)
    {
    BOOL fOk = FALSE;

    SetLastError (ERROR_INVALID_PARAMETER);

    if (psco != NULL)
        {
        psco->uliResult.QuadPart = 0;

        if ((psci != NULL)
            &&
            ((psci->pbSymbol    != NULL) ||
             (psci->pEntryPoint != NULL)))
            {
            fOk = w2kSpyControl (SPY_IO_CALL,
                                 psci, SPY_CALL_INPUT_,
                                 psco, SPY_CALL_OUTPUT_);
            }
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kCall (PULARGE_INTEGER puliResult,
                     PBYTE           pbSymbol,
                     PVOID           pEntryPoint,
                     BOOL            fFastCall,
                     DWORD           dArgumentBytes,
                     PVOID           pArguments)
    {
    SPY_CALL_INPUT  sci;
    SPY_CALL_OUTPUT sco;
    BOOL            fOk = FALSE;

    sci.fFastCall      = fFastCall;
    sci.dArgumentBytes = dArgumentBytes;
    sci.pArguments     = pArguments;
    sci.pbSymbol       = pbSymbol;
    sci.pEntryPoint    = pEntryPoint;

    fOk = w2kCallExecute (&sci, &sco);

    if (puliResult != NULL) *puliResult = sco.uliResult;
    return fOk; 
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kCallV (PULARGE_INTEGER puliResult,
                      PBYTE           pbSymbol,
                      BOOL            fFastCall,
                      DWORD           dArgumentBytes,
                      ...)
    {
    return w2kCall (puliResult, pbSymbol, NULL, fFastCall,
                    dArgumentBytes, &dArgumentBytes + 1);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI w2kCallNT (PBYTE pbSymbol,
                           DWORD dArgumentBytes,
                           ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCall (&uliResult, pbSymbol, NULL, FALSE,
                     dArgumentBytes, &dArgumentBytes + 1)

            ? uliResult.LowPart
            : STATUS_IO_DEVICE_ERROR);
    }

// -----------------------------------------------------------------

BYTE WINAPI w2kCall08 (BYTE  bDefault,
                       PBYTE pbSymbol,
                       BOOL  fFastCall,
                       DWORD dArgumentBytes,
                       ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCall (&uliResult, pbSymbol, NULL, fFastCall,
                     dArgumentBytes, &dArgumentBytes + 1)

            ? (BYTE) uliResult.LowPart
            : bDefault);
    }

// -----------------------------------------------------------------

WORD WINAPI w2kCall16 (WORD  wDefault,
                       PBYTE pbSymbol,
                       BOOL  fFastCall,
                       DWORD dArgumentBytes,
                       ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCall (&uliResult, pbSymbol, NULL, fFastCall,
                     dArgumentBytes, &dArgumentBytes + 1)

            ? (WORD) uliResult.LowPart
            : wDefault);
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCall32 (DWORD dDefault,
                        PBYTE pbSymbol,
                        BOOL  fFastCall,
                        DWORD dArgumentBytes,
                        ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCall (&uliResult, pbSymbol, NULL, fFastCall,
                     dArgumentBytes, &dArgumentBytes + 1)

            ? uliResult.LowPart
            : dDefault);
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kCall64 (QWORD qDefault,
                        PBYTE pbSymbol,
                        BOOL  fFastCall,
                        DWORD dArgumentBytes,
                        ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCall (&uliResult, pbSymbol, NULL, fFastCall,
                     dArgumentBytes, &dArgumentBytes + 1)

            ? uliResult.QuadPart
            : qDefault);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kCallP (PVOID pDefault,
                       PBYTE pbSymbol,
                       BOOL  fFastCall,
                       DWORD dArgumentBytes,
                       ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCall (&uliResult, pbSymbol, NULL, fFastCall,
                     dArgumentBytes, &dArgumentBytes + 1)

            ? (PVOID) uliResult.LowPart
            : pDefault);
    }

// =================================================================
// KERNEL COPY INTERFACE
// =================================================================

BOOL WINAPI w2kCopy (PULARGE_INTEGER puliResult,
                     PBYTE           pbSymbol,
                     PVOID           pEntryPoint,
                     DWORD           dBytes)
    {
    return w2kCall (puliResult, pbSymbol, pEntryPoint, FALSE,
                    0xFFFFFFFF - dBytes, NULL);
    }

// -----------------------------------------------------------------

BYTE WINAPI w2kCopy08 (BYTE  bDefault,
                       PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCopy (&uliResult, pbSymbol, NULL, 1)
            ? (BYTE) uliResult.LowPart
            : bDefault);
    }

// -----------------------------------------------------------------

WORD WINAPI w2kCopy16 (WORD  wDefault,
                       PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCopy (&uliResult, pbSymbol, NULL, 2)
            ? (WORD) uliResult.LowPart
            : wDefault);
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kCopy32 (DWORD dDefault,
                        PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCopy (&uliResult, pbSymbol, NULL, 4)
            ? uliResult.LowPart
            : dDefault);
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kCopy64 (QWORD qDefault,
                        PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCopy (&uliResult, pbSymbol, NULL, 8)
            ? uliResult.QuadPart
            : qDefault);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kCopyP (PVOID pDefault,
                       PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCopy (&uliResult, pbSymbol, NULL, 4)
            ? (PVOID) uliResult.LowPart
            : pDefault);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kCopyEP (PVOID pDefault,
                        PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kCopy (&uliResult, pbSymbol, NULL, 0)
            ? (PVOID) uliResult.LowPart
            : pDefault);
    }

// =================================================================
// EXTENDED KERNEL CALL INTERFACE
// =================================================================

BOOL WINAPI w2kXCall (PULARGE_INTEGER puliResult,
                      PBYTE           pbSymbol,
                      DWORD           dArgumentBytes,
                      PVOID           pArguments)
    {
    PIMG_TABLE pit;
    PIMG_ENTRY pie;
    BOOL       fOk = FALSE;

    if (((pit = w2kSymbolsGlobal (NULL))         != NULL) &&
        ((pie = imgTableResolve (pit, pbSymbol)) != NULL) &&
        (pie->pAddress != NULL))
        {
        fOk = w2kCall (puliResult, NULL, pie->pAddress,
                       pie->dConvention == IMG_CONVENTION_FASTCALL,
                       dArgumentBytes, pArguments);
        }
    else
        {
        if (puliResult != NULL) puliResult->QuadPart = 0;
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI w2kXCallV (PULARGE_INTEGER puliResult,
                       PBYTE           pbSymbol,
                       DWORD           dArgumentBytes,
                       ...)
    {
    return w2kXCall (puliResult, pbSymbol,
                     dArgumentBytes, &dArgumentBytes + 1);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI w2kXCallNT (PBYTE pbSymbol,
                            DWORD dArgumentBytes,
                            ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCall (&uliResult, pbSymbol,
                      dArgumentBytes, &dArgumentBytes + 1)

            ? uliResult.LowPart
            : STATUS_IO_DEVICE_ERROR);
    }

// -----------------------------------------------------------------

BYTE WINAPI w2kXCall08 (BYTE  bDefault,
                        PBYTE pbSymbol,
                        DWORD dArgumentBytes,
                        ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCall (&uliResult, pbSymbol,
                      dArgumentBytes, &dArgumentBytes + 1)

            ? (BYTE) uliResult.LowPart
            : bDefault);
    }

// -----------------------------------------------------------------

WORD WINAPI w2kXCall16 (WORD  wDefault,
                        PBYTE pbSymbol,
                        DWORD dArgumentBytes,
                        ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCall (&uliResult, pbSymbol,
                      dArgumentBytes, &dArgumentBytes + 1)

            ? (WORD) uliResult.LowPart
            : wDefault);
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kXCall32 (DWORD dDefault,
                         PBYTE pbSymbol,
                         DWORD dArgumentBytes,
                         ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCall (&uliResult, pbSymbol,
                      dArgumentBytes, &dArgumentBytes + 1)

            ? uliResult.LowPart
            : dDefault);
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kXCall64 (QWORD qDefault,
                         PBYTE pbSymbol,
                         DWORD dArgumentBytes,
                         ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCall (&uliResult, pbSymbol,
                      dArgumentBytes, &dArgumentBytes + 1)

            ? uliResult.QuadPart
            : qDefault);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kXCallP (PVOID pDefault,
                        PBYTE pbSymbol,
                        DWORD dArgumentBytes,
                        ...)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCall (&uliResult, pbSymbol,
                      dArgumentBytes, &dArgumentBytes + 1)

            ? (PVOID) uliResult.LowPart
            : pDefault);
    }

// =================================================================
// EXTENDED KERNEL COPY INTERFACE
// =================================================================

BOOL WINAPI w2kXCopy (PULARGE_INTEGER puliResult,
                      PBYTE           pbSymbol,
                      DWORD           dBytes)
    {
    return w2kXCall (puliResult, pbSymbol,
                     0xFFFFFFFF - dBytes, NULL);
    }

// -----------------------------------------------------------------

BYTE WINAPI w2kXCopy08 (BYTE  bDefault,
                        PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCopy (&uliResult, pbSymbol, 1)
            ? (BYTE) uliResult.LowPart
            : bDefault);
    }

// -----------------------------------------------------------------

WORD WINAPI w2kXCopy16 (WORD  wDefault,
                        PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCopy (&uliResult, pbSymbol, 2)
            ? (WORD) uliResult.LowPart
            : wDefault);
    }

// -----------------------------------------------------------------

DWORD WINAPI w2kXCopy32 (DWORD dDefault,
                         PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCopy (&uliResult, pbSymbol, 4)
            ? uliResult.LowPart
            : dDefault);
    }

// -----------------------------------------------------------------

QWORD WINAPI w2kXCopy64 (QWORD qDefault,
                         PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCopy (&uliResult, pbSymbol, 8)
            ? uliResult.QuadPart
            : qDefault);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kXCopyP (PVOID pDefault,
                        PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCopy (&uliResult, pbSymbol, 4)
            ? (PVOID) uliResult.LowPart
            : pDefault);
    }

// -----------------------------------------------------------------

PVOID WINAPI w2kXCopyEP (PVOID pDefault,
                         PBYTE pbSymbol)
    {
    ULARGE_INTEGER uliResult;

    return (w2kXCopy (&uliResult, pbSymbol, 0)
            ? (PVOID) uliResult.LowPart
            : pDefault);
    }

// =================================================================
// BEEP CONTROL
// =================================================================

BOOL WINAPI
w2kBeep (DWORD dDuration,
         DWORD dPitch)
    {
    BOOL fOk = TRUE;

    if (!_HalMakeBeep (dPitch)) fOk = FALSE;
    Sleep (dDuration);
    if (!_HalMakeBeep (0     )) fOk = FALSE;
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI
w2kBeepEx (DWORD dData,
           ...)
    {
    PDWORD pdData;
    BOOL   fOk = TRUE;

    for (pdData = &dData; pdData [0]; pdData += 2)
        {
        if (!w2kBeep (pdData [0], pdData [1])) fOk = FALSE;
        }
    return fOk;
    }

// =================================================================
// OBJECT MANAGEMENT
// =================================================================

POBJECT_HEADER WINAPI
w2kObjectHeader (POBJECT pObject)
    {
    DWORD          dOffset = OBJECT_HEADER_;
    POBJECT_HEADER pHeader = NULL;

    if (pObject != NULL)
        {
        pHeader = w2kSpyClone (BACK (pObject, dOffset),
                               dOffset);
        }
    return pHeader;
    }

// -----------------------------------------------------------------

POBJECT_CREATOR_INFO WINAPI
w2kObjectCreatorInfo (POBJECT_HEADER pHeader,
                      POBJECT        pObject)
    {
    DWORD                dOffset;
    POBJECT_CREATOR_INFO pCreatorInfo = NULL;

    if ((pHeader != NULL) && (pObject != NULL) &&
        (pHeader->ObjectFlags & OB_FLAG_CREATOR_INFO))
        {
        dOffset = OBJECT_CREATOR_INFO_ + OBJECT_HEADER_;

        pCreatorInfo = w2kSpyClone (BACK (pObject, dOffset),
                                    OBJECT_CREATOR_INFO_);
        }
    return pCreatorInfo;
    }

// -----------------------------------------------------------------

POBJECT_NAME WINAPI
w2kObjectName (POBJECT_HEADER pHeader,
               POBJECT        pObject)
    {
    DWORD        dOffset;
    POBJECT_NAME pName = NULL;

    if ((pHeader != NULL) && (pObject != NULL) &&
        (dOffset = pHeader->NameOffset))
        {
        dOffset += OBJECT_HEADER_;

        pName = w2kSpyClone (BACK (pObject, dOffset),
                             OBJECT_NAME_);
        }
    return pName;
    }

// -----------------------------------------------------------------

POBJECT_HANDLE_DB WINAPI
w2kObjectHandleDB (POBJECT_HEADER pHeader,
                   POBJECT        pObject)
    {
    DWORD             dOffset;
    POBJECT_HANDLE_DB pHandleDB = NULL;

    if ((pHeader != NULL) && (pObject != NULL) &&
        (dOffset = pHeader->HandleDBOffset))
        {
        dOffset += OBJECT_HEADER_;

        pHandleDB = w2kSpyClone (BACK (pObject, dOffset),
                                 OBJECT_HANDLE_DB_);
        }
    return pHandleDB;
    }

// -----------------------------------------------------------------

POBJECT_QUOTA_CHARGES WINAPI
w2kObjectQuotaCharges (POBJECT_HEADER pHeader,
                       POBJECT        pObject)
    {
    DWORD                 dOffset;
    POBJECT_QUOTA_CHARGES pQuotaCharges = NULL;

    if ((pHeader != NULL) && (pObject != NULL) &&
        (dOffset = pHeader->QuotaChargesOffset))
        {
        dOffset += OBJECT_HEADER_;

        pQuotaCharges = w2kSpyClone (BACK (pObject, dOffset),
                                     OBJECT_QUOTA_CHARGES_);
        }
    return pQuotaCharges;
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
w2kObjectType (POBJECT_HEADER pHeader)
    {
    POBJECT_TYPE pType = NULL;

    if (pHeader != NULL)
        {
        pType = w2kSpyClone (pHeader->ObjectType,
                             OBJECT_TYPE_);
        }
    return pType;
    }

// -----------------------------------------------------------------

PQUOTA_BLOCK WINAPI
w2kObjectQuotaBlock (POBJECT_HEADER pHeader)
    {
    PQUOTA_BLOCK pQuotaBlock = NULL;

    if ((pHeader != NULL) &&
        (!(pHeader->ObjectFlags & OB_FLAG_CREATE_INFO)))
        {
        pQuotaBlock = w2kSpyClone (pHeader->QuotaBlock,
                                   QUOTA_BLOCK_);
        }
    return pQuotaBlock;
    }

// -----------------------------------------------------------------

POBJECT_CREATE_INFO WINAPI
w2kObjectCreateInfo (POBJECT_HEADER pHeader)
    {
    POBJECT_CREATE_INFO pCreateInfo = NULL;

    if ((pHeader != NULL) &&
        (pHeader->ObjectFlags & OB_FLAG_CREATE_INFO))
        {
        pCreateInfo = w2kSpyClone (pHeader->ObjectCreateInfo,
                                   OBJECT_CREATE_INFO_);
        }
    return pCreateInfo;
    }

// -----------------------------------------------------------------

PW2K_OBJECT WINAPI
w2kObjectOpen (POBJECT pObject)
    {
    DWORD             dName, dType, dSize;
    PWORD             pwBuffer;
    PUNICODE_STRING   pus;
    POBJECT_HEADER    pHeader;
    W2K_OBJECT        wo;
    PW2K_OBJECT_FRAME pwof = NULL;

    if ((pHeader = w2kObjectHeader (pObject)) != NULL)
        {
        wo.pObject       = pObject;
        wo.pHeader       = pHeader;
        wo.pCreatorInfo  = w2kObjectCreatorInfo  (pHeader, pObject);
        wo.pName         = w2kObjectName         (pHeader, pObject);
        wo.pHandleDB     = w2kObjectHandleDB     (pHeader, pObject);
        wo.pQuotaCharges = w2kObjectQuotaCharges (pHeader, pObject);
        wo.pType         = w2kObjectType         (pHeader);
        wo.pQuotaBlock   = w2kObjectQuotaBlock   (pHeader);
        wo.pCreateInfo   = w2kObjectCreateInfo   (pHeader);

        dName = ((wo.pName              != NULL) &&
                 (wo.pName->Name.Buffer != NULL)
                 ? wo.pName->Name.Length / WORD_
                 : 0);

        dType = ((wo.pType                        != NULL) &&
                 (wo.pType->ObjectTypeName.Buffer != NULL)
                 ? wo.pType->ObjectTypeName.Length / WORD_
                 : 0);

        dSize = W2K_OBJECT_FRAME__ (dName+1 + dType+1);

        if ((pwof = w2kMemoryCreate (dSize)) != NULL)
            {
            ZeroMemory (pwof, dSize);

            pwBuffer = pwof->Buffer;

            pwof->Header                    = *pHeader;
            pwof->Header.ObjectFlags       &= ~OB_FLAG_CREATOR_INFO;
            pwof->Header.NameOffset         = 0;
            pwof->Header.HandleDBOffset     = 0;
            pwof->Header.QuotaChargesOffset = 0;
            pwof->Header.ObjectType         = NULL;
            pwof->Header.QuotaBlock         = NULL;
            pwof->Header.ObjectCreateInfo   = NULL;

            pwof->Object                    =  wo;
            pwof->Object.pHeader            = &pwof->Header;

            if (wo.pCreatorInfo != NULL)
                {
                pwof->CreatorInfo         = *wo.pCreatorInfo;
                pwof->Object.pCreatorInfo = &pwof->CreatorInfo;
                pwof->Header.ObjectFlags |= OB_FLAG_CREATOR_INFO;
                }
            pus = NULL;
            if (wo.pName != NULL)
                {
                pwof->Name          = *wo.pName;
                pwof->Object.pName  = &pwof->Name;

                pwof->Header.NameOffset =
                    OFFSET (W2K_OBJECT_FRAME, Header) -
                    OFFSET (W2K_OBJECT_FRAME, Name);

                pus = &pwof->Name.Name;
                }
            pwof->Object.pwName = pwBuffer;
            pwBuffer = w2kStringCopy (pus, pus, pwBuffer);

            if (wo.pHandleDB != NULL)
                {
                pwof->HandleDB         = *wo.pHandleDB;
                pwof->Object.pHandleDB = &pwof->HandleDB;

                pwof->Header.HandleDBOffset =
                    OFFSET (W2K_OBJECT_FRAME, Header) -
                    OFFSET (W2K_OBJECT_FRAME, HandleDB);
                }
            if (wo.pQuotaCharges != NULL)
                {
                pwof->QuotaCharges         = *wo.pQuotaCharges;
                pwof->Object.pQuotaCharges = &pwof->QuotaCharges;

                pwof->Header.QuotaChargesOffset =
                    OFFSET (W2K_OBJECT_FRAME, Header) -
                    OFFSET (W2K_OBJECT_FRAME, QuotaCharges);
                }
            pus = NULL;
            if (wo.pType != NULL)
                {
                pwof->Type              = *wo.pType;
                pwof->Object.pType      = &pwof->Type;
                pwof->Header.ObjectType = &pwof->Type;

                pus = &pwof->Type.ObjectTypeName;
                }
            pwof->Object.pwType = pwBuffer;
            pwBuffer = w2kStringCopy (pus, pus, pwBuffer);

            if (wo.pQuotaBlock != NULL)
                {
                pwof->QuotaBlock         = *wo.pQuotaBlock;
                pwof->Object.pQuotaBlock = &pwof->QuotaBlock;
                pwof->Header.QuotaBlock  = &pwof->QuotaBlock;
                }
            if (wo.pCreateInfo != NULL)
                {
                pwof->CreateInfo              = *wo.pCreateInfo;
                pwof->Object.pCreateInfo      = &pwof->CreateInfo;
                pwof->Header.ObjectCreateInfo = &pwof->CreateInfo;
                }
            }
        w2kMemoryDestroy (wo.pHeader      );
        w2kMemoryDestroy (wo.pCreatorInfo );
        w2kMemoryDestroy (wo.pName        );
        w2kMemoryDestroy (wo.pHandleDB    );
        w2kMemoryDestroy (wo.pQuotaCharges);
        w2kMemoryDestroy (wo.pType        );
        w2kMemoryDestroy (wo.pQuotaBlock  );
        w2kMemoryDestroy (wo.pCreateInfo  );
        }
    return (pwof != NULL ? &pwof->Object : NULL);
    }

// -----------------------------------------------------------------

PW2K_OBJECT WINAPI
w2kObjectClose (PW2K_OBJECT pwo)
    {
    if (pwo != NULL)
        {
        w2kMemoryDestroy
            (BACK (pwo, OFFSET (W2K_OBJECT_FRAME, Object)));
        }
    return NULL;
    }

// =================================================================
// OBJECT DIRECTORY MANAGEMENT
// =================================================================

POBJECT_DIRECTORY WINAPI
w2kDirectoryOpen (POBJECT_DIRECTORY pDir)
    {
    DWORD                    i;
    PERESOURCE               pLock;
    PPOBJECT_DIRECTORY_ENTRY ppEntry;
    POBJECT_DIRECTORY        pDir1 = NULL;

    if (((pLock = __ObpRootDirectoryMutex ()) != NULL) &&
        _ExAcquireResourceExclusiveLite (pLock, TRUE))
        {
        if ((pDir1 = w2kSpyClone (pDir, OBJECT_DIRECTORY_)) != NULL)
            {
            for (i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
                {
                ppEntry = pDir1->HashTable + i;

                while (*ppEntry != NULL)
                    {
                    if ((*ppEntry =
                            w2kSpyClone (*ppEntry,
                                         OBJECT_DIRECTORY_ENTRY_))
                        != NULL)
                        {
                        (*ppEntry)->Object =
                            w2kObjectOpen ((*ppEntry)->Object);

                        ppEntry = &(*ppEntry)->NextEntry;
                        }
                    }
                }
            }
        _ExReleaseResourceLite (pLock);
        }
    return pDir1;
    }

// -----------------------------------------------------------------

POBJECT_DIRECTORY WINAPI
w2kDirectoryClose (POBJECT_DIRECTORY pDir)
    {
    POBJECT_DIRECTORY_ENTRY pEntry, pEntry1;
    DWORD                   i;

    if (pDir != NULL)
        {
        for (i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
            {
            for (pEntry  = pDir->HashTable [i];
                 pEntry != NULL;
                 pEntry  = pEntry1)
                {
                pEntry1 = pEntry->NextEntry;

                w2kObjectClose   (pEntry->Object);
                w2kMemoryDestroy (pEntry);
                }
            }
        w2kMemoryDestroy (pDir);
        }
    return NULL;
    }

// -----------------------------------------------------------------

DWORD WINAPI
w2kDirectorySize (POBJECT_DIRECTORY pDir,
                  PWORD             pwType)
    {
    POBJECT_DIRECTORY_ENTRY  pEntry;
    POBJECT_NAME_INFORMATION pInfo;
    DWORD                    dInfo, i, n;
    DWORD                    dSize = 0;

    if (pDir != NULL)
        {
        dInfo = OBJECT_NAME_INFORMATION_ + (1000 * WORD_);
        pInfo = (pwType != NULL ? w2kMemoryCreate (dInfo) : NULL);

        if ((pwType == NULL) || (pInfo != NULL))
            {
            for (i = 0; i < OBJECT_HASH_TABLE_SIZE; i++)
                {
                for (pEntry  = pDir->HashTable [i];
                     pEntry != NULL;
                     pEntry  = pEntry->NextEntry)
                    {
                    n = 0;

                    if ((pwType == NULL)
                        ||
                        ((__ObQueryTypeName (pEntry->Object,
                                             pInfo, dInfo, &n)
                          == STATUS_SUCCESS)
                         &&
                         ((!lstrcmp (L"Directory",
                                     pInfo->Name.Buffer))
                          ||
                          w2kStringFilter (pwType,
                                           pInfo->Name.Buffer,
                                           TRUE))))
                        {
                        dSize++;
                        }
                    }
                }
            w2kMemoryDestroy (pInfo);
            }
        }
    return dSize;
    }

// =================================================================
// EXECUTIVE
// =================================================================

BOOLEAN WINAPI
_ExAcquireResourceExclusiveLite (PERESOURCE Resource,
                                 BOOLEAN    Wait)
    {
    return w2kCall08 (FALSE, "ExAcquireResourceExclusiveLite",FALSE,
                      8, Resource, Wait);
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_ExAcquireResourceSharedLite (PERESOURCE Resource,
                              BOOLEAN    Wait)
    {
    return w2kCall08 (FALSE, "ExAcquireResourceSharedLite", FALSE,
                      8, Resource, Wait);
    }

// -----------------------------------------------------------------

PVOID WINAPI
_ExAllocatePool (POOL_TYPE PoolType,
                 SIZE_T    NumberOfBytes)
    {
    return w2kCallP (NULL, "ExAllocatePool", FALSE,
                     8, PoolType, NumberOfBytes);
    }

// -----------------------------------------------------------------

PVOID WINAPI
_ExAllocatePoolWithTag (POOL_TYPE PoolType,
                        SIZE_T    NumberOfBytes,
                        DWORD     Tag)
    {
    return w2kCallP (NULL, "ExAllocatePoolWithTag", FALSE,
                     12, PoolType, NumberOfBytes, Tag);
    }

// -----------------------------------------------------------------

VOID WINAPI
_ExFreePool (PVOID P)
    {
    w2kCallV (NULL, "ExFreePool", FALSE,
              4, P);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
_ExFreePoolWithTag (PVOID P,
                    DWORD Tag)
    {
    w2kCallV (NULL, "ExFreePoolWithTag", FALSE,
              8, P, Tag);
    return;
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
__ExLockHandleTableEntry (PHANDLE_TABLE HandleTable,
                          PHANDLE_ENTRY HandleEntry)
    {
    return w2kXCall08 (FALSE, "ExLockHandleTableEntry",
                       8, HandleTable, HandleEntry);
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
__ExLockHandleTableExclusive (PHANDLE_TABLE HandleTable)
    {
    return w2kXCall08 (FALSE, "ExLockHandleTableExclusive",
                       4, HandleTable);
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
__ExLockHandleTableShared (PHANDLE_TABLE HandleTable)
    {
    return w2kXCall08 (FALSE, "ExLockHandleTableShared",
                       4, HandleTable);
    }

// -----------------------------------------------------------------

PHANDLE_ENTRY WINAPI
__ExMapHandleToPointer (PHANDLE_TABLE HandleTable,
                        HANDLE        Handle)
    {
    return w2kXCallP (NULL, "ExMapHandleToPointer",
                      8, HandleTable, Handle);
    }

// -----------------------------------------------------------------

VOID WINAPI
_ExReleaseResourceLite (PERESOURCE Resource)
    {
    w2kCallV (NULL, "ExReleaseResourceLite", TRUE,
              4, Resource);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
__ExUnlockHandleTableEntry (PHANDLE_TABLE HandleTable,
                            PHANDLE_ENTRY HandleEntry)
    {
    w2kXCallV (NULL, "ExUnlockHandleTableEntry",
               8, HandleTable, HandleEntry);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
__ExUnlockHandleTableShared (PERESOURCE Resource)
    {
    w2kXCallV (NULL, "ExUnlockHandleTableShared",
               4, Resource);
    return;
    }

// -----------------------------------------------------------------

PHANDLE_ENTRY WINAPI
__ExpLookupHandleTableEntry (PHANDLE_TABLE HandleTable,
                             HANDLE        Handle)
    {
    return w2kXCallP (NULL, "ExpLookupHandleTableEntry",
                      8, HandleTable, Handle);
    }

// =================================================================
// FILE SYSTEM RUNTIME LIBRARY
// =================================================================

PBYTE WINAPI
_FsRtlLegalAnsiCharacterArray (VOID)
    {
    return w2kCopyP (NULL, "FsRtlLegalAnsiCharacterArray");
    }

// =================================================================
// HARDWARE ABSTRACTION LAYER
// =================================================================
// pitch: 0 = off, 1..18 invalid, 19+ ok

BOOLEAN WINAPI
_HalMakeBeep (DWORD Pitch)
    {
    return w2kCall08 (FALSE, "hal.dll!HalMakeBeep", FALSE,
                      4, Pitch);
    }

// -----------------------------------------------------------------

VOID WINAPI
_HalQueryRealTimeClock (PTIME_FIELDS TimeFields)
    {
    w2kCallV (NULL, "hal.dll!HalQueryRealTimeClock", FALSE,
              4, TimeFields);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
_HalSetRealTimeClock (PTIME_FIELDS TimeFields)
    {
    w2kCallV (NULL, "hal.dll!HalSetRealTimeClock", FALSE,
              4, TimeFields);
    return;
    }

// =================================================================
// HANDLE TABLES
// =================================================================

PLIST_ENTRY WINAPI
__HandleTableListHead (VOID)
    {
    return w2kXCopyEP (NULL, "HandleTableListHead");
    }

// -----------------------------------------------------------------

PERESOURCE WINAPI
__HandleTableListLock (VOID)
    {
    return w2kXCopyP (NULL, "HandleTableListLock");
    }

// =================================================================
// KERNEL
// =================================================================

DWORD WINAPI
_KeI386MachineType (VOID)
    {
    return w2kCopy32 (0, "KeI386MachineType");
    }

// -----------------------------------------------------------------

BYTE WINAPI
_KeNumberProcessors (VOID)
    {
    return w2kCopy08 (0, "KeNumberProcessors");
    }

// -----------------------------------------------------------------

PSERVICE_DESCRIPTOR_TABLE WINAPI
_KeServiceDescriptorTable (VOID)
    {
    return w2kCopyEP (NULL, "KeServiceDescriptorTable");
    }

// -----------------------------------------------------------------

QWORD WINAPI
_KeTickCount (VOID)
    {
    return w2kCopy64 (0, "KeTickCount");
    }

// =================================================================
// MEMORY MANAGER
// =================================================================

PHYSICAL_ADDRESS WINAPI
_MmGetPhysicalAddress (PVOID BaseAddress)
    {
    PHYSICAL_ADDRESS pa;

    pa.QuadPart = w2kCall64 (0, "MmGetPhysicalAddress", FALSE,
                             4, BaseAddress);
    return pa;
    }

// -----------------------------------------------------------------

PVOID WINAPI
_MmGetSystemRoutineAddress (PWORD SystemRoutineName)
    {
    return w2kCallP (NULL, "MmGetSystemRoutineAddress", FALSE,
                     4, SystemRoutineName);
    }

// -----------------------------------------------------------------

PVOID WINAPI
_MmGetVirtualForPhysical (PHYSICAL_ADDRESS PhysicalAddress)
    {
    return w2kCallP (NULL, "MmGetVirtualForPhysical", FALSE,
                     8, PhysicalAddress);
    }

// -----------------------------------------------------------------

PVOID WINAPI
_MmHighestUserAddress (VOID)
    {
    return w2kCopyP (NULL, "MmHighestUserAddress");
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_MmIsAddressValid (PVOID VirtualAddress)
    {
    return w2kCall08 (FALSE, "MmIsAddressValid", FALSE,
                      4, VirtualAddress);
    }

// -----------------------------------------------------------------

PVOID WINAPI
_MmSystemRangeStart (VOID)
    {
    return w2kCopyP (NULL, "MmSystemRangeStart");
    }

// -----------------------------------------------------------------

PVOID WINAPI
_MmUserProbeAddress (VOID)
    {
    return w2kCopyP (NULL, "MmUserProbeAddress");
    }

// =================================================================
// NATIONAL LANGUAGE SUPPORT
// =================================================================

WORD WINAPI
_NlsAnsiCodePage (VOID)
    {
    return w2kCopy16 (0, "NlsAnsiCodePage");
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_NlsMbCodePageTag (VOID)
    {
    return w2kCopy08 (FALSE, "NlsMbCodePageTag");
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_NlsMbOemCodePageTag (VOID)
    {
    return w2kCopy08 (FALSE, "NlsMbOemCodePageTag");
    }

// -----------------------------------------------------------------

WORD WINAPI
_NlsOemCodePage (VOID)
    {
    return w2kCopy16 (0, "NlsOemCodePage");
    }

// =================================================================
// NATIVE API
// =================================================================

WORD WINAPI
_NtBuildNumber (VOID)
    {
    return w2kCopy16 (0, "NtBuildNumber");
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_NtClose (HANDLE Handle)
    {
    return w2kCallNT ("NtClose",
                      4, Handle);
    }

// -----------------------------------------------------------------

DWORD WINAPI
_NtGlobalFlag (VOID)
    {
    return w2kCopy32 (0, "NtGlobalFlag");
    }

// =================================================================
// OBJECT MANAGER
// =================================================================

VOID WINAPI
_ObDereferenceObject (POBJECT Object)
    {
    w2kCallV (NULL, "ObDereferenceObject", FALSE,
              4, Object);
    return;
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_ObOpenObjectByPointer (POBJECT         Object,
                        DWORD           HandleAttributes,
         /* optional */ PACCESS_STATE   PassedAccessState,
                        ACCESS_MASK     DesiredAccess,
                        POBJECT_TYPE    ObjectType,
                        KPROCESSOR_MODE AccessMode,
                        PHANDLE         Handle)
    {
    return w2kCallNT ("ObOpenObjectByPointer",
                      28, Object, HandleAttributes,
                          PassedAccessState, DesiredAccess,
                          ObjectType, AccessMode, Handle);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_ObQueryNameString (POBJECT                  Object,
                    POBJECT_NAME_INFORMATION NameString,
        /* bytes */ DWORD                    NameStringLength,
                    PDWORD                   ReturnLength)
    {
    return w2kCallNT ("ObQueryNameString",
                      16, Object, NameString, NameStringLength,
                          ReturnLength);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
__ObQueryTypeInfo (POBJECT_TYPE      ObjectType,
                   POBJECT_TYPE_INFO TypeInfo,
   /* bytes     */ DWORD             TypeInfoLength,
   /* init to 0 */ PDWORD            ReturnLength)
    {
    return w2kXCallNT ("ObQueryTypeInfo",
                       16, ObjectType, TypeInfo, TypeInfoLength,
                           ReturnLength);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
__ObQueryTypeName (POBJECT                  Object,
                   POBJECT_NAME_INFORMATION NameString,
       /* bytes */ DWORD                    NameStringLength,
                   PDWORD                   ReturnLength)
    {
    return w2kXCallNT ("ObQueryTypeName",
                       16, Object, NameString, NameStringLength,
                           ReturnLength);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_ObReferenceObjectByHandle
                     (HANDLE                     Handle,
                      ACCESS_MASK                DesiredAccess,
       /* optional */ POBJECT_TYPE               ObjectType,
                      KPROCESSOR_MODE            AccessMode,
                      PPOBJECT                   Object,
       /* optional */ POBJECT_HANDLE_INFORMATION HandleInformation)
    {
    return w2kCallNT ("ObReferenceObjectByHandle",
                      24, Handle, DesiredAccess, ObjectType,
                          AccessMode, Object, HandleInformation);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_ObReferenceObjectByPointer (POBJECT         Object,
                             ACCESS_MASK     DesiredAccess,
                             POBJECT_TYPE    ObjectType,
                             KPROCESSOR_MODE AccessMode)
    {
    return w2kCallNT ("ObReferenceObjectByPointer",
                      16, Object, DesiredAccess, ObjectType,
                          AccessMode);
    }

// -----------------------------------------------------------------

VOID WINAPI
_ObfDereferenceObject (POBJECT Object)
    {
    w2kCallV (NULL, "ObfDereferenceObject", TRUE,
              4, Object);
    return;
    }

// -----------------------------------------------------------------

#define _ObReferenceObject(Object) _ObfReferenceObject(Object)

VOID WINAPI
_ObfReferenceObject (POBJECT Object)
    {
    w2kCallV (NULL, "ObfReferenceObject", TRUE,
              4, Object);
    return;
    }

// -----------------------------------------------------------------

PHANDLE_TABLE WINAPI
__ObpKernelHandleTable (VOID)
    {
    return w2kXCopyP (NULL, "ObpKernelHandleTable");
    }

// -----------------------------------------------------------------

PERESOURCE WINAPI
__ObpRootDirectoryMutex (VOID)
    {
    return w2kXCopyP (NULL, "ObpRootDirectoryMutex");
    }

// -----------------------------------------------------------------

POBJECT_DIRECTORY WINAPI
__ObpRootDirectoryObject (VOID)
    {
    return w2kXCopyP (NULL, "ObpRootDirectoryObject");
    }

// -----------------------------------------------------------------

POBJECT_DIRECTORY WINAPI
__ObpTypeDirectoryObject (VOID)
    {
    return w2kXCopyP (NULL, "ObpTypeDirectoryObject");
    }

// =================================================================
// MEMORY PROBING
// =================================================================

DWORD WINAPI
_ProbeForRead (PVOID Address,
               DWORD Length,
               DWORD Alignment)
    {
    w2kCallV (NULL, "ProbeForRead", FALSE,
              12, Address, Length, Alignment);

    return GetLastError ();
    };

// -----------------------------------------------------------------

DWORD WINAPI
_ProbeForWrite (PVOID Address,
                DWORD Length,
                DWORD Alignment)
    {
    w2kCallV (NULL, "ProbeForWrite", FALSE,
              12, Address, Length, Alignment);

    return GetLastError ();
    };

// =================================================================
// PROCESS STRUCTURE
// =================================================================

HANDLE WINAPI
_PsGetCurrentProcessId (VOID)
    {
    return w2kCallP (0, "PsGetCurrentProcessId", FALSE, 0);
    }

// -----------------------------------------------------------------

HANDLE WINAPI
_PsGetCurrentThreadId (VOID)
    {
    return w2kCallP (0, "PsGetCurrentThreadId", FALSE, 0);
    }

// -----------------------------------------------------------------

LARGE_INTEGER WINAPI
_PsGetProcessExitTime (VOID)
    {
    LARGE_INTEGER li;

    li.QuadPart = w2kCall64 (0, "PsGetProcessExitTime", FALSE, 0);
    return li;
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_PsGetVersion (PDWORD          MajorVersion,
               PDWORD          MinorVersion,
               PDWORD          BuildNumber,
               PUNICODE_STRING CSDVersion)
    {
    return w2kCall08 (FALSE, "PsGetVersion", FALSE,
                      16, MajorVersion, MinorVersion,
                          BuildNumber,  CSDVersion);
    }

// -----------------------------------------------------------------

PEPROCESS WINAPI
_PsInitialSystemProcess (VOID)
    {
    return w2kCopyP (NULL, "PsInitialSystemProcess");
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_PsIsThreadTerminating (PETHREAD Thread)
    {
    return w2kCall08 (FALSE, "PsIsThreadTerminating", FALSE,
                      4, Thread);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_PsLookupProcessByProcessId (HANDLE     UniqueProcessId,
                             PPEPROCESS Process)
    {
    return w2kCallNT ("PsLookupProcessByProcessId",
                      8, UniqueProcessId, Process);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_PsLookupProcessThreadByCid (PCLIENT_ID Cid,
              /* optional */ PPEPROCESS Process,
                             PPETHREAD  Thread)
    {
    return w2kCallNT ("PsLookupProcessThreadByCid",
                      12, Cid, Process, Thread);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_PsLookupThreadByThreadId (HANDLE    UniqueThreadId,
                           PPETHREAD Thread)
    {
    return w2kCallNT ("PsLookupThreadByThreadId",
                      8, UniqueThreadId, Thread);
    }

// =================================================================
// RUNTIME LIBRARY
// =================================================================

NTSTATUS WINAPI
_RtlAnsiStringToUnicodeString (PUNICODE_STRING Destination,
                               PANSI_STRING    Source,
                               BOOLEAN         AllocateDestination)
    {
    return w2kCallNT ("RtlAnsiStringToUnicodeString",
                      12, Destination, Source, AllocateDestination);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlAppendUnicodeStringToString (PUNICODE_STRING Destination,
                                 PUNICODE_STRING Source)
    {
    return w2kCallNT ("RtlAppendUnicodeStringToString",
                      8, Destination, Source);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlAppendUnicodeToString (PUNICODE_STRING Destination,
                           PWORD           Source)
    {
    return w2kCallNT ("RtlAppendUnicodeToString",
                      8, Destination, Source);
    }

// -----------------------------------------------------------------

SIZE_T WINAPI
_RtlCompareMemory (PVOID  Source1,
                   PVOID  Source2,
                   SIZE_T Length)
    {
    return w2kCall32 (0, "RtlCompareMemory", FALSE,
                      12, Source1, Source2, Length);
    }

// -----------------------------------------------------------------

SIZE_T WINAPI
_RtlCompareMemoryUlong (PVOID  Source1,
                        PVOID  Source2,
            /* Bytes */ SIZE_T Length)
    {
    return w2kCall32 (0, "RtlCompareMemoryUlong", FALSE,
                      12, Source1, Source2, Length);
    }

// -----------------------------------------------------------------

LONG WINAPI
_RtlCompareUnicodeString (PUNICODE_STRING String1,
                          PUNICODE_STRING String2,
                          BOOLEAN         CaseInSensitive)
    {
    return w2kCall32 (0, "RtlCompareUnicodeString", FALSE,
                      12, String1, String2, CaseInSensitive);
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlCopyUnicodeString (PUNICODE_STRING Destination,
                       PUNICODE_STRING Source)
    {
    w2kCallV (NULL, "RtlCopyUnicodeString", FALSE,
              8, Destination, Source);
    return;
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_RtlEqualUnicodeString (PUNICODE_STRING String1,
                        PUNICODE_STRING String2,
                        BOOLEAN         CaseInSensitive)
    {
    return w2kCall08 (FALSE, "RtlEqualUnicodeString", FALSE,
                      12, String1, String2, CaseInSensitive);
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlFillMemory (PVOID  Destination,
                SIZE_T Length,
                BYTE   Fill)
    {
    w2kCallV (NULL, "RtlFillMemory", FALSE,
              12, Destination, Length, Fill);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlFillMemoryUlong (PVOID  Destination,
         /* Bytes */ SIZE_T Length,
                     DWORD  Fill)
    {
    w2kCallV (NULL, "RtlFillMemoryUlong", FALSE,
              12, Destination, Length, Fill);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlFreeUnicodeString (PUNICODE_STRING UnicodeString)
    {
    w2kCallV (NULL, "RtlFreeUnicodeString", FALSE,
              4, UnicodeString);
    return;
    }

// -----------------------------------------------------------------

PVOID WINAPI
_RtlImageDirectoryEntryToData (PVOID   Base,
                               BOOLEAN MappedAsImage,
                               WORD    DirectoryEntry,
                               PDWORD  Size)
    {
    return w2kCallP (NULL, "RtlImageDirectoryEntryToData", FALSE,
                     16, Base, MappedAsImage, DirectoryEntry, Size);
    }

// -----------------------------------------------------------------

PIMAGE_NT_HEADERS WINAPI
_RtlImageNtHeader (PVOID Base)
    {
    return w2kCallP (NULL, "RtlImageNtHeader", FALSE,
                     4, Base);
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlInitAnsiString (PANSI_STRING Destination,
                    PBYTE        Source)
    {
    w2kCallV (NULL, "RtlInitAnsiString", FALSE,
              8, Destination, Source);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlInitString (PSTRING Destination,
                PBYTE   Source)
    {
    w2kCallV (NULL, "RtlInitString", FALSE,
              8, Destination, Source);
    return;
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlInitUnicodeString (PUNICODE_STRING Destination,
                       PWORD           Source)
    {
    w2kCallV (NULL, "RtlInitUnicodeString", FALSE,
              8, Destination, Source);
    return;
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlInt64ToUnicodeString (QWORD          Value,
                          DWORD           Base,
                          PUNICODE_STRING String)
    {
    return w2kCallNT ("RtlInt64ToUnicodeString",
                      16, Value, Base, String);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlIntegerToUnicodeString (DWORD           Value,
                            DWORD           Base,
                            PUNICODE_STRING String)
    {
    return w2kCallNT ("RtlIntegerToUnicodeString",
                      12, Value, Base, String);
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlMoveMemory (PVOID  Destination,
                PVOID  Source,
                SIZE_T Length)
    {
    w2kCallV (NULL, "RtlMoveMemory", FALSE,
              12, Destination, Source, Length);
    return;
    }

// -----------------------------------------------------------------

BOOLEAN WINAPI
_RtlPrefixUnicodeString (PUNICODE_STRING String1,
                         PUNICODE_STRING String2,
                         BOOLEAN         CaseInSensitive)
    {
    return w2kCall08 (FALSE, "RtlPrefixUnicodeString", FALSE,
                      12, String1, String2, CaseInSensitive);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlUnicodeStringToAnsiString (PANSI_STRING    Destination,
                               PUNICODE_STRING Source,
                               BOOLEAN         AllocateDestination)
    {
    return w2kCallNT ("RtlUnicodeStringToAnsiString",
                      12, Destination, Source, AllocateDestination);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlUnicodeStringToInteger (PUNICODE_STRING String,
                            DWORD           Base,
                            PDWORD          Value)
    {
    return w2kCallNT ("RtlUnicodeStringToInteger",
                      12, String, Base, Value);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_RtlUpcaseUnicodeString (PUNICODE_STRING Destination,
                         PUNICODE_STRING Source,
                         BOOLEAN         AllocateDestination)
    {
    return w2kCallNT ("RtlUpcaseUnicodeString",
                      12, Destination, Source, AllocateDestination);
    }

// -----------------------------------------------------------------

VOID WINAPI
_RtlZeroMemory (PVOID  Destination,
                SIZE_T Length)
    {
    w2kCallV (NULL, "RtlZeroMemory", FALSE,
              8, Destination, Length);
    return;
    }

// =================================================================
// SECURITY
// =================================================================

PACL WINAPI
_SePublicDefaultDacl (VOID)
    {
    return w2kCopyP (NULL, "SePublicDefaultDacl");
    }

// -----------------------------------------------------------------

PACL WINAPI
_SeSystemDefaultDacl (VOID)
    {
    return w2kCopyP (NULL, "SeSystemDefaultDacl");
    }

// =================================================================
// TRANSPORT DRIVER INTERFACE (TDI)
// =================================================================

NTSTATUS WINAPI
_TdiDeregisterPnPHandlers (HANDLE BindingHandle)
    {
    return w2kCallNT ("tdi.sys!TdiDeregisterPnPHandlers",
                      4, BindingHandle);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_TdiEnumerateAddresses (HANDLE BindingHandle)
    {
    return w2kCallNT ("tdi.sys!TdiEnumerateAddresses",
                      4, BindingHandle);
    }

// -----------------------------------------------------------------

NTSTATUS WINAPI
_TdiRegisterPnPHandlers (PTDI_CLIENT_INTERFACE_INFO Info,
                         DWORD                      InfoSize,
                         PHANDLE                    BindingHandle)
    {
    return w2kCallNT ("tdi.sys!TdiRegisterPnPHandlers",
                      12, Info, InfoSize, BindingHandle);
    }

// =================================================================
// NATIVE API
// =================================================================

NTSTATUS WINAPI
_ZwClose (HANDLE Handle)
    {
    return w2kCallNT ("ZwClose",
                      4, Handle);
    }

// =================================================================
// OBJECT TYPES
// =================================================================

POBJECT_TYPE WINAPI
__CmpKeyObjectType (VOID)
    {
    return w2kXCopyP (NULL, "CmpKeyObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ExCallbackObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ExCallbackObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_ExDesktopObjectType (VOID)
    {
    return w2kCopyP (NULL, "ExDesktopObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_ExEventObjectType (VOID)
    {
    return w2kCopyP (NULL, "ExEventObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ExEventPairObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ExEventPairObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ExMutantObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ExMutantObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ExProfileObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ExProfileObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_ExSemaphoreObjectType (VOID)
    {
    return w2kCopyP (NULL, "ExSemaphoreObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ExTimerObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ExTimerObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_ExWindowStationObjectType (VOID)
    {
    return w2kCopyP (NULL, "ExWindowStationObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_IoAdapterObjectType (VOID)
    {
    return w2kCopyP (NULL, "IoAdapterObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__IoCompletionObjectType (VOID)
    {
    return w2kXCopyP (NULL, "IoCompletionObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__IoControllerObjectType (VOID)
    {
    return w2kXCopyP (NULL, "IoControllerObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_IoDeviceObjectType (VOID)
    {
    return w2kCopyP (NULL, "IoDeviceObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_IoDriverObjectType (VOID)
    {
    return w2kCopyP (NULL, "IoDriverObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_IoFileObjectType (VOID)
    {
    return w2kCopyP (NULL, "IoFileObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_LpcPortObjectType (VOID)
    {
    return w2kCopyP (NULL, "LpcPortObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__LpcWaitablePortObjectType (VOID)
    {
    return w2kXCopyP (NULL, "LpcWaitablePortObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_MmSectionObjectType (VOID)
    {
    return w2kCopyP (NULL, "MmSectionObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ObpDirectoryObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ObpDirectoryObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ObpSymbolicLinkObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ObpSymbolicLinkObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__ObpTypeObjectType (VOID)
    {
    return w2kXCopyP (NULL, "ObpTypeObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_PsJobType (VOID)
    {
    return w2kCopyP (NULL, "PsJobType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_PsProcessType (VOID)
    {
    return w2kCopyP (NULL, "PsProcessType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
_PsThreadType (VOID)
    {
    return w2kCopyP (NULL, "PsThreadType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__SepTokenObjectType (VOID)
    {
    return w2kXCopyP (NULL, "SepTokenObjectType");
    }

// -----------------------------------------------------------------

POBJECT_TYPE WINAPI
__WmipGuidObjectType (VOID)
    {
    return w2kXCopyP (NULL, "WmipGuidObjectType");
    }

// =================================================================
// DLL MANAGEMENT
// =================================================================

BOOL WINAPI DllMain (HINSTANCE hInstance,
                     DWORD     dReason,
                     PVOID     pReserved)
    {
    BOOL fOk = TRUE;

    switch (dReason)
        {
        case DLL_PROCESS_ATTACH:
            {
            InitializeCriticalSection (gpcs = &gcs);
            ghInstance = hInstance;
            break;
            }
        case DLL_PROCESS_DETACH:
            {
            gpit = imgMemoryDestroy (gpit);

            if (gpcs != NULL)
                {
                DeleteCriticalSection (gpcs);
                gpcs = NULL;
                }
            w2kSpyCleanup (FALSE);
            break;
            }
        }
    return fOk;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
