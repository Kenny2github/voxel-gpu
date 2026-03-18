#include "firmware/character_print.h"
#include "hardware/hardware.h"

static void draw_character(int x, int y, char c) {
	*(uint8_t *)(CHAR_BUF_CTRL->back_buffer + (y << 7) + x) = c;
}

void draw_string(char* buffer, int len, int y) {
    for (int i = 0; i < SCREEN_CHAR_W; ++i) {
		draw_character(i, y, 0);
	}

    for (int i = 0; i < len; ++i) {
		draw_character(i, y, buffer[i]);
	}

}

void clear_char_screen() {
    for(int i = 0; i < SCREEN_CHAR_W; i++)
        for(int j = 0; j < SCREEN_CHAR_H; j++)
            draw_character(i, j, 0);
}