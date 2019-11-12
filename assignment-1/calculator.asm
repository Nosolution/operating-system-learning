; 因为使用汇编对单个bit的操作比较蛋疼，因而采取了每字节存取一个十进制数这种十分浪费的实现方式
; 为了速度还把除法实现了一遍，亏大了
; 有写了减法的实现框架，但bug没de完，因此留了一半，也不打算继续实现了



section .bss
input: resb 1
a:  resb 41 ;num1
b:  resb 41 ;num2
c:  resb 41 ;temp
d:  resb 41 ;dest
e:  resb 41 ;temp
tmp:  resb 1
flag:   resb 1  ;nega flag


SECTION .data
nextline:       db  0Ah
minus           equ 45;'-' 
zero            db  0
ten             db  10
num_len         equ 41;41bytes to hold a number(large enough to load a product)
res_nega_bit    equ 4;;the bit of flag will be set 1 if the result is negative
a_nega_bit      equ 2;the bit of flag will be set to 1 if a is negative
b_nega_bit      equ 1;the bit of flag will be set to 1 if b is negative
greet_msg       db 'Please input two number.',0h
invalid_msg     db 'Invalid input. Please reinput.',0h
large_number_msg     db 'Some number is too large. Please reinput.',0h
bad_input_msg     db 'Bad input. Please reinput.',0h
check_msg       db   'checkout',0h
true_flag db    'true', 0Ah
false_flag db   'false',0Ah


SECTION .text
global  _start

_start:

    mov eax, a;
    call set_zero
    mov eax, b;
    call set_zero
    mov eax, c;
    call set_zero
    mov eax, d;
    call set_zero

    mov     byte[flag],  0
    mov     esi, 0
read_a: 
    mov     eax, 3
    mov     ebx, 0
    mov     ecx, input
    mov     edx, 1
    int     80h             ;input sys_call
    sub     byte[input], 30h;atoi
    mov     al,byte[input]

 ;if not a digit       
    cmp     al,0
    jl      __a_check_minus
    cmp     al,9
    jg      __a_check_minus
;has next number, shift left
    inc     esi
    cmp     esi, 21
    jg      large_number_warning   
    mov     ebx, eax
    mov     eax, a
    call    shiftl
    mov     byte[a] , bl
    jmp     read_a

__a_check_minus:
    cmp     al, -3
    je      __a_check_is_first_char;if is minus 
    cmp     esi, 0
    je      bad_input_warning; if has no number stored
    ;set esi to 0, read number b
    mov     esi, 0
    jmp     read_b

__a_check_is_first_char:
    cmp     esi, 0
    jne      bad_input_warning
    push    edx
    mov     edx, a_nega_bit
    and     dl, byte[flag]
    cmp     dl, a_nega_bit
    pop     edx
    je      bad_input_warning;if nega bit has been set
    or      byte[flag], a_nega_bit
    jmp     read_a
    
read_b:
    mov     eax,3
    mov     ebx,0
    mov     ecx,input
    mov     edx,1
    int     80h             ;input sys_call
    sub     byte[input],30h ;atoi
    mov     al,byte[input]
 ;if not a digit       
    cmp     al,0
    jl      __b_check_minus
    cmp     al,9
    jg      __b_check_minus


    
    inc     esi
    cmp     esi, num_len
    jg      large_number_warning
    mov     ebx, eax
    mov     eax, b
    call    shiftl

    mov     byte[b] , bl      
    jmp     read_b

__b_check_minus:
    cmp     al, -3
    je      __b_check_is_first_char;if is minus
    cmp     esi, 0
    je      bad_input_warning; if has no number stored
    jmp     calculate

__b_check_is_first_char:
    cmp     esi, 0
    jne      bad_input_warning
    push    edx
    mov     edx, b_nega_bit
    and     dl, byte[flag]
    cmp     dl, b_nega_bit
    pop     edx
    je      bad_input_warning;if nega bit has been set
    or      byte[flag], b_nega_bit
    jmp     read_b


calculate:
;back up, because a will be a+b
    mov     eax, a
    mov     ebx, c
    call    copy
    mov     eax, b
    mov     ebx, d
    call    copy
;addition
    mov     eax, a
    mov     ebx, b
    call    add_two_num
    call    output
;restore
    mov     eax, c
    mov     ebx, a
    call    copy
    mov     eax, d
    mov     ebx, b
    call    copy
;init
    mov     eax, d
    call    set_zero
    mov     eax, c
    call    set_zero
;multiplication
    mov     eax, a
    mov     ebx, b
    mov     ecx, c
    mov     edx, d
    call    multiply_two_num
    mov     eax, d
    call    output

    jmp     exit


bad_input_warning:
    mov     eax, bad_input_msg
    call    sprintLF
    jmp     exit

large_number_warning:
    mov    eax, large_number_msg
    call    sprintLF
    jmp     exit

exit:
    mov     ebx, 0      ; return 0 status on exit - 'No Errors'
    mov     eax, 1      ; invoke SYS_EXIT (kernel opcode 1)
    int     80h

   

;---------------------------------------
;output the number in addr
;
;param: eax, addr
output:
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
;init
    mov     esi, eax
    mov     ebx, eax
    mov     ecx, num_len-1      ;ecx is the end of digit

__sub_zero_loop:
    mov     eax, ebx
    add     eax, ecx

    call    is_zero
    jne     __check_minus
    dec     ecx
    cmp     ecx, 0
    jnl     __sub_zero_loop;if ecx>=0
    jmp     __print_zero;[eax] is 0

    inc     ecx

__check_minus:
    mov     edx, res_nega_bit
    and     dl, byte[flag]
    cmp     edx, res_nega_bit
    jne     __outputi_loop; if res is not nega
;print_minus:
    push    ecx

    mov     edx, 1
    push    2Dh
    mov     ecx, esp
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     ebx

    pop     ecx
    mov     ebx, esi

__outputi_loop:
    movzx   eax, byte[ebx+ecx]
    call    iprint
    dec     ecx
    cmp     ecx, 0
    jnl     __outputi_loop

;print lf
    push    0Ah
    mov     edx, 1
    mov     ecx, esp
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     edx
 
    pop     esi
    pop     edx
    pop     ebx
    pop     ecx
    pop     eax
    ret

__print_zero:
    mov     edx, 2
    push    0A30h
    mov     ecx, esp
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     eax

    pop     esi
    pop     edx
    pop     ebx
    pop     ecx
    pop     eax
    ret
    


;---------------------------------------
;add two numbers
;
;param: eax, addr1; ebx, addr2
;return: [addr1]+=[addr2]
add_two_num:
    ; call    print_c
    ; call    print_d

    push    eax
    push    ebx
    push    ecx
    push    edx

    push    eax     ;save eax
    mov     eax, a_nega_bit
    call    is_nega

    je      __a_nega    ; if a is nega
    ;else a is not nage
    mov     eax, b_nega_bit
    call    is_nega
    pop     eax
    je      __sbstct_two_num; if b is nega
    jmp     __final_add; else both are posi 

__a_nega:
    mov     eax, b_nega_bit
    call    is_nega 
    pop     eax 
    jne     __swap_operand; if b is not nega
;both a and b are nega
    or      byte[flag], res_nega_bit; result must be nega  

__final_add:
    mov     ecx, eax
    add     ecx, num_len    ;end of number
__add_loop:

    cmp     eax, ecx
    je      __check_res_sign ;jump out of loop
    call    add_digit
    push    ebx
    mov     ebx, ecx
    call    check_carry

    pop     ebx
    inc     eax
    inc     ebx
    jmp     __add_loop
    

__check_res_sign:

    call    is_substraction
    jne     __return; if not sbtct
    movzx   eax, byte[ecx]
    and     eax, 1
    cmp     eax, 1
    je      __return; true if the res of substraction is positive
    
    call    complete; the final carry only occurs in subtraction
    
    or      byte[flag], res_nega_bit

__return:

    mov     byte[ecx], 0; set outside 0
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

;swap the addrs for posi substract 'nega'
__swap_operand:
    xor     eax, ebx
    xor     ebx, eax
    xor     eax, ebx
;[addr1] substracts [addr2]
__sbstct_two_num:
    push    eax
    mov     eax, ebx
    call    complete

    pop     eax
    jmp     __final_add


;---------------------------------------
;check if the calculation is substraction
;
;return: The result is jz/je
is_substraction:
    push    eax

    movzx   eax, byte[flag]
    and     eax, 3
    cmp     eax, 1
    jl      __not
    cmp     eax, 2
    jg      __not
    mov     eax, 1
    cmp     eax, 1;make je = true
    pop     eax
    ret

__not:
    cmp     eax, 1;je==false
    pop     eax
    ret

;---------------------------------------
;make [addr1][7:0]+=[addr2][7:0], careless to carry
; 
;param: eax, addr1
;param: ebx, addr2
add_digit:

    push    eax
    push    ebx
    push    ecx
    push    edx             ;eax = addr1, ebx = addr2, ecx = 0, edx = 0

    mov     ecx, eax        ;eax = addr1, ebx = addr2, ecx = addr1, edx = 0
    mov     eax, ebx        ;eax = addr2, ebx = addr2, ecx = addr1, edx = 0
    call    _get_digit      ;eax = addr2, ebx = b, ecx = addr1, edx = 0
    mov     eax, ecx        ;eax = addr1, ebx = b, ecx = addr1, edx = 0
    call    add_immediate   ;eax = addr1, ebx = b, ecx = addr1, edx = 0, [addr1][7:0] = a+b

    pop     edx    
    pop     ecx
    pop     ebx
    pop     eax
    ret


;---------------------------------------
;make [addr][7:0]carryless to carry
; 
;param: eax, addr careless to carry
;param: ebx, immediate(0-9)
add_immediate:
    push    ecx             ;eax = addr, ebx = i, ecx = 0
    movzx   ecx, byte[eax]  ;eax = addr, ebx = i, ecx = a
    add     ebx, ecx        ;eax = addr, ebx = a+i, ecx = a
    call    _set_digit

    pop     ecx
    ret


;---------------------------------------
;multiply two number in src_addr1 and src_addr2, save result in dest_addr
;
;param: eax, src_addr1; ebx, src_addr2; ecx, tmp_addr; edx, dest_addr
;return: [dest_addr] = [src_addr1] * [src_addr2]
multiply_two_num:
    push    eax
    push    ebx
    push    ecx
    push    esi
    push    edi
    push    edx

    mov     esi, eax        ;esi = addr1
    mov     edi, ebx        ;edi = addr2
    
    push    num_len-1       ;[esp] = i

    
    ; ;set dest addr is zero
    ;     mov     eax, edx
    ;     call    set_zero
    ;copy num1 to tmp addr
    mov     ebx, ecx
    call    copy
__mul_digit_loop:
    mov     ebx, edi
    add     ebx, dword[esp]

    ; mov     ebx, edi+dword[esp] ;ebx = src_addr2 + bias(from num_len-1 to 0)
    call    __mul_digit         ;[tmp_addr] = num in eax * digit in [edi+[esp]]

    mov     eax, edx        ;eax = dest_addr
    mov     ebx, ecx        ;ebx = tmp_addr
    call    add_two_num     ;[dest_addr]+=[tmp_addr]

    mov     eax, esi        ;eax = src_addr1
    dec     dword[esp]

    cmp     dword[esp], 0
    jnl      __loop_with_sl     ;In the last time calculation needs no shift.
    ; cmp     dword[esp], 0
    ; je      __mul_digit_loop    
    

    pop    edx  ;消除num_len-1
    pop    edx
    pop    edi
    pop    esi
    pop    ecx
    pop    ebx
    pop    eax
    ret

__loop_with_sl:
    mov     eax, edx
    call    shiftl
    mov     eax, esi
    jmp     __mul_digit_loop
;---------------------------------------
;multiply a number with a digit
;
;param: eax, number_addr; ebx, digit_addr; ecx, dest_addr 
__mul_digit:

    push    edx
    push    esi
    push    edi
    push    ebx

    mov     esi, eax        ;esi = number_addr
    mov     edi, ecx        ;edi = dest_addr
    push    dword[ebx]      ;[esp] = digit

    mov     eax, ecx        ;eax = dest_addr
    call    set_zero        ;[addr+i] for i in range[0, num_len-1] is zero

    mov     eax, ebx
    call    is_zero
    je      __end_each_digit; if digit is zero, skip the loop

    mov     edx, 0          
    mov     ecx, 0          ;eax = dest_addr, ebx = digit_addr, ecx = 0, edx = 0, [esp] = digit

__each_digit:
    movzx   eax, byte[esi+ecx]  ;eax = number[i], ebx = *, ecx = i, edx = last quatient in [7:0]
    movzx   ebx, byte[esp]      ;eax = number[i], ebx = digit, ecx = i, edx = last quatient in [7:0]          
    push    edx                 ; push to avoid mul overriding       
    
    mul     ebx             ;eax = 0 + digit*num[i] in [7:0], ebx = digit , ecx = i, edx = *
    mov     ebx, 10         ;eax = 0 + digit*num[i] in [7:0], ebx = 10 , ecx = i, edx = *
    div     bl              ;eax = 0 + remainder in [15:8], quatient in [7:0], ebx = 10 , ecx = i, edx = *
    pop     ebx             ;eax = 0 + remainder in [15:8], quatient in [7:0], ebx = last quatient in [7:0] , ecx = i, edx = *
    add     byte[edi+ecx], ah 
    add     byte[edi+ecx], bl;byte[edi+ecx] = remainder + last quatient 
    mov     dl, al          ;eax = 0 + remainder in [15:8], quatient in [7:0], ebx = 10, ecx = i, edx = quatient in [7:0]

    mov     eax, edi
    add     eax, ecx        ;eax = dest_addr+i
    mov     ebx, eax
    inc     ebx             ;edx = dest_addr+i+1, because higher bytes are all zero. At most one carry is needed
    call    check_carry
    inc     ecx
    cmp     ecx, num_len    
    jl      __each_digit


__end_each_digit:
    pop     edx;for digit
    pop     ebx
    mov     eax, esi
    mov     ecx, edi
    pop     edi
    pop     esi
    pop     edx

    ret
    

;---------------------------------------
;check digits from [st_addr] needs carry and do carry until ed_addr
;
;param: eax, st_addr; ebx, ed_addr
check_carry:
    push    eax
    push    ecx
    push    edx
    push    ebx         ;[esp] = ed_addr

__carry_loop:

    mov     ebx, 1      ;ebx is carry
    cmp     eax, [esp] 
    je      __end_carry_loop ;if reached end 
    call    need_carry  
    jl     __end_carry_loop ;if not need carry
;do carry
    movzx   ecx, byte[eax]
    sub     ecx, 10
    and     ecx, 15

    mov     byte[eax], cl
    inc     eax
    call    add_immediate   ;[addr+1]+=1
    jmp     __carry_loop

__end_carry_loop:
    pop    ebx
    pop    edx
    pop    ecx
    pop    eax
    ret


;---------------------------------------
;check if the digit in [addr][7:0] needs carry.
;
;param: eax, addr
;return: The result is jnl
need_carry:
    push    ebx
    call    _get_digit
    cmp     ebx, 10
    ; jnl     print_true
    pop     ebx
    ret

; ---------------------------------------
; set number in addr to zero
;
; param: eax, addr
set_zero:
    push    ecx
    mov     ecx, 0
__zero_loop:
    cmp     ecx, num_len-4
    jng      __zero_dw
;else set one byte to zero
    mov     byte[eax+ecx], 0
    inc     ecx
    cmp     ecx, num_len
    jl      __zero_loop
    jmp     __end_zero_loop
__zero_dw:
    mov     dword[eax+ecx], 0
    add     ecx, 4
    cmp     ecx, num_len
    jl      __zero_loop
__end_zero_loop:
    pop     ecx
    ret



;---------------------------------------
;check if [addr][7:0] is zero.
;
;param: eax, addr
;return: The result is jz/je
is_zero: 
    push    ebx
    call    _get_digit
    cmp     ebx, 0
    pop     ebx
    ret


;---------------------------------------
;get [addr][7:0]
;
;param: eax, addr
;return: ebx, [addr][7:0]
_get_digit:
    movzx     ebx, byte[eax]
    ret


;---------------------------------------
;set value in ebx to addr
;
;param: eax, addr; ebx, value, valid in [7:0]
;return:    [addr][7:0] = ebx[7:0]
_set_digit:
    push    ebx
    and     ebx, 011111111b  
    mov     byte[eax], bl   ;[eax][7:0] = 0[7:4]+i

    pop     ebx
    ret


;---------------------------------------
;load number from bias to ebx
;
;param: eax, addr; ebx, value, valid in [3:0]
;return:    [addr][7:0] = 0000 + ebx[3:0]

;---------------------------------------
;complete digits from addr, assumed length is num_len
;
;param: eax, addr
complete:
    

    push    eax
    push    ecx
    mov     ecx, eax
    add     ecx, num_len    ;ecx acts as the ed
__complete_loop:
    cmp     eax, ecx
    je      __end_complete
    call    __invert
    inc     eax
    jmp     __complete_loop
__end_complete:

    pop     ecx
    pop     eax

    push    ebx
    mov     ebx, 1      ;取补还要+1
    call    add_immediate
    mov     ebx, eax
    add     ebx, num_len
    call    check_carry
    pop     ebx

    ret
;---------------------------------------
;invert the digit in addr in decimal, by a binary invertion then an calculate with 10
;
;param: eax, addr
__invert:
    push    ebx
    mov     ebx, 6
    call    add_immediate
    call    _get_digit      ;ebx: a, assumed that all are in range of [0, 9], implied that ebx[7:4] is 0
    xor     bl, 00001111b   ;ebx[7:0] = 0 + xor ebx[3:0]
    call    _set_digit
    pop     ebx             
    ret


;---------------------------------------
;check if the calculation result is negative
;
;param: eax, nega_bit
;return: The result is jz/je
is_nega:
    push    ecx
    push    edx

    movzx   ecx, byte[flag]
    mov     edx, eax
    and     edx, ecx
    cmp     edx, eax
    pop     edx
    pop     ecx
    ret


;---------------------------------------
;shift the number left
;
;param: eax, addr
shiftl:
    push    ebx
    push    ecx

    mov     ecx, num_len - 1
    ; sub     ecx, 1
__shiftl_loop:
    movzx   ebx, byte[eax+ecx-1]
    mov     byte[eax+ecx], bl
    dec     ecx
    cmp     ecx, 0
    jg      __shiftl_loop; if greater than 0
__end_shiftl_loop:
    mov     byte[eax], 0
    pop     ecx
    pop     ebx
    ret


;---------------------------------------
;shift the number right
;
;param: eax, addr
shiftr:
    push    ebx
    push    ecx

    mov     ecx, 1
__shiftr_loop:
    movzx   ebx, byte[eax+ecx]
    mov     byte[eax+ecx-1], bl
    inc     ecx
    cmp     ecx, num_len - 1
    jng      __shiftr_loop; if greater than 0
__end_shiftr_loop:
    mov     byte[eax+ecx-1], 0
    pop     ecx
    pop     ebx
    ret


;---------------------------------------
;copy the number in addr1 to addr2
;
;param: eax, addr1, ebx, addr2
copy:
    push    edx
    push    ecx
    mov     ecx, 0
__copy_loop:
    cmp     ecx, num_len-4
    jng     __copy_dw
;else copy one byte
    movzx   edx, byte[eax+ecx] ; mov     byte[ebx+ecx], byte[eax+ecx]
    mov     byte[ebx+ecx], dl
    inc     ecx
    cmp     ecx, num_len
    jl      __copy_loop
    jmp     __end_copy_loop
__copy_dw:
    mov     edx, [eax+ecx] ; mov     [ebx+ecx], [eax+ecx]
    mov     [ebx+ecx],  edx
    inc     ecx
    cmp     ecx, num_len
    jl      __copy_loop
__end_copy_loop:
    pop     ecx
    pop     edx
    ret


;-------------------------------------------------copied codes----------------------------------------------------
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
 
 
;------------------------------------------
; void iprintLF(Integer number)
; Integer printing function with linefeed (itoa)
iprintLF:
    call    iprint          ; call our integer printing function
 
    push    eax             ; push eax onto the stack to preserve it while we use the eax register in this function
    mov     eax, 0Ah        ; move 0Ah into eax - 0Ah is the ascii character for a linefeed
    push    eax             ; push the linefeed onto the stack so we can get the address
    mov     eax, esp        ; move the address of the current stack pointer into eax for sprint
    call    sprint          ; call our sprint function
    pop     eax             ; remove our linefeed character from the stack
    pop     eax             ; restore the original value of eax before our function was called
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



; print_a:
;     push    eax
;     push    ebx
;     mov     eax, a
;     mov     ebx, num_len
;     call    print_nums
;     pop     ebx
;     pop     eax
;     ret

; print_b:
;     push    eax
;     push    ebx
;     mov     eax, b
;     mov     ebx, num_len
;     call    print_nums
;     pop     ebx
;     pop     eax
;     ret

; print_c:
;     push    eax
;     push    ebx
;     mov     eax, c
;     mov     ebx, num_len
;     call    print_nums
;     pop     ebx
;     pop     eax
;     ret

; print_d:
;     push    eax
;     push    ebx
;     mov     eax, d
;     mov     ebx, num_len
;     call    print_nums
;     pop     ebx
;     pop     eax
;     ret