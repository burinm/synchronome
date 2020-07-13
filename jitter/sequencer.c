#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include "realtime.h"

void sequencer(int v);

pthread_t thread_framegrab;
void* frame(void* v);


sem_t sem_framegrab;


int running = 1;

int main() {

//Catch SIGALRM and run seqencer
struct sigaction s;
//memset(&s, 0, sizeof(struct sigaction));
s.sa_handler = sequencer;
sigaction(SIGALRM, &s, NULL);

//Setup semaphores
if (sem_init(&sem_framegrab, 0, 0) == -1) {
    perror("Couldn't init semaphore sem_framegrab");
    exit(-1);
}

if (set_main_realtime() == -1) {
    exit(-1);
}

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
it.it_interval.tv_sec = 1;
it.it_interval.tv_nsec = 0; 
it.it_value.tv_sec = 0;
it.it_value.tv_nsec = 1;

if (timer_settime(timer1, 0, &it, NULL) == -1 ) {
    perror("couldn't set timer");
    exit(-1);
}


printf("Ready.\n");
pthread_join(thread_framegrab, NULL);

}
void sequencer(int v) {
    sem_post(&sem_framegrab);
}

void* frame(void* v) {
    int ret = -1; 

    printf("Frame grabber started\n");
    while(running) {
        ret = sem_wait(&sem_framegrab);
        if (ret == -1) {
            perror("sem_wait sem_framegrab failed");
            //TODO - handle EINTR
            if (errno == EINTR) { //Aha, if this is in a different thread, the signal doesn't touch it
                continue;
            }
        }

        printf("[frame]\n");
    }
}
