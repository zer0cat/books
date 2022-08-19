;==================================
; WIN95UNI - Matt Pietrek 1995
; FILE: WIN95UNI.C
;==================================

.386
.model small

.code

public __GetPhysicalAddrFromLinear

__GetPhysicalAddrFromLinear proc
    push    ebp
    mov     ebp, esp

	mov		eax, [ebp+0Ch]

	shr		eax,22
	test	DWORD PTR [0FFBFE000h + eax*4], 1		;; Verify that the Page
	jz		fail									;; Directory is OK

	mov		eax,[ebp+0Ch]
	mov		ebx, eax
	shr		eax,12
	mov		eax, DWORD PTR [0FF800000h + eax * 4]	;; Get Page Table entry
	test	eax,1									;; is it present?
	jz		fail

	and		EAX, 0FFFFF000h			;; Get physical page
	and		ebx, 000000FFFh			;; combine with low order 12 bytes of
	or		eax, ebx				;; the linear address
	jmp		done

fail:
	mov		eax, -1

done:
    pop     ebp

	retf 4		;; NOTE! This is a 16:32 RETF, since we're in a 32 bit seg
__GetPhysicalAddrFromLinear endp

public __GetPageAttributes

__GetPageAttributes proc
    push    ebp
    mov     ebp, esp

	mov		eax, [ebp+0Ch]

	shr		eax,22
	test	DWORD PTR [0FFBFE000h + eax*4], 1		;; Verify that the Page
	jz		fail									;; Directory is OK

	mov		eax,[ebp+0Ch]
	mov		ebx, eax
	shr		eax,12
	mov		eax, DWORD PTR [0FF800000h + eax * 4]	;; Get Page Table entry
	test	eax,1									;; is it present?
	jz		fail

	and		eax, 000000FFFh
	jmp		done

fail:
	mov		eax, -1

done:
    pop     ebp

	retf 4		;; NOTE! This is a 16:32 RETF, since we're in a 32 bit seg
__GetPageAttributes endp

END
