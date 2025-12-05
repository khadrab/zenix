#include "pit.h"
#include "../../hal/irq.h"
#include "../../../include/io.h"
#include "../../core/monitor.h"

static volatile uint32_t system_ticks = 0;

static void timer_callback(registers_t* regs) {
    (void)regs;
    system_ticks++;
    
    // اطبع نقطة كل ثانية (100 ticks)
    if (system_ticks % 100 == 0) {
        print_char('.');
        
        // اطبع رقم الثانية كل 10 ثواني
        if (system_ticks % 1000 == 0) {
            print_char('[');
            print_dec(system_ticks / 100);
            print_char('s');
            print_char(']');
            print_char('\n');
        }
    }
}

void timer_init(uint32_t frequency) {
    irq_register_handler(0, timer_callback);
    
    uint32_t divisor = 1193180 / frequency;
    
    outb(0x43, 0x36);
    
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    
    outb(0x40, low);
    outb(0x40, high);
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