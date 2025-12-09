// kernel/mm/paging.c
#include "paging.h"
#include "pmm.h"
#include "../core/monitor.h"
#include "../hal/isr.h"

// Kernel page directory
static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

// Page tables pool (temporary, should use dynamic allocation)
static page_table_t page_tables[256] __attribute__((aligned(PAGE_SIZE)));
static uint32_t next_table_index = 0;

// Forward declaration
static void page_fault_handler_internal(uint32_t error_code, uint32_t faulting_addr);

// Assembly function to load page directory
extern void paging_load_directory(uint32_t phys_addr);
extern void paging_enable(void);

// Get a free page table
static page_table_t* alloc_page_table(void) {
    if (next_table_index >= 256) {
        return NULL; // Out of tables
    }
    
    page_table_t* table = &page_tables[next_table_index++];
    
    // Clear table
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        table->entries[i].present = 0;
        table->entries[i].rw = 1;
        table->entries[i].user = 0;
        table->entries[i].frame = 0;
    }
    
    return table;
}

// Page fault handler wrapper
void page_fault_handler(registers_t* regs) {
    // Get faulting address from CR2
    uint32_t faulting_addr;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_addr));
    
    page_fault_handler_internal(regs->err_code, faulting_addr);
}

// Page fault handler implementation
static void page_fault_handler_internal(uint32_t error_code, uint32_t faulting_addr) {
    page_fault_error_t error;
    *((uint32_t*)&error) = error_code;
    
    print_string("\n\n!!! PAGE FAULT !!!\n");
    print_string("Address: 0x");
    print_hex(faulting_addr);
    print_string("\n");
    
    print_string("Error: ");
    if (!error.present) print_string("Page not present ");
    if (error.write) print_string("Write access ");
    else print_string("Read access ");
    if (error.user) print_string("User mode ");
    else print_string("Supervisor mode ");
    if (error.reserved) print_string("Reserved bit ");
    if (error.inst_fetch) print_string("Instruction fetch ");
    print_string("\n");
    
    // Halt
    print_string("System halted.\n");
    for(;;) asm("cli; hlt");
}

void paging_init(void) {
    print_string("  Initializing paging...\n");
    
    // Allocate kernel directory (use pmm_alloc_page instead of pmm_alloc_block)
    kernel_directory = (page_directory_t*)pmm_alloc_page();
    if (!kernel_directory) {
        print_string("  [ERROR] Failed to allocate page directory\n");
        return;
    }
    
    print_string("    Page directory at: 0x");
    print_hex((uint32_t)kernel_directory);
    print_string("\n");
    
    // Clear directory
    for (int i = 0; i < PAGE_DIR_SIZE; i++) {
        kernel_directory->entries[i].present = 0;
        kernel_directory->entries[i].rw = 1;
        kernel_directory->entries[i].user = 0;
        kernel_directory->entries[i].frame = 0;
    }
    
    // Identity map first 16MB (kernel space + heap)
    print_string("    Identity mapping first 16MB...\n");
    
    for (uint32_t i = 0; i < 0x1000000; i += PAGE_SIZE) {
        paging_map_page(i, i, PAGE_PRESENT | PAGE_WRITE);
    }
    
    print_string("    Mapped 0x00000000 - 0x01000000\n");
    
    // Register page fault handler (ISR 14)
    isr_register_handler(14, page_fault_handler);
    
    // Switch to kernel directory
    current_directory = kernel_directory;
    
    print_string("    Loading page directory...\n");
    paging_load_directory((uint32_t)kernel_directory);
    
    print_string("    Enabling paging...\n");
    paging_enable();
    
    print_string("  [OK] Paging enabled!\n");
}

void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_index = PAGE_DIR_INDEX(virt);
    uint32_t table_index = PAGE_TABLE_INDEX(virt);
    
    // Get or create page table
    page_table_t* table;
    
    if (!kernel_directory->entries[dir_index].present) {
        // Allocate new page table
        table = alloc_page_table();
        if (!table) {
            print_string("Failed to allocate page table!\n");
            return;
        }
        
        // Set directory entry
        kernel_directory->entries[dir_index].present = 1;
        kernel_directory->entries[dir_index].rw = 1;
        kernel_directory->entries[dir_index].user = (flags & PAGE_USER) ? 1 : 0;
        kernel_directory->entries[dir_index].frame = ((uint32_t)table) >> 12;
    } else {
        // Get existing table
        table = (page_table_t*)(kernel_directory->entries[dir_index].frame << 12);
    }
    
    // Set page table entry
    table->entries[table_index].present = (flags & PAGE_PRESENT) ? 1 : 0;
    table->entries[table_index].rw = (flags & PAGE_WRITE) ? 1 : 0;
    table->entries[table_index].user = (flags & PAGE_USER) ? 1 : 0;
    table->entries[table_index].frame = phys >> 12;
}

void paging_unmap_page(uint32_t virt) {
    uint32_t dir_index = PAGE_DIR_INDEX(virt);
    uint32_t table_index = PAGE_TABLE_INDEX(virt);
    
    if (!kernel_directory->entries[dir_index].present) {
        return;
    }
    
    page_table_t* table = (page_table_t*)(kernel_directory->entries[dir_index].frame << 12);
    table->entries[table_index].present = 0;
    
    // Flush TLB
    asm volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

uint32_t paging_get_physical(uint32_t virt) {
    uint32_t dir_index = PAGE_DIR_INDEX(virt);
    uint32_t table_index = PAGE_TABLE_INDEX(virt);
    
    if (!kernel_directory->entries[dir_index].present) {
        return 0;
    }
    
    page_table_t* table = (page_table_t*)(kernel_directory->entries[dir_index].frame << 12);
    
    if (!table->entries[table_index].present) {
        return 0;
    }
    
    return (table->entries[table_index].frame << 12) | (virt & 0xFFF);
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    paging_load_directory((uint32_t)dir);
}

page_directory_t* paging_get_directory(void) {
    return current_directory;
}

// Test paging functionality
void paging_test(void) {
    print_string("\n=== Paging Tests ===\n");
    
    // Test 1: Identity mapping check
    print_string("Test 1: Identity mapping\n");
    uint32_t test_virt = 0x100000;
    uint32_t test_phys = paging_get_physical(test_virt);
    print_string("  Virtual 0x");
    print_hex(test_virt);
    print_string(" -> Physical 0x");
    print_hex(test_phys);
    if (test_virt == test_phys) {
        print_string(" [PASS]\n");
    } else {
        print_string(" [FAIL]\n");
    }
    
    // Test 2: Read/Write test
    print_string("Test 2: Memory read/write\n");
    uint32_t* test_ptr = (uint32_t*)0x200000;
    *test_ptr = 0xDEADBEEF;
    if (*test_ptr == 0xDEADBEEF) {
        print_string("  Write/Read: 0xDEADBEEF [PASS]\n");
    } else {
        print_string("  Write/Read failed [FAIL]\n");
    }
    
    // Test 3: Check page table exists
    print_string("Test 3: Page table lookup\n");
    uint32_t dir_idx = PAGE_DIR_INDEX(0x200000);
    if (kernel_directory->entries[dir_idx].present) {
        print_string("  Page table exists [PASS]\n");
    } else {
        print_string("  Page table missing [FAIL]\n");
    }
    
    // Test 4: Map new page
    print_string("Test 4: Dynamic mapping\n");
    uint32_t new_virt = 0x500000;
    uint32_t new_phys = 0x800000;
    paging_map_page(new_virt, new_phys, PAGE_PRESENT | PAGE_WRITE);
    uint32_t mapped_phys = paging_get_physical(new_virt);
    print_string("  Mapped 0x");
    print_hex(new_virt);
    print_string(" -> 0x");
    print_hex(mapped_phys);
    if (mapped_phys == new_phys) {
        print_string(" [PASS]\n");
    } else {
        print_string(" [FAIL]\n");
    }
    
    print_string("=== All Tests Complete ===\n\n");
}
