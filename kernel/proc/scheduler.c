#include "scheduler.h"
#include "process.h"
#include "../core/monitor.h"
#include "../drivers/timer/pit.h"
#include "../hal/irq.h"

// External references from process.c
extern process_t* current_process;
extern process_t* process_table[];
extern uint32_t next_pid;

#define MAX_PROCESSES 64

static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;
static uint32_t time_slice = 10;  // 100ms at 100Hz

// Add process to ready queue
static void enqueue_process(process_t* proc) {
    if (!proc) return;
    
    proc->next = NULL;
    
    if (!ready_queue_head) {
        ready_queue_head = proc;
        ready_queue_tail = proc;
    } else {
        ready_queue_tail->next = proc;
        ready_queue_tail = proc;
    }
}

// Remove process from ready queue
static process_t* dequeue_process(void) {
    if (!ready_queue_head) {
        return NULL;
    }
    
    process_t* proc = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    
    if (!ready_queue_head) {
        ready_queue_tail = NULL;
    }
    
    proc->next = NULL;
    return proc;
}

// Context switch (implemented in switch.asm)
extern void switch_context(registers_t* old, registers_t* new_ctx);

// Schedule next process
void schedule(void) {
    if (!current_process) return;
    
    // Save current process state
    if (current_process->state == PROCESS_RUNNING) {
        current_process->state = PROCESS_READY;
        enqueue_process(current_process);
    }
    
    // Find idle process
    process_t* idle = NULL;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == 0) {
            idle = process_table[i];
            break;
        }
    }
    
    // Get next process or idle
    process_t* next = dequeue_process();
    if (!next) {
        next = idle;
    }
    
    if (!next) return;
    
    // Switch to next process
    next->state = PROCESS_RUNNING;
    process_t* prev = current_process;
    current_process = next;
    
    // Perform context switch
    if (prev != next) {
        switch_context(&prev->regs, &next->regs);
    }
}

// Timer interrupt handler
void scheduler_tick(registers_t* regs) {
    static uint32_t ticks = 0;
    
    if (!current_process) return;
    
    // Update CPU time
    current_process->cpu_time++;
    
    // Check if time slice expired
    ticks++;
    if (ticks >= time_slice) {
        ticks = 0;
        
        // Save current register state
        current_process->regs = *regs;
        
        // Schedule next process
        schedule();
    }
}

// Initialize scheduler
void scheduler_init(void) {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    
    // Register timer interrupt handler
    irq_register_handler(0, scheduler_tick);
}
