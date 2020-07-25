/* camera_buffer.h - Management of v4l2 internal camera buffers
    burin (c) 2020
*/
#ifndef __CAMERA_BUFFER_H__
#define __CAMERA_BUFFER_H__

#include <stddef.h>
#include "setup.h"
#include "camera.h"
#include "buffer.h"
#include "resources.h"

#include <linux/videodev2.h>

int camera_request_buffers(video_t *v);
void camera_deallocate_internal_buffers(video_t *v);

int camera_mmap_buffers(video_t *v);

int camera_enqueue_buf(struct v4l2_buffer* b, int camera_fd);
int camera_dequeue_buf(struct v4l2_buffer* b, int camera_fd);


#endif
