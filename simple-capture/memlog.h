/* memlog.h (c) burin 2020

    Quick memory logger for timestamps/events. Use one per thread,
    then there is no need to worry about concurrency ;)
    Put logs back together at end.

    TODO - this is a circular(ish) buffer. If all tasks run independantly,
    then we will be missing events if tasks run at different rates
*/  

#include <stdint.h>
#include <string.h> //memset
#include <time.h>
#define MEMLOG_MAX  1024

typedef struct _entry {
    //Running 32bit arm, should be aligned?
    uint32_t event_id; 
    struct timespec time;
} memlog_s;

typedef struct _memlog_g {
    uint32_t index;
    memlog_s log[MEMLOG_MAX];
} memlog_t;

#define MEMLOG_E_NONE               0x0
#define MEMLOG_E_S1_RUN             0x1
#define MEMLOG_E_S2_RUN             0x2
#define MEMLOG_E_S3_RUN             0x3
#define MEMLOG_E_SEQUENCER          0x4

#define MEMLOG_E_S1_PERIOD       0x11 
#define MEMLOG_E_S2_PERIOD       0x12
#define MEMLOG_E_S3_PERIOD       0x13


#define MEMLOG_E_FIB_TEST           0xff

#if 0
#define MEMLOG_LOG(l, event, t)   (l)->log[(l)->index].event_id = event; \
                               clock_gettime(CLOCK_REALTIME, &(l)->log[(l)->index].time); \
                               (l)->index++; \
                               if ((l)->index == MEMLOG_MAX) { \
                                    (l)->index = 0; \
                               } 
#endif
void MEMLOG_LOG(memlog_t* l, uint32_t event);


memlog_t* memlog_init();
void memlog_free(memlog_t* m);

void memlog_dump(memlog_t* l);
void memlog_gnuplot_dump(memlog_t* l);
char* memlog_event_desc(uint32_t e);

/*private*/
#define NS_PER_SEC  1000000000UL
#define US_PER_SEC  1000000UL
                                  
uint64_t time_in_us(struct timespec t);

