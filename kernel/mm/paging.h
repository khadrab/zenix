// kernel/mm/paging.h
#ifndef PAGING_H
#define PAGING_H

#include "../../include/types.h"

// Forward declaration
typedef struct registers registers_t;

// Page directory/table entry flags
#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_SIZE_4MB   0x080
#define PAGE_GLOBAL     0x100

// Page sizes
#define PAGE_SIZE       4096
#define PAGE_TABLE_SIZE 1024
#define PAGE_DIR_SIZE   1024

// Calculate page-aligned address
#define PAGE_ALIGN_DOWN(addr) ((addr) & 0xFFFFF000)
#define PAGE_ALIGN_UP(addr)   (((addr) + 0xFFF) & 0xFFFFF000)

// Get page directory/table indices
#define PAGE_DIR_INDEX(addr)   ((addr) >> 22)
#define PAGE_TABLE_INDEX(addr) (((addr) >> 12) & 0x3FF)

// Page directory entry (points to page table)
typedef struct {
    uint32_t present    : 1;  // Page present in memory
    uint32_t rw         : 1;  // Read/write permission
    uint32_t user       : 1;  // User/supervisor
    uint32_t pwt        : 1;  // Page write-through
    uint32_t pcd        : 1;  // Page cache disabled
    uint32_t accessed   : 1;  // Accessed flag
    uint32_t reserved   : 1;  // Reserved (must be 0)
    uint32_t page_size  : 1;  // 0 = 4KB pages, 1 = 4MB pages
    uint32_t global     : 1;  // Global page (ignored)
    uint32_t available  : 3;  // Available for system use
    uint32_t frame      : 20; // Physical address >> 12
} __attribute__((packed)) page_dir_entry_t;

// Page table entry (points to physical page)
typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t pat        : 1;  // Page attribute table
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t frame      : 20;
} __attribute__((packed)) page_table_entry_t;

// Page directory (1024 entries)
typedef struct {
    page_dir_entry_t entries[PAGE_DIR_SIZE];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

// Page table (1024 entries)
typedef struct {
    page_table_entry_t entries[PAGE_TABLE_SIZE];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

// Page fault error code
typedef struct {
    uint32_t present    : 1;  // 0 = not present, 1 = protection violation
    uint32_t write      : 1;  // 0 = read, 1 = write
    uint32_t user       : 1;  // 0 = supervisor, 1 = user
    uint32_t reserved   : 1;  // 1 = reserved bit set
    uint32_t inst_fetch : 1;  // 1 = instruction fetch
} __attribute__((packed)) page_fault_error_t;

// Initialize paging
void paging_init(void);

// Map a virtual address to physical address
void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags);

// Unmap a page
void paging_unmap_page(uint32_t virt);

// Get physical address from virtual
uint32_t paging_get_physical(uint32_t virt);

// Switch page directory
void paging_switch_directory(page_directory_t* dir);

// Get current directory
page_directory_t* paging_get_directory(void);

// Test paging
void paging_test(void);

#endif // PAGING_H