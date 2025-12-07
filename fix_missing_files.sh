#!/bin/bash
echo "Fixing missing files for Zenix..."

# pic.h
cat > kernel/hal/pic.h << 'EOF'
#ifndef PIC_H
#define PIC_H
#include "../../include/types.h"
void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t mask1, uint8_t mask2);
void pic_disable(void);
uint16_t pic_get_isr(uint8_t irq);
#endif
EOF
echo "✓ Created kernel/hal/pic.h"

# pic.c
cat > kernel/hal/pic.c << 'EOF'
#include "pic.h"
#include "../../include/io.h"
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20
#define PIC_READ_ISR 0x0B
#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10
#define ICW4_8086 0x01

void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_set_mask(uint8_t mask1, uint8_t mask2) {
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

uint16_t pic_get_isr(uint8_t irq) {
    if (irq < 8) {
        outb(PIC1_COMMAND, PIC_READ_ISR);
        return (inb(PIC1_COMMAND) & (1 << irq)) != 0;
    }
    outb(PIC2_COMMAND, PIC_READ_ISR);
    return (inb(PIC2_COMMAND) & (1 << (irq - 8))) != 0;
}
EOF
echo "✓ Created kernel/hal/pic.c"

echo ""
echo "All files created! Now run:"
echo "  make clean && make all && make run-initrd"
