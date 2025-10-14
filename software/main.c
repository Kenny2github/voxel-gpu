#include <stdio.h>

int * const DEADBEEF = (int *)0xFF20BEF0;

int main(void) {
	printf("Hello world!\n");
	printf("0xFF20BEF0 = %08X\n", *DEADBEEF);
	while (1);
}
