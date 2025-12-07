#include "mouse.h"
#include <io.h>

// Include ISR for registers_t type
typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// Forward declare IRQ function - check your kernel/hal/irq.h for exact name
// Common names: irq_install_handler, irq_register_handler, register_interrupt_handler
extern void irq_register_handler(int irq, void (*handler)(registers_t*));

// Suppress unused parameter warning
#define UNUSED(x) (void)(x)

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

static void mouse_handler_wrapper(registers_t* regs) {
    UNUSED(regs);
    
    uint8_t status = inb(MOUSE_STATUS);
    
    if (!(status & MOUSE_BBIT)) return;
    if (!(status & MOUSE_F_BIT)) return;
    
    mouse_byte[mouse_cycle] = inb(MOUSE_PORT);
    mouse_cycle++;
    
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        
        // Parse mouse packet
        mouse_state.buttons = mouse_byte[0] & 0x07;
        
        int32_t dx = mouse_byte[1];
        int32_t dy = mouse_byte[2];
        
        // Handle sign extension
        if (mouse_byte[0] & 0x10) dx |= 0xFFFFFF00;
        if (mouse_byte[0] & 0x20) dy |= 0xFFFFFF00;
        
        // Update position
        mouse_state.x += dx;
        mouse_state.y -= dy;  // Invert Y axis
        
        // Clamp to screen bounds
        if (mouse_state.x < 0) mouse_state.x = 0;
        if (mouse_state.x >= 320) mouse_state.x = 319;
        if (mouse_state.y < 0) mouse_state.y = 0;
        if (mouse_state.y >= 200) mouse_state.y = 199;
    }
}

void mouse_init(void) {
    // Enable auxiliary device
    mouse_wait(1);
    outb(MOUSE_STATUS, 0xA8);
    
    // Enable interrupts
    mouse_wait(1);
    outb(MOUSE_STATUS, 0x20);
    mouse_wait(0);
    uint8_t status = inb(MOUSE_PORT) | 2;
    mouse_wait(1);
    outb(MOUSE_STATUS, 0x60);
    mouse_wait(1);
    outb(MOUSE_PORT, status);
    
    // Use default settings
    mouse_write(0xF6);
    mouse_wait(0);
    inb(MOUSE_PORT);
    
    // Enable data reporting
    mouse_write(0xF4);
    mouse_wait(0);
    inb(MOUSE_PORT);
    
    // Install IRQ handler
    irq_register_handler(12, mouse_handler_wrapper);
}

mouse_state_t mouse_get_state(void) {
    return mouse_state;
}