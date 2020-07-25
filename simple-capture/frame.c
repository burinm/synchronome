#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev
#ifdef CAPTURE_STANDALONE
#else
#include <pthread.h>
#endif
#include <semaphore.h>
#include <assert.h>

#include "frame.h"
#include "setup.h" //Keep this at top
#include "camera_buffer.h"
#include "resources.h"
#include "queue.h"
#include "buffer.h"
#include "transformation.h"
#include "processing.h"
#include "dumptools.h"
#include "memlog.h"

#ifdef PROFILE_FRAMES
    #include <time.h>
    #include <limits.h>
    #include "timetools.h"
#endif

#ifdef SHARPEN_ON
    #include "sharpen.h"
#endif

memlog_t* FRAME_LOG;

extern int printf_on;
extern int running;

#ifdef CAPTURE_STANDALONE
#else
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_framegrab;
#endif

void* frame(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

//Logging on
FRAME_LOG = memlog_init();

if (camera_check_init(&video) == -1) {
    if (close_camera(video.camera_fd) == -1) {
        console("problem closing fd=%d\n", video.camera_fd);
        perror(NULL);
    } else {
        console("closed camera device\n");
    }
    video_error_cleanup(ERROR_LEVEL_0, &video);
    error_unbarrier_exit(0);
}

if (camera_init_internal_buffers(&video) == -1) {
    video_error_cleanup(ERROR_LEVEL_2, &video);
    error_unbarrier_exit(-1);
}

if (allocate_other_buffers() == -1) {
    video_error_cleanup(ERROR_LEVEL_3, &video);
    error_unbarrier_exit(-1);
}

//Start streaming
if (start_streaming(&video) == -1) {
    perror("Couldn't start stream");
    video_error_cleanup(ERROR_LEVEL_3, &video);
    error_unbarrier_exit(-1);
}

if (try_refocus(video.camera_fd) == -1) {
    console("Refocus failed!\n");
    video_error_cleanup(ERROR_LEVEL_3, &video);
    error_unbarrier_exit(-1);
}

#ifdef SHARPEN_ON
    console("\nApplying filter: ");
    print_sharpen_filter();
    console("\n");
#endif

//For frame timestamp and profiling
struct timespec timestamp;

/* for profiling, turn off all console output */
#ifdef PROFILE_FRAMES
    printf("\n[Profiling ON - iters = %d, ", PROFILE_ITERS);
    fflush(stdout);
    printf_on = 0;

    //stats
    long int jitter_max = INT_MIN;
    long int jitter_min = INT_MAX;
    int jitter_frame = 60000000;
    struct timespec timestamp_last;
    struct timespec diff;
    long int average_ms = 0;
    int average_count = 0;
    int jitter_delay_count = 0;

    memset(&timestamp_last, 0, sizeof(struct timespec));
#else
    printf_on = 1;
#endif

int ret = -1;
struct v4l2_buffer current_b;

#ifdef CAPTURE_STANDALONE

#ifdef IMAGE_DIFF_PROFILE
int last_buffer_index = -1;
uint32_t changed_pixels = 0;
#endif

#else
int s_ret;
pthread_barrier_wait(&bar_thread_inits); //GO!!
#endif

while(running) {
    MEMLOG_LOG(FRAME_LOG, MEMLOG_E_S1_DONE);
#ifdef CAPTURE_STANDALONE
#else
    s_ret = sem_wait(&sem_framegrab);
#endif

    MEMLOG_LOG(FRAME_LOG, MEMLOG_E_S1_RUN);


#ifdef CAPTURE_STANDALONE
#else
    if (s_ret == -1) {
        perror("sem_wait sem_framegrab failed");
        if (errno == EINTR) { //Aha, if this is in a different thread, the signal doesn't touch it
            error_exit(-2);
        }
        video_error_cleanup(ERROR_FULL_INIT, &video);
        error_exit(-1);
    }
#endif

    memset(&current_b, 0, sizeof(struct v4l2_buffer));
    current_b.type = video.type;
    current_b.memory = video.memory;

    do { //TODO - safety breakout here
        ret = camera_dequeue_buf(&current_b, video.camera_fd);
        if (ret == -1) {
            perror("VIDIOC_DQBUF");
            error_exit(-1);
        }
    } while(ret == EAGAIN);

    //Got frame, timestamp it
    clock_gettime(CLOCK_MONOTONIC, &timestamp);

    MEMLOG_LOG(FRAME_LOG, MEMLOG_E_WCET_START);

    //console("buf index %d dequeued!\n", current_b.index);
    //dump_buffer_raw(&buffers[current_b.index]);

#ifdef PROFILE_FRAMES
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

    if (jitter_delay_count++ > 5) {
        if (diff.tv_nsec > jitter_max) {
            jitter_max = diff.tv_nsec;
        }

        if (diff.tv_nsec < jitter_min) {
            jitter_min = diff.tv_nsec;
        }
    }

    if (average_count == PROFILE_ITERS) {
        running = 0;
    }
#endif

#ifdef CAPTURE_STANDALONE

    #ifdef IMAGE_DIFF_PROFILE
        if (last_buffer_index != -1) {
            changed_pixels = frame_changes(&frame_buffers[last_buffer_index], &frame_buffers[current_b.index]);
            MEMLOG_LOG24(FRAME_LOG, MEMLOG_E_ADATA_24, changed_pixels);
            //printf("pixels:%d seconds\n", changed_pixels);
            //printf(" changed = %s\n", is_motion(changed_pixels) ? "yes" : "no");

        }
        last_buffer_index = current_b.index;
    #else
        do_transformations(&frame_buffers[current_b.index]);


    #endif


#else
assert(scan_buffer[scan_buffer_index].size == frame_buffers[current_b.index].size);

    COPY_BUFFER(scan_buffer[scan_buffer_index], frame_buffers[current_b.index]);

    memcpy(&scan_buffer[scan_buffer_index].time,
           &timestamp,
           sizeof(struct timespec));

    scan_buffer_index++;
    //ghetto circular buffer
    if (scan_buffer_index == SCAN_BUF_SIZE) {
        scan_buffer_index = 0;
    }


#if 0
    printf("Capture:    [index %d] (VIDIOC_DQBUF)\n", current_b.index);
    if (camera_enqueue_V42L_frame(frame_receive_Q, &current_b) == -1) {
        error_exit(-1);
    }
#endif
#endif

    //Requeue buffer - TODO - do I need to clear it?
    if (camera_enqueue_buf(&current_b, video.camera_fd) == -1) {
        perror("VIDIOC_QBUF");
        error_exit(-1);
    }

}

#ifdef CAPTURE_STANDALONE
#if 1
    //deallocate_processing(); TODO - refactor to one area
    for (int i=0; i<SCAN_BUF_SIZE; i++) {
        deallocate_buffer(&scan_buffer[i]);
    }
#endif
#endif

#ifdef PROFILE_FRAMES
    printf_on = 1;
    console("total frames = %d\n", average_count);
    console("average frame time: %03.3fms\n", (float)average_ms / (float)average_count);

    console("jitter max = % .ld us\n", (jitter_max - jitter_frame) / 1000);
    console("jitter min = % .ld us\n", (jitter_min - jitter_frame) / 1000);
    memlog_dump("frame.log", FRAME_LOG);
#endif

    memlog_dump("frame.log", FRAME_LOG);

return 0;
}

void video_error_cleanup(int state, video_t *v) {
    switch(state) {
        case ERROR_FULL_INIT:
            if (stop_streaming(v) == -1) {
                perror("Couldn't stop stream");
            }
        case ERROR_LEVEL_3:
            deallocate_other_buffers();
        case ERROR_LEVEL_2:
            camera_deallocate_internal_buffers(v);
        case ERROR_LEVEL_1:
            if (close_camera(v->camera_fd) == -1) {
                console("problem closing fd=%d\n", v->camera_fd);
                perror(NULL);
            } else {
                console("closed camera device\n");
            }
        case ERROR_LEVEL_0:
            break;

    };
}

int camera_init_internal_buffers(video_t *v) {
    //Request some buffers!
    if (camera_request_buffers(v) == -1) {
        perror("Couldn't allocate buffers");
        return -1;
    }

    if (camera_mmap_buffers(v) == -1) {
        perror("Couldn't allocate buffers");
        return -1;
    }

    //Enqueue all the buffers
    for (int i=0; i < v->num_buffers; i++) {
        struct v4l2_buffer b;
        memset(&b, 0, sizeof(struct v4l2_buffer));
        b.index = i;
        b.type = v->type;
        b.memory = v->memory;

        if (camera_enqueue_buf(&b, v->camera_fd) == -1) {
            console("Couldn't enqueue buffer, index %d:", i);
            perror(NULL);
            return -1;
        }

        //Not necessary, I think this is what the API guarantees?
        if ( (b.flags & V4L2_BUF_FLAG_MAPPED) &&
             (b.flags & V4L2_BUF_FLAG_QUEUED) &&
             ((b.flags & V4L2_BUF_FLAG_DONE) == 0)) {
            continue;
        }

        console("Could not queue buffer, index %d (flags)\n", i);
        return -1;

    }
return 0;
}

int camera_check_init(video_t *v) {
    /* This can all be setup/checked with v4l2-ctl also */
    if (show_camera_capabilities(v->camera_fd) == -1) {
        goto error;
    }

    if (enumerate_camera_image_formats(v->camera_fd) == -1) {
        goto error;
    }

    if (show_camera_image_format(v->camera_fd) == -1) {
        goto error;
    }

    if (camera_set_yuyv(v, X_RES, Y_RES) == -1) {
        goto error;
    }

    if (v->width != X_RES) {
        console("Requested width %d not set (returned %d)\n", X_RES, v->width);
        goto error;
    }

    if (v->height != Y_RES) {
        console("Requested height %d not set (returned %d)\n", Y_RES, v->height);
        goto error;
    }

    if (show_camera_image_format(v->camera_fd) == -1) {
        goto error;
    }
    /* End - This can all be setup/checked with v4l2-ctl also */

    //All good
    return 0;

error:
return -1;
}

int allocate_other_buffers() {
    //Allocate other buffers
    if (allocate_buffer(&wo_buffer, BYTES_PER_PIXEL) == -1) {
        console("couldn't allocate write out buffer\n");
        return -1;
    }

    #ifdef SHARPEN_ON
    if (allocate_buffer(&sharpen_buffer, BYTES_PER_PIXEL) == -1) {
        console("couldn't allocate write out buffer\n");
        return -1;
    }
    #endif
return 0;
}

void deallocate_other_buffers() {
    deallocate_buffer(&wo_buffer);
    #ifdef SHARPEN_ON
    deallocate_buffer(&sharpen_buffer);
    #endif
}
