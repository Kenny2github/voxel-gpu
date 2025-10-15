#ifndef GPU_ADDR_H
#define GPU_ADDR_H

static volatile struct {
	/**
	 * Address of first pixel buffer
	 */
	unsigned char * pixel_buffer1;
	/**
	 * Address of second pixel buffer
	 */
	unsigned char * pixel_buffer2;
} * const GPU = (void *)0xFF565800;

#define GPU_IRQ 16U

#endif
