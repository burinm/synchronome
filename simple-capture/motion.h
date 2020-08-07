/* motion.h - Homegrown image detection state machine
    burin (c) 2020
*/
#ifndef __MOTION_H__
#define __MOTION_H__

#include "buffer.h"
#include "setup.h"

#define MOTION_DETECTED                 2
#define MOTION_IGNORE                   1
#define MOTION_NONE                     0

#define MOTION_SENSITIVITY              35
#define MOTION_THRESHOLD                50

#define MOTION_DROP_FRAMES              25 //ignore the first second of frames
#define MOTION_BOOKEND_FRAMES           3

#ifdef MODE_ALWAYS_DETECT_FRAME
    /* synced selection
        We want to select as soon as tick settles
    */
    #define MOTION_GOOD_FRAMES_THRESHOLD    4
#else
    /* Continuous detection (bookend ~2 + 9 = 12) 25/2 = 12.5
        We want to setup to select right in the middle
    */
    #define MOTION_GOOD_FRAMES_THRESHOLD    9
#endif

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
void set_state_MOTION_STATE_SEARCHING();

void print_motion_state();

//For external testing
int frame_changes_RGB(buffer_t *first, buffer_t *second);

#endif
