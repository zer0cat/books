#include "ntdll.h"
#include <stdlib.h>
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

int main(int argc, char *argv[])
{
    if (argc == 1) return 0;

    ULONG pid = strtoul(argv[1], 0, 0);

    EnablePrivilege(SE_DEBUG_NAME);

    HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);

    ULONG n = 0x1000;
    PULONG p = new ULONG[n];

    while (NT::ZwQuerySystemInformation(NT::SystemHandleInformation, p, n * sizeof *p, 0)
           == STATUS_INFO_LENGTH_MISMATCH)

        delete [] p, p = new ULONG[n *= 2];

    NT::PSYSTEM_HANDLE_INFORMATION h = NT::PSYSTEM_HANDLE_INFORMATION(p + 1);

    for (ULONG i = 0; i < *p; i++) {

        if (h[i].ProcessId == pid) {
            HANDLE hObject;

            if (NT::ZwDuplicateObject(hProcess, HANDLE(h[i].Handle), NtCurrentProcess(), &hObject,
                                      0, 0, DUPLICATE_SAME_ATTRIBUTES)
                != STATUS_SUCCESS) continue;

            NT::OBJECT_BASIC_INFORMATION obi;

            NT::ZwQueryObject(hObject, NT::ObjectBasicInformation, &obi, sizeof obi, &n);

            printf("%p %04hx %6lx %2x %3lx %3ld %4ld ", 
                   h[i].Object, h[i].Handle, h[i].GrantedAccess,
                   int(h[i].Flags), obi.Attributes,
                   obi.HandleCount - 1, obi.PointerCount - 2);

            n = obi.TypeInformationLength + 2;

            NT::POBJECT_TYPE_INFORMATION oti = NT::POBJECT_TYPE_INFORMATION(new CHAR[n]);

            NT::ZwQueryObject(hObject, NT::ObjectTypeInformation, oti, n, &n);

            printf("%-14.*ws ", oti[0].Name.Length / 2, oti[0].Name.Buffer);

            n = obi.NameInformationLength == 0 
                ? MAX_PATH * sizeof (WCHAR) : obi.NameInformationLength;

            NT::POBJECT_NAME_INFORMATION oni = NT::POBJECT_NAME_INFORMATION(new CHAR[n]);

            NTSTATUS rv = NT::ZwQueryObject(hObject, NT::ObjectNameInformation, oni, n, &n);
            if (NT_SUCCESS(rv))
                printf("%.*ws", oni[0].Name.Length / 2, oni[0].Name.Buffer);

            printf("\n");

            CloseHandle(hObject);
        }
    }
    delete [] p;

    CloseHandle(hProcess);

    return 0;
}
