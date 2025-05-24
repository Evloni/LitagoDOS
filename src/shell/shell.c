#include "../../include/shell.h"
#include "../../include/vga.h"
#include "../../include/keyboardDriver.h"
#include "../../include/string.h"
#include "../../include/tests/memtest.h"
#include "../../include/memory/memory_map.h"
#include "../../include/memory/pmm.h"
#include "../../include/tests/syscall_test.h"
#include "../../include/version.h"
#include "../../include/fs/fat16.h"
#include "../../include/test.h"
#include "../../include/string.h"
#include "../../include/editor.h"
#include "../../include/drivers/iso_fs.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// External declarations for FAT16 variables
extern fat16_boot_sector_t boot_sector;
extern uint32_t root_dir_sectors;
extern uint32_t root_dir_start_sector;

// Shell visual elements
#define PROMPT_COLOR VGA_COLOR_LIGHT_GREEN
#define TEXT_COLOR VGA_COLOR_WHITE
#define HEADER_COLOR VGA_COLOR_LIGHT_CYAN
#define BORDER_COLOR VGA_COLOR_LIGHT_BLUE

// Track the prompt position
size_t prompt_x = 0;
size_t prompt_y = 0;

#define MAX_CMD_LENGTH 256
#define MAX_HISTORY 10

static char cmd_buffer[MAX_CMD_LENGTH];
static char cmd_history[MAX_HISTORY][MAX_CMD_LENGTH];
static int history_count = 0;
static int history_index = -1;  // Start at -1 to indicate no history position
static int cmd_index = 0;

// Array of built-in commands
static const char* builtin_commands[] = {
    "help", "ls", "cat", "echo", "shutdown", "reboot", "memtest",
    "memtest2", "memstats", "syscall", "version", "progtest",
    "mkfile", "rm", "clear", "edit"
};
static const int num_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

// Function to calculate Levenshtein distance between two strings
static int levenshtein_distance(const char* s1, const char* s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    // Use a fixed-size array that's large enough for our commands
    #define MAX_CMD_LEN 32
    int matrix[MAX_CMD_LEN + 1][MAX_CMD_LEN + 1];
    
    // Initialize first row and column
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    // Fill the matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            if (s1[i-1] == s2[j-1]) {
                matrix[i][j] = matrix[i-1][j-1];
            } else {
                int min = matrix[i-1][j-1];  // substitution
                if (matrix[i-1][j] < min) min = matrix[i-1][j];  // deletion
                if (matrix[i][j-1] < min) min = matrix[i][j-1];  // insertion
                matrix[i][j] = min + 1;
            }
        }
    }
    
    return matrix[len1][len2];
}

// Function to find the closest matching command
static const char* find_closest_command(const char* input) {
    const char* closest = NULL;
    int min_distance = 999;  // Start with a large number
    int threshold = 3;  // Maximum edit distance to consider
    
    // First try to find commands that start with the input
    for (int i = 0; i < num_builtin_commands; i++) {
        if (strncmp(builtin_commands[i], input, strlen(input)) == 0) {
            return builtin_commands[i];
        }
    }
    
    // If no prefix match found, use Levenshtein distance
    for (int i = 0; i < num_builtin_commands; i++) {
        int distance = levenshtein_distance(input, builtin_commands[i]);
        if (distance < min_distance && distance <= threshold) {
            min_distance = distance;
            closest = builtin_commands[i];
        }
    }
    
    return closest;
}

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
    terminal_writestring(info->os_name);
    terminal_writestring(" Version ");
    terminal_writestring(info->version_string);
    terminal_writestring("\nPlatform: ");
    terminal_writestring(info->os_platform);
    terminal_writestring(" (");
    terminal_writestring(info->os_arch);
    terminal_writestring(")\nBuild: ");
    terminal_writestring(info->build_date);
    terminal_writestring(" ");
    terminal_writestring(info->build_time);
    terminal_writestring("\n");
}

// Function to get all possible completions
static void get_completions(const char* partial, char** completions, int* num_completions) {
    *num_completions = 0;
    
    // Check built-in commands
    for (int i = 0; i < num_builtin_commands; i++) {
        if (strncmp(builtin_commands[i], partial, strlen(partial)) == 0) {
            completions[(*num_completions)++] = (char*)builtin_commands[i];
        }
    }
    
    // Check files in current directory
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) return;
    
    if (!iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return;
    }
    
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (root_dir[i].filename[0] == 0x00) break;
        if (root_dir[i].filename[0] == 0xE5) continue;
        if ((root_dir[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
        if (root_dir[i].attributes & FAT16_ATTR_VOLUME_ID) continue;
        
        // Format filename
        char name[13] = {0};
        int name_idx = 0;
        for (int j = 0; j < 8; j++) {
            if (root_dir[i].filename[j] != ' ') {
                name[name_idx++] = root_dir[i].filename[j];
            }
        }
        if (root_dir[i].extension[0] != ' ') {
            name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) {
                if (root_dir[i].extension[j] != ' ') {
                    name[name_idx++] = root_dir[i].extension[j];
                }
            }
        }
        name[name_idx] = '\0';
        
        if (strncmp(name, partial, strlen(partial)) == 0) {
            completions[(*num_completions)++] = strdup(name);
        }
    }
    
    free(root_dir);
}

// Function to handle tab completion
static void handle_tab_completion(void) {
    char* completions[256];  // Maximum number of completions
    int num_completions = 0;
    
    // Get all possible completions
    get_completions(cmd_buffer, completions, &num_completions);
    
    if (num_completions == 0) {
        // No completions found
        return;
    } else if (num_completions == 1) {
        // Single completion - complete it
        // Clear current line
        while (cmd_index > 0) {
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
            cmd_index--;
        }
        
        // Copy completion to buffer
        strcpy(cmd_buffer, completions[0]);
        cmd_index = strlen(cmd_buffer);
        
        // Redraw the command
        terminal_writestring(cmd_buffer);
        terminal_get_cursor(&prompt_x, &prompt_y);
    } else {
        // Multiple completions - show all possibilities
        terminal_writestring("\n");
        for (int i = 0; i < num_completions; i++) {
            terminal_writestring(completions[i]);
            terminal_writestring("\n");
        }
        terminal_writestring("\n");
        
        // Redraw prompt and command
        draw_prompt();
        
        // Clear and redraw the command
        while (cmd_index > 0) {
            terminal_putchar('\b');
            terminal_putchar(' '); 
            terminal_putchar('\b');
            cmd_index--;
        }
        terminal_writestring(cmd_buffer);
        cmd_index = strlen(cmd_buffer);
        
        // Update cursor position
        terminal_get_cursor(&prompt_x, &prompt_y);
    }
    
    // Free allocated completions
    for (int i = 0; i < num_completions; i++) {
        if (completions[i] != builtin_commands[i]) {  // Only free if it's not a builtin command
            free(completions[i]);
        }
    }
}

// Function to handle shell commands
static void handle_command(const char* command) {
    // Skip empty commands
    if (command[0] == '\0') return;
    
    // Extract the command name (first word)
    char cmd_name[32] = {0};
    int i = 0;
    while (command[i] != ' ' && command[i] != '\0' && i < 31) {
        cmd_name[i] = command[i];
        i++;
    }
    cmd_name[i] = '\0';
    
    // Check if command exists
    bool command_found = false;
    for (int i = 0; i < num_builtin_commands; i++) {
        if (strcmp(cmd_name, builtin_commands[i]) == 0) {
            command_found = true;
            break;
        }
    }
    
    // If command not found, suggest similar commands
    if (!command_found) {
        const char* suggestion = find_closest_command(cmd_name);
        if (suggestion) {
            terminal_writestring("Command not found. Did you mean: ");
            terminal_writestring(suggestion);
            terminal_writestring("?\n");
        } else {
            terminal_writestring("Unknown command. Type 'help' for available commands.\n");
        }
        return;
    }
    
    // Handle known commands
    if (strcmp(cmd_name, "help") == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  ls [path]      - List directory contents\n");
        terminal_writestring("  cat <file>     - Display file contents\n");
        terminal_writestring("  echo <text>    - Display text\n");
        terminal_writestring("  help           - Show this help message\n");
        terminal_writestring("  shutdown       - Power off the machine\n");
        terminal_writestring("  reboot         - Reboot the machine\n");
        terminal_writestring("  memtest        - Run basic memory test\n");
        terminal_writestring("  memtest2       - Run advanced memory management test\n");
        terminal_writestring("  memstats       - Show memory statistics\n");
        terminal_writestring("  syscall        - Run syscall test\n");
        terminal_writestring("  version        - Show OS version info\n");
        terminal_writestring("  progtest       - Run program loading test\n");
        terminal_writestring("  mkfile <file>  - Create a new empty file\n");
        terminal_writestring("  rm <file>      - Remove a file\n");
        terminal_writestring("  clear          - Clear the screen\n");
        terminal_writestring("  edit <file>    - Edit a file\n");
    } else if (strcmp(cmd_name, "shutdown") == 0) {
        terminal_writestring("Shutting down...\n");
        shutdown();
    } else if (strcmp(cmd_name, "reboot") == 0) {
        terminal_writestring("Rebooting...\n");
        reboot();
    } else if (strcmp(cmd_name, "echo") == 0) {
        const char* args = command + strlen(cmd_name);
        while (*args == ' ') args++;  // Skip spaces
        if (args != NULL) {
            terminal_writestring(args);
            terminal_writestring("\n");
        } else {
            terminal_writestring("\n");
        }
    } else if (strcmp(cmd_name, "memtest2") == 0) {
        test_memory_management();
    } else if (strcmp(cmd_name, "memtest") == 0) {
        memtest_run();
    } else if (strcmp(cmd_name, "memstats") == 0) {
        memstats();
    } else if (strcmp(cmd_name, "syscall") == 0) {
        test_syscalls();
    } else if (strcmp(cmd_name, "version") == 0) {
        version();
    } else if (strcmp(cmd_name, "progtest") == 0) {
        test_program_loading();
    } else if (strcmp(cmd_name, "ls") == 0) {
        const char* path = command + strlen(cmd_name);
        while (*path == ' ') path++;  // Skip spaces
        
        if (!fat16_list_directory(path)) {
            terminal_writestring("Failed to list directory\n");
        }
    } else if (strcmp(cmd_name, "cat") == 0) {
        const char* filename = command + strlen(cmd_name);
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
    } else if (strcmp(cmd_name, "mkfile") == 0) {
        const char* filename = command + strlen(cmd_name);
        while (*filename == ' ') filename++;  // Skip spaces
        
        if (*filename == '\0') {
            terminal_writestring("Usage: mkfile <filename>\n");
            return;
        }

        if (fat16_create_file(filename)) {
            terminal_writestring("File created successfully\n");
        } else {
            terminal_writestring("Failed to create file\n");
        }
    } else if (strcmp(cmd_name, "clear") == 0) {
        terminal_clear();
        draw_header();  // Redraw the header after clearing
    } else if (strcmp(cmd_name, "edit") == 0) {
        const char* filename = command + strlen(cmd_name);
        while (*filename == ' ') filename++;  // Skip spaces
        
        if (*filename == '\0') {
            terminal_writestring("Usage: edit <filename>\n");
            return;
        }
        editor_edit_command(filename);
    } else if (strcmp(cmd_name, "rm") == 0) {
        const char* filename = command + strlen(cmd_name);
        while (*filename == ' ') filename++;  // Skip spaces
        
        if (*filename == '\0') {
            terminal_writestring("Usage: rm <filename>\n");
            return;
        }

        if (fat16_remove_file(filename)) {
            terminal_writestring("File removed successfully\n");
        } else {
            terminal_writestring("Failed to remove file\n");
        }
    }
}

// Initialize shell
void shell_init(void) {
    memset(cmd_buffer, 0, MAX_CMD_LENGTH);
    memset(cmd_history, 0, sizeof(cmd_history));
    history_count = 0;
    history_index = -1;  // Start at -1 to indicate no history position
    cmd_index = 0;
    
    draw_header();
}

// Start shell
void shell_start(void) {
    terminal_clear();
    shell_init();
    
    while (1) {
        // Display prompt
        draw_prompt();
        
        // Reset command buffer
        memset(cmd_buffer, 0, MAX_CMD_LENGTH);
        cmd_index = 0;
        history_index = -1;  // Reset history index for new command
        
        // Read command
        while (1) {
            int key = keyboard_getchar();
            
            if (key == '\t') {  // Tab key
                handle_tab_completion();
                continue;
            }
            
            // Handle special keys
            if (key == '\033') {  // Escape sequence
                char next = keyboard_getchar();
                if (next == '[') {  // Control sequence introducer
                    char code = keyboard_getchar();
                    switch (code) {
                        case 'A':  // Up arrow - previous command
                            if (history_count > 0) {
                                // Clear current line
                                while (cmd_index > 0) {
                                    terminal_putchar('\b');
                                    terminal_putchar(' ');
                                    terminal_putchar('\b');
                                    cmd_index--;
                                }
                                
                                // Move to previous command in history
                                if (history_index == -1) {
                                    history_index = history_count - 1;
                                } else if (history_index > 0) {
                                    history_index--;
                                }
                                
                                // Copy command from history
                                strncpy(cmd_buffer, cmd_history[history_index], MAX_CMD_LENGTH);
                                cmd_index = strlen(cmd_buffer);
                                
                                // Display the command
                                terminal_writestring(cmd_buffer);
                                // Update cursor position
                                terminal_get_cursor(&prompt_x, &prompt_y);
                            }
                            break;
                        case 'B':  // Down arrow - next command
                            if (history_index != -1) {
                                // Clear current line
                                while (cmd_index > 0) {
                                    terminal_putchar('\b');
                                    terminal_putchar(' ');
                                    terminal_putchar('\b');
                                    cmd_index--;
                                }
                                
                                // Move to next command in history
                                if (history_index < history_count - 1) {
                                    history_index++;
                                    strncpy(cmd_buffer, cmd_history[history_index], MAX_CMD_LENGTH);
                                } else {
                                    history_index = -1;
                                    cmd_buffer[0] = '\0';
                                }
                                
                                // Update display
                                cmd_index = strlen(cmd_buffer);
                                terminal_writestring(cmd_buffer);
                                // Update cursor position
                                terminal_get_cursor(&prompt_x, &prompt_y);
                            }
                            break;
                    }
                }
            }
            else if (key == '\b') {  // Backspace
                if (cmd_index > 0) {
                    cmd_index--;
                    cmd_buffer[cmd_index] = '\0';
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                    // Update cursor position after backspace
                    terminal_get_cursor(&prompt_x, &prompt_y);
                }
            }
            else if (key == '\n') {  // Enter
                terminal_putchar('\n');
                break;
            }
            else if (key >= 32 && key <= 126 && cmd_index < MAX_CMD_LENGTH - 1) {
                // Insert character at current position
                cmd_buffer[cmd_index] = (char)key;
                terminal_putchar((char)key);
                cmd_index++;
                // Update cursor position after each character
                terminal_get_cursor(&prompt_x, &prompt_y);
            }
        }
        
        // Add command to history if it's not empty
        if (cmd_index > 0) {
            // Shift history down if we're at max capacity
            if (history_count == MAX_HISTORY) {
                for (int i = 0; i < MAX_HISTORY - 1; i++) {
                    strncpy(cmd_history[i], cmd_history[i + 1], MAX_CMD_LENGTH);
                }
                history_count--;
            }
            
            // Add new command to history
            strncpy(cmd_history[history_count], cmd_buffer, MAX_CMD_LENGTH);
            history_count++;
        }
        
        // Execute command
        handle_command(cmd_buffer);
    }
}

