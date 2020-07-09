#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "setup.h"

#include <linux/videodev2.h>

#define NUM_BUF 2

typedef struct {
    void* start;
    size_t size;
} buffer_t;

extern buffer_t buffers[NUM_BUF]; //Holds pointers to mmaped buffers

int request_buffers(video_t *v);
void deallocate_buffers(video_t *v);

int enqueue_buf(struct v4l2_buffer* b, int camera_fd);
int dequeue_buf(struct v4l2_buffer* b, int camera_fd);

#endif
