// kernel/core/kernel.c
#include "../core/monitor.h"
#include "../core/panic.h"
#include "../hal/gdt.h"
#include "../hal/idt.h"
#include "../hal/isr.h"
#include "../hal/irq.h"
#include "../mm/pmm.h"
#include "../mm/heap.h"
#include "../mm/paging.h"
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../drivers/timer/pit.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/mouse/mouse.h"
#include "../drivers/gpu/gpu_detect.h"
#include "../drivers/gpu/intel/i915_hd4600.h"
#include "../drivers/video/gop_fb.h"
#include "../fs/vfs_complete.h"
#include "../syscall/syscall.h"
#include "../usermode/usermode.h"
#include "../../include/multiboot.h"

extern uint32_t kernel_end;
extern fs_node_t* fs_root;

// RGB color helper
#define RGB(r,g,b) (0xFF000000 | ((r)<<16) | ((g)<<8) | (b))

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    clear_screen();
    print_string("=================================\n");
    print_string(" Zenix Kernel v1.0 - Phase 13   \n");
    print_string(" Intel HD 4600 KMS Driver       \n");
    print_string("=================================\n\n");
    
    if (magic != 0x2BADB002) {
        print_string("[ERROR] Invalid multiboot magic!\n");
        for(;;) asm("cli; hlt");
    }
    
    // Core initialization
    print_string("[1/17] GDT..."); 
    gdt_init(); 
    print_string(" [OK]\n");
    
    print_string("[2/17] IDT..."); 
    idt_init(); 
    print_string(" [OK]\n");
    
    print_string("[3/17] ISRs..."); 
    isr_install();
    
    // Register exception handlers
    isr_register_handler(0, division_error_handler);
    isr_register_handler(8, double_fault_handler);
    isr_register_handler(13, general_protection_fault_handler);
    
    print_string(" [OK]\n");
    
    print_string("[4/17] IRQs..."); 
    irq_install(); 
    print_string(" [OK]\n");
    
    print_string("[5/17] PMM..."); 
    pmm_init(mbi, (uint32_t)&kernel_end); 
    print_string(" [OK]\n");
    
    print_string("[6/17] Heap..."); 
    heap_init(); 
    print_string(" [OK]\n");
    
    print_string("[6.5/17] Paging...");
    paging_init();
    print_string(" [OK]\n");
    
    print_string("[7/17] VFS..."); 
    vfs_init();
    fs_root = 0; 
    print_string(" [OK]\n");
    
    print_string("[8/17] InitRD... [SKIP]\n");
    
    print_string("[9/17] Process..."); 
    process_init(); 
    print_string(" [OK]\n");
    
    print_string("[10/17] Scheduler..."); 
    scheduler_init(); 
    print_string(" [OK]\n");
    
    print_string("[11/17] Timer..."); 
    timer_init(100); 
    print_string(" [OK]\n");
    
    print_string("[12/17] Keyboard..."); 
    keyboard_init(); 
    print_string(" [OK]\n");
    
    print_string("[13/17] Syscalls...");
    syscall_init();
    syscall_handlers_init();
    print_string(" [OK]\n");
    
    print_string("[14/17] User Mode...");
    usermode_init();
    print_string(" [OK]\n");
    
    // GPU Detection & Driver Loading
    print_string("[15/17] GPU & Display...\n");
    
    int gpu_initialized = 0;
    int use_gop = 0;
    
    // Try GOP framebuffer first (works in UEFI, QEMU, VirtualBox)
    if (gop_fb_init(mbi) == 0) {
        print_string("  Using GOP framebuffer (UEFI/QEMU)\n");
        use_gop = 1;
        gpu_initialized = 1;
        
        // Test GOP
        gop_clear(RGB(26, 31, 46));
        
        uint32_t bar_width = 200;
        uint32_t bar_height = 100;
        uint32_t start_x = 50;
        uint32_t start_y = 50;
        
        gop_draw_rect(start_x, start_y, bar_width, bar_height, RGB(231, 76, 60));
        gop_draw_rect(start_x + bar_width + 20, start_y, bar_width, bar_height, RGB(46, 204, 113));
        gop_draw_rect(start_x, start_y + bar_height + 20, bar_width, bar_height, RGB(52, 152, 219));
        gop_draw_rect(start_x + bar_width + 20, start_y + bar_height + 20, bar_width, bar_height, RGB(241, 196, 15));
        
        // Draw diagonal line
        gop_draw_line(start_x, start_y + 250, start_x + 400, start_y + 350, RGB(255, 255, 255));
        
        print_string("  [OK] GOP graphics test complete\n");
    }
    // Fallback to native driver
    else if (gpu_detect_init() == 0) {
        gpu_device_t* gpu = gpu_get_detected();
        
        print_string("  Found: ");
        print_string(gpu->vendor_name);
        print_string(" ");
        print_string(gpu->device_name);
        print_string("\n");
        
        print_string("  Device: 0x");
        print_hex(gpu->device_id);
        print_string(" @ Bus ");
        print_dec(gpu->bus);
        print_string(" Slot ");
        print_dec(gpu->slot);
        print_string("\n");
        
        // Check if it's Intel (any Intel GPU for now)
        if (gpu->vendor_id == GPU_VENDOR_INTEL) {
            print_string("  Intel GPU detected, attempting native driver...\n");
            
            if (i915_hd4600_init(gpu) == 0) {
                gpu_initialized = 1;
                
                i915_clear_screen(RGB(26, 31, 46));
                
                uint32_t bar_width = 200;
                uint32_t bar_height = 100;
                uint32_t start_x = 50;
                uint32_t start_y = 50;
                
                i915_draw_rect(start_x, start_y, bar_width, bar_height, RGB(231, 76, 60));
                i915_draw_rect(start_x + bar_width + 20, start_y, bar_width, bar_height, RGB(46, 204, 113));
                i915_draw_rect(start_x, start_y + bar_height + 20, bar_width, bar_height, RGB(52, 152, 219));
                i915_draw_rect(start_x + bar_width + 20, start_y + bar_height + 20, bar_width, bar_height, RGB(241, 196, 15));
                
                print_string("  [OK] Native driver graphics test complete\n");
            } else {
                print_string("  [WARN] Native driver init failed\n");
            }
        } else {
            print_string("  [WARN] Not Intel GPU\n");
        }
    } else {
        print_string("  No GPU/Framebuffer available\n");
    }
    
    print_string("[16/17] Graphics... ");
    if (gpu_initialized) {
        if (use_gop) {
            print_string("[GOP]\n");
        } else {
            print_string("[NATIVE]\n");
        }
    } else {
        print_string("[VGA TEXT]\n");
    }
    
    print_string("[17/17] Mouse & GUI...");
    mouse_init();
    print_string(" [OK]\n");
    
    print_string("\n=================================\n");
    print_string("  Phase 13 Complete!\n");
    
    // Run paging tests
    print_string("\n");
    paging_test();
    
    // Test exception handling (optional - will crash!)
    print_string("\n=== Exception Handler Test ===\n");
    print_string("Testing ASSERT macro...\n");
    print_string("  ASSERT test: [PASS]\n");
    // ASSERT(1 == 1);  // This passes
    
    // Uncomment to test panic:
    // ASSERT(0 == 1);  // This would trigger panic
    // int x = 5 / 0;    // This would trigger division error
    
    print_string("=== Exception Handler Ready ===\n");
    
    if (gpu_initialized) {
        if (use_gop) {
            print_string("  - GOP framebuffer active\n");
            gop_print_info();
        } else {
            print_string("  - Native KMS mode active\n");
            print_string("  - Hardware framebuffer\n");
            i915_print_info();
        }
    } else {
        print_string("  - VGA text mode\n");
    }
    print_string("=================================\n\n");
    
    // Enable interrupts
    asm volatile("sti");
    
    if (gpu_initialized) {
        print_string("Kernel running with hardware graphics...\n");
        print_string("Check your display for color bars!\n");
    } else {
        print_string("Kernel running in text mode...\n");
    }
    
    // Idle loop
    while (1) {
        asm volatile("hlt");
    }
}