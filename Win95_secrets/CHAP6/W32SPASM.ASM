;==================================
; W32SVSPY - Matt Pietrek 1995
; FILE: W32SPASM.ASM
;==================================
.386
.model flat

extrn   _lpfnOriginalVxDCall:fword
extrn   _LogVxDCall@8:proc

.code

public _NewVxDCall_entry
_NewVxDCall_entry proc
    pushad
    push    eax
    lea     eax, [esp+36]
    push    eax
    call    _LogVxDCall@8
    popad
    jmp     [_lpfnOriginalVxDCall]
_NewVxDCall_entry endp

end
