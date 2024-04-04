#include <stdio.h>
#include <stdlib.h>

volatile int ADC_BASE = 0xFF204000;
volatile int HEX_BASE1 = 0xFF200020;
volatile int HEX_BASE2 = 0xFF200030;

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

int main(void) {
    struct ADC_t *const ADCp = (struct ADC_t *)ADC_BASE;
    while (1) {
        ADCp->channel0 = 1;
        while (ADCp->channel0 & 0x8000) {}
        WriteHEX(ADCp->channel0 & 0xFFF);
    }
}

void WriteHEX(int value) {
    const char kHexCodes[16] = {
        0b00111111,  // 0
        0b00000110,  // 1
        0b01011011,  // 2
        0b01001111,  // 3
        0b01100110,  // 4
        0b01101101,  // 5
        0b01111101,  // 6
        0b00000111,  // 7
        0b01111111,  // 8
        0b01101111,  // 9
        0b01110111,  // A
        0b01111100,  // B
        0b00111001,  // C
        0b01011110,  // D
        0b01111001,  // E
        0b01110001   // F
    };

    int* HexAdd1 = (int*)HEX_BASE1;
    int* HexAdd2 = (int*)HEX_BASE2;

    // Clear the HEX displays
    *HexAdd1 = 0;
    *HexAdd2 = 0;
    
    int value0 = kHexCodes[value % 10];
    int value1 = kHexCodes[(value / 10) % 10];
    int value2 = kHexCodes[(value / 100) % 10];
    int value3 = kHexCodes[(value / 1000) % 10];
    int value4 = kHexCodes[(value / 10000) % 10];
    int value5 = kHexCodes[(value / 100000) % 10];

    *HexAdd1 = value0 | (value1 << 8) | (value2 << 16) | (value3 << 24);
    *HexAdd2 = value4 | (value5 << 8);

}