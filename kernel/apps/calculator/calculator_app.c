// kernel/apps/calculator/calculator_app.c
// Modern calculator with RGBA colors

#include "../../gui/color.h"
#include "../../drivers/vga/vga.h"
#include "../../drivers/mouse/mouse.h"

// Calculator state
typedef struct {
    uint32_t x, y;
    uint32_t width, height;
    char display[32];
    int display_len;
    int running;
    int dragging;          // NEW: for window dragging
    int drag_offset_x;     // NEW
    int drag_offset_y;     // NEW
    double value;
    double stored_value;
    char operation;
    int new_number;
} calculator_t;

// Button definition
typedef struct {
    int32_t x, y;
    uint32_t w, h;
    char label[4];
    rgba_color_t color;
} button_t;

// Calculator buttons with beautiful Material Design colors
static button_t buttons[] = {
    // Row 1: Numbers 7-9 and divide
    {10, 60, 40, 30, "7", COLOR_MD_GRAY_100},
    {55, 60, 40, 30, "8", COLOR_MD_GRAY_100},
    {100, 60, 40, 30, "9", COLOR_MD_GRAY_100},
    {145, 60, 40, 30, "/", COLOR_MD_ORANGE},
    
    // Row 2: Numbers 4-6 and multiply
    {10, 95, 40, 30, "4", COLOR_MD_GRAY_100},
    {55, 95, 40, 30, "5", COLOR_MD_GRAY_100},
    {100, 95, 40, 30, "6", COLOR_MD_GRAY_100},
    {145, 95, 40, 30, "*", COLOR_MD_ORANGE},
    
    // Row 3: Numbers 1-3 and minus
    {10, 130, 40, 30, "1", COLOR_MD_GRAY_100},
    {55, 130, 40, 30, "2", COLOR_MD_GRAY_100},
    {100, 130, 40, 30, "3", COLOR_MD_GRAY_100},
    {145, 130, 40, 30, "-", COLOR_MD_ORANGE},
    
    // Row 4: 0, decimal, plus
    {10, 165, 85, 30, "0", COLOR_MD_GRAY_100},
    {100, 165, 40, 30, ".", COLOR_MD_GRAY_100},
    {145, 165, 40, 30, "+", COLOR_MD_ORANGE},
    
    // Bottom row: Clear and equals
    {10, 200, 85, 30, "C", COLOR_MD_RED},
    {100, 200, 85, 30, "=", COLOR_MD_GREEN},
};

static const int num_buttons = sizeof(buttons) / sizeof(button_t);

// Draw single button with shadow
void calculator_draw_button(calculator_t* calc, button_t* btn) {
    // Shadow (darker version)
    rgba_color_t shadow = rgba_darken(btn->color, 0.6f);
    uint8_t shadow_vga = rgba_to_vga(shadow);
    vga_fill_rect(calc->x + btn->x + 2, calc->y + btn->y + 2, 
                  btn->w, btn->h, shadow_vga);
    
    // Button background
    uint8_t bg_vga = rgba_to_vga(btn->color);
    vga_fill_rect(calc->x + btn->x, calc->y + btn->y, 
                  btn->w, btn->h, bg_vga);
    
    // Border
    uint8_t border_vga = rgba_to_vga(COLOR_MD_GRAY_400);
    vga_draw_rect(calc->x + btn->x, calc->y + btn->y, 
                  btn->w, btn->h, border_vga);
    
    // Center text
    uint32_t text_w = 0;
    const char* p = btn->label;
    while (*p++) text_w += 8;
    
    int32_t text_x = calc->x + btn->x + (btn->w - text_w) / 2;
    int32_t text_y = calc->y + btn->y + 10;
    
    uint8_t text_vga = rgba_to_vga(COLOR_MD_GRAY_900);
    vga_draw_string(text_x, text_y, btn->label, text_vga);
}

// Draw display
void calculator_draw_display(calculator_t* calc) {
    // Display background (white)
    uint8_t bg = rgba_to_vga(COLOR_WHITE);
    vga_fill_rect(calc->x + 10, calc->y + 10, 175, 40, bg);
    
    // Display border
    uint8_t border = rgba_to_vga(COLOR_MD_GRAY_300);
    vga_draw_rect(calc->x + 10, calc->y + 10, 175, 40, border);
    
    // Title bar for dragging (subtle)
    uint8_t title_bg = rgba_to_vga(COLOR_MD_GRAY_100);
    vga_fill_rect(calc->x, calc->y, calc->width, 8, title_bg);
    
    // Close button (X)
    uint8_t close_color = rgba_to_vga(COLOR_MD_RED);
    vga_fill_rect(calc->x + calc->width - 10, calc->y + 1, 8, 6, close_color);
    
    // Display text
    uint8_t text = rgba_to_vga(COLOR_BLACK);
    vga_draw_string(calc->x + 15, calc->y + 25, calc->display, text);
}

// Redraw calculator
void calculator_draw(calculator_t* calc) {
    // Clear window background
    uint8_t bg = rgba_to_vga(COLOR_MD_GRAY_50);
    vga_fill_rect(calc->x, calc->y, calc->width, calc->height, bg);
    
    // Draw display
    calculator_draw_display(calc);
    
    // Draw all buttons
    for (int i = 0; i < num_buttons; i++) {
        calculator_draw_button(calc, &buttons[i]);
    }
}

// Simple string to double conversion
static double simple_atof(const char* str) {
    double result = 0.0;
    double fraction = 0.0;
    int divisor = 1;
    int in_fraction = 0;
    int negative = 0;
    
    if (*str == '-') {
        negative = 1;
        str++;
    }
    
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            if (in_fraction) {
                divisor *= 10;
                fraction = fraction * 10.0 + (*str - '0');
            } else {
                result = result * 10.0 + (*str - '0');
            }
        } else if (*str == '.') {
            in_fraction = 1;
        }
        str++;
    }
    
    result += fraction / divisor;
    return negative ? -result : result;
}

// Simple double to string conversion
static void simple_ftoa(double value, char* buffer, int precision) {
    int int_part = (int)value;
    double frac_part = value - int_part;
    
    if (int_part < 0) {
        *buffer++ = '-';
        int_part = -int_part;
        frac_part = -frac_part;
    }
    
    char temp[32];
    int i = 0;
    if (int_part == 0) {
        temp[i++] = '0';
    } else {
        while (int_part > 0) {
            temp[i++] = '0' + (int_part % 10);
            int_part /= 10;
        }
    }
    
    while (i > 0) {
        *buffer++ = temp[--i];
    }
    
    if (precision > 0 && frac_part > 0.0) {
        *buffer++ = '.';
        for (int j = 0; j < precision; j++) {
            frac_part *= 10;
            int digit = (int)frac_part;
            *buffer++ = '0' + digit;
            frac_part -= digit;
        }
    }
    
    *buffer = '\0';
}

// Execute operation
void calculator_execute(calculator_t* calc) {
    calc->value = simple_atof(calc->display);
    
    if (calc->operation != 0) {
        switch (calc->operation) {
            case '+': calc->stored_value += calc->value; break;
            case '-': calc->stored_value -= calc->value; break;
            case '*': calc->stored_value *= calc->value; break;
            case '/':
                if (calc->value != 0.0) {
                    calc->stored_value /= calc->value;
                }
                break;
        }
        
        simple_ftoa(calc->stored_value, calc->display, 2);
        calc->display_len = 0;
        while (calc->display[calc->display_len]) calc->display_len++;
    } else {
        calc->stored_value = calc->value;
    }
}

// Handle button click
void calculator_handle_button(calculator_t* calc, int button_id) {
    button_t* btn = &buttons[button_id];
    char ch = btn->label[0];
    
    if (ch == 'C') {
        calc->display[0] = '0';
        calc->display[1] = '\0';
        calc->display_len = 1;
        calc->value = 0;
        calc->stored_value = 0;
        calc->operation = 0;
        calc->new_number = 1;
    } else if (ch >= '0' && ch <= '9') {
        if (calc->new_number) {
            calc->display[0] = ch;
            calc->display[1] = '\0';
            calc->display_len = 1;
            calc->new_number = 0;
        } else if (calc->display_len < 15) {
            if (calc->display[0] == '0' && calc->display_len == 1) {
                calc->display[0] = ch;
            } else {
                calc->display[calc->display_len++] = ch;
                calc->display[calc->display_len] = '\0';
            }
        }
    } else if (ch == '.') {
        if (calc->new_number) {
            calc->display[0] = '0';
            calc->display[1] = '.';
            calc->display[2] = '\0';
            calc->display_len = 2;
            calc->new_number = 0;
        } else {
            int has_decimal = 0;
            for (int i = 0; i < calc->display_len; i++) {
                if (calc->display[i] == '.') {
                    has_decimal = 1;
                    break;
                }
            }
            if (!has_decimal && calc->display_len < 15) {
                calc->display[calc->display_len++] = '.';
                calc->display[calc->display_len] = '\0';
            }
        }
    } else if (ch == '=') {
        calculator_execute(calc);
        calc->operation = 0;
        calc->new_number = 1;
    } else {
        if (!calc->new_number) {
            calculator_execute(calc);
        }
        calc->operation = ch;
        calc->new_number = 1;
    }
    
    calculator_draw(calc);
}

// Check if mouse is over button
int calculator_check_button(calculator_t* calc, int32_t mx, int32_t my) {
    mx -= calc->x;
    my -= calc->y;
    
    for (int i = 0; i < num_buttons; i++) {
        button_t* btn = &buttons[i];
        if (mx >= btn->x && mx < btn->x + (int32_t)btn->w &&
            my >= btn->y && my < btn->y + (int32_t)btn->h) {
            return i;
        }
    }
    return -1;
}

// Initialize calculator
void calculator_init(calculator_t* calc, uint32_t x, uint32_t y) {
    calc->x = x;
    calc->y = y;
    calc->width = 195;
    calc->height = 240;
    
    calc->display[0] = '0';
    calc->display[1] = '\0';
    calc->display_len = 1;
    calc->running = 1;
    calc->dragging = 0;      // NEW
    calc->drag_offset_x = 0; // NEW
    calc->drag_offset_y = 0; // NEW
    calc->value = 0;
    calc->stored_value = 0;
    calc->operation = 0;
    calc->new_number = 1;
}

// Global calculator instance
static calculator_t g_calculator;
static int g_calculator_active = 0;

// Forward declaration for close
static void calculator_close(void);

// Main calculator loop
void calculator_update(calculator_t* calc) {
    if (!calc->running) return;
    
    mouse_state_t mouse = mouse_get_state();
    static uint8_t last_buttons = 0;
    
    int32_t mx = (mouse.x * vga_get_width()) / 640;
    int32_t my = (mouse.y * vga_get_height()) / 480;
    
    // Handle mouse click
    if ((mouse.buttons & MOUSE_LEFT_BTN) && !(last_buttons & MOUSE_LEFT_BTN)) {
        // Check close button
        if (mx >= (int32_t)(calc->x + calc->width - 10) && 
            mx <= (int32_t)(calc->x + calc->width - 2) &&
            my >= (int32_t)calc->y + 1 && 
            my <= (int32_t)calc->y + 7) {
            calculator_close();
            last_buttons = mouse.buttons;
            return;
        }
        
        // Check title bar for dragging
        if (mx >= (int32_t)calc->x && mx <= (int32_t)(calc->x + calc->width) &&
            my >= (int32_t)calc->y && my <= (int32_t)(calc->y + 8)) {
            calc->dragging = 1;
            calc->drag_offset_x = mx - calc->x;
            calc->drag_offset_y = my - calc->y;
        } else {
            // Check button clicks
            int btn_id = calculator_check_button(calc, mx, my);
            if (btn_id >= 0) {
                calculator_handle_button(calc, btn_id);
            }
        }
    }
    
    // Handle dragging
    if (calc->dragging && (mouse.buttons & MOUSE_LEFT_BTN)) {
        int32_t new_x = mx - calc->drag_offset_x;
        int32_t new_y = my - calc->drag_offset_y;
        
        // Keep on screen
        if (new_x < 0) new_x = 0;
        if (new_y < 0) new_y = 0;
        if (new_x + (int32_t)calc->width > (int)vga_get_width()) new_x = (int)vga_get_width() - calc->width;
        if (new_y + (int32_t)calc->height > (int)vga_get_height() - 24) new_y = (int)vga_get_height() - 24 - calc->height; // 200 - 24 taskbar
        
        calc->x = new_x;
        calc->y = new_y;
    }
    
    // Release drag
    if (!(mouse.buttons & MOUSE_LEFT_BTN) && (last_buttons & MOUSE_LEFT_BTN)) {
        calc->dragging = 0;
    }
    
    last_buttons = mouse.buttons;
    
    // IMPORTANT: Redraw calculator every frame!
    calculator_draw(calc);
}

// Close calculator
static void calculator_close(void) {
    g_calculator.running = 0;
    g_calculator_active = 0;
}

// Public API
void start_calculator_app(void) {
    if (!g_calculator_active) {
        calculator_init(&g_calculator, 60, 30);
        g_calculator_active = 1;
        // Will be drawn in first update call
    }
}

void update_calculator_app(void) {
    if (g_calculator_active) {
        calculator_update(&g_calculator);
    }
}

int is_calculator_active(void) {
    return g_calculator_active;
}