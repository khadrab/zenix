// kernel/drivers/video/gop_fb.h
// UEFI GOP (Graphics Output Protocol) Framebuffer
// Works in both QEMU and Real Hardware!
#ifndef GOP_FB_H
#define GOP_FB_H

#include "../../../include/types.h"
#include "../../../include/multiboot.h"

typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t  bpp;
    int initialized;
} gop_fb_t;

// Initialize from multiboot framebuffer info
int gop_fb_init(multiboot_info_t* mbi);

// Drawing functions
void gop_clear(uint32_t color);
void gop_putpixel(uint32_t x, uint32_t y, uint32_t color);
void gop_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void gop_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);

// Info
gop_fb_t* gop_get_info(void);
void gop_print_info(void);

#endif // GOP_FB_H