#include "keyboard.h"
#include "../../hal/irq.h"
#include "../../../include/io.h"
#include "../../core/monitor.h"

// US QWERTY Scancode Set 1
static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

#define BUFFER_SIZE 256
static char input_buffer[BUFFER_SIZE];
static uint32_t buffer_read = 0;
static uint32_t buffer_write = 0;

static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;

static void keyboard_handler(registers_t* regs) {
    (void)regs;
    
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle special keys
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }
    
    // Only handle key press (not release)
    if (scancode & 0x80) {
        return;
    }
    
    // Convert scancode to ASCII
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
            if (caps_lock && ascii >= 'a' && ascii <= 'z') {
                ascii -= 32;
            }
        }
    }
    
    if (ascii != 0) {
        // Add to buffer
        input_buffer[buffer_write] = ascii;
        buffer_write = (buffer_write + 1) % BUFFER_SIZE;
        
        // Echo character
        if (ascii == '\b') {
            print_char('\b');
        } else if (ascii == '\n') {
            print_char('\n');
        } else {
            print_char(ascii);
        }
    }
}

void keyboard_init() {
    irq_register_handler(1, keyboard_handler);
}

char keyboard_getchar() {
    if (buffer_read == buffer_write) {
        return 0;
    }
    
    char c = input_buffer[buffer_read];
    buffer_read = (buffer_read + 1) % BUFFER_SIZE;
    return c;
}

void keyboard_wait_for_key() {
    while (buffer_read == buffer_write) {
        asm volatile("hlt");
    }
}