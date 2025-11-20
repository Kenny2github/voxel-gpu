#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/interrupts.h"

static int render_wait = 0;

void render() {
    // render_wait = 1;
    // GPU->do_render = 1;
    // while (render_wait);
}

static void enable_gpu_interrupt(void) {
    /* clear render interrupt */
    GPU->render_irq = 0;
}

static void handle_gpu_interrupt(void) {
    render_wait = 0;
}

void init_firmware() {
    GPU->voxel_buffer = (unsigned char *)GRID_START;
    GPU->voxel_count = 0;

    GPU->palette_buffer = (unsigned char *)PALETTE_START;
    GPU->palette_length = 128;

    // memset(GPU->palette_buffer + 2, 0x0, 1);
    // memset(GPU->palette_buffer + 3, 0x1, 1);

    config_interrupt(GPU_IRQ, enable_gpu_interrupt, handle_gpu_interrupt);
}
