// kernel/drivers/gpu/intel/intel_hd4600.h
// Intel HD Graphics 4600 (Haswell GT2) Driver
// Based on Intel Haswell PRM Documentation
#ifndef INTEL_HD4600_H
#define INTEL_HD4600_H

#include "../gpu_detect.h"

// Device IDs for Intel HD 4600
#define INTEL_HD4600_DEVICE_ID_1   0x0412  // Desktop GT2
#define INTEL_HD4600_DEVICE_ID_2   0x0416  // Mobile GT2

// MMIO Register Offsets (Haswell Specific)
// Display Engine Registers
#define HSW_PIPEA_CONF             0x70008
#define HSW_PIPEA_SRC              0x6001C
#define HSW_PIPEA_DATA_M1          0x60030
#define HSW_PIPEA_DATA_N1          0x60034
#define HSW_PIPEA_LINK_M1          0x60040
#define HSW_PIPEA_LINK_N1          0x60044

// Display Plane Registers (Primary Plane A)
#define HSW_DSPACNTR               0x70180
#define HSW_DSPASURF               0x7019C
#define HSW_DSPASTRIDE             0x70188
#define HSW_DSPASIZE               0x70190
#define HSW_DSPAPOS                0x7018C

// Power Well Control
#define HSW_PWR_WELL_CTL           0x45400
#define HSW_PWR_WELL_STATE         0x45404

// Display PLL (DPLL)
#define HSW_DPLL_CTRL1             0x6C058
#define HSW_DPLL_CTRL2             0x6C05C
#define HSW_LCPLL_CTL              0x130040

// DDI (Digital Display Interface) Registers
#define HSW_DDI_BUF_CTL_A          0x64000
#define HSW_DDI_BUF_TRANS_A        0x64E00

// Graphics Translation Table (GTT) - Stolen memory
#define HSW_GTT_BASE               0x800000
#define HSW_STOLEN_BASE            0x5C  // PCI Config offset

// Pixel Formats
#define PLANE_CTL_FORMAT_XRGB_8888 (4 << 24)
#define PLANE_CTL_ENABLE           (1 << 31)
#define PLANE_CTL_TILED_LINEAR     (0 << 10)

// Display mode structure
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    uint32_t pixel_clock;  // in kHz
    
    // Timing parameters
    uint32_t htotal;
    uint32_t hblank_start;
    uint32_t hblank_end;
    uint32_t hsync_start;
    uint32_t hsync_end;
    
    uint32_t vtotal;
    uint32_t vblank_start;
    uint32_t vblank_end;
    uint32_t vsync_start;
    uint32_t vsync_end;
} display_mode_t;

// Driver context
typedef struct {
    // PCI info
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t device_id;
    
    // Memory mappings
    uint32_t mmio_base;      // BAR 0
    uint32_t mmio_size;
    uint32_t gtt_base;       // Graphics Translation Table
    uint32_t stolen_base;    // Stolen memory base
    uint32_t stolen_size;    // Stolen memory size
    
    // Framebuffer
    uint32_t fb_phys;        // Physical address
    uint32_t* fb_virt;       // Virtual/mapped address
    uint32_t fb_size;
    uint32_t fb_stride;      // bytes per row
    
    // Current mode
    display_mode_t mode;
    int initialized;
} intel_hd4600_t;

// Main driver functions
int intel_hd4600_detect(gpu_device_t* gpu);
int intel_hd4600_init(gpu_device_t* gpu);
int intel_hd4600_set_mode(uint32_t width, uint32_t height);
void intel_hd4600_clear_screen(uint32_t color);
void intel_hd4600_putpixel(uint32_t x, uint32_t y, uint32_t color);
intel_hd4600_t* intel_hd4600_get_context(void);

#endif // INTEL_HD4600_H