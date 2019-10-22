#include "ntdll.h"

int main(int argc, char *argv[])
{
    HANDLE hFile1 = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, 0,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    NT::IO_STATUS_BLOCK iosb;
    NT::FILE_INTERNAL_INFORMATION fii;

    NT::ZwQueryInformationFile(hFile1, &iosb, &fii, sizeof fii, NT::FileInternalInformation);

    NT::UNICODE_STRING name = {sizeof fii.FileId, sizeof fii.FileId, PWSTR(&fii.FileId)};
    NT::OBJECT_ATTRIBUTES oa = {sizeof oa, hFile1, &name};
    HANDLE hFile2;

    NT::ZwOpenFile(&hFile2, GENERIC_READ | SYNCHRONIZE, &oa, &iosb, FILE_SHARE_READ,
                   FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_BY_FILE_ID);

    CloseHandle(hFile1);

    CHAR buf[400]; ULONG n;

    ReadFile(hFile2, buf, sizeof buf, &n, 0);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, n, &n, 0);

    CloseHandle(hFile2);

    return 0;
}
