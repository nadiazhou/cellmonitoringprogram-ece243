#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* the global variables are written by interrupt service routines; we have to
 * declare
 * these as volatile to avoid the compiler caching their values in registers */
volatile int ADC_value = 0;     // ADC value
volatile int voltage[100000] = {0};
volatile int voltage_index = 0;
volatile int voltage_scaling = 23; // scale of voltage y-axis (mV/pixel)
volatile int x_axis_scaling = 1; // scale of x-axis in terms of ADC samples (dynamic)
volatile int State = 0;


volatile int TIMER_BASE = 0xFF202000;
volatile int KEY_BASE = 0xFF200050;
volatile int SW_BASE = 0xFF200040;
volatile int LED_BASE = 0xFF200000;
volatile int ADC_BASE = 0xFF204000;
volatile int HEX_BASE1 = 0xFF200020;
volatile int HEX_BASE2 = 0xFF200030;



volatile int pixel_buffer_start;  // global variable
short int Buffer1[240][512];      // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];



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

/* function prototypes */
int main(void);
void interrupt_handler(void);
void interval_timer_ISR(void);
void pushbutton_ISR(void);
void WriteHEX(int value);
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int color);
void drawBox(int row, int col, short int color);
void swap(int *a, int *b);

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
    if (ipending & 0x2)  // pushbuttons are interrupt level 1
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

    *(interval_timer_ptr) = 0;                 // clear the interrupt

     ADCp->channel0 = 1;
    while (ADCp->channel0 & 0x8000);
    ADC_value = (ADCp->channel0 & 0xFFF);
    voltage[voltage_index] = ADC_value;
    voltage_index++;

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
    volatile int *interval_timer_ptr = (int *)TIMER_BASE; // interal timer base address

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
                if (press & 0x1) {    // KEY0
                    *(interval_timer_ptr + 1) = 0b1011;
                }
                if (press & 0x2) {   // KEY1
                    counter = 5000000;
                    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
                    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
                    *(interval_timer_ptr + 1) = 0b0111;
                }
                if (press & 0x4) {   // KEY2
                    counter = 500000;
                    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
                    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
                    *(interval_timer_ptr + 1) = 0b0111;
                }
                if (press & 0x8) {   // KEY3
                    counter = 50000;
                    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
                    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
                    *(interval_timer_ptr + 1) = 0b0111;
                }
            break;
        }


    return;
}





/*******************************************************************************

 ********************************************************************************/
int main(void) {

    volatile int *interval_timer_ptr = (int *)TIMER_BASE; // interal timer base address
    volatile int *KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

    /* set the interval timer period for scrolling the ADC reads */
    int counter = 5000000;  // 1/(50 MHz) x (5000000) = 100 msec, 10Hz
    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;
    /* start interval timer, enable its interrupts */
    *(interval_timer_ptr + 1) = 0b1011;  // STOP = 0, START = 1, CONT = 1, ITO = 1

    // enable interrupts for all pushbuttons
    *(KEY_ptr + 2) = 0xF;             
    

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


    /* set interrupt mask bits for levels 0 (interval timer) and level 1 (pushbuttons) */
    NIOS2_WRITE_IENABLE(0x3);
    NIOS2_WRITE_STATUS(1);  // enable Nios II interrupts


    
    
    
    while (1) {

        switch (State) {
            case 0:




            break;

            case 1:





            break;

            case 2:




            break;

            case 3:
                WriteHEX(ADC_value);
                clear_screen();

                for (int i = 0; i < 240; i++) {
                    plot_pixel(40+i, 199-voltage[i*x_axis_scaling]/voltage_scaling, 0xFF);
                }

                draw_line(40, 199, 279, 199, 0x0);
                draw_line(40, 199, 40, 20, 0x0);
                draw_line(279, 199, 279, 20, 0x0);
                draw_line(40, 20, 279, 20, 0x0);

                for (int i = 0; i < 6; i++) {
                    draw_line(40+48*i, 199, 40+48*i, 204, 0x0);
                }
                


                wait_for_vsync(); // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

                if (voltage_index >= x_axis_scaling*240) {
                    x_axis_scaling++;
                }
            break;
        }


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
        plot_pixel(x, y, 0xFFFF);
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

void drawBox(int row, int col, short int color) {
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
        plot_pixel(row + i, col + j, color);
        }
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
