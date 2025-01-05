section .text
global do_nothing;

; Function: do_nothing
; Output: rax = 0
do_nothing:
    mov rax, 1
    ret