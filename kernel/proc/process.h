#ifndef PROCESS_H
#define PROCESS_H

#include "../../include/types.h"

#define MAX_PROCESSES 32
#define KERNEL_STACK_SIZE 4096

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, eflags;
} registers_t;

typedef struct process {
    uint32_t pid;
    char name[32];
    process_state_t state;
    registers_t regs;
    uint32_t kernel_stack;
    uint32_t created_at;
    uint32_t cpu_time;
    struct process* next;
} process_t;

void process_init();
process_t* process_create(const char* name, void (*entry_point)());
void process_terminate(process_t* proc);
process_t* process_get_current();
void process_list();

extern process_t* current_process;
extern process_t* process_table[MAX_PROCESSES]; 

#endif