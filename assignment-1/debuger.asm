check_out:
    push    eax
    mov     eax, check_msg
    call    sprintLF
    pop     eax
    ret
;eax, addr
;ebx, num
print_ss:
    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     edx, ebx     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
    mov     ecx, eax   ; move the memory address of our message string into ecx
    mov     ebx, 1      ; write to the STDOUT file
    mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
    int     80h 

    pop    edx
    pop    ecx
    pop    ebx
    pop    eax
    ret

;eax, addr
;ebx, num
print_nums:
    push    ecx
    push    edx
    mov     ecx, 0
    mov     edx, eax
__inner_loop:
    movzx     eax, byte[edx+ecx]
    call    iprint
    inc     ecx
    cmp     ecx, ebx
    jl     __inner_loop

    call    print_lf
    pop      edx
    pop      ecx
    ret

print_flag:
    push    eax
    movzx   eax, byte[flag]
    call    iprintLF
    pop     eax
    ret

print_lf:
    push    edx
    push    ecx
    push    ebx
    push    eax
    push    0Ah
    mov     edx, 1
    mov     ecx, esp
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     eax
    pop     eax
    pop     ebx
    pop     ecx
    pop     edx
    ret

print_sp:
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    20h

    mov     edx, 1
    mov     ecx, esp
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     eax
    
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret
 
 ;eax, dw
 print_dw:
    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     ebx, eax
    mov     ecx, 3
    mov     edx, 0Fh

    shr     eax, 24
    and     eax, edx
    call    iprint
    call    print_sp
    mov     eax, ebx
    shr     eax, 16
    and     eax, edx
    call    iprint
    call    print_sp
    mov     eax, ebx
    shr     eax, 8
    and     eax, edx
    call    iprint
    call    print_sp
    mov     eax, ebx
    and     eax, edx
    call    iprintLF

    pop    edx
    pop    ecx
    pop    ebx
    pop    eax
    ret

print_true:
        push    eax;
        push    ebx;
        push    ecx;
        push    edx;

        mov     ecx, true_flag
        mov     edx, 5
        mov     ebx, 1
        mov     eax, 4
        int     80h

        pop    eax;
        pop    ebx;
        pop    ecx;
        pop    edx;

        ret

print_false:
        push    eax;
        push    ebx;
        push    ecx;
        push    edx;

        mov     ecx, false_flag
        mov     edx, 6
        mov     ebx, 1
        mov     eax, 4
        int     80h

        pop    eax;
        pop    ebx;
        pop    ecx;
        pop    edx;

        ret