#include "vga_mode13.h"

static uint8_t* vga_mem = (uint8_t*)VGA_MEMORY;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void vga_mode13_init(void) {
    // Set mode 13h via BIOS interrupt (can't do in protected mode)
    // Instead, write to VGA registers
    
    // This is a simplified version - full init needs many registers
    outb(0x3C8, 0);  // Reset palette index
    
    // Set some basic colors
    vga_mode13_set_palette(0, 0, 0, 0);        // Black
    vga_mode13_set_palette(1, 63, 0, 0);       // Red
    vga_mode13_set_palette(2, 0, 63, 0);       // Green  
    vga_mode13_set_palette(3, 0, 0, 63);       // Blue
    vga_mode13_set_palette(4, 63, 63, 0);      // Yellow
    vga_mode13_set_palette(15, 63, 63, 63);    // White
}

void vga_mode13_putpixel(uint16_t x, uint16_t y, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    vga_mem[y * VGA_WIDTH + x] = color;
}

void vga_mode13_clear(uint8_t color) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_mem[i] = color;
    }
}

void vga_mode13_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    for (uint16_t dy = 0; dy < h; dy++) {
        for (uint16_t dx = 0; dx < w; dx++) {
            vga_mode13_putpixel(x + dx, y + dy, color);
        }
    }
}

void vga_mode13_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x3C8, index);
    outb(0x3C9, r);
    outb(0x3C9, g);
    outb(0x3C9, b);
}