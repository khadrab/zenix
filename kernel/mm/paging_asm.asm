; kernel/mm/paging_asm.asm
; Paging assembly functions
[BITS 32]

global paging_load_directory
global paging_enable

; Load page directory into CR3
paging_load_directory:
    push ebp
    mov ebp, esp
    mov eax, [esp + 8]    ; Get directory address
    mov cr3, eax          ; Load into CR3
    mov esp, ebp
    pop ebp
    ret

; Enable paging by setting CR0.PG
paging_enable:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000    ; Set PG bit
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret