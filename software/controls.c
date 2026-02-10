#include "software/controls.h"
#include "hardware/hardware.h"
#include "firmware/interrupts.h"
#include "firmware/firmware.h"
#include "software/software_render.h"
#include <stdlib.h>
#include <stdio.h>

static struct Camera *camera = NULL;

void config_inputs(void) {

    // NOTE: May need to swap the IRQ IDs (and the related buffers) if needed, or simply swap the ports
    config_interrupt(PS2_DUAL_IRQ, config_mouse, mouse_input_handler);
    config_interrupt(PS2_IRQ, config_keyboard, keyboard_input_handler);    
}

void config_mouse(void) {
    PS2_DUAL->re = 0x1;

    camera = malloc(sizeof(struct Camera));
    *camera = (struct Camera){
        {0, 0, 256}, // pos
        {0, 0, -1}, // look
        {0, 1, 0}, // up
        {1, 0, 0} // right
    };
}

void config_keyboard(void) {
    PS2->re = 0x1;
}

void set_camera_default(struct Vector pos, struct Vector look, struct Vector up) {
    *camera = (struct Camera){
        pos,
        look,
        up,
        {0, 0, 1}
    };

    cross_product(&(camera->look), &(camera->up), &(camera->right));
    normalize(&(camera->look));
    normalize(&(camera->up));
    normalize(&(camera->right));

}

struct movement_key_status movement_keys_bool = {
    0, 0, 0, 0
};

void mouse_input_handler() {

    struct ps2_data PS2_data;

    int numOfBytes = 0;
    unsigned char mousePackets[3] = {0, 0, 0};

    /*** Reading mouse data */
    while (numOfBytes < 3) {
        PS2_data = PS2_DUAL->data;

        if(PS2_data.rvalid) {

            mousePackets[0] = mousePackets[1];
            mousePackets[1] = mousePackets[2];
            mousePackets[2] = PS2_data.data;

            if(mouse_status == REPORTING)
                numOfBytes++;
            else if(mouse_status == DEFAULT && mousePackets[1] == (unsigned char)0xAA && mousePackets[2] == (unsigned char)0x00) {
                mouse_status = WAIT_ACKNOWLEDGE;
                *((volatile int*)PS2_DUAL) = 0xF4;
            } else if(mouse_status == WAIT_ACKNOWLEDGE && mousePackets[2] == 0xFA) {
                mouse_status = REPORTING;
                continue;
            }
        }

    }

    struct {
        signed int x : 9;
        signed int y : 9;
    } signedPos;

    signedPos.x = ((int)(mousePackets[0] & 0b10000) << 4) | (mousePackets[1]);
    signedPos.y = -(((int)(mousePackets[0] & 0b100000) << 3) | (mousePackets[2]));
    /*** Movement of Camera Look */
    // Horizontal motion should rotate lookAt vector based on up-vector
    float angle_x = convert_mouse_val_to_rad(signedPos.x, SENSITIVITY_HORIZONTAL);
    // volatile uint32_t* test_x_angle = (volatile uint32_t*)(0xC80000C0);
    // *test_x_angle = convert_float_to_fixed(angle_x);
    if(angle_x != 0.0f) {
        struct AffineTransform3D rotate_horizontal_transform = rotate_transform(angle_x, camera->up);
        camera->look = transform_vector(&(rotate_horizontal_transform), camera->look);
        normalize(&(camera->look));
        cross_product(&(camera->look), &(camera->up), &(camera->right));
        normalize(&(camera->right));
    }

    // Vertical motion should rotate lookAt vector based on right-vector
    float angle_y = convert_mouse_val_to_rad(signedPos.y, SENSITIVITY_VERTICAL);
    // volatile int32_t* test_y_angle = (volatile int32_t*)(0xC80000D0);
    // *test_y_angle = convert_float_to_fixed(angle_y);
    if(angle_y != 0.0f) {
        struct AffineTransform3D rotate_horizontal_transform = rotate_transform(angle_y, camera->right);
        camera->look = transform_vector(&(rotate_horizontal_transform), camera->look);
        normalize(&(camera->look));
        cross_product(&(camera->right), &(camera->look), &(camera->up));
        normalize(&(camera->up));
    }
}

void keyboard_input_handler() {
    struct ps2_data PS2_data;

    uint8_t done = 0;
    unsigned char data[3];

    /*** Reading key press data */
    while(!done) {
        PS2_data = PS2->data;
        if (!PS2_data.rvalid) break;
        data[0] = data[1];
        data[1] = data[2];
        data[2] = PS2_data.data;

        if(keyboard_status == REPORTING && data[2] != 0xE0 && data[2] != 0xF0)
            done = 1;

        if(keyboard_status == DEFAULT && data[2] == 0xAA) {
            keyboard_status = REPORTING;
            continue;
        }
    }

    struct Vector applicable_vector = {0};

    if(data[0] == 0xF0 || data[1] == 0xF0) // If there is a break code detecting key releases
        return;

    /*** Movement of Camera Position */
    switch(data[2]) {
        case SPACE_KEY:
            applicable_vector = camera->up;
            //printf("Space was pressed\n");
            break;
        case SHIFT_KEY:
            applicable_vector = camera->up;
            negative_vector(&applicable_vector);
            //printf("Shift was pressed\n");
            break;
        case A_KEY:
            applicable_vector = camera->right;
            negative_vector(&applicable_vector);
            //printf("A was pressed\n");
            break;
        case D_KEY:
            applicable_vector = camera->right;
            //printf("D was pressed\n");
            break;
        case W_KEY:
            applicable_vector = camera->look;
            //printf("W was pressed\n");
            break;
        case S_KEY:
            applicable_vector = camera->look;
            negative_vector(&applicable_vector);
            //printf("S was pressed\n");
        default:
            break;
    }

    if(applicable_vector.x == 0 && applicable_vector.y == 0 && applicable_vector.z == 0) // No movement
        return;

    applicable_vector = multiply_vector(applicable_vector, MOVEMENT_SPEED);
    camera->pos = add_vector(camera->pos, applicable_vector);

    /** Movement of Camera View */
    float angle_x = 0;
    float angle_y = 0;
    if(data[1] == ARROW_KEY) {
        switch(data[2]) {
            case ARROW_LEFT:
                angle_y = M_PI / 6.0;
                break;
            case ARROW_RIGHT:
                angle_y = -M_PI / 6.0;
                break;
            case ARROW_UP:
                angle_x = M_PI / 6.0;
                break;
            case ARROW_DOWN:
                angle_x = -M_PI / 6.0;;
                break;
            default:
                break;
        }

        if(angle_x != 0.0f) {
            struct AffineTransform3D rotate_horizontal_transform = rotate_transform(angle_x, camera->up);
            camera->look = transform_vector(&(rotate_horizontal_transform), camera->look);
            normalize(&(camera->look));
            cross_product(&(camera->look), &(camera->up), &(camera->right));
            normalize(&(camera->right));
        }

        if(angle_y != 0.0f) {
            struct AffineTransform3D rotate_horizontal_transform = rotate_transform(angle_y, camera->right);
            camera->look = transform_vector(&(rotate_horizontal_transform), camera->look);
            normalize(&(camera->look));
            cross_product(&(camera->right), &(camera->look), &(camera->up));
            normalize(&(camera->up));
        }

    }
}

float convert_mouse_val_to_rad(const int x, const float ratio) {
    return ratio * x * (M_PI / 180.0f);
}

void update_camera() {
    set_camera(camera);
    set_camera_software(camera);
}
