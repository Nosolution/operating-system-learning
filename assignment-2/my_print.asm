global asm_prints
global asm_printi
global asm_printcs


    section .data
    ;定义颜色输出
    color_red:      db  1Bh, '[31;1m', 0 ;红色，31代表红色，1m中的1代表样式（1m->高亮，4m->下划线，etc）
    .len            equ $ - color_red
    color_default:  db  1Bh, '[37;0m', 0 ;默认色
    .len            equ $ - color_default
    color_sign:     db 0 ;颜色标志位，当前为白色->0，红色->1
    check_msg       db   'checkout',0h      


    section .text
asm_prints:
    
    mov eax, [esp+4]
    mov ebx, [esp+8]
    cmp ebx, 0
    je  default_color
    jmp red_color
default_color:
    call sprint
    jmp end_print
red_color:
    call set_red
    call sprint
    call set_default

end_print:
    ; pop eax
    ; pop eax
    
    ret

asm_printi:
    ; call set_default
    mov eax, [esp+4]
    call iprint
    ret

asm_printcs:
    ; call    set_default
    
    mov     ecx, [esp+4]
    mov     edx, [esp+8]
    mov     ebx, 1
    mov     eax, 4
    int     80h
    ret


set_red:
    push eax
    push ebx
    push ecx
    push edx

	mov eax, 4
    mov ebx, 1
    mov ecx, color_red
    mov edx, color_red.len
    int 80h
	
	; mov byte[color_sign], 1

    pop edx
    pop ecx
    pop ebx
    pop eax
	ret

set_default:
    push eax
    push ebx
    push ecx
    push edx

    mov eax, 4
    mov ebx, 1
    mov ecx, color_default
    mov edx, color_default.len
    int 80h
	
	; mov byte[color_sign], 0
    pop edx
    pop ecx
    pop ebx
    pop eax
	ret

check_out:
    push    eax
    mov     eax, check_msg
    call    sprintLF
    pop     eax
    ret


;------------------------------------------
; int slen(String message)
; String length calculation function
slen:
    push    ebx
    mov     ebx, eax
 
nextchar:
    cmp     byte [eax], 0
    jz      finished
    inc     eax
    jmp     nextchar
 
finished:
    sub     eax, ebx
    pop     ebx
    ret
 
;------------------------------------------
; void sprintLF(String message)
; String printing with line feed function
sprintLF:
    call    sprint
 
    push    eax
    mov     eax, 0AH
    push    eax
    mov     eax, esp
    call    sprint
    pop     eax
    pop     eax
    ret

;------------------------------------------
; void sprint(String message)
; String printing function
sprint:
    push    edx
    push    ecx
    push    ebx
    push    eax
    call    slen
 
    mov     edx, eax
    pop     eax
 
    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h
 
    pop     ebx
    pop     ecx
    pop     edx
    ret

;------------------------------------------
; void iprint(Integer number)
; Integer printing function (itoa)
iprint:
    push    eax             ; preserve eax on the stack to be restored after function runs
    push    ecx             ; preserve ecx on the stack to be restored after function runs
    push    edx             ; preserve edx on the stack to be restored after function runs
    push    esi             ; preserve esi on the stack to be restored after function runs
    mov     ecx, 0          ; counter of how many bytes we need to print in the end
 
divideLoop:
    inc     ecx             ; count each byte to print - number of characters
    mov     edx, 0          ; empty edx
    mov     esi, 10         ; mov 10 into esi
    idiv    esi             ; divide eax by esi
    add     edx, 48         ; convert edx to it's ascii representation - edx holds the remainder after a divide instruction
    push    edx             ; push edx (string representation of an intger) onto the stack
    cmp     eax, 0          ; can the integer be divided anymore?
    jnz     divideLoop      ; jump if not zero to the label divideLoop
 
printLoop:
    dec     ecx             ; count down each byte that we put on the stack
    mov     eax, esp        ; mov the stack pointer into eax for printing
    call    sprint          ; call our string print function
    pop     eax             ; remove last character from the stack to move esp forward
    cmp     ecx, 0          ; have we printed all bytes we pushed onto the stack?
    jnz     printLoop       ; jump is not zero to the label printLoop
 
    pop     esi             ; restore esi from the value we pushed onto the stack at the start
    pop     edx             ; restore edx from the value we pushed onto the stack at the start
    pop     ecx             ; restore ecx from the value we pushed onto the stack at the start
    pop     eax             ; restore eax from the value we pushed onto the stack at the start
    ret