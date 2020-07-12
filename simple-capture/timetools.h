#ifndef __TIMETOOLS_H__
#define __TIMETOOLS_H__

#include <time.h>


// x -y, Note: destructive to y, result. returns 1 if negative 
int timespec_subtract(struct timespec *result,
                       struct timespec *x,
                       struct timespec *y);


#endif
