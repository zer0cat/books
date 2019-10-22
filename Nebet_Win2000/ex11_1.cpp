#include "ntdll.h"
#include <stdio.h>
#include <imagehlp.h>

HANDLE hWakeup;

BOOL EnablePrivilege(PCSTR name)
{
    TOKEN_PRIVILEGES priv = {1, {0, 0, SE_PRIVILEGE_ENABLED}};
    LookupPrivilegeValue(0, name, &priv.Privileges[0].Luid);

    HANDLE hToken;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);

    AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof priv, 0, 0);
    BOOL rv = GetLastError() == ERROR_SUCCESS;

    CloseHandle(hToken);
    return rv;
}

PULONG LoadDrivers()
{
    ULONG n = 0x1000;
    PULONG p = new ULONG[n];

    while (NT::ZwQuerySystemInformation(NT::SystemModuleInformation, p, n, 0)
           == STATUS_INFO_LENGTH_MISMATCH)
        delete [] p, p = new ULONG[n = n * 2];
    return p;
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    return dwCtrlType == CTRL_C_EVENT ? SetEvent(hWakeup) : FALSE;
}


int main()
{
    ULONG shift = 3;

    EnablePrivilege(SE_SYSTEM_PROFILE_NAME);

    PULONG modules = LoadDrivers();

    NT::ZwSetIntervalProfile(10000, NT::ProfileTime);

    NT::PSYSTEM_MODULE_INFORMATION m = NT::PSYSTEM_MODULE_INFORMATION(modules + 1);

    PHANDLE h = new HANDLE[*modules];

    PULONG* p = new PULONG[*modules];

    for (ULONG i = 0; i < *modules; i++) {

        ULONG n = (m[i].Size >> (shift - 2)) + 1;

        p[i] = PULONG(VirtualAlloc(0, n, MEM_COMMIT, PAGE_READWRITE));

        NT::ZwCreateProfile(h + i, 0, m[i].Base, m[i].Size, shift, p[i], n, NT::ProfileTime, 0);

        NT::ZwStartProfile(h[i]);
    }

    hWakeup = CreateEvent(0, FALSE, FALSE, 0);

    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    printf("collecting...\n");

    WaitForSingleObject(hWakeup, INFINITE);

    for (i = 0; i < *modules; i++) {

        NT::ZwStopProfile(h[i]);

        CloseHandle(h[i]);
    }

    SymInitialize(0, 0, FALSE);
    SymSetOptions(SymGetOptions() | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);

    for (i = 0; i < *modules; i++) {

        SymLoadModule(0, 0, m[i].ImageName, m[i].ImageName + m[i].ModuleNameOffset,
                      ULONG(m[i].Base), m[i].Size);

        printf("%s\n", m[i].ImageName + m[i].ModuleNameOffset);

        ULONG n = (m[i].Size >> shift) + 1;

        for (ULONG j = 0; j < n; j++) {

            if (p[i][j] != 0) {

                IMAGEHLP_SYMBOL symbol[10];

                symbol[0].SizeOfStruct = sizeof symbol[0];
                symbol[0].MaxNameLength = sizeof symbol - sizeof symbol[0];

                ULONG disp = 0;

                SymGetSymFromAddr(0, ULONG(m[i].Base) + (j << shift), &disp, symbol);

                printf("%6ld %s+0x%lx\n", p[i][j], symbol[0].Name, disp);
            }
        }

        SymUnloadModule(0, ULONG(m[i].Base));

        VirtualFree(p[i], 0, MEM_RELEASE);
    }

    SymCleanup(0);

    delete [] m;
    delete [] h;
    delete [] p;

    return 0;
} 
