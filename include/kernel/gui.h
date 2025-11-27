/*
 * KaiOS - GUI System Header
 * Window manager, widgets, and UI components
 */

#ifndef KAIOS_GUI_H
#define KAIOS_GUI_H

#include "include/kernel/types.h"
#include "include/drivers/graphics.h"

// GUI constants
#define GUI_MAX_WINDOWS     16
#define GUI_MAX_WIDGETS     64
#define GUI_TITLE_HEIGHT    14
#define GUI_BORDER_WIDTH    2
#define GUI_BUTTON_HEIGHT   16
#define GUI_TASKBAR_HEIGHT  20
#define GUI_STARTMENU_WIDTH 100
#define GUI_ICON_SIZE       32

// Widget types
typedef enum {
    WIDGET_NONE = 0,
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_ICON,
    WIDGET_LIST,
    WIDGET_TEXTBOX
} widget_type_t;

// Widget state flags
#define WIDGET_VISIBLE   0x01
#define WIDGET_ENABLED   0x02
#define WIDGET_FOCUSED   0x04
#define WIDGET_HOVERED   0x08
#define WIDGET_PRESSED   0x10

// Forward declarations
struct window_t;
struct widget_t;

// Event callback type
typedef void (*widget_callback_t)(struct widget_t* widget);
typedef void (*window_callback_t)(struct window_t* window);

// Widget structure
typedef struct widget_t {
    widget_type_t type;
    int16_t x, y;           // Position relative to window
    int16_t width, height;
    uint8_t flags;
    char text[32];          // Label/button text
    uint8_t icon_id;        // Icon index
    widget_callback_t on_click;
    void* user_data;
} widget_t;

// Window structure
typedef struct window_t {
    int16_t x, y;
    int16_t width, height;
    char title[32];
    uint8_t flags;
    bool visible;
    bool minimized;
    bool dragging;
    int16_t drag_offset_x, drag_offset_y;
    widget_t widgets[16];
    uint8_t widget_count;
    window_callback_t on_close;
    void* user_data;
} window_t;

// Menu item
typedef struct {
    char text[24];
    uint8_t icon_id;
    void (*action)(void);
} menu_item_t;

// Start menu
typedef struct {
    bool visible;
    menu_item_t items[8];
    uint8_t item_count;
    int8_t hover_index;
} start_menu_t;

// Desktop icon
typedef struct {
    int16_t x, y;
    char name[16];
    uint8_t icon_id;
    bool selected;
    void (*action)(void);
} desktop_icon_t;

// GUI state
typedef struct {
    bool initialized;
    window_t* windows[GUI_MAX_WINDOWS];
    uint8_t window_count;
    window_t* active_window;
    window_t* dragging_window;
    start_menu_t start_menu;
    desktop_icon_t icons[8];
    uint8_t icon_count;
    int16_t mouse_x, mouse_y;
    bool mouse_down;
    bool redraw_needed;
} gui_state_t;

// Icons (built-in)
#define ICON_FOLDER     0
#define ICON_FILE       1
#define ICON_COMPUTER   2
#define ICON_TERMINAL   3
#define ICON_SETTINGS   4
#define ICON_SHUTDOWN   5
#define ICON_START      6
#define ICON_CLOSE      7

// GUI initialization
void gui_init(void);
void gui_shutdown(void);

// Main loop
void gui_update(void);
void gui_draw(void);
void gui_run(void);

// Window management
window_t* gui_create_window(const char* title, int16_t x, int16_t y, int16_t w, int16_t h);
void gui_destroy_window(window_t* window);
void gui_show_window(window_t* window);
void gui_hide_window(window_t* window);
void gui_focus_window(window_t* window);
void gui_minimize_window(window_t* window);

// Widget creation
widget_t* gui_add_button(window_t* window, int16_t x, int16_t y, int16_t w, int16_t h, const char* text);
widget_t* gui_add_label(window_t* window, int16_t x, int16_t y, const char* text);
widget_t* gui_add_icon(window_t* window, int16_t x, int16_t y, uint8_t icon_id, const char* label);

// Desktop icons
void gui_add_desktop_icon(int16_t x, int16_t y, const char* name, uint8_t icon_id, void (*action)(void));

// Start menu
void gui_add_start_item(const char* text, uint8_t icon_id, void (*action)(void));
void gui_toggle_start_menu(void);

// Drawing helpers
void gui_draw_window(window_t* window);
void gui_draw_widget(window_t* window, widget_t* widget);
void gui_draw_icon(int16_t x, int16_t y, uint8_t icon_id, bool selected);
void gui_draw_cursor(int16_t x, int16_t y);
void gui_draw_taskbar(void);
void gui_draw_start_menu(void);
void gui_draw_desktop(void);

// Event handling
void gui_handle_mouse_move(int16_t x, int16_t y);
void gui_handle_mouse_down(int16_t x, int16_t y, uint8_t button);
void gui_handle_mouse_up(int16_t x, int16_t y, uint8_t button);
void gui_handle_key(char key);

// Boot splash
void gui_show_splash(void);

#endif // KAIOS_GUI_H
