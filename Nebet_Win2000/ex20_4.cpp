#pragma warning(disable:4786) // identifier was truncated in the debug info

#include "ntdll.h"
#include <imagehlp.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>

#define elements(s) (sizeof (s) / sizeof *(s))

namespace NT {
    extern "C" {

typedef struct _DEBUG_MESSAGE {
    PORT_MESSAGE PortMessage;
    ULONG EventCode;
    ULONG Status;
    union {
        struct {
            EXCEPTION_RECORD ExceptionRecord;
            ULONG FirstChance;
        } Exception;
        struct {
            ULONG Reserved;
            PVOID StartAddress;
        } CreateThread;
        struct {
            ULONG Reserved;
            HANDLE FileHandle;
            PVOID Base;
            ULONG PointerToSymbolTable;
            ULONG NumberOfSymbols;
            ULONG Reserved2;
            PVOID EntryPoint;
        } CreateProcess;
        struct {
            ULONG ExitCode;
        } ExitThread;
        struct {
            ULONG ExitCode;
        } ExitProcess;
        struct {
            HANDLE FileHandle;
            PVOID Base;
            ULONG PointerToSymbolTable;
            ULONG NumberOfSymbols;
        } LoadDll;
        struct {
            PVOID Base;
        } UnloadDll;
    } u;
} DEBUG_MESSAGE, *PDEBUG_MESSAGE;

    }
}

typedef struct _DEBUG_STATUS {
    ULONG B0 : 1;
    ULONG B1 : 1;
    ULONG B2 : 1;
    ULONG B3 : 1;
    ULONG    : 9;
    ULONG BD : 1;
    ULONG BS : 1;
    ULONG BT : 1;
    ULONG    : 16;
} DEBUG_STATUS, *PDEBUG_STATUS;

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

struct Error {
    ULONG line;
    ULONG code;
    Error(ULONG line, ULONG code) : line(line), code(code) {}
};

struct enter {
    PCSTR name;
    BYTE opcode;
    LONG argc;
    enter() : name(0), opcode(0), argc(0) {}
    enter(PCSTR n, BYTE o = 0, ULONG a = 0) : name(n), opcode(o), argc(a) {}
};

struct leave {
    ULONG caller;
    ULONG callee;
    ULONG esp;
    leave() : caller(0), callee(0), esp(0) {}
    leave(ULONG from, ULONG to, ULONG sp) : caller(from), callee(to), esp(sp) {}
};

std::map<PVOID, enter> enters;
std::map<HANDLE, std::vector<leave> > leaves;
std::map<HANDLE, PVOID> steps;
std::map<HANDLE, HANDLE> threads;

HANDLE hDebuggee;
ULONG StartTime;

const unsigned char INT3 = 0xCC;
const int EXECUTE = PAGE_EXECUTE | PAGE_EXECUTE_READ
                  | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

BYTE GetByte(HANDLE hProcess, PVOID addr)
{
    BYTE byte;

    BOOL rv = ReadProcessMemory(hProcess, addr, &byte, sizeof byte, 0);
    if (rv != TRUE) throw Error(__LINE__, GetLastError());

    return byte;
}

VOID SetByte(HANDLE hProcess, PVOID addr, BYTE byte)
{
    BOOL rv = WriteProcessMemory(hProcess, addr, &byte, sizeof byte, 0);
    if (rv != TRUE) throw Error(__LINE__, GetLastError());
}

BYTE InsertBreakPoint(HANDLE hProcess, PVOID addr)
{
    MEMORY_BASIC_INFORMATION mbi;

    ULONG rv = VirtualQueryEx(hProcess, addr, &mbi, sizeof mbi);
    if (rv != sizeof mbi) return INT3;

    if ((mbi.Protect & EXECUTE) == 0) return INT3;

    BYTE op = GetByte(hProcess, addr);

    SetByte(hProcess, addr, INT3);

    return op;
}

VOID InsertBreakPoints(HANDLE hProcess, PVOID base)
{
    IMAGE_DOS_HEADER dos;
    IMAGE_NT_HEADERS nt;
    BOOL rv;

    rv = ReadProcessMemory(hProcess, base, &dos, sizeof dos, 0);
    if (rv != TRUE) throw Error(__LINE__, GetLastError());

    rv = ReadProcessMemory(hProcess, PBYTE(base) + dos.e_lfanew, &nt, sizeof nt, 0);
    if (rv != TRUE) throw Error(__LINE__, GetLastError());

    PIMAGE_DATA_DIRECTORY expdir = nt.OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT;
    ULONG size = expdir->Size;
    ULONG addr = expdir->VirtualAddress;

    if (size == 0) return;

    PIMAGE_EXPORT_DIRECTORY exports = PIMAGE_EXPORT_DIRECTORY(malloc(size));

    rv = ReadProcessMemory(hProcess, PBYTE(base) + addr, exports, size, 0);
    if (rv != TRUE) throw Error(__LINE__, GetLastError());

    PULONG functions =  PULONG(PBYTE(exports) - addr + ULONG(exports->AddressOfFunctions));
    PUSHORT ordinals = PUSHORT(PBYTE(exports) - addr + ULONG(exports->AddressOfNameOrdinals));
    PULONG fnames    =  PULONG(PBYTE(exports) - addr + ULONG(exports->AddressOfNames));

    for (ULONG i = 0; i < exports->NumberOfNames; i++) {
        ULONG ord = ordinals[i];

        if (functions[ord] < addr || functions[ord] >= addr + size) {
            PBYTE func = PBYTE(base) + functions[ord];

            PSTR name = PSTR(PBYTE(exports) - addr + fnames[i]);

            BYTE op = InsertBreakPoint(hProcess, func);

            if (enters.find(func) == enters.end()) enters[func] = enter(name, op);
        }
    }
}

VOID RemoveDeadBreakPoints()
{
    std::map<PVOID, enter> dead(enters);
    BYTE op;

    for (std::map<PVOID, enter>::iterator entry = dead.begin(); entry != dead.end(); entry++)
        if (ReadProcessMemory(hDebuggee, entry->first, &op, sizeof op, 0) == FALSE)
            enters.erase(entry->first);
}

ULONG ReturnBreak(PCONTEXT context, ULONG addr, HANDLE tid)
{
    std::vector<leave>& stack = leaves[tid];

    while (!stack.empty() && stack.back().esp < context->Esp) {
        stack.pop_back();
        printf("#");
    }

    if (addr == 0) return 0;

    stack.push_back(leave(addr, context->Eip - 1, context->Esp));

    PDEBUG_CONTROL dr7 = PDEBUG_CONTROL(&context->Dr7);
    PDEBUG_STATUS  dr6 = PDEBUG_STATUS(&context->Dr6);

    context->Dr0 = addr;
    dr7->L0 = 1, dr7->RWE0 = 0, dr7->LEN0 = 0, dr6->B0 = 0;

    return stack.size() - 1;
}

VOID ReportEntry(PCONTEXT context, NT::PDEBUG_MESSAGE dm)
{
    ULONG stack[17];
    CHAR buf[512];

    PVOID addr = dm->u.Exception.ExceptionRecord.ExceptionAddress;

    enter& entry = enters[addr];

    PCSTR s = entry.name;

    if (*s == '?' && UnDecorateSymbolName(s, buf, sizeof buf - 1, 0) > 0) s = buf;

    LONG argc = min(LONG(elements(stack)) - 1, entry.argc);

    if (argc == 0) argc = 3;

    BOOL rv = ReadProcessMemory(hDebuggee, PVOID(context->Esp),
                                stack, sizeof stack[0] * (1 + argc), 0);

    ULONG now = GetTickCount() - StartTime;

    ULONG n = rv ? ReturnBreak(context, stack[0], dm->PortMessage.ClientId.UniqueThread) : 0;

    printf("\n%4d.%02d %4x %*s%s(",
           now / 1000, (now % 1000) / 10, ULONG(dm->PortMessage.ClientId.UniqueThread), n, "", s);

    if (rv == TRUE) {
        switch (argc) {
          case 0:  break;
          case 1:  printf("%x", stack[1]); break;
          case 2:  printf("%x, %x", stack[1], stack[2]); break;
          case 3:  printf("%x, %x, %x", stack[1], stack[2], stack[3]); break;

          default:
            printf("%x, %x, %x, %x", stack[1], stack[2], stack[3], stack[4]);
            for (ULONG i = 5; i <= argc; i++) printf(", %x", stack[i]);
        }
    }

    printf(")");
}

VOID ReportExit(PCONTEXT context)
{
    printf(" -> %x", context->Eax);
}

ULONG HandleBreakPoint(NT::PDEBUG_MESSAGE dm)
{
    PVOID addr = dm->u.Exception.ExceptionRecord.ExceptionAddress;

    std::map<PVOID, enter>::iterator entry = enters.find(addr);

    if (entry != enters.end() && entry->second.opcode != 0xcc) {
        HANDLE hThread = threads[dm->PortMessage.ClientId.UniqueThread];

        CONTEXT context = {CONTEXT_DEBUG_REGISTERS | CONTEXT_CONTROL};

        GetThreadContext(hThread, &context);

        ReportEntry(&context, dm);

	SetByte(hDebuggee, addr, entry->second.opcode);

	steps[dm->PortMessage.ClientId.UniqueThread] = addr;

	context.EFlags |= 0x100;
	context.Eip -= 1;

        SetThreadContext(hThread, &context);
    }
    else {
        if (entry != enters.end() && entry->second.name != 0)
            printf("\nDebug exception at %s\n", entry->second.name);
        else
            printf("\nDebug exception at %p\n", addr);
    }

    return DBG_CONTINUE;
}

ULONG HandleSingleStep(NT::PDEBUG_MESSAGE dm)
{
    std::map<HANDLE, PVOID>::iterator step = steps.find(dm->PortMessage.ClientId.UniqueThread);

    if (step != steps.end()) {
        SetByte(hDebuggee, step->second, INT3);

        steps.erase(step);

        return DBG_CONTINUE;
    }

    HANDLE hThread = threads[dm->PortMessage.ClientId.UniqueThread];

    CONTEXT context = {CONTEXT_DEBUG_REGISTERS | CONTEXT_CONTROL | CONTEXT_INTEGER};

    GetThreadContext(hThread, &context);

    ULONG eaddr = context.Eip;

    std::vector<leave>& stack = leaves[dm->PortMessage.ClientId.UniqueThread];

    if (!stack.empty() && stack.back().caller == eaddr) {
	enters[PVOID(stack.back().callee)].argc = (context.Esp - stack.back().esp - 1) / 4;

	stack.pop_back();
    }

    ULONG iaddr = stack.empty() ? 0 : stack.back().caller;

    PDEBUG_CONTROL dr7 = PDEBUG_CONTROL(&context.Dr7);
    PDEBUG_STATUS  dr6 = PDEBUG_STATUS(&context.Dr6);

    context.Dr0 = iaddr;
    dr7->L0 = 1, dr7->RWE0 = 0, dr7->LEN0 = 0, dr6->B0 = 0;

    if (iaddr == eaddr) context.EFlags |= 0x100, dr7->L0 = 0;

    SetThreadContext(hThread, &context);

    ReportExit(&context);

    return DBG_CONTINUE;
}

ULONG HandleExceptionEvent(NT::PDEBUG_MESSAGE dm)
{
    switch (dm->u.Exception.ExceptionRecord.ExceptionCode) {
      case EXCEPTION_BREAKPOINT:
        return HandleBreakPoint(dm);

      case EXCEPTION_SINGLE_STEP:
        return HandleSingleStep(dm);

      default:
        printf("\nException %x at %p\n",
               dm->u.Exception.ExceptionRecord.ExceptionCode,
               dm->u.Exception.ExceptionRecord.ExceptionAddress);
    }

    return DBG_EXCEPTION_NOT_HANDLED;
}

ULONG HandleCreateProcessThreadEvent(NT::PDEBUG_MESSAGE dm)
{
    printf("\nProcess %x, Thread create %x\n",
           dm->PortMessage.ClientId.UniqueProcess, dm->PortMessage.ClientId.UniqueThread);

    NT::OBJECT_ATTRIBUTES oa = {sizeof oa};
    HANDLE hThread;

    NT::ZwOpenThread(&hThread, THREAD_ALL_ACCESS, &oa, &dm->PortMessage.ClientId);

    threads[dm->PortMessage.ClientId.UniqueThread] = hThread;

    leaves[dm->PortMessage.ClientId.UniqueThread] = std::vector<leave>();

    return DBG_CONTINUE;
}

ULONG HandleExitThreadEvent(NT::PDEBUG_MESSAGE dm)
{
    printf("\nThread %x exit code %x\n",
           dm->PortMessage.ClientId.UniqueThread, dm->u.ExitThread.ExitCode);

    leaves.erase(dm->PortMessage.ClientId.UniqueThread);

    return DBG_CONTINUE;
}

ULONG HandleExitProcessEvent(NT::PDEBUG_MESSAGE dm)
{
    printf("\nProcess %x exit code %x\n",
           dm->PortMessage.ClientId.UniqueProcess, dm->u.ExitProcess.ExitCode);

    leaves.erase(dm->PortMessage.ClientId.UniqueThread);

    return DBG_CONTINUE;
}

ULONG HandleLoadDllEvent(NT::PDEBUG_MESSAGE dm)
{
    InsertBreakPoints(hDebuggee, dm->u.LoadDll.Base);

    return DBG_CONTINUE;
}

ULONG HandleUnloadDllEvent(NT::PDEBUG_MESSAGE)
{
    RemoveDeadBreakPoints();

    return DBG_CONTINUE;
}

BOOL WINAPI HandlerRoutine(ULONG event)
{
    TerminateProcess(hDebuggee, 0);

    return TRUE;
}

HANDLE StartDebuggee(HANDLE hPort)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {sizeof si};

    PSTR cmd = strchr(GetCommandLine(), ' ') + 1;

    CreateProcess(0, cmd, 0, 0, 0, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, 0, 0, &si, &pi);

    NT::ZwSetInformationProcess(pi.hProcess, NT::ProcessDebugPort, &hPort, sizeof hPort);

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    return pi.hProcess;
}

int main(int argc, wchar_t *argv[])
{
    if (argc == 1) return 0;

    SetConsoleCtrlHandler(HandlerRoutine, TRUE);

    NT::OBJECT_ATTRIBUTES oa = {sizeof oa};
    HANDLE hPort;

    NT::ZwCreatePort(&hPort, &oa, 0, 0x78, 0);

    if ((hDebuggee = StartDebuggee(hPort)) == 0) return 0;

    StartTime = GetTickCount();

    NT::DEBUG_MESSAGE dm;

    do {
        NT::ZwReplyWaitReceivePort(hPort, 0, 0, &dm.PortMessage);

        try {
            switch (dm.EventCode + 1) {
              case EXCEPTION_DEBUG_EVENT:
                dm.Status = HandleExceptionEvent(&dm);
                break;

              case CREATE_THREAD_DEBUG_EVENT:
              case CREATE_PROCESS_DEBUG_EVENT:
                dm.Status = HandleCreateProcessThreadEvent(&dm);
                break;

              case EXIT_THREAD_DEBUG_EVENT:
                dm.Status = HandleExitThreadEvent(&dm);
                break;

              case EXIT_PROCESS_DEBUG_EVENT:
                dm.Status = HandleExitProcessEvent(&dm);
                break;

              case LOAD_DLL_DEBUG_EVENT:
                dm.Status = HandleLoadDllEvent(&dm);
                break;

              case UNLOAD_DLL_DEBUG_EVENT:
                dm.Status = HandleUnloadDllEvent(&dm);
                break;

              default:
                dm.Status = DBG_CONTINUE;
                printf("\nUnusual event %lx\n", dm.EventCode);
                break;
            }
        }
        catch (Error e) {
            printf("Error %ld on line %ld\n", e.code, e.line);

            TerminateProcess(hDebuggee, 0);
        }

        NT::ZwReplyPort(hPort, &dm.PortMessage);

    } while (dm.EventCode + 1 != EXIT_PROCESS_DEBUG_EVENT);

    return 0;
}
