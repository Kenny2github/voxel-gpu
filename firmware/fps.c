#include "interrupts.h"
#include "hardware/hardware.h"

int frames;

static const uint8_t num_to_hex[10] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F  // 9
};

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
	uint32_t frames_display = num_to_hex[frames % 10];
	frames_display |= num_to_hex[(frames / 10) % 10] << 8;
	frames_display |= num_to_hex[(frames / 100) % 10] << 16;
	frames_display |= num_to_hex[(frames / 1000) % 10] << 24;
	memset(HEX3_HEX0, frames_display, 4);
	frames = 0;
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

