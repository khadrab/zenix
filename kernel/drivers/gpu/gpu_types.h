// kernel/drivers/gpu/gpu_types.h
#ifndef GPU_TYPES_H
#define GPU_TYPES_H

#include "../../../include/types.h"

// GPU Vendor IDs
#define GPU_VENDOR_INTEL    0x8086
#define GPU_VENDOR_AMD      0x1002
#define GPU_VENDOR_NVIDIA   0x10DE

// GPU Types
typedef enum {
    GPU_TYPE_UNKNOWN = 0,
    GPU_TYPE_INTEL,
    GPU_TYPE_AMD,
    GPU_TYPE_NVIDIA
} gpu_type_t;

// GPU Device Information
typedef struct {
    // PCI Location
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    
    // Device IDs
    uint16_t vendor_id;
    uint16_t device_id;
    
    // BARs (Base Address Registers)
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    
    // Device Info
    gpu_type_t type;
    const char* vendor_name;
    const char* device_name;
    
    int detected;
} gpu_device_t;

#endif // GPU_TYPES_H