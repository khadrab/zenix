#include "process.h"
#include "../core/monitor.h"
#include "../mm/heap.h"
#include "../drivers/timer/pit.h"
#include "../../lib/libc/string.h"

#define KERNEL_STACK_SIZE 4096

process_t* process_table[MAX_PROCESSES];
static uint32_t next_pid = 0;
process_t* current_process = NULL;

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = NULL;
    }
    
    current_process = (process_t*)kmalloc(sizeof(process_t));
    memset(current_process, 0, sizeof(process_t));
    
    current_process->pid = next_pid++;
    strcpy(current_process->name, "idle");
    current_process->state = PROCESS_RUNNING;
    current_process->created_at = timer_get_ticks();
    current_process->cpu_time = 0;
    current_process->next = NULL;
    
    process_table[0] = current_process;
}

process_t* process_create(const char* name, void (*entry_point)(void)) {
    if (next_pid >= MAX_PROCESSES) {
        print_string("[PROC] Error: Process table full\n");
        return NULL;
    }
    
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        print_string("[PROC] Error: Failed to allocate process\n");
        return NULL;
    }
    
    memset(proc, 0, sizeof(process_t));
    
    uint32_t* stack = (uint32_t*)kmalloc(KERNEL_STACK_SIZE);
    if (!stack) {
        print_string("[PROC] Error: Failed to allocate stack\n");
        kfree(proc);
        return NULL;
    }
    
    proc->pid = next_pid++;
    strncpy(proc->name, name, 31);
    proc->name[31] = '\0';
    proc->state = PROCESS_READY;
    proc->created_at = timer_get_ticks();
    proc->cpu_time = 0;
    proc->kernel_stack = (uint32_t)stack + KERNEL_STACK_SIZE;
    
    memset(&proc->regs, 0, sizeof(registers_t));
    proc->regs.eip = (uint32_t)entry_point;
    proc->regs.cs = 0x08;
    proc->regs.ds = 0x10;
    proc->regs.eflags = 0x202;
    proc->regs.useresp = proc->kernel_stack;
    proc->regs.ss = 0x10;
    
    proc->next = NULL;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == NULL) {
            process_table[i] = proc;
            break;
        }
    }
    
    print_string("[PROC] Created process: ");
    print_string(proc->name);
    print_string(" (PID ");
    print_dec(proc->pid);
    print_string(")\n");
    
    return proc;
}

void process_terminate(process_t* proc) {
    if (!proc) return;
    
    proc->state = PROCESS_TERMINATED;
    
    print_string("[PROC] Process terminated: ");
    print_string(proc->name);
    print_string(" (PID ");
    print_dec(proc->pid);
    print_string(")\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == proc) {
            process_table[i] = NULL;
            break;
        }
    }
    
    if (proc->kernel_stack) {
        kfree((void*)(proc->kernel_stack - KERNEL_STACK_SIZE));
    }
    kfree(proc);
}

void process_list(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t* proc = process_table[i];
        if (proc) {
            if (proc->pid < 10) print_char(' ');
            print_dec(proc->pid);
            print_string("  ");
            
            print_string(proc->name);
            for (int j = strlen(proc->name); j < 18; j++) {
                print_char(' ');
            }
            
            switch (proc->state) {
                case PROCESS_READY:     print_string("READY   "); break;
                case PROCESS_RUNNING:   print_string("RUNNING "); break;
                case PROCESS_BLOCKED:   print_string("BLOCKED "); break;
                case PROCESS_TERMINATED: print_string("DEAD    "); break;
            }
            
            print_dec(proc->cpu_time);
            print_string(" ticks\n");
        }
    }
}

process_t* process_get_current(void) {
    return current_process;
}
