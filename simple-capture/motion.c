#include <stdlib.h> //abs
#include <stdio.h> //abs
#include <assert.h>
#include "motion.h"

void print_motion_state();

int motion_state = MOTION_STATE_START;
int good_frames_in_a_row = 0;
int drop_frames = MOTION_DROP_FRAMES;

int next_motion_state(int changed) {

    print_motion_state();

    switch(motion_state) {
        case MOTION_STATE_START: //TODO synchronization is in requirements. drift?
            if (drop_frames == MOTION_DROP_FRAMES) {
                motion_state = MOTION_STATE_SEARCHING;
            }
            drop_frames++;
            break;

        case MOTION_STATE_SEARCHING: //find different frame
            if (changed) {
                motion_state = MOTION_STATE_BOOKEND;
            }
            break;

        case MOTION_STATE_BOOKEND: //if there is a burst of different frames, don't start counting
            if (!changed) {
                motion_state = MOTION_STATE_COUNTING;
            }
            break;

        case MOTION_STATE_COUNTING:
            if (!changed) {
                good_frames_in_a_row++;
            } else {
                motion_state = MOTION_STATE_SEARCHING;
                good_frames_in_a_row = 0;
            }

            if (good_frames_in_a_row > MOTION_GOOD_FRAMES_THRESHOLD) {
                motion_state = MOTION_STATE_SEARCHING;
                good_frames_in_a_row = 0;
                return 1; //Hooray
            }
            break;
    }


return 0;
}

void print_motion_state() {

    switch(motion_state) {
        case MOTION_STATE_START:
            printf("MOTION_STATE_START:");
            break;

        case MOTION_STATE_SEARCHING: //find different frame
            printf("MOTION_STATE_SEARCHING:");
            break;

        case MOTION_STATE_BOOKEND: //find different frame
            printf("MOTION_STATE_BOOKEND:");
            break;

        case MOTION_STATE_COUNTING:
            printf("MOTION_STATE_COUNTING:");
            break;

        default:
            printf("MOTION - unknown state!");
    }

    printf("(%d)\n", good_frames_in_a_row);
}

int frame_changes(buffer_t *first, buffer_t *second) {

assert(first->start && second->start);
assert(first->size == second->size);

    int count = 0;
//TODO - iterate with words!
    //TODO - use words instead of bytes?
    unsigned char* f = first->start; 
    unsigned char* s = second->start; 

// YUYV

#if 0
box_t scan_area = { 213, 160, 426, 320 };

for (int i=scan_area.h1; i < scan_area.h2; i++, f += X_RES, s+= X_RES) { //every rom
    for (int j=scan_area.w1; j < scan_area.w2; j+=2, f+=2, s+=2) { //every other column (Y)
        int diff = abs(*f - *s);

        if (diff > 24) {
            //if (*f != *s) {
                //if ( ((*f) & 0xf0) == ((*s) &  0xf0)) {
                count++;
            //}
        }
    }
}
#endif

#if 1
    for (int i=0; i<first->size /2; i++) {
        int diff = abs(*f - *s);
        if (diff > MOTION_SENSITIVITY) { //sensitivity
            count++;
        }
        //Just diff Ys, skip U/V
        f+=2; s+=2;
    }
#endif

// greyscale
#if 0
    for (int i=0; i<first->size; i++) {
        if (*f != *s) {
            count++;
        }
        f++; s++;
    }
#endif
return count;
}

inline int is_motion(int count) {
    return next_motion_state(count > MOTION_THRESHOLD);
}
#if 0
inline int is_change() {
    return frame_changes(count > MOTION_THRESHOLD);
}
inline int image_threshold(int count) {
    return count > MOTION_THRESHOLD;
}
#endif
