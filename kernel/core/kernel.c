#include "monitor.h"
#include "../../include/types.h"
#include "../../include/multiboot.h"

extern void timer_init(uint32_t frequency);
extern void irq_install();
extern void keyboard_init();
extern void shell_init();
extern void shell_run();
extern void pmm_init(multiboot_info_t* mbi, uint32_t kernel_end);
extern void paging_init();
extern void heap_init();
extern void process_init();
extern void scheduler_init();

extern uint32_t kernel_end;

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    clear_screen();
    
    print_string("  _______                       \n");
    print_string(" '      /    ___  , __   ` _  .-\n");
    print_string("    .--'   .'   ` |'  `. |  \\,' \n");
    print_string("   /       |----' |    | |  /\\  \n");
    print_string(" ,'______/ `.___, /    | / /  \\ \n");
    print_string("                                 \n");
    print_string("  Zenix Kernel v0.7 - Phase 7   \n");
    print_string("=================================\n\n");
    
    if (magic != 0x2BADB002) {
        print_string("[ERROR] Invalid multiboot magic\n");
        while(1) asm("hlt");
    }
    
    print_string("[1/11] GDT...\n");
    gdt_init();
    
    print_string("[2/11] IDT...\n");
    idt_init();
    
    print_string("[3/11] ISRs...\n");
    isr_install();
    
    print_string("[4/11] IRQs...\n");
    irq_install();
    
    print_string("[5/11] PMM...\n");
    pmm_init(mbi, (uint32_t)&kernel_end);
    
    // print_string("[6/11] Paging...\n");
    // paging_init();
    
    print_string("[7/11] Heap...\n");
    heap_init();
    
    print_string("[8/11] Process Manager...\n");
    process_init();
    
    print_string("[9/11] Scheduler...\n");
    scheduler_init();
    
    print_string("[10/11] Timer (100Hz)...\n");
    timer_init(100);
    
    print_string("[11/11] Keyboard...\n");
    keyboard_init();
    
    print_string("\n[OK] All systems initialized!\n");
    print_string("Starting shell...\n\n");
    
    for (volatile int i = 0; i < 10000000; i++);
    
    shell_init();
    shell_run();
    
    while(1) {
        asm volatile("hlt");
    }
}