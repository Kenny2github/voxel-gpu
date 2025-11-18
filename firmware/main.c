#include "hardware/hardware.h"
#include "firmware/firmware.h"

void init_firmware() {
    GPU->voxel_buffer = (unsigned char *)GRID_START;
    GPU->voxel_count = 0;
}