#ifndef PAGING_H
#define PAGING_H

#include "../../include/types.h"

#define PAGE_SIZE 4096
#define PAGE_PRESENT   0x001
#define PAGE_WRITE     0x002
#define PAGE_USER      0x004

typedef struct page {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t* tables[1024];
    uint32_t tables_physical[1024];
    uint32_t physical_addr;
} page_directory_t;

void paging_init();
void paging_switch_directory(page_directory_t* dir);
page_t* paging_get_page(uint32_t address, int make, page_directory_t* dir);
void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags, page_directory_t* dir);
int paging_is_enabled();

extern page_directory_t* kernel_directory;
extern page_directory_t* current_directory;

#endif