/*
 * KaiOS - PS/2 Mouse Driver
 */

#include "include/drivers/mouse.h"
#include "include/drivers/io.h"
#include "include/kernel/idt.h"

// PS/2 controller ports
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_COMMAND_PORT 0x64

// PS/2 commands
#define PS2_CMD_READ_CONFIG    0x20
#define PS2_CMD_WRITE_CONFIG   0x60
#define PS2_CMD_DISABLE_MOUSE  0xA7
#define PS2_CMD_ENABLE_MOUSE   0xA8
#define PS2_CMD_SEND_TO_MOUSE  0xD4

// Mouse commands
#define MOUSE_CMD_SET_DEFAULTS  0xF6
#define MOUSE_CMD_ENABLE        0xF4
#define MOUSE_CMD_SET_SAMPLE    0xF3
#define MOUSE_CMD_GET_ID        0xF2

// Mouse state
static mouse_state_t mouse_state;
static int16_t mouse_max_x = 320;
static int16_t mouse_max_y = 200;

// Packet state
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
volatile uint32_t mouse_packet_count = 0;

// Event queue
#define MOUSE_EVENT_QUEUE_SIZE 16
static mouse_event_t event_queue[MOUSE_EVENT_QUEUE_SIZE];
static uint8_t event_head = 0;
static uint8_t event_tail = 0;

// Wait for PS/2 controller
static void mouse_wait_write(void) {
    int timeout = 100000;
    while (timeout--) {
        if (!(inb(PS2_STATUS_PORT) & 0x02)) return;
    }
}

static void mouse_wait_read(void) {
    int timeout = 100000;
    while (timeout--) {
        if (inb(PS2_STATUS_PORT) & 0x01) return;
    }
}

// Send command to mouse
static void mouse_write(uint8_t data) {
    mouse_wait_write();
    outb(PS2_COMMAND_PORT, PS2_CMD_SEND_TO_MOUSE);
    mouse_wait_write();
    outb(PS2_DATA_PORT, data);
}

// Read from mouse
static uint8_t mouse_read(void) {
    mouse_wait_read();
    return inb(PS2_DATA_PORT);
}

// Add event to queue
static void mouse_add_event(mouse_event_type_t type, uint8_t button) {
    event_queue[event_head].type = type;
    event_queue[event_head].x = mouse_state.x;
    event_queue[event_head].y = mouse_state.y;
    event_queue[event_head].button = button;
    event_head = (event_head + 1) % MOUSE_EVENT_QUEUE_SIZE;
}

void mouse_init(void) {
    // Initialize state
    mouse_state.x = mouse_max_x / 2;
    mouse_state.y = mouse_max_y / 2;
    mouse_state.dx = 0;
    mouse_state.dy = 0;
    mouse_state.buttons = 0;
    mouse_state.prev_buttons = 0;
    mouse_cycle = 0;
    
    // Enable auxiliary mouse device
    mouse_wait_write();
    outb(PS2_COMMAND_PORT, PS2_CMD_ENABLE_MOUSE);
    
    // Enable interrupts and ensure mouse port is enabled
    mouse_wait_write();
    outb(PS2_COMMAND_PORT, PS2_CMD_READ_CONFIG);
    mouse_wait_read();
    uint8_t status = inb(PS2_DATA_PORT);
    status |= 0x02;  // Enable IRQ12
    status &= ~0x20; // Enable Mouse Port (Clear Disable Mouse Bit)
    mouse_wait_write();
    outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_CONFIG);
    mouse_wait_write();
    outb(PS2_DATA_PORT, status);
    
    // Set defaults
    mouse_write(MOUSE_CMD_SET_DEFAULTS);
    mouse_read();  // ACK
    
    // Enable data reporting
    mouse_write(MOUSE_CMD_ENABLE);
    mouse_read();  // ACK
}

void mouse_handler(void) {
    // Read status
    uint8_t status = inb(PS2_STATUS_PORT);
    
    // If buffer is empty, we can't do anything
    if (!(status & 0x01)) {
        return;
    }
    
    // Read data
    int8_t data = inb(PS2_DATA_PORT);
    mouse_packet_count++;
    
    switch (mouse_cycle) {
        case 0:
            // First byte: buttons and sign bits
            // Bit 3 should always be set in a valid first byte
            // But we are permissive for now
            mouse_byte[0] = data;
            mouse_cycle = 1;
            break;
        case 1:
            // Second byte: X movement
            mouse_byte[1] = data;
            mouse_cycle = 2;
            break;
        case 2:
            // Third byte: Y movement
            mouse_byte[2] = data;
            mouse_cycle = 0;
            
            // Process complete packet
            mouse_state.prev_buttons = mouse_state.buttons;
            mouse_state.buttons = mouse_byte[0] & 0x07;
            
            // Calculate deltas (handle sign extension)
            int16_t dx = mouse_byte[1];
            int16_t dy = mouse_byte[2];
            
            if (mouse_byte[0] & 0x10) dx |= 0xFF00;  // Sign extend X
            if (mouse_byte[0] & 0x20) dy |= 0xFF00;  // Sign extend Y
            
            // Update position
            mouse_state.dx = dx;
            mouse_state.dy = -dy;  // Invert Y (screen coords)
            
            mouse_state.x += dx;
            mouse_state.y -= dy;  // Invert Y
            
            // Clamp to bounds
            if (mouse_state.x < 0) mouse_state.x = 0;
            if (mouse_state.y < 0) mouse_state.y = 0;
            if (mouse_state.x >= mouse_max_x) mouse_state.x = mouse_max_x - 1;
            if (mouse_state.y >= mouse_max_y) mouse_state.y = mouse_max_y - 1;
            
            // Generate events
            if (dx != 0 || dy != 0) {
                mouse_add_event(MOUSE_EVENT_MOVE, 0);
            }
            
            // Check button changes
            uint8_t changed = mouse_state.buttons ^ mouse_state.prev_buttons;
            if (changed & MOUSE_LEFT_BUTTON) {
                if (mouse_state.buttons & MOUSE_LEFT_BUTTON) {
                    mouse_add_event(MOUSE_EVENT_BUTTON_DOWN, MOUSE_LEFT_BUTTON);
                } else {
                    mouse_add_event(MOUSE_EVENT_BUTTON_UP, MOUSE_LEFT_BUTTON);
                    mouse_add_event(MOUSE_EVENT_CLICK, MOUSE_LEFT_BUTTON);
                }
            }
            if (changed & MOUSE_RIGHT_BUTTON) {
                if (mouse_state.buttons & MOUSE_RIGHT_BUTTON) {
                    mouse_add_event(MOUSE_EVENT_BUTTON_DOWN, MOUSE_RIGHT_BUTTON);
                } else {
                    mouse_add_event(MOUSE_EVENT_BUTTON_UP, MOUSE_RIGHT_BUTTON);
                    mouse_add_event(MOUSE_EVENT_CLICK, MOUSE_RIGHT_BUTTON);
                }
            }
            break;
    }
}

mouse_state_t* mouse_get_state(void) {
    return &mouse_state;
}

bool mouse_button_down(uint8_t button) {
    return (mouse_state.buttons & button) != 0;
}

bool mouse_button_clicked(uint8_t button) {
    return ((mouse_state.prev_buttons & button) && !(mouse_state.buttons & button));
}

bool mouse_poll_event(mouse_event_t* event) {
    if (event_head == event_tail) {
        return false;
    }
    
    *event = event_queue[event_tail];
    event_tail = (event_tail + 1) % MOUSE_EVENT_QUEUE_SIZE;
    return true;
}

void mouse_set_bounds(int16_t max_x, int16_t max_y) {
    mouse_max_x = max_x;
    mouse_max_y = max_y;
    
    // Clamp current position
    if (mouse_state.x >= max_x) mouse_state.x = max_x - 1;
    if (mouse_state.y >= max_y) mouse_state.y = max_y - 1;
}

void mouse_set_position(int16_t x, int16_t y) {
    mouse_state.x = x;
    mouse_state.y = y;
}
