/* camera_init.h - camera initialization routines
    burin (c) 2020

    Used to aggregate init functions so they can be used stand-alone
*/
#ifndef __CAMERA_INIT_H__
#define __CAMERA_INIT_H__

#include "camera.h"

int camera_init_all(video_t * video);
int camera_start_streaming(video_t *video);

#endif
