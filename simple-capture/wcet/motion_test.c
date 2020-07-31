#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../setup.h"
#include "../resources.h"
#include "../dumptools.h"
#include "../transformation.h"

int printf_on = 1;

int main() {
    FILE* stream;
    if ((stream = fopen("buffers.list", "r")) == NULL) {
        printf("motion: needs <buffers.list>\n");
        return 0;
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

            printf("load %s id=(%d)\n", filename, image_id);
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
                buf_num++;
                fclose(image);
                assert(buf_num < SCAN_BUF_SIZE + 1);
            }

        }
    }

    free(line_buf);
    fclose(stream);

    allocate_single_wo_buffer();

    int type;
#ifdef PPM_CAPTURE
    type = PPM_BUFFER;
#endif

#ifdef PGM_CAPTURE
    type = PGM_BUFFER;
#endif
    assert(type);

    for (int i=0; i<buf_num; i++) {
        yuv422torgb888(&scan_buffer[i], &wo_buffer, 0);
        wo_buffer.id = scan_buffer[i].id; 
        dump_raw_buffer_with_header(&wo_buffer, type, scan_buffer[i].id);
    }


deallocate_processing();
deallocate_single_wo_buffer();
return 0;    
}
