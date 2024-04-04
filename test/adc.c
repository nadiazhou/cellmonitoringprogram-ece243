#include <stdio.h>

volatile int ADC_BASE = 0xFF204000;
volatile int HEX_BASE = 0xFF200020;

// ADC register offsets
// data in lower 12 bits
struct ADC_t {
    volatile unsigned int channel0;  // write 1 to start conversion
    volatile unsigned int channel1;  // write 1 to auto sample
    volatile unsigned int channel2;
    volatile unsigned int channel3;
    volatile unsigned int channel4;
    volatile unsigned int channel5;
    volatile unsigned int channel6;
    volatile unsigned int channel7;
};

struct HEX_t {
    volatile unsigned char HEX0;
    volatile unsigned char HEX1;
    volatile unsigned char HEX2;
    volatile unsigned char HEX3;
    volatile unsigned char HEX4;
    volatile unsigned char HEX5;
};

int main(void) {
    struct ADC_t *const ADCp = (struct ADC_t *)ADC_BASE;
    struct HEX_t *const HEXp = (struct HEX_t *)HEX_BASE;

    while (1) {
        
    }
}

void WriteIndividualHEX(struct HEX_t HEXp, int value) {

}