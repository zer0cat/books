#include "ntdll.h"
#include <tlhelp32.h>
#include <stdlib.h>
#include <stdio.h>

struct ENTRIES {
    ULONG Offset;
    ULONG Count;
    ULONG Index;
    ENTRIES() : Offset(0), Count(0), Index(0) {}
    ENTRIES(ULONG m, ULONG n) : Offset(m), Count(n), Index(0) {}
};

enum EntryType {
    ProcessType,
    ThreadType,
    ModuleType,
    HeapType,
    MaxType
    };

NT::PSYSTEM_PROCESSES GetProcessesAndThreads()
{
    ULONG n = 0x100;
    NT::PSYSTEM_PROCESSES sp = new NT::SYSTEM_PROCESSES[n];

    while (NT::ZwQuerySystemInformation(NT::SystemProcessesAndThreadsInformation,
					sp, n * sizeof *sp, 0)
           == STATUS_INFO_LENGTH_MISMATCH)
        delete [] sp, sp = new NT::SYSTEM_PROCESSES[n = n * 2];
    return sp;
}

NT::PDEBUG_BUFFER GetModulesAndHeaps(ULONG pid, ULONG mask)
{
    NT::PDEBUG_BUFFER db = NT::RtlCreateQueryDebugBuffer(0, FALSE);
    NT::RtlQueryProcessDebugInformation(pid, mask, db);
    return db;
}

ULONG ProcessCount(NT::PSYSTEM_PROCESSES sp)
{
    ULONG n = 0;
    bool done = false;

    for (NT::PSYSTEM_PROCESSES p = sp; !done;
         p = NT::PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryDelta))
         n++, done = p->NextEntryDelta == 0;
    return n;
}

ULONG ThreadCount(NT::PSYSTEM_PROCESSES sp)
{
    ULONG n = 0;
    bool done = false;

    for (NT::PSYSTEM_PROCESSES p = sp; !done;
         p = NT::PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryDelta))
         n += p->ThreadCount, done = p->NextEntryDelta == 0;
    return n;
}

ULONG ModuleCount(NT::PDEBUG_BUFFER db)
{
    return db->ModuleInformation ? *PULONG(db->ModuleInformation) : 0;
}

ULONG HeapCount(NT::PDEBUG_BUFFER db)
{
    return db->HeapInformation ? *PULONG(db->HeapInformation) : 0;
}

VOID AddProcesses(PPROCESSENTRY32 pe, NT::PSYSTEM_PROCESSES sp)
{
    bool done = false;

    for (NT::PSYSTEM_PROCESSES p = sp; !done;
         p = NT::PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryDelta)) {

        pe->dwSize = sizeof *pe;
        pe->cntUsage = 0;
        pe->th32ProcessID = p->ProcessId;
        pe->th32DefaultHeapID = 0;
        pe->th32ModuleID = 0;
        pe->cntThreads = p->ThreadCount;
        pe->th32ParentProcessID = p->InheritedFromProcessId;
        pe->pcPriClassBase = p->BasePriority;
        pe->dwFlags = 0;
        sprintf(pe->szExeFile, "%.*ls",
                p->ProcessName.Length / 2, p->ProcessName.Buffer);

        pe++;

        done = p->NextEntryDelta == 0;
    }
}

VOID AddThreads(PTHREADENTRY32 te, NT::PSYSTEM_PROCESSES sp)
{
    bool done = false;

    for (NT::PSYSTEM_PROCESSES p = sp; !done;
         p = NT::PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryDelta)) {

        for (ULONG i = 0; i < p->ThreadCount; i++) {

            te->dwSize = sizeof *te;
            te->cntUsage = 0;
            te->th32ThreadID = DWORD(p->Threads[i].ClientId.UniqueThread);
            te->th32OwnerProcessID = p->ProcessId;
            te->tpBasePri = p->Threads[i].BasePriority;
            te->tpDeltaPri = p->Threads[i].Priority - p->Threads[i].BasePriority;
            te->dwFlags = 0;

            te++;
        }

        done = p->NextEntryDelta == 0;
    }
}

VOID AddModules(PMODULEENTRY32 me, NT::PDEBUG_BUFFER db, ULONG pid)
{
    ULONG n = ModuleCount(db);
    NT::PDEBUG_MODULE_INFORMATION p
        = NT::PDEBUG_MODULE_INFORMATION(PULONG(db->ModuleInformation) + 1);

    for (ULONG i = 0; i < n; i++) {

        me->dwSize = sizeof *me;
        me->th32ModuleID = 0;
        me->th32ProcessID = pid;
        me->GlblcntUsage = p[i].LoadCount;
        me->ProccntUsage = p[i].LoadCount;
        me->modBaseAddr = PBYTE(p[i].Base);
        me->modBaseSize = p[i].Size;
        me->hModule = HMODULE(p[i].Base);
        sprintf(me->szModule,  "%s", p[i].ImageName + p[i].ModuleNameOffset);
        sprintf(me->szExePath, "%s", p[i].ImageName);

        me++;
    }
}

VOID AddHeaps(PHEAPLIST32 hl, NT::PDEBUG_BUFFER db, ULONG pid)
{
    ULONG n = HeapCount(db);
    NT::PDEBUG_HEAP_INFORMATION p
        = NT::PDEBUG_HEAP_INFORMATION(PULONG(db->HeapInformation) + 1);

    for (ULONG i = 0; i < n; i++) {

        hl->dwSize = sizeof *hl;
        hl->th32ProcessID = pid;
        hl->th32HeapID = p[i].Base;
        hl->dwFlags = p[i].Flags;

        hl++;
    }
}

template<class T>
BOOL GetEntry(HANDLE hSnapshot, T entry, bool first, EntryType type)
{
    ENTRIES *entries = (ENTRIES*)MapViewOfFile(hSnapshot, FILE_MAP_WRITE,
                                               0, 0, 0);
    if (entries == 0) return FALSE;

    BOOL rv = TRUE;

    entries[type].Index = first ? 0 : entries[type].Index + 1;

    if (entries[type].Index >= entries[type].Count)
        SetLastError(ERROR_NO_MORE_FILES), rv = FALSE;
    if (entry->dwSize < sizeof *entry)
        SetLastError(ERROR_INSUFFICIENT_BUFFER), rv = FALSE;
    if (rv)
        *entry = T(PCHAR(entries) + entries[type].Offset)[entries[type].Index];

    UnmapViewOfFile(entries);
    return rv;
}

HANDLE
WINAPI
CreateToolhelp32Snapshot(DWORD flags, DWORD pid)
{
    if (pid == 0) pid = GetCurrentProcessId();

    ULONG mask = ((flags & TH32CS_SNAPMODULE) ? PDI_MODULES : 0) |
	((flags & TH32CS_SNAPHEAPLIST) ? PDI_HEAPS : 0);

    NT::PDEBUG_BUFFER db =
        (flags & (TH32CS_SNAPMODULE | TH32CS_SNAPHEAPLIST)) ? GetModulesAndHeaps(pid, mask) : 0;

    NT::PSYSTEM_PROCESSES sp = 
        (flags & (TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD)) ? GetProcessesAndThreads() : 0;

    ENTRIES entries[MaxType];
    ULONG n = sizeof entries;

    if (flags & TH32CS_SNAPPROCESS) {
        entries[ProcessType] = ENTRIES(n, ProcessCount(sp));
        n += entries[ProcessType].Count * sizeof (PROCESSENTRY32);
    }
    if (flags & TH32CS_SNAPTHREAD) {
        entries[ThreadType] = ENTRIES(n, ThreadCount(sp));
        n += entries[ThreadType].Count * sizeof (THREADENTRY32);
    }
    if (flags & TH32CS_SNAPMODULE) {
        entries[ModuleType] = ENTRIES(n, ModuleCount(db));
        n += entries[ModuleType].Count * sizeof (MODULEENTRY32);
    }
    if (flags & TH32CS_SNAPHEAPLIST) {
        entries[HeapType] = ENTRIES(n, HeapCount(db));
        n += entries[HeapType].Count * sizeof (HEAPLIST32);
    }

    SECURITY_ATTRIBUTES sa = {sizeof sa, 0, (flags & TH32CS_INHERIT) != 0};

    HANDLE hMap = CreateFileMapping(HANDLE(0xFFFFFFFF), &sa, PAGE_READWRITE | SEC_COMMIT, 0, n, 0);

    ENTRIES *p = (ENTRIES*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);

    for (int i = 0; i < MaxType; i++) p[i] = entries[i];

    if (flags & TH32CS_SNAPPROCESS)
        AddProcesses(PPROCESSENTRY32(PCHAR(p) + entries[ProcessType].Offset), sp);
    if (flags & TH32CS_SNAPTHREAD)
        AddThreads(PTHREADENTRY32(PCHAR(p) + entries[ThreadType].Offset), sp);
    if (flags & TH32CS_SNAPMODULE)
        AddModules(PMODULEENTRY32(PCHAR(p) + entries[ModuleType].Offset), db, pid);
    if (flags & TH32CS_SNAPHEAPLIST)
        AddHeaps(PHEAPLIST32(PCHAR(p) + entries[HeapType].Offset), db, pid);

    UnmapViewOfFile(p);

    if (sp) delete [] sp;

    if (db) NT::RtlDestroyQueryDebugBuffer(db);

    return hMap;
}

BOOL
WINAPI
Process32First(HANDLE hSnapshot, PPROCESSENTRY32 pe)
{
    return GetEntry(hSnapshot, pe, true, ProcessType);
}

BOOL
WINAPI
Process32Next(HANDLE hSnapshot, PPROCESSENTRY32 pe)
{
    return GetEntry(hSnapshot, pe, false, ProcessType);
}

BOOL
WINAPI
Thread32First(HANDLE hSnapshot, PTHREADENTRY32 te)
{
    return GetEntry(hSnapshot, te, true, ThreadType);
}

BOOL
WINAPI
Thread32Next(HANDLE hSnapshot, PTHREADENTRY32 te)
{
    return GetEntry(hSnapshot, te, false, ThreadType);
}

BOOL
WINAPI
Module32First(HANDLE hSnapshot, PMODULEENTRY32 me)
{
    return GetEntry(hSnapshot, me, true, ModuleType);
}

BOOL
WINAPI
Module32Next(HANDLE hSnapshot, PMODULEENTRY32 me)
{
    return GetEntry(hSnapshot, me, false, ModuleType);
}

BOOL
WINAPI
Heap32ListFirst(HANDLE hSnapshot, PHEAPLIST32 hl)
{
    return GetEntry(hSnapshot, hl, true, HeapType);
}

BOOL
WINAPI
Heap32ListNext(HANDLE hSnapshot, PHEAPLIST32 hl)
{
    return GetEntry(hSnapshot, hl, false, HeapType);
}
