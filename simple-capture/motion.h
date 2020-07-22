#ifndef __MOTION_H__
#define __MOTION_H__

#include "buffer.h"

#define MOTION_SENSITIVITY              35
#define MOTION_THRESHOLD                50

#define MOTION_START_GOOD_FRAMES        8
#define MOTION_GOOD_FRAMES_THRESHOLD    4

//At the start look for a bunch of same frames before we start
#define MOTION_STATE_START              0
//Look for a different frame, if found start counting
#define MOTION_STATE_SEARCHING          1
//Start of changing frames
#define MOTION_STATE_BOOKEND            2
//Count same frames up to threshold, then copy out, goto MOTION_STATE_SEARCHING
#define MOTION_STATE_COUNTING           3

typedef struct {
    int w1;
    int h1;
    int w2;
    int h2;
} box_t;

int is_motion(int count);
int next_motion_state(int changed);
int frame_changes(buffer_t *first, buffer_t *second);



#endif
