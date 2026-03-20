#include <stdlib.h>
#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include "firmware/timing.h"
#include "firmware/palette.h"

#define NUM_SHADERS 17

unsigned char* pixel_buffer;
unsigned char* char_buffer;
unsigned int palette_size;

void wait_for_vsync() {

    PIXEL_BUF_CTRL->swap = 0x1;
    CHAR_BUF_CTRL->swap = 0x1;
    /* Wait for both buffers to finish swapping */
    while (PIXEL_BUF_CTRL->status.s || CHAR_BUF_CTRL->status.s);
    ++frames;

    /* After swap, back_buffer points to the new draw buffer */
    pixel_buffer = PIXEL_BUF_CTRL->back_buffer;
    char_buffer = CHAR_BUF_CTRL->back_buffer;

}

void render() {
    // Before render, update GPU camera settings
    update_camera();

    float start = fw_time + (200E6f - cur_time()) / 200E6f;

    int row = 0;
    int col = 0;
    unsigned char *pixel_ptr = pixel_buffer;

    for (int i = 0; i < H_RESOLUTION * V_RESOLUTION; i += NUM_SHADERS) {
        GPU->start_pixel = i;
        while (GPU->render_status);

        for (int voxel_id = 0; voxel_id < voxel_count; ++voxel_id) {
            GPU->rasterize_voxel = voxel_space[voxel_id];
            while (GPU->render_status);
        }

        for (int palette_id = 1; palette_id < palette_size; ++palette_id) {
            GPU->shade_entry = (struct gpu_palette_entry){
                .voxel_id = palette_id, .color = palette_data[palette_id]
            };
            while (GPU->render_status);
        }

        for (int j = 0; j < NUM_SHADERS; ++j) {
            GPU->write_pixel = pixel_ptr;

            if (++col >= H_RESOLUTION) {
                col = 0;
                pixel_ptr = pixel_buffer + ((++row) << 10);
                if (row >= V_RESOLUTION) break;
            } else {
                pixel_ptr += 2;
            }
            
            while (GPU->render_status);
        }
        
    }

    float end = fw_time + (200E6f - cur_time()) / 200E6f;
    gpu_latency = end - start;

    /* GPU interrupt handled, swap buffers */
    wait_for_vsync();
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
    voxel_space = calloc(voxel_space_size, sizeof(uint32_t));

    /* Initialize double-buffering pointers for pixel + character buffers */
    PIXEL_BUF_CTRL->back_buffer = SDRAM_BASE;
    CHAR_BUF_CTRL->back_buffer = FPGA_CHAR_BASE;

    /* Use the controller-provided back buffer address for rendering */
    pixel_buffer = PIXEL_BUF_CTRL->back_buffer;
    char_buffer = CHAR_BUF_CTRL->back_buffer;

    enable_timer();
}
