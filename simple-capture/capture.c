#include <stdlib.h>
#include "setup.h"


int camera_fd = -1;

int main() {

if ((camera_fd = open_camera(CAMERA_DEV)) == -1) {
    exit(0);
}

if (show_camera_capabilities(camera_fd) == -1) {
    goto error;
}

if (enumerate_camera_image_formats(camera_fd) == -1) {
    goto error;
}

if (show_camera_image_format(camera_fd) == -1) {
    goto error;
}

if (camera_set_yuyv(camera_fd) == -1) {
    goto error;
}

if (show_camera_image_format(camera_fd) == -1) {
    goto error;
}




error:
close_camera(camera_fd);
}


