/*
 * KaiOS - VGA Text Mode Driver Header
 * Provides screen output functionality
 */

#ifndef KAIOS_VGA_H
#define KAIOS_VGA_H

#include "include/kernel/types.h"

// VGA text mode dimensions
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

// VGA colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// VGA functions
void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_write(const char* data, size_t size);
void vga_writestring(const char* data);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_set_cursor(size_t x, size_t y);
void vga_enable_cursor(uint8_t start, uint8_t end);
void vga_disable_cursor(void);
void vga_scroll(void);
size_t vga_get_row(void);
size_t vga_get_column(void);

// Helper macros
#define vga_entry_color(fg, bg) ((fg) | ((bg) << 4))
#define vga_entry(c, color) ((uint16_t)(c) | ((uint16_t)(color) << 8))

#endif // KAIOS_VGA_H
