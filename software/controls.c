#include "software/controls.h"
#include "hardware/hardware.h"
#include "firmware/interrupts.h"
#include "firmware/firmware.h"
#include "software/software_render.h"
#include "firmware/character_print.h"
#include <stdlib.h>
#include <stdio.h>

static struct Camera *camera = NULL;

volatile uint8_t enter_key_held = 0;

void draw_character(int x, int y, char c) {
	*(uint8_t *)(CHAR_BUF_CTRL + (y << 7) + x) = c;
}

void config_inputs(void) {

    // NOTE: May need to swap the IRQ IDs (and the related buffers) if needed, or simply swap the ports
    config_interrupt(PS2_DUAL_IRQ, config_mouse, mouse_input_handler);
    config_interrupt(PS2_IRQ, config_keyboard, keyboard_input_handler);    
}

void config_mouse(void) {
    PS2_DUAL->flags.re = 0x1;

    camera = malloc(sizeof(struct Camera));
    *camera = (struct Camera){
        {0, 0, 256}, // pos
        {0, 0, -1}, // look
        {0, 1, 0}, // up
        {1, 0, 0} // right
    };
}

void config_keyboard(void) {
    PS2->flags.re = 0x1;
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
    signedPos.y = (((int)(mousePackets[0] & 0b100000) << 3) | (mousePackets[2]));
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
    unsigned char data[3] = {0, 0, 0};

    /*** Reading key press data */
    int presses = 0;
    while(!done) {
        PS2_data = PS2->data;
        // if (!PS2_data.rvalid) break;
        data[0] = data[1];
        data[1] = data[2];
        data[2] = PS2_data.data;

        if(keyboard_status == REPORTING && data[2] != 0xE0 && data[2] != 0xF0) {
            done = 1; presses++;
        }

        if(keyboard_status == DEFAULT && data[2] == 0xAA) {
            keyboard_status = REPORTING;
            return;
        }
    }

    struct Vector applicable_vector = {0};
    
    char hex[10];
    sprintf(hex, "%02X %02X %02X %d", data[0], data[1], data[2], presses);
    draw_string(hex, 10, 30);

    if(data[0] == 0xF0 || data[1] == 0xF0) { // If there is a break code detecting key releases
        if (data[1] == ENTER_KEY || data[2] == ENTER_KEY){
            enter_key_held = 0;
        }
        return;
    }

    /*** Movement of Camera Position */

	int len = 0;
    char buffer[SCREEN_CHAR_W];

    switch(data[2]) {
        case SPACE_KEY:
            applicable_vector = camera->up;
            //printf("Space was pressed\n");
            len =  sprintf(buffer, "Space was pressed");
            break;
        case SHIFT_KEY:
            applicable_vector = camera->up;
            negative_vector(&applicable_vector);
            //printf("Shift was pressed\n");
            len =  sprintf(buffer, "Shift was pressed");
            break;
        case ENTER_KEY:
            enter_key_held = 1;
            break;
        case A_KEY:
            applicable_vector = camera->right;
            negative_vector(&applicable_vector);
            //printf("A was pressed\n");
            len =  sprintf(buffer, "A was pressed");
            break;
        case D_KEY:
            applicable_vector = camera->right;
            //printf("D was pressed\n");
            len =  sprintf(buffer, "D was pressed");
            break;
        case W_KEY:
            applicable_vector = camera->look;
            //printf("W was pressed\n");
            len =  sprintf(buffer, "W was pressed");
            break;
        case S_KEY:
            applicable_vector = camera->look;
            negative_vector(&applicable_vector);
            //printf("S was pressed\n");
            len =  sprintf(buffer, "S was pressed");
        default:
            break;
    }


    if(applicable_vector.x || applicable_vector.y || applicable_vector.z) { // No movement
        applicable_vector = multiply_vector(applicable_vector, MOVEMENT_SPEED);
        camera->pos = add_vector(camera->pos, applicable_vector);
    }

    /** Movement of Camera View */
    float angle_x = 0;
    float angle_y = 0;

    if(data[1] == ARROW_KEY) {
        switch(data[2]) {
            case ARROW_LEFT:
                angle_x = -M_PI / 6.0f;
            len =  sprintf(buffer, "Left was pressed");
                break;
            case ARROW_RIGHT:
                angle_x = M_PI / 6.0f;
            len =  sprintf(buffer, "Right was pressed");
                break;
            case ARROW_UP:
                angle_y = -M_PI / 6.0f;
            len =  sprintf(buffer, "Up was pressed");
                break;
            case ARROW_DOWN:
                angle_y = M_PI / 6.0f;
            len =  sprintf(buffer, "Down was pressed");
                break;
            default:
                break;
        }
    } else {
        switch(data[2]) {
            case J_KEY:
                angle_x = -M_PI / 6.0f;
            len =  sprintf(buffer, "J was pressed");
                break;
            case L_KEY:
                angle_x = M_PI / 6.0f;
            len =  sprintf(buffer, "L was pressed");
                break;
            case I_KEY:
                angle_y = -M_PI / 6.0f;
            len =  sprintf(buffer, "I was pressed");
                break;
            case K_KEY:
                angle_y = M_PI / 6.0f;
            len =  sprintf(buffer, "K was pressed");
                break;
            default:
                break;
        }
    }

    draw_string(buffer, len, 1);

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

float convert_mouse_val_to_rad(const int x, const float ratio) {
    return ratio * x * (M_PI / 180.0f);
}

void update_camera() {
    set_camera(camera);
    set_camera_software(camera);
}

uint8_t get_target_voxel(uint8_t *x, uint8_t *y, uint8_t *z) {
    // Currently, just place 2 voxels ahead of where camera looking
    struct Vector offset = multiply_vector(camera->look, 3.0f);
    struct Vector target_pos = add_vector(camera->pos, offset);

    int cx = (int)target_pos.x;
    int cy = (int)target_pos.y;
    int cz = (int)target_pos.z;

    if (cx >= 0 && cx < SIDE_LEN &&
        cy >= 0 && cy < SIDE_LEN &&
        cz >= 0 && cz < SIDE_LEN) 
    {       
        *x = (uint8_t)cx;
        *y = (uint8_t)cy;
        *z = (uint8_t)cz;
        return 1;
    }
    return 0;
}