#include <stdlib.h>
#include <string.h>
#include <time.h>
#define PS2_BASE 0xFF200100
#define HEX3_HEX0_BASE 0xFF200020
#define HEX5_HEX4_BASE 0xFF200030
#define SW 0xFF200040
#define MOUSEHEIGHT 4
#define MOUSEWIDTH 4
	
//if (SW & 1b1 == 1), draw
//if (SW & 1b01 == 1), erase
//change colour?-last
	
volatile int pixel_buffer_start; // global variable
short int Buffer1[640][480]; // Adjusted to 320 columns for clarity, assuming no padding is needed for this example
short int Buffer2[640][480];
volatile int *pixel_ctrl_ptr = (int *)0xFF203020; // Control register for the pixel buffer
int buffernum =0;

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();
void HEX_PS2(char, char, char);
void clearpixel(int x, int y);
int ctr=0;
struct PIT_t {
      volatile unsigned int      DR;
      volatile unsigned int      DIR;
      volatile unsigned int      MASK;
      volatile unsigned int      EDGE;
      };
struct PIT_t *const swp = ((struct PIT_t *)0xFF200040);

int mouseClearX [2][MOUSEWIDTH];
int mouseClearY [2][MOUSEHEIGHT];
	

int main(void) {
	for(int i=0; i<MOUSEWIDTH; i++){
		mouseClearX [0][i]=-1;
		mouseClearX [1][i]=-1;
	}
		
	for (int j = 0; j < MOUSEHEIGHT; j++) {
		mouseClearY [0][j]=-1;
		mouseClearY [1][j]=-1;

	}
    volatile int *PS2_ptr = (int *)PS2_BASE;
    int PS2_data, RVALID;
    char byte1 = 0, byte2 = 0, byte3 = 0;
    // PS/2 mouse reset
    *(PS2_ptr) = 0xFF;
    int dx = 100;
	int dy = 100; 
	

    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
	clear_screen();
    
    while (1) {
        PS2_data = *PS2_ptr;
        RVALID = PS2_data & 0x8000;//anding it with bit 15 to see if data avilable to read
		if (RVALID) {
            byte1 = byte2;
            byte2 = byte3;
            byte3 = PS2_data & 0xFF;
            HEX_PS2(byte1, byte2, byte3);
			if(ctr<3){
				ctr++;
					}
				else{
					ctr=1;
				}
			}
		if(ctr==3){
         dx += byte1; 
         dy -= byte2;

			if (dx < 0) dx = 0;
            if (dx > 319 - MOUSEWIDTH) dx = 319 - MOUSEWIDTH;
            if (dy < 0) dy = 0;
            if (dy > 239 - MOUSEHEIGHT) dy = 239 - MOUSEHEIGHT;
		
			if((swp->DR & 0b01) != 1){
			for(int i=0; i< MOUSEWIDTH && mouseClearX[buffernum][i]!=-1; i++){
				for(int j=0; j< MOUSEHEIGHT && mouseClearY[buffernum][j]!=-1; j++){
					clearpixel(
						mouseClearX[buffernum][i],mouseClearY[buffernum][j]);
				}
			}
            }
			
            else if((swp->DR & 0b10) != 1){//erase
			for (int x = 0; x < MOUSEWIDTH; x++) {
				for (int y = 0; y < MOUSEHEIGHT; y++) {
					plot_pixel(dx + x, dy + y, 0xFFFF); // Draw pixel in white
					if (x == 0) {
						mouseClearY[buffernum][y] = dy + y;
					}
				}
				mouseClearX[buffernum][x] = dx + x;
				//going to have to change this to new colour
			}
            }
			

            if ((byte2 == (char)0xAA) && (byte3 == (char)0x00)) {
                *PS2_ptr = 0xF4;
            }
		}
    }
}

//need to check if there is already a colour on screen
//detect that colour and make sure that it dooes
void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;
    one_pixel_address = (short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *one_pixel_address = line_color;
}

void wait_for_vsync()
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020; // Base address
    int status;
    *pixel_ctrl_ptr = 1; // Start the synchronization process
    status = *(pixel_ctrl_ptr + 3); // Read the status register
    while ((status & 0x01) != 0) // Polling loop waiting for S bit to go to 0
    {
        status = *(pixel_ctrl_ptr + 3);
    } // Loop/function exits when status bit goes to 0
}

void clear_screen(){

    //initialize variables to iterate through the pixels
    int x;
	int y;
	
    //go over each pixel in the vga display and set the colour of the pixel to black
    for (x = 0; x < 320; x++) {
		for (y = 0; y < 240; y++) {
			plot_pixel(x, y, 0x0000);	
		}
	}

}
//need leave trail functions
//double buffer array, one loop that clears and one loop that adapts it
	
	
void clearpixel(int x,int y){
	plot_pixel(x,y,0x0000);
}

void HEX_PS2(char b1, char b2, char b3) {
volatile int * HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
volatile int * HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;
/* SEVEN_SEGMENT_DECODE_TABLE gives the on/off settings for all segments in
* a single 7-seg display in the DE1-SoC Computer, for the hex digits 0 - F
*/
unsigned char seven_seg_decode_table[] = {
0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int shift_buffer, nibble;
unsigned char code;
int i;
shift_buffer = (b1 << 16) | (b2 << 8) | b3;
for (i = 0; i < 6; ++i) {
nibble = shift_buffer & 0x0000000F; // character is in rightmost nibble
code = seven_seg_decode_table[nibble];
hex_segs[i] = code;
shift_buffer = shift_buffer >> 4;
}
/* drive the hex displays */
*(HEX3_HEX0_ptr) = *(int *)(hex_segs);
*(HEX5_HEX4_ptr) = *(int *)(hex_segs + 4);
}