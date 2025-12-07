// kernel/hal/irq.c

#include "irq.h"
#include "idt.h"
#include "pic.h"
#include "../core/monitor.h"

// IRQ handler table
static isr_handler_t irq_handlers[16] = {0};

// Register an IRQ handler
void irq_register_handler(uint8_t irq, isr_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

// Common IRQ handler
void irq_handler(registers_t* regs) {
    // Check if this is a spurious IRQ7 or IRQ15
    if (regs->int_no == 39) {  // IRQ7
        if (!pic_get_isr(7)) {
            return; // Spurious IRQ7
        }
    }
    if (regs->int_no == 47) {  // IRQ15
        if (!pic_get_isr(15)) {
            pic_send_eoi(2);  // EOI to slave only
            return; // Spurious IRQ15
        }
    }
    
    // Call the registered handler if exists
    uint8_t irq_num = regs->int_no - 32;
    if (irq_num < 16 && irq_handlers[irq_num]) {
        irq_handlers[irq_num](regs);
    }
    
    // Send EOI
    pic_send_eoi(irq_num);
}

// Install IRQ handlers
void irq_install(void) {
    // Remap PIC
    pic_remap(0x20, 0x28);
    
    // Install IRQ handlers in IDT
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    // Unmask all IRQs
    pic_set_mask(0x00, 0x00);
    
    // Enable interrupts
    asm volatile("sti");
}