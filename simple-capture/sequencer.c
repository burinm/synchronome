#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include "realtime.h"
#include "capture.h"
#include "timetools.h"
#include "queue.h"
#include "processing.h"
#include "writeout.h"
#include "memlog.h"

void ctrl_c(int s);
void sequencer(int v);

pthread_barrier_t bar_thread_inits;

//framegrabbing thread
sem_t sem_framegrab;
pthread_t thread_framegrab;
//extern void* frame(void* v);
extern memlog_t* FRAME_LOG;

//processing thread
sem_t sem_processing;
pthread_t thread_processing;

//writeou thread
sem_t sem_writeout;
pthread_t thread_writeout;

int running = 1;
int printf_on = 1;

int main() {

//Ctrl C
struct sigaction s0;
s0.sa_handler = ctrl_c;
sigaction(SIGINT, &s0, NULL);

//Catch SIGUSR2 and run seqencer
struct sigaction s1;
s1.sa_handler = sequencer;
sigaction(SIGUSR2, &s1, NULL);

//init queues
if (init_queues() == -1) {
    exit(-1);
}

//Setup semaphores
if (sem_init(&sem_framegrab, 0, 0) == -1) {
    perror("Couldn't init semaphore sem_framegrab");
    exit(-1);
}

if (sem_init(&sem_processing, 0, 0) == -1) {
    perror("Couldn't init semaphore sem_processing");
    exit(-1);
}

if (sem_init(&sem_writeout, 0, 0) == -1) {
    perror("Couldn't init semaphore sem_writeout");
    exit(-1);
}

//Main enter realtime
if (set_main_realtime() == -1) {
    exit(-1);
}

//Startup tests
long int clock_get_latency = test_clock_gettime_latency();

//Camera init
video_t video;
memset(&video, 0, sizeof(video_t));
video.camera_fd = -1;

if (open_camera(CAMERA_DEV, &video) == -1) {
    exit(-1);
}

//Start, wait on (1)main, (2)frame, (3)processor, (4)writeout
pthread_barrier_init(&bar_thread_inits, NULL, 4);

printf("Creating frame grabber thread\n");
pthread_attr_t rt_sched_attr;  // For realtime H/M/L threads
schedule_realtime(&rt_sched_attr);

//Frame grabbing thread
schedule_priority(&rt_sched_attr, HIGH_PRI);

if (pthread_create(&thread_framegrab, &rt_sched_attr, frame, (void*)&video) == -1) {
    perror("Couldn't create frame grabber thread");
    exit(-1);
}

//Processing thread
schedule_priority(&rt_sched_attr, MID_PRI);

if (pthread_create(&thread_processing, &rt_sched_attr, processing, (void*)&video) == -1) {
    perror("Couldn't create processing thread");
    exit(-1);
}

//Writeout thread
schedule_priority(&rt_sched_attr, LOW_PRI);

if (pthread_create(&thread_writeout, &rt_sched_attr, writeout, (void*)&video) == -1) {
    perror("Couldn't create writeout thread");
    exit(-1);
}

//Interval timer for sequencer loop
timer_t timer1; // note not defined with a struct


//Install timer
struct sigevent sv;
sv.sigev_notify = SIGEV_SIGNAL;
sv.sigev_signo = SIGUSR2;

//if (timer_create(CLOCK_MONOTONIC, NULL, &timer1) == -1 ) {
if (timer_create(CLOCK_MONOTONIC, &sv, &timer1) == -1 ) {
    perror("Couldn't create timer");
    exit(-1);
}

printf("timer installed\n");

struct itimerspec it;
it.it_interval.tv_sec = 0;
it.it_interval.tv_nsec = 10000000; //10ms
//it.it_interval.tv_nsec = 60000000; //60ms
//it.it_interval.tv_nsec = 200000000; //200ms
it.it_value.tv_sec = 1; //delay 1 second to start
it.it_value.tv_nsec = 0;

pthread_barrier_wait(&bar_thread_inits); //GO!!

if (timer_settime(timer1, 0, &it, NULL) == -1 ) {
    perror("couldn't set timer");
    exit(-1);
}

printf("Ready.\n");
pthread_join(thread_framegrab, NULL);
pthread_join(thread_processing, NULL);
pthread_join(thread_writeout, NULL);

memlog_dump(FRAME_LOG);

printf("clock_gettime takes an average of %ld nsec to run\n", clock_get_latency);

}

static int sequence = 0;
void sequencer(int v) {
    if (sequence % 3 == 0) { // 3 * 10 = 30ms , 33Hz
        sem_post(&sem_framegrab);
    }

    if (sequence % 3 == 0) { // 3 * 10 = 30ms, 33Hz (must keep up with input)
        sem_post(&sem_processing);
    }

    if (sequence % 10 == 0) { // 10 * 10 = 100ms, 10Hz
        sem_post(&sem_writeout);
    }

    sequence++; if (sequence == 90) { sequence = 0; }
}

void ctrl_c(int s) {
    running = 0;
}
