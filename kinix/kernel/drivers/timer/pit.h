#ifndef PIT_H
#define PIT_H

#include "../../../include/types.h"

void timer_init(uint32_t frequency);
uint32_t timer_get_ticks();
void timer_wait(uint32_t ticks);

#endif