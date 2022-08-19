PROJNAME = SHOW16

!ifdef DEBUG
DEBUG_LINK = /CO
!endif

OBJS = $(PROJNAME).obj

$(PROJNAME).exe: $(OBJS) $(PROJNAME).res
  link /ON:N /A:16 $(DEBUG_LINK) $(OBJS), $(PROJNAME), , \
      LIBW.LIB OLDNAMES.LIB LLIBCEW.LIB,$(PROJNAME).def
  RC $(PROJNAME).res $(PROJNAME).exe

.c.obj:
  CL /W3 /YX /GAs /AL /G3 /c /Zi /W3 $<

$(PROJNAME).res: $(PROJNAME).rc 
    RC -R -FO $(PROJNAME).res $(PROJNAME).rc
