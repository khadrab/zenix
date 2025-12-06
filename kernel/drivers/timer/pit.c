#include "pit.h"
#include "../../hal/irq.h"
#include "../../../include/io.h"
#include "../../core/monitor.h"

volatile uint32_t system_ticks = 0;

// Forward declaration
extern void schedule();

static void timer_callback(registers_t* regs) {
    (void)regs;
    system_ticks++;
    
    // Call scheduler every 10 ticks (100ms)
    if (system_ticks % 10 == 0) {
        schedule();
    }
}

void timer_init(uint32_t frequency) {
    irq_register_handler(0, timer_callback);
    
    uint32_t divisor = 1193180 / frequency;
    
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks() {
    return system_ticks;
}

void timer_wait(uint32_t ticks) {
    uint32_t end = system_ticks + ticks;
    while(system_ticks < end) {
        asm volatile("hlt");
    }
}