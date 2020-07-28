#include <stdio.h>
#include <stddef.h>
#include "camera_init.h"
#include "camera_buffer.h"
#include "setup.h"

int camera_init_all(video_t * video) {

    if (camera_check_init(video) == -1) {
        if (close_camera(video->camera_fd) == -1) {
            console("problem closing fd=%d\n", video->camera_fd);
            perror(NULL);
        } else {
            console("closed camera device\n");
        }
        video_error_cleanup(ERROR_LEVEL_0, video);
        return(-1);
    }

    if (camera_init_internal_buffers(video) == -1) {
        video_error_cleanup(ERROR_LEVEL_2, video);
        return(-1);
    }

    if (allocate_single_wo_buffer() == -1) {
        video_error_cleanup(ERROR_LEVEL_3, video);
        return(-1);
    }

    #ifdef SHARPEN_ON
    if (allocate_sharpen_buffer() == -1) {
        video_error_cleanup(ERROR_LEVEL_3, video);
        return(-1);
    }
    #endif
return 0;
}

int camera_start_streaming(video_t *video) {
    //Start streaming
    if (start_streaming(video) == -1) {
        perror("Couldn't start stream");
        video_error_cleanup(ERROR_LEVEL_3, video);
        return(-1);
    }

    if (try_refocus(video->camera_fd) == -1) {
        console("Refocus failed!\n");
        video_error_cleanup(ERROR_LEVEL_3, video);
        return(-1);
    }

    #ifdef SHARPEN_ON
        console("\nApplying filter: ");
        print_sharpen_filter();
        console("\n");
    #endif
return 0;
}
