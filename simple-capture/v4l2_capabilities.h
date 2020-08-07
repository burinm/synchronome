/* v4l2_capabilities.h - convenience functions for v4l2 capability information
    burin (c) 2020
*/
#ifndef __V4L2_CQAPABILITIES_H__
#define __V4L2_CQAPABILITIES_H__

#include <stdint.h>

// https://stackoverflow.com/questions/16721346/is-there-a-macro-definition-to-check-the-linux-kernel-version
#include <linux/version.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,8,0)
    #define V4L2_CAP_TOUCH                  0x10000000
#endif

void print_caps(uint32_t);
void print_pixelformat(uint32_t pixelformat);

#endif
