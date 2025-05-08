#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

// Timer driver functions
bool timer_driver_init(void);
bool timer_driver_shutdown(void);

// Timer utility functions
void timer_delay_ms(uint32_t milliseconds);
uint32_t timer_get_ticks(void);

#endif // TIMER_DRIVER_H