#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "dumptools.h"
#include "transformation.h"
#include "setup.h"

int _open_for_write(char* suffix);

char filename[FILE_NAME_SIZE];
struct timespec timestamp;
int fd = -1;
int count = 0;


void dump_buffer_raw(buffer_t *b) {


    fd = _open_for_write("yuv");
    if (fd == -1) {
        return;
    }

    count = write(fd, b->start, b->size);
    if (count != b->size) {
        printf("all bytes not written %d of %d\n", count, b->size);
    }

    close(fd);
}

void dump_yuv422_to_rgb_raw(buffer_t *b) {
#define BYTES_YUYV_PIXELS 4
#define BYTES_RGB_PIXELS  6

//https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html (lame!)
#define xstr(s) str(s)
#define str(s) #s

//Boo, this is brittle
#define PPM_HEADER  xstr(P6\n)\
                    xstr(X_RES Y_RES)\
                    xstr(\n255\n)

#define PPM_SUFFIX      "ppm"

    int Y0, Cb, Y1, Cr;
    unsigned char R, G, B;
    unsigned char rgb_two_pixels[BYTES_RGB_PIXELS];

    fd = _open_for_write(PPM_SUFFIX);
    if (fd == -1) {
        return;
    }
    count = write(fd, PPM_HEADER, sizeof(PPM_HEADER));
    if (count != sizeof(PPM_HEADER)) {
        printf("all bytes not written %d of %d\n", count, sizeof(PPM_HEADER));
    }


    unsigned char * iter = (unsigned char*)b->start;
    for (int i=0; i<b->size; i+=BYTES_YUYV_PIXELS) {
        if (b->size - i >= BYTES_YUYV_PIXELS) {
            Y0 = (int)iter[i];
            Cb = (int)iter[i+1];
            Y1 = (int)iter[i+2];
            Cr = (int)iter[i+3];

            yuv2rgb(Y0, Cb, Cr, &R, &G, &B);
            rgb_two_pixels[0] = R; rgb_two_pixels[1] = G; rgb_two_pixels[2] = B;
            yuv2rgb(Y1, Cb, Cr, &R, &G, &B);
            rgb_two_pixels[3] = R; rgb_two_pixels[4] = G; rgb_two_pixels[5] = B;

            count = write(fd, rgb_two_pixels, BYTES_RGB_PIXELS);
            if (count != BYTES_RGB_PIXELS) {
                printf("all bytes not written %d of %d\n", count, BYTES_RGB_PIXELS);
            }
        }
    }
}

inline int _open_for_write(char* suffix) {

    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    snprintf(filename, FILE_NAME_SIZE, "frame%lu.%09lu.%s", timestamp.tv_sec, timestamp.tv_nsec, suffix);

    printf("writing:%s\n", filename);
    fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("Couldn't create file %s", filename);
        perror(NULL);
        return -1;
    }
    return fd;
}
