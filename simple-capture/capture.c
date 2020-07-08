#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev
#include <sys/mman.h>

#include "setup.h"
#include "buffer.h"
#include "dumptools.h"

#define USE_CTRL_C

int camera_fd = -1;

#define NUM_BUF 2
buffer_t buffers[NUM_BUF]; //Holds pointers to mmaped buffers

#ifdef USE_CTRL_C
/* catch signal */
#include <signal.h>
void ctrl_c(int addr);
#endif

int running = 1;

int main() {

#ifdef USE_CTRL_C
//install ctrl_c signal handler 
struct sigaction action;
action.sa_handler = ctrl_c;
sigaction(SIGINT, &action, NULL);
#endif

if ((camera_fd = open_camera(CAMERA_DEV)) == -1) {
    exit(0);
}

/* This can all be setup/checked with v4l2-ctl also */
if (show_camera_capabilities(camera_fd) == -1) {
    goto error;
}

if (enumerate_camera_image_formats(camera_fd) == -1) {
    goto error;
}

if (show_camera_image_format(camera_fd) == -1) {
    goto error;
}

if (camera_set_yuyv(camera_fd) == -1) {
    goto error;
}

if (show_camera_image_format(camera_fd) == -1) {
    goto error;
}
/* End - This can all be setup/checked with v4l2-ctl also */

#if 0 // Used for read(), which is not supported by my camera
printf("---start---\n");

if ((buffer.size = query_buffer_size(camera_fd)) == -1) {
    printf("Couldn't get buffer size\n");
    goto error;
}

printf("Allocating buffer of %d bytes\n", buffer.size);

if ((buffer.start = calloc(1, buffer.size)) == NULL) {
    printf("Couldn't allocate buffer!\n");
    goto error;
}
#endif

//Request some buffers!
struct v4l2_requestbuffers rb;
memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
rb.count = NUM_BUF;  //simple ping pong strategy
rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
rb.memory = V4L2_MEMORY_MMAP; //TODO try for DMA

if (ioctl(camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
    perror("Couldn't allocate buffers");
    goto error;
}

//Did we get all the buffer we asked for?
if (rb.count < NUM_BUF) {
    printf("Did not get requested amount of buffers\n");
    goto error2;
}

memset(&buffers, 0, sizeof(buffer_t) * NUM_BUF);

//Code taken/modified from here:
// https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/mmap.html
for (int i=0; i < rb.count; i++) {
    struct v4l2_buffer b;

    memset(&b, 0, sizeof(struct v4l2_buffer)); 
    b.index = i;
    b.type = rb.type;
    b.memory = rb.memory;

    if (ioctl(camera_fd, VIDIOC_QUERYBUF, &b) == -1) {
        printf("Couldn't get buffer info, index %d:", i);
        perror(NULL);
        goto error2;
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
    buffers[i].start = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, camera_fd, b.m.offset);
    if (buffers[i].start == MAP_FAILED) {
        printf("Couldn't map buffer!\n");
        goto error3;
    }

    printf("buffer #%d start=0x%p mmap=0x%u\n", i, buffers[i].start,  b.m.offset);


}

//Enqueue all the buffers
for (int i=0; i < rb.count; i++) {
    struct v4l2_buffer b;
    memset(&b, 0, sizeof(struct v4l2_buffer)); 
    b.index = i;
    b.type = rb.type;
    b.memory = rb.memory;

    if (ioctl(camera_fd, VIDIOC_QBUF, &b) == -1) {
        printf("Couldn't enqueue buffer, index %d:", i);
        perror(NULL);
        goto error3;
    }

    //Not necessary, I think this is what the API guarantees?
    if ( (b.flags & V4L2_BUF_FLAG_MAPPED) &&
         (b.flags & V4L2_BUF_FLAG_QUEUED) &&
         ((b.flags & V4L2_BUF_FLAG_DONE) == 0)) {
        continue;
    }

    printf("Could not queue buffer, index %d (flags)\n", i);
    goto error3;

}

//Start streaming
int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
if (ioctl(camera_fd, VIDIOC_STREAMON, &stream_type) == -1) {
    perror("Couldn't start stream");
    goto error3;
}


int ret = -1;
struct v4l2_buffer current_b;

while(running) {

    /* Not supported by my camera
    bytes_read = read(camera_fd, buffer.start, buffer.size);
    */
    memset(&current_b, 0, sizeof(struct v4l2_buffer)); 
    current_b.type = rb.type;
    current_b.memory = rb.memory;
    
    ret = ioctl(camera_fd, VIDIOC_DQBUF, &current_b);
    if (ret == -1) {
        perror("VIDIOC_DQBUF");
        running = 0;
        goto error3; 
    }
printf(".");

    if (ret == EAGAIN) {
        continue;
    }
    
    printf("buf index %d dequeued!\n", current_b.index);
    //dump_buffer_with_timestamp(&buffers[current_b.index]);
    dump_yuv422_to_rgb_raw(&buffers[current_b.index]);

    //Requeue buffer - TODO - do I need to clear it?
    if (ioctl(camera_fd, VIDIOC_QBUF, &current_b) == -1) {
        perror("VIDIOC_QBUF");
        running = 0;
        goto error3;
    }

printf(".");

}

error4:
if (ioctl(camera_fd, VIDIOC_STREAMOFF, &stream_type) == -1) {
    perror("Couldn't stop stream");
}

error3:
//TODO - Does the driver do this with VIDIOC_REQBUFS, count = 0?
    for (int i=0; i < rb.count; i++) {
        if (buffers[i].start) { 
            printf("munmap buffer #%d (%p)\n", i, buffers[i].start);
            munmap(buffers[i].start, buffers[i].size);
        }
    }

error2:
    //Try to free buffers. (count = 0) Implicit VIDIOC_STREAMOFF
    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    if (ioctl(camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
        perror("Couldn't free buffers");
    }
    printf("freed internal buffers\n");

error:
    if (close_camera(camera_fd) == -1) {
        printf("problem closing fd=%d\n", camera_fd);
        perror(NULL);
    } else {
        printf("closed camera device\n");
    }
    

}


#ifdef USE_CTRL_C
void ctrl_c(int addr) {
    printf("ctrl-c\n");
    running = 0;
}
#endif
