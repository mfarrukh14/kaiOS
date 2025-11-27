/*
 * KaiOS - Shell (Terminal) Implementation
 * Command-line interface with built-in commands
 */

#include "include/kernel/shell.h"
#include "include/kernel/fs.h"
#include "include/kernel/memory.h"
#include "include/kernel/string.h"
#include "include/drivers/vga.h"
#include "include/drivers/keyboard.h"

// Shell state
static char input_buffer[SHELL_MAX_INPUT];
static size_t input_pos = 0;
static char* argv[SHELL_MAX_ARGS];
static int argc = 0;

// Simple uptime counter (ticks)
static uint32_t uptime_ticks = 0;

// Forward declaration
static void print_prompt(void);

// I/O helper
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void shell_init(void) {
    input_pos = 0;
    memset(input_buffer, 0, SHELL_MAX_INPUT);
}

static void print_prompt(void) {
    char path[FS_MAX_PATH];
    fs_get_path(fs_get_current(), path, FS_MAX_PATH);
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("kai");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_writestring(":");
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_writestring(path);
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_writestring("$ ");
}

static int parse_args(char* input) {
    argc = 0;
    char* ptr = input;
    bool in_quotes = false;
    
    while (*ptr != '\0' && argc < SHELL_MAX_ARGS) {
        // Skip whitespace
        while (*ptr == ' ' && !in_quotes) ptr++;
        
        if (*ptr == '\0') break;
        
        // Handle quotes
        if (*ptr == '"') {
            in_quotes = !in_quotes;
            ptr++;
            continue;
        }
        
        argv[argc++] = ptr;
        
        // Find end of argument
        while (*ptr != '\0' && (*ptr != ' ' || in_quotes)) {
            if (*ptr == '"') {
                in_quotes = !in_quotes;
                // Remove the quote
                memmove(ptr, ptr + 1, strlen(ptr));
            } else {
                ptr++;
            }
        }
        
        if (*ptr != '\0') {
            *ptr++ = '\0';
        }
    }
    
    return argc;
}

void shell_process_command(char* input) {
    // Trim leading/trailing whitespace
    while (*input == ' ') input++;
    size_t len = strlen(input);
    while (len > 0 && input[len - 1] == ' ') {
        input[--len] = '\0';
    }
    
    if (len == 0) return;
    
    parse_args(input);
    
    if (argc == 0) return;
    
    // Built-in commands
    if (strcmp(argv[0], "help") == 0) {
        cmd_help(argc, argv);
    } else if (strcmp(argv[0], "clear") == 0) {
        cmd_clear(argc, argv);
    } else if (strcmp(argv[0], "echo") == 0) {
        cmd_echo(argc, argv);
    } else if (strcmp(argv[0], "ls") == 0 || strcmp(argv[0], "dir") == 0) {
        cmd_ls(argc, argv);
    } else if (strcmp(argv[0], "cd") == 0) {
        cmd_cd(argc, argv);
    } else if (strcmp(argv[0], "pwd") == 0) {
        cmd_pwd(argc, argv);
    } else if (strcmp(argv[0], "mkdir") == 0) {
        cmd_mkdir(argc, argv);
    } else if (strcmp(argv[0], "rmdir") == 0) {
        cmd_rmdir(argc, argv);
    } else if (strcmp(argv[0], "touch") == 0) {
        cmd_touch(argc, argv);
    } else if (strcmp(argv[0], "rm") == 0) {
        cmd_rm(argc, argv);
    } else if (strcmp(argv[0], "cat") == 0) {
        cmd_cat(argc, argv);
    } else if (strcmp(argv[0], "write") == 0) {
        cmd_write(argc, argv);
    } else if (strcmp(argv[0], "cp") == 0) {
        cmd_cp(argc, argv);
    } else if (strcmp(argv[0], "mv") == 0) {
        cmd_mv(argc, argv);
    } else if (strcmp(argv[0], "free") == 0) {
        cmd_free(argc, argv);
    } else if (strcmp(argv[0], "uname") == 0) {
        cmd_uname(argc, argv);
    } else if (strcmp(argv[0], "date") == 0) {
        cmd_date(argc, argv);
    } else if (strcmp(argv[0], "uptime") == 0) {
        cmd_uptime(argc, argv);
    } else if (strcmp(argv[0], "reboot") == 0) {
        cmd_reboot(argc, argv);
    } else if (strcmp(argv[0], "shutdown") == 0 || strcmp(argv[0], "halt") == 0) {
        cmd_shutdown(argc, argv);
    } else if (strcmp(argv[0], "sync") == 0) {
        cmd_sync(argc, argv);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_writestring("Unknown command: ");
        vga_writestring(argv[0]);
        vga_writestring("\nType 'help' for available commands.\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

void shell_run(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_writestring("\n====================================\n");
    vga_writestring("               KaiOS v1.0          \n");
    vga_writestring("====================================\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Type 'help' for a list of commands.\n\n");
    
    print_prompt();
    
    while (1) {
        char c = keyboard_getchar();
        uptime_ticks++;
        
        if (c == '\n') {
            vga_putchar('\n');
            input_buffer[input_pos] = '\0';
            
            if (input_pos > 0) {
                shell_process_command(input_buffer);
            }
            
            input_pos = 0;
            memset(input_buffer, 0, SHELL_MAX_INPUT);
            print_prompt();
        } else if (c == '\b') {
            if (input_pos > 0) {
                input_pos--;
                input_buffer[input_pos] = '\0';
                vga_putchar('\b');
            }
        } else if (c == '\t') {
            // Tab completion (simple - just add spaces)
            if (input_pos < SHELL_MAX_INPUT - 4) {
                for (int i = 0; i < 4; i++) {
                    input_buffer[input_pos++] = ' ';
                    vga_putchar(' ');
                }
            }
        } else if (c >= 32 && c < 127) {
            if (input_pos < SHELL_MAX_INPUT - 1) {
                input_buffer[input_pos++] = c;
                vga_putchar(c);
            }
        } else if (c == 3) {
            // Ctrl+C - cancel current input
            vga_writestring("^C\n");
            input_pos = 0;
            memset(input_buffer, 0, SHELL_MAX_INPUT);
            print_prompt();
        }
    }
}

// Command implementations
void cmd_help(int argc, char** argv) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_writestring("KaiOS Shell Commands:\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("  help       - Show this help message\n");
    vga_writestring("  clear      - Clear the screen\n");
    vga_writestring("  echo       - Print text to screen\n");
    vga_writestring("  ls/dir     - List directory contents\n");
    vga_writestring("  cd <path>  - Change directory\n");
    vga_writestring("  pwd        - Print working directory\n");
    vga_writestring("  mkdir      - Create a directory\n");
    vga_writestring("  rmdir      - Remove a directory\n");
    vga_writestring("  touch      - Create an empty file\n");
    vga_writestring("  rm         - Remove a file\n");
    vga_writestring("  cat        - Display file contents\n");
    vga_writestring("  write      - Write text to file\n");
    vga_writestring("  cp         - Copy a file\n");
    vga_writestring("  mv         - Move/rename a file\n");
    vga_writestring("  sync       - Save filesystem to disk\n");
    vga_writestring("  free       - Show memory usage\n");
    vga_writestring("  uname      - Show system info\n");
    vga_writestring("  date       - Show current date\n");
    vga_writestring("  uptime     - Show system uptime\n");
    vga_writestring("  reboot     - Reboot the system\n");
    vga_writestring("  shutdown   - Halt the system\n");
}

void cmd_clear(int argc, char** argv) {
    vga_clear();
}

void cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        vga_writestring(argv[i]);
        if (i < argc - 1) vga_putchar(' ');
    }
    vga_putchar('\n');
}

void cmd_ls(int argc, char** argv) {
    fs_node_t* dir = fs_get_current();
    
    if (argc > 1) {
        dir = fs_resolve_path(argv[1]);
        if (dir == NULL || dir->type != FS_DIRECTORY) {
            vga_writestring("ls: cannot access '");
            vga_writestring(argv[1]);
            vga_writestring("': No such directory\n");
            return;
        }
    }
    
    size_t count;
    fs_node_t** children = fs_readdir(dir, &count);
    
    if (count == 0) {
        vga_writestring("(empty)\n");
        return;
    }
    
    for (size_t i = 0; i < count; i++) {
        if (children[i]->type == FS_DIRECTORY) {
            vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            vga_writestring(children[i]->name);
            vga_writestring("/");
        } else {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_writestring(children[i]->name);
        }
        
        // Show file size for files
        if (children[i]->type == FS_FILE) {
            vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
            vga_writestring(" (");
            char size_str[16];
            utoa(children[i]->size, size_str, 10);
            vga_writestring(size_str);
            vga_writestring(" bytes)");
        }
        
        vga_writestring("\n");
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void cmd_cd(int argc, char** argv) {
    if (argc < 2) {
        fs_chdir("/");
        return;
    }
    
    if (fs_chdir(argv[1]) != 0) {
        vga_writestring("cd: ");
        vga_writestring(argv[1]);
        vga_writestring(": No such directory\n");
    }
}

void cmd_pwd(int argc, char** argv) {
    char path[FS_MAX_PATH];
    fs_get_path(fs_get_current(), path, FS_MAX_PATH);
    vga_writestring(path);
    vga_putchar('\n');
}

void cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        vga_writestring("mkdir: missing operand\n");
        return;
    }
    
    if (fs_mkdir(argv[1]) == NULL) {
        vga_writestring("mkdir: cannot create directory '");
        vga_writestring(argv[1]);
        vga_writestring("'\n");
    }
}

void cmd_rmdir(int argc, char** argv) {
    if (argc < 2) {
        vga_writestring("rmdir: missing operand\n");
        return;
    }
    
    int result = fs_rmdir(argv[1]);
    if (result == -1) {
        vga_writestring("rmdir: ");
        vga_writestring(argv[1]);
        vga_writestring(": No such directory\n");
    } else if (result == -2) {
        vga_writestring("rmdir: ");
        vga_writestring(argv[1]);
        vga_writestring(": Directory not empty\n");
    }
}

void cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        vga_writestring("touch: missing operand\n");
        return;
    }
    
    if (fs_create(argv[1]) == NULL) {
        vga_writestring("touch: cannot create file '");
        vga_writestring(argv[1]);
        vga_writestring("'\n");
    }
}

void cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        vga_writestring("rm: missing operand\n");
        return;
    }
    
    if (fs_delete(argv[1]) != 0) {
        vga_writestring("rm: cannot remove '");
        vga_writestring(argv[1]);
        vga_writestring("'\n");
    }
}

void cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        vga_writestring("cat: missing operand\n");
        return;
    }
    
    fs_node_t* file = fs_find(argv[1]);
    if (file == NULL) {
        vga_writestring("cat: ");
        vga_writestring(argv[1]);
        vga_writestring(": No such file\n");
        return;
    }
    
    if (file->type == FS_DIRECTORY) {
        vga_writestring("cat: ");
        vga_writestring(argv[1]);
        vga_writestring(": Is a directory\n");
        return;
    }
    
    if (file->size == 0) {
        return;  // Empty file
    }
    
    char* buffer = (char*)kmalloc(file->size + 1);
    if (buffer == NULL) {
        vga_writestring("cat: out of memory\n");
        return;
    }
    
    fs_read(file, buffer, file->size, 0);
    buffer[file->size] = '\0';
    vga_writestring(buffer);
    if (buffer[file->size - 1] != '\n') {
        vga_putchar('\n');
    }
    
    kfree(buffer);
}

void cmd_write(int argc, char** argv) {
    if (argc < 3) {
        vga_writestring("write: usage: write <file> <text>\n");
        return;
    }
    
    fs_node_t* file = fs_find(argv[1]);
    if (file == NULL) {
        file = fs_create(argv[1]);
        if (file == NULL) {
            vga_writestring("write: cannot create file\n");
            return;
        }
    }
    
    // Concatenate all remaining arguments
    char content[1024];
    content[0] = '\0';
    for (int i = 2; i < argc; i++) {
        strcat(content, argv[i]);
        if (i < argc - 1) strcat(content, " ");
    }
    strcat(content, "\n");
    
    if (fs_write(file, content, strlen(content)) != 0) {
        vga_writestring("write: failed to write to file\n");
    }
}

void cmd_cp(int argc, char** argv) {
    if (argc < 3) {
        vga_writestring("cp: usage: cp <source> <dest>\n");
        return;
    }
    
    if (fs_copy(argv[1], argv[2]) != 0) {
        vga_writestring("cp: cannot copy '");
        vga_writestring(argv[1]);
        vga_writestring("'\n");
    }
}

void cmd_mv(int argc, char** argv) {
    if (argc < 3) {
        vga_writestring("mv: usage: mv <source> <dest>\n");
        return;
    }
    
    if (fs_rename(argv[1], argv[2]) != 0) {
        vga_writestring("mv: cannot move '");
        vga_writestring(argv[1]);
        vga_writestring("'\n");
    }
}

void cmd_free(int argc, char** argv) {
    char buffer[32];
    
    vga_writestring("Memory Usage:\n");
    vga_writestring("  Used: ");
    utoa(memory_used(), buffer, 10);
    vga_writestring(buffer);
    vga_writestring(" bytes\n");
    vga_writestring("  Free: ");
    utoa(memory_free(), buffer, 10);
    vga_writestring(buffer);
    vga_writestring(" bytes\n");
}

void cmd_uname(int argc, char** argv) {
    bool all = (argc > 1 && strcmp(argv[1], "-a") == 0);
    
    vga_writestring("KaiOS");
    if (all) {
        vga_writestring(" kai 1.0.0 ");
        vga_writestring("i686 ");
        vga_writestring("KaiOS/x86");
    }
    vga_putchar('\n');
}

void cmd_date(int argc, char** argv) {
    // We don't have RTC driver, just show a placeholder
    vga_writestring("Thu Nov 27 12:00:00 UTC 2025\n");
}

void cmd_uptime(int argc, char** argv) {
    char buffer[32];
    vga_writestring("System uptime: ");
    utoa(uptime_ticks / 100, buffer, 10);  // Approximate seconds
    vga_writestring(buffer);
    vga_writestring(" seconds (approx)\n");
}

void cmd_reboot(int argc, char** argv) {
    // Auto-save filesystem before reboot
    if (fs_has_disk()) {
        vga_writestring("Saving filesystem...\n");
        if (fs_save()) {
            vga_writestring("Filesystem saved.\n");
        } else {
            vga_writestring("Warning: Failed to save filesystem.\n");
        }
    }
    
    vga_writestring("Rebooting...\n");
    
    // Triple fault to reboot (keyboard controller reset)
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = 0x64;
    }
    outb(0x64, 0xFE);
    
    // If that doesn't work, halt
    while (1) {
        __asm__ volatile("hlt");
    }
}

void cmd_shutdown(int argc, char** argv) {
    // Save filesystem before shutdown
    vga_writestring("Saving filesystem...\n");
    if (fs_save()) {
        vga_writestring("Filesystem saved.\n");
    }
    
    vga_writestring("System halted. You can power off now.\n");
    
    // Disable interrupts and halt
    __asm__ volatile("cli");
    while (1) {
        __asm__ volatile("hlt");
    }
}

void cmd_sync(int argc, char** argv) {
    vga_writestring("Syncing filesystem to disk...\n");
    
    if (!fs_has_disk()) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_writestring("Error: No disk available. Files only in memory.\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        return;
    }
    
    if (fs_save()) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_writestring("Filesystem saved to disk successfully!\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_writestring("Error: Failed to save filesystem to disk.\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}
