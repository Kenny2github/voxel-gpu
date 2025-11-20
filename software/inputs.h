#ifndef INPUTS_H
#define INPUTS_H

#include <stdio.h>

#define SENSITIVITY 0.20
#define RESOLUTION_X 640
#define RESOLUTION_Y 320

unsigned char mousePackets[3] = {0, 0, 0}; // click = 0, x = 1, y = 2
static int mouse[2];
int leftClick = 0;

int handleNum[3] = {-1, -1, -1}; 
int numOfHandles = 0;

int leftHold = 0;

typedef enum {
    DEFAULT,
    WAIT_ACKNOWLEDGE,
    REPORTING,
} Status;
Status currentStatus = DEFAULT;

static void init_PS2();

#endif