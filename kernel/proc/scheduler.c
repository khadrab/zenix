#include "scheduler.h"
#include "../core/monitor.h"

static process_t* ready_queue_head = 0;
static process_t* ready_queue_tail = 0;

extern void switch_context(uint32_t* old_esp, uint32_t new_esp);

void scheduler_init() {
    print_string("[SCHED] Initializing scheduler...\n");
    ready_queue_head = 0;
    ready_queue_tail = 0;
}

void scheduler_add(process_t* proc) {
    if (!proc) return;
    
    proc->state = PROCESS_READY;
    proc->next = 0;
    
    if (!ready_queue_head) {
        ready_queue_head = proc;
        ready_queue_tail = proc;
    } else {
        ready_queue_tail->next = proc;
        ready_queue_tail = proc;
    }
}

void scheduler_remove(process_t* proc) {
    if (!proc || !ready_queue_head) return;
    
    if (ready_queue_head == proc) {
        ready_queue_head = proc->next;
        if (!ready_queue_head) {
            ready_queue_tail = 0;
        }
        return;
    }
    
    process_t* current = ready_queue_head;
    while (current->next) {
        if (current->next == proc) {
            current->next = proc->next;
            if (ready_queue_tail == proc) {
                ready_queue_tail = current;
            }
            return;
        }
        current = current->next;
    }
}

void schedule() {
    if (!current_process) return;
    
    // Save current process state
    if (current_process->state == PROCESS_RUNNING) {
        current_process->state = PROCESS_READY;
        scheduler_add(current_process);
    }
    
    // Get next process
    process_t* next = ready_queue_head;
    if (!next) {
        // No process to run, return to idle
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i] && process_table[i]->pid == 0) {  // ← امسح extern من هنا
                next = process_table[i];
                break;
            }
        }
    } else {
        scheduler_remove(next);
    }
    
    if (!next || next == current_process) {
        return;
    }
    
    // Update CPU time
    current_process->cpu_time++;
    
    // Switch to next process
    process_t* old = current_process;
    current_process = next;
    next->state = PROCESS_RUNNING;
    
    // Context switch
    switch_context(&old->regs.esp, next->regs.esp);
}

void yield() {
    schedule();
}