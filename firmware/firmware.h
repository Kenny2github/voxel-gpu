#ifndef FIRMWARE_H
#define FIRMWARE_H

/* defs */
#define BLANK 0x0
#define WHITE 0x1

#define GRID_START 0xC3000000 // for now ive defined it as the lower end of SDRAM occupying (2^16^3)B, placement tbd

/* main */
void init_firmware();

/* voxels */
typedef struct v_pos {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} v_pos;

void init_voxel_space(uint8_t len);

void set_voxel(v_pos pos, uint8_t palette);

void set_voxel_range(v_pos corner0, v_pos corner1, uint8_t palette);

/* camera */
typedef struct cam_pos {
    float x;
    float y;
    float z;
} cam_pos;

#endif
