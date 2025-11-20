#include <stdio.h>
#include "hardware/hardware.h"
#include "software/controls.h"
#include "firmware/interrupts.h"
#include "software/external.h"
#include "firmware/firmware.h"
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
    config_interrupts();
    
    set_camera_settings(90.0, 1);

    // Setting up camera
    struct Vector camPos = {128, 128, 130};
    struct Vector camLook = {0, 0, -1};
    struct Vector camUp = {0, 1, 0};
    set_camera_default(camPos, camLook, camUp);

    // Setting up voxels
    v_pos startPos = {128, 128, 128};
    v_pos firstPos = {0, 0, 0};
    v_pos endPos = {SIDE_LEN-1, SIDE_LEN-1, SIDE_LEN-1};
    // fill_voxel_range(firstPos, endPos, 0x0);
    set_voxel(startPos, 1);
    
    while(1) {
        render();
    }
}
