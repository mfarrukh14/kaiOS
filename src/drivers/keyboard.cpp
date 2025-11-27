/*
 * KaiOS - Keyboard Driver
 * PS/2 keyboard input handling with scancode translation
 */

#include "include/drivers/keyboard.h"
#include "include/drivers/vga.h"

// I/O port functions
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Keyboard state
static bool shift_pressed = false;
static bool caps_lock = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;

// Keyboard buffer
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile size_t buffer_start = 0;
static volatile size_t buffer_end = 0;

// US keyboard layout - lowercase
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

// US keyboard layout - uppercase/shifted
static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

void keyboard_init(void) {
    // Wait for keyboard controller to be ready
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    
    // Enable keyboard
    outb(KEYBOARD_COMMAND_PORT, 0xAE);
    
    // Clear buffer
    buffer_start = 0;
    buffer_end = 0;
}

static void buffer_push(char c) {
    size_t next = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != buffer_start) {
        keyboard_buffer[buffer_end] = c;
        buffer_end = next;
    }
}

bool keyboard_has_input(void) {
    return buffer_start != buffer_end;
}

char keyboard_getchar(void) {
    while (buffer_start == buffer_end) {
        // Wait for input (with HLT to save CPU)
        __asm__ volatile("hlt");
    }
    
    char c = keyboard_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

char keyboard_read_scancode(void) {
    return inb(KEYBOARD_DATA_PORT);
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Key release
    if (scancode & 0x80) {
        scancode &= 0x7F;
        
        switch (scancode) {
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                shift_pressed = false;
                break;
            case KEY_LCTRL:
                ctrl_pressed = false;
                break;
            case KEY_LALT:
                alt_pressed = false;
                break;
        }
        return;
    }
    
    // Key press
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            shift_pressed = true;
            return;
        case KEY_LCTRL:
            ctrl_pressed = true;
            return;
        case KEY_LALT:
            alt_pressed = true;
            return;
        case KEY_CAPSLOCK:
            caps_lock = !caps_lock;
            return;
    }
    
    // Convert scancode to ASCII
    if (scancode < sizeof(scancode_to_ascii)) {
        char c;
        
        bool uppercase = shift_pressed != caps_lock;  // XOR
        
        if (uppercase) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
        
        if (c != 0) {
            // Handle Ctrl+C, Ctrl+D, etc.
            if (ctrl_pressed) {
                if (c >= 'a' && c <= 'z') {
                    c = c - 'a' + 1;
                } else if (c >= 'A' && c <= 'Z') {
                    c = c - 'A' + 1;
                }
            }
            
            buffer_push(c);
        }
    }
}
