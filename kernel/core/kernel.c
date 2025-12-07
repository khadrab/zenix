#include "../core/monitor.h"
#include "../hal/gdt.h"
#include "../hal/idt.h"
#include "../hal/isr.h"
#include "../hal/irq.h"
#include "../mm/pmm.h"
#include "../mm/heap.h"
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../drivers/timer/pit.h"
#include "../drivers/keyboard/keyboard.h"
#include "../shell/shell.h"
#include "../fs/vfs.h"
#include "../syscall/syscall.h"
#include "../usermode/usermode.h"
#include "../../include/multiboot.h"

extern uint32_t kernel_end;
extern fs_node_t* fs_root;

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    clear_screen();
    print_string("=================================\n");
    print_string(" Zenix Kernel v1.0 - Phase 10   \n");
    print_string("=================================\n\n");
    
    if (magic != 0x2BADB002) {
        print_string("[ERROR] Invalid multiboot!\n");
        for(;;) asm("cli; hlt");
    }
    
    print_string("[1/14] GDT..."); gdt_init(); print_string(" [OK]\n");
    print_string("[2/14] IDT..."); idt_init(); print_string(" [OK]\n");
    print_string("[3/14] ISRs..."); isr_install(); print_string(" [OK]\n");
    print_string("[4/14] IRQs..."); irq_install(); print_string(" [OK]\n");
    print_string("[5/14] PMM..."); pmm_init(mbi, (uint32_t)&kernel_end); print_string(" [OK]\n");
    print_string("[6/14] Heap..."); heap_init(); print_string(" [OK]\n");
    print_string("[7/14] VFS..."); fs_root = 0; print_string(" [OK]\n");
    print_string("[8/14] InitRD... [SKIP]\n");
    print_string("[9/14] Process..."); process_init(); print_string(" [OK]\n");
    print_string("[10/14] Scheduler..."); scheduler_init(); print_string(" [OK]\n");
    print_string("[11/14] Timer..."); timer_init(100); print_string(" [OK]\n");
    print_string("[12/14] Keyboard..."); keyboard_init(); print_string(" [OK]\n");
    print_string("[13/14] Syscalls...");
    syscall_init();
    syscall_handlers_init();
    print_string(" [OK]\n");
    
    print_string("[14/14] User Mode...");
    usermode_init();
    print_string(" [OK]\n");
    
    print_string("\n=================================\n");
    print_string("  Phase 10 Complete!\n");
    print_string("  - User Mode: Ready\n");
    print_string("  - Ring 3: Ready\n");
    print_string("=================================\n\n");
    
    shell_init();
    shell_run();
    
    for(;;) asm("cli; hlt");
}
