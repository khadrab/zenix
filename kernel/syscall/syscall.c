#include "syscall.h"
#include "../hal/idt.h"
#include "../core/monitor.h"

static syscall_handler_t syscall_table[MAX_SYSCALLS];

extern void syscall_entry(void);

void syscall_handler(registers_t* regs) {
    uint32_t syscall_num = regs->eax;
    
    if (syscall_num >= MAX_SYSCALLS || !syscall_table[syscall_num]) {
        regs->eax = (uint32_t)-1;
        return;
    }
    
    int ret = syscall_table[syscall_num](
        regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi
    );
    
    regs->eax = (uint32_t)ret;
}

void syscall_init(void) {
    for (int i = 0; i < MAX_SYSCALLS; i++) {
        syscall_table[i] = 0;
    }
    
    idt_set_gate(0x80, (uint32_t)syscall_entry, 0x08, 0xEE);
}

void syscall_register(uint32_t num, syscall_handler_t handler) {
    if (num < MAX_SYSCALLS) {
        syscall_table[num] = handler;
    }
}
