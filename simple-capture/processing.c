#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "processing.h"
#include "writeout.h"
#include "setup.h"
#include "buffer.h"
#include "queue.h"
#include "memlog.h"

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_processing;

memlog_t* PROCESSING_LOG;

int _init_processing();

/* This buffer can't fall behind (it will be used to select images),
    so it is the same count as the internal camera buffers.
    Simply copy the internal buffer to raw_buffers, so we
    can return the frame_descriptors to the camera driver
    asap
*/

void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    PROCESSING_LOG = memlog_init();

    if (_init_processing() == -1) {
        error_unbarrier_exit(-1);
    }

    int frame_test_mod = 0;
    int wo_buffer_index = 0;

    int s_ret = -1;
    struct v4l2_buffer b;

    pthread_barrier_wait(&bar_thread_inits); //GO!!

    while(running) {

        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_DONE);
        s_ret = sem_wait(&sem_processing);

        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_RUN);

        if (s_ret == -1) {
            perror("sem_wait sem_processing failed");
            error_exit(-2);
        }

        if (dequeue_V42L_frame(frame_receive_Q, &b) == -1) {
            printf("*Frame Processing: dequeue error\n");
            error_exit(-1);
        }

        printf("Processing: [index %d start=%p size=%d] (in)\n",
                b.index, buffers[b.index].start, buffers[b.index].size);

        assert(buffers[b.index].size == wo_buffers[wo_buffer_index].size);

        //TODO - testing, just write out every 16th frame
        if (frame_test_mod %3 == 0) {
            memcpy((unsigned char*)wo_buffers[wo_buffer_index].start,
                    (unsigned char*)buffers[b.index].start,
                    buffers[b.index].size);
        }

        //Requeue internal buffer - TODO - do I need to clear it?
        if (enqueue_buf(&b, video.camera_fd) == -1) {
            error_exit(-1);
        }
        printf("Processing: [index %d] (VIDIOC_QBUF)\n", b.index);

        //Writeout
        //TODO - testing, just write out every 3th frame
        if (frame_test_mod %3 == 0) {

            printf("Processing: [start=%p size=%d] (out)\n",
                    wo_buffers[wo_buffer_index].start, wo_buffers[wo_buffer_index].size);

            if (enqueue_P(writeout_Q, &wo_buffers[wo_buffer_index]) == -1) {
                error_exit(-1);
            }
        }
        frame_test_mod++;

        //ghetto circular buffer
        wo_buffer_index++;
        if (wo_buffer_index == NUM_WO_BUF) {
            wo_buffer_index = 0;
        }
    }
return 0;
}

int _init_processing() {
return 0;
}

void deallocate_processing() {
}
