#include "ntdll.h"
#include <stdlib.h>
#include <stdio.h>

template <int i> struct PORT_MESSAGEX : NT::PORT_MESSAGE {
    UCHAR Data[i];
};

DWORD WINAPI client(PVOID)
{
    NT::UNICODE_STRING name; 
    NT::RtlInitUnicodeString(&name, L"\\Test");

    HANDLE hSection = CreateFileMapping(HANDLE(0xFFFFFFFF), 0, PAGE_READWRITE, 0, 0x50000, 0);

    ULONG n, cd[] = {1, 2, 3, 4, 5}, cdn = sizeof cd;
    NT::SECURITY_QUALITY_OF_SERVICE sqos = {sizeof sqos, NT::SecurityImpersonation, TRUE, TRUE};
    NT::PORT_SECTION_WRITE psw = {sizeof psw, hSection, 0x20000, 0x30000};
    NT::PORT_SECTION_READ psr = {sizeof psr};
    HANDLE hPort;

    NT::ZwConnectPort(&hPort, &name, &sqos, &psw, &psr, &n, cd, &cdn);

    PORT_MESSAGEX<40> req, rep;
    CHAR txt[] = "Hello, World";

    memset(&req, 0xaa, sizeof req);
    memset(&rep, 0xcc, sizeof req);

    req.MessageType = NT::LPC_NEW_MESSAGE;
    req.MessageSize = sizeof req;
    req.VirtualRangesOffset = 0;
    req.DataSize = sizeof txt;
    strcpy(PSTR(req.Data), txt);

    while (true) {
        NT::ZwRequestWaitReplyPort(hPort, &req, &rep);

        printf("client(): type %hd, id %hu\n", rep.MessageType, rep.MessageId);

        Sleep(1000);
    }

    return 0;
}

int main()
{
    NT::UNICODE_STRING name; 
    NT::RtlInitUnicodeString(&name, L"\\Test");

    NT::OBJECT_ATTRIBUTES oa = {sizeof oa, 0, &name};
    PORT_MESSAGEX<40> req;
    HANDLE hPort;

    memset(&req, 0xee, sizeof req);

    NT::ZwCreatePort(&hPort, &oa, 0, sizeof req, 0);

    ULONG tid;
    HANDLE hThread = CreateThread(0, 0, client, 0, 0, &tid);

    NT::ZwListenPort(hPort, &req);

    ULONG n = 0x9000;
    HANDLE hSection = CreateFileMapping(HANDLE(0xFFFFFFFF), 0, PAGE_READWRITE, 0, n, 0);
    NT::PORT_SECTION_WRITE psw = {sizeof psw, hSection, 0, n};
    NT::PORT_SECTION_READ psr = {sizeof psr};
    HANDLE hPort2;

    req.DataSize = 4; req.Data[0] = 0xfe;

    NT::ZwAcceptConnectPort(&hPort2, 0xdeadbabe, &req, TRUE, &psw, &psr);

    NT::ZwCompleteConnectPort(hPort2);

    while (true) {
        ULONG portid;

        NT::ZwReplyWaitReceivePort(hPort2, &portid, 0, &req);

        printf("server(): type %hd, id %hu\n", req.MessageType, req.MessageId);

        req.DataSize = 1; req.Data[0] = 0xfd; 

        NT::ZwReplyPort(hPort2, &req);
    }

    return 0;
}
