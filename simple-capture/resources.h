/* resources.h - Central point to allocate buffers, queues
    burin (c) 2020

     Adjust all synchonome resources here
*/
#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <mqueue.h>
#include "buffer.h"
#include "queue.h"
#include "setup.h"

//Global stats
extern int total_frames_queued_g;
extern int total_frames_selected_g;
extern int total_frames_written_g;

//Internal camera buffers
#define CAMERA_NUM_BUF  3

//Scan buffer size
#define SCAN_BUF_SIZE 90

//Writeout buffer size
#define NUM_WO_BUF 100


//mmap buffers
extern buffer_t frame_buffers[CAMERA_NUM_BUF];

//scan buffers 
extern buffer_t scan_buffer[SCAN_BUF_SIZE];
extern int scan_buffer_index;

//write out buffers
extern buffer_t wo_buffers[NUM_WO_BUF];

//other buffers
extern buffer_t wo_buffer; //Temporary buffer for file output (write out)
extern buffer_t er_buffer; //Temporary buffer for file output (error out)
extern buffer_t sharpen_buffer;

//Allocations
int init_processing();
void deallocate_processing();

int init_writeout();
void deallocate_writeout();

int allocate_single_wo_buffer();
void deallocate_single_wo_buffer();

int allocate_single_er_buffer();
void deallocate_single_er_buffer();

int allocate_sharpen_buffer();
void deallocate_sharpen_buffer();

//Queues
//frame queue
extern queue_container_t frame_Q;

//writeout queue
extern queue_container_t writeout_Q;

#if 0
#define PROCESSING_Q "/processing_q"
extern mqd_t processing_Q;
#endif




#endif
