/* dumptools.h - Tools for writing ppm/pgm image files
    burin (c) 2020

    This tool knows about P6/P5 header formats

    Synchronome images custom header files contain the following:
    # [timestamp] TIMESTAMP_E [id]
    
    Every frame that is pulled from the camera has a unique id.
    Timestamps "should" be unique too, but it's easier to track
    and compare an id for debugging.

    The TIMESTAMP_E is used later and frames can be made into a
    faux memlog entry.    
*/
#ifndef __DUMPTOOLS_H__
#define __DUMPTOOLS_H__

#include "buffer.h"
#include "setup.h"

#define DEBUG_NONE  -1

#define PPM_BUFFER  1
#define PGM_BUFFER  2

#define FILE_NAME_SIZE 256 //To include long paths
#define FILE_NUMBER_MAX 1801

//https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html (lame!)
#define xstr(s) str(s)
#define str(s) #s

/* -----------------------PPM--------------------------- */
//Boo, this is brittle, c strings are terrible
#define PPM_HEADER_DESC  "P6\n"               //    2
#define PPM_HEADER_RES   xstr(X_RES Y_RES\n)  //    9 xxxx xxxx
//#9999999999.999999999 TIMESTAMP_E id:nnnnnn\n  (44)
#define PPM_TIMESTAMP_SIZE 34                 //   44
#define PPM_HEADER_DEPTH   "255\n"            //    5

// +1 because srncpy includes '\0' as one of the bytes
#define PPM_HEADER_MAX_LEN                        (60 + 2)

/* -----------------------PGM--------------------------- */
//Boo, this is brittle, c strings are terrible
#define PGM_HEADER_DESC  "P5\n"               //    2
#define PGM_HEADER_RES   xstr(X_RES Y_RES\n)  //    9 xxxx xxxx
//#9999999999.999999999 TIMESTAMP_E id:nnnnnn\n  (44)
#define PGM_TIMESTAMP_SIZE 34                 //   44
#define PGM_HEADER_DEPTH   "255\n"            //    5

// +1 because srncpy includes '\0' as one of the bytes
#define PGM_HEADER_MAX_LEN                        (60 + 2)


/* -------Selection of image type------------------------ */
#ifdef PPM_CAPTURE
    #define IMAGE_SUFFIX      "ppm"
    #define IMAGE_HEADER_MAX_LENGTH PPM_HEADER_MAX_LEN
#endif

#ifdef PGM_CAPTURE
    #define IMAGE_SUFFIX      "pgm"
    #define IMAGE_HEADER_MAX_LENGTH PGM_HEADER_MAX_LEN
#endif

extern char header_buf[PGM_HEADER_MAX_LEN];

void dump_buffer_raw(buffer_t *b, int id, int debug);
void dump_raw_buffer_with_header(buffer_t *b, int type, int debug_id);


#endif
