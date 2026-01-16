global _start

        section .text

_start:
        xor rdi, rdi    ; return code 0
        mov rax, 60     ; exit syscall
        syscall
