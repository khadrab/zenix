#include "../../include/io.h"
#include "../../include/types.h"

extern void irq_clear_mask(uint8_t irq_line);
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t a1, a2;
    
    // احفظ الـ mask الحالي
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
    
    // ابدأ التهيئة
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // اضبط الـ offsets
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();
    
    // اضبط الترابط بين Master و Slave
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();
    
    // اضبط 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // فعّل كل الـ IRQs (mask = 0)
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void pic_send_eoi(uint8_t irq) {
    if(irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}

void irq_set_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;
    
    if(irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) | (1 << irq_line);
    outb(port, value);
}

void irq_clear_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;
    
    if(irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) & ~(1 << irq_line);
    outb(port, value);
}