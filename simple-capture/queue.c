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
#include "queue.h"

extern int running;
mqd_t frame_receive_Q;


int init_queues() {

    struct mq_attr mq_attr = MQ_DEFAULTS;
    mq_attr.mq_msgsize = MQ_PAYLOAD_SIZE;

    //NON blocking for now, that way we can detect if sendQ is full
    frame_receive_Q = mq_open(FRAME_RECEIVE_Q, O_CREAT | O_RDWR | O_NONBLOCK , S_IRUSR | S_IWUSR, &mq_attr);

    if (frame_receive_Q == (mqd_t)-1) {
        perror("Couldn't create/open global message queue\n");
        return -1; 
    }

    //flush queue
    printf("flushing queue\n");
    unsigned int prio;
    char b[MQ_PAYLOAD_SIZE];
    struct timespec _t;
    while(1) {
            clock_gettime(CLOCK_REALTIME, &_t);
            _t.tv_sec += 2;
            int s =  mq_timedreceive(frame_receive_Q, b, MQ_PAYLOAD_SIZE, &prio, &_t);
            if (s == 0 || errno == ETIMEDOUT) {
                break;
            }
            if (s == -1) {
                perror(NULL);
            }
            printf(".");
            fflush(stdout);
    }
    printf("\n");
return 0;
}

void destroy_queues() {
    mq_unlink(FRAME_RECEIVE_Q);
}

int enqueue_P(mqd_t Q, buffer_t *p) {
    char b[MQ_PAYLOAD_SIZE];

    if (p) {
        memcpy(b, &p, MQ_PAYLOAD_SIZE); 
        printf("sending[]: priority = %d, length = %d buff_ptr %p\n",
                HI_PRI, MQ_PAYLOAD_SIZE, p);
        if (mq_send(Q, b, MQ_PAYLOAD_SIZE, HI_PRI) == 0) {
            return 0;
        }
        perror("Couldn't enqueue message!\n");
    } else {
        printf("ignored NUll ptr\n");
    }
return -1;
}

buffer_t* dequeue_P(mqd_t Q) {
    unsigned int prio = 0;
    int bytes_received = 0;
    char b[MQ_PAYLOAD_SIZE];    

    do{
        bytes_received = mq_receive(Q, b, MQ_PAYLOAD_SIZE, &prio);
        if (bytes_received == -1 && errno != EAGAIN) {
            perror("Couldn't get message!\n");
            running = 0;
            break;
        }
    } while (bytes_received < 1);

    printf("receive[]: priority = %d, length = %d buff_ptr %p\n",
            prio, bytes_received, b);
    buffer_t* p;
    memcpy(&p, b, MQ_PAYLOAD_SIZE);
    return p;
}



