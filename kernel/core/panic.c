// kernel/core/panic.c
#include "panic.h"
#include "monitor.h"

// Print stack trace
void print_stack_trace(uint32_t* ebp, int max_frames) {
    print_string("\nStack Trace:\n");
    print_string("------------\n");
    
    for (int frame = 0; frame < max_frames && ebp; frame++) {
        uint32_t eip = ebp[1];  // Return address
        
        if (eip == 0) break;
        
        print_string("  [");
        print_dec(frame);
        print_string("] 0x");
        print_hex(eip);
        
        // Try to get function name (would need symbol table)
        print_string("\n");
        
        // Move to next frame
        ebp = (uint32_t*)ebp[0];
        
        // Sanity check
        if ((uint32_t)ebp < 0x1000 || (uint32_t)ebp > 0x10000000) {
            break;
        }
    }
}

// Dump all registers
void dump_registers(registers_t* regs) {
    print_string("\nRegister Dump:\n");
    print_string("--------------\n");
    
    print_string("EAX=0x"); print_hex(regs->eax);
    print_string(" EBX=0x"); print_hex(regs->ebx);
    print_string(" ECX=0x"); print_hex(regs->ecx);
    print_string(" EDX=0x"); print_hex(regs->edx);
    print_string("\n");
    
    print_string("ESI=0x"); print_hex(regs->esi);
    print_string(" EDI=0x"); print_hex(regs->edi);
    print_string(" EBP=0x"); print_hex(regs->ebp);
    print_string(" ESP=0x"); print_hex(regs->esp);
    print_string("\n");
    
    print_string("EIP=0x"); print_hex(regs->eip);
    print_string(" EFLAGS=0x"); print_hex(regs->eflags);
    print_string("\n");
    
    print_string("CS=0x"); print_hex(regs->cs);
    print_string(" DS=0x"); print_hex(regs->ds);
    print_string(" SS=0x"); print_hex(regs->ss);
    print_string("\n");
}

// Kernel panic
void kernel_panic(const char* message, const char* file, uint32_t line) {
    asm volatile("cli");  // Disable interrupts
    
    print_string("\n\n");
    print_string("=====================================\n");
    print_string("!!!   KERNEL PANIC   !!!\n");
    print_string("=====================================\n\n");
    
    print_string("Message: ");
    print_string(message);
    print_string("\n");
    
    print_string("Location: ");
    print_string(file);
    print_string(":");
    print_dec(line);
    print_string("\n");
    
    // Get current EBP for stack trace
    uint32_t ebp;
    asm volatile("mov %%ebp, %0" : "=r"(ebp));
    print_stack_trace((uint32_t*)ebp, 10);
    
    print_string("\n");
    print_string("=====================================\n");
    print_string("System Halted.\n");
    print_string("=====================================\n");
    
    // Halt forever
    for(;;) {
        asm volatile("hlt");
    }
}

// Division by zero handler
void division_error_handler(registers_t* regs) {
    print_string("\n\n!!! DIVISION BY ZERO !!!\n");
    print_string("=========================\n");
    
    print_string("EIP: 0x");
    print_hex(regs->eip);
    print_string("\n");
    
    dump_registers(regs);
    print_stack_trace((uint32_t*)regs->ebp, 10);
    
    PANIC("Division by zero exception");
}

// General Protection Fault
void general_protection_fault_handler(registers_t* regs) {
    print_string("\n\n!!! GENERAL PROTECTION FAULT !!!\n");
    print_string("=================================\n");
    
    print_string("Error Code: 0x");
    print_hex(regs->err_code);
    print_string("\n");
    
    print_string("EIP: 0x");
    print_hex(regs->eip);
    print_string("\n");
    
    // Decode error code
    if (regs->err_code & 1) {
        print_string("External event\n");
    }
    
    uint32_t selector = regs->err_code >> 3;
    print_string("Selector: 0x");
    print_hex(selector);
    print_string("\n");
    
    dump_registers(regs);
    print_stack_trace((uint32_t*)regs->ebp, 10);
    
    PANIC("General protection fault");
}

// Double Fault
void double_fault_handler(registers_t* regs) {
    print_string("\n\n!!! DOUBLE FAULT !!!\n");
    print_string("=====================\n");
    
    print_string("This usually means the kernel stack is corrupted.\n");
    print_string("Error Code: 0x");
    print_hex(regs->err_code);
    print_string("\n");
    
    dump_registers(regs);
    
    PANIC("Double fault exception");
}