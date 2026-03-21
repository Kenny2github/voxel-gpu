#include "hardware/hardware.h"
#include "firmware/firmware.h"
#include <string.h>

#define abs(x) ((x >= 0) ? (x) : (-x))

unsigned int voxel_count;
struct gpu_voxel* voxel_space;
unsigned int voxel_space_size;

void set_voxel(v_pos pos, uint8_t palette) {
    if (voxel_count == voxel_space_size) {
        voxel_space_size *= 2;
        voxel_space = (struct gpu_voxel*)realloc(voxel_space, voxel_space_size * sizeof(struct gpu_voxel));
        if (voxel_space == NULL) {
            printf("Failed to allocate memory for voxel space\n");
            while (1);
        }
    }
    voxel_space[voxel_count++] = (struct gpu_voxel){
        .x = pos.x,
        .y = pos.y,
        .z = pos.z,
        .voxel_id = palette
    };
}

void init_voxel_list(void) {
    voxel_count = 0;
    voxel_space_size = 256;
    voxel_space = calloc(voxel_space_size, sizeof(uint32_t));
}

void clear_voxel_list(void) {
    if (voxel_space != NULL) {
        free(voxel_space);
        voxel_space = NULL;
    }
    voxel_count = 0;
}
