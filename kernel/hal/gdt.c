#include "gdt.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

struct gdt_entry gdt_entries[6];
struct gdt_ptr gdt_pointer;
struct tss_entry tss;

extern void gdt_flush(uint32_t);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
    
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss);
    
    // Clear TSS
   uint8_t* tss_ptr = (uint8_t*)&tss;
    for (uint32_t i = 0; i < sizeof(tss); i++) {
        tss_ptr[i] = 0;
    }
    
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    
    // Set segment registers to kernel data segment
    tss.cs = 0x0b;  // Kernel code (0x08) | RPL 3
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;  // Kernel data (0x10) | RPL 3
    
    // Add TSS descriptor (0xE9 = Present, DPL=0, Type=Available TSS)
    gdt_set_gate(num, base, limit, 0xE9, 0x00);
}

void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gdt_pointer.base = (uint32_t)&gdt_entries;
    
    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Kernel code segment (0x08)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    
    // Kernel data segment (0x10)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    // User code segment (0x18)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    
    // User data segment (0x20)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    
    // TSS segment (0x28) - will be set up properly
    write_tss(5, 0x10, 0x0);
    
    // Flush GDT
    gdt_flush((uint32_t)&gdt_pointer);
    
    // Load TSS (selector 0x28 = index 5 * 8)
    __asm__ volatile("mov $0x28, %ax; ltr %ax");
}

void set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}