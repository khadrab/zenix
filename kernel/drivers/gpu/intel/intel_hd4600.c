
// kernel/drivers/gpu/intel/intel_hd4600.c
#include "intel_hd4600.h"
#include "../../../core/monitor.h"

static intel_hd4600_t ctx = {0};

// MMIO helpers
static inline uint32_t mmio_read(uint32_t offset) {
    return *(volatile uint32_t*)(ctx.mmio_base + offset);
}

static inline void mmio_write(uint32_t offset, uint32_t value) {
    *(volatile uint32_t*)(ctx.mmio_base + offset) = value;
}

static inline void mmio_set_bits(uint32_t offset, uint32_t bits) {
    uint32_t val = mmio_read(offset);
    mmio_write(offset, val | bits);
}

static inline void mmio_clear_bits(uint32_t offset, uint32_t bits) {
    uint32_t val = mmio_read(offset);
    mmio_write(offset, val & ~bits);
}

// Wait for register bit to be set/cleared
static int mmio_wait_for(uint32_t offset, uint32_t mask, uint32_t value, int timeout_ms) {
    for (int i = 0; i < timeout_ms * 100; i++) {
        if ((mmio_read(offset) & mask) == value) {
            return 0;
        }
        // Small delay
        for (volatile int j = 0; j < 1000; j++);
    }
    return -1; // Timeout
}

// Detect if this is Intel HD 4600
int intel_hd4600_detect(gpu_device_t* gpu) {
    if (gpu->vendor_id != GPU_VENDOR_INTEL) {
        return 0;
    }
    
    // Check device ID
    if (gpu->device_id == INTEL_HD4600_DEVICE_ID_1 ||
        gpu->device_id == INTEL_HD4600_DEVICE_ID_2) {
        return 1;
    }
    
    return 0;
}

// Read PCI config (simplified - needs proper implementation)
static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) | 
                       ((uint32_t)bus << 16) | 
                       ((uint32_t)slot << 11) | 
                       ((uint32_t)func << 8) | 
                       (offset & 0xFC);
    
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(0xCFC));
    return ret;
}

// Initialize power wells
static int intel_hd4600_init_power(void) {
    print_string("    Enabling power wells...\n");
    
    // Enable Power Well 1 (always-on well)
    mmio_set_bits(HSW_PWR_WELL_CTL, (1 << 31));
    
    // Wait for power well to be enabled
    if (mmio_wait_for(HSW_PWR_WELL_STATE, (1 << 30), (1 << 30), 100) != 0) {
        print_string("    [WARN] Power well timeout\n");
    }
    
    return 0;
}

// Allocate framebuffer in stolen memory
static int intel_hd4600_alloc_framebuffer(uint32_t width, uint32_t height) {
    // Calculate framebuffer size (32bpp)
    ctx.fb_stride = width * 4;
    ctx.fb_size = ctx.fb_stride * height;
    
    // Read stolen memory base from PCI config
    uint32_t stolen_reg = pci_read_config(ctx.bus, ctx.slot, ctx.func, HSW_STOLEN_BASE);
    ctx.stolen_base = stolen_reg & 0xFFF00000;  // Bits 31:20
    
    // Use beginning of stolen memory for framebuffer
    ctx.fb_phys = ctx.stolen_base;
    ctx.fb_virt = (uint32_t*)ctx.fb_phys;  // Direct mapping for now
    
    print_string("    Framebuffer @ 0x");
    print_hex(ctx.fb_phys);
    print_string(" (");
    print_dec(ctx.fb_size / 1024);
    print_string(" KB)\n");
    
    return 0;
}

// Set up display plane
static void intel_hd4600_setup_plane(void) {
    // Disable plane first
    mmio_write(HSW_DSPACNTR, 0);
    mmio_write(HSW_DSPASURF, 0);
    
    // Configure plane control
    uint32_t plane_ctl = PLANE_CTL_ENABLE | 
                         PLANE_CTL_FORMAT_XRGB_8888 |
                         PLANE_CTL_TILED_LINEAR;
    
    // Set stride (must be 64-byte aligned)
    mmio_write(HSW_DSPASTRIDE, ctx.fb_stride);
    
    // Set position (0,0)
    mmio_write(HSW_DSPAPOS, 0);
    
    // Set size
    uint32_t size = ((ctx.mode.height - 1) << 16) | (ctx.mode.width - 1);
    mmio_write(HSW_DSPASIZE, size);
    
    // Enable plane with config
    mmio_write(HSW_DSPACNTR, plane_ctl);
    
    // Set surface address (this commits changes)
    mmio_write(HSW_DSPASURF, ctx.fb_phys);
}

// Configure display pipe
static int intel_hd4600_setup_pipe(void) {
    print_string("    Configuring display pipe...\n");
    
    // Disable pipe first
    mmio_clear_bits(HSW_PIPEA_CONF, (1 << 31));
    
    // Wait for pipe to disable
    if (mmio_wait_for(HSW_PIPEA_CONF, (1 << 30), 0, 100) != 0) {
        print_string("    [WARN] Pipe disable timeout\n");
    }
    
    // Set pipe source size
    uint32_t pipe_src = ((ctx.mode.width - 1) << 16) | (ctx.mode.height - 1);
    mmio_write(HSW_PIPEA_SRC, pipe_src);
    
    // Enable pipe
    mmio_set_bits(HSW_PIPEA_CONF, (1 << 31));
    
    // Wait for pipe to enable
    if (mmio_wait_for(HSW_PIPEA_CONF, (1 << 30), (1 << 30), 100) != 0) {
        print_string("    [WARN] Pipe enable timeout\n");
        return -1;
    }
    
    return 0;
}

// Main initialization
int intel_hd4600_init(gpu_device_t* gpu) {
    print_string("  Intel HD 4600 Driver Init\n");
    
    // Store PCI info
    ctx.bus = gpu->bus;
    ctx.slot = gpu->slot;
    ctx.func = gpu->func;
    ctx.device_id = gpu->device_id;
    
    // Map MMIO (BAR0)
    ctx.mmio_base = gpu->bar0 & 0xFFFFFFF0;
    ctx.mmio_size = 2 * 1024 * 1024;  // 2MB typical
    
    if (ctx.mmio_base == 0) {
        print_string("  [ERROR] Invalid MMIO base\n");
        return -1;
    }
    
    print_string("    MMIO @ 0x");
    print_hex(ctx.mmio_base);
    print_string("\n");
    
    // Initialize power
    intel_hd4600_init_power();
    
    // Set default mode (1024x768 for safety)
    ctx.mode.width = 1024;
    ctx.mode.height = 768;
    ctx.mode.refresh_rate = 60;
    
    // Allocate framebuffer
    intel_hd4600_alloc_framebuffer(ctx.mode.width, ctx.mode.height);
    
    // Setup display plane
    intel_hd4600_setup_plane();
    
    // Setup pipe
    intel_hd4600_setup_pipe();
    
    ctx.initialized = 1;
    
    print_string("  [OK] HD 4600 initialized\n");
    print_string("  Mode: ");
    print_dec(ctx.mode.width);
    print_string("x");
    print_dec(ctx.mode.height);
    print_string("@");
    print_dec(ctx.mode.refresh_rate);
    print_string("Hz\n");
    
    return 0;
}

// Set display mode
int intel_hd4600_set_mode(uint32_t width, uint32_t height) {
    if (!ctx.initialized) return -1;
    
    ctx.mode.width = width;
    ctx.mode.height = height;
    
    // Reallocate framebuffer
    intel_hd4600_alloc_framebuffer(width, height);
    
    // Reconfigure plane and pipe
    intel_hd4600_setup_plane();
    intel_hd4600_setup_pipe();
    
    return 0;
}

// Clear screen to color
void intel_hd4600_clear_screen(uint32_t color) {
    if (!ctx.initialized || !ctx.fb_virt) return;
    
    uint32_t pixels = (ctx.fb_stride / 4) * ctx.mode.height;
    for (uint32_t i = 0; i < pixels; i++) {
        ctx.fb_virt[i] = color;
    }
}

// Draw pixel
void intel_hd4600_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!ctx.initialized || !ctx.fb_virt) return;
    if (x >= ctx.mode.width || y >= ctx.mode.height) return;
    
    uint32_t offset = y * (ctx.fb_stride / 4) + x;
    ctx.fb_virt[offset] = color;
}

// Get context
intel_hd4600_t* intel_hd4600_get_context(void) {
    return ctx.initialized ? &ctx : NULL;
}