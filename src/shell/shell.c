#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/string.h"
#include "../include/tests/memtest.h"
#include "../include/memory/memory_map.h"
#include "../include/memory/pmm.h"
#include "../include/tests/syscall_test.h"
#include "../include/version.h"
#include "../include/fs/fat16.h"
#include <stdint.h>
#include <stdbool.h>

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
    terminal_writestring("Litago Version ");
    terminal_writestring(info->version_string);
    terminal_writestring("\nBuild: ");
    terminal_writestring(info->build_date);
    terminal_writestring(" ");
    terminal_writestring(info->build_time);
    terminal_writestring("\n");
}

// Function to handle shell commands
static void handle_command(const char* command) {
    if (strncmp(command, "ls", 2) == 0) {
        const char* path = command + 2;
        while (*path == ' ') path++;  // Skip spaces
        
        if (!fat16_list_directory(path)) {
            terminal_writestring("Failed to list directory\n");
        }
    } else if (strncmp(command, "cat", 3) == 0) {
        const char* filename = command + 3;
        while (*filename == ' ') filename++;  // Skip spaces
        
        if (*filename == '\0') {
            terminal_writestring("Usage: cat <filename>\n");
            return;
        }

        // Allocate buffer for file contents
        char* buffer = (char*)malloc(4096);  // 4KB buffer
        if (!buffer) {
            terminal_writestring("Failed to allocate memory\n");
            return;
        }

        int result = fat16_read_file(filename, buffer, 4096);
        if (result == -1) {
            terminal_writestring("Can't read a empty file\n");
        } else if (result) {
            terminal_writestring(buffer);
            terminal_writestring("\n");
        } else {
            terminal_writestring("Failed to read file\n");
        }

        free(buffer);
    } else if (strncmp(command, "help", 4) == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  ls [path]      - List directory contents\n");
        terminal_writestring("  cat <file>     - Display file contents\n");
        terminal_writestring("  echo <text>    - Display text\n");
        terminal_writestring("  help           - Show this help message\n");
        terminal_writestring("  shutdown       - Power off the machine\n");
        terminal_writestring("  reboot         - Reboot the machine\n");
        terminal_writestring("  memtest        - Run memory test\n");
        terminal_writestring("  memstats       - Show memory statistics\n");
        terminal_writestring("  syscall        - Run syscall test\n");
        terminal_writestring("  version        - Show OS version info\n");
    } else if (strncmp(command, "shutdown", 8) == 0) {
        terminal_writestring("Shutting down...\n");
        shutdown();
    } else if (strncmp(command, "reboot", 6) == 0) {
        terminal_writestring("Rebooting...\n");
        reboot();
    } else if (strncmp(command, "echo", 4) == 0) {
        const char* args = command + 4;
        if (args != NULL) {
            terminal_writestring(args);
            terminal_writestring("\n");
        } else {
            terminal_writestring("\n");
        }
    } else if (strncmp(command, "memtest", 7) == 0) {
        memtest_run();
    } else if (strncmp(command, "memstats", 8) == 0) {
        memstats();
    } else if (strncmp(command, "syscall", 7) == 0) {
        test_syscalls();
    } else if (strncmp(command, "version", 7) == 0) {
        version();
    } else if (strncmp(command, "mkfile", 6) == 0) {
        const char* filename = command + 6;
        while (*filename == ' ') filename++;
        if (*filename == '\0') {
            terminal_writestring("Usage: mkfile <filename>\n");
            return;
        }
        if (fat16_create_file(filename)) {
            terminal_writestring("File created successfully\n");
        } else {
            terminal_writestring("Failed to create file (already exists, no space, or error)\n");
        }
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
