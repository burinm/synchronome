#include "buffer.h"
#include "setup.h"

/* regular buffer management */
int allocate_buffer(buffer_t *b, int blocks) {
    b->index = -1;
    int size = sizeof(unsigned char) * FRAME_SIZE * blocks;
    b->start = malloc(size);
    b->size = size;
    if (b->start) {
        return 0;
    } else {
        return -1;
    }
}

int allocate_frame_buffer(buffer_t *b) {
    return allocate_buffer(b, NATIVE_CAMERA_FORMAT_SIZE);
}

void deallocate_buffer(buffer_t *b) {
    if (b->start) {
       free(b->start);
    }
}
