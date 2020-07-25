#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <stdint.h>

#define CAMERA_DEV  "/dev/video0"
#define CAMERA_MAJ_ID   81

#define ERROR_FULL_INIT 4
#define ERROR_LEVEL_3   3
#define ERROR_LEVEL_2   2
#define ERROR_LEVEL_1   1
#define ERROR_LEVEL_0   0

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

//Initalization routines
void video_error_cleanup(int state, video_t *v);
int camera_check_init(video_t *v);


#endif
