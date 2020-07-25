#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "buffer.h"
#include "setup.h"

//Internal camera buffers
#define CAMERA_NUM_BUF  3

//Scan buffer size
#define SCAN_BUF_SIZE 30

//Writeout buffer size
#define NUM_WO_BUF 300

extern buffer_t wo_buffer; //Temporary buffer for file output (write out)

//mmap buffers
extern buffer_t frame_buffers[CAMERA_NUM_BUF];

//scan buffers 
extern buffer_t scan_buffer[SCAN_BUF_SIZE];
extern int scan_buffer_index;

//write out buffers
extern buffer_t wo_buffers[NUM_WO_BUF];



#endif
