#ifndef PIT_H
#define PIT_H

#include "../../../include/types.h"

extern volatile uint32_t system_ticks;

void timer_init(uint32_t frequency);
uint32_t timer_get_ticks();
void timer_wait(uint32_t ticks);

#endif