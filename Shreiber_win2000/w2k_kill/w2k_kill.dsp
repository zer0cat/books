# Microsoft Developer Studio Project File - Name="w2k_kill" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=w2k_kill - Win2K kernel-mode driver (release)
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "w2k_kill.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "w2k_kill.mak" CFG="w2k_kill - Win2K kernel-mode driver (release)"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "w2k_kill - Win2K kernel-mode driver (release)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "w2k_kill - Win2K kernel-mode driver (debug)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "w2k_kill - Win2K kernel-mode driver (release)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /G4 /Gz /MT /W3 /Ox /Gf /Gy /I "E:\NTDDK\inc" /D "NDEBUG" /D "_X86_" /D "i386" /D "STD_CALL" /D "CONDITION_HANDLING" /D "WIN32_LEAN_AND_MEAN" /D "NT_UP" /U "NT_INST" /FD /c
# SUBTRACT CPP /Ot /Oa /Ow /Og /Oi /Os /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 E:\NTDDK\libfre\i386\ntoskrnl.lib E:\NTDDK\libfre\i386\hal.lib kernel32.lib /nologo /base:"0x10000" /entry:"DriverEntry@8" /pdb:".\mousecl.pdb" /machine:I386 /nodefaultlib /out:".\Release\w2k_kill.sys" /SUBSYSTEM:native
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "w2k_kill - Win2K kernel-mode driver (debug)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /G4 /Gz /MTd /W3 /Z7 /Oi /Gf /Gy /I "E:\NTDDK\inc" /D "_DEBUG" /D "_X86_" /D "i386" /D "STD_CALL" /D "CONDITION_HANDLING" /D "WIN32_LEAN_AND_MEAN" /D "NT_UP" /D "RDRDBG" /D "SRVDBG" /D "DBG" /D "_IDWBUILD" /U "NT_INST" /FD /Zel /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 E:\NTDDK\libchk\i386\ntoskrnl.lib E:\NTDDK\libchk\i386\hal.lib kernel32.lib /nologo /base:"0x10000" /entry:"DriverEntry@8" /incremental:no /debug /debugtype:both /machine:I386 /nodefaultlib /out:".\Debug\w2k_kill.sys" /SUBSYSTEM:native
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "w2k_kill - Win2K kernel-mode driver (release)"
# Name "w2k_kill - Win2K kernel-mode driver (debug)"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\w2k_kill.c
# End Source File
# Begin Source File

SOURCE=.\w2k_kill.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\w2k_kill.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
