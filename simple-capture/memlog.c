/* memlog.c - burin (c) 2020
*/
#include <stdlib.h> //calloc, free
#include <stdio.h>
#include <assert.h>
#include "memlog.h"
#include "timetools.h"

memlog_t* memlog_init() {
    memlog_t* mem_log = (memlog_t*)calloc(1, sizeof(memlog_t));
    if (!mem_log) {
        printf("Error: couldn't allocate memory\n");
        exit(-1);
    }
return mem_log;
}

void memlog_free(memlog_t* m) {
    if (m) {
        free(m);
    }
}

inline void MEMLOG_LOG(memlog_t* l, uint32_t event) {
    assert(l->index < MEMLOG_MAX);
    l->log[l->index].event_id = event;
    clock_gettime(CLOCK_MONOTONIC, &l->log[l->index].time);
    l->index++;
    if (l->index >= MEMLOG_MAX) {
        l->index = 0;
    }
}


void memlog_dump(memlog_t* l) {
    struct timespec t_prev = l->log[0].time; //TODO, problem if first entry is MEMLOG_E_NONE
    struct timespec diff;
    struct timespec t;
    int not_increasing = 0;

    for (int i=0; i < MEMLOG_MAX; i++) {
        if (l->log[i].event_id > MEMLOG_E_NONE) {

            //timespec_subtract is destructive, save previous
            t = l->log[i].time;

            // Without the correct formatting, overflows vargs!
            // https://stackoverflow.com/questions/8304259/formatting-struct-timespec
            
            not_increasing = timespec_subtract(&diff, &l->log[i].time, &t_prev);

            printf("%lld.%.9ld [%s]", (long long)l->log[i].time.tv_sec, l->log[i].time.tv_nsec,
                                      memlog_event_desc(l->log[i].event_id));
            printf("%s", not_increasing == 1 ? " *not in order\n" : "");
            printf(" diff = %lld.%.9ld\n", (long long)diff.tv_sec, diff.tv_nsec);
            t_prev = t;
        }
    }
}

void memlog_gnuplot_dump(memlog_t* l) {
    uint64_t t = 0;

    for (int i=0; i < MEMLOG_MAX; i++) {
        if (l->log[i].event_id > MEMLOG_E_NONE) {
            t = time_in_us(l->log[i].time);
            printf("%.11lld %d\n", t, l->log[i].event_id);
        }
    }
}

char* memlog_event_desc(uint32_t e) {
    switch(e) {
        case MEMLOG_E_NONE:
            return "NONE";
            break;

        case MEMLOG_E_S1_PERIOD:
            return "MEMLOG_E_S1_PERIOD";
            break;

        case MEMLOG_E_S2_PERIOD:
            return "MEMLOG_E_S2_PERIOD";
            break;

        case MEMLOG_E_S3_PERIOD:
            return "MEMLOG_E_S3_PERIOD";
            break;

        case MEMLOG_E_S1_RUN:
            return "MEMLOG_E_S1_RUN";
            break;

        case MEMLOG_E_S2_RUN:
            return "MEMLOG_E_S2_RUN";
            break;

        case MEMLOG_E_S3_RUN:
            return "MEMLOG_E_S3_RUN";
            break;

        case MEMLOG_E_SEQUENCER:
            return "SEQUENCER_MARK";
            break;

        case MEMLOG_E_FIB_TEST:
            return "FIBTEST_MARK";
            break;
    };

return "Event error";
}

uint64_t time_in_us(struct timespec t) {
    return (t.tv_sec * US_PER_SEC + t.tv_nsec / 1000);
}
