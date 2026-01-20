#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/interrupts.h"
#include "firmware/timing.h"
#include "firmware/palette.h"

static volatile int render_wait = 0;

void wait_for_vsync() {
    *((int*)PIXEL_BUF_CTRL) = 1;
    while (*((int*)PIXEL_BUF_CTRL + 3) & 1);
    ++frames;

    GPU->pixel_buffer = PIXEL_BUF_CTRL->back_buffer;
}

void render() {
    render_wait = 1;
    GPU->do_render = 1;
    double start = cur_time() / 200E6 * fw_time;
    while (render_wait);
    gpu_latency = cur_time() / 200E6 + fw_time - start;

    /* GPU interrupt handled, swap buffers */
    wait_for_vsync();
}

static void enable_gpu_interrupt(void) {
    /* clear render interrupt */
    GPU->render_irq = 0;
}

static void handle_gpu_interrupt(void) {
    render_wait = 0;
}

static void fill_palette_buffer(void) {
    for (uint16_t i = 0; i < sizeof(palette_data) / sizeof(palette_data[0]); ++i) {
        uint16_t upper_color = (palette_data[i] & 0xFF00) >> 8;
        uint16_t lower_color = (palette_data[i] & 0x00FF);
        *((uint32_t *)PALETTE_START + i * 2) = lower_color;
        *((uint32_t *)PALETTE_START + i * 2 + 1) = upper_color;
    }
}

void init_firmware() {


    fill_palette_buffer();
    clear_grid();

    GPU->voxel_buffer = (unsigned char *)GRID_START;
    GPU->voxel_count = 0;

    GPU->palette_buffer = (unsigned char *)PALETTE_START;
    GPU->palette_length = sizeof(palette_data) / sizeof(palette_data[0]);

    PIXEL_BUF_CTRL->back_buffer = SDRAM_BASE;

    GPU->pixel_buffer = SDRAM_BASE;


    config_interrupt(GPU_IRQ, enable_gpu_interrupt, handle_gpu_interrupt);
    enable_timer();
}
