#include <stdio.h>
#include "../hardware/hardware.h"

int main(void) {
    printf(
        "%p: pixel_buffer1 = %p\n", &((*GPU).pixel_buffer1), GPU->pixel_buffer1
    );
    printf(
        "%p: pixel_buffer2 = %p\n", &((*GPU).pixel_buffer2), GPU->pixel_buffer2
    );
    GPU->pixel_buffer2 = (unsigned char *)SDRAM_BASE;
    printf(
        "%p: pixel_buffer2 = %p\n", &((*GPU).pixel_buffer2), GPU->pixel_buffer2
    );
    while (1) {
    }
}
