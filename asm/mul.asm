	
                section         .text
                %define         LEN 128 * 8

                global          _start
_start:

                sub             rsp, 4 * LEN
                mov             rcx, 128

                lea             rdi, [rsp + LEN]
                call            read_long
                mov             rdi, rsp
                call            read_long

                lea             rsi, [rsp + LEN]
                lea             r8, [rsp + 2 * LEN]
;   rsi - first multiplyer
;   rdi - second multiplyer
;   r8 - answer 
                call            mul_long_long

                mov             rdi, r8
                add 			rcx, rcx
                call            write_long

                mov             al, 0x0a
                call            write_char

                jmp             exit
                
; multiplies two long numbers
;    rdi - adress of multiplyer #1 (long number)
;    rsi - adress of multiplyer #2 (long number)
;    rcx - length of long number in qwords
; result:
;   answer is written to r8(length of the answer is 2 * rcx)
mul_long_long:
                push            rsi
                push            rdi
                push            rcx 
                push 			r10

                xor             r9, r9

                ; allocate memory for an additional number
                mov             rbp, rsp 
                sub             rsp, LEN

.loop:          
                ; remember the first multiplier
                mov 			r11, rsp
                push            rsi
                lea             rsi, [r11]
                call            copy_long_number
                pop             rsi

                ; multiply the first multiplier by the next digit of the second
                mov             rbx, [rsi]
                push 			rdi
                lea 			rdi, [r11]
                call            mul_long_short
                pop 			rdi            
                add             rsi, 8
       
                ; add this number with offset
                push            rsi
                push            rdi
                lea 			rdi, [r11]
                mov             rsi, r8
                call            add_long_long 
                pop             rdi
                pop             rsi
           
                ; increase offset
                inc             r9
                cmp		 		r9, rcx
                jne             .loop

                mov             rsp, rbp   

                pop 			r10   
                pop             rcx
                pop             rdi 
                pop             rsi

                ret

; copies long number
;   rdi - adress from
;   rsi - adress where
;   rcx - length of number in qwords
copy_long_number:
                push            rdi
                push            rsi 
                push            rcx                
         
.loop:          
                mov             rax, [rdi]
                mov             [rsi], rax                
                add             rdi,    8
                add             rsi,    8
                dec             rcx
                jnz             .loop
                       
                pop             rcx
                pop             rsi 
                pop             rdi
                ret    
                
                
; adds two long number with offset: 
;    rdi - address of summand #1 (long number)
;    rsi - address of summand #2 (long number)
;    rcx - length of long numbers in qwords
;    r9 - offset rdi in qwords
;    r10 - reminder from mul_long_short
; result:
;    sum is written to rsi
add_long_long:
                push            rdi
                push            rsi
                push            rcx
                push            r9

                clc
.loop:

                mov             rax, [rdi]
                lea             rdi, [rdi + 8]
                adc             [rsi + 8 * r9], rax
                lea             rsi, [rsi + 8]
                dec             rcx
                jnz             .loop

                ; add remainder from mul_long_short
                adc		 		[rsi + 8 * r9], r10

                pop             r9
                pop             rcx
                pop             rsi
                pop             rdi
                ret

; adds 64-bit number to long number
;    rdi - address of summand #1 (long number)
;    rax - summand #2 (64-bit unsigned)
;    rcx - length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies long number by a short
;    rdi - address of multiplier #1 (long number)
;    rbx - multiplier #2 (64-bit unsigned)
;    rcx - length of long number in qwords
; result:
;    product is written to rdi
;    r10 - reminder
mul_long_short:
                push            rax
                push            rdi
                push            rcx

                xor             r10, r10
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, r10
                adc             rdx, 0
                mov             [rdi], rax
                add             rdi, 8
                mov             r10, rdx
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rdi
                pop             rax
                ret

; divides long number by a short
;    rdi - address of dividend (long number)
;    rbx - divisor (64-bit unsigned)
;    rcx - length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx - remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

; assigns a zero to long number
;    rdi - argument (long number)
;    rcx - length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi - argument (long number)
;    rcx - length of long number in qwords
; result:
;    ZF = 1 if zero
is_zero:
                push            rax
                push            rdi 

                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi - location for output (long number)
;    rcx - length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi - argument (long number)
;    rcx - length of long number in qwords
write_long:
                push            rax
                push            rcx

                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al - char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi - string
;    rdx - size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


                section         .rodata
invalid_char_msg:
                db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg