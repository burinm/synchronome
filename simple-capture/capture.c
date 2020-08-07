/* capture.c - Stand-alone driver for testing frame capture
    burin (c) 2020

    Compiles in frame() function into "capture" executable
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev
#include <assert.h>

#include "setup.h" //Keep this at top
#include "frame.h"
#include "camera_buffer.h"
#include "camera_init.h"
#include "resources.h"
#include "buffer.h"
#include "transformation.h"
#include "processing.h"
#include "dumptools.h"
#include "memlog.h"

#ifdef IMAGE_DIFF_PROFILE
    #include "motion.h"
#endif


#ifdef PROFILE_FRAMES
    #include <time.h>
    #include <limits.h>
    #include "timetools.h"
#endif

#ifdef SHARPEN_ON
    #include "sharpen.h"
#endif


/* catch signal */
#include <signal.h>
void ctrl_c(int addr);
int printf_on = 1;
int running = 1;

int main() {

    //install ctrl_c signal handler
    struct sigaction action;
    action.sa_handler = ctrl_c;
    sigaction(SIGINT, &action, NULL);

    video_t video;
    memset(&video, 0, sizeof(video_t));
    video.camera_fd = -1;

    if (open_camera(CAMERA_DEV, &video) == -1) {
        error_exit(-1);
    }

    if (camera_init_all(&video) == -1 ) {
        printf("Couldn't finish camera init\n");
        error_exit(-1);
    }

    if (camera_start_streaming(&video) == -1 ) {
        printf("Couldn't start camera streaming\n");
        error_exit(-1);
    }

    init_processing();

    int ret = (int)frame((void*)&video);

    //This does camera stop, clean up etc..
    video_error_cleanup(ERROR_FULL_INIT, &video);

return ret;
}

void ctrl_c(int addr) {
    console("ctrl-c\n");
    running = 0;
}
