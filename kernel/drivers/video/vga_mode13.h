// kernel/drivers/video/vga_mode13.h
// VGA Mode 13h (320x200x8) - Works everywhere!
#ifndef VGA_MODE13_H
#define VGA_MODE13_H

#include "../../include/types.h"

#define VGA_WIDTH  320
#define VGA_HEIGHT 200
#define VGA_MEMORY 0xA0000

// Initialize Mode 13h
void vga_mode13_init(void);

// Drawing
void vga_mode13_putpixel(uint16_t x, uint16_t y, uint8_t color);
void vga_mode13_clear(uint8_t color);
void vga_mode13_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);

// Palette
void vga_mode13_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

#endif