#include <assert.h>
#include "transformation.h"
#include "setup.h"
#include "dumptools.h"


void do_transformations(buffer_t* b) {

#ifdef PPM_CAPTURE
    #ifdef SHARPEN_ON
        yuv422toG8(b, &sharpen_buffer, 0);
    #else
        yuv422torgb888(b, &wo_buffer, 0);
    #endif
#endif

#ifdef PGM_CAPTURE
    #ifdef SHARPEN_ON
        y_channel_sharpen(b, &wo_buffer, 0);
        //yuv422toG8(&buffers[current_b.index], &sharpen_buffer, 0);
    #else
        yuv422toG8(b, &wo_buffer, 0);
    #endif
#endif

    #ifdef SHARPEN_ON //Uncomment for PPM sharpen
    //sharpen(&sharpen_buffer, &wo_buffer, 0);
    #endif

#ifdef PROFILE_FRAMES
#else
    //Write out buffer to disk
    dump_rgb_raw_buffer(&wo_buffer);
#endif
}


// yuv2rgb - (c) from siewert starter code
// This is probably the most acceptable conversion from camera YUYV to RGB
//
// Wikipedia has a good discussion on the details of various conversions and cites good references:
// http://en.wikipedia.org/wiki/YUV
//
// Also http://www.fourcc.org/yuv.php
//
// What's not clear without knowing more about the camera in question is how often U & V are sampled compared
// to Y.
//
// E.g. YUV444, which is equivalent to RGB, where both require 3 bytes for each pixel
//      YUV422, which we assume here, where there are 2 bytes for each pixel, with two Y samples for one U & V,
//              or as the name implies, 4Y and 2 UV pairs
//      YUV420, where for every 4 Ys, there is a single UV pair, 1.5 bytes for each pixel or 36 bytes for 24 pixels

void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b)
{
   int r1, g1, b1;

   // replaces floating point coefficients
   int c = y-16, d = u - 128, e = v - 128;

   // Conversion that avoids floating point
   r1 = (298 * c           + 409 * e + 128) >> 8;
   g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
   b1 = (298 * c + 516 * d           + 128) >> 8;

   // Computed values may need clipping.
   if (r1 > 255) r1 = 255;
   if (g1 > 255) g1 = 255;
   if (b1 > 255) b1 = 255;

   if (r1 < 0) r1 = 0;
   if (g1 < 0) g1 = 0;
   if (b1 < 0) b1 = 0;

   *r = r1 ;
   *g = g1 ;
   *b = b1 ;
}

//From starter code (c) seiwert
void yuv2rgb_float(float y, float u, float v,
        unsigned char *r, unsigned char *g, unsigned char *b)
{
    float r_temp, g_temp, b_temp;

    // R = 1.164(Y-16) + 1.1596(V-128)
    r_temp = 1.164*(y-16.0) + 1.1596*(v-128.0);
    *r = r_temp > 255.0 ? 255 : (r_temp < 0.0 ? 0 : (unsigned char)r_temp);

    // G = 1.164(Y-16) - 0.813*(V-128) - 0.391*(U-128)
    g_temp = 1.164*(y-16.0) - 0.813*(v-128.0) - 0.391*(u-128.0);
    *g = g_temp > 255.0 ? 255 : (g_temp < 0.0 ? 0 : (unsigned char)g_temp);

    // B = 1.164*(Y-16) + 2.018*(U-128)
    b_temp = 1.164*(y-16.0) + 2.018*(u-128.0);
    *b = b_temp > 255.0 ? 255 : (b_temp < 0.0 ? 0 : (unsigned char)b_temp);
}

/* Matrix transformations  */
void yuv422torgb888(buffer_t *src, buffer_t *dst, size_t offset) {

    int Y0, Y1;
    int Cb, Cr;
    unsigned char R, G, B;
    int count = offset;
    unsigned char * dest = (unsigned char*)dst->start;

    unsigned char * iter = (unsigned char*)src->start;
    for (int i=0; i<src->size; i+=BYTES_YUYV_PIXELS) {
        if (src->size - i >= BYTES_YUYV_PIXELS) {
            Y0 = (int)iter[i];
            Cb = (int)iter[i+1];
            Y1 = (int)iter[i+2];
            Cr = (int)iter[i+3];

assert(count < dst->size);

            yuv2rgb(Y0, Cb, Cr, &R, &G, &B);
            dest[count + 0] = R; dest[count + 1] = G; dest[count + 2] = B;

            yuv2rgb(Y1, Cb, Cr, &R, &G, &B);
            dest[count + 3] = R; dest[count + 4] = G; dest[count + 5] = B;

            count += BYTES_RGB_PIXELS;


        }
    }
}

void yuv422toG8(buffer_t *src, buffer_t *dst, size_t offset) {

    int Y0, Y1;
    unsigned char Grey;
    int count = offset;
    unsigned char * dest = (unsigned char*)dst->start;

    unsigned char * iter = (unsigned char*)src->start;
    for (int i=0; i<src->size; i+=BYTES_YUYV_PIXELS) {
        if (src->size - i >= BYTES_YUYV_PIXELS) {
            Y0 = (int)iter[i];
            Y1 = (int)iter[i+2];

assert(count+1 < dst->size);

            yuv2grey(Y0, &Grey);
            dest[count + 0] = Grey;

            yuv2grey(Y1, &Grey);
            dest[count + 1] = Grey;

            count += BYTES_GREY_PIXELS;


        }
    }
}

inline void yuv2grey(int y, unsigned char *grey)
{
   *grey = y;
}
