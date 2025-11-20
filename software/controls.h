#ifndef CONTROLS_H
#define CONTROLS_H

#include "firmware/interrupts.h"
#include "software/vector_math.h"
#include <stdint.h>

#define MOUSE_IRQ_ID 79
#define KEYBOARD_IRQ_ID 89

#define SENSITIVITY_VERTICAL 1.0
#define SENSITIVITY_HORIZONTAL 1.0

#define W_KEY 0x1D
#define A_KEY 0x1C
#define D_KEY 0x23
#define S_KEY 0x1B
#define SPACE_KEY 0x29
#define SHIFT_KEY 0x12

#define MOVEMENT_SPEED 1.0

/**
 * @brief PS/2 protocol status
 */
typedef enum {
    DEFAULT,
    WAIT_ACKNOWLEDGE,
    REPORTING,
} PS2_status;

static PS2_status mouse_status;
static PS2_status keyboard_status;

struct movement_key_status {
    uint32_t forward : 1;
    uint32_t backward : 1;
    uint32_t left : 1;
    uint32_t right : 1;
};

struct Camera {
    struct Vector pos; // 8 bits integer coordinate, 8 bits fraction precision
    struct Vector look;
    struct Vector up;
    struct Vector right;
};

struct Camera_formatted {
    struct Vector_16fixed pos;
    struct Vector_16fixed look;
    struct Vector_16fixed up;
    struct Vector_16fixed right;
};

void config_inputs();
void config_mouse();
void config_keyboard();

void set_camera_default(struct Vector pos, struct Vector look, struct Vector up);

void mouse_input_handler();
void keyboard_input_handler();

float convert_mouse_val_to_rad(int x, float ratio); // Ratio is in (pixels / degrees)

#endif