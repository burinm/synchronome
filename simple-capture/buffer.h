#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
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


#endif

int allocate_buffer(buffer_t *b, int blocks);
int allocate_frame_buffer(buffer_t *b);
void deallocate_buffer(buffer_t *b);

