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

PWSTR CopyEnvironment(HANDLE hProcess)
{
    PWSTR env = GetEnvironmentStringsW();

    ULONG n; 
    for (n = 0; env[n] != 0; n += wcslen(env + n) + 1) ; n *= sizeof *env;

    ULONG m = n;
    PVOID p = 0;
    NT::ZwAllocateVirtualMemory(hProcess, &p, 0, &m, MEM_COMMIT, PAGE_READWRITE);

    NT::ZwWriteVirtualMemory(hProcess, p, env, n, 0);

    return PWSTR(p);
}

VOID CreateProcessParameters(HANDLE hProcess, NT::PPEB Peb, NT::PUNICODE_STRING ImageFile)
{
    NT::PPROCESS_PARAMETERS pp;

    NT::RtlCreateProcessParameters(&pp, ImageFile, 0, 0, 0, 0, 0, 0, 0, 0);

    pp->Environment = CopyEnvironment(hProcess);

    ULONG n = pp->Size;
    PVOID p = 0;
    NT::ZwAllocateVirtualMemory(hProcess, &p, 0, &n, MEM_COMMIT, PAGE_READWRITE);

    NT::ZwWriteVirtualMemory(hProcess, p, pp, pp->Size, 0);

    NT::ZwWriteVirtualMemory(hProcess, PCHAR(Peb) + 0x10, &p, sizeof p, 0);

    NT::RtlDestroyProcessParameters(pp);
}

int exec(NT::PUNICODE_STRING name)
{
    HANDLE hProcess, hThread, hSection, hFile;

    NT::OBJECT_ATTRIBUTES oa = {sizeof oa, 0, name, OBJ_CASE_INSENSITIVE};
    NT::IO_STATUS_BLOCK iosb;
    NT::ZwOpenFile(&hFile, FILE_EXECUTE | SYNCHRONIZE, &oa, &iosb,
                   FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);

    oa.ObjectName = 0;

    NT::ZwCreateSection(&hSection, SECTION_ALL_ACCESS, &oa, 0, PAGE_EXECUTE, SEC_IMAGE, hFile);

    NT::ZwClose(hFile);

    NT::ZwCreateProcess(&hProcess, PROCESS_ALL_ACCESS, &oa,
                        NtCurrentProcess(), TRUE, hSection, 0, 0);

    NT::SECTION_IMAGE_INFORMATION sii;
    NT::ZwQuerySection(hSection, NT::SectionImageInformation, &sii, sizeof sii, 0);

    NT::ZwClose(hSection);

    NT::USER_STACK stack = {0};

    ULONG n = sii.StackReserve;
    NT::ZwAllocateVirtualMemory(hProcess, &stack.ExpandableStackBottom, 0, &n,
                                MEM_RESERVE, PAGE_READWRITE);

    stack.ExpandableStackBase = PCHAR(stack.ExpandableStackBottom) + sii.StackReserve;
    stack.ExpandableStackLimit = PCHAR(stack.ExpandableStackBase) - sii.StackCommit;

    n = sii.StackCommit + PAGE_SIZE;
    PVOID p = PCHAR(stack.ExpandableStackBase) - n;
    NT::ZwAllocateVirtualMemory(hProcess, &p, 0, &n, MEM_COMMIT, PAGE_READWRITE);

    ULONG x; n = PAGE_SIZE;
    NT::ZwProtectVirtualMemory(hProcess, &p, &n, PAGE_READWRITE | PAGE_GUARD, &x);

    NT::CONTEXT context = {CONTEXT_FULL};
    context.SegGs = 0;
    context.SegFs = 0x38;
    context.SegEs = 0x20;
    context.SegDs = 0x20;
    context.SegSs = 0x20;
    context.SegCs = 0x18;
    context.EFlags = 0x3000;
    context.Esp = ULONG(stack.ExpandableStackBase) - 4;
    context.Eip = ULONG(sii.EntryPoint);

    NT::CLIENT_ID cid;

    NT::ZwCreateThread(&hThread, THREAD_ALL_ACCESS, &oa, hProcess, &cid, &context, &stack, TRUE);

    NT::PROCESS_BASIC_INFORMATION pbi;
    NT::ZwQueryInformationProcess(hProcess, NT::ProcessBasicInformation, &pbi, sizeof pbi, 0);

    CreateProcessParameters(hProcess, pbi.PebBaseAddress, name);

    InformCsrss(hProcess, hThread, ULONG(cid.UniqueProcess), ULONG(cid.UniqueThread));

    NT::ZwResumeThread(hThread, 0);

    NT::ZwClose(hProcess);
    NT::ZwClose(hThread);

    return int(cid.UniqueProcess);
}

extern "C"
int wmain(int argc, wchar_t *argv[])
{
    NT::UNICODE_STRING ImageFile;
    NT::RtlInitUnicodeString(&ImageFile, argv[1]);

    exec(&ImageFile);

    return 0;
}
