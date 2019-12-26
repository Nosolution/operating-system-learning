
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
INT_VECTOR_SYS_CALL equ 0x90
_NR_dly equ 1
_NR_print_str      equ 2
_NR_P 		equ 3
_NR_V		equ 4

; 导出符号
global	get_ticks
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
    push ebx
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	pop ebx
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

