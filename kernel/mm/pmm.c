#include "pmm.h"
#include "../core/monitor.h"

#define BITMAP_SIZE (128 * 1024)

uint32_t total_memory = 0;
uint32_t used_blocks = 0;
uint32_t total_blocks = 0;
uint32_t* memory_bitmap = 0;

static inline void bitmap_set(uint32_t bit) {
    memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void bitmap_clear(uint32_t bit) {
    memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline uint32_t bitmap_test(uint32_t bit) {
    return memory_bitmap[bit / 32] & (1 << (bit % 32));
}

static int32_t bitmap_find_free() {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!bitmap_test(i)) {
            return i;
        }
    }
    return -1;
}

void pmm_init(multiboot_info_t* mbi, uint32_t kernel_end) {
    print_string("[PMM] Initializing Physical Memory Manager...\n");
    
    if (mbi->flags & MULTIBOOT_FLAG_MEM) {
        total_memory = (mbi->mem_lower + mbi->mem_upper) * 1024;
        print_string("[PMM] Total memory: ");
        print_dec(total_memory / 1024 / 1024);
        print_string(" MB\n");
    } else {
        total_memory = 32 * 1024 * 1024;
        print_string("[PMM] Assuming 32 MB\n");
    }
    
    total_blocks = total_memory / PAGE_SIZE;
    used_blocks = total_blocks;
    
    memory_bitmap = (uint32_t*)((kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
    
    print_string("[PMM] Bitmap at: 0x");
    print_hex((uint32_t)memory_bitmap);
    print_string("\n");
    
    for (uint32_t i = 0; i < BITMAP_SIZE / 4; i++) {
        memory_bitmap[i] = 0xFFFFFFFF;
    }
    
    uint32_t bitmap_end = (uint32_t)memory_bitmap + BITMAP_SIZE;
    uint32_t free_start_addr = (bitmap_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint32_t free_start = free_start_addr / PAGE_SIZE;
    uint32_t free_end = total_memory / PAGE_SIZE;
    
    print_string("[PMM] Free region starts at: 0x");
    print_hex(free_start_addr);
    print_string("\n");
    
    for (uint32_t i = free_start; i < free_end; i++) {
        bitmap_clear(i);
        used_blocks--;
    }
    
    print_string("[PMM] Available: ");
    print_dec(pmm_get_free_memory() / 1024 / 1024);
    print_string(" MB\n");
}

void* pmm_alloc_page() {
    int32_t page = bitmap_find_free();
    if (page < 0) {
        return 0;
    }
    
    bitmap_set(page);
    used_blocks++;
    
    return (void*)(page * PAGE_SIZE);
}

void pmm_free_page(void* page) {
    uint32_t page_num = (uint32_t)page / PAGE_SIZE;
    
    if (page_num >= total_blocks) {
        return;
    }
    
    if (!bitmap_test(page_num)) {
        return;
    }
    
    bitmap_clear(page_num);
    used_blocks--;
}

uint32_t pmm_get_total_memory() {
    return total_memory;
}

uint32_t pmm_get_used_memory() {
    return used_blocks * PAGE_SIZE;
}

uint32_t pmm_get_free_memory() {
    return (total_blocks - used_blocks) * PAGE_SIZE;
}