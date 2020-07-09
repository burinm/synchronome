#ifndef __SETUP_H__
#define __SETUP_H__

#define CAMERA_DEV  "/dev/video0"
#define CAMERA_MAJ_ID   81

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
int camera_set_yuyv(int camera_fd);

//Get size of capture buffer from format data
int query_buffer_size(int camera_fd);

int start_streaming(video_t *v);
int stop_streaming(video_t *v);

#endif
