#ifndef GDT_H
#define GDT_H

#include "../../include/types.h"

void gdt_init(void);
void set_kernel_stack(uint32_t stack);

#endif