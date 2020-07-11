#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sharpen.h"

buffer_t sharpen_buffer;
float SHARPEN_FLT[SHARPEN_SIZE +1] = SHARPEN_TYPE;

void print_sharpen_filter() {
    printf("<sharpen %dx%d>\n\n", SHARPEN_ROWS, SHARPEN_COLS);
    
    for (int row=0; row < SHARPEN_ROWS; row++) {
        printf("[ ");
        for (int col=0; col < SHARPEN_COLS; col++) {
            printf("% 01.1f ", SHARPEN_FLT[row * SHARPEN_COLS + col]);
        }
        printf("]\n");
    }

}

int init_sharpen_buffer(buffer_t* b) {
    b->start = malloc(sizeof(unsigned char) * SHARPEN_BUF_SIZE);
    b->size = SHARPEN_BUF_SIZE;

    if (b->start) {
        return 0;
    }

return -1;
}

void sharpen(buffer_t *src, buffer_t* dst, size_t offset) {
    float row_sum;
    int filter_pos;

//TODO - copy outside of filter box...

    unsigned char* in = (unsigned char*)src->start;
    unsigned char* out = (unsigned char*)dst->start;
    /* FILTER_RANGE is the distance from the center of the pixel
       we are transforming. So we need to leave a border around
       the transformation
    */

    for (int i=0; i < Y_RES - SHARPEN_ROWS; i++) {

        
        for (int j=0; j < X_RES - SHARPEN_COLS; j++) {
            filter_pos = 0;
            row_sum = 0;

            //Sum up the box of values * filter matrix
            for (int k=0; k < SHARPEN_ROWS; k++) {
                for (int l=0; l < SHARPEN_COLS; l++) {
                    int index = (k * X_RES + l) + (i * X_RES) + j;
assert(index < src->size);
                    row_sum += SHARPEN_FLT[filter_pos++] * (float)in[index];
                }
            }

            //The clamps!!
            if (row_sum < 0.0) {
                row_sum = 0;
            }
            if (row_sum > 255.0) {
                row_sum = 255;
            }

            //Center pixel
            int pos =  (i + FILTER_RANGE) * X_RES +
                       (j + FILTER_RANGE) +
                       offset;
assert(pos < dst->size);
            out[pos] = (uint8_t)row_sum;
        } 
    }
        
        
}
