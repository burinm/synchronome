#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sharpen.h"
#include "transformation.h" //BYTES_YUYV_PIXELS

buffer_t sharpen_buffer;
float SHARPEN_FLT[SHARPEN_SIZE +1] = SHARPEN_TYPE;

void print_sharpen_filter() {
    console("<sharpen %dx%d>\n\n", SHARPEN_ROWS, SHARPEN_COLS);
    
    for (int row=0; row < SHARPEN_ROWS; row++) {
        console("[ ");
        for (int col=0; col < SHARPEN_COLS; col++) {
            console("% 01.1f ", SHARPEN_FLT[row * SHARPEN_COLS + col]);
        }
        console("]\n");
    }

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

    //pixel position (one loop for greyscale, 3 for rgb)
    for (int pp = 0; pp < BYTES_PER_PIXEL; pp++) {

        for (int i=0; i < Y_RES - SHARPEN_ROWS; i++) {

            for (int j=pp; j < (X_RES - SHARPEN_COLS) * BYTES_PER_PIXEL; j+=BYTES_PER_PIXEL) {
                filter_pos = 0;
                row_sum = 0;

                //Sum up the box of values * filter matrix
                for (int k=0; k < SHARPEN_ROWS; k++) {
                    for (int l=pp; l < SHARPEN_COLS * BYTES_PER_PIXEL; l+=BYTES_PER_PIXEL) {
                        int index = (k * X_RES + l) + (i * X_RES) + j;
    assert(index < src->size);
    assert(filter_pos < SHARPEN_SIZE);
                        row_sum += SHARPEN_FLT[filter_pos++] * (float)in[index];
                    }
                }

                //The clamps!! https://youtu.be/T9BdI-jB8xc
                if (row_sum < 0.0) {
                    row_sum = 0;
                }
                if (row_sum > 255.0) {
                    row_sum = 255;
                }

                //Center pixel
                int pos =  ((i + FILTER_RANGE) * X_RES) * BYTES_PER_PIXEL +
                           (j + FILTER_RANGE) * BYTES_PER_PIXEL +
                           pp +
                           offset;
    assert(pos < dst->size);
                out[pos] = (uint8_t)row_sum;
            }
        }
    }

}

/* Take the Y channel only, sharpen and write out as greyscale all in one step*/
void y_channel_sharpen(buffer_t *src, buffer_t* dst, size_t offset) {
    float row_sum;
    int filter_pos;


//TODO - copy outside of filter box...

    unsigned char* in = (unsigned char*)src->start;
    unsigned char* out = (unsigned char*)dst->start;

    //YUYV is  X*2 x Y total pixels of storage
    #define YUYV_SAMPLE         2

    for (int i=0, d_i=0; i < X_RES * (Y_RES - SHARPEN_ROWS) * YUYV_SAMPLE; i+= X_RES * YUYV_SAMPLE, d_i+=X_RES) {

        for (int j=0, d_j=0; j < (X_RES - SHARPEN_COLS) * YUYV_SAMPLE; j+=YUYV_SAMPLE, d_j++) {
            filter_pos = 0;
            row_sum = 0;

            //Sum up the box of values * filter matrix
            for (int k=0; k < SHARPEN_COLS * SHARPEN_ROWS * YUYV_SAMPLE; k+= SHARPEN_ROWS * YUYV_SAMPLE) {
                for (int l=0; l < SHARPEN_COLS * YUYV_SAMPLE; l+=YUYV_SAMPLE) {
                    int index = i + j + k + l;
assert(index < src->size);
assert(filter_pos < SHARPEN_SIZE);
                    if (index >= src->size) {
                        continue;
                    }
                    row_sum += SHARPEN_FLT[filter_pos++] * (float)in[index];
                }
            }

            //clamp values
            if (row_sum < 0.0) {
                row_sum = 0;
            }
            if (row_sum > 255.0) {
                row_sum = 255;
            }

            //Center pixel
            int pos =  d_i + d_j + offset;
assert(pos < dst->size);
            out[pos] = (uint8_t)row_sum;
        }
    }
}
