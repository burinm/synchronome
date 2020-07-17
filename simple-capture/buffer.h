#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stddef.h>
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
void camera_deallocate_internal_buffers(video_t *v);

int mmap_buffers(video_t *v);

int enqueue_buf(struct v4l2_buffer* b, int camera_fd);
int dequeue_buf(struct v4l2_buffer* b, int camera_fd);

#define NATIVE_CAMERA_FORMAT_SIZE   2

/* regular buffer management */
#ifdef PPM_CAPTURE
    #define BYTES_PER_PIXEL 3
#endif

#ifdef PGM_CAPTURE
    #define BYTES_PER_PIXEL 1
#endif

#define FRAME_SIZE  (X_RES * Y_RES)
#if FRAME_SIZE == 0
    #error "Frame size is zero!"
#endif

int allocate_buffer(buffer_t *b, int blocks);
int allocate_frame_buffer(buffer_t *b);
void deallocate_buffer(buffer_t *b);

#endif
