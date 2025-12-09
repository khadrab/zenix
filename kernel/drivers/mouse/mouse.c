#include "mouse.h"
#include <io.h>

// Suppress unused parameter warning
#define UNUSED(x) (void)(x)

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

extern void irq_register_handler(int irq, void (*handler)(registers_t*));

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

static mouse_state_t mouse_state = {160, 100, 0};
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];

// Add overflow detection
static uint32_t mouse_packet_count = 0;
static uint32_t mouse_error_count = 0;

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(MOUSE_STATUS) & MOUSE_BBIT) == 1) {
                return;
            }
        }
    } else {
        while (timeout--) {
            if ((inb(MOUSE_STATUS) & MOUSE_ABIT) == 0) {
                return;
            }
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(MOUSE_STATUS, MOUSE_WRITE);
    mouse_wait(1);
    outb(MOUSE_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_PORT);
}

static void mouse_handler_wrapper(registers_t* regs) {
    UNUSED(regs);
    
    uint8_t status = inb(MOUSE_STATUS);
    
    // Check if data is available from mouse
    if (!(status & MOUSE_BBIT)) return;
    if (!(status & MOUSE_F_BIT)) return;
    
    uint8_t data = inb(MOUSE_PORT);
    
    // Validate packet - first byte should have bit 3 set
    if (mouse_cycle == 0 && !(data & 0x08)) {
        // Invalid packet, reset cycle
        mouse_error_count++;
        return;
    }
    
    mouse_byte[mouse_cycle] = data;
    mouse_cycle++;
    
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        mouse_packet_count++;
        
        // Parse mouse packet
        mouse_state.buttons = mouse_byte[0] & 0x07;
        
        int32_t dx = mouse_byte[1];
        int32_t dy = mouse_byte[2];
        
        // Handle sign extension
        if (mouse_byte[0] & 0x10) dx |= 0xFFFFFF00;
        if (mouse_byte[0] & 0x20) dy |= 0xFFFFFF00;
        
        // Check for overflow
        if (mouse_byte[0] & 0x40) {
            // X overflow - ignore this packet
            return;
        }
        if (mouse_byte[0] & 0x80) {
            // Y overflow - ignore this packet
            return;
        }
        
        // Apply acceleration for smoother movement
        if (dx > 5 || dx < -5) dx *= 2;
        if (dy > 5 || dy < -5) dy *= 2;
        
        // Update position
        mouse_state.x += dx;
        mouse_state.y -= dy;  // Invert Y axis
        
        // Clamp to screen bounds
        if (mouse_state.x < 0) mouse_state.x = 0;
        if (mouse_state.x >= 640) mouse_state.x = 639;
        if (mouse_state.y < 0) mouse_state.y = 0;
        if (mouse_state.y >= 480) mouse_state.y = 479;
    }
}

void mouse_init(void) {
    uint8_t status;
    
    // Enable auxiliary device
    mouse_wait(1);
    outb(MOUSE_STATUS, 0xA8);
    
    // Enable interrupts
    mouse_wait(1);
    outb(MOUSE_STATUS, 0x20);
    mouse_wait(0);
    status = inb(MOUSE_PORT) | 2;
    mouse_wait(1);
    outb(MOUSE_STATUS, 0x60);
    mouse_wait(1);
    outb(MOUSE_PORT, status);
    
    // Use default settings
    mouse_write(0xF6);
    mouse_read();  // Acknowledge
    
    // Enable data reporting
    mouse_write(0xF4);
    mouse_read();  // Acknowledge
    
    // Set sample rate (optional, for better performance)
    mouse_write(0xF3);
    mouse_read();
    mouse_write(100);  // 100 samples/sec
    mouse_read();
    
    // Install IRQ handler
    irq_register_handler(12, mouse_handler_wrapper);
    
    // Reset error counters
    mouse_packet_count = 0;
    mouse_error_count = 0;
}

mouse_state_t mouse_get_state(void) {
    return mouse_state;
}

uint32_t mouse_get_packet_count(void) {
    return mouse_packet_count;
}

uint32_t mouse_get_error_count(void) {
    return mouse_error_count;
}