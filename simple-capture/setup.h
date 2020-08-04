#ifndef __SETUP_H__
#define __SETUP_H__

/* Re-sync every frame (This currently works at 1Hz if lighting is good)
   When this is off, the system syncs, then autoselects a frame every
   MOTION_FRAMES_SEC frames (1 second)
*/
//#define MODE_ALWAYS_DETECT_FRAME

//Test frames without writing to disk
//#define PROFILE_FRAMES
#define PROFILE_ITERS   500

//Standalone/profiling option
//#define IMAGE_DIFF_TEST //Do image motion test instead of processing

#define X_RES   640
#define Y_RES   480
//#define X_RES  1920
//#define Y_RES  1080

//Select one
#define PPM_CAPTURE
//#define PGM_CAPTURE

//Select other options
//#define SHARPEN_ON
//#define SHARPEN_TYPE SHARPEN_WIKIPEDIA_EXAMPLE
#define SHARPEN_TYPE SHARPEN_PROFESSOR_EXAMPLE

#define NATIVE_CAMERA_FORMAT_SIZE   2

#ifdef PPM_CAPTURE
    #define BYTES_PER_PIXEL 3
#endif

#ifdef PGM_CAPTURE
    #define BYTES_PER_PIXEL 1
#endif

#define FRAME_SIZE  (X_RES * Y_RES)
#if FRAME_SIZE == 0
    #error "Frame size is zero!"
#endif


#include <stdio.h>
extern int printf_on;
// https://stackoverflow.com/questions/20639632/how-to-wrap-printf-into-a-function-or-macro
#define console(format, ...) if (printf_on) { printf((format), ##__VA_ARGS__); }
#define console_error(format, ...) printf((format), ##__VA_ARGS__)

#ifdef CAPTURE_STANDALONE
    #define error_exit(x)   exit(x)
    #define error_unbarrier_exit(x) exit(x)
#else
    #define error_exit(x)   { \
                                  running =0; \
                                  return((void*)x); \
                            }
    #define error_unbarrier_exit(x) \
                            { pthread_barrier_wait(&bar_thread_inits); \
                                  running =0; \
                                  return((void*)x); \
                            }
#endif

#endif
