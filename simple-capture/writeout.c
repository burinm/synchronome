#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "writeout.h"
#include "resources.h"
#include "camera.h"
#include "queue.h"
#include "setup.h"
#include "transformation.h"
#include "resources.h"
#include "realtime.h"
#include "memlog.h"

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_writeout;

memlog_t* WRITEOUT_LOG;

void* writeout(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    WRITEOUT_LOG = memlog_init();

    int s_ret = -1;

    //pthread_barrier_wait(&bar_thread_inits); //GO!!

    //Best effort!
    s_ret = sem_wait(&sem_writeout);
    if (s_ret == -1) {
        perror("sem_wait sem_writeout failed");
        error_exit(-2);
    }

    if (schedule_best_effort_priority(-19) == -1) {
        perror("writeout prio (-19)");
        error_exit(-2);
    }


    while(running) {

        MEMLOG_LOG(WRITEOUT_LOG, MEMLOG_E_S3_DONE);

        int wo_index;
        if (dequeue_P(&writeout_Q, &wo_index) == -1) {
            printf("[Writeout: dequeue error]\n");
            error_exit(-1);
        }

        MEMLOG_LOG(WRITEOUT_LOG, MEMLOG_E_S3_RUN);

            console("Writeout:   [start=%p size=%d] (i/o->)\n", wo_buffers[wo_index].start, wo_buffers[wo_index].size);

        do_transformations(&wo_buffers[wo_index], &wo_buffer);
        total_frames_written_g++;
        if (total_frames_written_g == WRITEOUT_TOTAL_FRAMES) {
           running = 0;
           return 0;
        }

    }

return 0;
}
