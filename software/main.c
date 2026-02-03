#include <stdio.h>
#include "hardware/hardware.h"
#include "software/controls.h"
#include "firmware/interrupts.h"
#include "software/external.h"
#include "firmware/firmware.h"
#include "software/software_render.h"
int main(void) {
    // printf(
    //     "%p: pixel_buffer1 = %p\n", &((*GPU).pixel_buffer1), GPU->pixel_buffer1
    // );
    // printf(
    //     "%p: pixel_buffer2 = %p\n", &((*GPU).pixel_buffer2), GPU->pixel_buffer2
    // );
    // GPU->pixel_buffer2 = (unsigned char *)SDRAM_BASE;
    // printf(
    //     "%p: pixel_buffer2 = %p\n", &((*GPU).pixel_buffer2), GPU->pixel_buffer2
    // );
    reset_hex();

    // Setting up interrupts
    config_inputs();
    init_firmware();
    setup_pixel_buffer_software();
    config_interrupts();
    
    set_camera_settings(90.0, 1);
    set_camera_settings_software(90.0, 1);

    // Setting up camera
    struct Vector camPos = {34, 34, 36};
    struct Vector camLook = {0, 0, -1};
    struct Vector camUp = {0, 1, 0};
    set_camera_default(camPos, camLook, camUp);
    set_camera_default_software(camPos, camLook, camUp);

    // Setting up voxels
    v_pos startPos = {32, 32, 32};
    v_pos firstPos = {0, 0, 0};
    v_pos endPos = {SIDE_LEN-1, SIDE_LEN-1, SIDE_LEN-1};
    // fill_voxel_range(firstPos, endPos, 0x0);
    set_voxel(startPos, 0x1);
    clear_screen_software();
    wait_for_vsync_software(); // wait_for_vsync();
    while(1) {
        clear_screen_software();
        render_software(); // render();
        wait_for_vsync_software(); // wait_for_vsync();
    }
}
