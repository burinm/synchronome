#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../setup.h"
#include "../resources.h"
#include "../dumptools.h"
#include "../transformation.h"
#include "../motion.h"
#include "../timetools.h"

#define NUM_TEST_BUFFERS    1801
buffer_t test_buffers[NUM_TEST_BUFFERS];

#define INPUT_PPM
//#define INPUT_PGM

#ifdef INPUT_PPM
    #define IN_BUFFER_SZ (3)
#endif

#ifdef INPUT_PGM
    #define IN_BUFFER_SZ (1)
#endif

#define DIFF_NUM_START  1000000

extern int motion_state;

int printf_on = 1;
int rebuild_images = 0;
int diff_images = 0;

void frame_changes_writeout(buffer_t *first, buffer_t *second, buffer_t *out);

int main(int argc, char* argv[]) {
    FILE* stream;
    if ((stream = fopen("frames.list", "r")) == NULL) {
        printf("verify_f: needs <frames.list>\n");
        return 0;
    }

    if (argc != 3) {
        printf("usage: verify_f <frame limit> <jitter limit ms>\n");
        return 0;
    }

    int frame_limit = atoi(argv[1]);
    float jitter_limit = atof(argv[2]) * 1000000L; //to ns

    printf("Mark all frames < %d as no change, Mark all jitter over %dns as suspect\n", frame_limit, jitter_limit);


    size_t frame_total_bytes = FRAME_SIZE * IN_BUFFER_SZ;
    printf("image size is %dx%d (bytes: %d)\n", X_RES, Y_RES, frame_total_bytes);

    for (int i=0; i < NUM_TEST_BUFFERS; i++) {
        if (allocate_buffer(&test_buffers[i], IN_BUFFER_SZ) == -1) {
            return -1;
        }
    }

    char *line_buf;
    size_t read_n = 0;

    int buf_num = 0;
    char load_buffer[X_RES];

    while(1) {
        if (getline(&line_buf, &read_n, stream) == -1) {
            break;
        }
        if (read_n > 3) { // shortest filename would be "x\n\0"

            //C is terrible
            char filename[255];

            char* filename_p = strtok(line_buf, "\n");
            strcpy(filename, filename_p);

            strtok(filename_p, "."); //"eat up [frame].nnnnnn.ppn
            char* image_id_c = strtok(NULL, "."); //.[nnnnnn].ppn
            int image_id = atoi(image_id_c);

            printf("load %s id=(%d)\n", filename, image_id);
            //printf("load %s\n", filename);
            FILE* image;
            if ((image = fopen(filename, "rb")) == NULL) {
                printf("Couldn't open %s!\n", filename);         
            } else {

                char dummy;
                #define COMMENT_LEN 42 //0000351847.726530079 TIMESTAMP_E id:000011
                #define TIMESTAMP_LEN   20
                #define ID_LEN 6
                char comment[COMMENT_LEN];
                char timestamp[TIMESTAMP_LEN + 1];
                char id_str[ID_LEN + 1];

                int header_count = 0;
                int header_token = 0;

                while(1) { //burn off header
                     fread(&dummy, 1, 1, image);

                     if (dummy == '#') {
                        fread(&comment, 1, COMMENT_LEN, image);
                        strncpy(timestamp, &comment[0], TIMESTAMP_LEN);
                        timestamp[TIMESTAMP_LEN] = '\0';
                        strncpy(id_str, &comment[36], ID_LEN);
                        id_str[ID_LEN] = '\0';
                        continue;
                     }

                     if (dummy == '\x0a') {
                         header_token++;
                     }

                     if (header_token == 4) {
                         break;
                     }

                    header_count++;
                    if (header_count > 255) {
                        fclose(image);
                        printf("File corrupted. Header wrong.\n");
                        exit(-1);
                    }
                }

                size_t bytes_read; 
                size_t offset = 0;
                while(1) {


                    bytes_read = fread(load_buffer, 1, X_RES, image);
                    if (bytes_read == 0) {
                        break;
                    }
                    memcpy((char*)(test_buffers[buf_num].start + offset), load_buffer, bytes_read);
                    offset += bytes_read;
                    assert(offset <= frame_total_bytes);
                }

                assert(offset == frame_total_bytes);

                char sec_string[10];
                char nsec_string[9];
                strncpy(sec_string, timestamp, 10);
                strncpy(nsec_string, &timestamp[11], 9);
                long long sec = atol(sec_string); 
                long nsec = atol(nsec_string);

                struct timespec temp_time;
                memset(&temp_time, 0, sizeof(struct timespec));
                temp_time.tv_sec = sec;
                temp_time.tv_nsec = nsec;

                printf("timestamp:%lld.%.9ld\n", (long long)temp_time.tv_sec, temp_time.tv_nsec);
                BUFFER_SET_TIMESTAMP(test_buffers[buf_num], temp_time); 

                int i_id = atoi(id_str);
                test_buffers[buf_num].id = i_id;

                buf_num++;
                fclose(image);
                assert(buf_num < NUM_TEST_BUFFERS + 1);
            }

        }
    }

    free(line_buf);
    fclose(stream);



    /* Same algorithm from processing */

    //Start here
    motion_state = MOTION_STATE_SEARCHING;

    int last_buffer_index = -1;
    int changed_pixels = 0;
    int did_frame_tick = 0;

    int num_frames_till_selection = 0;

    for (int i=0; i<buf_num; i++) {

        int current_index = i;
        if (last_buffer_index != -1) {

            changed_pixels = frame_changes_RGB(&test_buffers[last_buffer_index], &test_buffers[current_index]);

#if 0
            printf("current_index     : %lld.%.9ld\n",
                        (long long)test_buffers[current_index].time.tv_sec,
                        test_buffers[current_index].time.tv_nsec);

            printf("last_buffer_index : %lld.%.9ld\n",
                        (long long)test_buffers[last_buffer_index].time.tv_sec,
                        test_buffers[last_buffer_index].time.tv_nsec);

#endif
            struct timespec diff_time;
            timespec_subtract(&diff_time, &test_buffers[current_index].time,
                                          &test_buffers[last_buffer_index].time);


            printf("Processing: [id:%06d] vs [id:%06d] pixel diff: %d ",
                    test_buffers[current_index].id,
                    test_buffers[last_buffer_index].id,
                    changed_pixels);

            printf("time diff: %lld.%.9ld ", (long long)diff_time.tv_sec, diff_time.tv_nsec);

            struct timespec one_second;
            struct timespec jitter_time;
            one_second.tv_sec = 1;
            one_second.tv_nsec = 0;

            char* sign;
            if (diff_time.tv_sec >= 1L) {
                timespec_subtract(&jitter_time, &diff_time, &one_second);
                sign = "+";
            } else {
                timespec_subtract(&jitter_time, &one_second, &diff_time);
                sign = "-";
            }

            //printf("jitter: %s%lld.%.9ld ", sign, (long long)jitter_time.tv_sec, jitter_time.tv_nsec);
            // Print ms
            printf("jitter: %s%lld sec %.9f ms ", sign, (long long)jitter_time.tv_sec, (float)jitter_time.tv_nsec / 1000000);

            if (jitter_time.tv_sec > 0 || jitter_time.tv_nsec > jitter_limit) {
                printf("*j ");
            }

            if (changed_pixels < frame_limit) {
                printf("*same? ");
            }

            printf("\n");
    


        }
        last_buffer_index = current_index;

    }

    struct timespec diff_time;
    timespec_subtract(&diff_time, //Total time between first/last frame
                      &test_buffers[last_buffer_index].time,
                      &test_buffers[0].time);

    printf("total elapsed: %lld.%.9ld\n", (long long)diff_time.tv_sec, diff_time.tv_nsec);


    for (int i=0; i < NUM_TEST_BUFFERS; i++) {
        deallocate_buffer(&test_buffers[i]);
    }


return 0;    
}

