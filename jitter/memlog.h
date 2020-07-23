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

#define MEMLOG_MAX 30000 

typedef struct _entry {
    //Running 32bit arm, should be aligned?
    uint32_t event_id; 
    struct timespec time;
} memlog_s;

typedef struct _memlog_g {
    uint32_t index;
    memlog_s log[MEMLOG_MAX];
} memlog_t;

/* memlog events */
//Program flow events
#define MEMLOG_E_NONE           0x0
#define MEMLOG_E_S1_RUN         0x1
#define MEMLOG_E_S2_RUN         0x2
#define MEMLOG_E_S3_RUN         0x3
#define MEMLOG_E_SEQUENCER      0x4

#define MEMLOG_E_S1_DONE       0x11
#define MEMLOG_E_S2_DONE       0x12
#define MEMLOG_E_S3_DONE       0x13

#define MEMLOG_E_WCET_START    0x1a
#define MEMLOG_E_WCET_DONE     0x1b


//Data logging events
#define MEMLOG_E_ADATA_24       0x20
#define MEMLOG_E_BDATA_24       0x21
#define MEMLOG_E_CDATA_24       0x22
#define MEMLOG_E_FIB_TEST      0xff

//Field manipulation
#define MEMLOG_ID_MASK             0xff
#define MEMLOG_ID(x)               (x & MEMLOG_ID_MASK)

//Field data manipulation
#define MEMLOG_DATA24(x)           (x >> 8)
#define MEMLOG_ENCODE24(event, data)    (event + (data << 8))


#if 0
#define MEMLOG_LOG(l, event, t)   (l)->log[(l)->index].event_id = event; \
                               clock_gettime(CLOCK_REALTIME, &(l)->log[(l)->index].time); \
                               (l)->index++; \
                               if ((l)->index == MEMLOG_MAX) { \
                                    (l)->index = 0; \
                               } 
#endif
void MEMLOG_LOG(memlog_t* l, uint32_t event);
void MEMLOG_LOG24(memlog_t* l, uint32_t event, uint32_t data);


memlog_t* memlog_init();
void memlog_free(memlog_t* m);

void memlog_dump(char* f, memlog_t* l);
void memlog_gnuplot_dump(memlog_t* l);
char* memlog_event_desc(uint32_t e);

/*private*/
#define NS_PER_SEC  1000000000UL
#define US_PER_SEC  1000000UL
                                  
uint64_t time_in_us(struct timespec t);

