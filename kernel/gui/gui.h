#ifndef GUI_H
#define GUI_H

#include "../../include/types.h"

#define MAX_WINDOWS 16

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint8_t color;
    uint8_t border_color;
    char title[64];
    uint8_t visible;
    uint8_t active;
} window_t;

void gui_init(void);
void gui_update(void);
void gui_draw_desktop(void);
void gui_draw_taskbar(void);
void gui_draw_cursor(void);

int gui_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, 
                      const char* title, uint8_t color);
void gui_close_window(int id);
void gui_draw_window(int id);
void gui_handle_mouse_click(int32_t x, int32_t y);

#endif