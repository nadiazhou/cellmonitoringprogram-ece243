#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>







#ifndef __NIOS2_CTRL_REG_MACROS__
#define __NIOS2_CTRL_REG_MACROS__
/*****************************************************************************/
/* Macros for accessing the control registers. */
/*****************************************************************************/
#define NIOS2_READ_STATUS(dest) \
    do {                          \
        dest = __builtin_rdctl(0);  \
    } while (0)
#define NIOS2_WRITE_STATUS(src) \
    do {                          \
        __builtin_wrctl(0, src);    \
    } while (0)
#define NIOS2_READ_ESTATUS(dest) \
    do {                           \
        dest = __builtin_rdctl(1);   \
    } while (0)
#define NIOS2_READ_BSTATUS(dest) \
    do {                           \
        dest = __builtin_rdctl(2);   \
    } while (0)
#define NIOS2_READ_IENABLE(dest) \
    do {                           \
        dest = __builtin_rdctl(3);   \
    } while (0)
#define NIOS2_WRITE_IENABLE(src) \
    do {                           \
        __builtin_wrctl(3, src);     \
    } while (0)
#define NIOS2_READ_IPENDING(dest) \
    do {                            \
        dest = __builtin_rdctl(4);    \
    } while (0)
#define NIOS2_READ_CPUID(dest) \
    do {                         \
        dest = __builtin_rdctl(5); \
    } while (0)
#endif









/* the global variables are written by interrupt service routines; we have to
 * declare
 * these as volatile to avoid the compiler caching their values in registers */
volatile int ADC_value = 0;  // ADC value
volatile int voltage[100000] = {0};
volatile int current[100000] = {0};
volatile int power[100000] = {0};
volatile int energy = 0; // Joules
volatile int sample_index = 0;
volatile int voltage_scaling = 28;  // scale of voltage y-axis (mV/pixel) (180 pixel height)
volatile int current_scaling = 23;  // scale of current y-axis (mA/pixel) (180 pixel height)
volatile int x_axis_scaling = 1;  // scale of x-axis in terms of ADC samples (dynamic)
volatile int State = 3;
volatile int mouse_x = 100;
volatile int mouse_y = 100;

volatile int TIMER_BASE = 0xFF202000;
volatile int KEY_BASE = 0xFF200050;
volatile int SW_BASE = 0xFF200040;
volatile int LED_BASE = 0xFF200000;
volatile int ADC_BASE = 0xFF204000;
volatile int HEX_BASE1 = 0xFF200020;
volatile int HEX_BASE2 = 0xFF200030;
volatile int PS2_BASE = 0xFF200100;

volatile int pixel_buffer_start;  // global variable
short int Buffer1[240][512];      // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

static int font[128][2] = {
    {0x7E7E7E7E, 0x00007E7E}, /* NUL */
    {0x7E7E7E7E, 0x00007E7E}, /* SOH */
    {0x7E7E7E7E, 0x00007E7E}, /* STX */
    {0x7E7E7E7E, 0x00007E7E}, /* ETX */
    {0x7E7E7E7E, 0x00007E7E}, /* EOT */
    {0x7E7E7E7E, 0x00007E7E}, /* ENQ */
    {0x7E7E7E7E, 0x00007E7E}, /* ACK */
    {0x7E7E7E7E, 0x00007E7E}, /* BEL */
    {0x7E7E7E7E, 0x00007E7E}, /* BS */
    {0x00000000, 0x00000000}, /* TAB */
    {0x7E7E7E7E, 0x00007E7E}, /* LF */
    {0x7E7E7E7E, 0x00007E7E}, /* VT */
    {0x7E7E7E7E, 0x00007E7E}, /* FF */
    {0x7E7E7E7E, 0x00007E7E}, /* CR */
    {0x7E7E7E7E, 0x00007E7E}, /* SO */
    {0x7E7E7E7E, 0x00007E7E}, /* SI */

    {0x7E7E7E7E, 0x00007E7E}, /* DLE */
    {0x7E7E7E7E, 0x00007E7E}, /* DC1 */
    {0x7E7E7E7E, 0x00007E7E}, /* DC2 */
    {0x7E7E7E7E, 0x00007E7E}, /* DC3 */
    {0x7E7E7E7E, 0x00007E7E}, /* DC4 */
    {0x7E7E7E7E, 0x00007E7E}, /* NAK */
    {0x7E7E7E7E, 0x00007E7E}, /* SYN */
    {0x7E7E7E7E, 0x00007E7E}, /* ETB */
    {0x7E7E7E7E, 0x00007E7E}, /* CAN */
    {0x7E7E7E7E, 0x00007E7E}, /* EM */
    {0x7E7E7E7E, 0x00007E7E}, /* SUB */
    {0x7E7E7E7E, 0x00007E7E}, /* ESC */
    {0x7E7E7E7E, 0x00007E7E}, /* FS */
    {0x7E7E7E7E, 0x00007E7E}, /* GS */
    {0x7E7E7E7E, 0x00007E7E}, /* RS */
    {0x7E7E7E7E, 0x00007E7E}, /* US */

    {0x00000000, 0x00000000}, /* (space) */
    {0x8080808, 0x00080000},  /* ! */
    {0x28280000, 0x00000000}, /* " */
    {0x287C287C, 0x28280000}, /* # */
    {0x81E281C, 0x0A3C0800},  /* $ */
    {0x60946816, 0x29060000}, /* % */
    {0x1C202019, 0x26190000}, /* & */
    {0x8080000, 0x00000000},  /* ' */
    {0x8102020, 0x10080000},  /* ( */
    {0x10080404, 0x08100000}, /* ) */
    {0x2A1C3E1C, 0x2A000000}, /* * */
    {0x0008083E, 0x08080000}, /* + */
    {0x00000000, 0x00081000}, /* , */
    {0x0000003C, 0x00000000}, /* - */
    {0x00000000, 0x00800000}, /* . */
    {0x00020408, 0x10204000}, /* / */

    {0x18244242, 0x24180000}, /* 0 */
    {0x08180808, 0x081C0000}, /* 1 */
    {0x3C420418, 0x207E0000}, /* 2 */
    {0x3C420418, 0x423C0000}, /* 3 */
    {0x8182848, 0x7C080000},  /* 4 */
    {0x7E407C02, 0x423C0000}, /* 5 */
    {0x3C407C42, 0x423C0000}, /* 6 */
    {0x7E040810, 0x20400000}, /* 7 */
    {0x3C423C42, 0x423C0000}, /* 8 */
    {0x3C42423E, 0x023C0000}, /* 9 */
    {0x00000800, 0x00080000}, /* : */
    {0x00000800, 0x00081000}, /* ; */
    {0x06186060, 0x18060000}, /* < */
    {0x00007E00, 0x7E000000}, /* = */
    {0x60180618, 0x60000000}, /* > */
    {0x38440418, 0x00100000}, /* ? */
    {0x03C449C, 0x945C201C},  /* @ */
    {0x3C42423C, 0x42420000}, /* A */
    {0x78447844, 0x44780000}, /* B */
    {0x38448080, 0x44380000}, /* C */
    {0x78444444, 0x44780000}, /* D */
    {0x7C407840, 0x407C0000}, /* E */
    {0x7C407840, 0x40400000}, /* F */
    {0x3844809C, 0x44380000}, /* G */
    {0x42427E42, 0x42420000}, /* H */
    {0x3E080808, 0x083E0000}, /* I */
    {0x1C040404, 0x44380000}, /* J */
    {0x44485070, 0x48440000}, /* K */
    {0x40404040, 0x407E0000}, /* L */
    {0x41635549, 0x41410000}, /* M */
    {0x4262524A, 0x46420000}, /* N */
    {0x1C222222, 0x221C0000}, /* O */
    {0x78447840, 0x40400000}, /* P */
    {0x1C222222, 0x221C0200}, /* Q */
    {0x78447850, 0x48440000}, /* R */
    {0x1C22100C, 0x221C0000}, /* S */
    {0x7F080808, 0x08080000}, /* T */
    {0x42424242, 0x423C0000}, /* U */
    {0x81424224, 0x24180000}, /* V */
    {0x41414955, 0x63220000}, /* W */
    {0x42241818, 0x24420000}, /* X */
    {0x41221408, 0x08080000}, /* Y */
    {0x7E040810, 0x207E0000}, /* Z */
    {0x38202020, 0x20380000}, /* [ */
    {0x40201008, 0x04020000}, /* \ */
    {0x38080808, 0x08380000}, /* ] */
    {0x10280000, 0x00000000}, /* ^ */
    {0x0, 0x7E0000},          /* _ */
    {0x10080000, 0x00000000}, /* ` */
    {0x00003C02, 0x3E463A00}, /* a */
    {0x40407C42, 0x62625C00}, /* b */
    {0x00001C20, 0x201C0000}, /* c */
    {0x02023E46, 0x463A0000}, /* d */
    {0x00003C42, 0x7E403C00}, /* e */
    {0x18103810, 0x10100000}, /* f */
    {0x0000344C, 0x44340438}, /* g */
    {0x20203824, 0x24240000}, /* h */
    {0x8000808, 0x08080000},  /* i */
    {0x8001808, 0x08080870},  /* j */
    {0x20202428, 0x302C0000}, /* k */
    {0x10101010, 0x10180000}, /* l */
    {0x0000665A, 0x42420000}, /* m */
    {0x00002E32, 0x22220000}, /* n */
    {0x00003C42, 0x423C0000}, /* o */
    {0x00005C62, 0x427C4040}, /* p */
    {0x00003A46, 0x423E0202}, /* q */
    {0x00002C32, 0x20200000}, /* r */
    {0x00001C20, 0x18043800}, /* s */
    {0x0000103C, 0x10101800}, /* t */
    {0x00002222, 0x261A0000}, /* u */
    {0x00004242, 0x24180000}, /* v */
    {0x00008181, 0x5A660000}, /* w */
    {0x00004224, 0x18660000}, /* x */
    {0x4222, 0x14081060},     /* y */
    {0x00007C08, 0x10207C00}, /* z */
    {0x1C103030, 0x101C0000}, /* { */
    {0x8080808, 0x08080800},  /* | */
    {0x38080C0C, 0x08380000}, /* } */
    {0x32, 0x4C000000},       /* ~ */
    {0x7E7E7E7E, 0x7E7E0000}  /* DEL */
};

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



/* function prototypes */
int main(void);
void interrupt_handler(void);
void interval_timer_ISR(void);
void pushbutton_ISR(void);
void ps2_ISR(void);
void WriteHEX(int value);
void WriteHEXadecimal(int value);
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int color);
void drawFilledBox(int x1, int y1, int x2, int y2, short int color);
void DrawRectangle(int x1, int y1, int x2, int y2, short int color);
void swap(int *a, int *b);
void DrawInteger(int x, int y, int num, int color);
void DrawCharacter(int x, int y, char c, int color);
void DrawString(int x, int y, char *string, int color);

/* The assembly language code below handles CPU reset processing */
void the_reset(void) __attribute__((section(".reset")));
void the_reset(void)
/*******************************************************************************
 * Reset code. By giving the code a section attribute with the name ".reset" we
 * allow the linker program to locate this code at the proper reset vector
 * address. This code just calls the main program.
 ******************************************************************************/
{
    asm(".set noat");       /* Instruct the assembler NOT to use reg at (r1) as
                            * a temp register for performing optimizations */
    asm(".set nobreak");    /* Suppresses a warning message that says that
                            * some debuggers corrupt regs bt (r25) and ba
                            * (r30)
                            */
    asm("movia r2, main");  // Call the C language main program
    asm("jmp r2");
}
/* The assembly language code below handles CPU exception processing. This
 * code should not be modified; instead, the C language code in the function
 * interrupt_handler() can be modified as needed for a given application.
 */
void the_exception(void) __attribute__((section(".exceptions")));
void the_exception(void)
/*******************************************************************************
 * Exceptions code. By giving the code a section attribute with the name
 * ".exceptions" we allow the linker program to locate this code at the proper
 * exceptions vector address.
 * This code calls the interrupt handler and later returns from the exception.
 ******************************************************************************/
{
    asm("subi sp, sp, 128");
    asm("stw et, 96(sp)");
    asm("rdctl et, ctl4");
    asm("beq et, r0, SKIP_EA_DEC");  // Interrupt is not external
    asm("subi ea, ea, 4");           /* Must decrement ea by one instruction
                                        * for external interupts, so that the
                                        * interrupted instruction will be run */
    asm("SKIP_EA_DEC:");
    asm("stw r1, 4(sp)");  // Save all registers
    asm("stw r2, 8(sp)");
    asm("stw r3, 12(sp)");
    asm("stw r4, 16(sp)");
    asm("stw r5, 20(sp)");
    asm("stw r6, 24(sp)");
    asm("stw r7, 28(sp)");
    asm("stw r8, 32(sp)");
    asm("stw r9, 36(sp)");
    asm("stw r10, 40(sp)");
    asm("stw r11, 44(sp)");
    asm("stw r12, 48(sp)");
    asm("stw r13, 52(sp)");
    asm("stw r14, 56(sp)");
    asm("stw r15, 60(sp)");
    asm("stw r16, 64(sp)");
    asm("stw r17, 68(sp)");
    asm("stw r18, 72(sp)");
    asm("stw r19, 76(sp)");
    asm("stw r20, 80(sp)");
    asm("stw r21, 84(sp)");
    asm("stw r22, 88(sp)");
    asm("stw r23, 92(sp)");
    asm("stw r25, 100(sp)");  // r25 = bt (skip r24 = et, because it is saved
    // above)
    asm("stw r26, 104(sp)");  // r26 = gp
    // skip r27 because it is sp, and there is no point in saving this
    asm("stw r28, 112(sp)");  // r28 = fp
    asm("stw r29, 116(sp)");  // r29 = ea
    asm("stw r30, 120(sp)");  // r30 = ba
    asm("stw r31, 124(sp)");  // r31 = ra
    asm("addi fp, sp, 128");
    asm("call interrupt_handler");  // Call the C language interrupt handler
    asm("ldw r1, 4(sp)");           // Restore all registers
    asm("ldw r2, 8(sp)");
    asm("ldw r3, 12(sp)");
    asm("ldw r4, 16(sp)");
    asm("ldw r5, 20(sp)");
    asm("ldw r6, 24(sp)");
    asm("ldw r7, 28(sp)");
    asm("ldw r8, 32(sp)");
    asm("ldw r9, 36(sp)");
    asm("ldw r10, 40(sp)");
    asm("ldw r11, 44(sp)");
    asm("ldw r12, 48(sp)");
    asm("ldw r13, 52(sp)");
    asm("ldw r14, 56(sp)");
    asm("ldw r15, 60(sp)");
    asm("ldw r16, 64(sp)");
    asm("ldw r17, 68(sp)");
    asm("ldw r18, 72(sp)");
    asm("ldw r19, 76(sp)");
    asm("ldw r20, 80(sp)");
    asm("ldw r21, 84(sp)");
    asm("ldw r22, 88(sp)");
    asm("ldw r23, 92(sp)");
    asm("ldw r24, 96(sp)");
    asm("ldw r25, 100(sp)");  // r25 = bt
    asm("ldw r26, 104(sp)");  // r26 = gp
    // skip r27 because it is sp, and we did not save this on the stack
    asm("ldw r28, 112(sp)");  // r28 = fp
    asm("ldw r29, 116(sp)");  // r29 = ea
    asm("ldw r30, 120(sp)");  // r30 = ba
    asm("ldw r31, 124(sp)");  // r31 = ra
    asm("addi sp, sp, 128");
    asm("eret");
}
/******************************************************************************
 * Interrupt Service Routine
 * Determines what caused the interrupt and calls the appropriate
 * subroutine.
 *
 * ipending - Control register 4 which has the pending external interrupts
 ******************************************************************************/
void interrupt_handler(void) {
    int ipending;
    NIOS2_READ_IPENDING(ipending);
    if (ipending & 0x1)  // interval timer is interrupt level 0
    {
        interval_timer_ISR();
    }
    if (ipending & 0x80) // PS2 are interrupt level 1
    {
        ps2_ISR();
    }
    if (ipending & 0x2)  // pushbuttons are interrupt level 2
    {
        pushbutton_ISR();
    }
    // else, ignore the interrupt
    return;
}

/*******************************************************************************
 * Interval timer interrupt service routine
 ******************************************************************************/
void interval_timer_ISR() {
    volatile int *interval_timer_ptr = (int *)TIMER_BASE;
    struct ADC_t *const ADCp = (struct ADC_t *)ADC_BASE;

    *(interval_timer_ptr) = 0;  // clear the interrupt

    ADCp->channel0 = 1;
    while (ADCp->channel0 & 0x8000);

    voltage[sample_index] = (ADCp->channel0 & 0xFFF) * 1000 / 4096 * 5;
    current[sample_index] = (ADCp->channel1 & 0xFFF) * 1000 / 4096 * 5 / 2;
    energy = energy + voltage[sample_index] * current[sample_index] * 36 / 10 / 3600 / 100;

    sample_index++;
}

/*******************************************************************************
 * Pushbutton - Interrupt Service Routine
 *
 * This routine checks which KEY has been pressed and updates the global
 * variables as required.
 ******************************************************************************/

void pushbutton_ISR(void) {
    volatile int *KEY_ptr = (int *)KEY_BASE;
    volatile int *slider_switch_ptr = (int *)SW_BASE;
    volatile int *interval_timer_ptr = (int *)TIMER_BASE;  // interal timer base address

    int press;
    int counter;

    press = *(KEY_ptr + 3);  // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press;  // Clear the interrupt

    switch (State) {
        case 0:
            if (press & 0x1) {
                State = 3;
            }
        break;

        case 1:

        break;

        case 2:

        break;

        case 3:
            if (press & 0x1) {  // KEY0
                *(interval_timer_ptr + 1) = 0b1011;
            }
            if (press & 0x2) {  // KEY1
                counter = 5000000;
                *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
                *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
                *(interval_timer_ptr + 1) = 0b0111;
            }
            if (press & 0x4) {  // KEY2
                counter = 500000;
                *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
                *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
                *(interval_timer_ptr + 1) = 0b0111;
            }
            if (press & 0x8) {  // KEY3
                counter = 50000;
                *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
                *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
                *(interval_timer_ptr + 1) = 0b0111;
            }
        break;
    }

    return;
}

volatile int byte_count = 0;
char byte1 = 0;
char byte2 = 0;
char byte3 = 0;

void ps2_ISR(void) {


    volatile int *PS2_ptr = (int *)PS2_BASE;
    int PS2_data, RVALID, RAVAIL;
    PS2_data = *(PS2_ptr);  // read the Data register in the PS/2 port
    RVALID = (PS2_data & 0x8000);	// extract the RVALID field
    if (RVALID != 0) {
        /* always save the last three bytes received */
        byte1 = byte2;
        byte2 = byte3;
        byte3 = PS2_data & 0xFF;
        byte_count++;
    }
    if ((byte2 == 0xAA) && (byte3 == 0x00)) {
        // mouse inserted; initialize sending of data
        *(PS2_ptr) = 0xF4;
        byte_count = 0;
    } else if (byte_count == 3) { 
        WriteHEXadecimal(byte1<<16 | byte2<<8 | byte3);
        byte_count = 0;

        if ((byte2 < 128) && (mouse_x < 310)) {
            mouse_x = mouse_x + byte2;
        } else if ((mouse_x > 10)) {
            mouse_x = mouse_x - (256 - byte2);
        }
        if ((byte3 < 128) && (mouse_y < 230)) {
            mouse_y = mouse_y + byte3;
        } else if ((mouse_y > 10)) {
            mouse_y = mouse_y - (256 - byte3);
        }
    }
    /*
	if (RVALID) {
        // always save the last three bytes received
        byte1 = PS2_data & 0xFF;
        PS2_data = *(PS2_ptr);  // read the Data register in the PS/2 port
        byte2 = PS2_data & 0xFF;
        if( (byte1 == 0xAA) && (byte2 == 0x00) ) {
            // mouse inserted; initialize sending of data
            *(PS2_ptr) = 0xF4;
        } else {
            PS2_data = *(PS2_ptr);  // read the Data register in the PS/2 port
            byte3 = PS2_data & 0xFF;
            if (byte2 < 128) {
                mouse_x = mouse_x + byte2;
            } else {
                mouse_x = mouse_x - (256 - byte2);
            }
            if (byte3 < 128) {
                mouse_y = mouse_y + byte3;
            } else {
                mouse_y = mouse_y - (256 - byte3);
            }
        }
	}
    */
    //WriteHEXadecimal(byte1<<16 | byte2<<8 | byte3);
}

void WriteHEXadecimal(int value) {
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

    int *HexAdd1 = (int *)HEX_BASE1;
    int *HexAdd2 = (int *)HEX_BASE2;

    // Clear the HEX displays
    *HexAdd1 = 0;
    *HexAdd2 = 0;

    int value0 = kHexCodes[value & 0xF];
    int value1 = kHexCodes[(value >> 4) & 0xF];
    int value2 = kHexCodes[(value >> 8) & 0xF];
    int value3 = kHexCodes[(value >> 12) & 0xF];
    int value4 = kHexCodes[(value >> 16) & 0xF];
    int value5 = kHexCodes[(value >> 20) & 0xF];

    *HexAdd1 = value0 | (value1 << 8) | (value2 << 16) | (value3 << 24);
    *HexAdd2 = value4 | (value5 << 8);

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

    int *HexAdd1 = (int *)HEX_BASE1;
    int *HexAdd2 = (int *)HEX_BASE2;

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

void plot_pixel(int x, int y, short int line_color) {
    volatile short int *one_pixel_address;
    one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
    *one_pixel_address = line_color;
}

void wait_for_vsync() {
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    int status;
    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x1) != 0) {
        status = *(pixel_ctrl_ptr + 3);
    }
}

void clear_screen() {
    int y, x;
    for (x = 0; x < 320; x++) {
        for (y = 0; y < 240; y++) {
        plot_pixel(x, y, 0x94B2);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int color) {
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);

    int y = y0;
    int y_step;

    if (y0 < y1) {
        y_step = 1;
    } else {
        y_step = -1;
    }

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
        plot_pixel(y, x, color);
        } else {
        plot_pixel(x, y, color);
        }
        error = error + deltay;
        if (error > 0) {
        y = y + y_step;
        error = error - deltax;
        }
    }
}

void DrawFilledBox(int x1, int y1, int x2, int y2, short int color) {
    for (int i = x1; i <= x2; i++) {
        for (int j = y1; j <= y2; j++) {
        plot_pixel(i, j, color);
        }
    }
}

void DrawRectangle(int x1, int y1, int x2, int y2, short int color) {
    for (int i = x1; i <= x2; i++) {
        plot_pixel(i, y1, color);
        plot_pixel(i, y2, color);
    }
    for (int i = y1; i <= y2; i++) {
        plot_pixel(x1, i, color);
        plot_pixel(x2, i, color);
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void DrawCharacter(int x, int y, char c, int color) {
    int i, j;
    unsigned int mask;
    int charIndex = c;

    mask = 0x80000000;  // Reset mask for each new line
    // Loop for the top half of the character (8x4)
    for (i = 0; i < 8; i++) {
        if (i == 4) {
        mask = 0x80000000;  // Reset mask for the bottom half of the character
        }
        for (j = 0; j < 8; j++) {
        // Use font[charIndex][0] for the top 32 bits
        if ((i < 4 && (font[charIndex][0] & mask)) ||
            (i >= 4 && (font[charIndex][1] & mask))) {
            plot_pixel(x + j, y + i, color);
        }
        mask >>= 1;  // Shift mask right for next bit
        }
    }
}

void DrawString(int x, int y, char *string, int color) {
    int i = 0;
    while (string[i]) {
        DrawCharacter(x + i * 8, y, string[i], color);
        i++;
    }
}

void DrawInteger(int x, int y, int num, int color) {
    char str[10];
    sprintf(str, "%d", num);
    DrawString(x, y, str, color);
}

void DrawMouse(void) {
    // mouse cursor in the shape of a finger
    static const short int mouse_cursor[14][13] = {
        {1, 1, 1, 1, 1, 0x0, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 0x0, 0xFFFF, 0x0, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 0x0, 0xFFFF, 0x0, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 0x0, 0xFFFF, 0x0, 1, 1, 1, 1, 1},
        {1, 0x0, 1, 1, 0x0, 0xFFFF, 0x0, 0x0, 1, 1, 1, 1, 1},
        {0x0, 0xFFFF, 0x0, 1, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 0x0, 1, 1, 1},
        {0x0, 0xFFFF, 0xFFFF, 0x0, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 0x0, 1},
        {0x0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0},
        {1, 0x0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0},
        {1, 1, 0x0, 0xFFFF, 0xFFFF, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 0xFFFF, 0xFFFF, 0x0},
        {1, 1, 1, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 0xFFFF, 0x0, 1},
        {1, 1, 1, 0x0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0, 1},
        {1, 1, 1, 1, 0x0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0, 1, 1},
        {1, 1, 1, 1, 1, 0x0, 0x0, 0x0, 0x0, 0x0, 1, 1, 1}
    };
    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 13; i++) {
            if (mouse_cursor[i][j] != 1) {
                plot_pixel(mouse_x + j - 5, mouse_y + i - 1, mouse_cursor[i][j]);
            }
        }
    }

}


/*******************************************************************************

 ********************************************************************************/
int main(void) {
    volatile int *interval_timer_ptr = (int *)TIMER_BASE; // interal timer base address
    volatile int *KEY_ptr = (int *)KEY_BASE;  // pushbutton KEY address
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int *PS2_ptr = (int *)PS2_BASE;

    /* set the interval timer period for scrolling the ADC reads */
    int counter = 5000000;  // 1/(50 MHz) x (5000000) = 100 msec, 10Hz
    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
    /* start interval timer, enable its interrupts */
    *(interval_timer_ptr + 1) = 0b0111;  // STOP = 0, START = 1, CONT = 1, ITO = 1

    // enable interrupts for all pushbuttons
    *(KEY_ptr + 2) = 0xF;

    // enable interrupts for PS2
    *(PS2_ptr + 1) = 0x1;

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;  // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *(pixel_ctrl_ptr);
    clear_screen();  // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
    clear_screen();  // pixel_buffer_start points to the pixel buffer

    /* set interrupt mask bits for levels 0 (interval timer) and level 1
    * (pushbuttons) */
    NIOS2_WRITE_IENABLE(0x83);
    NIOS2_WRITE_STATUS(1);  // enable Nios II interrupts


    WriteHEXadecimal(0xABCDEF);
    while (1) {
        switch (State) {
            case 0:

            break;

            case 1:
                plot_pixel(mouse_x, mouse_y, 0xF800);
                wait_for_vsync();  // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
                //WriteHEX(mouse_x);
                clear_screen();

            break;

            case 2:

            break;

            case 3:
                WriteHEX(ADC_value);
                clear_screen();
                DrawFilledBox(40, 20, 280, 200, 0xFFFF);

                for (int i = 0; i < 240; i++) {
                    plot_pixel(40 + i, 200 - voltage[i * x_axis_scaling] / voltage_scaling, 0xFF);
                    plot_pixel(40 + i, 200 - current[i * x_axis_scaling] / current_scaling, 0xF800);
                }

                
                DrawRectangle(40, 20, 280, 200, 0x0);

                for (int i = 0; i < 6; i++) {
                    draw_line(40 + 48 * i, 200, 40 + 48 * i, 204, 0x0);
                }

                /*
                for (int i = 0; i < 16; i++) {
                    DrawCharacter(40 + 10 * i, 20, i, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 10, i + 16, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 20, i + 32, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 30, i + 48, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 40, i + 64, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 50, i + 80, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 60, i + 96, 0x0);
                    DrawCharacter(40 + 10 * i, 20 + 70, i + 112, 0x0);
                }
                */

                DrawString(40, 10, "Graphing Monitor", 0x0);
                DrawString(120, 210, "Time (sec)", 0x0);
                DrawString(10, 220, "Voltage (mV):", 0x0);
                DrawInteger(10+14*8, 220, voltage[sample_index-1], 0x0);
                DrawString(10, 230, "Current (mA):", 0x0);
                DrawInteger(10+14*8, 230, current[sample_index-1], 0x0);
                DrawString(170, 220, "Power (mW):", 0x0);
                DrawInteger(170+12*8, 220, voltage[sample_index-1] * current[sample_index-1]/1000, 0x0);
                DrawString(170, 230, "Energy (J):", 0x0);
                DrawInteger(170+12*8, 230, energy, 0x0);

                plot_pixel(mouse_x, mouse_y, 0xF800);
                //DrawMouse();
                wait_for_vsync();  // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer

                if (sample_index >= x_axis_scaling * 240) {
                    x_axis_scaling++;
                }
                break;
        }
    }
}


