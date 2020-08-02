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

    q->count = 0;

    struct mq_attr mq_attr_current = MQ_DEFAULTS;
    mq_attr_current.mq_msgsize = q->max_payload_size;
    mq_attr_current.mq_maxmsg = q->num_elems;
    q->q = mq_open(q->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_current);

    if (q->q == (mqd_t)-1) {
        perror("Couldn't create/open frame queue");
        return -1; 
    }

    q->b = malloc(q->max_payload_size);
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

//void*, code is now generic
int enqueue_P(queue_container_t *q, void *p) {
    int ret;

    if (q->count == q->num_elems) {
        printf("enqueue_P - (safety) enqueue_P full!\n");
        return -1;
    }

    q->count++;

    if (p) {
        memcpy(q->b, (unsigned char*)p, q->max_payload_size);

#if 0
        printf(" sending_P size %d (#%d)\n",
                sizeof(p),
                q->count);
#endif

        //2 second timeout (for ctrl_c)
        struct timespec _t;
        clock_gettime(CLOCK_REALTIME, &_t);
        _t.tv_sec += 2;


        ret = mq_timedsend(q->q, q->b, q->max_payload_size, HI_PRI, &_t);
        if (ret == 0) {
            return 0;
        }
        if (errno == EAGAIN) {
            return -1;
        }
        perror("enqueue_P - Couldn't enqueue message!");
    } else {
        printf("ignored NUll ptr\n");
    }
return -1;
}

int dequeue_P(queue_container_t *q, void *p) {

    unsigned int prio = 0;
    int bytes_received = 0;

    q->count--;

        //2 second timeout (for ctrl_c)
        //struct timespec _t;
        //clock_gettime(CLOCK_REALTIME, &_t);
        //_t.tv_sec += 2;

        //bytes_received = mq_timedreceive(q->q, q->b, q->max_payload_size, &prio, &_t);
        bytes_received = mq_receive(q->q, q->b, q->max_payload_size, &prio);
        if (bytes_received == -1) {
            if (errno == EAGAIN) {
                return -1;
            }
            perror("dequeue_P - Couldn't get message!");
            return -1;
        }

    memcpy(p, (char *)(q->b), q->max_payload_size);

#if 0
    printf(" receive_P size %d (#%d)\n",
            sizeof(p),
            q->count);
#endif

return 0;
}
