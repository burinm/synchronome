#ifndef __WRITEOUT_H__
#define __WRITEOUT_H__

#include "buffer.h"

#define NUM_WO_BUF 50
extern buffer_t wo_buffers[NUM_WO_BUF];

void* writeout(void* v);

#endif
