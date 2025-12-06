#include "process.h"
#include "../mm/heap.h"
#include "../../lib/libc/string.h"
#include "../core/monitor.h"
#include "../drivers/timer/pit.h"

process_t* process_table[MAX_PROCESSES];
static uint32_t next_pid = 1;
process_t* current_process = 0;

void process_init() {
    print_string("[PROC] Initializing process management...\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = 0;
    }
    
    // Create idle process (PID 0)
    process_t* idle = (process_t*)kmalloc(sizeof(process_t));
    if (!idle) {
        print_string("[PROC] Failed to create idle process!\n");
        return;
    }
    
    idle->pid = 0;
    strcpy(idle->name, "idle");
    idle->state = PROCESS_RUNNING;
    idle->kernel_stack = 0;
    idle->created_at = timer_get_ticks();
    idle->cpu_time = 0;
    idle->next = 0;
    
    process_table[0] = idle;
    current_process = idle;
    
    print_string("[PROC] Process management initialized\n");
}

process_t* process_create(const char* name, void (*entry_point)()) {
    // Find free slot
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i] == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        print_string("[PROC] No free process slots!\n");
        return 0;
    }
    
    // Allocate process structure
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        print_string("[PROC] Failed to allocate process!\n");
        return 0;
    }
    
    // Allocate kernel stack
    uint32_t stack = (uint32_t)kmalloc(KERNEL_STACK_SIZE);
    if (!stack) {
        kfree(proc);
        print_string("[PROC] Failed to allocate stack!\n");
        return 0;
    }
    
    // Initialize process
    proc->pid = next_pid++;
    strncpy(proc->name, name, 31);
    proc->name[31] = '\0';
    proc->state = PROCESS_READY;
    proc->kernel_stack = stack + KERNEL_STACK_SIZE;
    proc->created_at = timer_get_ticks();
    proc->cpu_time = 0;
    proc->next = 0;
    
    // Setup initial stack frame
    uint32_t* stack_ptr = (uint32_t*)proc->kernel_stack;
    
    *(--stack_ptr) = 0x202;           // EFLAGS (IF=1)
    *(--stack_ptr) = (uint32_t)entry_point;  // EIP
    *(--stack_ptr) = 0;               // EAX
    *(--stack_ptr) = 0;               // ECX
    *(--stack_ptr) = 0;               // EDX
    *(--stack_ptr) = 0;               // EBX
    *(--stack_ptr) = proc->kernel_stack; // ESP
    *(--stack_ptr) = 0;               // EBP
    *(--stack_ptr) = 0;               // ESI
    *(--stack_ptr) = 0;               // EDI
    
    proc->regs.esp = (uint32_t)stack_ptr;
    proc->regs.eip = (uint32_t)entry_point;
    proc->regs.eflags = 0x202;
    
    process_table[slot] = proc;
    
    print_string("[PROC] Created process: ");
    print_string(name);
    print_string(" (PID ");
    print_dec(proc->pid);
    print_string(")\n");
    
    return proc;
}

void process_terminate(process_t* proc) {
    if (!proc || proc->pid == 0) {
        return;  // Can't kill idle process
    }
    
    proc->state = PROCESS_TERMINATED;
    
    // Free resources
    if (proc->kernel_stack) {
        kfree((void*)(proc->kernel_stack - KERNEL_STACK_SIZE));
    }
    
    // Remove from table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == proc) {
            process_table[i] = 0;
            break;
        }
    }
    
    kfree(proc);
}

process_t* process_get_current() {
    return current_process;
}

void process_list() {
    print_string("\nPID  Name              State    CPU Time\n");
    print_string("---  ----------------  -------  --------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i]) {
            process_t* p = process_table[i];
            
            // PID
            if (p->pid < 10) print_char(' ');
            print_dec(p->pid);
            print_string("   ");
            
            // Name
            print_string(p->name);
            for (uint32_t j = strlen(p->name); j < 16; j++) {
                print_char(' ');
            }
            print_string("  ");
            
            // State
            switch(p->state) {
                case PROCESS_READY:   print_string("READY  "); break;
                case PROCESS_RUNNING: print_string("RUNNING"); break;
                case PROCESS_BLOCKED: print_string("BLOCKED"); break;
                case PROCESS_TERMINATED: print_string("DEAD   "); break;
            }
            print_string("  ");
            
            // CPU time
            print_dec(p->cpu_time);
            print_string(" ticks\n");
        }
    }
    print_char('\n');
}