#include "resources.h"


buffer_t wo_buffer;

//mmap buffers
buffer_t frame_buffers[CAMERA_NUM_BUF];

//scan buffers 
buffer_t scan_buffer[SCAN_BUF_SIZE];
int scan_buffer_index;

//write out buffers
buffer_t wo_buffers[NUM_WO_BUF];

//Allocations processing
int init_processing() {

    for (int i=0; i<SCAN_BUF_SIZE; i++) {
        if (allocate_frame_buffer(&scan_buffer[i]) == -1) {
            return -1;
        }
    }
return 0;
}

void deallocate_processing() {
    for (int i=0; i<SCAN_BUF_SIZE; i++) {
        deallocate_buffer(&scan_buffer[i]);
    }
}

//Allocations writeout
int init_writeout() {
    for (int i=0; i < NUM_WO_BUF; i++) {
        if (allocate_frame_buffer(&wo_buffers[i]) == -1)  {
            deallocate_writeout();
            return -1;
        }
    }
return 0;
}

void deallocate_writeout() {
    for (int i=0; i < NUM_WO_BUF; i++) {
        deallocate_buffer(&wo_buffers[i]);
    }
}
