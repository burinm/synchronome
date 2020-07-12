#ifndef __SETUP_H__
#define __SETUP_H__

//Test frames without writing to disk
#define PROFILE_FRAMES
#define PROFILE_ITERS   500

#define CAMERA_DEV  "/dev/video0"
#define CAMERA_MAJ_ID   81

#define X_RES   640
#define Y_RES   480

//Select one
//#define PPM_CAPTURE
#define PGM_CAPTURE

//Select other options
#define SHARPEN_ON
//#define SHARPEN_TYPE SHARPEN_WIKIPEDIA_EXAMPLE
#define SHARPEN_TYPE SHARPEN_PROFESSOR_EXAMPLE

extern int printf_on;
// https://stackoverflow.com/questions/20639632/how-to-wrap-printf-into-a-function-or-macro
#define console(format, ...) if (printf_on) { printf((format), ##__VA_ARGS__); }
#define console_error(format, ...) printf((format), ##__VA_ARGS__)

#include <stdint.h>

typedef struct {
    int camera_fd;
    int width;
    int height;
    uint32_t num_buffers;
    uint32_t type;
    uint32_t memory;
} video_t;


int open_camera(char* camera, video_t *v);
int close_camera(int fd);
int show_camera_capabilities(int camera_fd);
int enumerate_camera_image_formats(int camera_fd);
int show_camera_image_format(int camera_fd);
int camera_set_yuyv(video_t *v, int width, int height);

//Get size of capture buffer from format data
int query_buffer_size(int camera_fd);

int start_streaming(video_t *v);
int stop_streaming(video_t *v);

//Some controls
int try_refocus(int camera_fd);


#endif
