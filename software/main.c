#include <stdio.h>
#include "hardware/hardware.h"
#include "software/controls.h"
#include "firmware/interrupts.h"
#include "software/external.h"
#include "firmware/firmware.h"
#include "software/software_render.h"
#include "model-headers/monkey.h"
#include "firmware/timing.h"

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

    uint32_t prev_time = cur_time() * 0.001;
    float movement_speed = 0.2f;

    while(1) {
        // Handle movement loop
        {
            uint32_t curr_time = cur_time() * 0.001 + fw_time * 1000;
            struct Vector applicable_vector = {0};
            int32_t temp_delta = curr_time - prev_time;
            if(temp_delta < 0)
                temp_delta = (1000 - prev_time) + curr_time;

            float dt = temp_delta * 0.001f;
            struct Vector temp_vector = {0};
            uint8_t changed = 0;

            temp_vector = camera.look;

            struct MovementPacket tempMovement = movement;
            struct RotatePacket tempRotate = rotate;
            rotate = (struct RotatePacket){0, 0};
            if(tempMovement.forward && !tempMovement.backward) {
                changed = 1;
                applicable_vector = add_vector(applicable_vector, temp_vector);
            } else if(!tempMovement.forward && tempMovement.backward) {
                changed = 1;
                negative_vector(&temp_vector);
                applicable_vector = add_vector(applicable_vector, temp_vector);
            }

            temp_vector = camera.right;
            if(tempMovement.right && !tempMovement.left) {
                changed = 1;
                applicable_vector = add_vector(applicable_vector, temp_vector);
            } else if(!tempMovement.right && tempMovement.left) {
                changed = 1;
                negative_vector(&temp_vector);
                applicable_vector = add_vector(applicable_vector, temp_vector);
            }

            temp_vector = camera.up;
            if(tempMovement.up && !tempMovement.down) {
                changed = 1;
                applicable_vector = add_vector(applicable_vector, temp_vector);
            } else if(!tempMovement.up && tempMovement.down) {
                changed = 1;
                negative_vector(&temp_vector);
                applicable_vector = add_vector(applicable_vector, temp_vector);
            }

            if(changed)
                camera.pos = add_vector(camera.pos, multiply_vector(applicable_vector, dt * movement_speed));

            if(tempRotate.mouse_dx != 0) {
                float angle_x = convert_mouse_val_to_rad(tempRotate.mouse_dx, SENSITIVITY_HORIZONTAL);
                struct AffineTransform3D rotate_horizontal_transform = rotate_transform(angle_x, camera.true_up);
                camera.look = transform_vector(&(rotate_horizontal_transform), camera.look);

            }
            if(tempRotate.mouse_dy != 0) {
                float angle_y = convert_mouse_val_to_rad(tempRotate.mouse_dy, SENSITIVITY_VERTICAL);
                struct AffineTransform3D rotate_horizontal_transform = rotate_transform(angle_y, camera.right);
                camera.look = transform_vector(&(rotate_horizontal_transform), camera.look);
            }

            if(tempRotate.mouse_dx != 0 || tempRotate.mouse_dy != 0) {
                normalize(&(camera.look));
                cross_product(&(camera.look), &(camera.true_up), &(camera.right));
                normalize(&(camera.right));
                cross_product(&(camera.right), &(camera.look), &(camera.up));
                normalize(&(camera.up));
            }

            prev_time = curr_time;
        }


        clear_screen_software();
        render_software(); // render();
        wait_for_vsync_software(); // wait_for_vsync();
        
    }
}
