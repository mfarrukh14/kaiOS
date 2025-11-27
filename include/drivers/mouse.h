/*
 * KaiOS - PS/2 Mouse Driver Header
 */

#ifndef KAIOS_MOUSE_H
#define KAIOS_MOUSE_H

#include "include/kernel/types.h"

// Mouse button states
#define MOUSE_LEFT_BUTTON   0x01
#define MOUSE_RIGHT_BUTTON  0x02
#define MOUSE_MIDDLE_BUTTON 0x04

// Mouse state
typedef struct {
    int16_t x;
    int16_t y;
    int16_t dx;      // Delta X since last read
    int16_t dy;      // Delta Y since last read
    uint8_t buttons; // Current button state
    uint8_t prev_buttons; // Previous button state (for click detection)
} mouse_state_t;

// Mouse event types
typedef enum {
    MOUSE_EVENT_NONE = 0,
    MOUSE_EVENT_MOVE,
    MOUSE_EVENT_BUTTON_DOWN,
    MOUSE_EVENT_BUTTON_UP,
    MOUSE_EVENT_CLICK
} mouse_event_type_t;

// Mouse event
typedef struct {
    mouse_event_type_t type;
    int16_t x;
    int16_t y;
    uint8_t button;  // Which button (for button events)
} mouse_event_t;

// Initialize mouse driver
void mouse_init(void);

// Get current mouse state
mouse_state_t* mouse_get_state(void);

// Check if mouse button is pressed
bool mouse_button_down(uint8_t button);

// Check if mouse button was just clicked
bool mouse_button_clicked(uint8_t button);

// Mouse interrupt handler
void mouse_handler(void);

// Poll for mouse event
bool mouse_poll_event(mouse_event_t* event);

// Set mouse position bounds
void mouse_set_bounds(int16_t max_x, int16_t max_y);

// Set mouse position
void mouse_set_position(int16_t x, int16_t y);

// Debug counter
extern volatile uint32_t mouse_packet_count;

#endif // KAIOS_MOUSE_H
