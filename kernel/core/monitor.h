#ifndef MONITOR_H
#define MONITOR_H

#include "../../include/types.h"

void clear_screen(void);
void print_char(char c);
void print_string(const char* str);
void print_dec(uint32_t n);
void print_hex(uint32_t n);
void set_text_color(uint8_t foreground, uint8_t background);
void scroll_up(void);
void scroll_down(void);

#endif
