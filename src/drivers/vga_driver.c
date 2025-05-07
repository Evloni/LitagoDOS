#include "../../include/vga.h"
#include "../../include/driver.h"
#include <stdbool.h>

bool vga_driver_init(void) {
    // Initialize the VGA hardware
    vga_init();
    return true;
}

bool vga_shutdown(void) {
    // Nothing to do for VGA shutdown
    return true;
}

