#include "paging.h"
#include "pmm.h"
#include "../hal/isr.h"
#include "../core/monitor.h"

page_directory_t* kernel_directory = 0;
page_directory_t* current_directory = 0;
static int paging_enabled = 0;

static void page_fault_handler(registers_t* regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    print_string("\n[PAGE FAULT] ");
    
    if (!(regs->err_code & 0x1)) print_string("Not present ");
    if (regs->err_code & 0x2) print_string("Write ");
    else print_string("Read ");
    if (regs->err_code & 0x4) print_string("User ");
    else print_string("Kernel ");
    
    print_string("at 0x");
    print_hex(faulting_address);
    print_string("\nEIP: 0x");
    print_hex(regs->eip);
    print_string("\n");
    
    for(;;) asm("hlt");
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3" :: "r"(dir->physical_addr));
    
    if (!paging_enabled) {
        uint32_t cr0;
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000;
        asm volatile("mov %0, %%cr0" :: "r"(cr0));
        paging_enabled = 1;
    }
}

page_t* paging_get_page(uint32_t address, int make, page_directory_t* dir) {
    address /= PAGE_SIZE;
    uint32_t table_idx = address / 1024;
    uint32_t page_idx = address % 1024;
    
    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[page_idx];
    } else if (make) {
        uint32_t phys = (uint32_t)pmm_alloc_page();
        if (!phys) {
            return 0;
        }
        
        dir->tables[table_idx] = (page_table_t*)phys;
        dir->tables_physical[table_idx] = phys | PAGE_PRESENT | PAGE_WRITE;
        
        // Clear table
        page_table_t* table = (page_table_t*)phys;
        for (int i = 0; i < 1024; i++) {
            table->pages[i].present = 0;
            table->pages[i].rw = 0;
            table->pages[i].user = 0;
            table->pages[i].frame = 0;
        }
        
        return &table->pages[page_idx];
    }
    
    return 0;
}

void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags, page_directory_t* dir) {
    page_t* page = paging_get_page(virt, 1, dir);
    if (page) {
        page->present = (flags & PAGE_PRESENT) ? 1 : 0;
        page->rw = (flags & PAGE_WRITE) ? 1 : 0;
        page->user = (flags & PAGE_USER) ? 1 : 0;
        page->frame = phys / PAGE_SIZE;
    }
}

void paging_init() {
    print_string("[PAGING] Initializing virtual memory...\n");
    
    // Allocate page directory
    uint32_t dir_phys = (uint32_t)pmm_alloc_page();
    if (!dir_phys) {
        print_string("[PAGING] Failed to allocate page directory!\n");
        return;
    }
    
    kernel_directory = (page_directory_t*)dir_phys;
    kernel_directory->physical_addr = dir_phys;
    
    // Clear directory
    for (int i = 0; i < 1024; i++) {
        kernel_directory->tables[i] = 0;
        kernel_directory->tables_physical[i] = 0;
    }
    
    print_string("[PAGING] Page directory at: 0x");
    print_hex(dir_phys);
    print_string("\n");
    
    // Identity map first 32MB (kernel + PMM + heap + page structures)
    print_string("[PAGING] Identity mapping first 32MB...\n");
    
    uint32_t map_size = 32 * 1024 * 1024;  // 32MB
    uint32_t num_pages = map_size / PAGE_SIZE;
    
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t phys = i * PAGE_SIZE;
        paging_map_page(phys, phys, PAGE_PRESENT | PAGE_WRITE, kernel_directory);
    }
    
    print_string("[PAGING] Mapped ");
    print_dec(map_size / 1024 / 1024);
    print_string(" MB\n");
    
    // Register page fault handler
    print_string("[PAGING] Registering page fault handler...\n");
    isr_register_handler(14, page_fault_handler);
    
    // Enable paging
    print_string("[PAGING] Enabling paging...\n");
    paging_switch_directory(kernel_directory);
    
    print_string("[PAGING] Virtual memory enabled successfully!\n");
}

int paging_is_enabled() {
    return paging_enabled;
}