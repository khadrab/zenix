#ifndef SYSCALL_H
#define SYSCALL_H

#include "../../include/types.h"
#include "../hal/isr.h"

#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_GETPID  3
#define SYS_SLEEP   4

#define MAX_SYSCALLS 256

typedef int (*syscall_handler_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

void syscall_init(void);
void syscall_register(uint32_t num, syscall_handler_t handler);
void syscall_handlers_init(void);

#endif
