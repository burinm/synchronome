#include <stdlib.h> //abs
#include <stdio.h> //abs
#include <assert.h>
#include "motion.h"

//int motion_state = MOTION_STATE_START;
int motion_state = MOTION_STATE_SEARCHING;

int drop_frames = 0;
int bookend_frames = 0;
int good_frames_in_a_row = 0;

int next_motion_state(int changed) {

    switch(motion_state) {
#if 0
        case MOTION_STATE_START: //TODO synchronization is in requirements. drift?
            if (drop_frames++ == MOTION_DROP_FRAMES) {
                drop_frames = 0; //We shouldn't ever start over, just for completeness
                motion_state = MOTION_STATE_SEARCHING;
            }
            return MOTION_IGNORE;
            break;
#endif

        case MOTION_STATE_SEARCHING: //find different frame
            if (changed) {
                motion_state = MOTION_STATE_BOOKEND;
            }
            break;

        case MOTION_STATE_BOOKEND: //assume a burst of different frames, skip them
            bookend_frames++;
            if (bookend_frames == MOTION_BOOKEND_FRAMES) {
                bookend_frames = 0;
                motion_state = MOTION_STATE_COUNTING;
            }
            break;

        case MOTION_STATE_COUNTING: //now look for all clean frames in a row
            if (!changed) {
                good_frames_in_a_row++;
            } else {
                motion_state = MOTION_STATE_SEARCHING;
                good_frames_in_a_row = 0;
            }

            if (good_frames_in_a_row > MOTION_GOOD_FRAMES_THRESHOLD) {
                motion_state = MOTION_STATE_SEARCHING;
                good_frames_in_a_row = 0;
                return MOTION_DETECTED; //Hooray
            }
            break;
    }


return MOTION_NONE;
}

void set_state_MOTION_STATE_SEARCHING() {
    motion_state = MOTION_STATE_SEARCHING;
    good_frames_in_a_row = 0;
}

void print_motion_state() {

    char* state;
    switch(motion_state) {
        case MOTION_STATE_START:
            state = "MOTION_STATE_START:";
            break;

        case MOTION_STATE_SEARCHING: //find different frame
            state = "MOTION_STATE_SEARCHING:";
            break;

        case MOTION_STATE_BOOKEND: //find different frame
            state = "MOTION_STATE_BOOKEND:";
            break;

        case MOTION_STATE_COUNTING:
            state = "MOTION_STATE_COUNTING:";
            break;

        default:
            state = "MOTION - unknown state!";
    }

    printf("%-25s(%d)", state, good_frames_in_a_row);
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
int frame_changes_RGB(buffer_t *first, buffer_t *second) {
assert(first->start && second->start);
assert(first->size == second->size);

    int count = 0;
    unsigned char* f = first->start; 
    unsigned char* s = second->start; 

// RGB
    int diff_total = 0;
    int diff_r = 0;
    int diff_g = 0;
    int diff_b = 0;

    for (int i=0; i<first->size; i+=3) {
        diff_r = abs(*f - *s);
        f++; s++;
        diff_g = abs(*f - *s);
        f++; s++;
        diff_b = abs(*f - *s);
        f++; s++;

        diff_total = diff_r + diff_g + diff_b; 
        if (diff_total > MOTION_SENSITIVITY * 3) { //sensitivity
            count++;
        }
    }
return count;
}
