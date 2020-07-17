/* flash_wcet - try and find the worst latency for writing an image to flash
    (c) burin 2020

    For fun, but no profit, try running this on a remote
    machine while testing:

    watch rsync -avh 10.0.0.17:~/ecen5623/simple-capture/wcet/*.ppm .
*/

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

struct timespec start_time;
struct timespec end_time;

struct timespec timestamp;
struct timespec last_timestamp;
last_timestamp.tv_sec = 0;

struct timespec diff_time;

int max_ms = 0;
int total_count = 0;

struct timespec write_frequency;
write_frequency.tv_sec = 0;
write_frequency.tv_nsec = 100000000; //100ms, 10Hz


clock_gettime(CLOCK_MONOTONIC, &start_time);

for (int i=0; i < TEST_ITERS; i++) {

    clock_gettime(CLOCK_MONOTONIC, &timestamp);

    if (last_timestamp.tv_sec != 0) {
        timespec_subtract(&diff_time, &timestamp, &last_timestamp);

        int diff_ms = diff_time.tv_sec * 1000 + diff_time.tv_nsec/1000000;

        if (diff_ms > max_ms) {
            max_ms = diff_ms;
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

    total_count++;
}

clock_gettime(CLOCK_MONOTONIC, &end_time);

if (timespec_subtract(&diff_time, &end_time, &start_time) == 1) {
    printf("negative!\n");
} else {
    printf("Total elapsed time: %llds\n", (long long)diff_time.tv_sec);
    printf("average write time per (%d bytes) image: %fs\n", random_images[0].size, (float)diff_time.tv_sec/total_count);
}

}
