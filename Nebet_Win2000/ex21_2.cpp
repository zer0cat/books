#include <windows.h>

typedef ULONG NTSTATUS;

extern "C"
NTSTATUS
NTAPI
RtlDecompressBuffer(
    USHORT CompressionFormat,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PULONG ReturnLength
    );

int main(int argc, char *argv[])
{
    if (argc != 3) return 0;

    HANDLE hFile1 = CreateFile(argv[1], GENERIC_READ, 
                               FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    HANDLE hFile2 = CreateFile(argv[2], GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    ULONG n = GetFileSize(hFile1, 0);

    HANDLE hMapping1 = CreateFileMapping(hFile1, 0, PAGE_READONLY, 0, 0, 0);
    HANDLE hMapping2 = CreateFileMapping(hFile2, 0, PAGE_READWRITE, 0, n, 0);

    PCHAR p = PCHAR(MapViewOfFileEx(hMapping1, FILE_MAP_READ, 0, 0, 0, 0));
    PCHAR q = PCHAR(MapViewOfFileEx(hMapping2, FILE_MAP_WRITE, 0, 0, 0, 0));

    for (ULONG m, i = 0; i < n; i += m)
        RtlDecompressBuffer(COMPRESSION_FORMAT_LZNT1, q + i, n - i, p + i, n - i, &m);

    return 0;
}
