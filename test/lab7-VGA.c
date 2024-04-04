#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int pixels = 30;

volatile int pixel_buffer_start;  // global variable
short int Buffer1[240][512];      // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

void clear_screen();
void wait_for_vsync();
void draw_line(int x0, int y0, int x1, int y1, short int color);
void drawBox(int row, int col, short int color);
void updateRowCol(int *row, int *col, int *direction, int *color);
void swap(int *a, int *b);

int main(void) {
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) =
        (int)&Buffer1;  // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *(pixel_ctrl_ptr);
    clear_screen();  // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
    clear_screen();  // pixel_buffer_start points to the pixel buffer

    int row[pixels], col[pixels], dir[pixels], oldrow[pixels], oldcol[pixels],
        color[pixels];
    for (int i = 0; i < pixels; i++) {
        row[i] = (rand() % 237) + 1;
        col[i] = (rand() % 317) + 1;
        dir[i] = rand() % 4;
        oldrow[i] = 1;
        oldcol[i] = 1;
        color[i] = ((rand() % 32) + 1) * 1024 + 10000;
    }

    while (1) {
        /* Erase any boxes and lines that were drawn in the last iteration */

        for (int i = 0; i < pixels - 1; i++) {
        drawBox(col[i], row[i], color[i]);
        draw_line(col[i], row[i], col[i + 1], row[i + 1], color[i]);
        }
        draw_line(col[0], row[0], col[pixels - 1], row[pixels - 1],
                color[pixels - 1]);

        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)

        wait_for_vsync();  // swap front and back buffers on VGA vertical sync

        // drawBox(row1, col1, 0x0000);

        pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer

        for (int i = 0; i < pixels - 1; i++) {
        drawBox(oldcol[i], oldrow[i], 0x0000);
        draw_line(oldcol[i], oldrow[i], oldcol[i + 1], oldrow[i + 1], 0x0000);
        }
        draw_line(oldcol[0], oldrow[0], oldcol[pixels - 1], oldrow[pixels - 1],
                0x0000);

        for (int i = 0; i < pixels; i++) {
        oldcol[i] = col[i];
        oldrow[i] = row[i];
        updateRowCol(&row[i], &col[i], &dir[i], &color[i]);
        }
    }
}

// code for subroutines (not shown)

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
        plot_pixel(x, y, 0);
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

void updateRowCol(int *row, int *col, int *direction, int *color) {
    static const int directionsToDeltaRow[] = {1, -1, -1, 1};
    static const int directionsToDeltaCol[] = {1, 1, -1, -1};
    if (*row == 1 || *row == 238) {
        if (*row == 1 && *col == 1) {
        *direction = 0;
        } else if (*row == 1 && *col == 318) {
        *direction = 3;
        } else if (*row == 238 && *col == 1) {
        *direction = 1;
        } else if (*row == 238 && *col == 318) {
        *direction = 2;
        } else if (*direction == 0) {
        *direction = 1;
        } else if (*direction == 1) {
        *direction = 0;
        } else if (*direction == 2) {
        *direction = 3;
        } else if (*direction == 3) {
        *direction = 2;
        }
        *color = ((rand() % 32) + 1) * 1024 + 18000;
    } else if (*col == 1 || *col == 318) {
        if (*direction == 0) {
        *direction = 3;
        } else if (*direction == 1) {
        *direction = 2;
        } else if (*direction == 2) {
        *direction = 1;
        } else if (*direction == 3) {
        *direction = 0;
        }
        *color = ((rand() % 32) + 1) * 1024 + 18000;
    }
    *row += directionsToDeltaRow[*direction];
    *col += directionsToDeltaCol[*direction];
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
