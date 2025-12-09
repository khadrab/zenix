// kernel/drivers/vga/vga.c - Full VESA VBE Driver
#include "vga.h"
#include "vga_font.h"
#include "../../mm/heap.h"

extern vbe_mode_info_t vbe_mode_info;

uint32_t VGA_WIDTH = 320;
uint32_t VGA_HEIGHT = 200;

static uint8_t* framebuffer = 0;
static uint8_t* backbuffer = 0;
static int vga_active = 0;
static uint32_t bytes_per_pixel = 1;

void vga_init(void) {
    VGA_WIDTH = vbe_mode_info.width;
    VGA_HEIGHT = vbe_mode_info.height;
    bytes_per_pixel = vbe_mode_info.bpp / 8;
    
    framebuffer = (uint8_t*)vbe_mode_info.framebuffer;
    
    // Allocate backbuffer
    uint32_t size = vbe_mode_info.pitch * vbe_mode_info.height;
    backbuffer = (uint8_t*)kmalloc(size);
    
    if (!backbuffer) {
        backbuffer = framebuffer;
    }
    
    vga_active = 1;
}

int vga_is_active(void) { return vga_active; }
uint32_t vga_get_width(void) { return VGA_WIDTH; }
uint32_t vga_get_height(void) { return VGA_HEIGHT; }
uint32_t vga_get_bpp(void) { return vbe_mode_info.bpp; }

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    if (vbe_mode_info.bpp == 32) {
        return (r << 16) | (g << 8) | b;
    } else if (vbe_mode_info.bpp == 24) {
        return (r << 16) | (g << 8) | b;
    } else if (vbe_mode_info.bpp == 16) {
        return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    } else {
        // 8-bit palette
        return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
    }
}

void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    uint32_t offset = y * vbe_mode_info.pitch + x * bytes_per_pixel;
    
    if (vbe_mode_info.bpp == 32) {
        *(uint32_t*)(backbuffer + offset) = color;
    } else if (vbe_mode_info.bpp == 24) {
        backbuffer[offset] = color & 0xFF;
        backbuffer[offset + 1] = (color >> 8) & 0xFF;
        backbuffer[offset + 2] = (color >> 16) & 0xFF;
    } else if (vbe_mode_info.bpp == 16) {
        *(uint16_t*)(backbuffer + offset) = (uint16_t)color;
    } else {
        backbuffer[offset] = (uint8_t)color;
    }
}

void vga_set_pixel_rgb(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
    vga_set_pixel(x, y, rgb(r, g, b));
}

uint32_t vga_get_pixel(uint32_t x, uint32_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return 0;
    
    uint32_t offset = y * vbe_mode_info.pitch + x * bytes_per_pixel;
    
    if (vbe_mode_info.bpp == 32) {
        return *(uint32_t*)(backbuffer + offset);
    } else if (vbe_mode_info.bpp == 24) {
        return backbuffer[offset] | (backbuffer[offset + 1] << 8) | (backbuffer[offset + 2] << 16);
    } else if (vbe_mode_info.bpp == 16) {
        return *(uint16_t*)(backbuffer + offset);
    } else {
        return backbuffer[offset];
    }
}

void vga_clear_screen(uint32_t color) {
    vga_fill_rect(0, 0, VGA_WIDTH, VGA_HEIGHT, color);
}

void vga_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t dy = 0; dy < h; dy++) {
        for (uint32_t dx = 0; dx < w; dx++) {
            vga_set_pixel(x + dx, y + dy, color);
        }
    }
}

void vga_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t i = 0; i < w; i++) {
        vga_set_pixel(x + i, y, color);
        vga_set_pixel(x + i, y + h - 1, color);
    }
    for (uint32_t i = 0; i < h; i++) {
        vga_set_pixel(x, y + i, color);
        vga_set_pixel(x + w - 1, y + i, color);
    }
}

void vga_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int dx = (int)x2 - (int)x1;
    int dy = (int)y2 - (int)y1;
    int steps = dx > dy ? (dx > 0 ? dx : -dx) : (dy > 0 ? dy : -dy);
    
    float xinc = dx / (float)steps;
    float yinc = dy / (float)steps;
    float x = x1, y = y1;
    
    for (int i = 0; i <= steps; i++) {
        vga_set_pixel((uint32_t)x, (uint32_t)y, color);
        x += xinc;
        y += yinc;
    }
}

void vga_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color) {
    int x = radius, y = 0, err = 0;
    while (x >= y) {
        vga_set_pixel(cx + x, cy + y, color);
        vga_set_pixel(cx + y, cy + x, color);
        vga_set_pixel(cx - y, cy + x, color);
        vga_set_pixel(cx - x, cy + y, color);
        vga_set_pixel(cx - x, cy - y, color);
        vga_set_pixel(cx - y, cy - x, color);
        vga_set_pixel(cx + y, cy - x, color);
        vga_set_pixel(cx + x, cy - y, color);
        y++;
        err += 1 + 2*y;
        if (2*(err-x) + 1 > 0) {
            x--;
            err += 1 - 2*x;
        }
    }
}

void vga_fill_circle(uint32_t cx, uint32_t cy, uint32_t r, uint32_t color) {
    for (uint32_t y = 0; y <= r; y++) {
        for (uint32_t x = 0; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                vga_set_pixel(cx + x, cy + y, color);
                vga_set_pixel(cx - x, cy + y, color);
                vga_set_pixel(cx + x, cy - y, color);
                vga_set_pixel(cx - x, cy - y, color);
            }
        }
    }
}

void vga_draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) return;
    
    // Scale for higher resolutions
    uint32_t scale = (VGA_WIDTH >= 800) ? 2 : 1;
    
    for (uint32_t row = 0; row < 8; row++) {
        uint8_t bits = font_8x8[uc][row];
        for (uint32_t col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                for (uint32_t sy = 0; sy < scale; sy++) {
                    for (uint32_t sx = 0; sx < scale; sx++) {
                        vga_set_pixel(x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

void vga_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color) {
    uint32_t char_w = (VGA_WIDTH >= 800) ? 16 : 8;
    uint32_t offset = 0;
    while (*str) {
        vga_draw_char(x + offset, y, *str, color);
        str++;
        offset += char_w;
    }
}

void vga_swap_buffers(void) {
    if (backbuffer == framebuffer) return;
    
    uint32_t size = vbe_mode_info.pitch * vbe_mode_info.height;
    for (uint32_t i = 0; i < size; i++) {
        framebuffer[i] = backbuffer[i];
    }
}