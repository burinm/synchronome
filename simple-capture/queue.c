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
mqd_t processing_Q;


int init_queues() {

    struct mq_attr mq_attr_frame = MQ_DEFAULTS;
    mq_attr_frame.mq_msgsize = MQ_FRAME_PAYLOAD_SIZE;
    frame_receive_Q = mq_open(FRAME_RECEIVE_Q, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_frame);

    if (frame_receive_Q == (mqd_t)-1) {
        perror("Couldn't create/open frame queue\n");
        return -1; 
    }

    struct mq_attr mq_attr_processing = MQ_DEFAULTS;
    mq_attr_processing.mq_msgsize = MQ_BUFFER_PAYLOAD_SIZE;
    processing_Q = mq_open(PROCESSING_Q, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_processing);

    if (processing_Q == (mqd_t)-1) {
        perror("Couldn't create/open processing queue\n");
        return -1;
    }

    unsigned int prio;
    struct timespec _t;

    //flush frame queue
    printf("flushing frame queue\n");
    char b[MQ_FRAME_PAYLOAD_SIZE];
    while(1) {
            clock_gettime(CLOCK_REALTIME, &_t);
            _t.tv_sec += 1;
            int s =  mq_timedreceive(frame_receive_Q, b, MQ_FRAME_PAYLOAD_SIZE, &prio, &_t);
            if (s == 0 || errno == ETIMEDOUT) {
                break;
            }
            if (s == -1) {
                perror(NULL);
                return -1;
            }
            printf(".");
            fflush(stdout);
    }

    //flush processing queue
    printf("flushing processing queue\n");
    char c[MQ_BUFFER_PAYLOAD_SIZE];
    while(1) {
            clock_gettime(CLOCK_REALTIME, &_t);
            _t.tv_sec += 1;
            int s =  mq_timedreceive(processing_Q, c, MQ_BUFFER_PAYLOAD_SIZE, &prio, &_t);
            if (s == 0 || errno == ETIMEDOUT) {
                break;
            }
            if (s == -1) {
                perror(NULL);
                return -1;
            }
            printf(".");
            fflush(stdout);
    }
    printf("\n");
return 0;
}

void destroy_queues() {
    mq_unlink(FRAME_RECEIVE_Q);
    mq_unlink(PROCESSING_Q);
}

//plain buffers
int enqueue_P(mqd_t Q, buffer_t *p) {
    char b[MQ_BUFFER_PAYLOAD_SIZE];

    if (p) {
        memcpy(b, &p, MQ_BUFFER_PAYLOAD_SIZE);
        printf("sending[]: priority = %d, length = %d buff_ptr %p\n",
                HI_PRI, MQ_BUFFER_PAYLOAD_SIZE, p);
        if (mq_send(Q, b, MQ_BUFFER_PAYLOAD_SIZE, HI_PRI) == 0) {
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
    char b[MQ_BUFFER_PAYLOAD_SIZE];

    do{
        bytes_received = mq_receive(Q, b, MQ_BUFFER_PAYLOAD_SIZE, &prio);
        if (bytes_received == -1 && errno != EAGAIN) {
            perror("Couldn't get message!\n");
            return NULL;
            break;
        }
    } while (bytes_received < 1);

    printf("receive[]: priority = %d, length = %d buff_ptr %p\n",
            prio, bytes_received, b);
    buffer_t* p;
    memcpy(&p, b, MQ_BUFFER_PAYLOAD_SIZE);
    return p;
}

//v4l2 frames
int enqueue_V42L_frame(mqd_t Q, struct v4l2_buffer *p) {
    char b[MQ_FRAME_PAYLOAD_SIZE];

    if (p) {
        memcpy(b, p, MQ_FRAME_PAYLOAD_SIZE);
        printf("sending[index %d type %u memory %u]: priority = %d, length = %d buff_ptr %p\n",
                p->index, p->type, p->memory,
                HI_PRI, MQ_FRAME_PAYLOAD_SIZE, p);
        if (mq_send(Q, b, MQ_FRAME_PAYLOAD_SIZE, HI_PRI) == 0) {
            return 0;
        }
        perror("Couldn't enqueue message!\n");
    } else {
        printf("ignored NUll ptr\n");
    }
return -1;
}

int dequeue_V42L_frame(mqd_t Q, struct v4l2_buffer *p) {
    unsigned int prio = 0;
    int bytes_received = 0;
    char b[MQ_FRAME_PAYLOAD_SIZE];

    do{
        bytes_received = mq_receive(Q, b, MQ_FRAME_PAYLOAD_SIZE, &prio);
        if (bytes_received == -1 && errno != EAGAIN) {
            perror("Couldn't get message!\n");
            return -1;
        }
    } while (bytes_received < 1);

    memcpy(p, (struct v4l2_buffer*)b, MQ_FRAME_PAYLOAD_SIZE);

    printf("receive[index %d type %u memory %u]: priority = %d, length = %d buff_ptr %p (%d bytes)\n",
            p->index, p->type, p->memory,
            HI_PRI, MQ_FRAME_PAYLOAD_SIZE, p, bytes_received);
return 0;
}



