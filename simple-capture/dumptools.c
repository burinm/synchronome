#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "dumptools.h"
#include "transformation.h"
#include "setup.h"

char header_buf[PGM_HEADER_MAX_LEN];
buffer_t wo_buffer;

int _open_for_write(int index, char* suffix);

char filename[FILE_NAME_SIZE];
struct timespec timestamp;
int fd = -1;
int count = 0;

void dump_buffer_raw(buffer_t *b) {
    static int frame_num = 0;

    fd = _open_for_write(frame_num, "yuv");
    if (fd == -1) {
        return;
    }

    //TODO - break into chunks?
    count = write(fd, b->start, b->size);
    if (count != b->size) {
        console("all bytes not written %d of %d\n", count, b->size);
    }

    frame_num++;

    close(fd);
}

int header_with_timestamp(int fd) {

    int header_size = 0;
    int count = 0;

#ifdef PPM_CAPTURE
    header_size = snprintf(header_buf, PGM_HEADER_MAX_LEN, "%s%s#%010lu sec %09lu nsec\n%s",
                PPM_HEADER_DESC,
                PPM_HEADER_RES,
                timestamp.tv_sec, timestamp.tv_nsec,
                PPM_HEADER_DEPTH);
#endif

#ifdef PGM_CAPTURE
    header_size = snprintf(header_buf, PGM_HEADER_MAX_LEN, "%s%s#%010lu sec %09lu nsec\n%s",
                PGM_HEADER_DESC,
                PGM_HEADER_RES,
                timestamp.tv_sec, timestamp.tv_nsec,
                PGM_HEADER_DEPTH);
#endif

assert(header_size > 0);

    count = write(fd, header_buf, header_size);
    if (count != header_size) {
        console("all header bytes not written %d of %d\n", count, header_size);
    }

return count;
}

void dump_rgb_raw_buffer(buffer_t *b) {

    static uint16_t frame_num = 0;

    fd = _open_for_write(frame_num, IMAGE_SUFFIX);
    if (fd == -1) {
        return;
    }

    (void)header_with_timestamp(fd);

    //TODO - break into chunks?
    count = write(fd, b->start, b->size);
    if (count != b->size) {
        console("all bytes not written %d of %d\n", count, b->size);
    }

    frame_num++;

    close(fd);
}

int _open_for_write(int index, char* suffix) {

    //TODO - eliminate snprintf
    snprintf(filename, FILE_NAME_SIZE, "frame.%06u.%s", index, suffix);

    console("writing:%s\n", filename);
    fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        console("Couldn't create file %s", filename);
        perror(NULL);
        return -1;
    }
    return fd;
}
