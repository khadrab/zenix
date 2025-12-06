#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

void scheduler_init();
void scheduler_add(process_t* proc);
void scheduler_remove(process_t* proc);
void schedule();
void yield();

#endif