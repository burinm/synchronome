/* pieces taken from:
    siewert (c) last decade
    http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/example-sync-updated-2/pthread3ok.c
    http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/sequencer_generic/seqgen3.c

    modified by burin (c) 2020, Exerise #3 and final project
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "realtime.h"
#include "timetools.h"

int set_main_realtime() { //TODO - for now, this is sharing processor 1

    cpu_set_t threadcpu;
    CPU_ZERO(&threadcpu);
    int coreid = PROCESSOR_ONE;
    CPU_SET(coreid, &threadcpu);

    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &threadcpu) == -1) {
        perror("Couldn't set main program CPU affinity");
        return -1;
    }
    struct sched_param main_param;
    if (sched_getparam(getpid(), &main_param) == -1) {
        perror("Couldn't get main program schedule attributes");
        return -1;
    }

    main_param.sched_priority = HIGHEST_PRI;
    if (sched_setscheduler(getpid(), SCHED_FIFO, &main_param) == -1) {
        perror("Couldn't set main to realtime");
        return -1;
    }

return 0;
}

int schedule_realtime(pthread_attr_t *attr) {
    //Setup realtime threads' schedule policy
    if (pthread_attr_init(attr) != 0) {
        perror("pthread_attr_init");
        return -1;
    }

    if (pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED) != 0) {
        perror("pthread_attr_setinheritsched");
        return -1;
    }

    if (pthread_attr_setschedpolicy(attr, SCHED_FIFO) != 0) {
        perror("pthread_attr_setschedpolicy");
        return -1;
    }

    //For now, AMP design, procesor 1
    cpu_set_t threadcpu;
    CPU_ZERO(&threadcpu);
    int coreid = PROCESSOR_ONE;
    CPU_SET(coreid, &threadcpu);

    if (pthread_attr_setaffinity_np(attr, sizeof(cpu_set_t), &threadcpu) != 0) {
        perror("pthread_attr_setaffinity_np");
        return -1;
    }

return 0;
}

int schedule_priority(pthread_attr_t *attr, int pri) {
    struct sched_param rt_param; //TODO - can this be on the stack?
    rt_param.sched_priority = pri;
    if (pthread_attr_setschedparam(attr, &rt_param) == 0) {
        return 0;
    }
return -1;
}

long int test_clock_gettime_latency() {
    struct timespec start;
    struct timespec finish;
    struct timespec dummy;
    long int count = 100000;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (long int i=0; i < count; i++) {
        (void)clock_gettime(CLOCK_MONOTONIC, &dummy);
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);

    if (timespec_subtract(&dummy, &finish, &start) == 1) {
        printf("time was negative!\n");
    }

    long int result = dummy.tv_sec * 1000000000;
    result += dummy.tv_nsec;
    if (dummy.tv_sec > 0) {
        printf("Warning, may overflow\n");
    }
    printf("clock_gettime took %ld nsec for %ld iterations\n", result, count);

    result /= count;

return result;
}
