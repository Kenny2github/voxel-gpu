#include "interrupts.h"
#include "hardware/hardware.h"

int frames;

void enable_timer_interrupt(void) {
	volatile int* timer_ptr = (int*)MPCORE_PRIV_TIMER;
	// timer is 200MHz
	*(timer_ptr) = 200E6;
	// enable interrupts and countdown
	*(timer_ptr + 2) = 0b111;
}

static void handle_timer_interrupt(void) {
	volatile int* timer_ptr = (int*)MPCORE_PRIV_TIMER;
	// reset interrupt bit by writting 1 to it
	*(timer_ptr + 3) &= 1;
	// TODO: calc fps and display
	// frames = 0;
}

void enable_timer(void) {
	frames = 0;
	config_interrupt(PRIVATE_TIMER_IRQ, NULL, &handle_timer_interrupt);
}

void disable_timer(void) {
	volatile int* timer_ptr = (int*)MPCORE_PRIV_TIMER;
	// disable interrupts, auto-reload and countdown
	*(timer_ptr + 2) = 0b000;
}

