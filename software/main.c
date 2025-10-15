#include <stdio.h>
#include "address_map_arm.h"
#include "hardware.h"

int main(void) {
	printf("0xFF565800: pixel_buffer1 = %08X\n", GPU->pixel_buffer1);
	printf("0xFF565804: pixel_buffer2 = %08X\n", GPU->pixel_buffer2);
	GPU->pixel_buffer2 = SDRAM_BASE;
	printf("0xFF565804: pixel_buffer2 = %08X\n", GPU->pixel_buffer2);
	while (1);
}
