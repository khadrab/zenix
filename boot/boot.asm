bits 32

section .multiboot
    MULTIBOOT_MAGIC equ 0x1BADB002
    MULTIBOOT_FLAGS equ 0x00000003
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
    
    align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern kernel_main
extern kernel_end

_start:
    mov esp, stack_top
    
    push ebx  ; Multiboot info structure
    push eax  ; Multiboot magic
    
    call kernel_main
    
    cli
.hang:
    hlt
    jmp .hang