#include "resources.h"



//mmap buffers
buffer_t frame_buffers[CAMERA_NUM_BUF];

//scan buffers 
buffer_t scan_buffer[SCAN_BUF_SIZE];
int scan_buffer_index;

//write out buffers
buffer_t wo_buffers[NUM_WO_BUF];

//single wo buffer
buffer_t wo_buffer;

//sharpen buffer
buffer_t sharpen_buffer;

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

//Allocations other
int allocate_single_wo_buffer() {
    if (allocate_buffer(&wo_buffer, BYTES_PER_PIXEL) == -1) {
        console("couldn't allocate write out buffer\n");
        return -1;
    }
return 0;
}

int allocate_sharpen_buffer() {
    if (allocate_buffer(&sharpen_buffer, BYTES_PER_PIXEL) == -1) {
        console("couldn't allocate write out buffer\n");
        return -1;
    }
return 0;
}

void deallocate_single_wo_buffer() {
    deallocate_buffer(&wo_buffer);
}

void deallocate_sharpen_buffer() {
    deallocate_buffer(&sharpen_buffer);
}


//Queues
queue_container_t frame_Q = {
    .name = "/frame_recieve_q",
    .max_payload_size = sizeof(buffer_t),
    .num_elems = CAMERA_NUM_BUF,
    .b = NULL
};

queue_container_t writeout_Q = {
    .name = "/writeout_q",
    .max_payload_size = sizeof(buffer_t),
    .num_elems = NUM_WO_BUF,
    .b = NULL
};
