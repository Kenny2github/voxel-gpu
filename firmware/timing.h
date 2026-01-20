#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

extern int frames;
extern unsigned int fw_time;
extern double gpu_latency;

/**
 * Enables interrupts per second for A9 private timer
 */
void enable_timer_interrupt(void);

/**
 * Enables A9 private timer functionality
 */
void enable_timer(void);

/**
 * Disables A9 private timer functionality
 */
void disable_timer(void);

/**
 * Get current time in A9 private timer
 */
uint32_t cur_time(void);

#endif
