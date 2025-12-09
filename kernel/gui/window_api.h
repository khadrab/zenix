// kernel/gui/window_api.h - Application Window API
#ifndef WINDOW_API_H
#define WINDOW_API_H

#include "../../include/types.h"
#include "color.h"

// Window handle (opaque to applications)
typedef int window_handle_t;

// Window styles
typedef enum {
    WS_NORMAL       = 0x00,  // Normal window with title bar
    WS_BORDERLESS   = 0x01,  // No border or title bar
    WS_DIALOG       = 0x02,  // Dialog-style window
    WS_TOOL         = 0x04,  // Tool window (smaller title bar)
    WS_TRANSPARENT  = 0x08   // Transparent background support
} window_style_t;

// Window events
typedef enum {
    WE_NONE = 0,
    WE_CLOSE,           // User clicked close button
    WE_MINIMIZE,        // User clicked minimize
    WE_MAXIMIZE,        // User clicked maximize
    WE_FOCUS_GAINED,    // Window gained focus
    WE_FOCUS_LOST,      // Window lost focus
    WE_MOUSE_MOVE,      // Mouse moved in window
    WE_MOUSE_DOWN,      // Mouse button pressed
    WE_MOUSE_UP,        // Mouse button released
    WE_KEY_DOWN,        // Key pressed
    WE_KEY_UP,          // Key released
    WE_PAINT            // Window needs to be repainted
} window_event_type_t;

// Window event structure
typedef struct {
    window_event_type_t type;
    window_handle_t window;
    
    union {
        struct {
            int32_t x, y;
            uint8_t buttons;
        } mouse;
        
        struct {
            uint8_t keycode;
            uint8_t modifiers;
        } key;
    } data;
} window_event_t;

// Drawing context for a window
typedef struct {
    window_handle_t handle;
    uint32_t width;
    uint32_t height;
    uint8_t* framebuffer;  // Direct pixel access
} window_context_t;

// ============================================================================
// WINDOW MANAGEMENT API
// ============================================================================

// Create a new window
// Returns: window handle or -1 on error
window_handle_t window_create(const char* title, 
                              uint32_t x, uint32_t y,
                              uint32_t width, uint32_t height,
                              window_style_t style);

// Destroy a window
void window_destroy(window_handle_t handle);

// Show/hide window
void window_show(window_handle_t handle);
void window_hide(window_handle_t handle);

// Set window position
void window_set_position(window_handle_t handle, uint32_t x, uint32_t y);

// Set window size
void window_set_size(window_handle_t handle, uint32_t width, uint32_t height);

// Set window title
void window_set_title(window_handle_t handle, const char* title);

// Bring window to front
void window_focus(window_handle_t handle);

// ============================================================================
// DRAWING API
// ============================================================================

// Get drawing context for window
window_context_t* window_get_context(window_handle_t handle);

// Clear window with color
void window_clear(window_context_t* ctx, rgba_color_t color);

// Draw pixel with anti-aliasing support
void window_draw_pixel(window_context_t* ctx, int32_t x, int32_t y, rgba_color_t color);

// Draw rectangle
void window_draw_rect(window_context_t* ctx, 
                     int32_t x, int32_t y,
                     uint32_t width, uint32_t height,
                     rgba_color_t color);

// Fill rectangle
void window_fill_rect(window_context_t* ctx,
                     int32_t x, int32_t y,
                     uint32_t width, uint32_t height,
                     rgba_color_t color);

// Draw text with anti-aliasing
void window_draw_text(window_context_t* ctx,
                     int32_t x, int32_t y,
                     const char* text,
                     rgba_color_t color);

// Draw text with background
void window_draw_text_bg(window_context_t* ctx,
                        int32_t x, int32_t y,
                        const char* text,
                        rgba_color_t fg_color,
                        rgba_color_t bg_color);

// Draw line with anti-aliasing
void window_draw_line(window_context_t* ctx,
                     int32_t x1, int32_t y1,
                     int32_t x2, int32_t y2,
                     rgba_color_t color);

// Draw circle
void window_draw_circle(window_context_t* ctx,
                       int32_t cx, int32_t cy,
                       uint32_t radius,
                       rgba_color_t color);

// Fill circle
void window_fill_circle(window_context_t* ctx,
                       int32_t cx, int32_t cy,
                       uint32_t radius,
                       rgba_color_t color);

// Refresh window (copy to screen)
void window_refresh(window_handle_t handle);

// ============================================================================
// EVENT HANDLING API
// ============================================================================

// Poll for next event (non-blocking)
// Returns: 1 if event available, 0 if no events
int window_poll_event(window_event_t* event);

// Wait for next event (blocking)
void window_wait_event(window_event_t* event);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Check if window is visible
int window_is_visible(window_handle_t handle);

// Check if window has focus
int window_has_focus(window_handle_t handle);

// Get window position
void window_get_position(window_handle_t handle, uint32_t* x, uint32_t* y);

// Get window size
void window_get_size(window_handle_t handle, uint32_t* width, uint32_t* height);

#endif // WINDOW_API_H