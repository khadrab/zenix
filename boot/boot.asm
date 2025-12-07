; boot/boot.asm - Multiboot kernel entry point with framebuffer support

MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_VIDEO_MODE    equ 1<<2
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_VIDEO_MODE
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

section .multiboot
align 4
multiboot_header:
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM
    
    ; Address fields (unused for ELF kernels)
    dd 0  ; header_addr
    dd 0  ; load_addr
    dd 0  ; load_end_addr
    dd 0  ; bss_end_addr
    dd 0  ; entry_addr
    
    ; Video mode fields
    dd 0        ; mode_type: 0 = linear graphics, 1 = text mode
    dd 320      ; width
    dd 200      ; height
    dd 8        ; depth (8-bit color for VGA mode 13h compatibility)

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Push multiboot info
    push ebx  ; multiboot_info_t* mbi
    push eax  ; magic number
    
    ; Call kernel
    call kernel_main
    
    ; Hang if kernel returns
    cli
.hang:
    hlt
    jmp .hang