#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/string.h"
#include "../include/driver.h"
#include "../include/tests/memtest.h"
#include "../include/memory/memory_map.h"
#include "../include/memory/pmm.h"
#include "../include/tests/syscall_test.h"
#include "../include/version.h"
#include "../include/disk_test.h"
#include "../include/fat16.h"
#include <stdint.h>

// Syscall number for read
#define SYSCALL_READ 1 

// Helper to print a byte as two hex digits (for diagnostics)
static void shell_print_byte_hex(uint8_t value) {
    const char *hex_chars = "0123456789ABCDEF";
    terminal_putchar(hex_chars[(value >> 4) & 0x0F]);
    terminal_putchar(hex_chars[value & 0x0F]);
}

// Helper function to make a syscall for reading a character
static inline char syscall_read_char() {
    char c;
    asm volatile (
        "movl %1, %%eax\n\t"
        "int $0x80\n\t"
        "movb %%al, %0"
        : "=r" (c)         // Output: c
        : "i" (SYSCALL_READ) // Input: syscall number
        : "%eax"           // Clobbered: eax (kernel returns value in eax)
    );
    return c;
}

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
    terminal_writestring("Litago Version ");
    terminal_writestring(info->version_string);
    terminal_writestring("\nBuild: ");
    terminal_writestring(info->build_date);
    terminal_writestring(" ");
    terminal_writestring(info->build_time);
    terminal_writestring("\n");
}

// Simple toupper implementation
static char toupper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

// Converts a user filename (e.g., "TEST.TXT") to FAT16 8.3 format (11 bytes, space padded, uppercase)
static void filename_to_fat16_8_3(const char* filename, char out[11]) {
    // Initialize with spaces
    for (int i = 0; i < 11; ++i) out[i] = ' ';
    int i = 0, j = 0;
    // Copy name part (up to 8 chars, stop at dot or end)
    while (filename[i] && filename[i] != '.' && j < 8) {
        out[j++] = toupper(filename[i++]);
    }
    // If there is a dot, copy extension (up to 3 chars)
    if (filename[i] == '.') {
        ++i; // skip dot
        int k = 8;
        while (filename[i] && k < 11) {
            out[k++] = toupper(filename[i++]);
        }
    }
}

// Restore format_fat16_filename for FAT16 8.3 to string conversion
void format_fat16_filename(const uint8_t input_8_3_name[11], char* output_buffer) {
    int out_idx = 0;
    // Copy name part, skip trailing spaces
    for (int i = 0; i < 8; ++i) {
        if (input_8_3_name[i] == ' ') {
            break;
        }
        output_buffer[out_idx++] = input_8_3_name[i];
    }
    // Check if there's an extension (first char of ext part is not space)
    if (input_8_3_name[8] != ' ') {
        output_buffer[out_idx++] = '.';
        for (int i = 0; i < 3; ++i) {
            if (input_8_3_name[8 + i] == ' ') {
                break;
            }
            output_buffer[out_idx++] = input_8_3_name[8 + i];
        }
    }
    output_buffer[out_idx] = '\0';
}

void shell_ls(void) {
    FAT16_VolumeInfo vol_info;
    if (fat16_init(0, &vol_info) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Failed to initialize FAT16 filesystem\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    terminal_writestring("Type     Size    Name\n");
    terminal_writestring("--------------------------\n");

    #define MAX_ROOT_ENTRIES_TO_LIST 64
    FAT16_DirectoryEntry entries[MAX_ROOT_ENTRIES_TO_LIST];
    int num_found = 0;

    int result = fat16_list_root_directory(&vol_info, entries, MAX_ROOT_ENTRIES_TO_LIST, &num_found);

    if (result != 0) {
        terminal_writestring("Error listing root directory.\n");
        return;
    }

    if (num_found == 0) {
        terminal_writestring("(empty)\n");
        return;
    }

    char formatted_name[13];
    char size_str[12];

    for (int i = 0; i < num_found; ++i) {
        format_fat16_filename(entries[i].DIR_Name, formatted_name);

        if ((entries[i].DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
            terminal_writestring("DIR      ");
            terminal_writestring("        ");
        } else {
            terminal_writestring("FILE     ");
            // Print file size
            int k = 0;
            uint32_t n = entries[i].DIR_FileSize;
            if (n == 0) {
                size_str[k++] = '0';
            } else {
                char t[12];
                int c = 0;
                while (n > 0) {
                    t[c++] = (n % 10) + '0';
                    n /= 10;
                }
                for (int z = c - 1; z >= 0; z--) {
                    size_str[k++] = t[z];
                }
            }
            size_str[k] = '\0';
            terminal_writestring(size_str);
            terminal_writestring("    ");
        }

        terminal_writestring(formatted_name);
        terminal_writestring("\n");
    }
}

void shell_cat(const char* filename) {
    FAT16_VolumeInfo vol_info;
    if (fat16_init(0, &vol_info) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Failed to initialize FAT16 filesystem\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    // Use helper to convert to 8.3 format
    char name_8_3[11];
    filename_to_fat16_8_3(filename, name_8_3);

    // Find the file in root directory
    FAT16_DirectoryEntry file_entry;
    if (fat16_find_entry_in_root(&vol_info, name_8_3, &file_entry) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    // Check if it's a directory
    if ((file_entry.DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Cannot cat a directory: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    // Read and display file contents
    uint8_t buffer[4096]; // Buffer for file contents
    uint32_t bytes_read;
    
    if (fat16_read_file(&vol_info, &file_entry, buffer, sizeof(buffer), &bytes_read) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Error reading file: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    // Display file contents
    for (uint32_t i = 0; i < bytes_read; i++) {
        terminal_putchar(buffer[i]);
    }
    terminal_writestring("\n");
}

// Function to handle shell commands
static void handle_command(const char* command) {
    // Find the first space to separate command and arguments
    const char* args = strchr(command, ' ');
    
    if (strncmp(command, "shutdown", 8) == 0) {
        terminal_writestring("Shutting down...\n");
        shutdown();
    } else if (strncmp(command, "reboot", 6) == 0) {
        terminal_writestring("Rebooting...\n");
        reboot();
    } else if (strncmp(command, "echo", 4) == 0) {
        if (args != NULL) {
            // Skip the space after "echo"
            args++;
            terminal_writestring(args);
            terminal_writestring("\n");
        } else {
            terminal_writestring("\n");
        }
    } else if (strncmp(command, "ls", 2) == 0) {
        shell_ls();
    } else if (strncmp(command, "clear", 5) == 0) {
        terminal_clear();
        draw_header();
    } else if (strncmp(command, "disktest", 8) == 0) {
        run_disk_tests();
    } else if (strncmp(command, "memtest", 7) == 0) {
        memtest_run();
    } else if (strncmp(command, "memstats", 8) == 0) {
        memstats();
    } else if (strncmp(command, "syscall", 7) == 0) {
        test_syscalls();
    } else if (strncmp(command, "version", 7) == 0) {
        version();
    } else if (strncmp(command, "help", 4) == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  shutdown  - Shutdown the system\n");
        terminal_writestring("  reboot    - Restart the system\n");
        terminal_writestring("  echo      - Display text (e.g., echo Hello World)\n");
        terminal_writestring("  ls        - List files in the root directory\n");
        terminal_writestring("  disktest  - Run disk driver tests\n");
        terminal_writestring("  memtest   - Test system memory\n");
        terminal_writestring("  memstats  - Show memory statistics\n");
        terminal_writestring("  syscall   - Test system calls\n");
        terminal_writestring("  clear     - Clear the screen\n");
        terminal_writestring("  version   - Show system version\n");
        terminal_writestring("  help      - Show this help message\n");
    } else if (strncmp(command, "cat", 3) == 0) {
        if (args != NULL) {
            // Skip leading spaces
            while (*args == ' ') args++;
            shell_cat(args);
        } else {
            terminal_writestring("Usage: cat <filename>\n");
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
        char c = syscall_read_char();
        
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
