#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/interrupts.h"
#include "firmware/palette.h"

static int render_wait = 0;

void render() {
    render_wait = 1;
    GPU->do_render = 1;
    while (render_wait);

    // Swap buffer here
}

static void enable_gpu_interrupt(void) {
    /* clear render interrupt */
    GPU->render_irq = 0;
}

static void handle_gpu_interrupt(void) {
    render_wait = 0;
}

static void fill_palette_buffer(void) {
    for (uint16_t i = 0; i < GPU->palette_length; ++i) {
        uint16_t upper_color = (palette_data[i] & 0xFF00) >> 8;
        uint16_t lower_color = (palette_data[i] & 0x00FF);
        *(GPU->palette_buffer + i * 2) = lower_color;
        *(GPU->palette_buffer + i * 2 + 1) = upper_color;
    }
}

void init_firmware() {
    GPU->voxel_buffer = (unsigned char *)GRID_START;
    GPU->voxel_count = 0;

    GPU->palette_buffer = (unsigned char *)PALETTE_START;
    GPU->palette_length = sizeof(palette_data) / sizeof(palette_data[0]);

    fill_palette_buffer();
    clear_grid();

    config_interrupt(GPU_IRQ, enable_gpu_interrupt, handle_gpu_interrupt);
}
