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



#endif

int allocate_buffer(buffer_t *b, int blocks);
int allocate_frame_buffer(buffer_t *b);
void deallocate_buffer(buffer_t *b);

