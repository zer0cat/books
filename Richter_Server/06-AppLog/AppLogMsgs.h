/**************************************************************
Module:  AppLogMsgs.mc 
Notices: Copyright (c) 2000 Jeffrey Richter
**************************************************************/
//******************** CATEGORY SECTION ***********************
// Category IDs are 16-bit values
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: CAT_APPEXECSTATUS
//
// MessageText:
//
//  Application execution status
//
#define CAT_APPEXECSTATUS                ((WORD)0x00000001L)

//
// MessageId: CAT_APPEVENT
//
// MessageText:
//
//  Application event
//
#define CAT_APPEVENT                     ((WORD)0x00000002L)

//********************* MESSAGE SECTION ***********************
// Event IDs are 32-bit values
//
// MessageId: MSG_APPSTART
//
// MessageText:
//
//  Application started.
//
#define MSG_APPSTART                     ((DWORD)0x00000064L)

//
// MessageId: MSG_APPSTOP
//
// MessageText:
//
//  Application stopped.
//
#define MSG_APPSTOP                      ((DWORD)0x00000065L)

//
// MessageId: MSG_ERROR
//
// MessageText:
//
//  Application generated error code %1: "%%%1"
//
#define MSG_ERROR                        ((DWORD)0x00000066L)

//************************ END OF FILE ************************
