#ifndef VGA_H
#define VGA_H

#include "../../../include/types.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_MEMORY 0xA0000

// Color palette (VGA 256 colors)
#define COLOR_BLACK       0
#define COLOR_BLUE        1
#define COLOR_GREEN       2
#define COLOR_CYAN        3
#define COLOR_RED         4
#define COLOR_MAGENTA     5
#define COLOR_BROWN       6
#define COLOR_LIGHT_GRAY  7
#define COLOR_DARK_GRAY   8
#define COLOR_LIGHT_BLUE  9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN  11
#define COLOR_LIGHT_RED   12
#define COLOR_PINK        13
#define COLOR_YELLOW      14
#define COLOR_WHITE       15

void vga_init(void);
void vga_set_pixel(uint32_t x, uint32_t y, uint8_t color);
uint8_t vga_get_pixel(uint32_t x, uint32_t y);
void vga_clear_screen(uint8_t color);
void vga_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color);
void vga_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color);
void vga_draw_char(uint32_t x, uint32_t y, char c, uint8_t color);
void vga_draw_string(uint32_t x, uint32_t y, const char* str, uint8_t color);
void vga_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t color);

#endif