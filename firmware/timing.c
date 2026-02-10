#include <stdio.h>
#include "interrupts.h"
#include "hardware/hardware.h"

// double frame_time;
// int fps;
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

void enable_timer_interrupt(void) {
	// timer is 200MHz, set to count by microsec
	MPCORE_PRIV_TIMER->load = 1E6;
	MPCORE_PRIV_TIMER->prescaler = 199;
	// enable interrupts and countdown
	MPCORE_PRIV_TIMER->e = 1;
	MPCORE_PRIV_TIMER->a = 1;
	MPCORE_PRIV_TIMER->i = 1;
}

static void handle_timer_interrupt(void) {
	// reset interrupt bit by writting 1 to it
	MPCORE_PRIV_TIMER->f = 1;
	struct hex3_hex0_registers frames_display;
	// frames_display.hex0 = num_to_hex[fps % 10];
	// frames_display.hex1 = num_to_hex[(fps / 10) % 10];
	// frames_display.hex2 = num_to_hex[(fps / 100) % 10];
	// frames_display.hex3 = num_to_hex[(fps / 1000) % 10];
	frames_display.hex0 = num_to_hex[frames % 10];
	frames_display.hex1 = num_to_hex[(frames / 10) % 10];
	frames_display.hex2 = num_to_hex[(frames / 100) % 10];
	frames_display.hex3 = num_to_hex[(frames / 1000) % 10];
	*HEX3_HEX0 = frames_display;
	frames = 0;
	++fw_time;
	// if (fw_time % 5 == 0) printf("GPU latency: %e sec\n", gpu_latency);
}

void enable_timer(void) {
	// frame_time = 0;
	// fps = 0;
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

// void compute_fps() {
//     const float smoothing = 0.9;

//     double new_time = cur_time() / 1E6 + fw_time;
//     if (frame_time != 0) {
//         fps = fps * smoothing + (1 - smoothing) / (new_time - frame_time);
//     }
//     frame_time = new_time;
// }
