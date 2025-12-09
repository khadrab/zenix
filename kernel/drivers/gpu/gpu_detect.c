// kernel/drivers/gpu/gpu_detect.c
// GPU Detection Implementation
#include "gpu_detect.h"
#include "../../core/monitor.h"

// Port I/O functions
static inline void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// PCI Configuration Space Access
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static gpu_device_t detected_gpu = {0};

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) | 
                       ((uint32_t)bus << 16) | 
                       ((uint32_t)slot << 11) | 
                       ((uint32_t)func << 8) | 
                       (offset & 0xFC);
    
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static const char* get_intel_name(uint16_t device_id) {
    // Simplified Intel GPU identification
    if (device_id >= 0x0100 && device_id <= 0x01FF) return "Intel HD Graphics (Gen7)";
    if (device_id >= 0x0400 && device_id <= 0x04FF) return "Intel HD Graphics (Gen8)";
    if (device_id >= 0x1600 && device_id <= 0x16FF) return "Intel HD Graphics (Gen9)";
    if (device_id >= 0x5900 && device_id <= 0x59FF) return "Intel UHD Graphics (Gen10)";
    if (device_id >= 0x9B00 && device_id <= 0x9BFF) return "Intel UHD Graphics (Gen11)";
    return "Intel Graphics Controller";
}

int gpu_detect_init(void) {
    print_string("  Scanning PCI for GPU...\n");
    
    detected_gpu.detected = 0;
    
    // Scan PCI bus for VGA-compatible devices
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t config = pci_read_config(bus, slot, func, 0);
                uint16_t vendor = config & 0xFFFF;
                uint16_t device = (config >> 16) & 0xFFFF;
                
                if (vendor == 0xFFFF) continue;
                
                // Read class code
                uint32_t class_reg = pci_read_config(bus, slot, func, 0x08);
                uint8_t base_class = (class_reg >> 24) & 0xFF;
                uint8_t sub_class = (class_reg >> 16) & 0xFF;
                
                // VGA Compatible Controller (Class 03, Subclass 00)
                if (base_class == 0x03 && sub_class == 0x00) {
                    detected_gpu.vendor_id = vendor;
                    detected_gpu.device_id = device;
                    detected_gpu.bus = bus;
                    detected_gpu.slot = slot;
                    detected_gpu.func = func;
                    
                    // Read BARs
                    detected_gpu.bar0 = pci_read_config(bus, slot, func, 0x10);
                    detected_gpu.bar1 = pci_read_config(bus, slot, func, 0x14);
                    detected_gpu.bar2 = pci_read_config(bus, slot, func, 0x18);
                    
                    // Identify vendor
                    switch (vendor) {
                        case GPU_VENDOR_INTEL:
                            detected_gpu.type = GPU_TYPE_INTEL;
                            detected_gpu.vendor_name = "Intel";
                            detected_gpu.device_name = get_intel_name(device);
                            break;
                        case GPU_VENDOR_AMD:
                            detected_gpu.type = GPU_TYPE_AMD;
                            detected_gpu.vendor_name = "AMD";
                            detected_gpu.device_name = "AMD Radeon Graphics";
                            break;
                        case GPU_VENDOR_NVIDIA:
                            detected_gpu.type = GPU_TYPE_NVIDIA;
                            detected_gpu.vendor_name = "NVIDIA";
                            detected_gpu.device_name = "NVIDIA GeForce";
                            break;
                        default:
                            detected_gpu.type = GPU_TYPE_UNKNOWN;
                            detected_gpu.vendor_name = "Unknown";
                            detected_gpu.device_name = "Unknown GPU";
                            break;
                    }
                    
                    detected_gpu.detected = 1;
                    return 0;
                }
            }
        }
    }
    
    print_string("  No GPU detected\n");
    return -1;
}

gpu_device_t* gpu_get_detected(void) {
    return detected_gpu.detected ? &detected_gpu : NULL;
}

int gpu_is_detected(void) {
    return detected_gpu.detected;
}

void gpu_print_info(void) {
    if (!detected_gpu.detected) {
        print_string("  No GPU detected\n");
        return;
    }
    
    print_string("  GPU: ");
    print_string(detected_gpu.vendor_name);
    print_string(" ");
    print_string(detected_gpu.device_name);
    print_string("\n");
    
    print_string("  Vendor ID: 0x");
    print_hex(detected_gpu.vendor_id);
    print_string("\n");
    
    print_string("  Device ID: 0x");
    print_hex(detected_gpu.device_id);
    print_string("\n");
    
    print_string("  Location: Bus ");
    print_dec(detected_gpu.bus);
    print_string(", Slot ");
    print_dec(detected_gpu.slot);
    print_string(", Func ");
    print_dec(detected_gpu.func);
    print_string("\n");
    
    print_string("  BAR0: 0x");
    print_hex(detected_gpu.bar0);
    print_string("\n");
}