#include "usermode.h"
#include "../core/monitor.h"
#include "../hal/gdt.h"
#include "../mm/heap.h"

extern void jump_usermode(void);

// Kernel stack for TSS (used when returning from user mode)
static uint32_t kernel_stack_top = 0;

void usermode_init(void) {
    print_string("       Setting up user mode...\n");
    print_string("       Allocating kernel stack...\n");
    
    // Allocate 8KB kernel stack for TSS
    uint32_t kernel_stack = (uint32_t)kmalloc(0x2000);
    if (kernel_stack == 0) {
        print_string("       [ERROR] Failed to allocate kernel stack!\n");
        return;
    }
    
    // Stack grows downward, so set to top of allocated region
    kernel_stack_top = kernel_stack + 0x2000;
    
    // Set TSS ESP0 to kernel stack
    set_kernel_stack(kernel_stack_top);
    
    print_string("       Kernel stack set at 0x");
    print_hex(kernel_stack_top);
    print_string("\n");
}

void sample_usermode_program(void) {
    volatile int x = 0;
    while(1) {
        x++;
        if (x > 1000000) {
            asm volatile(
                "mov $1, %%eax\n"
                "mov $1, %%ebx\n"
                "mov %0, %%ecx\n"
                "mov $25, %%edx\n"
                "int $0x80\n"
                :
                : "r"("Hello from Ring 3!\n")
                : "eax", "ebx", "ecx", "edx"
            );
            x = 0;
        }
    }
}

void enter_usermode(void) {
    print_string("\n[USERMODE] Switching to Ring 3...\n");
    
    // Allocate user stack (8KB)
    uint32_t user_stack = (uint32_t)kmalloc(0x2000);
    if (user_stack == 0) {
        print_string("[ERROR] Failed to allocate user stack!\n");
        return;
    }
    user_stack += 0x2000;  // Point to top of stack
    
    print_string("[USERMODE] User stack at 0x");
    print_hex(user_stack);
    print_string("\n");
    
    print_string("[USERMODE] Kernel stack at 0x");
    print_hex(kernel_stack_top);
    print_string("\n");
    
    print_string("[USERMODE] Jumping to Ring 3...\n\n");
    
    asm volatile(
        "cli\n"
        
        // Set segment registers to user data segment (0x23 = 0x20 | 0x03)
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        // Push SS (stack segment)
        "pushl $0x23\n"
        
        // Push ESP (stack pointer) - use allocated user stack
        "pushl %1\n"
        
        // Push EFLAGS with interrupts enabled
        "pushf\n"
        "pop %%eax\n"
        "or $0x200, %%eax\n"    // Set IF (interrupt flag)
        "push %%eax\n"
        
        // Push CS (code segment) - user code segment (0x1B = 0x18 | 0x03)
        "pushl $0x1B\n"
        
        // Push EIP (instruction pointer) - address of user function
        "push %0\n"
        
        // Return to user mode
        "iret\n"
        :
        : "r"(sample_usermode_program), "r"(user_stack)
        : "eax"
    );
}

void usermode_test(void) {
    print_string("=== User Mode Test ===\n\n");
    print_string("Current privilege: Ring 0 (Kernel)\n");
    print_string("Preparing to jump to Ring 3...\n\n");
    
    // Make sure TSS is set up
    if (kernel_stack_top == 0) {
        print_string("[ERROR] Kernel stack not initialized!\n");
        print_string("Call usermode_init() first!\n");
        return;
    }
    
    enter_usermode();
}