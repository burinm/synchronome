#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h> //close for DMA fd

#include "buffer.h"
#include "setup.h"

//mmap buffers
buffer_t buffers[NUM_BUF];
buffer_t wo_buffer;

//Count of enqueued frames
int enqueue_count = 0;

#if 0 //DMA buffers
int dma_fds[NUM_BUF];
void _close_dma_fds();
#endif

void _free_buffers(video_t *v);
void _munmap_buffers(int num_buffers);


int request_buffers(video_t *v) {
    assert(v->camera_fd != -1);

    struct v4l2_requestbuffers rb;

    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.count = NUM_BUF;  //simple ping pong strategy
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    //Note DMA doesn't seem to be supported
    //rb.memory = V4L2_MEMORY_DMABUF; // Invalid argument

    if (ioctl(v->camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
        perror("Couldn't allocate buffers");
        return -1; 
    }

    //Did we get all the buffer we asked for?
    if (rb.count < NUM_BUF) {
        console("Did not get requested amount of buffers\n");

        _free_buffers(v);
        return -1;
    }

    v->num_buffers = rb.count;
    v->type = rb.type;
    v->memory = rb.memory;
return 0;
}

int mmap_buffers(video_t *v) {

    memset(&buffers, 0, sizeof(buffer_t) * NUM_BUF);

    //Code taken/modified from here:
    // https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/mmap.html
    for (int i=0; i < v->num_buffers; i++) {
        struct v4l2_buffer b;

        memset(&b, 0, sizeof(struct v4l2_buffer));
        b.index = i;
        b.type = v->type;
        b.memory = v->memory;

        if (ioctl(v->camera_fd, VIDIOC_QUERYBUF, &b) == -1) {
            console("Couldn't get buffer info, index %d:", i);
            perror(NULL);
            _free_buffers(v);
            return -1;
        }

        buffers[i].size = b.length;
        /* From Documentation:

            https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/buffer.html#c.v4l2_buffer
            .m.offset

            For the single-planar API and when memory is V4L2_MEMORY_MMAP
            this is the offset of the buffer from the start of the device
            memory. The value is returned by the driver and apart of
            serving as parameter to the mmap() function not useful for
            applications.
         */
        buffers[i].start = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                                v->camera_fd, b.m.offset);

        if (buffers[i].start == MAP_FAILED) {
            console("Couldn't map buffer!\n");
            camera_deallocate_internal_buffers(v);
            return -1;
        }

        console("buffer #%d start=0x%p mmap=0x%u\n", i, buffers[i].start,  b.m.offset);

    }

return 0;
}

#if 0
int export_dma_buffers(video_t *v) {
    for (int i=0; i < v->num_buffers; i++) {
        //Code taken/modified from here:
        // https://www.kernel.org/doc/html/v4.19/media/uapi/v4l/vidioc-expbuf.html#vidioc-expbuf
        struct v4l2_exportbuffer e;
        memset(&e, 0, sizeof(struct v4l2_exportbuffer));
        e.type = v->type;
        e.index = i;

        if (ioctl(v->camera_fd, VIDIOC_EXPBUF, &e) == -1) {
            console("Couldn't get DMA fd, index %d:", i);
            perror(NULL);
            _close_dma_fds(v->num_buffers);
            return -1;
        }

        if (e.fd < 1) {
            console("DMA fd error: %d\n", e.fd);
            return -1;
        }

        dma_fds[i] = e.fd;
    }
return 0;
}
#endif

void camera_deallocate_internal_buffers(video_t *v) {
     _munmap_buffers(v->num_buffers);
     _free_buffers(v);
}

int enqueue_buf(struct v4l2_buffer* b, int camera_fd) {
    int ret = ioctl(camera_fd, VIDIOC_QBUF, b);
    enqueue_count++;
    return ret;
}

int dequeue_buf(struct v4l2_buffer* b, int camera_fd) {
    if (enqueue_count == 0) {
        printf("No available queued frame buffers!\n");
        return -1;
    }
    int ret = ioctl(camera_fd, VIDIOC_DQBUF, b);
    enqueue_count--;
    return ret;
}




void _free_buffers(video_t *v) {
    //Try to free buffers. (count = 0) Implicit VIDIOC_STREAMOFF
    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.type = v->type;
    rb.memory = v->memory;

    if (ioctl(v->camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
        perror("Couldn't free buffers");
    }
    console("freed internal buffers\n");
}

void _munmap_buffers(int num_buffers) {
//TODO - Does the driver do this with VIDIOC_REQBUFS, count = 0?
for (int i=0; i < num_buffers; i++) {
        if (buffers[i].start) {
            console("munmap buffer #%d (%p)\n", i, buffers[i].start);
            if (munmap(buffers[i].start, buffers[i].size) == -1) {
                console("Couldn't munmap buffer #%d (%p) size %u", i, buffers[i].start, buffers[i].size);
                perror(NULL);
            }
        }
    }
}

#if 0
void _close_dma_fds(int num_buffers) {
    for (int i=0; i < num_buffers; i++) {
        if (dma_fds[i]) {
            close(dma_fds[i]);
        }
    }
}
#endif


/* regular buffer management */
int allocate_buffer(buffer_t *b, int blocks) {
    int size = sizeof(unsigned char) * FRAME_SIZE * blocks;
    b->start = malloc(size);
    b->size = size;
    if (b->start) {
        return 0;
    } else {
        return -1;
    }
}
int allocate_frame_buffer(buffer_t *b) {
    return allocate_buffer(b, NATIVE_CAMERA_FORMAT_SIZE);
}

void deallocate_buffer(buffer_t *b) {
    if (b->start) {
       free(b->start);
    }
}
