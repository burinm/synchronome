#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "dumptools.h"

char filename[FILE_NAME_SIZE];
struct timespec timestamp;
int fd = -1;
int count = 0;

void dump_buffer_with_timestamp(buffer_t *b) {
    
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    snprintf(filename, FILE_NAME_SIZE, "frame%lu.%09lu.yuv", timestamp.tv_sec, timestamp.tv_nsec);

    printf("writing:%s\n", filename);

    fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("Couldn't create file %s", filename);
        perror(NULL);
        return;
    }

    count = write(fd, b->start, b->size);
    if (count != b->size) {
        printf("all bytes not written %d of %d\n", count, b->size);
    }
    
    close(fd);    

}
