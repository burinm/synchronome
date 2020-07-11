#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev

#include "setup.h"
#include "buffer.h"
#include "transformation.h"
#include "dumptools.h"

#define USE_CTRL_C

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

//Allocate other buffers
if (wo_buffer_init(&wo_buffer) == -1) {
    printf("couldn't allocate write out buffer\n");
    exit(-1);
}

video_t video;
memset(&video, 0, sizeof(video_t));
video.camera_fd = -1;

if (open_camera(CAMERA_DEV, &video) == -1) {
    exit(0);
}


/* This can all be setup/checked with v4l2-ctl also */
if (show_camera_capabilities(video.camera_fd) == -1) {
    goto error;
}

if (enumerate_camera_image_formats(video.camera_fd) == -1) {
    goto error;
}

if (show_camera_image_format(video.camera_fd) == -1) {
    goto error;
}

if (camera_set_yuyv(&video, X_RES, Y_RES) == -1) {
    goto error;
}

if (video.width != X_RES) {
    printf("Requested width %d not set (returned %d)\n", X_RES, video.width);
    goto error;
}

if (video.height != Y_RES) {
    printf("Requested height %d not set (returned %d)\n", Y_RES, video.height);
    goto error;
}

if (show_camera_image_format(video.camera_fd) == -1) {
    goto error;
}
/* End - This can all be setup/checked with v4l2-ctl also */

//Request some buffers!
if (request_buffers(&video) == -1) {
    perror("Couldn't allocate buffers");
    goto error;
}

//Enqueue all the buffers
for (int i=0; i < video.num_buffers; i++) {
    struct v4l2_buffer b;
    memset(&b, 0, sizeof(struct v4l2_buffer)); 
    b.index = i;
    b.type = video.type;
    b.memory = video.memory;

    
    if (enqueue_buf(&b, video.camera_fd) == -1) {
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
if (start_streaming(&video) == -1) {
    perror("Couldn't start stream");
    goto error3;
}

if (try_refocus(video.camera_fd) == -1) {
    printf("Refocus failed!\n");
}


int ret = -1;
struct v4l2_buffer current_b;

int header_length = 0;

while(running) {

    memset(&current_b, 0, sizeof(struct v4l2_buffer)); 
    current_b.type = video.type;
    current_b.memory = video.memory;
    
    ret = dequeue_buf(&current_b, video.camera_fd);
    if (ret == -1) {
        perror("VIDIOC_DQBUF");
        running = 0;
        goto error4; 
    }
printf(".");

    if (ret == EAGAIN) {
        continue;
    }
    
    printf("buf index %d dequeued!\n", current_b.index);
    //dump_buffer_raw(&buffers[current_b.index]);

    //Stamp header
#ifdef PPM_CAPTURE
    header_length = ppm_header_with_timestamp(&wo_buffer);
    //Buffer transformation
    yuv422torgb888(&buffers[current_b.index], &wo_buffer, header_length);
#endif

#ifdef PGM_CAPTURE
    header_length = pgm_header_with_timestamp(&wo_buffer);
    yuv422toG8(&buffers[current_b.index], &wo_buffer, header_length);
#endif

    //Write out buffer to disk
    dump_rgb_raw_buffer(&wo_buffer);

    //Requeue buffer - TODO - do I need to clear it?
    if (enqueue_buf(&current_b, video.camera_fd) == -1) {
        perror("VIDIOC_QBUF");
        running = 0;
        goto error4;
    }

printf(".");

}

error4:
if (stop_streaming(&video) == -1) {
    perror("Couldn't stop stream");
}

error3:
deallocate_buffers(&video);

//error2:
 //   free_buffers();

error:
    if (close_camera(video.camera_fd) == -1) {
        printf("problem closing fd=%d\n", video.camera_fd);
        perror(NULL);
    } else {
        printf("closed camera device\n");
    }

return 0;
}


#ifdef USE_CTRL_C
void ctrl_c(int addr) {
    printf("ctrl-c\n");
    running = 0;
}
#endif
