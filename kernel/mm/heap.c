#include "heap.h"
#include "pmm.h"
#include "../core/monitor.h"

#define HEAP_START 0x00400000  // 4MB
#define HEAP_SIZE  0x00400000  // 4MB heap

typedef struct heap_block {
    uint32_t size;
    uint32_t used;
    struct heap_block* next;
} heap_block_t;

static heap_block_t* heap_start = 0;
static uint32_t heap_used = 0;

void heap_init() {
    print_string("[HEAP] Initializing kernel heap...\n");
    
    heap_start = (heap_block_t*)HEAP_START;
    heap_start->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_start->used = 0;
    heap_start->next = 0;
    heap_used = 0;
    
    print_string("[HEAP] Heap at 0x");
    print_hex(HEAP_START);
    print_string(", size: ");
    print_dec(HEAP_SIZE / 1024);
    print_string(" KB\n");
}

void* kmalloc(uint32_t size) {
    if (size == 0) {
        return 0;
    }
    
    // Align to 4 bytes
    size = (size + 3) & ~3;
    
    heap_block_t* current = heap_start;
    
    while (current) {
        if (!current->used && current->size >= size) {
            // Split block if too large
            if (current->size > size + sizeof(heap_block_t) + 4) {
                heap_block_t* new_block = (heap_block_t*)((uint32_t)current + sizeof(heap_block_t) + size);
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->used = 0;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->used = 1;
            heap_used += current->size;
            
            return (void*)((uint32_t)current + sizeof(heap_block_t));
        }
        
        current = current->next;
    }
    
    return 0;  // Out of memory
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    
    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));
    
    if (!block->used) {
        return;  // Already freed
    }
    
    block->used = 0;
    heap_used -= block->size;
    
    // Merge with next block if free
    if (block->next && !block->next->used) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    // Merge with previous block
    heap_block_t* current = heap_start;
    while (current && current->next != block) {
        current = current->next;
    }
    
    if (current && !current->used) {
        current->size += sizeof(heap_block_t) + block->size;
        current->next = block->next;
    }
}

uint32_t heap_get_used() {
    return heap_used;
}

uint32_t heap_get_free() {
    return HEAP_SIZE - heap_used - sizeof(heap_block_t);
}