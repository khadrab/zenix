// kernel/apps/app_manager.h - Application Management System
#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "../../include/types.h"

#define MAX_APPS 16

typedef enum {
    APP_CALCULATOR,
    APP_TEXT_EDITOR,
    APP_FILE_BROWSER,
    APP_TERMINAL
} app_type_t;

typedef struct {
    app_type_t type;
    char name[32];
    uint32_t pid;           // Associated process ID
    uint8_t active;
    void (*init_func)(void);
    void (*update_func)(void);
    void (*cleanup_func)(void);
} app_t;

// Initialize app manager
void app_manager_init(void);

// Launch an app (creates process + initializes app)
int app_launch(app_type_t type);

// Close an app (terminates process)
void app_close(int app_id);

// Update all active apps
void app_manager_update(void);

// Get app info
app_t* app_get_info(int app_id);

#endif // APP_MANAGER_H