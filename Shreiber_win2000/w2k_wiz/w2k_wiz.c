
// __________________________________________________________
//
//                         w2k_wiz.c
//             SBS Windows 2000 Code Wizard V1.00
//                08-27-2000 Sven B. Schreiber
//                       sbs@orgon.com
// __________________________________________________________

#include "w2k_wiz.h"

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
// MESSAGE TEXTS
// =================================================================

TBYTE gatAborted [] = T("\r\nInitialization error\r\n");
TBYTE gatTarget  [] = T("\r\nProject %s\r\n");
TBYTE gatLoading [] = T("\r\nLoading %s ... ");
TBYTE gatWriting [] = T(    "Writing %s ... ");
TBYTE gatOk      [] = T("OK");
TBYTE gatError   [] = T("ERROR");
TBYTE gatLine    [] = T("%s\r\n");

// =================================================================
// GLOBAL VARIABLES
// =================================================================

BYTE gabName     [MAX_PATH];  // %n = project name (canonical)
BYTE gabNameU    [MAX_PATH];  // %N = project name (upper case)
BYTE gabSettings [MAX_PATH];  // %s = settings path

BYTE gabDay      [N_DECIMAL]; // %d = current day
BYTE gabMonth    [N_DECIMAL]; // %m = current month
BYTE gabYear     [N_DECIMAL]; // %y = current year

BYTE gabText     [N_VALUE];   // %t = default description
BYTE gabCompany  [N_VALUE];   // %c = author's company
BYTE gabAuthor   [N_VALUE];   // %a = author's name
BYTE gabEmail    [N_VALUE];   // %e = author's email address
BYTE gabPrefix   [N_VALUE];   // %p = author's ProgID prefix

BYTE gabInclude  [N_VALUE];   // %i = DDK include path
BYTE gabFree     [N_VALUE];   // %l = DDK library path (release)
BYTE gabChecked  [N_VALUE];   // %L = DDK library path (debug)

// =================================================================
// REGISTRY MANAGEMENT
// =================================================================

BOOL WINAPI RegistryRoot (PTBYTE ptRoot,
                          PHKEY  phk)
    {
    BOOL fOk = FALSE;

    if (phk != NULL)
        {
        if      (!lstrcmpi (ptRoot, T("HKEY_CLASSES_ROOT")))
            {
            *phk = HKEY_CLASSES_ROOT;
            }
        else if (!lstrcmpi (ptRoot, T("HKEY_CURRENT_USER")))
            {
            *phk = HKEY_CURRENT_USER;
            }
        else if (!lstrcmpi (ptRoot, T("HKEY_LOCAL_MACHINE")))
            {
            *phk = HKEY_LOCAL_MACHINE;
            }
        else if (!lstrcmpi (ptRoot, T("HKEY_USERS")))
            {
            *phk = HKEY_USERS;
            }
        else if (!lstrcmpi (ptRoot, T("HKEY_PERFORMANCE_DATA")))
            {
            *phk = HKEY_PERFORMANCE_DATA;
            }
        else if (!lstrcmpi (ptRoot, T("HKEY_CURRENT_CONFIG")))
            {
            *phk = HKEY_CURRENT_CONFIG;
            }
        else if (!lstrcmpi (ptRoot, T("HKEY_DYN_DATA")))
            {
            *phk = HKEY_DYN_DATA;
            }
        else
            {
            *phk = INVALID_HKEY;
            }
        fOk = (*phk != INVALID_HKEY);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI RegistryParse (PTBYTE  ptPath,
                           PHKEY   phkRoot,
                           PPTBYTE pptKey,
                           PPTBYTE pptValue)
    {
    DWORD dStop;
    HKEY  hkRoot = INVALID_HKEY;
    DWORD dKey   = 0;
    DWORD dValue = 0;

    if (ptPath != NULL)
        {
        while (ptPath [dKey] && (ptPath [dKey] != '\\'))
              dKey++;

        if (ptPath [dKey]) ptPath [dKey++] = 0;

        dValue = dStop = dKey + lstrlen (ptPath + dKey);

        while ((dValue > dKey) && (ptPath [dValue-1] != '\\'))
              dValue--;

        if (dValue > dKey) ptPath [dValue-1] = 0; else dKey = dStop;

        RegistryRoot (ptPath, &hkRoot);
        }
    if (phkRoot  != NULL) *phkRoot  = hkRoot;
    if (pptKey   != NULL) *pptKey   = ptPath + dKey;
    if (pptValue != NULL) *pptValue = ptPath + dValue;
    return (hkRoot != INVALID_HKEY);
    }

// -----------------------------------------------------------------

DWORD WINAPI RegistryRead (PTBYTE ptPath,
                           PTBYTE ptBuffer,
                           DWORD  dBuffer)
    {
    HKEY   hkRoot, hk;
    PTBYTE ptKey, ptValue;
    DWORD  dType;
    DWORD  n = 0;

    if ((ptBuffer != NULL) && dBuffer)
        {
        if (RegistryParse (ptPath, &hkRoot, &ptKey, &ptValue)
            &&
            (RegOpenKeyEx (hkRoot, ptKey, 0, KEY_READ, &hk)
             == ERROR_SUCCESS))
            {
            n = dBuffer * sizeof (TBYTE);

            if ((RegQueryValueEx (hk, ptValue, NULL,
                                  &dType, (PVOID) ptBuffer, &n)
                 == ERROR_SUCCESS)
                &&
                (dType == REG_SZ))
                {
                if (n /= sizeof (TBYTE)) n--;
                }
            else
                {
                n = 0;
                }
            RegCloseKey (hk);
            }
        ptBuffer [n] = 0;
        }
    return n;
    }

// =================================================================
// FILE MANAGEMENT
// =================================================================

DWORD WINAPI FileRoot (PTBYTE ptPath)
    {
    DWORD dRoot = 0;

    if ((ptPath != NULL) && ptPath [0])
        {
        if (ptPath [0] == '\\')
            {
            if (ptPath [1] == '\\')
                {
                for (dRoot = 2;
                     ptPath [dRoot] && (ptPath [dRoot] != '\\');
                     dRoot++);
                }
            }
        else
            {
            if (ptPath [1] == ':') dRoot = 2;
            }
        }
    return dRoot;
    }

// -----------------------------------------------------------------

HANDLE WINAPI FileOpen (PTBYTE ptPath)
    {
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((ptPath != NULL) && ptPath [0])
        {
        hf = CreateFile (ptPath, GENERIC_READ,
                         FILE_SHARE_READ, NULL, OPEN_EXISTING,
                         FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI FileNew (PTBYTE ptPath)
    {
    DWORD  i;
    HANDLE hf = INVALID_HANDLE_VALUE;

    if ((ptPath != NULL) && ptPath [0])
        {
        i = FileRoot (ptPath);

        while (ptPath [i])
            {
            while (ptPath [i] && (ptPath [++i] != '\\'));

            if (ptPath [i] == '\\')
                {
                ptPath [i] = 0;
                CreateDirectory (ptPath, NULL);
                ptPath [i] = '\\';
                }
            }
        hf = CreateFile (ptPath, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                         FILE_FLAG_SEQUENTIAL_SCAN |
                         FILE_ATTRIBUTE_NORMAL, NULL);
        }
    return hf;
    }

// -----------------------------------------------------------------

HANDLE WINAPI FileClose (HANDLE hf)
    {
    if (hf != INVALID_HANDLE_VALUE) CloseHandle (hf);
    return INVALID_HANDLE_VALUE;
    }

// -----------------------------------------------------------------

PBYTE WINAPI FileUnload (PBYTE pbData)
    {
    if (pbData != NULL) LocalFree (pbData);
    return NULL;
    }

// -----------------------------------------------------------------

PBYTE WINAPI FileLoad (PTBYTE ptPath,
                       PDWORD pdData)
    {
    HANDLE hf;
    DWORD  n;
    DWORD  dData  = 0;
    PBYTE  pbData = NULL;

    if ((hf = FileOpen (ptPath)) != INVALID_HANDLE_VALUE)
        {
        dData = GetFileSize (hf, NULL);

        if ((dData != INVALID_FILE_SIZE) && (dData < MAXDWORD) &&
            ((pbData = LocalAlloc (LMEM_FIXED, dData+1)) != NULL))
            {
            if (ReadFile (hf, pbData, dData, &n, NULL) &&
                (dData == n))
                {
                pbData [dData] = 0;
                }
            else
                {
                pbData = FileUnload (pbData);
                }
            }
        FileClose (hf);

        if (pbData == NULL) dData = 0;
        }
    if (pdData != NULL) *pdData = dData;
    return pbData;
    }

// -----------------------------------------------------------------

BOOL WINAPI FileWrite (HANDLE hf,
                       PBYTE  pbData,
                       DWORD  dData)
    {
    DWORD dData1, n;
    BOOL  fOk = FALSE;

    if ((hf != INVALID_HANDLE_VALUE) && (pbData != NULL))
        {
        dData1 = (dData != MAXDWORD ? dData : lstrlenA (pbData));

        fOk = WriteFile (hf, pbData, dData1, &n, NULL) &&
              (dData1 == n);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI FileEscape (HANDLE hf,
                        BYTE   bEscape)
    {
    BYTE  abData [] = "% ";
    PBYTE pbData    = abData;

    switch (abData [1] = bEscape)
        {
        case 'n': pbData = gabName;     break;
        case 'N': pbData = gabNameU;    break;
        case 's': pbData = gabSettings; break;

        case 'd': pbData = gabDay;      break;
        case 'm': pbData = gabMonth;    break;
        case 'y': pbData = gabYear;     break;

        case 't': pbData = gabText;     break;
        case 'c': pbData = gabCompany;  break;
        case 'a': pbData = gabAuthor;   break;
        case 'e': pbData = gabEmail;    break;
        case 'p': pbData = gabPrefix;   break;

        case 'i': pbData = gabInclude;  break;
        case 'l': pbData = gabFree;     break;
        case 'L': pbData = gabChecked;  break;
        }
    return FileWrite (hf, pbData, MAXDWORD);
    }

// -----------------------------------------------------------------

BOOL WINAPI FileCopy (PTBYTE ptSource,
                      PTBYTE ptTarget,
                      BOOL   fCustomize)
    {
    PBYTE  pbData;
    DWORD  dData, i, n;
    HANDLE hf;
    BOOL   fOk = FALSE;

    printf (gatLoading, ptSource);
    pbData = FileLoad (ptSource, &dData);
    printf (gatLine, (pbData != NULL ? gatOk : gatError));

    if (pbData != NULL)
        {
        printf (gatWriting, ptTarget);

        if ((hf = FileNew (ptTarget)) != INVALID_HANDLE_VALUE)
            {
            if (fCustomize)
                {
                for (n = i = 0; i < dData; i++)
                    {
                    if ((pbData [i] == '%') && (i+1 < dData))
                        {
                        if ((!FileWrite  (hf, pbData+n, (i++)-n)) ||
                            (!FileEscape (hf, pbData [i])))
                            break;

                        n = i+1;
                        }
                    }
                fOk = (i == dData) && FileWrite (hf, pbData+n, i-n);
                }
            else
                {
                fOk = FileWrite (hf, pbData, dData);
                }
            FileClose (hf);
            }
        printf (gatLine, (fOk ? gatOk : gatError));

        FileUnload (pbData);
        }
    return fOk;
    }

// -----------------------------------------------------------------

BOOL WINAPI FileCopyEx (PTBYTE ptFolder,
                        PTBYTE ptSource,
                        PTBYTE ptTarget,
                        BOOL   fCustomize)
    {
    TBYTE atSource [MAX_PATH];
    TBYTE atTarget [MAX_PATH];
    DWORD i, j, n;
    BOOL  fOk = FALSE;

    if ((ptFolder != NULL) &&
        (ptSource != NULL) && (ptTarget != NULL) &&
        (i = GetModuleFileName (NULL, atSource, MAX_PATH)))
        {
        while (i && (atSource [i-1] != '\\')
                 && (atSource [i-1] != ':' )
                 && (atSource [--i] != '.' ));

        if ((atSource [i] == '.') &&
            (i + lstrlen (ptSource) < MAX_PATH))
            {
            lstrcpy (atSource+i, ptSource);

            if (n = lstrlen (ptFolder))
                {
                i = j = n-1;

                while (i && (ptFolder [i-1] != '\\')
                         && (ptFolder [i-1] != ':' )) i--;

                if ((j -= i) &&
                    (n+j + lstrlen (ptTarget) < MAX_PATH))
                    {
                    lstrcpy  (atTarget,     ptFolder);
                    lstrcpyn (atTarget+n,   ptFolder+i, j+1);
                    lstrcpy  (atTarget+n+j, ptTarget);

                    fOk = FileCopy (atSource, atTarget, fCustomize);
                    }
                }
            }
        }
    return fOk;
    }

// =================================================================
// SETTINGS MANAGEMENT
// =================================================================

DWORD WINAPI SettingsFolder (PTBYTE ptRoot,
                             PTBYTE ptFolder,
                             PTBYTE ptBuffer,
                             DWORD  dBuffer)
    {
    TBYTE  atPath [MAX_PATH];
    PTBYTE ptFile;
    DWORD  i, j, k;
    DWORD  n = 0;

    if ((ptBuffer != NULL) && dBuffer)
        {
        i = 0;

        if ((ptFolder == NULL) || (!ptFolder [i]))
            {
            lstrcpyn (atPath, T("."), MAX_PATH);
            }
        else if (ptFolder [i] != '*')
            {
            lstrcpyn (atPath, ptFolder, MAX_PATH);
            }
        else
            {
            if (ptFolder [++i]                                &&
                ((ptFolder [i] != '\\') || ptFolder [++i])    &&
                (j = RegistryRead (ptRoot, atPath, MAX_PATH)) &&
                ((atPath [j-1] == '\\') || (++j < MAX_PATH))  &&
                (j + lstrlen (ptFolder+i) < MAX_PATH))
                {
                atPath [j-1] = '\\';
                lstrcpy (atPath+j, ptFolder+i);
                }
            else
                {
                atPath [0] = 0;
                }
            }
        if (atPath [0])
            {
            n = GetFullPathName (atPath, dBuffer, ptBuffer,
                                 &ptFile);
            }
        if (n && (n < MAX_PATH) &&
            ((ptBuffer [n-1] == '\\') || (++n < MAX_PATH)))
            {
            ptBuffer [n-1] = '\\';
            }
        else
            {
            n = 0;
            }
        i = j = (n ? n-1 : 0);

        while (i && (ptBuffer [i-1] != '\\')
                 && (ptBuffer [i-1] != ':' )) i--;

        if ((j-i) && (j-i < MAX_PATH))
            {
            for (k = 0; k < j-i; k++)
                {
                if (ptBuffer [i+k] < 0x0100)
                    {
                    gabName [k] = (BYTE) ptBuffer [i+k];
                    }
                else
                    {
                    sprintfA (gabName+k, "%C", ptBuffer [i+k]);
                    }
                }
            gabName [k] = 0;

            lstrcpyA   (gabNameU, gabName);
            CharUpperA (gabNameU);
            }
        ptBuffer [n] = 0;
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI SettingsValue (PBYTE pbKey,
                            PBYTE pbValue)
    {
    DWORD i;
    DWORD n = 0;

    if (pbValue != NULL)
        {
        if (pbKey != NULL)
            {
            GetPrivateProfileStringA ("Settings", pbKey, "",
                                      pbValue, N_VALUE,
                                      gabSettings);

            for (i = 0; pbValue [i] == ' '; i++);
            while (pbValue [i]) pbValue [n++] = pbValue [i++];
            while (n && (pbValue [n-1] == ' ')) n--;
            }
        pbValue [n] = 0;
        }
    return n;
    }

// -----------------------------------------------------------------

DWORD WINAPI SettingsLoad (PBYTE  pbExtension,
                           PTBYTE ptFolder,
                           PTBYTE ptBuffer,
                           DWORD  dBuffer)
    {
    BYTE       abRoot [N_VALUE];
    TBYTE      atRoot [N_VALUE];
    SYSTEMTIME st;
    DWORD      i;
    DWORD      n = 0;

    if ((ptBuffer != NULL) && dBuffer)
        {
        if ((pbExtension != NULL) &&
            (i = GetModuleFileNameA (NULL, gabSettings, MAX_PATH)))
            {
            while (i && (gabSettings [i-1] != '\\')
                     && (gabSettings [i-1] != ':' )
                     && (gabSettings [--i] != '.' ));

            if ((gabSettings [i] == '.') &&
                (i + lstrlenA (pbExtension) < MAX_PATH))
                {
                lstrcpyA (gabSettings+i, pbExtension);

                GetLocalTime (&st);

                sprintfA (gabDay,   "%02hu", st.wDay  );
                sprintfA (gabMonth, "%02hu", st.wMonth);
                sprintfA (gabYear,  "%04hu", st.wYear );

                if (SettingsValue ("Text",    gabText   ) &&
                    SettingsValue ("Company", gabCompany) &&
                    SettingsValue ("Author",  gabAuthor ) &&
                    SettingsValue ("Email",   gabEmail  ) &&
                    SettingsValue ("Prefix",  gabPrefix ) &&
                    SettingsValue ("Include", gabInclude) &&
                    SettingsValue ("Free",    gabFree   ) &&
                    SettingsValue ("Checked", gabChecked) &&
                    SettingsValue ("Root",     abRoot   ))
                    {
                    for (i = 0; i < N_VALUE; i++)
                        {
                        if (!(atRoot [i] = abRoot [i])) break;
                        }
                    n = SettingsFolder (atRoot, ptFolder,
                                        ptBuffer, dBuffer);
                    }
                }
            }
        ptBuffer [n] = 0;
        }
    return n;
    }

// =================================================================
// MAIN PROGRAM
// =================================================================

DWORD Main (DWORD argc, PTBYTE *argv, PTBYTE *argp)
    {
    TBYTE atFolder [MAX_PATH];

    printf (atAbout);

    if (argc < 2)
        {
        printf (atUsage, T("[*]<folder>"));
        }
    else
        {
        if (SettingsLoad (".ini", argv [1], atFolder, MAX_PATH))
            {
            printf (gatTarget, atFolder);

            FileCopyEx (atFolder, T(".tc"), T(".c"),   TRUE);
            FileCopyEx (atFolder, T(".td"), T(".def"), TRUE);
            FileCopyEx (atFolder, T(".th"), T(".h"),   TRUE);
            FileCopyEx (atFolder, T(".ti"), T(".ico"), FALSE);
            FileCopyEx (atFolder, T(".tp"), T(".dsp"), TRUE);
            FileCopyEx (atFolder, T(".tr"), T(".rc"),  TRUE);
            FileCopyEx (atFolder, T(".tw"), T(".dsw"), TRUE);
            }
        else
            {
            printf (gatAborted);
            }
        }
    return 0;
    }

// =================================================================
// END OF PROGRAM
// =================================================================
