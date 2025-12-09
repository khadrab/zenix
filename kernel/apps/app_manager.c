// kernel/apps/app_manager.c - Application Management Implementation
#include "app_manager.h"
#include "../proc/process.h"
#include "../core/monitor.h"

// External app functions
extern void start_calculator_app(void);
extern void update_calculator_app(void);
extern int is_calculator_active(void);

static app_t apps[MAX_APPS];
static int app_count = 0;

// App entry points (process functions)
void calculator_process_entry(void) {
    start_calculator_app();
    
    // Keep process alive while app is active
    while (is_calculator_active()) {
        update_calculator_app();
        // Yield to prevent CPU hogging
        for (volatile int i = 0; i < 1000; i++);
    }
}

void app_manager_init(void) {
    for (int i = 0; i < MAX_APPS; i++) {
        apps[i].active = 0;
        apps[i].pid = 0;
    }
    app_count = 0;
    
    print_string("[APP] Application Manager initialized\n");
}

int app_launch(app_type_t type) {
    if (app_count >= MAX_APPS) {
        print_string("[APP] Error: Too many apps running\n");
        return -1;
    }
    
    // Check if app already running
    for (int i = 0; i < app_count; i++) {
        if (apps[i].active && apps[i].type == type) {
            print_string("[APP] App already running\n");
            return i;
        }
    }
    
    int app_id = app_count++;
    app_t* app = &apps[app_id];
    
    app->type = type;
    app->active = 1;
    
    // Configure based on type
    switch (type) {
        case APP_CALCULATOR:
            app->init_func = start_calculator_app;
            app->update_func = update_calculator_app;
            app->cleanup_func = NULL;
            
            // Create process for calculator
            process_t* proc = process_create("calculator", calculator_process_entry);
            if (proc) {
                app->pid = proc->pid;
                print_string("[APP] Launched Calculator (PID ");
                print_dec(app->pid);
                print_string(")\n");
            } else {
                print_string("[APP] Failed to create calculator process\n");
                app->active = 0;
                app_count--;
                return -1;
            }
            break;
            
        case APP_TEXT_EDITOR:
            // TODO: Implement text editor
            print_string("[APP] Text editor not yet implemented\n");
            app->active = 0;
            app_count--;
            return -1;
            
        case APP_FILE_BROWSER:
            // TODO: Implement file browser
            print_string("[APP] File browser not yet implemented\n");
            app->active = 0;
            app_count--;
            return -1;
            
        case APP_TERMINAL:
            // TODO: Implement terminal
            print_string("[APP] Terminal not yet implemented\n");
            app->active = 0;
            app_count--;
            return -1;
            
        default:
            print_string("[APP] Unknown app type\n");
            app->active = 0;
            app_count--;
            return -1;
    }
    
    return app_id;
}

void app_close(int app_id) {
    if (app_id < 0 || app_id >= app_count) return;
    
    app_t* app = &apps[app_id];
    if (!app->active) return;
    
    // Call cleanup if exists
    if (app->cleanup_func) {
        app->cleanup_func();
    }
    
    // Terminate process
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == app->pid) {
            process_terminate(process_table[i]);
            break;
        }
    }
    
    app->active = 0;
    
    print_string("[APP] Closed app (PID ");
    print_dec(app->pid);
    print_string(")\n");
}

void app_manager_update(void) {
    // Note: Apps update themselves in their processes
    // This function can be used for global app management tasks
    
    // Clean up terminated apps
    for (int i = 0; i < app_count; i++) {
        if (apps[i].active) {
            // Check if process still exists
            int process_exists = 0;
            for (int j = 0; j < MAX_PROCESSES; j++) {
                if (process_table[j] && 
                    process_table[j]->pid == apps[i].pid &&
                    process_table[j]->state != PROCESS_TERMINATED) {
                    process_exists = 1;
                    break;
                }
            }
            
            if (!process_exists) {
                apps[i].active = 0;
            }
        }
    }
}

app_t* app_get_info(int app_id) {
    if (app_id < 0 || app_id >= app_count) return NULL;
    return &apps[app_id];
}