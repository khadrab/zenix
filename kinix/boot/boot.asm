; boot/boot.asm
section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .bss
align 16
stack_bottom:
    resb 32768          ; 32KB stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    mov ebp, esp

    ; تأكد إننا في 1M
    jmp enter_kernel

enter_kernel:
    call kernel_main

    cli
.hang:
    hlt
    jmp .hang