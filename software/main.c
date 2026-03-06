#include <stdio.h>
#include "hardware/hardware.h"
#include "software/controls.h"
#include "firmware/interrupts.h"
#include "software/external.h"
#include "firmware/firmware.h"
#include "software/software_render.h"
#include "model-headers/monkey.h"
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
    
    set_voxel(startPos, 1);
    // set_voxel((v_pos){34, 32, 32}, 1);
    // load_monkey();
    clear_screen_software();
    wait_for_vsync_software(); // wait_for_vsync();

    // Voxel placement set up - For placing voxels when holding down right click
    // int last_placed_x = -1;
    // int last_placed_y = -1;
    // int last_placed_z = -1;
    // int hold_cooldown = 0;

    // Voxel placement set up - For placing voxels with click and release
    uint8_t handle_right_click = 0;

    while(1) {
        
        if (mouse_right_click_held)
        {
            if (!handle_right_click)
            {
                uint8_t target_x, target_y, target_z;

                if (get_target_voxel(&target_x, &target_y, &target_z))
                {
                    v_pos new_voxel_pos = {target_x, target_y, target_z};
                    set_voxel(new_voxel_pos, 1);
                    printf("Voxel placed at: %d, %d, %d\n", target_x, target_y, target_z);
                }

                handle_right_click = 1;
            }
        }
        else
        {
            handle_right_click = 0;
        }

        clear_screen_software();
        render_software(); // render();
        wait_for_vsync_software(); // wait_for_vsync();
    }
}
