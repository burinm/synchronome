#include <stdio.h>
#include <stdint.h>
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

int _open_for_write(int index, char* prefix, char* suffix);
int _header_with_timestamp(int fd, struct timespec *timestamp, int type, int id);


char filename[FILE_NAME_SIZE];
struct timespec timestamp;
int fd = -1;
int count = 0;

void dump_buffer_raw(buffer_t *b, int id, int debug) {

    char* prefix = NULL;
    if (debug == 1) {
        prefix = "./errors/buffer";
    }

    fd = _open_for_write(id, prefix, "yuv");
    if (fd == -1) {
        return;
    }

    //TODO - break into chunks?
    count = write(fd, b->start, b->size);
    if (count != b->size) {
        console("all bytes not written %d of %d\n", count, b->size);
    }

    close(fd);
}

int _header_with_timestamp(int fd, struct timespec *timestamp, int type, int id) {

    int header_size = 0;
    int count = 0;

if (type == PPM_BUFFER) {

    header_size = snprintf(header_buf, PGM_HEADER_MAX_LEN, "%s%s#%010lld.%09ld TIMESTAMP_E id:%06d\n%s",
                PPM_HEADER_DESC,
                PPM_HEADER_RES,
                (long long)timestamp->tv_sec, timestamp->tv_nsec,
                id,
                PPM_HEADER_DEPTH);

} else if (type == PGM_BUFFER) {

    header_size = snprintf(header_buf, PGM_HEADER_MAX_LEN, "%s%s#%010lld.%09ld TIMESTAMP_E id:%06d\n%s",
                PGM_HEADER_DESC,
                PGM_HEADER_RES,
                (long long)timestamp->tv_sec, timestamp->tv_nsec,
                id,
                PGM_HEADER_DEPTH);
}

#ifdef PGM_CAPTURE
#endif

assert(header_size > 0);

    count = write(fd, header_buf, header_size);
    if (count != header_size) {
        console("all header bytes not written %d of %d\n", count, header_size);
    }

return count;
}

static struct timespec frame_time;
void dump_raw_buffer_with_header(buffer_t *b, int type, int debug_id) {

    static uint16_t frame_num_s = 0;

    char* prefix = NULL;
    if (debug_id > -1) {
        prefix = "./errors/buffer";
    }

    uint16_t frame_num = frame_num_s;
    if (debug_id > -1) {
       frame_num = debug_id;
    }


    char* suffix = "xxx";
    if (type == PPM_BUFFER) {
        suffix = "ppm";
    } else if (type == PGM_BUFFER) {
        suffix = "pgm";
    }

    fd = _open_for_write(frame_num, prefix, suffix);
    if (fd == -1) {
        return;
    }

    BUFFER_GET_TIMESTAMP(*b, frame_time);

    (void)_header_with_timestamp(fd, &frame_time, type, b->id);

    //TODO - break into chunks?
    count = write(fd, b->start, b->size);
    if (count != b->size) {
        console("all bytes not written %d of %d\n", count, b->size);
    }

    if (debug_id == -1) {
        frame_num_s++;
        //Don't fill up disk ;)
        if (frame_num_s == FILE_NUMBER_MAX) {
            frame_num_s = 0;
        }
    }

    close(fd);
}

int _open_for_write(int index, char* prefix, char* suffix) {

    if (prefix == NULL) {
        prefix = "./frames/frame";
    }

    //TODO - eliminate snprintf
    snprintf(filename, FILE_NAME_SIZE, "%s.%06u.%s", prefix, index, suffix);

    console("writing:%s\n", filename);
    fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IROTH);
    if (fd == -1) {
        console("Couldn't create file %s", filename);
        perror(NULL);
        return -1;
    }
    return fd;
}
