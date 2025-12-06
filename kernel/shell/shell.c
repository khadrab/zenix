#include "shell.h"
#include "../core/monitor.h"
#include "../drivers/keyboard/keyboard.h"
#include "../../lib/libc/string.h"
#include "../drivers/timer/pit.h"
#include "../proc/process.h"

#define COMMAND_BUFFER_SIZE 256

static char command_buffer[COMMAND_BUFFER_SIZE];
static uint32_t command_pos = 0;

extern volatile uint32_t system_ticks;

extern uint32_t pmm_get_total_memory();
extern uint32_t pmm_get_used_memory();
extern uint32_t pmm_get_free_memory();
extern uint32_t heap_get_used();
extern uint32_t heap_get_free();


extern int paging_is_enabled();

static void shell_vmstat() {
    print_string("Virtual Memory Status:\n");
    print_string("======================\n");
    
    if (paging_is_enabled()) {
        print_string("Status: ENABLED\n");
        print_string("Page Size: 4096 bytes\n");
        print_string("Identity Mapped: 32 MB\n");
    } else {
        print_string("Status: DISABLED\n");
    }
    
    print_string("\nPhysical Memory:\n");
    extern uint32_t pmm_get_total_memory();
    extern uint32_t pmm_get_used_memory();
    extern uint32_t pmm_get_free_memory();
    
    print_string("  Total: ");
    print_dec(pmm_get_total_memory() / 1024 / 1024);
    print_string(" MB\n");
    
    print_string("  Used:  ");
    print_dec(pmm_get_used_memory() / 1024 / 1024);
    print_string(" MB\n");
    
    print_string("  Free:  ");
    print_dec(pmm_get_free_memory() / 1024 / 1024);
    print_string(" MB\n");
}

static void shell_clear() {
    clear_screen();
    print_string("ZShell\n");
    print_string("================\n\n");
}

static void shell_help() {
    print_string("Available commands:\n");
    print_string("  help     - Show this help\n");
    print_string("  clear    - Clear screen\n");
    print_string("  uptime   - Show system uptime\n");
    print_string("  meminfo  - Show memory info\n");
    print_string("  vmstat   - Virtual memory status\n");
    print_string("  ps       - List processes\n");
    print_string("  spawn    - Spawn test processes\n");
    print_string("  echo     - Echo text\n");
    print_string("  about    - About Zenix\n");
}

static void shell_uptime() {
    uint32_t seconds = system_ticks / 100;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    print_string("Uptime: ");
    print_dec(hours);
    print_char('h');
    print_char(' ');
    print_dec(minutes);
    print_char('m');
    print_char(' ');
    print_dec(seconds);
    print_char('s');
    print_char('\n');
}

static void shell_echo(const char* args) {
    print_string(args);
    print_char('\n');
}


extern void process_list();
extern process_t* process_create(const char* name, void (*entry_point)());

// Test process functions
void test_process_a() {
    for (int i = 0; i < 5; i++) {
        print_string("[Process A] Running iteration ");
        print_dec(i);
        print_string("\n");
        for (volatile int j = 0; j < 10000000; j++);
    }
}

void test_process_b() {
    for (int i = 0; i < 5; i++) {
        print_string("[Process B] Running iteration ");
        print_dec(i);
        print_string("\n");
        for (volatile int j = 0; j < 10000000; j++);
    }
}


static void shell_about() {
    print_string("Zenix OS - Phase 4\n");
    print_string("==================\n");
    print_string("A simple 32-bit operating system\n");
    print_string("Features:\n");
    print_string("  * Protected Mode\n");
    print_string("  * GDT & IDT\n");
    print_string("  * Hardware Interrupts\n");
    print_string("  * PIT Timer (100Hz)\n");
    print_string("  * PS/2 Keyboard\n");
    print_string("  * Basic Shell\n");
}

static void shell_meminfo() {
    print_string("Memory Information:\n");
    print_string("===================\n");
    
    print_string("Physical Memory:\n");
    print_string("  Total: ");
    print_dec(pmm_get_total_memory() / 1024 / 1024);
    print_string(" MB\n");
    
    print_string("  Used:  ");
    print_dec(pmm_get_used_memory() / 1024 / 1024);
    print_string(" MB\n");
    
    print_string("  Free:  ");
    print_dec(pmm_get_free_memory() / 1024 / 1024);
    print_string(" MB\n\n");
    
    print_string("Kernel Heap:\n");
    print_string("  Used:  ");
    print_dec(heap_get_used() / 1024);
    print_string(" KB\n");
    
    print_string("  Free:  ");
    print_dec(heap_get_free() / 1024);
    print_string(" KB\n");
}

static void shell_execute(const char* cmd) {
    if (strlen(cmd) == 0) {
        return;
    }
    
    // Parse command
    char command[32] = {0};
    const char* args = cmd;
    
    // Extract command
    uint32_t i = 0;
    while (*args && *args != ' ' && i < 31) {
        command[i++] = *args++;
    }
    command[i] = '\0';
    
    // Skip spaces
    while (*args == ' ') {
        args++;
    }
    
    // Execute command
    if (strcmp(command, "help") == 0) {
        shell_help();
    } else if (strcmp(command, "clear") == 0) {
        shell_clear();
    } else if (strcmp(command, "uptime") == 0) {
        shell_uptime();
    } else if (strcmp(command, "echo") == 0) {
        shell_echo(args);
    } else if (strcmp(command, "meminfo") == 0) {
        shell_meminfo();
    } else if (strcmp(command, "ps") == 0) {
        process_list();
    } else if (strcmp(command, "spawn") == 0) {
        process_create("test_a", test_process_a);
        process_create("test_b", test_process_b);
        print_string("Spawned test processes\n");
    } else if (strcmp(command, "vmstat") == 0) {
        shell_vmstat();
    } else if (strcmp(command, "about") == 0) {
        shell_about();
    } else {
        print_string("Unknown command: ");
        print_string(command);
        print_string("\nType 'help' for available commands.\n");
    }
}

void shell_init() {
    command_pos = 0;
    memset(command_buffer, 0, COMMAND_BUFFER_SIZE);
}

void shell_run() {
    shell_clear();
    print_string("Type 'help' for available commands.\n\n");
    
    while (1) {
        print_string("zenix> ");
        command_pos = 0;
        memset(command_buffer, 0, COMMAND_BUFFER_SIZE);
        
        // Read command
        while (1) {
            char c = keyboard_getchar();
            if (c == 0) {
                asm volatile("hlt");
                continue;
            }
            
            if (c == '\n') {
                command_buffer[command_pos] = '\0';
                print_char('\n');
                shell_execute(command_buffer);
                break;
            } else if (c == '\b') {
                if (command_pos > 0) {
                    command_pos--;
                    command_buffer[command_pos] = '\0';
                }
            } else if (command_pos < COMMAND_BUFFER_SIZE - 1) {
                command_buffer[command_pos++] = c;
            }
        }
    }
}