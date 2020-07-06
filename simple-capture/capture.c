#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev

#include "setup.h"
#include "buffer.h"

#define USE_CTRL_C

int camera_fd = -1;
buffer_t buffer = {0};

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
rb.count = 2;  //simple ping pong strategy
rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
rb.memory = V4L2_MEMORY_MMAP; //TODO try for DMA

if (ioctl(camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
    perror("Couldn't allocate buffers");
    goto error;
}

ssize_t bytes_read = -1;
while(running) {

    /* Not supported by my camera
    bytes_read = read(camera_fd, buffer.start, buffer.size);
    */
    

    if (bytes_read == -1) {
        perror(NULL);
        running = 0;
        break;
    }

    if (errno == EAGAIN) {
        printf(".");
    } else {
        printf("(%u)", bytes_read);
        perror("!");
    }

}

error2:
    //Try to free buffers. (count = 0) Implicit VIDIOC_STREAMOFF
    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    if (ioctl(camera_fd, VIDIOC_REQBUFS, &rb) == -1) {
        perror("Couldn't free buffers");
    }
error:
    close_camera(camera_fd);

    if (buffer.start) {
        free(buffer.start);
    }
}


#ifdef USE_CTRL_C
void ctrl_c(int addr) {
    printf("ctrl-c\n");
    running = 0;
}
#endif
