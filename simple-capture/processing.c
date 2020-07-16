#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "processing.h"
#include "transformation.h"
#include "setup.h"
#include "buffer.h"
#include "queue.h"

extern int running;
extern pthread_barrier_t bar_thread_inits;

int _init_processing();
void _deallocate_processing();

/* This buffer can't fall behind (it will be used to select images),
    so it is the same count as the internal camera buffers.
    Simply copy the internal buffer to raw_buffers, so we
    can return the frame_descriptors to the camera driver
    asap
*/

buffer_t raw_buffers[NUM_BUF];
int raw_index = 0;

void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    if (_init_processing() == -1) {
        error_exit(-1);
    }

    pthread_barrier_wait(&bar_thread_inits); //GO!!

    struct v4l2_buffer b;
    while(running) {
        if (dequeue_V42L_frame(frame_receive_Q, &b) == -1) {
            printf("[Frame Processing: dequeue error\n");
            _deallocate_processing();
            error_exit(-1);
        }

        printf("[Processing: got frame buffer #%d start=%p size=%d\n", b.index, buffers[b.index].start, buffers[b.index].size);

assert(buffers[b.index].size == raw_buffers[raw_index].size);
        //For now, ghetto circular buffer for testing
        memcpy((unsigned char*)raw_buffers[raw_index++].start, (unsigned char*)buffers[b.index].start, buffers[b.index].size);
        if (raw_index == NUM_BUF) {
            raw_index = 0;
        }
        printf("[Processing: frame copied\n");

        //Requeue internal buffer - TODO - do I need to clear it?
        if (enqueue_buf(&b, video.camera_fd) == -1) {
            _deallocate_processing();
            error_exit(-1);
        }
        printf("[Processing: reenqueued frame %d\n", b.index);

        //do_transformations(&buffers[b.index]);
    }

printf("[Processing: normal exit]\n");
return 0;
}

int _init_processing() {
    for (int i=0; i < NUM_BUF; i++) {
        if (allocate_frame_buffer(&raw_buffers[i]) == -1)  {
            _deallocate_processing();
            return -1;
        }
    }
return 0;
}

void _deallocate_processing() {
    for (int i=0; i < NUM_BUF; i++) {
        deallocate_buffer(&raw_buffers[i]);
    }
}
