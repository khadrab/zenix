#include "keyboard.h"
#include "../../hal/irq.h"
#include "../../../include/io.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static char input_buffer[256];
static int buffer_head = 0;
static int buffer_tail = 0;
static int shift_pressed = 0;
static int caps_lock = 0;

static unsigned char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
    0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static unsigned char scancode_to_ascii_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
    0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void keyboard_handler(registers_t* regs) {
    (void)regs;
    
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    if (scancode == 0x2A || scancode == 0x36) {  // Left/Right Shift pressed
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {  // Left/Right Shift released
        shift_pressed = 0;
        return;
    }
    if (scancode == 0x3A) {  // Caps Lock
        caps_lock = !caps_lock;
        return;
    }
    
    if (scancode & 0x80) {
        return;
    }
    
    char ascii = 0;
    if (shift_pressed || caps_lock) {
        ascii = scancode_to_ascii_shift[scancode];
    } else {
        ascii = scancode_to_ascii[scancode];
    }
    
    if (ascii != 0) {
        int next_head = (buffer_head + 1) % 256;
        if (next_head != buffer_tail) {
            input_buffer[buffer_head] = ascii;
            buffer_head = next_head;
        }
    }
}

void keyboard_init(void) {
    buffer_head = 0;
    buffer_tail = 0;
    shift_pressed = 0;
    caps_lock = 0;
    
    irq_register_handler(1, keyboard_handler);
}

char keyboard_getchar(void) {
    if (buffer_head == buffer_tail) {
        return 0;
    }
    
    char c = input_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % 256;
    return c;
}
