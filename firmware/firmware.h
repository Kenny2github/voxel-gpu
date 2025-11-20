#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <stdint.h>
#include "software/controls.h"

/* defs */
#define BLANK 0x0
#define WHITE 0x1

#define PALETTE_START 0xC2FFFF00 // enough for 128 colors (1B palette -> 2B color)
#define GRID_START 0xC3000000

#define SIDE_LEN 256
#define ASPECT_RATIO (4.0f/3.0f)

/* main */

/**
 * initializes firmware settings and hardware registers
 */
void init_firmware();

/**
 * starts a render and waits for interrupt to signal completion
 */
void render();


/* voxels */

typedef struct v_pos {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} v_pos;

/**
 * sets voxel at pos to the given palette index.
 * @param pos position of the voxel to set
 * @param palette palette index to set the voxel to
 */
void set_voxel(v_pos pos, uint8_t palette);

/**
 * sets all voxels in cube defined by corner0 and corner1
 * to the given palette index.
 * @param corner0 one corner of cube
 * @param corner1 opposite corner of cube
 * @param palette palette index to set the voxels to
 */
void fill_voxel_range(v_pos corner0, v_pos corner1, uint8_t palette);



/* camera */

typedef struct cam_pos {
    float x;
    float y;
    float z;
} cam_pos;

/**
 * configures camera settings.
 * @param _fov_degrees field of view in degrees
 * @param _focal_length focal length in voxel units
 */
void set_camera_settings(float _fov_degrees, float _focal_length);

/**
* sets camera position and orientation in the voxel space.
* the position of the camera and the top left / top right
* / bottom left positions are written to HW.
* set_camera_settings must be called before this function.
* @param cam The position of the camera.
* @param lookAt The point the camera is looking at.
* @param up The up direction for the camera.
*/
void set_camera(struct Camera* camera);

#endif
