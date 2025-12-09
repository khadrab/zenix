// kernel/drivers/gpu/intel/i915_hd4600.c
#include "i915_hd4600.h"
#include "../../../core/monitor.h"

static i915_device_t dev = {0};

// I/O Port Access Helpers
static inline void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// PCI Config Space Access
static inline uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = 0x80000000 | ((uint32_t)bus << 16) | 
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) | 
                       (offset & 0xFC);
    
    outl(0xCF8, address);
    return inl(0xCFC);
}

static inline void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = 0x80000000 | ((uint32_t)bus << 16) | 
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) | 
                       (offset & 0xFC);
    
    outl(0xCF8, address);
    outl(0xCFC, value);
}

// MMIO Access (identity mapped for simplicity)
static inline uint32_t mmio_read32(uint32_t reg) {
    return *(volatile uint32_t*)(dev.mmio_base + reg);
}

static inline void mmio_write32(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(dev.mmio_base + reg) = value;
}

// Wait for condition with timeout
static int wait_for(uint32_t reg, uint32_t mask, uint32_t value, int timeout_ms) {
    for (int i = 0; i < timeout_ms * 1000; i++) {
        if ((mmio_read32(reg) & mask) == value) {
            return 0;
        }
        // Microsecond delay
        for (volatile int j = 0; j < 10; j++);
    }
    return -1;
}

// Check if device is HD 4600
int i915_hd4600_detect(gpu_device_t* gpu) {
    if (gpu->vendor_id != GPU_VENDOR_INTEL) {
        return 0;
    }
    
    // Support multiple Haswell GT2 device IDs
    switch (gpu->device_id) {
        case 0x0412:  // HD 4600 Desktop
        case 0x0416:  // HD 4600 Mobile
        case 0x041E:  // HD 4400
        case 0x0A16:  // HD 4400 Mobile
        case 0x0A1E:  // HD 4200
            return 1;
        default:
            return 0;
    }
}

// Enable bus mastering and memory access
static void enable_pci_resources(void) {
    uint32_t cmd = pci_config_read(dev.bus, dev.slot, dev.func, 0x04);
    cmd |= 0x07;  // Enable: Bus Master, Memory Space, I/O Space
    pci_config_write(dev.bus, dev.slot, dev.func, 0x04, cmd);
}

// Initialize power wells
static int init_power(void) {
    print_string("    Enabling display power...\n");
    
    // Enable power well
    uint32_t val = mmio_read32(PWR_WELL_CTL);
    val |= HSW_PWR_WELL_ENABLE;
    mmio_write32(PWR_WELL_CTL, val);
    
    // Wait for power well to stabilize
    if (wait_for(PWR_WELL_STATE, (1 << 30), (1 << 30), 200) != 0) {
        print_string("    [WARN] Power well timeout\n");
        return -1;
    }
    
    print_string("    Power well active\n");
    return 0;
}

// Allocate framebuffer from stolen memory
static int alloc_framebuffer(uint32_t width, uint32_t height, uint8_t bpp) {
    dev.width = width;
    dev.height = height;
    dev.bpp = bpp;
    dev.pitch = (width * (bpp / 8) + 63) & ~63;  // 64-byte aligned
    dev.fb_size = dev.pitch * height;
    
    // Use beginning of stolen memory (GTT/BAR2)
    dev.fb_phys = dev.gtt_base;
    dev.fb_virt = dev.fb_phys;  // Identity mapping
    
    print_string("    Framebuffer: ");
    print_dec(width);
    print_string("x");
    print_dec(height);
    print_string("x");
    print_dec(bpp);
    print_string(" @ 0x");
    print_hex(dev.fb_phys);
    print_string("\n");
    
    print_string("    Size: ");
    print_dec(dev.fb_size / 1024);
    print_string(" KB, Pitch: ");
    print_dec(dev.pitch);
    print_string(" bytes\n");
    
    return 0;
}

// Configure display plane
static void setup_plane(void) {
    // Disable plane first
    mmio_write32(DSPACNTR, 0);
    mmio_write32(DSPASURF, 0);
    
    // Wait for disable
    for (volatile int i = 0; i < 100000; i++);
    
    // Set stride
    mmio_write32(DSPASTRIDE, dev.pitch);
    
    // Configure plane: Enable + XRGB8888 format
    uint32_t plane_ctl = PLANE_CTL_ENABLE | PLANE_CTL_FORMAT_XRGB8888;
    mmio_write32(DSPACNTR, plane_ctl);
    
    // Set surface address (this triggers update)
    mmio_write32(DSPASURF, dev.fb_phys);
    
    print_string("    Display plane configured\n");
}

// Configure pipe
static int setup_pipe(void) {
    print_string("    Configuring display pipe...\n");
    
    // Check if pipe is already enabled
    uint32_t pipe_conf = mmio_read32(PIPEA_CONF);
    if (pipe_conf & PIPEACONF_ENABLE) {
        print_string("    Pipe already enabled, keeping configuration\n");
        
        // Just set source size
        uint32_t src = ((dev.width - 1) << 16) | (dev.height - 1);
        mmio_write32(PIPEA_SRC, src);
        
        return 0;
    }
    
    // If pipe disabled, we can't enable it safely without full modeset
    // This requires DPLL configuration, timing parameters, etc.
    print_string("    [WARN] Pipe disabled - full modesetting needed\n");
    print_string("    Boot with VESA mode or let BIOS initialize display\n");
    
    return -1;
}

// Main initialization
int i915_hd4600_init(gpu_device_t* gpu) {
    print_string("\n  Intel HD 4600 Native Driver\n");
    print_string("  ===========================\n");
    
    // Store PCI info
    dev.bus = gpu->bus;
    dev.slot = gpu->slot;
    dev.func = gpu->func;
    dev.device_id = gpu->device_id;
    
    // Get BARs
    dev.mmio_base = gpu->bar0 & 0xFFFFFFF0;
    dev.gtt_base = gpu->bar2 & 0xFFFFFFF0;
    
    if (dev.mmio_base == 0 || dev.gtt_base == 0) {
        print_string("    [ERROR] Invalid BAR addresses\n");
        return -1;
    }
    
    // Typical sizes (can read from BAR)
    dev.mmio_size = 4 * 1024 * 1024;      // 4MB
    dev.gtt_size = 256 * 1024 * 1024;     // 256MB
    
    print_string("    MMIO: 0x");
    print_hex(dev.mmio_base);
    print_string(" (");
    print_dec(dev.mmio_size / (1024*1024));
    print_string(" MB)\n");
    
    print_string("    GTT:  0x");
    print_hex(dev.gtt_base);
    print_string(" (");
    print_dec(dev.gtt_size / (1024*1024));
    print_string(" MB)\n");
    
    // Enable PCI resources
    enable_pci_resources();
    
    // Initialize power
    if (init_power() != 0) {
        print_string("    [ERROR] Power initialization failed\n");
        return -1;
    }
    
    // Check current display state
    uint32_t pipe_conf = mmio_read32(PIPEA_CONF);
    uint32_t plane_ctl = mmio_read32(DSPACNTR);
    
    print_string("    Current state: Pipe ");
    if (pipe_conf & PIPEACONF_ENABLE) {
        print_string("ON");
    } else {
        print_string("OFF");
    }
    print_string(", Plane ");
    if (plane_ctl & PLANE_CTL_ENABLE) {
        print_string("ON\n");
    } else {
        print_string("OFF\n");
    }
    
    // Try to detect current mode from registers
    if (pipe_conf & PIPEACONF_ENABLE) {
        uint32_t src = mmio_read32(PIPEA_SRC);
        uint32_t w = (src & 0xFFFF) + 1;
        uint32_t h = ((src >> 16) & 0xFFFF) + 1;
        
        print_string("    Detected mode: ");
        print_dec(w);
        print_string("x");
        print_dec(h);
        print_string("\n");
        
        // Allocate framebuffer for detected mode
        alloc_framebuffer(w, h, 32);
    } else {
        // Default fallback
        print_string("    Using default: 1024x768x32\n");
        alloc_framebuffer(1024, 768, 32);
    }
    
    // Setup display plane
    setup_plane();
    
    // Configure pipe (if needed)
    setup_pipe();
    
    dev.initialized = 1;
    
    print_string("  ===========================\n");
    print_string("  [OK] Driver initialized\n\n");
    
    return 0;
}

// Cleanup
void i915_hd4600_cleanup(void) {
    if (!dev.initialized) return;
    
    // Disable plane
    mmio_write32(DSPACNTR, 0);
    mmio_write32(DSPASURF, 0);
    
    dev.initialized = 0;
}

// Clear screen
void i915_clear_screen(uint32_t color) {
    if (!dev.initialized) return;
    
    uint32_t* fb = (uint32_t*)dev.fb_virt;
    uint32_t pixels = (dev.pitch / 4) * dev.height;
    
    for (uint32_t i = 0; i < pixels; i++) {
        fb[i] = color;
    }
}

// Draw pixel
void i915_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!dev.initialized) return;
    if (x >= dev.width || y >= dev.height) return;
    
    uint32_t* fb = (uint32_t*)dev.fb_virt;
    fb[y * (dev.pitch / 4) + x] = color;
}

// Draw rectangle
void i915_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t dy = 0; dy < h; dy++) {
        for (uint32_t dx = 0; dx < w; dx++) {
            i915_putpixel(x + dx, y + dy, color);
        }
    }
}

// Get device info
i915_device_t* i915_get_device(void) {
    return dev.initialized ? &dev : NULL;
}

// Print detailed info
void i915_print_info(void) {
    if (!dev.initialized) {
        print_string("  i915 driver not initialized\n");
        return;
    }
    
    print_string("  Active framebuffer: ");
    print_dec(dev.width);
    print_string("x");
    print_dec(dev.height);
    print_string("x");
    print_dec(dev.bpp);
    print_string("\n");
}