#include "monitor.h"

uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
uint8_t vga_color = 0x0F;
uint32_t cursor_x = 0;
uint32_t cursor_y = 0;

void scroll_screen() {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }
    
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        vga_buffer[i] = (vga_color << 8) | ' ';
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (vga_color << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            uint32_t index = cursor_y * VGA_WIDTH + cursor_x;
            vga_buffer[index] = (vga_color << 8) | ' ';
        }
    } else {
        uint32_t index = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[index] = (vga_color << 8) | c;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str);
        str++;
    }
}

void print_hex(uint32_t n) {
    const char hex[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        print_char(hex[(n >> i) & 0xF]);
    }
}

void print_dec(uint32_t n) {
    if (n == 0) {
        print_char('0');
        return;
    }
    
    char buffer[12];
    int i = 0;
    
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    while (i > 0) {
        print_char(buffer[--i]);
    }
}