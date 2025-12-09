[BITS 16]
[ORG 0x7C00]

start:
    ; Set video mode 13h
    mov ax, 0x0013
    int 0x10
    
    ; Load kernel at 0x1000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 50      ; sectors
    mov ch, 0       ; cylinder
    mov cl, 2       ; sector
    mov dh, 0       ; head
    int 0x13
    
    ; Jump to kernel
    jmp 0x1000:0x0000

times 510-($-$$) db 0
dw 0xAA55
