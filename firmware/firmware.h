#ifndef FIRMWARE_H
#define FIRMWARE_H

/* defs */
#define BLANK 0x0
#define WHITE 0x1

#define GRID_START 0xC2000000 // for now ive defined it as halfway of SDRAM, placement tbd

/* main */
void init_firmware();

/* voxels */
typedef struct v_pos {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} v_pos;

// void init_voxel_space();

void set_voxel(v_pos pos, uint16_t palette);

void set_voxel_range(v_pos corner0, v_pos corner1, uint16_t palette);

/* camera */
typedef struct cam_pos {
    float x;
    float y;
    float z;
} cam_pos;

#endif
