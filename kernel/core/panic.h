// kernel/core/panic.h
#ifndef PANIC_H
#define PANIC_H

#include "../../include/types.h"
#include "../hal/isr.h"

// Panic with message
void kernel_panic(const char* message, const char* file, uint32_t line);

// Exception handlers
void division_error_handler(registers_t* regs);
void general_protection_fault_handler(registers_t* regs);
void double_fault_handler(registers_t* regs);

// Stack trace
void print_stack_trace(uint32_t* ebp, int max_frames);

// Register dump
void dump_registers(registers_t* regs);

// Macro for easy panic
#define PANIC(msg) kernel_panic(msg, __FILE__, __LINE__)

// Assert macro
#define ASSERT(condition) \
    if (!(condition)) { \
        kernel_panic("Assertion failed: " #condition, __FILE__, __LINE__); \
    }

#endif // PANIC_H
