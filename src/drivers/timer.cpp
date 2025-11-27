/*
 * KaiOS - Timer Driver (PIT - Programmable Interval Timer)
 */

#include "include/drivers/timer.h"
#include "include/drivers/io.h"
#include "include/kernel/idt.h"

// Timer tick counter
static volatile uint32_t timer_ticks = 0;

// Timer interrupt handler
static void timer_callback(registers_t* regs) {
    timer_ticks++;
}

void timer_init(uint32_t frequency) {
    // Register timer interrupt handler
    register_interrupt_handler(32, timer_callback);
    
    // Calculate divisor
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Send the command byte
    outb(0x43, 0x36);
    
    // Send divisor (low byte first, then high byte)
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_wait(uint32_t ticks) {
    uint32_t target = timer_ticks + ticks;
    while (timer_ticks < target) {
        __asm__ volatile("hlt");
    }
}

void timer_sleep_ms(uint32_t ms) {
    uint32_t ticks = (ms * TIMER_FREQUENCY) / 1000;
    if (ticks == 0) ticks = 1;
    timer_wait(ticks);
}
