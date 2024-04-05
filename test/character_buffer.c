#include <stdio.h>

volatile int CHARACTER_BUFFER_BASE = 0x09000000;

int main(void) {
    volatile int *character_buffer = (int *)CHARACTER_BUFFER_BASE;
    while (1) {
        for (int i = 0; i < 240; i++) {
            for (int j = 0; j < 320; j++) {
                character_buffer[i * 320 + j] = 0x30 + (i % 10);
            }
        }
    }
}