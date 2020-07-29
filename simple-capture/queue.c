/* queue.c - queues for passing image buffers around 
    burin (c) 2020
    burin (c) 2019 - parts taken from here:
        https://github.com/burinm/aesd5013/blob/master/src/hw4/ipcs/mq.c
        https://github.com/burinm/aesd5013/blob/master/src/hw4/ipcs/message.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "queue.h"
#include "resources.h"

extern int running;

int init_queue(queue_container_t *q) {

    //Frame Q
    struct mq_attr mq_attr_current = MQ_DEFAULTS;
    mq_attr_current.mq_msgsize = q->max_payload_size;
    mq_attr_current.mq_maxmsg = q->num_elems;
    q->q = mq_open(q->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_current);

    if (q->q == (mqd_t)-1) {
        perror("Couldn't create/open frame queue");
        return -1; 
    }

    q->b = malloc(q->max_payload_size * sizeof(char));
    if (q->b == NULL) {
        printf("Malloc failed - init_queue\n");
        return -1;
    }

return 0;
}

int flush_queue(queue_container_t *q) {

    unsigned int prio;

    struct mq_attr mq_attr_current;
    struct mq_attr mq_attr_old;

    if (mq_getattr(q->q, &mq_attr_current) == -1 ) {
        perror("flushQ  - mq_getattr");
        return -1;
    }

    mq_attr_current.mq_flags |= O_NONBLOCK;

    if (mq_setattr(q->q, &mq_attr_current, &mq_attr_old) == -1 ) {
        perror("flushQ - mq_setattr");
        return -1;
    }

    printf("flushing %s\n", q->name);

    while(1) {
            int s =  mq_receive(q->q, q->b, MQ_FRAME_PAYLOAD_SIZE, &prio);
            if (s == -1) {
                if (errno == EAGAIN) {
                    break;
                }
                perror(NULL);
                return -1;
            }
            printf(".");
            fflush(stdout);
    }


    if (mq_setattr(q->q, &mq_attr_old, NULL) == -1 ) {
        perror("flushQ - mq_setattr");
        return -1;
    }

return 0;
}

void destroy_queue(queue_container_t *q) {
    mq_unlink(q->name);
}

//buffer_t, but we could make it void*, code is now generic
int enqueue_P(queue_container_t *q, void *p) {

printf("%d %d\n", sizeof(p), q->max_payload_size);
assert(sizeof(p) == q->max_payload_size);

    if (q->count == q->num_elems) {
        printf("enqueue_P - (safety) enqueue_P full!\n");
        return -1;
    }

    q->count++;

    if (p) {
        memcpy(q->b, (unsigned char*)p, q->max_payload_size);

        printf(" sending_P size %d (#%d)\n",
                sizeof(p),
                q->count);

        //2 second timeout (for ctrl_c)
        struct timespec _t;
        clock_gettime(CLOCK_REALTIME, &_t);
        _t.tv_sec += 2;

        if (mq_timedsend(q->q, q->b, q->max_payload_size, HI_PRI, &_t) == 0) {
            return 0;
        }
        perror("enqueue_P - Couldn't enqueue message!");
    } else {
        printf("ignored NUll ptr\n");
    }
return -1;
}

int dequeue_P(queue_container_t *q, void *p) {

assert(sizeof(p) == q->max_payload_size);

    unsigned int prio = 0;
    int bytes_received = 0;

    q->count--;

#if 0
    //2 second timeout (for ctrl_c)
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    _t.tv_sec += 2;
#endif

    do{
        //bytes_received = mq_timedreceive(Q, b, MQ_BUFFER_PAYLOAD_SIZE, &prio, &_t);
        bytes_received = mq_receive(q->q, q->b, MQ_BUFFER_PAYLOAD_SIZE, &prio);
        if (bytes_received == -1 && errno != EAGAIN) {
            perror("dequeue_P - Couldn't get message!");
            return -1;
        }
    } while (bytes_received < 1);


    memcpy(p, (buffer_t *)(q->b), MQ_BUFFER_PAYLOAD_SIZE);

    printf(" receive_P size %d (#%d)\n",
            sizeof(p),
            q->count);

return 0;
}

//v4l2 frames
#if 0 // replaced with generic enqueue/dequeue
int enqueue_V42L_frame(mqd_t Q, struct v4l2_buffer *p) {
    char b[MQ_FRAME_PAYLOAD_SIZE];

    if (p) {
        memcpy(b, (unsigned char*)p, MQ_FRAME_PAYLOAD_SIZE);
        printf("   sending  [index %d type %u memory %u]\n",
                p->index, p->type, p->memory);
        if (mq_send(Q, b, MQ_FRAME_PAYLOAD_SIZE, HI_PRI) == 0) {
            return 0;
        }
        perror("enqueue_V42L_frame - Couldn't enqueue message!");
    } else {
        printf("ignored NUll ptr\n");
    }
return -1;
}

int dequeue_V42L_frame(mqd_t Q, struct v4l2_buffer *p) {
    unsigned int prio = 0;
    int bytes_received = 0;
    char b[MQ_FRAME_PAYLOAD_SIZE];

    //2 second timeout (for ctrl_c)
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    _t.tv_sec += 2;

    do{
        bytes_received = mq_timedreceive(Q, b, MQ_FRAME_PAYLOAD_SIZE, &prio, &_t);
        if (bytes_received == -1 && errno != EAGAIN) {
            perror("dequeue_V42L_frame - Couldn't get message!");
            return -1;
        }
    } while (bytes_received < 1);

    memcpy(p, (struct v4l2_buffer*)b, MQ_FRAME_PAYLOAD_SIZE);

    printf("   receive  [index %d type %u memory %u]\n",
            p->index, p->type, p->memory);
return 0;
}
#endif
