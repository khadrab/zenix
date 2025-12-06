#include "irq.h"
#include "idt.h"
#include "isr.h"
#include "../../include/io.h"

void irq_register_handler(uint8_t irq, isr_t handler) {
    isr_register_handler(IRQ0 + irq, handler);
}

void irq_install() {
    pic_remap(0x20, 0x28);
    
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    asm volatile("sti");
}

void irq_handler(registers_t* regs) {
    // Call handler
    if (interrupt_handlers[regs->int_no]) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
    
    // Send EOI
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}