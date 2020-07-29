/* queue.h - demonstrate a message Q 
    burin (c) 2020
    burin (c) 2019 - parts taken from here:
        https://github.com/burinm/aesd5013/blob/master/src/hw4/ipcs/mq.c
        https://github.com/burinm/aesd5013/blob/master/src/hw4/ipcs/message.c
*/
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <mqueue.h>
#include <linux/videodev2.h>
#include "buffer.h"

#define MAX_QUEUE_SIZE 10
#define MAX_PAYLOAD_SZ 128

#define HIGHEST_Q_PRI  (sysconf(_SC_MQ_PRIO_MAX) - 1)
#define HI_PRI     30
#define NO_TIMEOUT  NULL

#define MQ_DEFAULTS { \
    .mq_flags = 0, \
    .mq_maxmsg =  MAX_QUEUE_SIZE, \
    .mq_msgsize = MAX_PAYLOAD_SZ, \
    .mq_curmsgs = 0}

#define MQ_BUFFER_PAYLOAD_SIZE (sizeof(buffer_t))
#define MQ_FRAME_PAYLOAD_SIZE (sizeof(struct v4l2_buffer))

typedef struct {
    mqd_t q;
    char* name;
    int max_payload_size;
    int num_elems;
    char* b; //receive buffer
    int count; //sanity check counter
} queue_container_t;

int init_queue(queue_container_t *q);
int flush_queue(queue_container_t *q);
void destroy_queue(queue_container_t *q); //never called, we use flush, Makefile rm


int enqueue_P(queue_container_t *q, void *p);
int dequeue_P(queue_container_t *q, void *p);

int enqueue_V42L_frame(mqd_t Q, struct v4l2_buffer *p);
int dequeue_V42L_frame(mqd_t Q, struct v4l2_buffer *p);



#endif
