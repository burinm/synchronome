#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>

#include "realtime.h"
#include "timetools.h"
#include "memlog.h"

void ctrl_c(int s);
void sequencer(int v);

sem_t sem_framegrab;
pthread_t thread_framegrab;
void* frame(void* v);
memlog_t* FRAME_LOG;

//stats
long int jitter_max = INT_MIN;
long int jitter_min = INT_MAX;
int jitter_frame = 10000000;

int running = 1;

int main() {

//Ctrl C
struct sigaction s0;
s0.sa_handler = ctrl_c;
sigaction(SIGINT, &s0, NULL);

//Catch SIGALRM and run seqencer
struct sigaction s1;
s1.sa_handler = sequencer;
sigaction(SIGALRM, &s1, NULL);

//Logging on
FRAME_LOG = memlog_init();

//Setup semaphores
if (sem_init(&sem_framegrab, 0, 0) == -1) {
    perror("Couldn't init semaphore sem_framegrab");
    exit(-1);
}

if (set_main_realtime() == -1) {
    exit(-1);
}

//Startup tests
long int clock_get_latency = test_clock_gettime_latency();
printf("clock_gettime takes an average of %ld nsec to run\n", clock_get_latency);


printf("Creating frame grabber thread\n");
pthread_attr_t rt_sched_attr;  // For realtime H/M/L threads
schedule_realtime(&rt_sched_attr);
schedule_priority(&rt_sched_attr, HIGH_PRI);

if (pthread_create(&thread_framegrab, &rt_sched_attr, frame, NULL) == -1) {
    perror("Couldn't create frame grabber thread");
    exit(-1);
}

//Interval timer for sequencer loop
timer_t timer1; // note not defined with a struct


printf("timer id = %d\n", timer1);
//Install timer
if (timer_create(CLOCK_MONOTONIC, NULL, &timer1) == -1 ) {
    perror("Couldn't create timer");
    exit(-1);
}
printf("timer id = %d\n", timer1);

struct itimerspec it;
it.it_interval.tv_sec = 0;
it.it_interval.tv_nsec = 10000000; //10ms 
it.it_value.tv_sec = 1; //delay 1 second to start
it.it_value.tv_nsec = 0;

if (timer_settime(timer1, 0, &it, NULL) == -1 ) {
    perror("couldn't set timer");
    exit(-1);
}


printf("Ready.\n");
pthread_join(thread_framegrab, NULL);

memlog_dump(FRAME_LOG);

printf("jitter max = % .ld\n", jitter_max - jitter_frame);
printf("jitter min = % .ld\n", jitter_min - jitter_frame);

}
void sequencer(int v) {
    sem_post(&sem_framegrab);
}

void* frame(void* v) {
    int ret = -1; 
    struct timespec tick;
    struct timespec tick_prev;
    struct timespec diff;

    memset (&tick_prev, 0, sizeof(struct timespec));

    printf("Frame grabber started\n");
    while(running) {

        ret = sem_wait(&sem_framegrab);

        clock_gettime(CLOCK_MONOTONIC, &tick);

        if (tick_prev.tv_sec != 0) {
            timespec_subtract(&diff, &tick, &tick_prev);
            //printf("jitter %lld.%.9ld\n", (long long)diff.tv_sec, diff.tv_nsec);

            if (diff.tv_nsec > jitter_max) {
                jitter_max = diff.tv_nsec;
            }

            if (diff.tv_nsec < jitter_min) {
                jitter_min = diff.tv_nsec;
            }
        }
        if (ret == -1) {
            perror("sem_wait sem_framegrab failed");
            //TODO - handle EINTR
            if (errno == EINTR) { //Aha, if this is in a different thread, the signal doesn't touch it
                continue;
            }
        }
        MEMLOG_LOG(FRAME_LOG, MEMLOG_E_S1_RUN);
        //printf("[frame]\n");
        tick_prev = tick;
    }
}

void ctrl_c(int s) {
    running = 0;
}
