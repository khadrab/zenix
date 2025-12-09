// kernel/gui/gui.c - Modern Clean Design
#include "gui.h"
#include "color.h"
#include "../drivers/vga/vga.h"
#include "../drivers/mouse/mouse.h"
#include "../apps/app_manager.h"


extern void start_calculator_app(void);
extern void update_calculator_app(void);
extern int is_calculator_active(void);

static window_t windows[MAX_WINDOWS];
static int window_count = 0;
static mouse_state_t last_mouse = {0, 0, 0};
static int dragging_window = -1;
static int drag_offset_x = 0;
static int drag_offset_y = 0;

// Modern color scheme - Clean & Professional
#define BG_COLOR           0x09    // Soft blue background
#define TASKBAR_COLOR      0x00    // Black taskbar (modern)
#define WINDOW_BG          0x3F    // Pure white windows
#define ACCENT_COLOR       0x0C    // Red accent
#define TEXT_COLOR         0x00    // Black text
#define SHADOW_COLOR       0x08    // Subtle shadow
#define TASKBAR_HEIGHT 28
#define TASKBAR_Y (VGA_HEIGHT - TASKBAR_HEIGHT)

void gui_init(void) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].visible = 0;
        windows[i].active = 0;
    }
    
    // Clear with background
    vga_clear_screen(BG_COLOR);
    
    // Start calculator automatically
    start_calculator_app();

        // Initialize app manager
    app_manager_init();
}

void gui_draw_desktop(void) {
    // Simple solid color background - modern & clean
    vga_fill_rect(0, 0, vga_get_width(), vga_get_height() - 28, BG_COLOR);
}

void gui_draw_taskbar(void) {
    int32_t taskbar_y = vga_get_height() - 28;
    
    // Modern flat taskbar
    vga_fill_rect(0, taskbar_y, vga_get_width(), 24, TASKBAR_COLOR);
    
    // App launcher button (left)
    vga_fill_rect(4, taskbar_y + 4, 50, 16, ACCENT_COLOR);
    vga_draw_string(10, taskbar_y + 8, "Apps", 0x3F);
    
    // System info (right)
    vga_draw_string(vga_get_width() - 70, taskbar_y + 8, "12:34 PM", 0x3F);
}

void gui_draw_cursor(void) {
    mouse_state_t mouse = mouse_get_state();
    
    int mx = (mouse.x * (int)vga_get_width()) / 640;
    int my = (mouse.y * (int)vga_get_height()) / 480;
    
    if (mx < 0) mx = 0;
    if (mx >= (int)vga_get_width() - 10) mx = (int)vga_get_width() - 11;
    if (my < 0) my = 0;
    if (my >= (int)vga_get_height() - 14) my = (int)vga_get_height() - 15;
    
    // Modern minimal cursor
    // Shadow
    for (int i = 0; i <= 10; i++) {
        vga_set_pixel(mx + 1, my + i + 1, SHADOW_COLOR);
    }
    
    // White cursor
    for (int i = 0; i <= 10; i++) {
        vga_set_pixel(mx, my + i, 0x3F);
    }
    vga_draw_line(mx, my, mx + 6, my + 6, 0x3F);
    
    last_mouse = mouse;
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
    windows[id].border_color = SHADOW_COLOR;
    windows[id].visible = 1;
    windows[id].active = 0;
    
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
    
    // Subtle shadow
    vga_fill_rect(win->x + 3, win->y + 3, win->width, win->height, SHADOW_COLOR);
    
    // Window body
    vga_fill_rect(win->x, win->y, win->width, win->height, win->color);
    vga_draw_rect(win->x, win->y, win->width, win->height, SHADOW_COLOR);
    
    // Title bar (minimal)
    vga_fill_rect(win->x + 1, win->y + 1, win->width - 2, 24, 0x38);
    vga_draw_string(win->x + 10, win->y + 10, win->title, TEXT_COLOR);
    
    // Close button (modern - simple red square)
    vga_fill_rect(win->x + win->width - 20, win->y + 7, 10, 10, ACCENT_COLOR);
}

void gui_close_window(int id) {
    if (id < 0 || id >= window_count) return;
    windows[id].visible = 0;
}

void gui_handle_mouse_click(int32_t mx, int32_t my) {
    mx = (mx * vga_get_width()) / 640;
    my = (my * vga_get_height()) / 480;
    
    // Check taskbar launcher button
    int32_t taskbar_y = vga_get_height() - 28;
    if (my >= taskbar_y + 4 && my <= taskbar_y + 20) {
        if (mx >= 4 && mx <= 54) {
            // Launch calculator via app manager
            app_launch(APP_CALCULATOR);
            return;
        }
    }
    
    // Check windows
    for (int i = window_count - 1; i >= 0; i--) {
        if (!windows[i].visible) continue;
        
        window_t* win = &windows[i];
        
        // Check close button
        if (mx >= (int32_t)(win->x + win->width - 20) && 
            mx <= (int32_t)(win->x + win->width - 10) &&
            my >= (int32_t)(win->y + 7) && 
            my <= (int32_t)(win->y + 17)) {
            gui_close_window(i);
            return;
        }
        
        // Check title bar for dragging
        if (mx >= (int32_t)win->x && mx <= (int32_t)(win->x + win->width) &&
            my >= (int32_t)win->y && my <= (int32_t)(win->y + 24)) {
            for (int j = 0; j < window_count; j++) {
                windows[j].active = 0;
            }
            windows[i].active = 1;
            dragging_window = i;
            drag_offset_x = mx - win->x;
            drag_offset_y = my - win->y;
            return;
        }
    }
}

void gui_handle_mouse_release(void) {
    dragging_window = -1;
}

void gui_handle_mouse_drag(int32_t mx, int32_t my) {
    if (dragging_window < 0) return;
    
    mx = (mx * vga_get_width()) / 640;
    my = (my * vga_get_height()) / 480;
    
    window_t* win = &windows[dragging_window];
    
    int32_t new_x = mx - drag_offset_x;
    int32_t new_y = my - drag_offset_y;
    
    if (new_x < 0) new_x = 0;
    if (new_y < 0) new_y = 0;
    if (new_x + (int32_t)win->width > (int)vga_get_width()) 
        new_x = vga_get_width() - win->width;
    if (new_y + (int32_t)win->height > (int)vga_get_width() - 28) 
        new_y = vga_get_height() - 28 - win->height;
    
    win->x = (uint32_t)new_x;
    win->y = (uint32_t)new_y;
}

void gui_update(void) {
    mouse_state_t mouse = mouse_get_state();
    static uint8_t last_buttons = 0;
    
    // Handle mouse events
    if (mouse.buttons & MOUSE_LEFT_BTN) {
        if (!(last_buttons & MOUSE_LEFT_BTN)) {
            gui_handle_mouse_click(mouse.x, mouse.y);
        } else if (dragging_window >= 0) {
            gui_handle_mouse_drag(mouse.x, mouse.y);
        }
    } else {
        if (last_buttons & MOUSE_LEFT_BTN) {
            gui_handle_mouse_release();
        }
    }
    
    last_buttons = mouse.buttons;
    
      gui_draw_desktop();
    
    for (int i = 0; i < window_count; i++) {
        if (windows[i].visible) {
            gui_draw_window(i);
        }
    }
    
    // Update apps through app manager
    app_manager_update();
    
    // Calculator still draws itself
    if (is_calculator_active()) {
        update_calculator_app();
    }
    
    gui_draw_taskbar();
    gui_draw_cursor();
    vga_swap_buffers();
}