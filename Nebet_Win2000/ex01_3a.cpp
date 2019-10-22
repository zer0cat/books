#include "ntdll.h"
#include <windbgkd.h>
#include <imagehlp.h>
#include <stdlib.h>

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

void LoadModules()
{
    ULONG n;
    NT::ZwQuerySystemInformation(NT::SystemModuleInformation, &n, 0, &n);
    PULONG p = new ULONG[n];
    NT::ZwQuerySystemInformation(NT::SystemModuleInformation, p, n * sizeof *p, 0);

    NT::PSYSTEM_MODULE_INFORMATION module = NT::PSYSTEM_MODULE_INFORMATION(p + 1);

    for (ULONG i = 0; i < *p; i++)
        SymLoadModule(0, 0, module[i].ImageName,
                      module[i].ImageName + module[i].ModuleNameOffset,
                      ULONG(module[i].Base), module[i].Size);

    delete [] p;
}

DWORD GetAddress(PSTR expr)
{
    PCHAR s;
    ULONG n = strtoul(expr, &s, 16);

    if (*s == 0) return n;

    IMAGEHLP_SYMBOL symbol;

    symbol.SizeOfStruct = sizeof symbol;
    symbol.MaxNameLength = sizeof symbol.Name;

    return SymGetSymFromName(0, expr, &symbol) == TRUE ? symbol.Address : 0;
}

void SetSpecialCall(DWORD addr)
{
    DBGKD_MANIPULATE_STATE64 op = {0};
    op.u.SetSpecialCall.SpecialCall = addr;

    NT::ZwSystemDebugControl(NT::DebugSetSpecialCall, &op, 4, 0, 0, 0);
}

void SetSpecialCalls()
{
    DBGKD_MANIPULATE_STATE64 op[4];

    NT::ZwSystemDebugControl(NT::DebugQuerySpecialCalls, 0, 0, op, sizeof op, 0);

    if (op[0].u.QuerySpecialCalls.NumberOfSpecialCalls == 0) {
        SetSpecialCall(GetAddress("HAL!KfLowerIrql"));
        SetSpecialCall(GetAddress("HAL!KfReleaseSpinLock"));
        SetSpecialCall(GetAddress("HAL!HalRequestSoftwareInterrupt"));
        SetSpecialCall(GetAddress("NTOSKRNL!SwapContext"));
        SetSpecialCall(GetAddress("NTOSKRNL!KiUnlockDispatcherDatabase"));
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) return 0;

    NT::SYSTEM_KERNEL_DEBUGGER_INFORMATION kd;

    NT::ZwQuerySystemInformation(NT::SystemKernelDebuggerInformation, &kd, sizeof kd, 0);
    if (kd.DebuggerEnabled == FALSE) return 0;

    EnablePrivilege(SE_DEBUG_NAME);

    SymInitialize(0, 0, FALSE);
    SymSetOptions(SymGetOptions() | SYMOPT_DEFERRED_LOADS);

    LoadModules();

    SetSpecialCalls();

    DBGKD_MANIPULATE_STATE64 op = {0};
    op.u.SetInternalBreakpoint.BreakpointAddress = GetAddress(argv[1]);
    op.u.SetInternalBreakpoint.Flags = argc < 3 ? 0 : strtoul(argv[2], 0, 16);

    NT::ZwSystemDebugControl(NT::DebugSetInternalBreakpoint, &op, sizeof op, 0, 0, 0);

    return 0;
}
