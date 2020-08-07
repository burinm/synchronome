/* processing.c - Frame selection driver - main thread
    burin (c) 2020
    
    This is where the magic happens!

    This code looks messy, that's because it supports too many options:

    -----10Hz options ---- processing() at line:93 - MODE_10Hz

        1) 10Hz - one time sync

            Try to find 5 good ticks at startup, then start capturing frames
    
            This algorithm is baked into processing.c, but is should
            eventually be modularized.

            Specific to an Android phone
                1) Sync - look for bookend that does: (x5)
                    a) one frame change, b) x2 no frame changes
                2) Continuously capture frames
                    Alternate
                        every 2 frames
                        every 3 frames

                This is because 25Hz isn't divisible by 10Hz...grrr

        2) 10Hz - Stopwatch specific [NOT USED]

            This code is abandoned because I couldn't get good
            photography of the LCD on the stopwatch.

            The particular stopwatch I was using had a pattern where
            two frames in a row would always show no change. Syncing
            on this appeared to be successful.

    -----1Hz options ---- processing() at line:299

        1) 1Hz - continuous scanning  - MODE_ALWAYS_DETECT_FRAME

            Try to detect every watch tick and record that frame.

             This works great *under ideal lighting* conditions,
             but is sensitive to vibrations. Since the mass of
             the watch is light, slight movement causes shift and
             the system thinks it's a tick

        2) 1Hz - one time sync - !MODE_ALWAYS_DETECT_FRAME

            Try to find 5 good ticks at startup, then start capturing frames

             This strategy works great even when the camera decides
             to defocus in the middle of a run! The timing drift is
             negligable and this mode easily selects correct frames
             for 30 minutes 



*/
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
#include "realtime.h"
#include "memlog.h"

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
    #include "timetools.h"
#endif

#include "dumptools.h" //FOR errors only
#include "transformation.h" //FOR errors only
extern int freeze_system; //DEBUG

extern int running;
extern pthread_barrier_t bar_thread_inits;
extern sem_t sem_processing;
extern sem_t sem_teardown;

memlog_t* PROCESSING_LOG;

#ifdef MODE_10Hz
void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    PROCESSING_LOG = memlog_init();

    int wo_buffer_index = 0;

    int s_ret = -1;

    int last_buffer_index = -1;
    int current_index = 0;

    int did_frame_tick = MOTION_NONE;

    int num_frames_till_selection = 0;

    int is_even = 0;
    int changed_pixels = 0;
    int zero_pixels = 0;
    int is_startup = 1;
    int startup_count = 0;

    int last_sync_frame = -1;
    struct timespec time_diff;
    struct timespec current_stamp;
    struct timespec last_stamp;

    //Best effort!
    s_ret = sem_wait(&sem_processing);
    if (s_ret == -1) {
        perror("sem_wait sem_processing failed");
        error_exit(-2);
    }

    if (schedule_best_effort_priority(-20) == -1) {
        perror("processing prio (-20)");
        error_exit(-2);
    }

    while(running) {

        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_DONE);
        while(1) {

            int ret;
            ret = dequeue_P(&frame_Q, &current_index);
            if (ret == 1) {
                break; //Empty queue, give back CPU
            } else if (ret == -1) {
                printf("*Frame Processing: dequeue error\n");
                error_exit(-1);
            }

            //Need at least two frames to start comparing
            if (last_buffer_index == -1) {
                last_buffer_index = current_index;
                break;
            }

            did_frame_tick = MOTION_NONE;

#if 0 // free run
            did_frame_tick = MOTION_DETECTED;
            is_startup = 0;
#endif

    if (is_startup) {
            changed_pixels = frame_changes(&scan_buffer[last_buffer_index], &scan_buffer[current_index]);
            //did_frame_tick = is_motion(changed_pixels);
assert(scan_buffer[current_index].size == wo_buffers[wo_buffer_index].size);

            /* New 10Hz strategy. There is a pattern with this stopwatch where two
               frames with no changes appear in a row together. Attempt to sync to
               this pattern
            */
    printf("pixels %d\n", changed_pixels);

#if 0 //LCD stopwatch
            if (changed_pixels == 0) { //no change
                zero_pixels++; 
                //if (zero_pixels == 2) {// x2
                if (zero_pixels == 1) {// x2
                    zero_pixels = 0;
                    did_frame_tick = MOTION_DETECTED;
                    console("         <-#%d----- 00 ------>\n", startup_count);
                }
            } else {
                zero_pixels = 0;
            }
#endif

#if 1 // ANDROID_PHONE - display too slow!!
            if (changed_pixels > 0) {
                zero_pixels = 1;
            }

            if (zero_pixels == 1 && changed_pixels == 0) {
                did_frame_tick = MOTION_DETECTED;
                zero_pixels = 0;
            }

#endif



#if 1
            if (did_frame_tick == MOTION_DETECTED) {
                if (last_sync_frame != -1) {
                    BUFFER_GET_TIMESTAMP(scan_buffer[current_index], current_stamp);
                    BUFFER_GET_TIMESTAMP(scan_buffer[last_sync_frame], last_stamp);

                    if (timespec_subtract(&time_diff, &current_stamp, &last_stamp) == 0) {

                        //20ms jitter check
                        if (time_diff.tv_sec == 0) {
                            if (time_diff.tv_nsec < 130000000L) { // < 200ms
                                startup_count++;
                                if (startup_count == 5) {
                                    is_startup = 0;
                                    printf_on = 0; //No more console
#if 1 // ANDROID_PHONE
    //                                num_frames_till_selection = -3; //Try and put in middle of changes
#endif
    //                                num_frames_till_selection = -12; //Try and put in middle of changes
                                    last_buffer_index = current_index;
                                    continue;
                                }
                                console("sync #%d %lld.%.9ld \n", startup_count,
                                        (long long)time_diff.tv_sec,
                                        time_diff.tv_nsec);
                            } else {
                                startup_count = 0;
                                printf("sync jiiter too large %9ldns\n", time_diff.tv_nsec);
                            }
                        }
                    }
                }
                last_sync_frame = current_index;
            }
#endif

            last_buffer_index = current_index;
            break;
    }


            num_frames_till_selection++;

            if (num_frames_till_selection > 0) { //Add delay after trigger
                if (is_even) {
                    if (num_frames_till_selection == 2) {
                        did_frame_tick = MOTION_DETECTED;
                        is_even = 0;
                    }
                } else {
                    if (num_frames_till_selection == 3) {
                        did_frame_tick = MOTION_DETECTED;
                        is_even = 1;
                    }

                }
            }

            console("Processing index [%2d] (#%06d) auto (pixels %d)  = %s(offset %d)\n",
                    current_index,
                    scan_buffer[current_index].id,
                    changed_pixels,
                    did_frame_tick == MOTION_DETECTED ? "yes" : "no ",
                    num_frames_till_selection);


            if (did_frame_tick == MOTION_DETECTED) { //Found frame and sent to write Q
                COPY_BUFFER(wo_buffers[wo_buffer_index], scan_buffer[current_index]);
                COPY_BUFFER_TIMESTAMP(wo_buffers[wo_buffer_index], scan_buffer[current_index]);
                wo_buffers[wo_buffer_index].id = scan_buffer[current_index].id;

                if (enqueue_P(&writeout_Q, &wo_buffer_index) == -1) {
                    perror("writeout Q full!");
                    error_exit(-1);
                }
                total_frames_selected_g++;

                //ghetto circular buffer
                wo_buffer_index++;
                if (wo_buffer_index == NUM_WO_BUF) {
                    wo_buffer_index = 0;
                }
            }


            last_buffer_index = current_index;


            if (did_frame_tick == MOTION_DETECTED) { //Found frame and sent to write Q
                break;
            }
        } //forever, until change is detected
        num_frames_till_selection = 0;
        //}

    }
    return 0;
}

#else
void* processing(void* v) {
    video_t video;
    memcpy(&video, (video_t*)v, sizeof(video_t));

    PROCESSING_LOG = memlog_init();

    int wo_buffer_index = 0;

    int s_ret = -1;
    //struct v4l2_buffer b;

    int last_buffer_index = -1;
    int changed_pixels = 0;
    int did_frame_tick = MOTION_NONE;

    int is_startup = 1;
    int startup_frames_ignore = 0;

    int num_frames_till_selection = 0;

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
    struct timespec time_diff;
    struct timespec current_stamp;
    struct timespec last_stamp;
#endif

    //pthread_barrier_wait(&bar_thread_inits); //GO!!

    //Best effort!
    s_ret = sem_wait(&sem_processing);
    if (s_ret == -1) {
        perror("sem_wait sem_processing failed");
        error_exit(-2);
    }

    if (schedule_best_effort_priority(-20) == -1) {
        perror("processing prio (-20)");
        error_exit(-2);
    }



    while(running) {


        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_DONE);
        MEMLOG_LOG(PROCESSING_LOG, MEMLOG_E_S2_RUN);
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

            //Need at least two frames to start comparing
            if (last_buffer_index == -1) {
                last_buffer_index = current_index;
                break;
            }

            num_frames_till_selection++;


            did_frame_tick = MOTION_NONE;

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
        if (is_startup) {
#endif
            changed_pixels = frame_changes(&scan_buffer[last_buffer_index], &scan_buffer[current_index]);
            did_frame_tick = is_motion(changed_pixels);
assert(scan_buffer[current_index].size == wo_buffers[wo_buffer_index].size);

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
       }
#endif

#ifdef MODE_ALWAYS_DETECT_FRAME
            //Startup
            if (is_startup) {
                if (did_frame_tick == MOTION_DETECTED) {
                    startup_frames_ignore++;
                    if (startup_frames_ignore == MOTION_SELECTIONS_IGNORE) {
                        is_startup = 0;
                    }
                    printf("ignoring %d frame(s)\n", startup_frames_ignore);
                    //TODO, there should be a better way to organize loop logic

                    //This needs to go in here, skip still needs to run same algorithm
                    last_buffer_index = current_index;
                    break;
                }
            }
#else
            //Startup
            if (is_startup) {
                if (did_frame_tick == MOTION_DETECTED) {
                    BUFFER_GET_TIMESTAMP(scan_buffer[current_index], current_stamp);
                    BUFFER_GET_TIMESTAMP(scan_buffer[last_buffer_index], last_stamp);

                    if (timespec_subtract(&time_diff, &current_stamp, &last_stamp) == 0) {

                        ///80ms jitter check
                        if (time_diff.tv_sec == 0) {
                            if (time_diff.tv_nsec < 80000000L) { // < 80ms
                                startup_frames_ignore++;
                                if (startup_frames_ignore == MOTION_SELECTIONS_IGNORE) {
                                    is_startup = 0;
                                }
                                console("sync #%d %lld.%.9ld \n", startup_frames_ignore,
                                                                 (long long)time_diff.tv_sec,
                                                                 time_diff.tv_nsec);
                                last_buffer_index = current_index;
                                break;
                            }
                        }
                    }
                    printf("sync #%d %lld.%.9ld failed\n",startup_frames_ignore,
                                                          (long long)time_diff.tv_sec,
                                                          time_diff.tv_nsec);
                    startup_frames_ignore = 0;
                    last_buffer_index = current_index;
                    break;
                }
            } else {

                //We should be synced, grab a frame every second
                if (num_frames_till_selection == MOTION_FRAMES_SEC) {
                    did_frame_tick = MOTION_DETECTED;
                }
            }


#endif

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
        if(is_startup) {
#endif
            console("Processing index [%2d] - [%2d] (#%06d) vs [%2d] (#%06d) changed_pixels=%4d tick=",
                    current_index,
                    last_buffer_index,
                    scan_buffer[last_buffer_index].id,
                    current_index,
                    scan_buffer[current_index].id,
                    changed_pixels);

            console("%s ", did_frame_tick ? "yes" : "no ");
            print_motion_state();
            console("\n");

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
       }
#endif

#ifdef MODE_ALWAYS_DETECT_FRAME
#else
            console("Processing index [%2d] (#%06d) auto selected  = %s(offset %d)\n",
                    current_index,
                    scan_buffer[current_index].id,
                    did_frame_tick == MOTION_DETECTED ? "yes" : "no ",
                    num_frames_till_selection);
#endif


            //Copy frame to writeout buffer
            if (did_frame_tick == MOTION_DETECTED) {

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


            if (did_frame_tick == MOTION_DETECTED) { //Found frame and sent to write Q
                    break;
            }
        } //forever, until change is detected

        //Startup condition, skip error check
        if (is_startup) {
            //TODO, there should be a better way to organzixe loop logic
            num_frames_till_selection = 0;
            continue;
        }

        MEMLOG_LOG24(PROCESSING_LOG, MEMLOG_E_ADATA_24, num_frames_till_selection); //how many step it took to select
        MEMLOG_LOG24(PROCESSING_LOG, MEMLOG_E_BDATA_24, scan_buffer[last_buffer_index].id); //selected frame id


        //timing off +/-120ms!
        if (num_frames_till_selection > MOTION_FRAMES_SEC + MOTION_FRAMES_DRIFT ||
            num_frames_till_selection < MOTION_FRAMES_SEC - MOTION_FRAMES_DRIFT) {
            freeze_system = 1; //Makes sequencer stop timer
            printf("Bork! took %d frames to select. id:%d \n", num_frames_till_selection,
                                                            scan_buffer[last_buffer_index].id );
            for (int i=0; i < SCAN_BUF_SIZE; i++) {
                int buffer_id = scan_buffer[i].id;

                dump_buffer_raw(&scan_buffer[i], buffer_id, 1);

#if 0 //dump actual processed frame
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
#endif
            }
            sem_post(&sem_teardown);
            while(1); //boo
        }
        num_frames_till_selection = 0;
    }
return 0;
}
#endif
