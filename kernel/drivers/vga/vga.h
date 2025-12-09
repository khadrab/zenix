// kernel/drivers/vga/vga.h - Fixed VBE structure
#ifndef VGA_H
#define VGA_H

#include "../../../include/types.h"

// VBE Mode Info (must match boot.asm)
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t framebuffer;
} vbe_mode_info_t;

// External VBE info from boot.asm
extern vbe_mode_info_t vbe_mode_info;

// VGA Width/Height (will be set from VBE)
extern uint32_t VGA_WIDTH;
extern uint32_t VGA_HEIGHT;

// Initialize VGA
void vga_init(void);

// Check if VGA is active
int vga_is_active(void);

// Get display info
uint32_t vga_get_width(void);
uint32_t vga_get_height(void);
uint32_t vga_get_bpp(void);

// Color conversion
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);

// Pixel operations
void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color);
void vga_set_pixel_rgb(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
uint32_t vga_get_pixel(uint32_t x, uint32_t y);

// Screen operations
void vga_clear_screen(uint32_t color);
void vga_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vga_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

// Drawing operations
void vga_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
void vga_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color);
void vga_fill_circle(uint32_t cx, uint32_t cy, uint32_t r, uint32_t color);

// Text operations
void vga_draw_char(uint32_t x, uint32_t y, char c, uint32_t color);
void vga_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);

// Buffer operations
void vga_swap_buffers(void);

#endif // VGA_H