#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "ntfs.h"

ULONG BytesPerFileRecord;
HANDLE hVolume;
BOOT_BLOCK bootb;
PFILE_RECORD_HEADER MFT;

template <class T1, class T2> inline 
T1* Padd(T1* p, T2 n) { return (T1*)((char *)p + n); }

ULONG RunLength(PUCHAR run)
{
    return (*run & 0xf) + ((*run >> 4) & 0xf) + 1;
}

LONGLONG RunLCN(PUCHAR run)
{
    UCHAR n1 = *run & 0xf;
    UCHAR n2 = (*run >> 4) & 0xf;
    LONGLONG lcn = n2 == 0 ? 0 : CHAR(run[n1 + n2]);

    for (LONG i = n1 + n2 - 1; i > n1; i--)
        lcn = (lcn << 8) + run[i];
    return lcn;
}

ULONGLONG RunCount(PUCHAR run)
{
    UCHAR n = *run & 0xf;
    ULONGLONG count = 0;

    for (ULONG i = n; i > 0; i--)
        count = (count << 8) + run[i];
    return count;
}

BOOL FindRun(PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn,
             PULONGLONG lcn, PULONGLONG count)
{
    if (vcn < attr->LowVcn || vcn > attr->HighVcn) return FALSE;

    *lcn = 0;
    ULONGLONG base = attr->LowVcn;

    for (PUCHAR run = PUCHAR(Padd(attr, attr->RunArrayOffset));
         *run != 0;
         run += RunLength(run)) {

        *lcn += RunLCN(run);
        *count = RunCount(run);

        if (base <= vcn && vcn < base + *count) {
            *lcn = RunLCN(run) == 0 ? 0 : *lcn + vcn - base;
            *count -= ULONG(vcn - base);

            return TRUE;
        }
        else
            base += *count;
    }

    return FALSE;
}

PATTRIBUTE FindAttribute(PFILE_RECORD_HEADER file,
                         ATTRIBUTE_TYPE type, PWSTR name)
{
    for (PATTRIBUTE attr = PATTRIBUTE(Padd(file, file->AttributesOffset));
         attr->AttributeType != -1;
         attr = Padd(attr, attr->Length)) {

        if (attr->AttributeType == type) {
            if (name == 0 && attr->NameLength == 0) return attr;

            if (name != 0 && wcslen(name) == attr->NameLength
                && _wcsicmp(name, PWSTR(Padd(attr, attr->NameOffset))) == 0) return attr;
        }
    }

    return 0;
}  

VOID FixupUpdateSequenceArray(PFILE_RECORD_HEADER file)
{
    PUSHORT usa = PUSHORT(Padd(file, file->Ntfs.UsaOffset));
    PUSHORT sector = PUSHORT(file);

    for (ULONG i = 1; i < file->Ntfs.UsaCount; i++) {
        sector[255] = usa[i];
        sector += 256;
    }
}

VOID ReadSector(ULONGLONG sector, ULONG count, PVOID buffer)
{
    ULARGE_INTEGER offset;
    OVERLAPPED overlap = {0};
    ULONG n;

    offset.QuadPart = sector * bootb.BytesPerSector;
    overlap.Offset = offset.LowPart; overlap.OffsetHigh = offset.HighPart;

    ReadFile(hVolume, buffer, count * bootb.BytesPerSector, &n, &overlap);
}

VOID ReadLCN(ULONGLONG lcn, ULONG count, PVOID buffer)
{
    ReadSector(lcn * bootb.SectorsPerCluster, count * bootb.SectorsPerCluster, buffer);
}

VOID ReadExternalAttribute(PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, ULONG count, PVOID buffer)
{
    ULONGLONG lcn, runcount;
    ULONG readcount, left;
    PUCHAR bytes = PUCHAR(buffer);

    for (left = count; left > 0; left -= readcount) {
        FindRun(attr, vcn, &lcn, &runcount);

        readcount = ULONG(min(runcount, left));

        ULONG n = readcount * bootb.BytesPerSector * bootb.SectorsPerCluster;

        if (lcn == 0)
            memset(bytes, 0, n);
        else
            ReadLCN(lcn, readcount, bytes);

        vcn += readcount;
        bytes += n;
    }
}

ULONG AttributeLength(PATTRIBUTE attr)
{
    return attr->Nonresident == FALSE
        ? PRESIDENT_ATTRIBUTE(attr)->ValueLength
	: ULONG(PNONRESIDENT_ATTRIBUTE(attr)->DataSize);
}

ULONG AttributeLengthAllocated(PATTRIBUTE attr)
{
    return attr->Nonresident == FALSE
        ? PRESIDENT_ATTRIBUTE(attr)->ValueLength
	: ULONG(PNONRESIDENT_ATTRIBUTE(attr)->AllocatedSize);
}

VOID ReadAttribute(PATTRIBUTE attr, PVOID buffer)
{
    if (attr->Nonresident == FALSE) {
        PRESIDENT_ATTRIBUTE rattr = PRESIDENT_ATTRIBUTE(attr);
        memcpy(buffer, Padd(rattr, rattr->ValueOffset), rattr->ValueLength);
    }
    else {
        PNONRESIDENT_ATTRIBUTE nattr = PNONRESIDENT_ATTRIBUTE(attr);
        ReadExternalAttribute(nattr, 0, ULONG(nattr->HighVcn) + 1, buffer);
    }
}

VOID ReadVCN(PFILE_RECORD_HEADER file, ATTRIBUTE_TYPE type, ULONGLONG vcn, ULONG count, PVOID buffer)
{
    PNONRESIDENT_ATTRIBUTE attr = PNONRESIDENT_ATTRIBUTE(FindAttribute(file, type, 0));

    if (attr == 0 || (vcn < attr->LowVcn || vcn > attr->HighVcn)) {
        // Support for huge files

        PATTRIBUTE attrlist = FindAttribute(file, AttributeAttributeList, 0);

        DebugBreak();
    }

    ReadExternalAttribute(attr, vcn, count, buffer);
}

VOID ReadFileRecord(ULONG index, PFILE_RECORD_HEADER file)
{
    ULONG clusters = bootb.ClustersPerFileRecord;
    if (clusters > 0x80) clusters = 1;

    PUCHAR p = new UCHAR[bootb.BytesPerSector * bootb.SectorsPerCluster * clusters];

    ULONGLONG vcn = ULONGLONG(index) * BytesPerFileRecord
                  / bootb.BytesPerSector / bootb.SectorsPerCluster;

    ReadVCN(MFT, AttributeData, vcn, clusters, p);

    LONG m = (bootb.SectorsPerCluster * bootb.BytesPerSector / BytesPerFileRecord) - 1;

    ULONG n = m > 0 ? (index & m) : 0;

    memcpy(file, p + n * BytesPerFileRecord, BytesPerFileRecord);

    delete [] p;

    FixupUpdateSequenceArray(file);
}

VOID LoadMFT()
{
    BytesPerFileRecord = bootb.ClustersPerFileRecord < 0x80
                       ? bootb.ClustersPerFileRecord * bootb.SectorsPerCluster
                           * bootb.BytesPerSector
		       : 1 << (0x100 - bootb.ClustersPerFileRecord);

    MFT = PFILE_RECORD_HEADER(new UCHAR[BytesPerFileRecord]);

    ReadSector(bootb.MftStartLcn * bootb.SectorsPerCluster, 
               BytesPerFileRecord / bootb.BytesPerSector, MFT);

    FixupUpdateSequenceArray(MFT);
}

BOOL bitset(PUCHAR bitmap, ULONG i)
{
    return (bitmap[i >> 3] & (1 << (i & 7))) != 0;
}

VOID FindDeleted()
{
    PATTRIBUTE attr = FindAttribute(MFT, AttributeBitmap, 0);
    PUCHAR bitmap = new UCHAR[AttributeLengthAllocated(attr)];

    ReadAttribute(attr, bitmap);

    ULONG n = AttributeLength(FindAttribute(MFT, AttributeData, 0)) / BytesPerFileRecord;

    PFILE_RECORD_HEADER file = PFILE_RECORD_HEADER(new UCHAR[BytesPerFileRecord]);

    for (ULONG i = 0; i < n; i++) {
        if (bitset(bitmap, i)) continue;

        ReadFileRecord(i, file);

        if (file->Ntfs.Type == 'ELIF' && (file->Flags & 1) == 0) {
            attr = FindAttribute(file, AttributeFileName, 0);
            if (attr == 0) continue;

            PFILENAME_ATTRIBUTE name
                = PFILENAME_ATTRIBUTE(Padd(attr, PRESIDENT_ATTRIBUTE(attr)->ValueOffset));

            printf("%8lu %.*ws\n", i, int(name->NameLength), name->Name);
        }
    }
}

VOID DumpData(ULONG index, PCSTR filename)
{
    PFILE_RECORD_HEADER file = PFILE_RECORD_HEADER(new UCHAR[BytesPerFileRecord]);
    ULONG n;

    ReadFileRecord(index, file);

    if (file->Ntfs.Type != 'ELIF') return;

    PATTRIBUTE attr = FindAttribute(file, AttributeData, 0);
    if (attr == 0) return;

    PUCHAR buf = new UCHAR[AttributeLengthAllocated(attr)];

    ReadAttribute(attr, buf);

    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    WriteFile(hFile, buf, AttributeLength(attr), &n, 0);

    CloseHandle(hFile);

    delete [] buf;
}

int main(int argc, char *argv[])
{
    CHAR drive[] = "\\\\.\\C:";
    ULONG n;

    if (argc < 2) return 0;

    drive[4] = argv[1][0];

    hVolume = CreateFile(drive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                         OPEN_EXISTING, 0, 0);

    ReadFile(hVolume, &bootb, sizeof bootb, &n, 0);

    LoadMFT();

    if (argc == 2) FindDeleted();
    if (argc == 4) DumpData(strtoul(argv[2], 0, 0), argv[3]);

    CloseHandle(hVolume);

    return 0;
}
