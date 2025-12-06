#ifndef PMM_H
#define PMM_H

#include "../../include/types.h"
#include "../../include/multiboot.h"

#define PAGE_SIZE 4096
#define BITMAP_SIZE (128 * 1024)  // ← أضف هذا

extern uint32_t* memory_bitmap;
extern uint32_t total_blocks;
extern uint32_t total_memory;
extern uint32_t used_blocks;

void pmm_init(multiboot_info_t* mbi, uint32_t kernel_end);
void* pmm_alloc_page();
void pmm_free_page(void* page);
uint32_t pmm_get_total_memory();
uint32_t pmm_get_used_memory();
uint32_t pmm_get_free_memory();

#endif