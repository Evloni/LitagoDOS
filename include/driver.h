#ifndef DRIVER_H
#define DRIVER_H

#include <stdbool.h>

// Driver structure
typedef struct {
    const char* name;
    bool (*init)(void);
    bool (*shutdown)(void);
    void* private_data;
} driver_t;

// Driver registration functions
bool register_driver(const char* name, bool (*init)(void), bool (*shutdown)(void), void* private_data);
bool init_drivers(void);
void shutdown_drivers(void);

#endif // DRIVER_H