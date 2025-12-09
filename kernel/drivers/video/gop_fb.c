// kernel/drivers/video/gop_fb.c
#include "gop_fb.h"
#include "../../core/monitor.h"

static gop_fb_t fb = {0};

int gop_fb_init(multiboot_info_t* mbi) {
    print_string("  Checking for framebuffer...\n");
    
    // Check if framebuffer info is present
    if (!(mbi->flags & MULTIBOOT_FLAG_FRAMEBUFFER)) {
        print_string("  [INFO] No framebuffer flag in multiboot\n");
        return -1;
    }
    
    print_string("  Framebuffer flag detected\n");
    print_string("  Type: ");
    print_dec(mbi->framebuffer_type);
    print_string("\n");
    
    // Check framebuffer type (must be RGB)
    if (mbi->framebuffer_type != 1) {  // 1 = RGB, 0 = indexed, 2 = text
        print_string("  [INFO] Framebuffer is not RGB mode (type != 1)\n");
        return -1;
    }
    
    // Store framebuffer info
    fb.framebuffer = (uint32_t*)(uint32_t)mbi->framebuffer_addr;
    fb.width = mbi->framebuffer_width;
    fb.height = mbi->framebuffer_height;
    fb.pitch = mbi->framebuffer_pitch;
    fb.bpp = mbi->framebuffer_bpp;
    
    print_string("  FB Address: 0x");
    print_hex((uint32_t)fb.framebuffer);
    print_string("\n");
    print_string("  Resolution: ");
    print_dec(fb.width);
    print_string("x");
    print_dec(fb.height);
    print_string("x");
    print_dec(fb.bpp);
    print_string("\n");
    
    // Validate
    if (fb.framebuffer == NULL || fb.width == 0 || fb.height == 0) {
        print_string("  [ERROR] Invalid framebuffer parameters\n");
        return -1;
    }
    
    // Check if address looks valid (not obviously wrong)
    if ((uint32_t)fb.framebuffer < 0x100000) {
        print_string("  [WARN] Framebuffer address seems too low\n");
    }
    
    fb.initialized = 1;
    
    print_string("  [OK] GOP Framebuffer initialized\n");
    
    return 0;
}

void gop_clear(uint32_t color) {
    if (!fb.initialized) return;
    
    uint32_t pixels = (fb.pitch / 4) * fb.height;
    for (uint32_t i = 0; i < pixels; i++) {
        fb.framebuffer[i] = color;
    }
}

void gop_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb.initialized) return;
    if (x >= fb.width || y >= fb.height) return;
    
    fb.framebuffer[y * (fb.pitch / 4) + x] = color;
}

void gop_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t dy = 0; dy < h; dy++) {
        for (uint32_t dx = 0; dx < w; dx++) {
            gop_putpixel(x + dx, y + dy, color);
        }
    }
}

void gop_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (dx > dy) ? dx : dy;
    
    if (steps < 0) steps = -steps;
    if (steps == 0) steps = 1;
    
    float x_inc = (float)dx / steps;
    float y_inc = (float)dy / steps;
    
    float x = x1;
    float y = y1;
    
    for (int i = 0; i <= steps; i++) {
        gop_putpixel((uint32_t)x, (uint32_t)y, color);
        x += x_inc;
        y += y_inc;
    }
}

gop_fb_t* gop_get_info(void) {
    return fb.initialized ? &fb : NULL;
}

void gop_print_info(void) {
    if (!fb.initialized) {
        print_string("  GOP framebuffer not initialized\n");
        return;
    }
    
    print_string("  Active: ");
    print_dec(fb.width);
    print_string("x");
    print_dec(fb.height);
    print_string("x");
    print_dec(fb.bpp);
    print_string("\n");
}