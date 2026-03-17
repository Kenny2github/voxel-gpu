#include <stdio.h>
#include "interrupts.h"
#include "firmware/firmware.h"
#include "hardware/hardware.h"

int frames;
unsigned int fw_time;
double gpu_latency;

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

static inline void draw_character(int x, int y, char c) {
	*(uint8_t *)(CHAR_BUF_CTRL + (y << 7) + x) = c;
}

static void write_gpu_latency() {
	// clear previous characters
	for (int i = 0; i < SCREEN_CHAR_W; ++i) {
		draw_character(i, 0, 0);
	}

	char buffer[SCREEN_CHAR_W];
	int len = sprintf(buffer, sizeof(buffer), "GPU latency: %e ms", gpu_latency * 1000);
	int curr_x = 0, curr_y = 0;

	for (int i = 0; i < len; ++i) {
		draw_character(curr_x++, curr_y, buffer[i]);
	}
}

void enable_timer_interrupt(void) {
	// timer is 200MHz
	MPCORE_PRIV_TIMER->load = 1E6;
	MPCORE_PRIV_TIMER->prescaler = 199;
	// enable interrupts and countdown
	MPCORE_PRIV_TIMER->e = 1;
	MPCORE_PRIV_TIMER->a = 1;
	MPCORE_PRIV_TIMER->i = 1;
}

static void handle_timer_interrupt(void) {
	// reset interrupt bit by writting 1 to it
	*(timer_ptr + 3) &= 1;
	uint32_t frames_display = num_to_hex[frames % 10];
	frames_display |= num_to_hex[(frames / 10) % 10] << 8;
	frames_display |= num_to_hex[(frames / 100) % 10] << 16;
	frames_display |= num_to_hex[(frames / 1000) % 10] << 24;
	memset(HEX3_HEX0, frames_display, 4);
	write_gpu_latency();
	frames = 0;
	++fw_time;
}

void enable_timer(void) {
	frames = 0;
	fw_time = 0;
	gpu_latency = 0;
	config_interrupt(PRIVATE_TIMER_IRQ, NULL, &handle_timer_interrupt);
	enable_timer_interrupt();
}

void disable_timer(void) {
	// disable interrupts, auto-reload and countdown
	MPCORE_PRIV_TIMER->e = 0;
	MPCORE_PRIV_TIMER->a = 0;
	MPCORE_PRIV_TIMER->i = 0;
}

uint32_t cur_time(void) {
	return MPCORE_PRIV_TIMER->counter;
}
