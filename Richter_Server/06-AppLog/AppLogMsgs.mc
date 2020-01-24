;/**************************************************************
;Module:  AppLogMsgs.mc 
;Notices: Copyright (c) 2000 Jeffrey Richter
;**************************************************************/


;//******************** CATEGORY SECTION ***********************
;// Category IDs are 16-bit values
MessageIdTypedef=WORD

MessageId=1
SymbolicName=CAT_APPEXECSTATUS
Language=English
Application execution status
.

MessageId=2
SymbolicName=CAT_APPEVENT
Language=English
Application event
.


;//********************* MESSAGE SECTION ***********************
;// Event IDs are 32-bit values
MessageIdTypedef=DWORD

MessageId=100
SymbolicName=MSG_APPSTART
Language=English
Application started.
.

MessageId=
SymbolicName=MSG_APPSTOP
Language=English
Application stopped.
.

MessageId=
SymbolicName=MSG_ERROR
Language=English
Application generated error code %1: "%%%1"
.


;//************************ END OF FILE ************************
