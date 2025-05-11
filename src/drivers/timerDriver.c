#include "../../include/timerDriver.h"
#include "../../include/io.h"
#include "../../include/vga.h"
#include <stdbool.h>

// PIT (Programmable Interval Timer) constants
#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43

// Timer tick count
static uint32_t timer_ticks = 0;

// Timer interrupt handler
void timer_handler(struct regs *r) {
    timer_ticks++;
}

// Initialize the timer
static void timer_init(void) {
    // Calculate the divisor for the desired frequency (1000 Hz = 1ms intervals)
    uint32_t divisor = PIT_FREQUENCY / 1000;
    
    // Send command byte: channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(PIT_COMMAND, 0x36);
    
    // Send divisor
    outb(PIT_CHANNEL0, divisor & 0xFF);         // Low byte
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);  // High byte
}

// Get the current tick count
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

// Delay for a specified number of milliseconds
void timer_delay_ms(uint32_t milliseconds) {
    uint32_t start = timer_get_ticks();
    while (timer_get_ticks() - start < milliseconds) {
        // Wait for the specified number of milliseconds
        __asm__("hlt");
    }
}

// Timer driver initialization
bool timer_driver_init(void) {
    timer_init();
    return true;
}

// Timer driver shutdown
bool timer_driver_shutdown(void) {
    return true;
}
