PROJ = SIMONSEZ

OBJS = $(PROJ).obj HOOKAPI.OBJ

DEBUG = 0
CC = cl

#
# Warning!  Optimizations have been turned off here in the retail build to
# circumvent a VC++ optimizer "feature" that screws up the code. See near
# the end of SIMONSEZ.C for details
#
CFLAGS_D_DEXE32 = /D_X86_ /W3 /D_DEBUG  /DWIN32_LEAN_AND_MEAN /D "_WINDOWS" /D "WIN32" /Zi /Fd"$(PROJ).PDB" /Fp"$(PROJ).PCH" /YX
CFLAGS_R_DEXE32 = /D_X86_ /W3 /DNDEBUG /DWIN32_LEAN_AND_MEAN /D "_WINDOWS" /D "WIN32"
LFLAGS_D_DEXE32 = /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:windows
LFLAGS_R_DEXE32 = /SUBSYSTEM:windows
LIBS_D_DEXE32 = kernel32.lib USER32.LIB
LIBS_R_DEXE32 = kernel32.lib USER32.LIB

!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE32)
LFLAGS = $(LFLAGS_D_DEXE32)
LIBS = $(LIBS_D_DEXE32)
!else
CFLAGS = $(CFLAGS_R_DEXE32)
LFLAGS = $(LFLAGS_R_DEXE32)
LIBS = $(LIBS_R_DEXE32)
!endif

all: $(PROJ).EXE

.c.obj:
    $(CC) $(CFLAGS) /c $<

$(PROJ).EXE: $(OBJS)
    echo >NUL @<<$(PROJ).CRF
$(LFLAGS)
$(OBJS)
-OUT:$(PROJ).EXE
$(LIBS)
<<
    link @$(PROJ).CRF
