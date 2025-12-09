#ifndef MOUSE_H
#define MOUSE_H

#include "../../../include/types.h"

typedef struct {
    int32_t x;
    int32_t y;
    uint8_t buttons;
} mouse_state_t;

void mouse_init(void);
mouse_state_t mouse_get_state(void);
void mouse_handler(void);

uint32_t mouse_get_packet_count(void);
uint32_t mouse_get_error_count(void);

#define MOUSE_LEFT_BTN   0x01
#define MOUSE_RIGHT_BTN  0x02
#define MOUSE_MIDDLE_BTN 0x04

#endif