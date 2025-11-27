/*
 * KaiOS - VGA Text Mode Driver
 * Handles all screen output in text mode
 */

#include "include/drivers/vga.h"

// VGA text buffer address
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// Current state
static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;

// I/O port functions
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
    vga_enable_cursor(14, 15);
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
    vga_set_cursor(0, 0);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = vga_entry_color(fg, bg);
}

void vga_enable_cursor(uint8_t start, uint8_t end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void vga_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void vga_set_cursor(size_t x, size_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = VGA_MEMORY[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
            vga_row = VGA_HEIGHT - 1;
        }
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\t') {
        vga_column = (vga_column + 8) & ~7;
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
                vga_row = VGA_HEIGHT - 1;
            }
        }
    } else if (c == '\b') {
        if (vga_column > 0) {
            vga_column--;
            VGA_MEMORY[vga_row * VGA_WIDTH + vga_column] = vga_entry(' ', vga_color);
        }
    } else {
        VGA_MEMORY[vga_row * VGA_WIDTH + vga_column] = vga_entry(c, vga_color);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
                vga_row = VGA_HEIGHT - 1;
            }
        }
    }
    vga_set_cursor(vga_column, vga_row);
}

void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

void vga_writestring(const char* data) {
    size_t i = 0;
    while (data[i] != '\0') {
        vga_putchar(data[i]);
        i++;
    }
}

size_t vga_get_row(void) {
    return vga_row;
}

size_t vga_get_column(void) {
    return vga_column;
}
