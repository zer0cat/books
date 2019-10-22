!ifdef debug
cdebug = -Od -Z7
ldebug = -debug:full -debugtype:cv
!else
cdebug = -Ox
ldebug = -release
!endif

CPPFLAGS = -c -W3 -GX -MD -nologo $(cdebug) -D_X86_ -DSTRICT -DWIN32_LEAN_AND_MEAN 
lflags = -incremental:no -pdb:none -nologo $(ldebug)

llibs = ntdll.lib kernel32.lib advapi32.lib imagehlp.lib

!ifdef dll
$(target).dll: $(target).obj ; link $(lflags) $(llibs) $** -dll
!else
$(target).exe: $(target).obj ; link $(lflags) $(llibs) $**
!endif
