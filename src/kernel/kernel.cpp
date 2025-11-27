/*
 * KaiOS - Kernel Main Entry Point
 * Initializes all kernel subsystems and starts the shell
 */

#include "include/kernel/types.h"
#include "include/kernel/idt.h"
#include "include/kernel/memory.h"
#include "include/kernel/fs.h"
#include "include/kernel/shell.h"
#include "include/kernel/gui.h"
#include "include/drivers/vga.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/timer.h"
#include "include/drivers/ata.h"
#include "include/drivers/mouse.h"

// Multiboot magic number check
#define MULTIBOOT_MAGIC 0x2BADB002

// Multiboot info structure (partial)
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;  // Physical address of command line
};

// Boot mode - can be changed to boot into shell instead
static bool gui_mode = true;

// Simple string compare
static bool str_contains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return true;
        haystack++;
    }
    return false;
}

// Forward declare keyboard handler
extern void keyboard_handler(void);

// Keyboard IRQ wrapper
static void keyboard_irq_handler(registers_t* regs) {
    (void)regs;
    keyboard_handler();
}

// Mouse IRQ wrapper
static void mouse_irq_handler(registers_t* regs) {
    (void)regs;
    mouse_handler();
}

extern "C" void kernel_main(uint32_t magic, multiboot_info* mboot_info) {
    // Initialize VGA display first (so we can show output)
    vga_init();
    
    // Clear screen and show boot message
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_writestring("KaiOS - Lightweight Operating System\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Booting kernel...\n\n");
    
    // Check multiboot magic and parse command line
    if (magic == MULTIBOOT_MAGIC) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_writestring("[OK] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("Multiboot verified\n");
        
        // Check if command line is present (bit 2 of flags)
        if (mboot_info && (mboot_info->flags & 0x04)) {
            const char* cmdline = (const char*)mboot_info->cmdline;
            if (str_contains(cmdline, "mode=term")) {
                gui_mode = false;
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_writestring("[OK] ");
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                vga_writestring("Boot mode: Terminal\n");
            } else if (str_contains(cmdline, "mode=gui")) {
                gui_mode = true;
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_writestring("[OK] ");
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                vga_writestring("Boot mode: GUI\n");
            }
        }
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_writestring("[WARN] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("Invalid multiboot magic\n");
    }
    
    // Initialize memory management
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing memory manager...\n");
    memory_init();
    
    // Initialize IDT (interrupts)
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing interrupt descriptor table...\n");
    idt_init();
    
    // Initialize timer
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing timer (100 Hz)...\n");
    timer_init(TIMER_FREQUENCY);
    
    // Initialize keyboard
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing keyboard driver...\n");
    keyboard_init();
    register_interrupt_handler(33, keyboard_irq_handler);  // IRQ1 = keyboard
    
    // Initialize mouse driver
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing mouse driver...\n");
    mouse_init();
    register_interrupt_handler(44, mouse_irq_handler);  // IRQ12 = mouse
    
    // Initialize ATA disk driver
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing ATA disk driver...\n");
    ata_init();
    
    if (ata_is_present()) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_writestring("[OK] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("ATA disk detected\n");
    } else {
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_writestring("[--] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("No ATA disk (using memory-only filesystem)\n");
    }
    
    // Initialize file system
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Initializing file system...\n");
    
    // Try to load filesystem from disk
    bool loaded_from_disk = false;
    if (ata_is_present()) {
        loaded_from_disk = fs_load();
    }
    
    if (loaded_from_disk) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_writestring("[OK] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("Filesystem loaded from disk\n");
    } else {
        // Create fresh filesystem
        fs_init();
        if (ata_is_present()) {
            vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
            vga_writestring("[--] ");
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_writestring("No saved filesystem, created new (use 'sync' to save)\n");
        } else {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_writestring("[OK] ");
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_writestring("Created new filesystem\n");
        }
    }
    
    // Enable interrupts
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("[OK] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_writestring("Enabling interrupts...\n");
    __asm__ volatile("sti");
    
    if (gui_mode) {
        // Show boot splash and start GUI
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_writestring("[OK] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("Starting graphical interface...\n");
        
        // Brief pause to show messages
        for (volatile int i = 0; i < 10000000; i++);
        
        // Show splash screen
        gui_show_splash();
        
        // Initialize and run GUI
        gui_init();
        gui_run();
    } else {
        // Initialize shell
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_writestring("[OK] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_writestring("Starting shell...\n");
        shell_init();
        
        // Run the shell (this never returns)
        shell_run();
    }
    
    // If we somehow get here, halt
    while (1) {
        __asm__ volatile("hlt");
    }
}
