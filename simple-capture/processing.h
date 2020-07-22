#ifndef __PROCESSING_H__
#define __PROCESSING_H__

#include "buffer.h"

#define SCAN_BUF_SIZE 30 //TODO - WARNING, this is defined two places, refactor
extern buffer_t scan_buffer[SCAN_BUF_SIZE];
extern int scan_buffer_index;

void* processing(void* v);
int init_processing();
void deallocate_processing();


#endif
