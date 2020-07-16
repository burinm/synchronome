#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "processing.h"
#include "setup.h"
#include "buffer.h"
#include "queue.h"

extern int running;

void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    struct v4l2_buffer b;
    while(running) {
        if (dequeue_V42L_frame(frame_receive_Q, &b) == -1) {
            printf("[Frame Processing: dequeue error\n");
            running = 0;
            pthread_exit((void*)-1);
        }

        printf("[Frame Processing: got frame index %d\n", b.index);
        //Requeue internal buffer - TODO - do I need to clear it?
        if (enqueue_buf(&b, video.camera_fd) == -1) {
            perror("VIDIOC_QBUF");
            running = 0;
            pthread_exit((void*)-1); 
        }
        printf("[Processing: reenqueued frame %d\n", b.index);


    }

return 0;
}
