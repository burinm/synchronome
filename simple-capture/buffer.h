/* buffer.h - manage buffers for video information - allocation, copies, timestamps
    burin (c) 2020
*/
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    void* start;
    size_t size;
    struct timespec time;
    int index;
    int id;
} buffer_t;

//convenience macro
#define COPY_BUFFER(dst, src)   assert ((dst).size == (src).size); \
                                memcpy( dst.start, \
                                        src.start, \
                                        (dst).size)

#define COPY_BUFFER_TIMESTAMP(dst, src) \
                                memcpy( &(dst).time, \
                                        &(src).time, \
                                        sizeof(struct timespec))

#define BUFFER_GET_TIMESTAMP(b, timestamp) \
                                memcpy( &timestamp, \
                                        &(b).time, \
                                        sizeof(struct timespec))

#define BUFFER_SET_TIMESTAMP(b, timestamp) \
                                memcpy( &(b).time, \
                                        &timestamp, \
                                        sizeof(struct timespec))

// Get a new buffer of size x_resolution x y_resolution * blocks
int allocate_buffer(buffer_t *b, int blocks);
// Get a new buffer of size x_resolution x y_resolution
int allocate_frame_buffer(buffer_t *b);
// Return buffer, safe for NULL
void deallocate_buffer(buffer_t *b);

#endif


