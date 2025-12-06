global switch_context

switch_context:
    push ebp
    mov ebp, esp
    
    ; Save old context
    mov eax, [ebp + 8]    ; old_esp pointer
    
    pushf
    push ebx
    push esi
    push edi
    
    mov [eax], esp        ; Save old ESP
    
    ; Load new context
    mov eax, [ebp + 12]   ; new_esp value
    mov esp, eax          ; Switch to new stack
    
    pop edi
    pop esi
    pop ebx
    popf
    
    pop ebp
    ret