/* motion_test.c - Debugging frame dumps - detection errors
    burin (c) 2020

    motion test can do the following:
        -b Rebuild ppm/pgm images from raw YUYV dump 
        -d Diff YUYV images and create "shadow" movement images
        r  Re-run detection algorithm on dumped frames
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../setup.h"
#include "../resources.h"
#include "../dumptools.h"
#include "../transformation.h"
#include "../motion.h"

#define OUTPUT_PPM
//#define OUTPUT_PGM

#ifdef OUTPUT_PPM
    #define OUT_BUFFER_SZ (3)
#endif

#ifdef OUTPUT_PGM
    #define OUT_BUFFER_SZ (1)
#endif

#define DIFF_NUM_START  10000

extern int motion_state;
buffer_t out_buffer;

int printf_on = 1;
int rebuild_images = 0;
int diff_images = 0;

void frame_changes_writeout(buffer_t *first, buffer_t *second, buffer_t *out);

int main(int argc, char* argv[]) {
    FILE* stream;
    if ((stream = fopen("buffers.list", "r")) == NULL) {
        printf("motion: needs <buffers.list>\n");
        return 0;
    }

    if (argc != 2) {
        printf("usage: motion <-b/-d/r>  rebuild/diff/run motion\n");
        return 0;
    }

        
    if (argc == 2) {
        if (strcmp(argv[1], "-b") == 0) {
            rebuild_images = 1;
        }
    }

    if (argc == 2) {
        if (strcmp(argv[1], "-d") == 0) {
            diff_images = 1;
        }
    }

    size_t frame_total_bytes = FRAME_SIZE * NATIVE_CAMERA_FORMAT_SIZE;
    printf("image size is %dx%d (bytes: %d)\n", X_RES, Y_RES, frame_total_bytes);

    init_processing();

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

            strtok(filename_p, "."); //"eat up [buffer].nnnnnn.yuv
            char* image_id_c = strtok(NULL, "."); //.[nnnnnn].yuv
            int image_id = atoi(image_id_c);

            printf("#%d load %s id=(%d)\n", buf_num, filename, image_id);
            //printf("load %s\n", filename);
            FILE* image;
            if ((image = fopen(filename, "rb")) == NULL) {
                printf("Couldn't open %s!\n", filename);         
            } else {
                size_t bytes_read; 
                size_t offset = 0;
                while(1) {
                    bytes_read = fread(load_buffer, 1, X_RES, image);
                    if (bytes_read == 0) {
                        break;
                    }
                    memcpy((char*)(scan_buffer[buf_num].start + offset), load_buffer, bytes_read);
                    offset += bytes_read;
                    assert(offset <= frame_total_bytes);
                }

                assert(offset == frame_total_bytes);

                scan_buffer[buf_num].id = image_id;
                fclose(image);

                buf_num++;
                assert(buf_num <= SCAN_BUF_SIZE);
            }

        }
    }

    free(line_buf);
    fclose(stream);

if (rebuild_images) { //just writeout ppms 

    /* Write frames back out as sanity test
        note timestamp info from raw yuv
        future enhancement: dump raw serialized buffer_t structures!
    */

    if (allocate_buffer(&out_buffer, OUT_BUFFER_SZ) == -1) {
        console("couldn't allocate error out buffer\n");
        return -1;
    }

    int type;

    for (int i=0; i<buf_num; i++) {
#ifdef OUTPUT_PPM
        type = PPM_BUFFER;
        yuv422torgb888(&scan_buffer[i], &out_buffer, 0);
#endif

#ifdef OUTPUT_PGM
        type = PGM_BUFFER;
        yuv422toG8(&scan_buffer[i], &out_buffer, 0);
#endif

    assert(type);

        out_buffer.id = scan_buffer[i].id;
        dump_raw_buffer_with_header(&out_buffer, type, scan_buffer[i].id);
    }
} else if (diff_images) {

    if (allocate_buffer(&out_buffer, OUT_BUFFER_SZ) == -1) {
        console("couldn't allocate error out buffer\n");
        return -1;
    }

    int previous_index = -1;
    for (int current_index = 0; current_index < buf_num; current_index++) {
       if (previous_index != -1) {
            frame_changes_writeout(&scan_buffer[previous_index],
                                   &scan_buffer[current_index],
                                   &out_buffer);
            int type;
#ifdef OUTPUT_PPM
            type = PPM_BUFFER;
#endif

#ifdef OUTPUT_PGM
            type = PGM_BUFFER;
#endif
            out_buffer.id = scan_buffer[previous_index].id;
            dump_raw_buffer_with_header(&out_buffer,
                                        type,
                                        scan_buffer[previous_index].id + DIFF_NUM_START);
        }
        previous_index = current_index;
    }


} else { //motion test

    /* Same algorithm from processing */

    //Start here
    motion_state = MOTION_STATE_SEARCHING;

    int last_buffer_index = -1;
    int changed_pixels = 0;
    int did_frame_tick = 0;

    int num_frames_till_selection = 0;

    for (int i=0; i<buf_num; i++) {

        int current_index = i;
        num_frames_till_selection++;

        did_frame_tick = 0;

        if (last_buffer_index != -1) {

            changed_pixels = frame_changes(&scan_buffer[last_buffer_index], &scan_buffer[current_index]);
            did_frame_tick = is_motion(changed_pixels);

            printf("Processing index [%2d] - [%2d] (#%06d) vs [%2d] (#%06d) changed_pixels=%4d tick=",
                    current_index,
                    last_buffer_index,
                    scan_buffer[last_buffer_index].id,
                    current_index,
                    scan_buffer[current_index].id,
                    changed_pixels);

            printf("%s ", did_frame_tick ? "yes" : "no ");
            print_motion_state();
            printf("\n");


        }
        last_buffer_index = current_index;

    }

    deallocate_buffer(&out_buffer);
}

deallocate_processing();
return 0;    
}

void frame_changes_writeout(buffer_t *first, buffer_t *second, buffer_t *out) {

    unsigned char* f = first->start;
    unsigned char* s = second->start;
    unsigned char* o = out->start;
    int count = 0;

assert(first->start && second->start);
assert(first->size == second->size);
printf("%d %d \n", first->size * OUT_BUFFER_SZ,  out->size);
    memset(out->start, 0, out->size);

    for (int i=0; i<first->size /2; i++) {
        int diff = abs(*f - *s);
        if (diff > MOTION_SENSITIVITY) { //sensitivity
            count++;
#ifdef OUTPUT_PPM
            *o = 255; o++; *o = 255; o++; *o = 255; o++;
#endif
#ifdef OUTPUT_PGM
            *o = 255; o++; //greyscale of diffs
#endif
        } else {
#ifdef OUTPUT_PPM
            o+=3;
#endif
#ifdef OUTPUT_PGM
            o++;
#endif
        }
        //Just diff Ys, skip U/V
        f+=2; s+=2;
    }
}
