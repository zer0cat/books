#include "ntdll.h"

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

PVOID GetFromToken(HANDLE hToken, TOKEN_INFORMATION_CLASS tic)
{
    DWORD n;

    BOOL rv = GetTokenInformation(hToken, tic, 0, 0, &n);
    if (rv == FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER) return 0;

    PBYTE p = new BYTE[n];

    return GetTokenInformation(hToken, tic, p, n, &n) == FALSE ? 0 : p;
}

HANDLE SystemToken()
{
    EnablePrivilege(SE_CREATE_TOKEN_NAME);

    HANDLE hToken;
    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken);

    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;

    PSID system;
    AllocateAndInitializeSid(&nt, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &system);

    TOKEN_USER user = {{system, 0}};

    LUID luid;
    AllocateLocallyUniqueId(&luid);

    TOKEN_SOURCE source = {{'*', '*', 'A', 'N', 'O', 'N', '*', '*'},
                           {luid.LowPart, luid.HighPart}};

    LUID authid = SYSTEM_LUID;

    PTOKEN_STATISTICS stats = PTOKEN_STATISTICS(GetFromToken(hToken, TokenStatistics));

    NT::SECURITY_QUALITY_OF_SERVICE sqos
        = {sizeof sqos, NT::SecurityAnonymous, SECURITY_STATIC_TRACKING, FALSE};

    NT::OBJECT_ATTRIBUTES oa = {sizeof oa, 0, 0, 0, 0, &sqos};

    HANDLE hToken2 = 0;

    NT::ZwCreateToken(&hToken2, TOKEN_ALL_ACCESS, &oa, TokenPrimary, 
		      NT::PLUID(&authid), // NT::PLUID(&stats->AuthenticationId),
		      NT::PLARGE_INTEGER(&stats->ExpirationTime),
		      &user,
		      PTOKEN_GROUPS(GetFromToken(hToken, TokenGroups)),
		      PTOKEN_PRIVILEGES(GetFromToken(hToken, TokenPrivileges)),
		      PTOKEN_OWNER(GetFromToken(hToken, TokenOwner)),
		      PTOKEN_PRIMARY_GROUP(GetFromToken(hToken, TokenPrimaryGroup)),
		      PTOKEN_DEFAULT_DACL(GetFromToken(hToken, TokenDefaultDacl)),
		      &source);

    CloseHandle(hToken);

    return hToken2;
}

int main()
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si = {sizeof si};

    return CreateProcessAsUser(SystemToken(), 0, "cmd", 0, 0, FALSE,
                               CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, 0, 0, &si, &pi);
} 
