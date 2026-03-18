#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/interrupts.h"
#include "firmware/timing.h"
#include "firmware/palette.h"

#define NUM_SHADERS 6

static volatile int render_wait = 0;
unsigned char* pixel_buffer;
unsigned char* char_buffer;
unsigned int palette_size;
int cur_start_pixel;

void wait_for_vsync() {

    PIXEL_BUF_CTRL->buffer = 0x1;
    CHAR_BUF_CTRL->buffer = 0x1;
    /* Wait for both buffers to finish swapping */
    while (PIXEL_BUF_CTRL->status.s || CHAR_BUF_CTRL->status.s);
    ++frames;

    /* After swap, back_buffer points to the new draw buffer */
    pixel_buffer = PIXEL_BUF_CTRL->back_buffer;
    char_buffer = CHAR_BUF_CTRL->back_buffer;

}

void render() {
    render_wait = 1;

    // Before render, update GPU camera settings
    update_camera();

    double start = fw_time + (200E6 - cur_time()) / 200E6;

    for (int i = 0; i < H_RESOLUTION * V_RESOLUTION; i += NUM_SHADERS) {
        GPU->start_pixel = i;
        cur_start_pixel = i;
        while (render_wait);
        render_wait = 1;
    }
    double end = fw_time + (200E6 - cur_time()) / 200E6;
    gpu_latency = end - start;

    /* GPU interrupt handled, swap buffers */
    wait_for_vsync();
}

static void enable_gpu_interrupt(void) {
    /* do nothing */
    ;
}

static void handle_gpu_interrupt(void) {
    static int state = 0;
    static int id = 0;
    if (!GPU->render_status) {
        switch (state) {
            case 0: // RASTERIZE
                GPU->rasterize_voxel = voxel_space[id++];
                if (id == voxel_count) {
                    id = 0;
                    state = 1;
                }
                break;

            case 1: // SHADE
                GPU->shade_entry = (struct gpu_palette_entry){
                    .voxel_id = id,
                    .color = palette_data[id++]
                };
                if (id == palette_size) {
                    id = 0;
                    state = 2;
                }
                break;

            case 2: // WRITE_PIXEL
                int pixel_id = cur_start_pixel + id;
                int row = pixel_id / H_RESOLUTION;
                int col = pixel_id % H_RESOLUTION;
                GPU->write_pixel = pixel_buffer + (row << 10 | col << 1);
                if (++id == NUM_SHADERS) {
                    id = 0;
                    state = 0;
                    render_wait = 0;
                }
                break;

            default:
                break;
        }
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

    // fill_palette_buffer();
    palette_size = sizeof(palette_data) / sizeof(palette_data[0]);
    clear_grid();

    voxel_count = 0;
    voxel_space_size = 256;
    voxel_space = (uint32_t*)calloc(voxel_space_size, sizeof(uint32_t));

    /* Initialize double-buffering pointers for pixel + character buffers */
    PIXEL_BUF_CTRL->back_buffer = SDRAM_BASE;
    CHAR_BUF_CTRL->back_buffer = FPGA_CHAR_BASE;

    /* Use the controller-provided back buffer address for rendering */
    pixel_buffer = PIXEL_BUF_CTRL->back_buffer;
    char_buffer = CHAR_BUF_CTRL->back_buffer;

    config_interrupt(GPU_IRQ, enable_gpu_interrupt, handle_gpu_interrupt);
    enable_timer();
}
