; boot/boot.asm - Full VESA VBE Support
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
    dd 0, 0, 0, 0, 0  ; Unused for ELF
    
    ; Video mode request
    dd 0              ; mode_type (0 = linear graphics)
    dd 1024           ; width
    dd 768            ; height  
    dd 32             ; depth (32-bit color)

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .data
global vbe_mode_info
vbe_mode_info:
    .width:       dd 320
    .height:      dd 200
    .pitch:       dd 320
    .bpp:         dd 8
    .framebuffer: dd 0xA0000

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    
    ; Check for framebuffer info (GRUB 2)
    mov edi, ebx
    test dword [edi], 1 << 12
    jz .no_framebuffer
    
    ; Extract framebuffer info
    mov eax, [edi + 88]
    mov [vbe_mode_info.framebuffer], eax
    
    mov eax, [edi + 96]
    mov [vbe_mode_info.width], eax
    
    mov eax, [edi + 100]
    mov [vbe_mode_info.height], eax
    
    mov eax, [edi + 104]
    mov [vbe_mode_info.pitch], eax
    
    movzx eax, byte [edi + 110]
    mov [vbe_mode_info.bpp], eax
    
    jmp .boot_kernel
    
.no_framebuffer:
    ; Fallback values (Mode 13h)
    mov dword [vbe_mode_info.width], 320
    mov dword [vbe_mode_info.height], 200
    mov dword [vbe_mode_info.pitch], 320
    mov dword [vbe_mode_info.bpp], 8
    mov dword [vbe_mode_info.framebuffer], 0xA0000
    
.boot_kernel:
    push ebx
    push eax
    call kernel_main
    
    cli
.hang:
    hlt
    jmp .hang