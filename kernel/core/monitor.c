#include "monitor.h"

static uint16_t* video_memory = (uint16_t*)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t attribute = 0x0F;

// Buffer للسطور القديمة (scrollback)
#define SCROLLBACK_LINES 100
static char scrollback_buffer[SCROLLBACK_LINES][80];
static int scrollback_count = 0;
static int scrollback_offset = 0;

void clear_screen(void) {
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i] = (attribute << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    scrollback_count = 0;
    scrollback_offset = 0;
}

static void save_line_to_scrollback(void) {
    if (scrollback_count < SCROLLBACK_LINES) {
        for (int i = 0; i < 80; i++) {
            scrollback_buffer[scrollback_count][i] = 
                (char)(video_memory[i] & 0xFF);
        }
        scrollback_count++;
    }
}

static void scroll(void) {
    if (cursor_y >= 25) {
        // حفظ السطر الأول قبل المسح
        save_line_to_scrollback();
        
        // تحريك كل الأسطر لأعلى
        for (int i = 0; i < 24 * 80; i++) {
            video_memory[i] = video_memory[i + 80];
        }
        
        // مسح السطر الأخير
        for (int i = 24 * 80; i < 25 * 80; i++) {
            video_memory[i] = (attribute << 8) | ' ';
        }
        cursor_y = 24;
    }
}

void scroll_up(void) {
    if (scrollback_offset < scrollback_count) {
        scrollback_offset++;
        // إعادة رسم الشاشة من الـ buffer
        // (تطبيق مبسط - يعرض رسالة فقط)
        print_string("[Scroll up - ");
        print_dec(scrollback_offset);
        print_string("/");
        print_dec(scrollback_count);
        print_string("]");
    }
}

void scroll_down(void) {
    if (scrollback_offset > 0) {
        scrollback_offset--;
        print_string("[Scroll down]");
    }
}

void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            video_memory[cursor_y * 80 + cursor_x] = (attribute << 8) | ' ';
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
    } else {
        video_memory[cursor_y * 80 + cursor_x] = (attribute << 8) | c;
        cursor_x++;
    }
    
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    
    scroll();
}

void print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

void print_dec(uint32_t n) {
    if (n == 0) {
        print_char('0');
        return;
    }
    
    char buffer[32];
    int i = 0;
    
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    while (i > 0) {
        print_char(buffer[--i]);
    }
}

void print_hex(uint32_t n) {
    const char* hex = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        print_char(hex[(n >> i) & 0xF]);
    }
}

void set_text_color(uint8_t foreground, uint8_t background) {
    attribute = (background << 4) | (foreground & 0x0F);
}
