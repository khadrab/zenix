#include "gui.h"
#include "../drivers/vga/vga.h"
#include "../drivers/mouse/mouse.h"
#include "../../lib/libc/string.h"

static window_t windows[MAX_WINDOWS];
static int window_count = 0;
static mouse_state_t last_mouse_state = {0, 0, 0};

void gui_init(void) {
    // Initialize all windows as invisible
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].visible = 0;
        windows[i].active = 0;
    }
    
    // Draw initial desktop
    gui_draw_desktop();
    
    // Create a welcome window
    int win = gui_create_window(60, 40, 200, 100, "Welcome to Zenix!", COLOR_LIGHT_GRAY);
    if (win >= 0) {
        windows[win].visible = 1;
        windows[win].active = 1;
    }
}

void gui_draw_desktop(void) {
    // Draw gradient background
    for (uint32_t y = 0; y < VGA_HEIGHT - 20; y++) {
        uint8_t color = COLOR_BLUE + (y / 30) % 6;
        vga_fill_rect(0, y, VGA_WIDTH, 1, color);
    }
    
    // Draw taskbar
    gui_draw_taskbar();
}

void gui_draw_taskbar(void) {
    // Taskbar at bottom
    vga_fill_rect(0, VGA_HEIGHT - 20, VGA_WIDTH, 20, COLOR_DARK_GRAY);
    vga_draw_rect(0, VGA_HEIGHT - 20, VGA_WIDTH, 20, COLOR_WHITE);
    
    // Start button
    vga_fill_rect(2, VGA_HEIGHT - 18, 40, 16, COLOR_LIGHT_GRAY);
    vga_draw_rect(2, VGA_HEIGHT - 18, 40, 16, COLOR_WHITE);
    vga_draw_string(6, VGA_HEIGHT - 14, "Zenix", COLOR_BLACK);
}

void gui_draw_cursor(void) {
    mouse_state_t mouse = mouse_get_state();
    
    // Erase old cursor (redraw what was there)
    if (last_mouse_state.x != mouse.x || last_mouse_state.y != mouse.y) {
        // Simple cursor: draw a small cross
        vga_draw_line(mouse.x - 3, mouse.y, mouse.x + 3, mouse.y, COLOR_WHITE);
        vga_draw_line(mouse.x, mouse.y - 3, mouse.x, mouse.y + 3, COLOR_WHITE);
        vga_draw_line(mouse.x - 2, mouse.y, mouse.x + 2, mouse.y, COLOR_BLACK);
        vga_draw_line(mouse.x, mouse.y - 2, mouse.x, mouse.y + 2, COLOR_BLACK);
    }
    
    last_mouse_state = mouse;
}

int gui_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                      const char* title, uint8_t color) {
    if (window_count >= MAX_WINDOWS) return -1;
    
    int id = window_count++;
    windows[id].x = x;
    windows[id].y = y;
    windows[id].width = width;
    windows[id].height = height;
    windows[id].color = color;
    windows[id].border_color = COLOR_WHITE;
    windows[id].visible = 1;
    windows[id].active = 0;
    
    // Copy title
    int i;
    for (i = 0; i < 63 && title[i] != '\0'; i++) {
        windows[id].title[i] = title[i];
    }
    windows[id].title[i] = '\0';
    
    return id;
}

void gui_draw_window(int id) {
    if (id < 0 || id >= window_count || !windows[id].visible) return;
    
    window_t* win = &windows[id];
    
    // Draw window background
    vga_fill_rect(win->x, win->y, win->width, win->height, win->color);
    
    // Draw title bar
    uint8_t title_color = win->active ? COLOR_BLUE : COLOR_DARK_GRAY;
    vga_fill_rect(win->x, win->y, win->width, 16, title_color);
    
    // Draw title text
    vga_draw_string(win->x + 4, win->y + 4, win->title, COLOR_WHITE);
    
    // Draw close button
    vga_fill_rect(win->x + win->width - 14, win->y + 2, 12, 12, COLOR_RED);
    vga_draw_string(win->x + win->width - 11, win->y + 4, "X", COLOR_WHITE);
    
    // Draw border
    vga_draw_rect(win->x, win->y, win->width, win->height, win->border_color);
}

void gui_handle_mouse_click(int32_t x, int32_t y) {
    // Check if clicked on window close button
    for (int i = window_count - 1; i >= 0; i--) {
        if (!windows[i].visible) continue;
        
        window_t* win = &windows[i];
        
        // Check close button
        if (x >= (int32_t)(win->x + win->width - 14) && 
            x <= (int32_t)(win->x + win->width - 2) &&
            y >= (int32_t)(win->y + 2) && 
            y <= (int32_t)(win->y + 14)) {
            gui_close_window(i);
            return;
        }
        
        // Check if clicked on title bar (for activation)
        if (x >= (int32_t)win->x && x <= (int32_t)(win->x + win->width) &&
            y >= (int32_t)win->y && y <= (int32_t)(win->y + 16)) {
            // Deactivate all windows
            for (int j = 0; j < window_count; j++) {
                windows[j].active = 0;
            }
            windows[i].active = 1;
            return;
        }
    }
}

void gui_close_window(int id) {
    if (id < 0 || id >= window_count) return;
    windows[id].visible = 0;
}

void gui_update(void) {
    // Redraw everything
    gui_draw_desktop();
    
    // Draw all visible windows
    for (int i = 0; i < window_count; i++) {
        if (windows[i].visible) {
            gui_draw_window(i);
        }
    }
    
    // Handle mouse
    mouse_state_t mouse = mouse_get_state();
    
    // Check for clicks
    static uint8_t last_buttons = 0;
    if ((mouse.buttons & MOUSE_LEFT_BTN) && !(last_buttons & MOUSE_LEFT_BTN)) {
        gui_handle_mouse_click(mouse.x, mouse.y);
        // Redraw after click
        gui_draw_desktop();
        for (int i = 0; i < window_count; i++) {
            if (windows[i].visible) {
                gui_draw_window(i);
            }
        }
    }
    last_buttons = mouse.buttons;
    
    // Draw cursor last
    gui_draw_cursor();
}