/* processing.h - Frame selection driver
    burin (c) 2020
*/
#ifndef __PROCESSING_H__
#define __PROCESSING_H__

#include "buffer.h"
#include "setup.h"

#define MOTION_FRAMES_SEC  25 //TODO, link with seqencer code #error
#define MOTION_SELECTIONS_IGNORE  5
/* How many frames can we miss the middle by?
    This is affected by how long the bookend is,
    and that depends on how long the "bounce" or
    overshoot of the analog tick is
*/
#define MOTION_FRAMES_DRIFT       3 // +/- 3 frames = .040 x 3 = 120ms

void* processing(void* v);

#endif
