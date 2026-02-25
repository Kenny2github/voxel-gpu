#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/interrupts.h"
#include "firmware/timing.h"
#include "firmware/palette.h"

#define H_RESOLUTION 320
#define V_RESOLUTION 240
#define NUM_SHADERS 200

static volatile int render_wait = 0;
unsigned char* pixel_buffer;
unsigned int palette_size;

void wait_for_vsync() {
    
    PIXEL_BUF_CTRL->buffer = 0x1;
    while (PIXEL_BUF_CTRL->status.s);
    ++frames;

    pixel_buffer = PIXEL_BUF_CTRL->back_buffer;
}

void render() {
    render_wait = 1;

    // Before render, update GPU camera settings
    update_camera();

    double start = cur_time() / 200E6 * fw_time;

    for (int i = 0; i < H_RESOLUTION * V_RESOLUTION; i += NUM_SHADERS) {
        GPU->start_pixel = pixel_buffer + i;
        while (render_wait);

        for (int voxel_id = 0; voxel_id < voxel_count; ++voxel_id) {
            GPU->rasterize_voxel = (struct gpu_voxel){
                .voxel_id = voxel_space[voxel_id] & 0x3,
                .x = (voxel_space[voxel_id] >> 22) & 0x3FF,
                .y = (voxel_space[voxel_id] >> 12) & 0x3FF,
                .z = (voxel_space[voxel_id] >> 2) & 0x3FF
            };
            while (render_wait);
        }

        for (int palette_id = 0; palette_id < palette_size; ++palette_id) {
            GPU->shade_entry = (struct gpu_palette_entry){
                .voxel_id = palette_id,
                .color = palette_data[palette_id]
            };
            while (render_wait);
        }

        for (int j = i; j < i + NUM_SHADERS; ++j) {
            int row = j / H_RESOLUTION;
            int col = j % H_RESOLUTION;
            GPU->write_pixel = pixel_buffer + (row << 10 | col << 1);
            while (render_wait);
        }
    }
    gpu_latency = cur_time() / 200E6 + fw_time - start;

    /* GPU interrupt handled, swap buffers */
    wait_for_vsync();
}

static void enable_gpu_interrupt(void) {
    /* clear render interrupt */
    GPU->clear_error = 1;
}

static void handle_gpu_interrupt(void) {
    if (!GPU->render_status) {
        render_wait = 0;
    }
}

// static void fill_palette_buffer(void) {
//     for (uint16_t i = 0; i < sizeof(palette_data) / sizeof(palette_data[0]); ++i) {
//         uint16_t upper_color = (palette_data[i] & 0xFF00) >> 8;
//         uint16_t lower_color = (palette_data[i] & 0x00FF);
//         *((uint32_t *)PALETTE_START + i * 2) = lower_color;
//         *((uint32_t *)PALETTE_START + i * 2 + 1) = upper_color;
//     }
// }

void init_firmware() {
    voxel_count = 0;

    fill_palette_buffer();
    palette_size = sizeof(palette_data) / sizeof(palette_data[0]);
    clear_grid();

    voxel_count = 0;
    voxel_space_size = 256;
    voxel_space = (uint32_t*)calloc(voxel_space_size, sizeof(uint32_t));

    PIXEL_BUF_CTRL->back_buffer = SDRAM_BASE;

    pixel_buffer = SDRAM_BASE;

    config_interrupt(GPU_IRQ, enable_gpu_interrupt, handle_gpu_interrupt);
    enable_timer();
}
