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

#ifdef PROFILE_FRAMES
    #include <time.h>
    #include "timetools.h"
#endif

#ifdef SHARPEN_ON
    #include "sharpen.h"
#endif

#define USE_CTRL_C

#ifdef USE_CTRL_C
/* catch signal */
#include <signal.h>
void ctrl_c(int addr);
#endif

//TODO - make error macro that will still print out in silent mode
int printf_on = 1;
int running = 1;

int main() {

#ifdef USE_CTRL_C
//install ctrl_c signal handler 
struct sigaction action;
action.sa_handler = ctrl_c;
sigaction(SIGINT, &action, NULL);
#endif

//Allocate other buffers
if (allocate_frame_buffer(&wo_buffer) == -1) {
    console("couldn't allocate write out buffer\n");
    exit(-1);
}

#ifdef SHARPEN_ON
if (allocate_frame_buffer(&sharpen_buffer) == -1) {
    console("couldn't allocate write out buffer\n");
    exit(-1);
}
#endif

//TODO - free sharpen_buffer
//TODO - free wo_buffer

//console("wo %d, sh %d\n", wo_buffer.size, sharpen_buffer.size);

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
    console("Requested width %d not set (returned %d)\n", X_RES, video.width);
    goto error;
}

if (video.height != Y_RES) {
    console("Requested height %d not set (returned %d)\n", Y_RES, video.height);
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
        console("Couldn't enqueue buffer, index %d:", i);
        perror(NULL);
        goto error3;
    }

    //Not necessary, I think this is what the API guarantees?
    if ( (b.flags & V4L2_BUF_FLAG_MAPPED) &&
         (b.flags & V4L2_BUF_FLAG_QUEUED) &&
         ((b.flags & V4L2_BUF_FLAG_DONE) == 0)) {
        continue;
    }

    console("Could not queue buffer, index %d (flags)\n", i);
    goto error3;

}

#ifdef SHARPEN_ON
    console("\nApplying filter: ");
    print_sharpen_filter();
    console("\n");
#endif

//Start streaming
if (start_streaming(&video) == -1) {
    perror("Couldn't start stream");
    goto error3;
}

if (try_refocus(video.camera_fd) == -1) {
    console("Refocus failed!\n");
}

/* for profiling, turn off all console output */
#ifdef PROFILE_FRAMES
    printf_on = 0;
#else
    printf_on = 1;
#endif

int ret = -1;
struct v4l2_buffer current_b;

#ifdef PROFILE_FRAMES
struct timespec timestamp;
struct timespec timestamp_last;
struct timespec diff;
float average_ms = 0;
int average_count = 0;

memset(&timestamp_last, 0, sizeof(struct timespec));
#endif

while(running) {

#ifdef PROFILE_FRAMES
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    if (timestamp_last.tv_sec != 0) {
        if (timespec_subtract(&diff, &timestamp, &timestamp_last) == 0) {    
            console("diff: %010lu.%09lu\n", diff.tv_sec, diff.tv_nsec); 
            average_ms += diff.tv_sec * 1000 + diff.tv_nsec / 1000000;
            average_count++;
        } else {
            console_error("diff: negative!!\n");
        }
    }
    timestamp_last = timestamp;

    if (average_count == PROFILE_ITERS) {
        running = 0;
    }
#endif

    memset(&current_b, 0, sizeof(struct v4l2_buffer)); 
    current_b.type = video.type;
    current_b.memory = video.memory;
    
    ret = dequeue_buf(&current_b, video.camera_fd);
    if (ret == -1) {
        perror("VIDIOC_DQBUF");
        running = 0;
        goto error4; 
    }
console(".");

    if (ret == EAGAIN) {
        continue;
    }
    
    console("buf index %d dequeued!\n", current_b.index);
    //dump_buffer_raw(&buffers[current_b.index]);

#ifdef PPM_CAPTURE
    #ifdef SHARPEN_ON
        yuv422toG8(&buffers[current_b.index], &sharpen_buffer, 0);
    #else
        yuv422torgb888(&buffers[current_b.index], &wo_buffer, 0);
    #endif
#endif

#ifdef PGM_CAPTURE

    #ifdef SHARPEN_ON
        yuv422toG8(&buffers[current_b.index], &sharpen_buffer, 0);
    #else
        yuv422toG8(&buffers[current_b.index], &wo_buffer, 0);

    #endif

#endif

    #ifdef SHARPEN_ON
    sharpen(&sharpen_buffer, &wo_buffer, 0);
    #endif

#ifndef PROFILE_FRAMES
    //Write out buffer to disk
    dump_rgb_raw_buffer(&wo_buffer);
#endif


    //Requeue buffer - TODO - do I need to clear it?
    if (enqueue_buf(&current_b, video.camera_fd) == -1) {
        perror("VIDIOC_QBUF");
        running = 0;
        goto error4;
    }

console(".");

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
        console("problem closing fd=%d\n", video.camera_fd);
        perror(NULL);
    } else {
        console("closed camera device\n");
    }
#ifdef PROFILE_FRAMES
    printf_on = 1;
    console("frames = %d\n", average_count);
    console("average frame time: %03.3fms\n", average_ms / average_count);
#endif

return 0;
}


#ifdef USE_CTRL_C
void ctrl_c(int addr) {
    console("ctrl-c\n");
    running = 0;
}
#endif
