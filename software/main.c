#include <stdio.h>
#include "hardware/hardware.h"
#include "software/controls.h"
#include "firmware/interrupts.h"
#include "software/external.h"
int main(void) {
    // printf(
    //     "%p: pixel_buffer1 = %p\n", &((*GPU).pixel_buffer1), GPU->pixel_buffer1
    // );
    // printf(
    //     "%p: pixel_buffer2 = %p\n", &((*GPU).pixel_buffer2), GPU->pixel_buffer2
    // );
    // GPU->pixel_buffer2 = (unsigned char *)SDRAM_BASE;
    // printf(
    //     "%p: pixel_buffer2 = %p\n", &((*GPU).pixel_buffer2), GPU->pixel_buffer2
    // );
    reset_hex();
    config_inputs();
    config_interrupts();
    while (1);
}
