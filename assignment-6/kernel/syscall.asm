
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

INT_VECTOR_SYS_CALL equ 0x90
_NR_get_ticks       equ 0
_NR_write	    equ 1
_NR_dly equ 2
_NR_print_str      equ 3
_NR_P 		equ 4
_NR_V		equ 5

; 导出符号
global	get_ticks
global	write
global	dly
global  print_str

global	P
global  V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          void write(char* buf, int len);
; ====================================================================================
write:
        mov     eax, _NR_write
        mov     ebx, [esp + 4]
        mov     ecx, [esp + 8]
        int     INT_VECTOR_SYS_CALL
        ret
dly:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_dly
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret

print_str:
	push ebx
	push ecx

	mov ebx, [esp + 12]
	mov ecx, [esp + 16]

	mov	eax, _NR_print_str
	int	INT_VECTOR_SYS_CALL

	pop ecx
	pop ebx

	ret

P:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_P
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret

V:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_V
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret