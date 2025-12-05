#include "idt.h"

struct idt_entry idt_entries[256];
struct idt_ptr   idt_pointer;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector  = selector;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags;
}

void idt_init() {
    idt_pointer.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_pointer.base  = (uint32_t)&idt_entries;
    
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    idt_load((uint32_t)&idt_pointer);
}