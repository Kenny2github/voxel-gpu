#ifndef FPS_H
#define FPS_H

extern int frames;

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

#endif