;==================================
; APISPY32 - Matt Pietrek 1995
; FILE: ASMRETRN.ASM
;==================================

.386
.model flat

extrn _CCommonReturnPoint:proc

.code

public _AsmCommonReturnPoint

_AsmCommonReturnPoint proc
    SUB     ESP,4   ; Make space for return address
    PUSHAD
    MOV     EAX,ESP
    PUSH    EAX
    CALL    _CCommonReturnPoint
    ADD     ESP,4
    POPAD
    RET
_AsmCommonReturnPoint endp

END
