#include <stdint.h>
#include "monitor.h"

extern void timer_init(uint32_t frequency);
extern void irq_install();
extern uint32_t timer_get_ticks();

void kernel_main() {
    clear_screen();
    
    print_string("                                     \n");
    print_string("'||'  |'   ||            ||          \n");
    print_string(" || .'    ...  .. ...   ...  ... ... \n");
    print_string(" ||'|.     ||   ||  ||   ||   '|..'  \n");
    print_string(" ||  ||    ||   ||  ||   ||    .|.   \n");
    print_string(".||.  ||. .||. .||. ||. .||. .|  ||. \n");
    print_string("                                     \n");
    print_string("     Kinix Kernel  v0.3 - Phase 3    \n");
    print_string("=====================================\n");
    print_string("\n");
    
    print_string("[INIT] Initializing GDT...\n");
    gdt_init();
    print_string("[OK] GDT loaded successfully\n");
    
    print_string("[INIT] Initializing IDT...\n");
    idt_init();
    print_string("[OK] IDT loaded successfully\n");
    
    print_string("[INIT] Installing ISRs...\n");
    isr_install();
    print_string("[OK] ISRs installed\n");
    
    print_string("[INIT] Installing IRQ handlers...\n");
    irq_install();
    print_string("[OK] IRQ handlers installed\n");
    
    print_string("[INIT] Initializing Timer (100Hz)...\n");
    timer_init(100);
    print_string("[OK] Timer initialized\n");
    
    print_string("\n");
    print_string("System ready! Timer ticks: ");

    for (;;) {
    asm volatile("hlt");
    }
}