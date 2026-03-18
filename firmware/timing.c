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
	*(uint8_t *)(CHAR_BUF_CTRL->back_buffer + (y << 7) + x) = c;
}

static void write_gpu_latency() {
	// clear previous characters
	for (int i = 0; i < SCREEN_CHAR_W; ++i) {
		draw_character(i, 0, 0);
	}

	char buffer[SCREEN_CHAR_W];
	int len = sprintf(buffer, "GPU latency: %e ms", gpu_latency * 1000);
	int curr_x = 0, curr_y = 0;

	for (int i = 0; i < len; ++i) {
		draw_character(curr_x++, curr_y, buffer[i]);
	}
}

void enable_timer_interrupt(void) {
	MPCORE_PRIV_TIMER->load = 200E6;
	// enable interrupts and countdown
	MPCORE_PRIV_TIMER->control = (struct private_timer_control_register){
		.i = 1,
		.a = 1,
		.e = 1
	};
}

static void handle_timer_interrupt(void) {
	// reset interrupt bit by writting 1 to it
	MPCORE_PRIV_TIMER->f = 1;
	struct hex3_hex0_registers frames_display;
	frames_display.hex0 = num_to_hex[frames % 10];
	frames_display.hex1 = num_to_hex[(frames / 10) % 10];
	frames_display.hex2 = num_to_hex[(frames / 100) % 10];
	frames_display.hex3 = num_to_hex[(frames / 1000) % 10];
	*HEX3_HEX0 = frames_display;
	write_gpu_latency();
	frames = 0;
	++fw_time;
}

void enable_timer(void) {
	frames = 0;
	fw_time = 0;
	gpu_latency = 0;
	config_interrupt(PRIVATE_TIMER_IRQ, &enable_timer_interrupt, &handle_timer_interrupt);
}

void disable_timer(void) {
	// disable interrupts, auto-reload and countdown
	MPCORE_PRIV_TIMER->control = (struct private_timer_control_register){
		.i = 0,
		.a = 0,
		.e = 0
	};
}

uint32_t cur_time(void) {
	return MPCORE_PRIV_TIMER->counter;
}
