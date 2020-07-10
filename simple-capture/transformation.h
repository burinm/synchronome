#ifndef __TRANSFORMATION_H__
#define __TRANSFORMATION_H__

#include <stddef.h>
#include "buffer.h"

void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
void yuv2rgb_float(float y, float u, float v, unsigned char *r, unsigned char *g, unsigned char *b);

//Matrix transforms using spare buffer
void yuv444torgb888(buffer_t *src, buffer_t *dst, size_t offset);



#endif
