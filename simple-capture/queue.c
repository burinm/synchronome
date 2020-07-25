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
mqd_t writeout_Q;


int init_queues() {

    //Frame Q
    struct mq_attr mq_attr_frame = MQ_DEFAULTS;
    mq_attr_frame.mq_msgsize = MQ_FRAME_PAYLOAD_SIZE;
    mq_attr_frame.mq_maxmsg = FRAME_RECEIVE_Q_SIZE;
    frame_receive_Q = mq_open(FRAME_RECEIVE_Q, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_frame);

    if (frame_receive_Q == (mqd_t)-1) {
        perror("Couldn't create/open frame queue");
        return -1; 
    }

#if 0
    //Processing Q
    struct mq_attr mq_attr_processing = MQ_DEFAULTS;
    mq_attr_processing.mq_msgsize = MQ_BUFFER_PAYLOAD_SIZE;
    mq_attr_processing.mq_maxmsg = CAMERA_NUM_BUF;
    processing_Q = mq_open(PROCESSING_Q, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_processing);

    if (processing_Q == (mqd_t)-1) {
        perror("Couldn't create/open processing queue");
        return -1;
    }
#endif

    //Writeout Q
    struct mq_attr mq_attr_writeout = MQ_DEFAULTS;
    mq_attr_writeout.mq_msgsize = MQ_BUFFER_PAYLOAD_SIZE;
    mq_attr_writeout.mq_maxmsg = WRITEOUT_Q_SIZE;
    writeout_Q = mq_open(WRITEOUT_Q, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_attr_writeout);

    if (writeout_Q == (mqd_t)-1) {
        perror("Couldn't create/open writeout queue");
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
            if (s == -1) {
                if (errno == ETIMEDOUT) {
                    break;
                }
                perror(NULL);
                return -1;
            }
            printf(".");
            fflush(stdout);
    }

#if 0
    //flush processing queue
    printf("flushing processing queue\n");
    char d[MQ_BUFFER_PAYLOAD_SIZE];
    while(1) {
            clock_gettime(CLOCK_REALTIME, &_t);
            _t.tv_sec += 1;
            int s =  mq_timedreceive(processing_Q, d, MQ_BUFFER_PAYLOAD_SIZE, &prio, &_t);
            if (s == -1) {
                if (errno == ETIMEDOUT) {
                    break;
                }
                perror(NULL);
                return -1;
            }
            printf(".");
            fflush(stdout);
    }
#endif

    //flush writeout queue
    printf("flushing writeout queue\n");
    char c[MQ_BUFFER_PAYLOAD_SIZE];
    while(1) {
            clock_gettime(CLOCK_REALTIME, &_t);
            _t.tv_sec += 1;
            int s =  mq_timedreceive(writeout_Q, c, MQ_BUFFER_PAYLOAD_SIZE, &prio, &_t);
            if (s == -1) {
                if (errno == ETIMEDOUT) {
                    break;
                }
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
    //mq_unlink(PROCESSING_Q);
    mq_unlink(WRITEOUT_Q);
}

//plain buffers
static int enqueue_P_count = 0;

int enqueue_P(mqd_t Q, buffer_t *p) {
    char b[MQ_BUFFER_PAYLOAD_SIZE];

    if (enqueue_P_count == NUM_WO_BUF) {
        printf("enqueue_P - (safety) enqueue_P full!\n");
        return -1;
    }

    enqueue_P_count++;

    if (p) {
        memcpy(b, (unsigned char*)p, MQ_BUFFER_PAYLOAD_SIZE);

        printf(" sending_P  [start %p size %d] (#%d)\n",
                (unsigned char*)p->start, p->size,
                enqueue_P_count);

        //2 second timeout (for ctrl_c)
        struct timespec _t;
        clock_gettime(CLOCK_REALTIME, &_t);
        _t.tv_sec += 2;

        if (mq_timedsend(Q, b, MQ_BUFFER_PAYLOAD_SIZE, HI_PRI, &_t) == 0) {
            return 0;
        }
        perror("enqueue_P - Couldn't enqueue message!");
    } else {
        printf("ignored NUll ptr\n");
    }
return -1;
}

int dequeue_P(mqd_t Q, buffer_t *p) {
    unsigned int prio = 0;
    int bytes_received = 0;
    char b[MQ_BUFFER_PAYLOAD_SIZE];

    enqueue_P_count--;

#if 0
    //2 second timeout (for ctrl_c)
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    _t.tv_sec += 2;
#endif

    do{
        //bytes_received = mq_timedreceive(Q, b, MQ_BUFFER_PAYLOAD_SIZE, &prio, &_t);
        bytes_received = mq_receive(Q, b, MQ_BUFFER_PAYLOAD_SIZE, &prio);
        if (bytes_received == -1 && errno != EAGAIN) {
            perror("dequeue_P - Couldn't get message!");
            return -1;
        }
    } while (bytes_received < 1);


    memcpy(p, (buffer_t *)b, MQ_BUFFER_PAYLOAD_SIZE);

    printf(" receive_P  [start %p size %d] (#%d)\n",
            p->start, p->size,
            enqueue_P_count);

return 0;
}

//v4l2 frames
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



