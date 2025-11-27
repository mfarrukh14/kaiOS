/*
 * KaiOS - GUI System Implementation
 * Desktop environment with windows, taskbar, start menu
 */

#include "include/kernel/gui.h"
#include "include/kernel/fs.h"
#include "include/kernel/memory.h"
#include "include/kernel/string.h"
#include "include/drivers/graphics.h"
#include "include/drivers/mouse.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/timer.h"

// Global GUI state
static gui_state_t gui;

// Cursor bitmap (8x11 - smaller cursor)
static const uint8_t cursor_data[11][8] = {
    {1,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0},
    {1,2,2,1,0,0,0,0},
    {1,2,2,2,1,0,0,0},
    {1,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,1,0},
    {1,2,2,2,1,1,0,0},
    {1,2,1,1,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
};

// Simple icon bitmaps (16x16)
static const uint8_t icon_folder[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,6,6,6,6,0,0,0,0,0,0,0,0,0,0},
    {0,6,14,14,14,14,6,6,6,6,6,6,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,14,14,14,14,14,14,14,14,14,14,6,0,0,0},
    {0,6,6,6,6,6,6,6,6,6,6,6,6,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t icon_file[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,7,7,7,7,7,7,7,0,0,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,7,7,0,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,15,7,7,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,15,15,7,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,15,15,7,0,0,0,0,0},
    {0,0,7,15,8,8,8,8,15,15,7,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,15,15,7,0,0,0,0,0},
    {0,0,7,15,8,8,8,8,8,15,7,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,15,15,7,0,0,0,0,0},
    {0,0,7,15,8,8,8,15,15,15,7,0,0,0,0,0},
    {0,0,7,15,15,15,15,15,15,15,7,0,0,0,0,0},
    {0,0,7,7,7,7,7,7,7,7,7,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t icon_computer[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0},
    {0,8,9,9,9,9,9,9,9,9,9,9,8,0,0,0},
    {0,8,9,1,1,1,1,1,1,1,1,9,8,0,0,0},
    {0,8,9,1,11,11,11,11,11,1,1,9,8,0,0,0},
    {0,8,9,1,11,11,11,11,11,1,1,9,8,0,0,0},
    {0,8,9,1,11,11,11,11,11,1,1,9,8,0,0,0},
    {0,8,9,1,1,1,1,1,1,1,1,9,8,0,0,0},
    {0,8,9,9,9,9,9,9,9,9,9,9,8,0,0,0},
    {0,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0},
    {0,0,0,0,8,8,8,8,8,0,0,0,0,0,0,0},
    {0,0,8,8,8,8,8,8,8,8,8,0,0,0,0,0},
    {0,0,8,8,8,8,8,8,8,8,8,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t icon_terminal[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0},
    {0,8,0,0,0,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,10,0,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,0,10,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,0,0,10,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,0,10,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,10,0,10,10,10,0,0,0,0,8,0,0,0},
    {0,8,0,0,0,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,0,0,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,0,0,0,0,0,0,0,0,0,0,8,0,0,0},
    {0,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

// Forward declarations
static void file_manager_open(void);
static void terminal_open(void);
static void do_shutdown(void);

// Draw a 16x16 icon
void gui_draw_icon(int16_t x, int16_t y, uint8_t icon_id, bool selected) {
    const uint8_t (*icon)[16] = NULL;
    
    switch (icon_id) {
        case ICON_FOLDER:   icon = icon_folder; break;
        case ICON_FILE:     icon = icon_file; break;
        case ICON_COMPUTER: icon = icon_computer; break;
        case ICON_TERMINAL: icon = icon_terminal; break;
        default:            icon = icon_file; break;
    }
    
    // Draw selection background if selected
    if (selected) {
        gfx_fillrect(x - 2, y - 2, 20, 20, COLOR_MENU_SELECT);
    }
    
    // Draw icon
    for (int16_t row = 0; row < 16; row++) {
        for (int16_t col = 0; col < 16; col++) {
            uint8_t pixel = icon[row][col];
            if (pixel != 0) {
                gfx_putpixel(x + col, y + row, pixel);
            }
        }
    }
}

void gui_draw_cursor(int16_t x, int16_t y) {
    for (int16_t row = 0; row < 11; row++) {
        for (int16_t col = 0; col < 8; col++) {
            uint8_t pixel = cursor_data[row][col];
            if (pixel == 1) {
                gfx_putpixel(x + col, y + row, COLOR_BLACK);
            } else if (pixel == 2) {
                gfx_putpixel(x + col, y + row, COLOR_WHITE);
            }
        }
    }
}

void gui_init(void) {
    memset(&gui, 0, sizeof(gui_state_t));
    
    // Graphics already initialized by splash screen, just setup palette
    gfx_setup_palette();
    
    // Initialize mouse
    mouse_init();
    mouse_set_bounds(GFX_WIDTH, GFX_HEIGHT);
    mouse_set_position(GFX_WIDTH / 2, GFX_HEIGHT / 2);
    
    gui.mouse_x = GFX_WIDTH / 2;
    gui.mouse_y = GFX_HEIGHT / 2;
    
    // Add desktop icons
    gui_add_desktop_icon(20, 30, "My PC", ICON_COMPUTER, file_manager_open);
    gui_add_desktop_icon(20, 80, "Files", ICON_FOLDER, file_manager_open);
    gui_add_desktop_icon(20, 130, "Terminal", ICON_TERMINAL, terminal_open);
    
    // Add start menu items
    gui_add_start_item("Files", ICON_FOLDER, file_manager_open);
    gui_add_start_item("Terminal", ICON_TERMINAL, terminal_open);
    gui_add_start_item("Shutdown", ICON_FILE, do_shutdown);
    
    gui.initialized = true;
    gui.redraw_needed = true;
    
    // Force initial draw
    gui_draw();
}

void gui_shutdown(void) {
    gui.initialized = false;
    gfx_exit();
}

void gui_add_desktop_icon(int16_t x, int16_t y, const char* name, uint8_t icon_id, void (*action)(void)) {
    if (gui.icon_count >= 8) return;
    
    desktop_icon_t* icon = &gui.icons[gui.icon_count++];
    icon->x = x;
    icon->y = y;
    strncpy(icon->name, name, 15);
    icon->name[15] = '\0';
    icon->icon_id = icon_id;
    icon->selected = false;
    icon->action = action;
}

void gui_add_start_item(const char* text, uint8_t icon_id, void (*action)(void)) {
    if (gui.start_menu.item_count >= 8) return;
    
    menu_item_t* item = &gui.start_menu.items[gui.start_menu.item_count++];
    strncpy(item->text, text, 23);
    item->text[23] = '\0';
    item->icon_id = icon_id;
    item->action = action;
}

void gui_toggle_start_menu(void) {
    gui.start_menu.visible = !gui.start_menu.visible;
    gui.start_menu.hover_index = -1;
    gui.redraw_needed = true;
}

window_t* gui_create_window(const char* title, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (gui.window_count >= GUI_MAX_WINDOWS) return NULL;
    
    window_t* window = (window_t*)kmalloc(sizeof(window_t));
    if (!window) return NULL;
    
    memset(window, 0, sizeof(window_t));
    window->x = x;
    window->y = y;
    window->width = w;
    window->height = h;
    strncpy(window->title, title, 31);
    window->title[31] = '\0';
    window->visible = true;
    window->flags = WIDGET_VISIBLE | WIDGET_ENABLED;
    
    gui.windows[gui.window_count++] = window;
    gui.active_window = window;
    gui.redraw_needed = true;
    
    return window;
}

void gui_destroy_window(window_t* window) {
    if (!window) return;
    
    // Find and remove from list
    for (uint8_t i = 0; i < gui.window_count; i++) {
        if (gui.windows[i] == window) {
            // Shift remaining windows
            for (uint8_t j = i; j < gui.window_count - 1; j++) {
                gui.windows[j] = gui.windows[j + 1];
            }
            gui.window_count--;
            break;
        }
    }
    
    if (gui.active_window == window) {
        gui.active_window = gui.window_count > 0 ? gui.windows[gui.window_count - 1] : NULL;
    }
    
    kfree(window);
    gui.redraw_needed = true;
}

void gui_show_window(window_t* window) {
    if (window) {
        window->visible = true;
        window->minimized = false;
        gui.redraw_needed = true;
    }
}

void gui_hide_window(window_t* window) {
    if (window) {
        window->visible = false;
        gui.redraw_needed = true;
    }
}

void gui_focus_window(window_t* window) {
    if (!window) return;
    
    // Move window to top
    for (uint8_t i = 0; i < gui.window_count; i++) {
        if (gui.windows[i] == window) {
            for (uint8_t j = i; j < gui.window_count - 1; j++) {
                gui.windows[j] = gui.windows[j + 1];
            }
            gui.windows[gui.window_count - 1] = window;
            break;
        }
    }
    
    gui.active_window = window;
    gui.redraw_needed = true;
}

widget_t* gui_add_button(window_t* window, int16_t x, int16_t y, int16_t w, int16_t h, const char* text) {
    if (!window || window->widget_count >= 16) return NULL;
    
    widget_t* widget = &window->widgets[window->widget_count++];
    widget->type = WIDGET_BUTTON;
    widget->x = x;
    widget->y = y;
    widget->width = w;
    widget->height = h;
    widget->flags = WIDGET_VISIBLE | WIDGET_ENABLED;
    strncpy(widget->text, text, 31);
    widget->text[31] = '\0';
    
    return widget;
}

widget_t* gui_add_label(window_t* window, int16_t x, int16_t y, const char* text) {
    if (!window || window->widget_count >= 16) return NULL;
    
    widget_t* widget = &window->widgets[window->widget_count++];
    widget->type = WIDGET_LABEL;
    widget->x = x;
    widget->y = y;
    widget->width = gfx_text_width(text);
    widget->height = 8;
    widget->flags = WIDGET_VISIBLE;
    strncpy(widget->text, text, 31);
    widget->text[31] = '\0';
    
    return widget;
}

widget_t* gui_add_icon(window_t* window, int16_t x, int16_t y, uint8_t icon_id, const char* label) {
    if (!window || window->widget_count >= 16) return NULL;
    
    widget_t* widget = &window->widgets[window->widget_count++];
    widget->type = WIDGET_ICON;
    widget->x = x;
    widget->y = y;
    widget->width = 16;
    widget->height = 16;
    widget->flags = WIDGET_VISIBLE;
    widget->icon_id = icon_id;
    if (label) {
        strncpy(widget->text, label, 31);
        widget->text[31] = '\0';
    }
    
    return widget;
}

void gui_draw_taskbar(void) {
    int16_t y = GFX_HEIGHT - GUI_TASKBAR_HEIGHT;
    
    // Taskbar background
    gfx_fillrect(0, y, GFX_WIDTH, GUI_TASKBAR_HEIGHT, COLOR_TASKBAR);
    gfx_hline(0, y, GFX_WIDTH, COLOR_TASKBAR_DARK);
    
    // Start button
    bool start_hover = (gui.mouse_x < 50 && gui.mouse_y >= y);
    uint8_t start_color = start_hover || gui.start_menu.visible ? COLOR_MENU_SELECT : COLOR_BUTTON;
    gfx_fillrect(2, y + 2, 46, GUI_TASKBAR_HEIGHT - 4, start_color);
    gfx_rect_3d(2, y + 2, 46, GUI_TASKBAR_HEIGHT - 4, COLOR_BUTTON_LIGHT, COLOR_BUTTON_DARK, !gui.start_menu.visible);
    gfx_puts(8, y + 6, "Start", COLOR_WHITE, 255);
    
    // Window buttons in taskbar
    int16_t btn_x = 54;
    for (uint8_t i = 0; i < gui.window_count; i++) {
        window_t* win = gui.windows[i];
        if (!win->visible && !win->minimized) continue;
        
        uint8_t btn_color = (win == gui.active_window) ? COLOR_MENU_SELECT : COLOR_BUTTON;
        gfx_fillrect(btn_x, y + 2, 60, GUI_TASKBAR_HEIGHT - 4, btn_color);
        gfx_rect_3d(btn_x, y + 2, 60, GUI_TASKBAR_HEIGHT - 4, COLOR_BUTTON_LIGHT, COLOR_BUTTON_DARK, true);
        
        // Truncate title if needed
        char short_title[8];
        strncpy(short_title, win->title, 7);
        short_title[7] = '\0';
        gfx_puts(btn_x + 4, y + 6, short_title, COLOR_WHITE, 255);
        
        btn_x += 64;
        if (btn_x > GFX_WIDTH - 70) break;
    }
    
    // Clock area (just show uptime for now)
    uint32_t uptime = timer_get_ticks() / 100;
    char time_str[16];
    
    // Debug: Show mouse packet count
    // sprintf(time_str, "%d", mouse_packet_count);
    
    // Format time MM:SS
    time_str[0] = '0' + (uptime / 600) % 6;
    time_str[1] = '0' + (uptime / 60) % 10;
    time_str[2] = ':';
    time_str[3] = '0' + (uptime % 60) / 10;
    time_str[4] = '0' + (uptime % 60) % 10;
    time_str[5] = '\0';
    
    gfx_puts(GFX_WIDTH - 40, y + 6, time_str, COLOR_WHITE, 255);
    
    // Debug: Show mouse coordinates in top right
    char debug_str[32];
    // Simple int to string conversion
    int mx = gui.mouse_x;
    int my = gui.mouse_y;
    int mp = mouse_packet_count;
    
    // Manual itoa since we don't have sprintf
    int idx = 0;
    debug_str[idx++] = 'M';
    debug_str[idx++] = ':';
    
    // Packet count
    if (mp == 0) debug_str[idx++] = '0';
    else {
        int temp = mp;
        int digits = 0;
        while (temp > 0) { temp /= 10; digits++; }
        temp = mp;
        for (int i = 0; i < digits; i++) {
            int p = 1;
            for (int j = 0; j < digits - 1 - i; j++) p *= 10;
            debug_str[idx++] = '0' + (temp / p) % 10;
        }
    }
    debug_str[idx] = '\0';
    
    gfx_puts(GFX_WIDTH - 80, 2, debug_str, COLOR_WHITE, 0); // Transparent bg
}

void gui_draw_start_menu(void) {
    if (!gui.start_menu.visible) return;
    
    int16_t menu_h = gui.start_menu.item_count * 20 + 4;
    int16_t menu_y = GFX_HEIGHT - GUI_TASKBAR_HEIGHT - menu_h;
    
    // Menu background
    gfx_fillrect(2, menu_y, GUI_STARTMENU_WIDTH, menu_h, COLOR_MENU_BG);
    gfx_rect(2, menu_y, GUI_STARTMENU_WIDTH, menu_h, COLOR_BUTTON_DARK);
    
    // Menu items
    for (uint8_t i = 0; i < gui.start_menu.item_count; i++) {
        int16_t item_y = menu_y + 2 + i * 20;
        
        if (i == gui.start_menu.hover_index) {
            gfx_fillrect(4, item_y, GUI_STARTMENU_WIDTH - 4, 18, COLOR_MENU_SELECT);
        }
        
        // Icon
        gui_draw_icon(8, item_y + 1, gui.start_menu.items[i].icon_id, false);
        
        // Text
        gfx_puts(28, item_y + 5, gui.start_menu.items[i].text, COLOR_WHITE, 255);
    }
}

void gui_draw_desktop(void) {
    // Desktop background - Windows 2000 Blue
    gfx_clear(1); // Dark blue
    
    // Draw desktop icons
    for (uint8_t i = 0; i < gui.icon_count; i++) {
        desktop_icon_t* icon = &gui.icons[i];
        
        // Selection highlight
        if (icon->selected) {
            gfx_fillrect(icon->x - 4, icon->y - 4, 40, 44, COLOR_MENU_SELECT);
        }
        
        // Icon
        gui_draw_icon(icon->x, icon->y, icon->icon_id, false);
        
        // Label (centered below icon)
        int16_t text_w = gfx_text_width(icon->name);
        int16_t text_x = icon->x + 8 - text_w / 2;
        gfx_puts(text_x, icon->y + 20, icon->name, COLOR_WHITE, 255);
    }
}

void gui_draw_window(window_t* window) {
    if (!window || !window->visible || window->minimized) return;
    
    int16_t x = window->x;
    int16_t y = window->y;
    int16_t w = window->width;
    int16_t h = window->height;
    
    // Window shadow
    gfx_fillrect(x + 2, y + 2, w, h, COLOR_BLACK);
    
    // Window background
    gfx_fillrect(x, y, w, h, COLOR_WINDOW_BG);
    
    // Title bar
    uint8_t title_color = (window == gui.active_window) ? COLOR_WINDOW_TITLE : COLOR_BUTTON_DARK;
    gfx_fillrect(x, y, w, GUI_TITLE_HEIGHT, title_color);
    
    // Title text
    gfx_puts(x + 4, y + 3, window->title, COLOR_WHITE, 255);
    
    // Close button
    int16_t close_x = x + w - 14;
    int16_t close_y = y + 2;
    gfx_fillrect(close_x, close_y, 12, 10, COLOR_LIGHT_RED);
    gfx_rect_3d(close_x, close_y, 12, 10, COLOR_BUTTON_LIGHT, COLOR_BUTTON_DARK, true);
    gfx_putchar(close_x + 2, close_y + 1, 'X', COLOR_WHITE, 255);
    
    // Window border
    gfx_rect(x, y, w, h, COLOR_BUTTON_DARK);
    
    // Draw widgets
    for (uint8_t i = 0; i < window->widget_count; i++) {
        gui_draw_widget(window, &window->widgets[i]);
    }
}

void gui_draw_widget(window_t* window, widget_t* widget) {
    if (!(widget->flags & WIDGET_VISIBLE)) return;
    
    int16_t x = window->x + widget->x;
    int16_t y = window->y + GUI_TITLE_HEIGHT + widget->y;
    
    switch (widget->type) {
        case WIDGET_BUTTON: {
            bool pressed = (widget->flags & WIDGET_PRESSED);
            uint8_t bg = (widget->flags & WIDGET_HOVERED) ? COLOR_MENU_SELECT : COLOR_BUTTON;
            gfx_fillrect(x, y, widget->width, widget->height, bg);
            gfx_rect_3d(x, y, widget->width, widget->height, 
                       COLOR_BUTTON_LIGHT, COLOR_BUTTON_DARK, !pressed);
            
            // Center text
            int16_t text_w = gfx_text_width(widget->text);
            int16_t text_x = x + (widget->width - text_w) / 2;
            int16_t text_y = y + (widget->height - 8) / 2;
            gfx_puts(text_x, text_y, widget->text, COLOR_WHITE, 255);
            break;
        }
        case WIDGET_LABEL:
            gfx_puts(x, y, widget->text, COLOR_WHITE, 255);
            break;
        case WIDGET_ICON:
            gui_draw_icon(x, y, widget->icon_id, widget->flags & WIDGET_HOVERED);
            break;
        default:
            break;
    }
}

void gui_draw(void) {
    // Draw desktop
    gui_draw_desktop();
    
    // Draw windows (back to front)
    for (uint8_t i = 0; i < gui.window_count; i++) {
        gui_draw_window(gui.windows[i]);
    }
    
    // Draw taskbar
    gui_draw_taskbar();
    
    // Draw start menu
    gui_draw_start_menu();
    
    // Draw cursor last
    mouse_state_t* mouse = mouse_get_state();
    gui_draw_cursor(mouse->x, mouse->y);
    
    // Swap buffers
    gfx_swap_buffers();
}

void gui_handle_mouse_move(int16_t x, int16_t y) {
    gui.mouse_x = x;
    gui.mouse_y = y;
    
    // Handle window dragging
    if (gui.dragging_window) {
        gui.dragging_window->x = x - gui.dragging_window->drag_offset_x;
        gui.dragging_window->y = y - gui.dragging_window->drag_offset_y;
        
        // Keep on screen
        if (gui.dragging_window->x < 0) gui.dragging_window->x = 0;
        if (gui.dragging_window->y < 0) gui.dragging_window->y = 0;
        if (gui.dragging_window->x + gui.dragging_window->width > GFX_WIDTH)
            gui.dragging_window->x = GFX_WIDTH - gui.dragging_window->width;
        if (gui.dragging_window->y + gui.dragging_window->height > GFX_HEIGHT - GUI_TASKBAR_HEIGHT)
            gui.dragging_window->y = GFX_HEIGHT - GUI_TASKBAR_HEIGHT - gui.dragging_window->height;
        
        gui.redraw_needed = true;
        return;
    }
    
    // Update start menu hover
    if (gui.start_menu.visible) {
        int16_t menu_h = gui.start_menu.item_count * 20 + 4;
        int16_t menu_y = GFX_HEIGHT - GUI_TASKBAR_HEIGHT - menu_h;
        
        if (x >= 2 && x < 2 + GUI_STARTMENU_WIDTH && y >= menu_y && y < menu_y + menu_h) {
            gui.start_menu.hover_index = (y - menu_y - 2) / 20;
            if (gui.start_menu.hover_index >= gui.start_menu.item_count) {
                gui.start_menu.hover_index = -1;
            }
        } else {
            gui.start_menu.hover_index = -1;
        }
        gui.redraw_needed = true;
    }
    
    // Update widget hover states
    for (uint8_t i = 0; i < gui.window_count; i++) {
        window_t* win = gui.windows[i];
        if (!win->visible || win->minimized) continue;
        
        for (uint8_t j = 0; j < win->widget_count; j++) {
            widget_t* widget = &win->widgets[j];
            int16_t wx = win->x + widget->x;
            int16_t wy = win->y + GUI_TITLE_HEIGHT + widget->y;
            
            bool hover = (x >= wx && x < wx + widget->width &&
                         y >= wy && y < wy + widget->height);
            
            if (hover && !(widget->flags & WIDGET_HOVERED)) {
                widget->flags |= WIDGET_HOVERED;
                gui.redraw_needed = true;
            } else if (!hover && (widget->flags & WIDGET_HOVERED)) {
                widget->flags &= ~WIDGET_HOVERED;
                gui.redraw_needed = true;
            }
        }
    }
}

void gui_handle_mouse_down(int16_t x, int16_t y, uint8_t button) {
    if (button != MOUSE_LEFT_BUTTON) return;
    
    gui.mouse_down = true;
    int16_t taskbar_y = GFX_HEIGHT - GUI_TASKBAR_HEIGHT;
    
    // Start button click
    if (x < 50 && y >= taskbar_y) {
        gui_toggle_start_menu();
        return;
    }
    
    // Start menu item click
    if (gui.start_menu.visible) {
        int16_t menu_h = gui.start_menu.item_count * 20 + 4;
        int16_t menu_y = GFX_HEIGHT - GUI_TASKBAR_HEIGHT - menu_h;
        
        if (x >= 2 && x < 2 + GUI_STARTMENU_WIDTH && y >= menu_y) {
            if (gui.start_menu.hover_index >= 0) {
                void (*action)(void) = gui.start_menu.items[gui.start_menu.hover_index].action;
                gui.start_menu.visible = false;
                gui.redraw_needed = true;
                if (action) action();
                return;
            }
        }
        // Click outside menu closes it
        gui.start_menu.visible = false;
        gui.redraw_needed = true;
    }
    
    // Check desktop icons (double-click to open)
    if (y < taskbar_y) {
        for (uint8_t i = 0; i < gui.icon_count; i++) {
            desktop_icon_t* icon = &gui.icons[i];
            if (x >= icon->x - 4 && x < icon->x + 36 &&
                y >= icon->y - 4 && y < icon->y + 40) {
                
                // Toggle selection
                for (uint8_t j = 0; j < gui.icon_count; j++) {
                    gui.icons[j].selected = false;
                }
                icon->selected = true;
                
                // Check for double-click (simple: if already selected, open)
                static uint32_t last_click = 0;
                static uint8_t last_icon = 255;
                uint32_t now = timer_get_ticks();
                
                if (last_icon == i && (now - last_click) < 50) {
                    if (icon->action) icon->action();
                }
                
                last_click = now;
                last_icon = i;
                gui.redraw_needed = true;
                return;
            }
        }
        
        // Deselect all icons
        for (uint8_t i = 0; i < gui.icon_count; i++) {
            gui.icons[i].selected = false;
        }
    }
    
    // Check windows (back to front, so check in reverse)
    for (int i = gui.window_count - 1; i >= 0; i--) {
        window_t* win = gui.windows[i];
        if (!win->visible || win->minimized) continue;
        
        // Check if click is in window
        if (x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height) {
            
            // Focus window
            gui_focus_window(win);
            
            // Close button
            int16_t close_x = win->x + win->width - 14;
            int16_t close_y = win->y + 2;
            if (x >= close_x && x < close_x + 12 && y >= close_y && y < close_y + 10) {
                gui_destroy_window(win);
                return;
            }
            
            // Title bar (dragging)
            if (y < win->y + GUI_TITLE_HEIGHT) {
                gui.dragging_window = win;
                win->drag_offset_x = x - win->x;
                win->drag_offset_y = y - win->y;
                return;
            }
            
            // Check widgets
            for (uint8_t j = 0; j < win->widget_count; j++) {
                widget_t* widget = &win->widgets[j];
                int16_t wx = win->x + widget->x;
                int16_t wy = win->y + GUI_TITLE_HEIGHT + widget->y;
                
                if (x >= wx && x < wx + widget->width &&
                    y >= wy && y < wy + widget->height) {
                    
                    if (widget->type == WIDGET_BUTTON) {
                        widget->flags |= WIDGET_PRESSED;
                        gui.redraw_needed = true;
                    }
                }
            }
            
            return;
        }
    }
}

void gui_handle_mouse_up(int16_t x, int16_t y, uint8_t button) {
    if (button != MOUSE_LEFT_BUTTON) return;
    
    gui.mouse_down = false;
    gui.dragging_window = NULL;
    
    // Handle button releases
    for (uint8_t i = 0; i < gui.window_count; i++) {
        window_t* win = gui.windows[i];
        if (!win->visible) continue;
        
        for (uint8_t j = 0; j < win->widget_count; j++) {
            widget_t* widget = &win->widgets[j];
            
            if (widget->flags & WIDGET_PRESSED) {
                widget->flags &= ~WIDGET_PRESSED;
                gui.redraw_needed = true;
                
                // Check if still over button (click completed)
                int16_t wx = win->x + widget->x;
                int16_t wy = win->y + GUI_TITLE_HEIGHT + widget->y;
                
                if (x >= wx && x < wx + widget->width &&
                    y >= wy && y < wy + widget->height) {
                    if (widget->on_click) {
                        widget->on_click(widget);
                    }
                }
            }
        }
    }
}

void gui_update(void) {
    // Process mouse events
    mouse_event_t event;
    while (mouse_poll_event(&event)) {
        switch (event.type) {
            case MOUSE_EVENT_MOVE:
                gui_handle_mouse_move(event.x, event.y);
                gui.redraw_needed = true;
                break;
            case MOUSE_EVENT_BUTTON_DOWN:
                gui_handle_mouse_down(event.x, event.y, event.button);
                break;
            case MOUSE_EVENT_BUTTON_UP:
            case MOUSE_EVENT_CLICK:
                gui_handle_mouse_up(event.x, event.y, event.button);
                break;
            default:
                break;
        }
    }
    
    // Check for keyboard input (ESC to close menu/window)
    if (keyboard_has_input()) {
        char key = keyboard_getchar();
        if (key == 27) {  // ESC
            if (gui.start_menu.visible) {
                gui.start_menu.visible = false;
                gui.redraw_needed = true;
            } else if (gui.active_window) {
                gui_destroy_window(gui.active_window);
            }
        }
    }
}

void gui_show_splash(void) {
    gfx_init();
    
    // Black background
    gfx_clear(COLOR_BLACK);
    
    // Draw KaiOS logo (ASCII art style)
    const char* logo[] = {
        " _  __     _  ___  ____  ",
        "| |/ /__ _(_)/ _ \\/ ___| ",
        "| ' // _` | | | | \\___ \\ ",
        "| . \\ (_| | | |_| |___) |",
        "|_|\\_\\__,_|_|\\___/|____/ ",
    };
    
    int16_t start_y = 70;
    for (int i = 0; i < 5; i++) {
        int16_t text_w = gfx_text_width(logo[i]);
        int16_t x = (GFX_WIDTH - text_w) / 2;
        gfx_puts(x, start_y + i * 10, logo[i], COLOR_LIGHT_CYAN, 255);
    }
    
    // Version
    const char* version = "v1.0";
    int16_t ver_w = gfx_text_width(version);
    gfx_puts((GFX_WIDTH - ver_w) / 2, start_y + 60, version, COLOR_WHITE, 255);
    
    // Loading text
    const char* loading = "Loading...";
    int16_t load_w = gfx_text_width(loading);
    gfx_puts((GFX_WIDTH - load_w) / 2, start_y + 80, loading, COLOR_DARK_GRAY, 255);
    
    gfx_swap_buffers();
    
    // Simple delay
    for (volatile int i = 0; i < 30000000; i++);
}

void gui_run(void) {
    // Force initial draw
    gui_draw();
    
    while (gui.initialized) {
        gui_update();
        
        // Always redraw to show cursor movement
        gui_draw();
        
        // Small delay to prevent 100% CPU
        for (volatile int i = 0; i < 50000; i++);
    }
}

// ============================================================================
// Application windows
// ============================================================================

static window_t* file_manager_window = NULL;

static void file_manager_close_clicked(widget_t* widget) {
    (void)widget;
    if (file_manager_window) {
        gui_destroy_window(file_manager_window);
        file_manager_window = NULL;
    }
}

static void file_manager_open(void) {
    if (file_manager_window) {
        gui_focus_window(file_manager_window);
        return;
    }
    
    file_manager_window = gui_create_window("File Manager", 60, 30, 200, 120);
    if (!file_manager_window) return;
    
    // Add labels showing current directory contents
    gui_add_label(file_manager_window, 8, 8, "Current: /");
    
    // List files
    fs_node_t* root = fs_get_root();
    size_t count;
    fs_node_t** children = fs_readdir(root, &count);
    
    int16_t y = 24;
    for (size_t i = 0; i < count && i < 5; i++) {
        widget_t* icon = gui_add_icon(file_manager_window, 8, y, 
                                       children[i]->type == FS_DIRECTORY ? ICON_FOLDER : ICON_FILE,
                                       children[i]->name);
        (void)icon;
        
        gui_add_label(file_manager_window, 28, y + 4, children[i]->name);
        y += 18;
    }
    
    // Close button
    widget_t* close_btn = gui_add_button(file_manager_window, 140, 95, 50, 16, "Close");
    if (close_btn) close_btn->on_click = file_manager_close_clicked;
}

static void terminal_open(void) {
    // For now, just show a message - full terminal would require text mode switch
    window_t* term = gui_create_window("Terminal", 80, 40, 160, 100);
    if (!term) return;
    
    gui_add_label(term, 8, 8, "Terminal Mode");
    gui_add_label(term, 8, 24, "Press ESC or");
    gui_add_label(term, 8, 40, "close to exit");
    gui_add_label(term, 8, 60, "GUI and use");
    gui_add_label(term, 8, 76, "text terminal.");
}

static void do_shutdown(void) {
    // Save filesystem
    fs_save();
    
    // Halt
    gfx_clear(COLOR_BLACK);
    gfx_puts(100, 90, "Shutting down...", COLOR_WHITE, 255);
    gfx_swap_buffers();
    
    for (volatile int i = 0; i < 50000000; i++);
    
    __asm__ volatile("cli");
    while (1) {
        __asm__ volatile("hlt");
    }
}
