#ifndef PIC_H
#define PIC_H
#include "../../include/types.h"
void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t mask1, uint8_t mask2);
void pic_disable(void);
uint16_t pic_get_isr(uint8_t irq);
#endif
