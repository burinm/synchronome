#ifndef __DUMPTOOLS_H__
#define __DUMPTOOLS_H__

#include "buffer.h"

#define FILE_NAME_SIZE  128

void dump_buffer_with_timestamp(buffer_t *b);
void dump_yuv422_to_rgb_raw(buffer_t *b);

#endif
