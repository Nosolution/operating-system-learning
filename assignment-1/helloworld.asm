            ; Hello World Program - asmtutor.com
            ; Compile with: nasm -f elf helloworld.asm
            ; Link with (64 bit systems require elf_i386 option): ld -m elf_i386 helloworld.o -o helloworld
            ; Run with: ./helloworld
%include        'functions.asm'
%include        'debuger.asm'


SECTION .data
msg     db      'Hello World!', 0Ah, 0h     ; assign msg variable with your message string
true_flag db    'true', 0Ah, 0h
false_flag db   'false',0Ah, 0h
check_msg       db   'checkout',0h

one     equ     1

SECTION .bss
tmp     resw    2
d       resb    1
flag:   resb 1

SECTION .text
global  _start

_start:

        push    msg

        mov     edx, 13     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
        mov     ecx, [esp]    ; move the memory address of our message string into ecx
        mov     ebx, 1      ; write to the STDOUT file
        mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
        int     80h      

        mov     dword[tmp], 12345678h
        mov     ebx, tmp
        lea     eax, [ebx];  = mov   eax, ebx
        call    iprintLF
        ; lea     eax, [ebx]
        ; call    iprintLF
        ; mov     eax, [eax]
        ; call    iprintLF
        ; je      print_true
        ; jmp     print_false        
        ; mov     eax, dword[tmp+1]
        ; call    iprintLF
        
    
done:
        mov     ebx, 0      ; return 0 status on exit - 'No Errors'
        mov     eax, 1      ; invoke SYS_EXIT (kernel opcode 1)
        int     80h

t:
        cmp     eax,one
        ret



test_flag:
        push    eax
        mov     eax, 2
        cmp     eax, 1
        pop     eax
        ret


