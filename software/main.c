#include <stdio.h>
#include "hardware/hardware.h"
#include "software/controls.h"
#include "firmware/interrupts.h"
#include "software/external.h"
#include "firmware/firmware.h"
#include "software/software_render.h"
#include "model-headers/monkey.h"
int main(void) {
    reset_hex();

    // Setting up interrupts
    config_inputs();
    init_firmware();
    // setup_pixel_buffer_software();
    config_interrupts();

    set_camera_settings(90.0, 1);
    // set_camera_settings_software(90.0, 1);

    // Setting up camera
    struct Vector camPos = {20, 0.5, 0.5};
    struct Vector camLook = {-1, 0, 0};
    struct Vector camUp = {0, 1, 0};
    set_camera_default(camPos, camLook, camUp);
    // set_camera_default_software(camPos, camLook, camUp);

    // Setting up voxels
    // v_pos startPos = {32, 32, 32};
    // v_pos firstPos = {0, 0, 0};
    // v_pos endPos = {SIDE_LEN-1, SIDE_LEN-1, SIDE_LEN-1};
    // fill_voxel_range(firstPos, endPos, 0x0);

    set_voxel((v_pos){1,0,0}, 1);
    set_voxel((v_pos){1,2,2}, 1);
    set_voxel((v_pos){1,2,-2}, 1);
    set_voxel((v_pos){1,-2,2}, 1);
    set_voxel((v_pos){1,-2,-2}, 1);
    set_voxel((v_pos){-1,2,2}, 1);
    set_voxel((v_pos){-1,2,-2}, 1);
    set_voxel((v_pos){-1,-2,2}, 1);
    set_voxel((v_pos){-1,-2,-2}, 1);
    // set_voxel((v_pos){34, 32, 32}, 1);
    // load_monkey();
    // clear_screen_software();
    // wait_for_vsync_software(); // wait_for_vsync();

    uint8_t handle_enter_key = 0;

    while(1) {

        if (enter_key_held)
        {
            if (!handle_enter_key)
            {
                uint8_t target_x, target_y, target_z;

                if (get_target_voxel(&target_x, &target_y, &target_z))
                {
                    v_pos new_voxel_pos = {target_x, target_y, target_z};
                    set_voxel(new_voxel_pos, 1);
                    // printf("Voxel placed at: %d, %d, %d\n", target_x, target_y, target_z); // Comment out later
                }

                handle_enter_key = 1;
            }
        }
        else
        {
            handle_enter_key = 0;
        }

        // clear_screen_software();
        render();
    }
}
