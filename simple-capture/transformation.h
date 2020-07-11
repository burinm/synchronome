#ifndef __TRANSFORMATION_H__
#define __TRANSFORMATION_H__

#include <stddef.h>
#include "buffer.h"

#define BYTES_YUYV_PIXELS   4
#define BYTES_RGB_PIXELS    6
#define BYTES_GREY_PIXELS   2

void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
void yuv2grey(int y, unsigned char *grey);
void yuv2rgb_float(float y, float u, float v, unsigned char *r, unsigned char *g, unsigned char *b);

//Matrix transforms using spare buffer
void yuv422torgb888(buffer_t *src, buffer_t *dst, size_t offset);
void yuv422toG8(buffer_t *src, buffer_t *dst, size_t offset);



#endif
