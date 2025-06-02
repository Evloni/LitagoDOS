#include "../../include/shell.h"
#include "../../include/drivers/vbe.h"
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
#include "../../include/timerDriver.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// VBE display dimensions
#define VBE_WIDTH 1024  // Assuming 1024x768 resolution
#define VBE_HEIGHT 768
#define VBE_BPP 32     // Bits per pixel
#define VBE_PITCH (VBE_WIDTH * (VBE_BPP / 8))

// External declarations for FAT16 variables
extern fat16_boot_sector_t boot_sector;
extern uint32_t root_dir_sectors;
extern uint32_t root_dir_start_sector;

// Shell visual elements
#define PROMPT_COLOR 0xFF00FF00  // Light Green
#define TEXT_COLOR 0xFFFFFFFF    // White
#define HEADER_COLOR 0xFF00FFFF  // Light Cyan
#define BORDER_COLOR 0xFF0000FF  // Light Blue

// Track the prompt position
size_t prompt_x = 0;
size_t prompt_y = 0;

// Current directory tracking
#define MAX_PATH_LENGTH 256
static char current_directory[MAX_PATH_LENGTH] = "/";

// Function to get current directory
const char* get_current_directory(void) {
    return current_directory;
}

// Function to set current directory
bool set_current_directory(const char* path) {
    if (!path || path[0] == '\0') {
        return false;
    }
    
    // Try to change directory
    if (fat16_change_directory(path, &current_cluster)) {
        // Update current directory path
        if (strcmp(path, "/") == 0) {
            strcpy(current_directory, "/");
        } else if (strcmp(path, "..") == 0) {
            // Remove the last directory from the path
            char* last_slash = strrchr(current_directory, '/');
            if (last_slash) {
                if (last_slash == current_directory) {
                    // If we're at /dir, go back to /
                    current_directory[1] = '\0';
                } else {
                    // Otherwise, truncate at the last slash
                    *last_slash = '\0';
                }
            }
        } else {
            // For now, just append the directory name
            if (strcmp(current_directory, "/") != 0) {
                strcat(current_directory, "/");
            }
            strcat(current_directory, path);
        }
        return true;
    }
    
    return false;
}

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
    "mkfile", "rm", "clear", "edit", "cursortest", "cd"
};
static const int num_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

// Shell cursor state
static int prev_cursor_x = -1;
static int prev_cursor_y = -1;

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
    // Use VBE colors directly
    vbe_draw_string(0, 0, "+------------------------------------------------------------------+", BORDER_COLOR, &font_8x16);
    vbe_draw_string(0, 16, "|                                                                  |", BORDER_COLOR, &font_8x16);
    vbe_draw_string(0, 32, "|                    Litago Operating System                       |", HEADER_COLOR, &font_8x16);
    vbe_draw_string(0, 48, "|                                                                  |", BORDER_COLOR, &font_8x16);
    vbe_draw_string(0, 64, "+------------------------------------------------------------------+", BORDER_COLOR, &font_8x16);
    vbe_draw_string(0, 96, "  Type 'help' for a list of commands\n", TEXT_COLOR, &font_8x16);
}

void draw_prompt() {
    // Calculate total prompt length
    int prompt_len = 8 + strlen(current_directory) + 2;  // "[litago:" + dir + "] "
    
    // Clear the line first
    vbe_draw_rect(0, vbe_cursor_y, VBE_WIDTH, font_8x16.height, 0x00000000);
    
    // Draw the prompt
    vbe_draw_string(0, vbe_cursor_y, "[litago:", PROMPT_COLOR, &font_8x16);
    vbe_draw_string(8 * 8, vbe_cursor_y, current_directory, PROMPT_COLOR, &font_8x16);
    vbe_draw_string((8 + strlen(current_directory)) * 8, vbe_cursor_y, "] ", PROMPT_COLOR, &font_8x16);
    
    // Update cursor position
    vbe_cursor_x = prompt_len * 8;  // 8 pixels per character
    vbe_cursor_y = vbe_cursor_y;    // Keep the same line
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
        clear_input_line(cmd_index);
        
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
        clear_input_line(cmd_index);
        cmd_index = 0;
        
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
        terminal_writestring("  cd [path]      - Change directory\n");
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
        terminal_writestring("  cursortest     - Test ANSI cursor movement\n");
    } else if (strcmp(cmd_name, "cursortest") == 0) {
        ansi_set_enabled(true);
        // Test ANSI cursor movement
        terminal_writestring("\nTesting ANSI cursor movement:\n");
        terminal_writestring("Press any key to continue each test...\n\n");

        terminal_writestring("\x1B[31mRed\x1B[0m Normal\n");
        terminal_writestring("\x1B[1mBold\x1B[0m Normal\n");
        terminal_writestring("\x1B[4mUnderline\x1B[0m Normal\n");
        terminal_writestring("\x1B[32;44mGreen on Blue\x1B[0m Normal\n");
        
        // 1. Basic cursor movement
        terminal_writestring("1. Basic cursor movement:\n");
        terminal_writestring("Right 5: ");
        terminal_writestring("\x1B[5C");
        terminal_writestring("X\n");
        terminal_writestring("Left 3: ");
        terminal_writestring("\x1B[3D");
        terminal_writestring("Y\n");
        terminal_writestring("Up 2: ");
        terminal_writestring("\x1B[2A");
        terminal_writestring("Z\n");
        terminal_writestring("Down 1: ");
        terminal_writestring("\x1B[1B");
        terminal_writestring("W\n");
        keyboard_getchar();  // Wait for key press
        terminal_writestring("\n");
        
        // 2. Line movement
        terminal_writestring("2. Line movement:\n");
        terminal_writestring("Next line: ");
        terminal_writestring("\x1B[1E");
        terminal_writestring("NEXT\n");
        terminal_writestring("Prev line: ");
        terminal_writestring("\x1B[1F");
        terminal_writestring("PREV\n");
        keyboard_getchar();  // Wait for key press
        terminal_writestring("\n");
        
        // 3. Horizontal positioning
        terminal_writestring("3. Horizontal positioning:\n");
        terminal_writestring("Col 10: ");
        terminal_writestring("\x1B[10G");
        terminal_writestring("COL10\n");
        keyboard_getchar();  // Wait for key press
        terminal_writestring("\n");
        
        // 4. Absolute positioning
        terminal_writestring("4. Absolute positioning:\n");
        terminal_writestring("Pos (5,5): ");
        terminal_writestring("\x1B[5;5H");
        terminal_writestring("POS\n");
        keyboard_getchar();  // Wait for key press
        terminal_writestring("\n");
        
        // 5. Cursor save/restore
        terminal_writestring("5. Cursor save/restore:\n");
        terminal_writestring("Save pos -> Move -> Restore: ");
        terminal_writestring("\x1B[s");
        terminal_writestring("\x1B[10;10H");
        terminal_writestring("\x1B[u");
        terminal_writestring("RESTORED\n");
        keyboard_getchar();  // Wait for key press
        terminal_writestring("\n");
        
        // 6. Screen clearing
        terminal_writestring("6. Screen clearing:\n");
        terminal_writestring("Clear line: ");
        terminal_writestring("\x1B[K");
        terminal_writestring("CLEARED\n");
        terminal_writestring("Clear screen: ");
        terminal_writestring("\x1B[J");
        terminal_writestring("CLEARED\n");
        
        terminal_writestring("\nTest complete. Press any key to return to shell.\n");
        keyboard_getchar();  // Wait for key press
        terminal_writestring("\x1B[0m"); // Reset all attributes and colors to default
        ansi_set_enabled(false);

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
        
        // Read current directory
        fat16_dir_entry_t* dir_entries = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
        if (!dir_entries) {
            terminal_writestring("Failed to allocate memory\n");
            return;
        }

        if (fat16_read_directory(current_cluster, dir_entries, boot_sector.root_entries)) {
            // Print header
            terminal_writestring("Name           Size    Type\n");
            terminal_writestring("----------------------------------------\n");

            for (int i = 0; i < boot_sector.root_entries; i++) {
                if (dir_entries[i].filename[0] == 0x00) break;
                if (dir_entries[i].filename[0] == 0xE5) continue;
                if ((dir_entries[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
                if (dir_entries[i].attributes & FAT16_ATTR_VOLUME_ID) continue;

                // Format filename
                char name[13] = {0};
                int name_idx = 0;
                for (int j = 0; j < 8; j++) {
                    if (dir_entries[i].filename[j] != ' ') {
                        name[name_idx++] = dir_entries[i].filename[j];
                    }
                }
                if (dir_entries[i].extension[0] != ' ') {
                    name[name_idx++] = '.';
                    for (int j = 0; j < 3; j++) {
                        if (dir_entries[i].extension[j] != ' ') {
                            name[name_idx++] = dir_entries[i].extension[j];
                        }
                    }
                }
                name[name_idx] = '\0';

                // Print name, padded to 16 chars
                terminal_writestring(name);
                int name_len = strlen(name);
                for (int s = name_len; s < 16; s++) {
                    terminal_putchar(' ');
                }

                // Print size or blank for directories
                if (dir_entries[i].attributes & FAT16_ATTR_DIRECTORY) {
                    terminal_writestring("        ");
                } else {
                    char size_str[16];
                    itoa(dir_entries[i].file_size, size_str, 10);
                    terminal_writestring(size_str);
                    int size_len = strlen(size_str);
                    for (int s = size_len; s < 8; s++) {
                        terminal_putchar(' ');
                    }
                }

                // Print type
                const char* type_str = get_file_type(&dir_entries[i]);
                terminal_writestring(type_str);
                terminal_putchar('\n');
            }
        } else {
            terminal_writestring("Failed to read directory\n");
        }

        free(dir_entries);
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

        if (fat16_create_file(filename, current_cluster)) {
            terminal_writestring("File created successfully\n");
        } else {
            terminal_writestring("Failed to create file\n");
        }
    } else if (strcmp(cmd_name, "clear") == 0) {
        terminal_clear();
        draw_header();  // Redraw the header after clearing
        vbe_cursor_x = 0;
        vbe_cursor_y = 125; // or wherever you want the prompt to start
        draw_prompt();  // Set cursor and draw prompt at the correct position
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
    } else if (strcmp(cmd_name, "cd") == 0) {
        const char* path = command + strlen(cmd_name);
        while (*path == ' ') path++;  // Skip spaces
        
        if (*path == '\0') {
            // If no path provided, show current directory
            terminal_writestring(current_directory);
            terminal_writestring("\n");
            return;
        }
        
        if (set_current_directory(path)) {
            // Directory changed successfully
        } else {
            terminal_writestring("Failed to change directory\n");
        }
    }
}

// Initialize shell
void shell_init(void) {
    // Clear screen and draw header
    vbe_clear_screen(0x00000000);
    // Initialize terminal first
    if (!vbe_initialize()) {
        terminal_writestring("VBE initialization failed\n");
        // If terminal initialization fails, we can't continue
        while(1) {
            __asm__("hlt");  // Halt the system
        }
    }
    
    // Clear command buffer and history
    memset(cmd_buffer, 0, MAX_CMD_LENGTH);
    memset(cmd_history, 0, sizeof(cmd_history));
    history_count = 0;
    history_index = -1;  // Start at -1 to indicate no history position
    cmd_index = 0;
    
    draw_header();
    
    // Move cursor below header
    vbe_cursor_x = 0;
    vbe_cursor_y = 125; // or 0, or wherever you want the prompt to start
    prev_cursor_x = -1;
    prev_cursor_y = -1;
    // Draw initial prompt
    draw_prompt();
}

// Start shell
void shell_start(void) {
    shell_init();
    // Initialize command buffer
    memset(cmd_buffer, 0, MAX_CMD_LENGTH);
    cmd_index = 0;
    
    // Draw initial header and prompt
    draw_header();
    draw_prompt();
    
    // Main shell loop
    while (1) {
        // Check for keyboard input
        if (keyboard_buffer_has_data()) {
            char c = keyboard_getchar();
            
            // Handle special keys
            if (c == '\b') {  // Backspace
                if (cmd_index > 0) {
                    cmd_index--;
                    cmd_buffer[cmd_index] = '\0';
                    vbe_cursor_x -= 8;  // Move cursor back one character
                    vbe_draw_rect(vbe_cursor_x, vbe_cursor_y, 8, font_8x16.height, 0x00000000);
                }
            } else if (c == '\n') {  // Enter
                terminal_putchar('\n');
                if (cmd_index > 0) {
                    cmd_buffer[cmd_index] = '\0';
                    
                    // Save command to history
                    if (history_count < MAX_HISTORY) {
                        strcpy(cmd_history[history_count++], cmd_buffer);
                    } else {
                        // Shift history up if we're at max capacity
                        for (int i = 0; i < MAX_HISTORY - 1; i++) {
                            strcpy(cmd_history[i], cmd_history[i + 1]);
                        }
                        strcpy(cmd_history[MAX_HISTORY - 1], cmd_buffer);
                    }
                    
                    handle_command(cmd_buffer);
                    cmd_index = 0;
                    memset(cmd_buffer, 0, MAX_CMD_LENGTH);
                }
                draw_prompt();
            } else if (c == '\t') {  // Tab
                handle_tab_completion();
            } else if (c == '\033') {  // Escape sequence start
                // Read the next character
                if (keyboard_buffer_has_data()) {
                    char next = keyboard_getchar();
                    if (next == '[') {  // ANSI escape sequence
                        if (keyboard_buffer_has_data()) {
                            char code = keyboard_getchar();
                            if (code == 'A') {  // Up arrow
                                if (history_count > 0) {
                                    // Get current cursor position
                                    int current_y = vbe_cursor_y;
                                    
                                    // Clear current line
                                    clear_input_line(cmd_index);
                                    
                                    // Move up in history
                                    if (history_index == -1) {
                                        history_index = history_count - 1;
                                    } else if (history_index > 0) {
                                        history_index--;
                                    }
                                    
                                    // Copy history entry to buffer
                                    strcpy(cmd_buffer, cmd_history[history_index]);
                                    cmd_index = strlen(cmd_buffer);
                                    
                                    // Redraw prompt and command
                                    draw_prompt();
                                    vbe_draw_string(vbe_cursor_x, current_y, cmd_buffer, TEXT_COLOR, &font_8x16);
                                    vbe_cursor_x = vbe_cursor_x + (cmd_index * 8);  // Update cursor position
                                    vbe_cursor_y = current_y;
                                }
                            } else if (code == 'B') {  // Down arrow
                                if (history_index != -1) {
                                    // Get current cursor position
                                    int current_y = vbe_cursor_y;
                                    
                                    // Clear current line
                                    clear_input_line(cmd_index);
                                    
                                    // Move down in history
                                    if (history_index < history_count - 1) {
                                        history_index++;
                                        strcpy(cmd_buffer, cmd_history[history_index]);
                                    } else {
                                        history_index = -1;
                                        memset(cmd_buffer, 0, MAX_CMD_LENGTH);
                                    }
                                    
                                    // Redraw prompt and command
                                    draw_prompt();
                                    if (cmd_index > 0) {
                                        vbe_draw_string(vbe_cursor_x, current_y, cmd_buffer, TEXT_COLOR, &font_8x16);
                                        vbe_cursor_x = vbe_cursor_x + (cmd_index * 8);  // Update cursor position
                                        vbe_cursor_y = current_y;
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (c >= 32 && c <= 126) {  // Printable characters
                if (cmd_index < MAX_CMD_LENGTH - 1) {
                    cmd_buffer[cmd_index++] = c;
                    terminal_putchar(c);
                }
            }
        }
        
        // Small delay to prevent CPU hogging
        for (volatile int i = 0; i < 1000; i++);
    }
}

// Helper function to clear input line
void clear_input_line(int length) {
    // Get current cursor position
    int current_x = vbe_cursor_x;
    int current_y = vbe_cursor_y;
    
    // Move cursor to the start of the input line
    vbe_cursor_x = 9 * 8;  // Move to after the prompt
    vbe_cursor_y = current_y;
    
    // Clear the entire line from prompt to end
    vbe_draw_rect(vbe_cursor_x, vbe_cursor_y, VBE_WIDTH - vbe_cursor_x, font_8x16.height, 0x00000000);
    
    // Reset command buffer and index
    memset(cmd_buffer, 0, MAX_CMD_LENGTH);
    cmd_index = 0;
}

