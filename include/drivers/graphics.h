/*
 * KaiOS - VGA Graphics Driver Header
 * Mode 13h (320x200 256 colors) graphics support
 */

#ifndef KAIOS_GRAPHICS_H
#define KAIOS_GRAPHICS_H

#include "include/kernel/types.h"

// Screen dimensions (Mode 13h)
#define GFX_WIDTH   320
#define GFX_HEIGHT  200
#define GFX_BPP     8

// Standard 256-color palette indices
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GRAY    7
#define COLOR_DARK_GRAY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

// Custom colors (set in palette)
#define COLOR_DESKTOP_BG    16
#define COLOR_TASKBAR       17
#define COLOR_TASKBAR_DARK  18
#define COLOR_WINDOW_BG     19
#define COLOR_WINDOW_TITLE  20
#define COLOR_BUTTON        21
#define COLOR_BUTTON_LIGHT  22
#define COLOR_BUTTON_DARK   23
#define COLOR_MENU_BG       24
#define COLOR_MENU_SELECT   25
#define COLOR_ICON_FOLDER   26
#define COLOR_ICON_FILE     27

// Rectangle structure
typedef struct {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} rect_t;

// Graphics mode functions
void gfx_init(void);
void gfx_exit(void);
bool gfx_is_initialized(void);

// Basic drawing
void gfx_clear(uint8_t color);
void gfx_putpixel(int16_t x, int16_t y, uint8_t color);
uint8_t gfx_getpixel(int16_t x, int16_t y);
void gfx_swap_buffers(void);

// Shape drawing
void gfx_hline(int16_t x, int16_t y, int16_t width, uint8_t color);
void gfx_vline(int16_t x, int16_t y, int16_t height, uint8_t color);
void gfx_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void gfx_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void gfx_fillrect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void gfx_rect_3d(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t light, uint8_t dark, bool raised);

// Text drawing (8x8 font)
void gfx_putchar(int16_t x, int16_t y, char c, uint8_t fg, uint8_t bg);
void gfx_puts(int16_t x, int16_t y, const char* str, uint8_t fg, uint8_t bg);
int gfx_text_width(const char* str);

// Bitmap drawing
void gfx_blit(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data);
void gfx_blit_transparent(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data, uint8_t trans_color);

// Palette
void gfx_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void gfx_setup_palette(void);

// Clipping
void gfx_set_clip(int16_t x, int16_t y, int16_t w, int16_t h);
void gfx_reset_clip(void);

#endif // KAIOS_GRAPHICS_H
