#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "processing.h"
#include "camera_buffer.h"
#include "resources.h"
#include "writeout.h"
#include "setup.h"
#include "buffer.h"
#include "queue.h"
#include "motion.h"
#include "memlog.h"

#include "dumptools.h" //FOR errors only
#include "transformation.h" //FOR errors only
extern int freeze_system; //DEBUG

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_processing;
extern sem_t sem_teardown;

memlog_t* PROCESSING_LOG;

void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    PROCESSING_LOG = memlog_init();

    int wo_buffer_index = 0;

    int s_ret = -1;
    //struct v4l2_buffer b;

    int last_buffer_index = -1;
    int changed_pixels = 0;
    int did_frame_tick = 0;

    int num_frames_till_selection = 0;


    //pthread_barrier_wait(&bar_thread_inits); //GO!!

    //Best effort!
    s_ret = sem_wait(&sem_processing);
    if (s_ret == -1) {
        perror("sem_wait sem_processing failed");
        error_exit(-2);
    }

    while(running) {


        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_DONE);
        while(1) {


            int current_index;
            int ret;
            ret = dequeue_P(&frame_Q, &current_index);
            if (ret == 1) {
                break; //Empty queue, give back CPU
            } else if (ret == -1) {
                printf("*Frame Processing: dequeue error\n");
                error_exit(-1);
            }
            num_frames_till_selection++;

            printf("Processing: [index %d] (VIDIOC_DEQBUF)\n", current_index);

            did_frame_tick = 0;

            if (last_buffer_index != -1) {

                printf("Processing: [index %d start=%p size=%d] (in)\n",
                        last_buffer_index,
                        scan_buffer[last_buffer_index].start,
                        scan_buffer[last_buffer_index].size);

                changed_pixels = frame_changes(&scan_buffer[last_buffer_index], &scan_buffer[current_index]);
                printf("Processing %d (%06d) vs %d (%06d) frame diff = %d ",
                                last_buffer_index,
                                scan_buffer[last_buffer_index].id,
                                current_index,
                                scan_buffer[current_index].id,
                                changed_pixels);
                did_frame_tick = is_motion(changed_pixels);

                printf("%s\n", did_frame_tick ? "yes" : "no");

                assert(scan_buffer[current_index].size == wo_buffers[wo_buffer_index].size);

            }

            if (num_frames_till_selection > 26) { //Sync missed frame, force algorithm
                MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_FORCE_FRAME);
                set_state_MOTION_STATE_SEARCHING();
                did_frame_tick = 1;
            }

            //Copy frame to writeout buffer
            if (did_frame_tick) {
#if 0
                memcpy((unsigned char*)wo_buffers[wo_buffer_index].start,
                        (unsigned char*)(scan_buffer[current_index].start),
                        scan_buffer[current_index].size);
#endif
                COPY_BUFFER(wo_buffers[wo_buffer_index], scan_buffer[current_index]);
                COPY_BUFFER_TIMESTAMP(wo_buffers[wo_buffer_index], scan_buffer[current_index]);
                wo_buffers[wo_buffer_index].id = scan_buffer[current_index].id;

                enqueue_P(&writeout_Q, &wo_buffer_index);
                total_frames_selected_g++;

                //ghetto circular buffer
                wo_buffer_index++;
                if (wo_buffer_index == NUM_WO_BUF) {
                    wo_buffer_index = 0;
                }

            }

            last_buffer_index = current_index;


            if (did_frame_tick) {
                    //Found frame and sent to write Q
                    break;
            }
        } //forever, until change is detected
        MEMLOG_LOG24(PROCESSING_LOG, MEMLOG_E_ADATA_24, num_frames_till_selection);

        if (num_frames_till_selection > 35) {
            freeze_system = 1; //Makes sequencer stop timer
            printf("Bork! took %d frames to select. id:%d \n", num_frames_till_selection,
                                                            scan_buffer[last_buffer_index].id );
            for (int i=0; i < SCAN_BUF_SIZE; i++) {
                int buffer_id = scan_buffer[i].id;

                dump_buffer_raw(&scan_buffer[i], buffer_id, 1);

                COPY_BUFFER_TIMESTAMP(er_buffer, scan_buffer[i]);
                er_buffer.id =  scan_buffer[i].id;

                #ifdef PPM_CAPTURE
                    yuv422torgb888(&scan_buffer[i], &er_buffer, 0);
                    dump_raw_buffer_with_header(&er_buffer, PPM_BUFFER, buffer_id);
                #endif

                #ifdef PGM_CAPTURE
                    yuv422toG8(&scan_buffer[i], &er_buffer, 0);
                    dump_raw_buffer_with_header(&er_buffer, PGM_BUFFER, buffer_id);
                #endif
            }
            sem_post(&sem_teardown);
            while(1); //boo
        }
        num_frames_till_selection = 0;
    }
return 0;
}
