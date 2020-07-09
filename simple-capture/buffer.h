#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <linux/videodev2.h>

#define NUM_BUF 2

typedef struct {
    void* start;
    size_t size;
} buffer_t;

extern buffer_t buffers[NUM_BUF]; //Holds pointers to mmaped buffers

int request_buffers(struct v4l2_requestbuffers* rb, int camera_fd);
void deallocate_buffers(struct v4l2_requestbuffers* rb, int camera_fd);

#endif
