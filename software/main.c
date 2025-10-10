#include <stdio.h>

int * const DEADBEEF = (int *)0xFF20BEF0;

int main(void) {
	printf("Hello world!");
	printf("0xFF20BEF0 = %08X", *DEADBEEF);
	while (1);
}
