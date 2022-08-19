PROJ = APISPY32

OBJS = w32ssupp.obj \
       apispy32.obj \
       intrcpt.obj  \
       loadapis.obj \
       log.obj      \
       return.obj   \
       asmretrn.obj

DEBUG = 0
CC = cl
RC = rc

CFLAGS_D_DDLL32 = /D_X86_ /W3 /D_DEBUG  /D "_WINDOWS" /D "WIN32" /YX /Zi /Fd"$(PROJ).PDB" /Fp"$(PROJ)" /YX
CFLAGS_R_DDLL32 = /D_X86_ /W3 /O2 /DNDEBUG  /D "_WINDOWS" /D "WIN32"
LFLAGS_D_DDLL32 = /DLL /DEBUG /DEBUGTYPE:both /SUBSYSTEM:console
LFLAGS_R_DDLL32 = /DLL /SUBSYSTEM:console
LIBS_D_DDLL32 = kernel32.lib USER32.LIB
LIBS_R_DDLL32 = kernel32.lib USER32.LIB

!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DDLL32)
LFLAGS = $(LFLAGS_D_DDLL32)
LIBS = $(LIBS_D_DDLL32)
!else
CFLAGS = $(CFLAGS_R_DDLL32)
LFLAGS = $(LFLAGS_R_DDLL32)
LIBS = $(LIBS_R_DDLL32)
!endif

all: $(PROJ).DLL

.c.obj:
    $(CC) $(CFLAGS) /c $<

$(PROJ).DLL: $(OBJS)
    echo >NUL @<<$(PROJ).CRF
$(LFLAGS)
$(OBJS)
-OUT:$(PROJ).DLL
$(LIBS)
<<
    link @$(PROJ).CRF
