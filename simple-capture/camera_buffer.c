#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
//#include <stdlib.h>
#include <assert.h>
#include <unistd.h> //close for DMA fd

#include "camera_buffer.h"

//Count of enqueued frames
static int camera_enqueue_count = 0;

#if 0 //DMA buffers
int dma_fds[CAMERA_NUM_BUF];
void _close_dma_fds();
#endif

void _free_camera_buffers(video_t *v);
void _munmap_camera_buffers(int num_buffers);


int camera_request_buffers(video_t *v) {
    assert(v->camera_fd != -1);

    struct v4l2_requestbuffers rb;

    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.count = CAMERA_NUM_BUF;  //simple ping pong strategy
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    //Note DMA doesn't seem to be supported
    //rb.memory = V4L2_MEMORY_DMABUF; // Invalid argument

    if (ioctl(v->camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
        perror("Couldn't allocate buffers");
        return -1; 
    }

    //Did we get all the buffer we asked for?
    if (rb.count < CAMERA_NUM_BUF) {
        console("Did not get requested amount of buffers\n");

        _free_camera_buffers(v);
        return -1;
    }

    v->num_buffers = rb.count;
    v->type = rb.type;
    v->memory = rb.memory;
return 0;
}

int camera_mmap_buffers(video_t *v) {

    //memset(&frame_buffers, 0, sizeof(buffer_t) * CAMERA_NUM_BUF);

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
            _free_camera_buffers(v);
            return -1;
        }

        frame_buffers[i].size = b.length;
        /* From Documentation:

            https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/buffer.html#c.v4l2_buffer
            .m.offset

            For the single-planar API and when memory is V4L2_MEMORY_MMAP
            this is the offset of the buffer from the start of the device
            memory. The value is returned by the driver and apart of
            serving as parameter to the mmap() function not useful for
            applications.
         */
        frame_buffers[i].start = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                                v->camera_fd, b.m.offset);

        if (frame_buffers[i].start == MAP_FAILED) {
            console("Couldn't map buffer!\n");
            camera_deallocate_internal_buffers(v);
            return -1;
        }

        console("buffer #%d start=0x%p mmap=0x%u\n", i, frame_buffers[i].start,  b.m.offset);
    }

return 0;
}


void camera_deallocate_internal_buffers(video_t *v) {
     _munmap_camera_buffers(v->num_buffers);
     _free_camera_buffers(v);
}

int camera_enqueue_buf(struct v4l2_buffer* b, int camera_fd) {
    int ret = ioctl(camera_fd, VIDIOC_QBUF, b);
    camera_enqueue_count++;
    return ret;
}

int camera_dequeue_buf(struct v4l2_buffer* b, int camera_fd) {
    if (camera_enqueue_count == 0) {
        printf("dequeue_buf - (saftey) No available queued frame buffers!\n");
        return -1;
    }
    int ret = ioctl(camera_fd, VIDIOC_DQBUF, b);
    camera_enqueue_count--;
    return ret;
}


//Private functions
void _free_camera_buffers(video_t *v) {
    //Try to free buffers. (count = 0) Implicit VIDIOC_STREAMOFF
    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.type = v->type;
    rb.memory = v->memory;

    if (ioctl(v->camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
        perror("VIDIOC_REQBUFS (count=0) Couldn't free internal buffers");
    } else {
        console("freed internal buffers\n");
    }
}

void _munmap_camera_buffers(int num_buffers) {
//TODO - Does the driver do this with VIDIOC_REQBUFS, count = 0?
for (int i=0; i < num_buffers; i++) {
        if (frame_buffers[i].start) {
            console("munmap buffer #%d (%p)\n", i, frame_buffers[i].start);
            if (munmap(frame_buffers[i].start, frame_buffers[i].size) == -1) {
                console("Couldn't munmap buffer #%d (%p) size %u", i, frame_buffers[i].start, frame_buffers[i].size);
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
