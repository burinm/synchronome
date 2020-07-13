#ifndef __REALTIME_H__
#define __REALTIME_H__

#include <pthread.h>

#define RT_MAX_PRIO     (sched_get_priority_max(SCHED_FIFO))
#define RT_MIN_PRIO     (sched_get_priority_min(SCHED_FIFO))

#define NUM_PROCESSORS      4 //Raspberry Pi 3b+
#define PROCESSOR_ZERO  0
#define PROCESSOR_ONE   1
#define PROCESSOR_TWO   2
#define PROCESSOR_THREE 3

#define HIGHEST_PRI   (RT_MAX_PRIO - 1)
#define HIGH_PRI   (RT_MAX_PRIO - 2)
#define MID_PRI    (RT_MAX_PRIO - 3)
#define LOW_PRI    (RT_MAX_PRIO - 4)



int set_main_realtime();
int schedule_realtime(pthread_attr_t *attr);
int schedule_priority(pthread_attr_t *attr, int pri);


#endif
