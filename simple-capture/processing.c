#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "processing.h"
#include "camera_buffer.h"
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

void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    PROCESSING_LOG = memlog_init();

    int wo_buffer_index = 0;

    int s_ret = -1;
    //struct v4l2_buffer b;

    int last_buffer_index = -1;
    int changed_pixels = 0;
    int did_frame_tick = 0;


    //pthread_barrier_wait(&bar_thread_inits); //GO!!

    //Best effort!
    s_ret = sem_wait(&sem_processing);
    if (s_ret == -1) {
        perror("sem_wait sem_processing failed");
        error_exit(-2);
    }

    while(running) {

        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_RUN);

        while(1) {

            int current_index;
            if (dequeue_P(&frame_Q, &current_index) == -1) {
                printf("*Frame Processing: dequeue error\n");
                error_exit(-1);
            }

            printf("Processing: [index %d] (VIDIOC_DEQBUF)\n", current_index);

            if (last_buffer_index != -1) {

                printf("Processing: [index %d start=%p size=%d] (in)\n",
                        last_buffer_index,
                        scan_buffer[last_buffer_index].start,
                        scan_buffer[last_buffer_index].size);

                changed_pixels = frame_changes(&scan_buffer[last_buffer_index], &scan_buffer[current_index]);
                printf("Processing %d vs %d frame diff = %d ", last_buffer_index, current_index, changed_pixels);
                did_frame_tick = is_motion(changed_pixels);

                printf("%s\n", did_frame_tick ? "yes" : "no");

#if 1
                assert(scan_buffer[current_index].size == wo_buffers[wo_buffer_index].size);

                //Copy frame to writeout buffer
                if (did_frame_tick) {
                    memcpy((unsigned char*)wo_buffers[wo_buffer_index].start,
                            (unsigned char*)(scan_buffer[current_index].start),
                            scan_buffer[current_index].size);

                    enqueue_P(&writeout_Q, &wo_buffer_index);

                    //ghetto circular buffer
                    wo_buffer_index++;
                    if (wo_buffer_index == NUM_WO_BUF) {
                        wo_buffer_index = 0;
                    }


#endif
                    //Found frame and sent to write Q
                    if (did_frame_tick) {
                        break;
                    }
                }
            }
            last_buffer_index = current_index;
        } //forever, until change is detected
    }
return 0;
}
