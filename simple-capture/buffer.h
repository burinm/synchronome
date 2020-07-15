#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "setup.h"

#include <linux/videodev2.h>

//Internal camera buffers
#define NUM_BUF   10

typedef struct {
    void* start;
    size_t size;
} buffer_t;

extern buffer_t buffers[NUM_BUF]; //Holds pointers to mmaped buffers
extern buffer_t wo_buffer; //Temporary buffer for file output (write out)


int request_buffers(video_t *v);
void deallocate_buffers(video_t *v);

int mmap_buffers(video_t *v);

int enqueue_buf(struct v4l2_buffer* b, int camera_fd);
int dequeue_buf(struct v4l2_buffer* b, int camera_fd);

/* regular buffer management */
#ifdef PPM_CAPTURE
    #define BYTES_PER_PIXEL 3
#endif

#ifdef PGM_CAPTURE
    #define BYTES_PER_PIXEL 1
#endif

#define FRAME_SIZE  (X_RES * Y_RES * BYTES_PER_PIXEL)
#if FRAME_SIZE == 0
    #error "Frame size is zero!"
#endif

int allocate_frame_buffer(buffer_t *b);
//TODO - deallocate frame buffer

#endif
