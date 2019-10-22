#include "ntdll.h"
#include "windbgkd.h"
#include <stdio.h>

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

int main()
{
    DBGKD_GET_INTERNAL_BREAKPOINT bp[20];
    ULONG n;

    EnablePrivilege(SE_DEBUG_NAME);

    NT::ZwSystemDebugControl(NT::DebugGetTraceInformation, 0, 0, bp, sizeof bp, &n);

    for (int i = 0; i * sizeof (DBGKD_GET_INTERNAL_BREAKPOINT) < n; i++)

        printf("%lx %lx %ld %ld %ld %ld %ld\n",
               bp[i].BreakpointAddress, bp[i].Flags,
               bp[i].Calls, bp[i].MaxCallsPerPeriod,
               bp[i].MinInstructions, bp[i].MaxInstructions,
               bp[i].TotalInstructions);

    return 0;
}
