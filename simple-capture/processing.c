#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "processing.h"
#include "resources.h"
#include "writeout.h"
#include "setup.h"
#include "buffer.h"
#include "queue.h"
#include "motion.h"
#include "memlog.h"

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_processing;

memlog_t* PROCESSING_LOG;

int init_processing();

extern buffer_t scan_buffer[SCAN_BUF_SIZE];
int scan_buffer_index = 0;

void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    PROCESSING_LOG = memlog_init();

    if (init_processing() == -1) {
        error_unbarrier_exit(-1);
    }

    int frame_test_mod = 0;
    int wo_buffer_index = 0;

    int s_ret = -1;
    //struct v4l2_buffer b;

    int last_buffer_index = -1;
    int current_index = 0;
    int changed_pixels = 0;
    int did_frame_tick = 0;


    pthread_barrier_wait(&bar_thread_inits); //GO!!

    while(running) {

        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_DONE);
        s_ret = sem_wait(&sem_processing);

        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_RUN);

        if (s_ret == -1) {
            perror("sem_wait sem_processing failed");
            error_exit(-2);
        }

        last_buffer_index = scan_buffer_index;
        current_index = last_buffer_index +1;
        if (current_index == SCAN_BUF_SIZE) {
            current_index = 0;
        }

//did_frame_tick = 0;
//int breakout = 0;
while(did_frame_tick == 0) { //hack loop until circular buffer is coded
//breakout++; if (breakout > 5) { break;}

#if 0
        if (dequeue_V42L_frame(frame_receive_Q, &b) == -1) {
            printf("*Frame Processing: dequeue error\n");
            error_exit(-1);
        }
#endif

        printf("Processing: [index %d start=%p size=%d] (in)\n",
                        last_buffer_index,
                        scan_buffer[last_buffer_index].start,
                        scan_buffer[last_buffer_index].size);

        changed_pixels = frame_changes(&scan_buffer[last_buffer_index], &scan_buffer[current_index]);
        printf("frame diff = %d ", changed_pixels);
        did_frame_tick = is_motion(changed_pixels);

        printf("%s\n", did_frame_tick ? "yes" : "no");

        last_buffer_index  = current_index;

        current_index++;
        if (current_index == SCAN_BUF_SIZE) {
            current_index = 0;
        }

#if 0
        assert(buffers[b.index].size == wo_buffers[wo_buffer_index].size);

        //Copy frame to writeout buffer
        if (did_frame_tick) {
            memcpy((unsigned char*)wo_buffers[wo_buffer_index].start,
                    (unsigned char*)buffers[b.index].start,
                    buffers[b.index].size);
        }
#endif

#if 0
        //Requeue internal buffer - TODO - do I need to clear it?
        if (enqueue_buf(&b, video.camera_fd) == -1) {
            error_exit(-1);
        }
        printf("Processing: [index %d] (VIDIOC_QBUF)\n", b.index);
#endif


        //Writeout
        if (did_frame_tick) {

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
} //end hack for now
}
return 0;
}

int init_processing() {

    for (int i=0; i<SCAN_BUF_SIZE; i++) {
        if (allocate_frame_buffer(&scan_buffer[i]) == -1) {
            return -1;
        }
    }
return 0;
}

void deallocate_processing() {
    for (int i=0; i<SCAN_BUF_SIZE; i++) {
        deallocate_buffer(&scan_buffer[i]);
    }
}
