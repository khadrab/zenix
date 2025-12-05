#ifndef MONITOR_H
#define MONITOR_H

#include <types.h>

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

extern uint16_t* vga_buffer;
extern uint8_t vga_color;
extern uint32_t cursor_x;
extern uint32_t cursor_y;

void clear_screen();
void scroll_screen();
void print_char(char c);
void print_string(const char* str);
void print_hex(uint32_t n);
void print_dec(uint32_t n);

extern void gdt_init();
extern void idt_init();
extern void isr_install();

#endif