#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/string.h"
#include "../include/driver.h"
#include "../include/tests/memtest.h"
#include "../include/memory/memory_map.h"
#include "../include/memory/pmm.h"
#include "../include/tests/syscall_test.h"
#include "../include/version.h"
#include <stdint.h>

// Shell visual elements
#define PROMPT_COLOR VGA_COLOR_LIGHT_GREEN
#define TEXT_COLOR VGA_COLOR_WHITE
#define HEADER_COLOR VGA_COLOR_LIGHT_CYAN
#define BORDER_COLOR VGA_COLOR_LIGHT_BLUE

// Track the prompt position
size_t prompt_x = 0;
size_t prompt_y = 0;



void draw_header() {
    terminal_setcolor(HEADER_COLOR);
    terminal_set_cursor(2, 1);
    terminal_writestring("+------------------------------------------------------------------+");
    terminal_set_cursor(2, 2);
    terminal_writestring("|                                                                  |");
    terminal_set_cursor(2, 3);
    terminal_writestring("|                    Litago Operating System                       |");
    terminal_set_cursor(2, 4);
    terminal_writestring("|                                                                  |");
    terminal_set_cursor(2, 5);
    terminal_writestring("+------------------------------------------------------------------+");
    terminal_writestring("\n  Type 'help' for a list of commands\n");
}

void draw_prompt() {
    terminal_setcolor(PROMPT_COLOR);
    terminal_writestring("\n[litago] ");
    terminal_setcolor(TEXT_COLOR);
    
    // Store the current cursor position after drawing the prompt
    terminal_get_cursor(&prompt_x, &prompt_y);
}

static void shutdown() {
    shutdown_drivers();
    
    // Try QEMU's shutdown port first
    outw(0x604, 0x2000);
    
    // If that doesn't work, try Bochs shutdown port
    outw(0xB004, 0x2000);
    
    // If we're still here, try the i8042 keyboard controller reset
    outb(0x64, 0xFE);
    
    // If all else fails, just exit QEMU
    outw(0x604, 0x2000);
    outw(0x604, 0x2000);  // Send it twice to be sure
    
    // If we get here, something went wrong
    terminal_writestring("Shutdown failed. Please power off manually.\n");
    while(1) {
        __asm__("hlt");
    }
}

static void reboot() {
    shutdown_drivers();
    
    // Try to reboot using the keyboard controller
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);  // Send reset command to keyboard controller
    
    // If that doesn't work, try triple fault
    // This will cause the CPU to reset
    asm volatile("lidt 0");  // Load invalid IDT
    asm volatile("int3");    // Trigger interrupt
    
    // If we get here, something went wrong
    terminal_writestring("Reboot failed. Please reset manually.\n");
    while(1) {
        __asm__("hlt");
    }
}

static void print_mem_size(uint64_t bytes) {
    uint64_t gb = bytes / (1024 * 1024 * 1024);
    bytes %= (1024 * 1024 * 1024);
    uint64_t mb = bytes / (1024 * 1024);
    bytes %= (1024 * 1024);
    uint64_t kb = bytes / 1024;

    char buf[32];
    if (gb > 0) {
        itoa(gb, buf, 10);
        terminal_writestring(buf);
        terminal_writestring(" GB ");
    }
    if (mb > 0 || gb > 0) {
        itoa(mb, buf, 10);
        terminal_writestring(buf);
        terminal_writestring(" MB ");
    }
    itoa(kb, buf, 10);
    terminal_writestring(buf);
    terminal_writestring(" KB");
}

static void memstats() {
    terminal_writestring("Memory stats:\n  Total memory: ");
    uint64_t total_bytes = (uint64_t)pmm_get_total_pages() * 4096;
    print_mem_size(total_bytes);
    terminal_writestring("\n  Free memory: ");
    uint64_t free_bytes = (uint64_t)pmm_get_free_pages() * 4096;
    print_mem_size(free_bytes);
    terminal_writestring("\n");
}

static void version() {
    const struct version_info* info = get_version_info();
    terminal_writestring("LitagoDOS Version ");
    terminal_writestring(info->version_string);
    terminal_writestring("\nBuild: ");
    terminal_writestring(info->build_date);
    terminal_writestring(" ");
    terminal_writestring(info->build_time);
    terminal_writestring("\n");
}

// Function to handle shell commands
static void handle_command(const char* command) {
    if (strcmp(command, "shutdown") == 0) {
        terminal_writestring("Shutting down...\n");
        shutdown();
    } else if (strcmp(command, "reboot") == 0) {
        terminal_writestring("Rebooting...\n");
        reboot();
    } else if (strcmp(command, "print") == 0) {
        terminal_writestring("Hello from LitagoDOS!\n");
    } else if (strcmp(command, "memtest") == 0) {
        memtest_run();
    } else if (strcmp(command, "memstats") == 0) {
        memstats();
    } else if (strcmp(command, "syscall") == 0) {
        test_syscalls();
    } else if (strcmp(command, "version") == 0) {
        version();
    } else if (strcmp(command, "help") == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  shutdown - Shutdown the system\n");
        terminal_writestring("  reboot   - Restart the system\n");
        terminal_writestring("  print    - Print a test message\n");
        terminal_writestring("  memtest  - Test system memory\n");
        terminal_writestring("  memstats - Show memory statistics\n");
        terminal_writestring("  syscall  - Test system calls\n");
        terminal_writestring("  version  - Show system version\n");
        terminal_writestring("  help     - Show this help message\n");
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\n");
    }
}

void shell_init() {
    terminal_clear();
    draw_header();
    draw_prompt();
}

void shell_run() {
    char command[256];
    int command_index = 0;

    while (1) {
        char c = terminal_getchar();
        
        if (c == '\n') {
            command[command_index] = '\0';
            terminal_putchar('\n');
            
            if (command_index > 0) {
                handle_command(command);
            }
            
            draw_prompt();
            command_index = 0;
        } else if (c == '\b') {
            if (command_index > 0) {
                command_index--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        } else if (c != 0 && command_index < 255) {
            command[command_index++] = c;
            terminal_putchar(c);
        }
    }
}
