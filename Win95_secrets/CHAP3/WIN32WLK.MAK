PROJ = WIN32WLK

OBJS = $(PROJ).obj

DEBUG = 0
CC = cl

CFLAGS_D_DEXE32 = /D_X86_ /W3 /D_DEBUG /DWIN32_LEAN_AND_MEAN /D "_WINDOWS" /D "WIN32" /Zi /Fd"$(PROJ).PDB" /Fp"$(PROJ).PCH" /YX
CFLAGS_R_DEXE32 = /D_X86_ /W3 /O2 /DNDEBUG  /D "_WINDOWS" /D "WIN32"
LFLAGS_D_DEXE32 = /DEBUG /DEBUGTYPE:CV /SUBSYSTEM:windows,4
LFLAGS_R_DEXE32 = /SUBSYSTEM:windows,4

LIBS_D_DEXE32 = kernel32.lib th32.lib K32LIB.LIB USER32.LIB 
LIBS_R_DEXE32 = kernel32.lib th32.lib K32LIB.LIB USER32.LIB 

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

$(PROJ).EXE: $(OBJS) $(PROJ).RES
    echo >NUL @<<$(PROJ).CRF
$(LFLAGS)
$(OBJS)
-OUT:$(PROJ).EXE
$(LIBS)
$(PROJ).RES
<<
    link @$(PROJ).CRF

win32wlk.c: win32wlk.h module32.h procdb.h threaddb.h k32objs.h

$(PROJ).RES: $(PROJ).RC
  RC -r $(PROJ).RC
