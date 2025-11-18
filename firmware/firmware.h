#ifndef FIRMWARE_H
#define FIRMWARE_H

/* defs */
#define BLANK 0x0
#define WHITE 0x1

#define PALETTE_START 0xC2FFFF00 // enough for 128 colors (1B palette -> 2B color)
#define GRID_START 0xC3000000

#define SIDE_LEN 256

/* main */
void init_firmware();

/* voxels */
typedef struct v_pos {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} v_pos;

void init_voxel_space();

void set_voxel(v_pos pos, uint8_t palette);

void set_voxel_range(v_pos corner0, v_pos corner1, uint8_t palette);

/* camera */
typedef struct cam_pos {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} cam_pos;

/**
* Sets the camera position and orientation in the voxel space.
* the position of the camera and the top left / top right
* / bottom left positions are written to HW
* @param cam The position of the camera.
* @param lookAt The point the camera is looking at.
* @param up The up direction for the camera.
*/
void set_camera(cam_pos cam, cam_pos look_at, cam_pos up);

#endif
