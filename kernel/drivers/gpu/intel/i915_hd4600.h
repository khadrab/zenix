// kernel/drivers/gpu/intel/i915_hd4600.h
// Intel HD Graphics 4600 (Haswell GT2) Native Driver
#ifndef I915_HD4600_H
#define I915_HD4600_H

#include "../../../../include/types.h"
#include "../gpu_detect.h"

// Device ID
#define INTEL_HD4600_DEVICE_ID  0x0412

// Register offsets - Haswell Display
#define PIPEA_CONF              0x70008
#define PIPEA_SRC               0x6001C
#define DSPACNTR                0x70180  // Primary plane A control
#define DSPASURF                0x7019C  // Primary plane A surface address
#define DSPASTRIDE              0x70188  // Primary plane A stride
#define PIPEACONF_ENABLE        (1 << 31)
#define PIPEACONF_STATE         (1 << 30)

// Plane control bits
#define PLANE_CTL_ENABLE        (1 << 31)
#define PLANE_CTL_FORMAT_SHIFT  24
#define PLANE_CTL_FORMAT_XRGB8888 (4 << PLANE_CTL_FORMAT_SHIFT)

// Power management
#define PWR_WELL_CTL            0x45400
#define PWR_WELL_STATE          0x45404
#define HSW_PWR_WELL_ENABLE     (1 << 31)

// Display port control (for DDI)
#define DDI_BUF_CTL_A           0x64000
#define DDI_BUF_CTL_ENABLE      (1 << 31)

// PCH registers
#define PCH_PP_STATUS           0xC7200
#define PCH_PP_CONTROL          0xC7204

// GTT (Graphics Translation Table)
#define GTT_OFFSET              0x800000

// Device context
typedef struct {
    // PCI location
    uint8_t bus, slot, func;
    uint16_t device_id;
    
    // Memory regions (physical addresses)
    uint32_t mmio_base;      // BAR0 - MMIO registers
    uint32_t gtt_base;       // BAR2 - GTT/Stolen memory
    uint32_t mmio_size;
    uint32_t gtt_size;
    
    // Framebuffer (in stolen memory)
    uint32_t fb_phys;        // Physical address
    uint32_t fb_virt;        // Virtual/identity mapped
    uint32_t fb_size;
    
    // Display mode
    uint32_t width;
    uint32_t height;
    uint32_t pitch;          // Bytes per line
    uint8_t  bpp;            // Bits per pixel
    
    int initialized;
} i915_device_t;

// API Functions
int i915_hd4600_detect(gpu_device_t* gpu);
int i915_hd4600_init(gpu_device_t* gpu);
void i915_hd4600_cleanup(void);

// Drawing functions
void i915_clear_screen(uint32_t color);
void i915_putpixel(uint32_t x, uint32_t y, uint32_t color);
void i915_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

// Info
i915_device_t* i915_get_device(void);
void i915_print_info(void);

#endif // I915_HD4600_H