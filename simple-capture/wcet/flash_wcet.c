#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../buffer.h"
#include "../dumptools.h"
#include "../timetools.h"

int printf_on = 0;

#define TEST_ITERS (1800 * 10)
#define NUM_IMAGES  10

#define BBP_PPM 3
int main() {

buffer_t random_images[NUM_IMAGES];

for (int i=0; i<NUM_IMAGES; i++) {
    allocate_buffer(&random_images[i], BBP_PPM);
}

for (int j=0; j < NUM_IMAGES; j++) {
    unsigned char* p = random_images[j].start;
    for (int i=0; i < random_images[j].size; i++) {
        *p++ = rand() % 256; 
    }
}

int random_index = 0;

struct timespec timestamp;
struct timespec last_timestamp;
last_timestamp.tv_sec = 0;

struct timespec diff_time;

int max_ms = 0;

struct timespec write_frequency;
write_frequency.tv_sec = 0;
write_frequency.tv_nsec = 100000000; //100ms, 10Hz


for (int i=0; i < TEST_ITERS; i++) {

    clock_gettime(CLOCK_MONOTONIC, &timestamp);

    if (last_timestamp.tv_sec != 0) {
        timespec_subtract(&diff_time, &timestamp, &last_timestamp);

        int total_ms = diff_time.tv_sec * 1000 + diff_time.tv_nsec/1000000;

        if (total_ms > max_ms) {
            max_ms = total_ms;
            printf("New max found: %dms\n", max_ms);
        }
        
    }
    last_timestamp = timestamp;

    //Since flash is so slow, we don't care if nanosleep lags
    nanosleep(&write_frequency, NULL);
     
    dump_rgb_raw_buffer(&random_images[random_index]);

    random_index++;
    if (random_index == NUM_IMAGES) {
        random_index = 0;
    }
}









}
