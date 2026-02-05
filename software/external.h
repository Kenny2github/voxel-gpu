#ifndef EXTERNAL_H
#define EXTERNAL_H

static int hex_digits[10] = {
    0x3F, // 0 
    0x06, // 1 
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

void display_hex(int pos, int digit);

#endif