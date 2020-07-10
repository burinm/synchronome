#ifndef __DUMPTOOLS_H__
#define __DUMPTOOLS_H__

#include "buffer.h"

extern buffer_t wo_buffer;

#define FILE_NAME_SIZE 32

//https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html (lame!)
#define xstr(s) str(s)
#define str(s) #s

//Boo, this is brittle, c strings are terrible
#define PPM_HEADER_DESC  "P6\n"               //    2
#define PPM_HEADER_RES   xstr(X_RES Y_RES\n)  //    9 xxxx xxxx
//# (1) 9999999999 sec (15) 999999999 nsec\n (15)
#define PPM_TIMESTAMP_SIZE 31                 //   31
#define PPM_HEADER_DEPTH   "255\n"            //    5

// +1 because srncpy includes '\0' as one of the bytes
#define PPM_HEADER_MAX_LEN                        (47 + 1)


#define PPM_SUFFIX      "ppm"
#define BYTES_PER_PIXEL 3 //Changes with output format


#define WRITEOUT_BUF_SIZE (PPM_HEADER_MAX_LEN + X_RES * Y_RES * BYTES_PER_PIXEL)

void dump_buffer_raw(buffer_t *b);
void dump_yuv422_to_rgb_raw(buffer_t *b);

int ppm_header_with_timestamp(buffer_t *b);
void dump_rgb_raw_buffer(buffer_t *b);

int wo_buffer_init(buffer_t *b);

#endif
