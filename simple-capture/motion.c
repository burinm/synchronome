#include <assert.h>
#include "motion.h"

int frame_percent_change(buffer_t *first, buffer_t *second) {

assert(first->start && second->start);
assert(first->size == second->size);

    int count = 0;
    //TODO - use words instead of bytes?
    unsigned char* f = first->start; 
    unsigned char* s = second->start; 

// YUYV

#if 1
box_t scan_area = { 213, 160, 426, 320 };

    for (int i=scan_area.h1; i < scan_area.h2; i++, f += X_RES, s+= X_RES) { //every rom
        for (int j=scan_area.w1; j < scan_area.w2; j+=2, f+=2, s+=2) { //every other column (Y)
            if (*f != *s) {
                count++;
            }
        }
    }
#endif


#if 0
    for (int i=0; i<first->size /2; i++) {
        //if (((*f) & 0xf0) != ((*s) & 0xf0)) {
        if (*f != *s) {
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
