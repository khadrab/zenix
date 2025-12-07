#include "shell.h"
#include "../core/monitor.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/timer/pit.h"
#include "../mm/pmm.h"
#include "../mm/heap.h"
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../fs/vfs.h"
#include "../usermode/usermode.h"
#include "../../lib/libc/string.h"

extern fs_node_t* fs_root;

static char input_buffer[256];
static int input_index = 0;

static void shell_prompt(void) {
    print_string("zenix> ");
}

static void shell_clear(void) {
    clear_screen();
    print_string("Zenix OS Shell - Phase 10\n");
    print_string("Type 'help' for available commands\n\n");
}

static void shell_help(void) {
    print_string("Available commands:\n");
    print_string("  help     - Show this help\n");
    print_string("  clear    - Clear screen\n");
    print_string("  uptime   - Show system uptime\n");
    print_string("  meminfo  - Show memory information\n");
    print_string("  ps       - List processes\n");
    print_string("  spawn    - Spawn test processes\n");
    print_string("  ls       - List files\n");
    print_string("  cat      - Display file contents\n");
    print_string("  syscall  - Test system calls\n");
    print_string("  usermode - Test user mode\n");
    print_string("  echo     - Echo text\n");
    print_string("  about    - About Zenix OS\n");
}

static void shell_uptime(void) {
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    print_string("Uptime: ");
    print_dec(hours);
    print_string("h ");
    print_dec(minutes % 60);
    print_string("m ");
    print_dec(seconds % 60);
    print_string("s\n");
}

static void shell_meminfo(void) {
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

static void shell_ps(void) {
    print_string("PID  Name              State    CPU Time\n");
    print_string("---  ----------------  -------  --------\n");
    process_list();
}

static void test_process_a(void) {
    for (int i = 0; i < 10; i++) {
        print_string("[Process A] Running iteration ");
        print_dec(i);
        print_string("\n");
        for (volatile int j = 0; j < 10000000; j++);
    }
}

static void test_process_b(void) {
    for (int i = 0; i < 10; i++) {
        print_string("[Process B] Running iteration ");
        print_dec(i);
        print_string("\n");
        for (volatile int j = 0; j < 10000000; j++);
    }
}

static void shell_spawn(void) {
    print_string("Spawning test processes...\n");
    process_create("test_a", test_process_a);
    process_create("test_b", test_process_b);
}

static void shell_ls(void) {
    if (!fs_root) {
        print_string("Error: No filesystem mounted\n");
        return;
    }
    
    print_string("Directory listing:\n");
    int i = 0;
    dirent_t* node = 0;
    
    while ((node = fs_readdir(fs_root, i)) != 0) {
        print_string("  ");
        print_string(node->name);
        
        fs_node_t* fsnode = fs_finddir(fs_root, node->name);
        if (fsnode && (fsnode->flags & FS_DIRECTORY)) {
            print_string("/");
        }
        print_string("\n");
        i++;
        
        if (i > 100) break;
    }
}

static void shell_cat(const char* filename) {
    if (!fs_root) {
        print_string("Error: No filesystem mounted\n");
        return;
    }
    
    if (!filename || filename[0] == '\0') {
        print_string("Usage: cat <filename>\n");
        return;
    }
    
    fs_node_t* fsnode = fs_finddir(fs_root, (char*)filename);
    if (!fsnode) {
        print_string("Error: File not found\n");
        return;
    }
    
    uint8_t buffer[257];
    uint32_t offset = 0;
    uint32_t sz;
    
    while ((sz = fs_read(fsnode, offset, 256, buffer)) > 0) {
        buffer[sz] = '\0';
        print_string((char*)buffer);
        offset += sz;
    }
    print_char('\n');
}

static void shell_echo(const char* text) {
    if (text && text[0] != '\0') {
        print_string(text);
    }
    print_char('\n');
}

static void shell_about(void) {
    print_string("\nZenix OS - Phase 10\n");
    print_string("====================\n");
    print_string("A real operating system\n");
    print_string("Features:\n");
    print_string("  - Protected mode (x86)\n");
    print_string("  - Memory management\n");
    print_string("  - Multitasking\n");
    print_string("  - System calls\n");
    print_string("  - User mode (Ring 3)\n\n");
}

static void shell_syscall_test(void) {
    print_string("=== System Call Test ===\n\n");
    
    print_string("1. Testing sys_getpid:\n");
    int pid;
    asm volatile(
        "mov $3, %%eax\n"
        "int $0x80\n"
        : "=a"(pid)
    );
    print_string("   Current PID = ");
    print_dec(pid);
    print_string("\n\n");
    
    print_string("2. Testing sys_write:\n");
    const char* msg = "   [Syscall Output] Hello from int 0x80!\n";
    int ret;
    asm volatile(
        "mov $1, %%eax\n"
        "mov $1, %%ebx\n"
        "mov %1, %%ecx\n"
        "mov $41, %%edx\n"
        "int $0x80\n"
        : "=a"(ret)
        : "r"(msg)
        : "ebx", "ecx", "edx"
    );
    print_string("   Bytes written: ");
    print_dec(ret);
    print_string("\n\n");
    
    print_string("3. Testing sys_sleep:\n");
    print_string("   Sleeping for 1000ms...\n");
    asm volatile(
        "mov $4, %%eax\n"
        "mov $1000, %%ebx\n"
        "int $0x80\n"
        :
        :
        : "eax", "ebx"
    );
    print_string("   Done!\n\n");
    
    print_string("=== Test Complete ===\n");
}

static void shell_usermode_test(void) {
    usermode_test();
}

static void shell_execute(const char* command) {
    if (strlen(command) == 0) return;
    
    char cmd[64] = {0};
    char args[192] = {0};
    
    int i = 0;
    while (command[i] && command[i] != ' ' && i < 63) {
        cmd[i] = command[i];
        i++;
    }
    cmd[i] = '\0';
    
    if (command[i] == ' ') {
        i++;
        int j = 0;
        while (command[i] && j < 191) {
            args[j++] = command[i++];
        }
        args[j] = '\0';
    }
    
    if (strcmp(cmd, "help") == 0) {
        shell_help();
    } else if (strcmp(cmd, "clear") == 0) {
        shell_clear();
    } else if (strcmp(cmd, "uptime") == 0) {
        shell_uptime();
    } else if (strcmp(cmd, "meminfo") == 0) {
        shell_meminfo();
    } else if (strcmp(cmd, "ps") == 0) {
        shell_ps();
    } else if (strcmp(cmd, "spawn") == 0) {
        shell_spawn();
    } else if (strcmp(cmd, "ls") == 0) {
        shell_ls();
    } else if (strcmp(cmd, "cat") == 0) {
        shell_cat(args);
    } else if (strcmp(cmd, "echo") == 0) {
        shell_echo(args);
    } else if (strcmp(cmd, "syscall") == 0) {
        shell_syscall_test();
    } else if (strcmp(cmd, "usermode") == 0) {
        shell_usermode_test();
    } else if (strcmp(cmd, "about") == 0) {
        shell_about();
    } else {
        print_string("Unknown command: ");
        print_string(cmd);
        print_string("\n");
    }
}

void shell_init(void) {
    input_index = 0;
    memset(input_buffer, 0, sizeof(input_buffer));
    shell_clear();
}

void shell_run(void) {
    shell_prompt();
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c == 0) continue;
        
        if (c == '\n') {
            print_char('\n');
            input_buffer[input_index] = '\0';
            shell_execute(input_buffer);
            input_index = 0;
            memset(input_buffer, 0, sizeof(input_buffer));
            shell_prompt();
        } else if (c == '\b') {
            if (input_index > 0) {
                input_index--;
                input_buffer[input_index] = '\0';
                print_char('\b');
            }
        } else if (input_index < 255) {
            input_buffer[input_index++] = c;
            print_char(c);
        }
    }
}
