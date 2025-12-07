#include "vga.h"
#include <io.h>

static uint8_t* vga_buffer = (uint8_t*)VGA_MEMORY;

// Simple 8x8 font (only basic ASCII for now)
static uint8_t font_8x8[128][8] = {
    // Space (32)
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    [33] = {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},
    // A (65)
    [65] = {0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    // B (66)
    [66] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    // C (67)
    [67] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    // D (68)
    [68] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    // E (69)
    [69] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00},
    // F (70)
    [70] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00},
    // G (71)
    [71] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},
    // H (72)
    [72] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    // I (73)
    [73] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    // Add more characters as needed...
    // For demo purposes, we'll just use these
};

void vga_init(void) {
    // In protected mode, we can't use BIOS to set video mode
    // QEMU starts in text mode by default
    // For now, we'll just write to VGA memory at 0xA0000
    // Note: This assumes mode 13h is already set or QEMU handles it
    
    // Clear the screen
    vga_clear_screen(COLOR_BLACK);
}

void vga_set_pixel(uint32_t x, uint32_t y, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    vga_buffer[y * VGA_WIDTH + x] = color;
}

uint8_t vga_get_pixel(uint32_t x, uint32_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return 0;
    return vga_buffer[y * VGA_WIDTH + x];
}

void vga_clear_screen(uint8_t color) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = color;
    }
}

void vga_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color) {
    // Top and bottom lines
    for (uint32_t i = 0; i < width; i++) {
        vga_set_pixel(x + i, y, color);
        vga_set_pixel(x + i, y + height - 1, color);
    }
    
    // Left and right lines
    for (uint32_t i = 0; i < height; i++) {
        vga_set_pixel(x, y + i, color);
        vga_set_pixel(x + width - 1, y + i, color);
    }
}

void vga_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color) {
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            vga_set_pixel(x + i, y + j, color);
        }
    }
}

void vga_draw_char(uint32_t x, uint32_t y, char c, uint8_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) return;
    
    for (uint32_t row = 0; row < 8; row++) {
        uint8_t bits = font_8x8[uc][row];
        for (uint32_t col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                vga_set_pixel(x + col, y + row, color);
            }
        }
    }
}

void vga_draw_string(uint32_t x, uint32_t y, const char* str, uint8_t color) {
    uint32_t offset = 0;
    while (*str) {
        vga_draw_char(x + offset, y, *str, color);
        str++;
        offset += 8;
    }
}

void vga_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (dx > dy ? dx : dy);
    if (steps < 0) steps = -steps;
    
    float x_inc = dx / (float)steps;
    float y_inc = dy / (float)steps;
    
    float x = x1;
    float y = y1;
    
    for (int i = 0; i <= steps; i++) {
        vga_set_pixel((uint32_t)x, (uint32_t)y, color);
        x += x_inc;
        y += y_inc;
    }
}