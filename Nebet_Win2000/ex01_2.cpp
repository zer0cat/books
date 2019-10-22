#pragma warning(disable:4786) // identifier was truncated in the debug info

#include "ntdll.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>

struct OBJECTS_AND_TYPES {
    std::map<ULONG, NT::PSYSTEM_OBJECT_TYPE_INFORMATION, std::less<ULONG> > types;
    std::map<PVOID, NT::PSYSTEM_OBJECT_INFORMATION, std::less<PVOID> > objects;
};

std::vector<NT::SYSTEM_HANDLE_INFORMATION> GetHandles()
{
    ULONG n;
    PULONG p = new ULONG[n = 0x100];

    while (NT::ZwQuerySystemInformation(NT::SystemHandleInformation, p, n * sizeof *p, 0)
           == STATUS_INFO_LENGTH_MISMATCH)

        delete [] p, p = new ULONG[n *= 2];

    NT::PSYSTEM_HANDLE_INFORMATION h = NT::PSYSTEM_HANDLE_INFORMATION(p + 1);

    return std::vector<NT::SYSTEM_HANDLE_INFORMATION>(h, h + *p);
}

OBJECTS_AND_TYPES GetObjectsAndTypes()
{
    ULONG n;
    PCHAR p = new CHAR[n = 0x1000];

    while (NT::ZwQuerySystemInformation(NT::SystemObjectInformation, p, n * sizeof *p, 0)
           == STATUS_INFO_LENGTH_MISMATCH)

        delete [] p, p = new CHAR[n *= 2];

    OBJECTS_AND_TYPES oats;

    for (NT::PSYSTEM_OBJECT_TYPE_INFORMATION
         t = NT::PSYSTEM_OBJECT_TYPE_INFORMATION(p); ;
         t = NT::PSYSTEM_OBJECT_TYPE_INFORMATION(p + t->NextEntryOffset)) {

        oats.types[t->TypeNumber] = t;

        for (NT::PSYSTEM_OBJECT_INFORMATION 
             o = NT::PSYSTEM_OBJECT_INFORMATION(PCHAR(t->Name.Buffer) + t->Name.MaximumLength); ;
             o = NT::PSYSTEM_OBJECT_INFORMATION(p + o->NextEntryOffset)) {

            oats.objects[o->Object] = o;

            if (o->NextEntryOffset == 0) break;
        }
        if (t->NextEntryOffset == 0) break;
    }

    return oats;
}

int main(int argc, char *argv[])
{
    if (argc == 1) return 0;

    ULONG pid = strtoul(argv[1], 0, 0);

    OBJECTS_AND_TYPES oats = GetObjectsAndTypes();

    std::vector<NT::SYSTEM_HANDLE_INFORMATION> handles = GetHandles();

    NT::SYSTEM_OBJECT_INFORMATION defobj = {0};

    printf("Object   Hnd  Access Fl Atr  #H   #P Type           Name\n");

    for (std::vector<NT::SYSTEM_HANDLE_INFORMATION>::iterator
         h = handles.begin(); h != handles.end(); h++) {

        if (h->ProcessId == pid) {

            NT::PSYSTEM_OBJECT_TYPE_INFORMATION t = oats.types[h->ObjectTypeNumber];
            NT::PSYSTEM_OBJECT_INFORMATION o = oats.objects[h->Object];

            if (o == 0) o = &defobj;

            printf("%p %04hx %6lx %2x %3hx %3ld %4ld %-14.*S %.*S\n", 
                   h->Object, h->Handle, h->GrantedAccess, int(h->Flags),
                   o->Flags, o->HandleCount, o->PointerCount,
                   t->Name.Length, t->Name.Buffer, o->Name.Length, o->Name.Buffer);
        }
    }

    return 0;
}
