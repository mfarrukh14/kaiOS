/*
 * KaiOS - Timer Driver Header (PIT - Programmable Interval Timer)
 */

#ifndef KAIOS_TIMER_H
#define KAIOS_TIMER_H

#include "include/kernel/types.h"

// PIT constants
#define PIT_FREQUENCY 1193180
#define TIMER_FREQUENCY 100  // 100 Hz (10ms per tick)

// Timer functions
void timer_init(uint32_t frequency);
uint32_t timer_get_ticks(void);
void timer_wait(uint32_t ticks);
void timer_sleep_ms(uint32_t ms);

#endif // KAIOS_TIMER_H
