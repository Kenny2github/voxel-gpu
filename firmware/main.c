#include "hardware/hardware.h"
#include "firmware/firmware.h"

void init_firmware() {
    GPU->voxel_buffer = (unsigned char *)GRID_START;
    GPU->voxel_count = 0;

    GPU->palette_buffer = (unsigned char *)PALETTE_START;
    GPU->palette_length = 128;

    // memset(GPU->palette_buffer + 2, 0x0, 1);
    // memset(GPU->palette_buffer + 3, 0x1, 1); 
}