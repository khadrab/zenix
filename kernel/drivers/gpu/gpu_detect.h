// kernel/drivers/gpu/gpu_detect.h
// GPU Detection Interface
#ifndef GPU_DETECT_H
#define GPU_DETECT_H

#include "gpu_types.h"

// Initialize GPU detection (does NOT switch modes)
int gpu_detect_init(void);

// Get detected GPU info
gpu_device_t* gpu_get_detected(void);

// Check if GPU was detected
int gpu_is_detected(void);

// Print GPU info to console
void gpu_print_info(void);

#endif // GPU_DETECT_H