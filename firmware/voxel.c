#include "hardware/hardware.h"
#include "firmware/firmware.h"

#define abs(x) ((x >= 0) ? (x) : (-x))

void set_voxel(v_pos pos, uint8_t palette) {
    *(GPU->voxel_buffer + pos.x + pos.y * SIDE_LEN + pos.z * SIDE_LEN * SIDE_LEN) = palette;
    ++GPU->voxel_count;
    render();
}

void fill_voxel_range(v_pos corner0, v_pos corner1, uint8_t palette) {
    v_pos start;
    v_pos end;
    uint8_t dist_x, dist_y, dist_z;

    if (corner0.x < corner1.x) {
        start.x = corner0.x;
        end.x = corner1.x;
    } else {
        start.x = corner1.x;
        end.x = corner0.x;
    }
    dist_x = end.x - start.x + 1;

    if (corner0.y < corner1.y) {
        start.y = corner0.y;
        end.y = corner1.y;
    } else {
        start.y = corner1.y;
        end.y = corner0.y;
    }
    dist_y = end.y - start.y + 1;

    if (corner0.z < corner1.z) {
        start.z = corner0.z;
        end.z = corner1.z;
    } else {
        start.z = corner1.z;
        end.z = corner0.z;
    }
    dist_z = end.z - start.z + 1;

    /* xy horizontal, y vertical */
    unsigned char* y_offset;
    unsigned char* voxel_offset;
    for (uint8_t y = start.y; y <= end.y; ++y) {
        y_offset = GPU->voxel_buffer + y * SIDE_LEN * SIDE_LEN;
        for (uint8_t z = start.z; z <= end.z; ++z) {
            voxel_offset = y_offset + z * SIDE_LEN;
            memset(voxel_offset, palette, dist_x);
        }
    }

    /* TODO: what to do when calling this function over a range that may already contain voxels? */
    GPU->voxel_count += dist_x * dist_y * dist_z;

    render();
}