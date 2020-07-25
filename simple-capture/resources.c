#include "resources.h"


buffer_t wo_buffer;

//mmap buffers
buffer_t frame_buffers[CAMERA_NUM_BUF];

//scan buffers 
buffer_t scan_buffer[SCAN_BUF_SIZE];
int scan_buffer_index;

//write out buffers
buffer_t wo_buffers[NUM_WO_BUF];


