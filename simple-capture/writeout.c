#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "writeout.h"
#include "queue.h"
#include "setup.h"
#include "transformation.h"
#include "memlog.h"

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_writeout;

memlog_t* WRITEOUT_LOG;

int _init_writeout();
void _deallocate_writeout();

/* This buffer can't fall behind (it will be used to select images),
    so it is the same count as the internal camera buffers.
    Simply copy the internal buffer to raw_buffers, so we
    can return the frame_descriptors to the camera driver
    asap
*/

buffer_t wo_buffers[NUM_WO_BUF];

void* writeout(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    WRITEOUT_LOG = memlog_init();

    if (_init_writeout() == -1) {
        error_unbarrier_exit(-1);
    }

    pthread_barrier_wait(&bar_thread_inits); //GO!!

    int s_ret = -1;
    while(running) {

        s_ret = sem_wait(&sem_writeout);

        MEMLOG_LOG(WRITEOUT_LOG, MEMLOG_E_S1_RUN);

        if (s_ret == -1) {
            perror("sem_wait sem_writeout failed");
            _deallocate_writeout();
            error_exit(-2);
        }

        buffer_t b;
        if (dequeue_P(writeout_Q, &b) == -1) {
            printf("[Writeout: dequeue error]\n");
            _deallocate_writeout();
            error_exit(-1);
        }
        printf("[writeout] %p\n", (unsigned char*)b.start);
        do_transformations(&b);

    }

printf("[Writeout: normal exit]\n");
return 0;
}

int _init_writeout() {
    for (int i=0; i < NUM_BUF; i++) {
        if (allocate_buffer(&wo_buffers[i], BYTES_PER_PIXEL) == -1)  {
            _deallocate_writeout();
            return -1;
        }
    }
return 0;
}

void _deallocate_writeout() {
    for (int i=0; i < NUM_BUF; i++) {
        deallocate_buffer(&wo_buffers[i]);
    }
}
