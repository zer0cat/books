#include "ntdll.h"
#include <stdio.h>

namespace NT {
    extern "C" {

NTSTATUS
NTAPI
CsrClientCallServer(
    IN PVOID Message,
    IN PVOID,
    IN ULONG Opcode,
    IN ULONG Size
    );

    }
}

VOID InheritAll()
{
    ULONG n = 0x1000;
    PULONG p = new ULONG[n];

    while (NT::ZwQuerySystemInformation(NT::SystemHandleInformation, p, n * sizeof *p, 0)
           == STATUS_INFO_LENGTH_MISMATCH)
        delete [] p, p = new ULONG[n *= 2];

    NT::PSYSTEM_HANDLE_INFORMATION h = NT::PSYSTEM_HANDLE_INFORMATION(p + 1);

    ULONG pid = GetCurrentProcessId();

    for (ULONG i = 0; i < *p; i++)
        if (h[i].ProcessId == pid)
            SetHandleInformation(HANDLE(h[i].Handle), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    delete [] p;
}

VOID InformCsrss(HANDLE hProcess, HANDLE hThread, ULONG pid, ULONG tid)
{
    struct CSRSS_MESSAGE {
        ULONG Unknown1;
        ULONG Opcode;
        ULONG Status;
        ULONG Unknown2;
    };

    struct {
        NT::PORT_MESSAGE PortMessage;
        CSRSS_MESSAGE CsrssMessage;
        PROCESS_INFORMATION ProcessInformation;
        NT::CLIENT_ID Debugger;
        ULONG CreationFlags;
        ULONG VdmInfo[2];
    } csrmsg = {{0}, {0}, {hProcess, hThread, pid, tid}, {0}, 0, {0}};

    NT::CsrClientCallServer(&csrmsg, 0, 0x10000, 0x24);
}

__declspec(naked) int child()
{
    typedef BOOL (WINAPI *CsrpConnectToServer)(PWSTR);

    // CsrpConnectToServer(0x77F68CC0)(L"\\Windows");
    // CsrpConnectToServer(0x77F8F65D)(L"\\Windows");
    CsrpConnectToServer(0x77F922F5)(L"\\Windows");

    __asm mov   eax, 0
    __asm mov   esp, ebp
    __asm pop   ebp
    __asm ret
}


#pragma optimize("y", off)  // disable frame pointer omission

int fork()
{
    HANDLE hProcess, hThread;

    InheritAll();

    NT::OBJECT_ATTRIBUTES oa = {sizeof oa};

    NT::ZwCreateProcess(&hProcess, PROCESS_ALL_ACCESS, &oa, NtCurrentProcess(), TRUE, 0, 0, 0);

    NT::CONTEXT context = {CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS | CONTEXT_FLOATING_POINT};

    NT::ZwGetContextThread(NtCurrentThread(), &context);

    context.Eip = ULONG(child);

    MEMORY_BASIC_INFORMATION mbi;

    NT::ZwQueryVirtualMemory(NtCurrentProcess(), PVOID(context.Esp),
                             NT::MemoryBasicInformation, &mbi, sizeof mbi, 0);

    NT::USER_STACK stack = {0, 0, PCHAR(mbi.BaseAddress) + mbi.RegionSize,
                            mbi.BaseAddress, mbi.AllocationBase};

    NT::CLIENT_ID cid;

    NT::ZwCreateThread(&hThread, THREAD_ALL_ACCESS, &oa,
                       hProcess, &cid, &context, &stack, TRUE);

    NT::THREAD_BASIC_INFORMATION tbi;

    NT::ZwQueryInformationThread(NtCurrentThread(), NT::ThreadBasicInformation,
                                 &tbi, sizeof tbi, 0);

    NT::PNT_TIB tib = tbi.TebBaseAddress;

    NT::ZwQueryInformationThread(hThread, NT::ThreadBasicInformation, &tbi, sizeof tbi, 0);

    NT::ZwWriteVirtualMemory(hProcess, tbi.TebBaseAddress,
                             &tib->ExceptionList, sizeof tib->ExceptionList, 0);

    InformCsrss(hProcess, hThread, ULONG(cid.UniqueProcess), ULONG(cid.UniqueThread));

    NT::ZwResumeThread(hThread, 0);

    NT::ZwClose(hThread);
    NT::ZwClose(hProcess);

    return int(cid.UniqueProcess);
}

#pragma optimize("", on)


int main()
{
    int n = fork();
    Sleep(n * 10);
    Beep(100, 100);
    printf("%d\n", n);
    return 0;
}
