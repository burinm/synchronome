#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "writeout.h"
#include "queue.h"
#include "setup.h"
#include "transformation.h"
#include "resources.h"
#include "memlog.h"

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_writeout;

memlog_t* WRITEOUT_LOG;

int _init_writeout();

void* writeout(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    WRITEOUT_LOG = memlog_init();

    if (_init_writeout() == -1) {
        error_unbarrier_exit(-1);
    }

    int s_ret = -1;

    pthread_barrier_wait(&bar_thread_inits); //GO!!

        //Best effort!
        s_ret = sem_wait(&sem_writeout);
        if (s_ret == -1) {
            perror("sem_wait sem_writeout failed");
            error_exit(-2);
        }

    while(running) {

 //       MEMLOG_LOG(WRITEOUT_LOG, MEMLOG_E_S3_DONE);
        MEMLOG_LOG(WRITEOUT_LOG, MEMLOG_E_S3_RUN);

        buffer_t b;
        if (dequeue_P(writeout_Q, &b) == -1) {
            printf("[Writeout: dequeue error]\n");
            error_exit(-1);
        }
            printf("Writeout:   [start=%p size=%d] (i/o->)\n", b.start, b.size);

        do_transformations(&b);

    }

return 0;
}

int _init_writeout() {
    for (int i=0; i < NUM_WO_BUF; i++) {
        if (allocate_frame_buffer(&wo_buffers[i]) == -1)  {
            deallocate_writeout();
            return -1;
        }
    }
return 0;
}

void deallocate_writeout() {
    for (int i=0; i < NUM_WO_BUF; i++) {
        deallocate_buffer(&wo_buffers[i]);
    }
}
