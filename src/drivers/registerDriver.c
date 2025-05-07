#include <stdbool.h>
#include <stddef.h>
#include "../include/vga.h"

// Driver structure
typedef struct {
    const char* name;
    bool (*init)(void);
    bool (*shutdown)(void);
    void* private_data;
} driver_t;

// Driver registry
#define MAX_DRIVERS 16
static driver_t drivers[MAX_DRIVERS];
static size_t driver_count = 0;

// Register a new driver
bool register_driver(const char* name, bool (*init)(void), bool (*shutdown)(void), void* private_data) {
    if (driver_count >= MAX_DRIVERS) {
        return false;
    }
    
    drivers[driver_count].name = name;
    drivers[driver_count].init = init;
    drivers[driver_count].shutdown = shutdown;
    drivers[driver_count].private_data = private_data;
    driver_count++;
    return true;
}

// Initialize all registered drivers
bool init_drivers(void) {
    for (size_t i = 0; i < driver_count; i++) {
        if (!drivers[i].init()) {
            terminal_writestring("Failed to initialize driver: ");
            terminal_writestring(drivers[i].name);
            terminal_writestring("\n");
            return false;
        }
    }
    return true;
}

// Shutdown all drivers
void shutdown_drivers(void) {
    for (size_t i = driver_count; i > 0; i--) {
        if (drivers[i-1].shutdown) {
            drivers[i-1].shutdown();
        }
    }
}