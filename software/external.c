#include "software/external.h"
#include "hardware/hardware.h"

void display_hex(int leftPos, int num) {
    int pos = 5 - leftPos;
    int one = num%10;

    switch(pos) {
        case 0:
            HEX3_HEX0->hex0 = hex_digits[one];
            break;
        case 1:
            HEX3_HEX0->hex1 = hex_digits[one];
            break;
        case 2:
            HEX3_HEX0->hex2 = hex_digits[one];
            break;
        case 3:
            HEX3_HEX0->hex3 = hex_digits[one];
            break;
        case 4:
            HEX5_HEX4->hex4 = hex_digits[one];
            break;
        case 5:
            HEX5_HEX4->hex5 = hex_digits[one];
            break;
    }
}

void reset_hex() {
    *((volatile uint32_t*)HEX3_HEX0) = 0x0;
    *((volatile uint32_t*)HEX5_HEX4) = 0x0;
}