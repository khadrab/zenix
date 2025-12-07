#include "syscall.h"
#include "../core/monitor.h"
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../drivers/timer/pit.h"

static int sys_exit(uint32_t status, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5) {
    (void)a2; (void)a3; (void)a4; (void)a5;
    
    print_string("[SYSCALL] exit(");
    print_dec(status);
    print_string(")\n");
    
    process_t* current = process_get_current();
    if (current) {
        process_terminate(current);
    }
    
    return 0;
}

static int sys_write(uint32_t fd, uint32_t buf, uint32_t count, uint32_t a4, uint32_t a5) {
    (void)a4; (void)a5;
    
    if (fd == 1 || fd == 2) {
        const char* str = (const char*)buf;
        for (uint32_t i = 0; i < count; i++) {
            print_char(str[i]);
        }
        return (int)count;
    }
    
    return -1;
}

static int sys_read(uint32_t fd, uint32_t buf, uint32_t count, uint32_t a4, uint32_t a5) {
    (void)fd; (void)buf; (void)count; (void)a4; (void)a5;
    return -1;
}

static int sys_getpid(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    
    process_t* current = process_get_current();
    return current ? (int)current->pid : -1;
}

static int sys_sleep(uint32_t ms, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5) {
    (void)a2; (void)a3; (void)a4; (void)a5;
    
    uint32_t start = timer_get_ticks();
    uint32_t ticks_to_wait = ms / 10;
    
    while ((timer_get_ticks() - start) < ticks_to_wait) {
        asm volatile("hlt");
    }
    
    return 0;
}

void syscall_handlers_init(void) {
    syscall_register(SYS_EXIT, sys_exit);
    syscall_register(SYS_WRITE, sys_write);
    syscall_register(SYS_READ, sys_read);
    syscall_register(SYS_GETPID, sys_getpid);
    syscall_register(SYS_SLEEP, sys_sleep);
}
