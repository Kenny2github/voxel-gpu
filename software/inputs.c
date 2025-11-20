#include "software/inputs.h"
#include "hardware/hardware.h"
#include "firmware/interrupts.h"

void config_PS2(void) {
    volatile int* ptr = (volatile int*)0xFF200100;
    *(ptr + 0x1) = 0x1;
}

void mouseInput() {
    volatile int * PS2_ptr = (int *) PS2;
    int PS2_data, RVALID;

    int numOfBytes = 0;

    *(PS2_ptr) = 0xFF;

    while (numOfBytes < 3) {
        PS2_data = *(PS2_ptr);
        RVALID = (PS2_data & 0x8000);

        if (RVALID) {

            mousePackets[0] = mousePackets[1];
            mousePackets[1] = mousePackets[2];
            mousePackets[2] = PS2_data & 0xFF;

            if(currentStatus == REPORTING)
                numOfBytes++;


            if(currentStatus == DEFAULT && mousePackets[1] == (unsigned char)0xAA && mousePackets[2] == (unsigned char)0x00) {
                currentStatus = WAIT_ACKNOWLEDGE;
                *(PS2_ptr) = 0xF4;
            } 

            if(currentStatus == WAIT_ACKNOWLEDGE && mousePackets[2] == 0xFA) {
                currentStatus = REPORTING;
                continue;
            }
        }

    }

    struct {
        signed int x : 9;
        signed int y : 9;
    } signedPos;

    signedPos.x = ((mousePackets[0] & 0b10000) << 4) | (mousePackets[1]);
    signedPos.y = ((mousePackets[0] & 0b100000) << 3) | (mousePackets[2]);


    mouse[0] += signedPos.x * SENSITIVITY;
    mouse[1] -= signedPos.y * SENSITIVITY;
    
    if(mouse[0] > RESOLUTION_X)
        mouse[0] = RESOLUTION_X;
    if(mouse[1] > RESOLUTION_Y)
        mouse[1] = RESOLUTION_Y;

    if(mouse[0] < 0)
        mouse[0] = 0;
    if(mouse[1] < 0)
        mouse[1] = 0;

    leftClick = mousePackets[0] & 0b1;

    if(leftClick != 1)
        leftHold = 0;
}

void init_PS2() {
    config_interrupt(PS2_IRQ, config_PS2, mouseInput);
}