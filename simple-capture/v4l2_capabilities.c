#include <stdio.h>
#include <linux/videodev2.h>
#include "setup.h"
#include "v4l2_capabilities.h"

void print_caps(uint32_t c) {
    if (c & V4L2_CAP_VIDEO_CAPTURE) {
        console("V4L2_CAP_VIDEO_CAPTURE ");
    }

    if (c & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        console("V4L2_CAP_VIDEO_CAPTURE_MPLANE ");
    }

    if (c & V4L2_CAP_VIDEO_OUTPUT) {
        console("V4L2_CAP_VIDEO_OUTPUT ");
    }

    if (c & V4L2_CAP_VIDEO_OUTPUT_MPLANE) {
        console("V4L2_CAP_VIDEO_OUTPUT_MPLANE ");
    }

    if (c & V4L2_CAP_VIDEO_M2M) {
        console("V4L2_CAP_VIDEO_M2M ");
    }

    if (c & V4L2_CAP_VIDEO_M2M_MPLANE) {
        console("V4L2_CAP_VIDEO_M2M_MPLANE ");
    }

    if (c & V4L2_CAP_VIDEO_OVERLAY) {
        console("V4L2_CAP_VIDEO_OVERLAY ");
    }

    if (c & V4L2_CAP_VBI_CAPTURE) {
        console("V4L2_CAP_VBI_CAPTURE ");
    }

    if (c & V4L2_CAP_VBI_OUTPUT) {
        console("V4L2_CAP_VBI_OUTPUT ");
    }

    if (c & V4L2_CAP_SLICED_VBI_CAPTURE) {
        console("V4L2_CAP_SLICED_VBI_CAPTURE ");
    }

    if (c & V4L2_CAP_SLICED_VBI_OUTPUT) {
        console("V4L2_CAP_SLICED_VBI_OUTPUT ");
    }

    if (c & V4L2_CAP_RDS_CAPTURE) {
        console("V4L2_CAP_RDS_CAPTURE ");
    }

    if (c & V4L2_CAP_VIDEO_OUTPUT_OVERLAY) {
        console("V4L2_CAP_VIDEO_OUTPUT_OVERLAY ");
    }

    if (c & V4L2_CAP_HW_FREQ_SEEK) {
        console("V4L2_CAP_HW_FREQ_SEEK ");
    }

    if (c & V4L2_CAP_RDS_OUTPUT) {
        console("V4L2_CAP_RDS_OUTPUT ");
    }

    if (c & V4L2_CAP_TUNER) {
        console("V4L2_CAP_TUNER ");
    }

    if (c & V4L2_CAP_AUDIO) {
        console("V4L2_CAP_AUDIO ");
    }

    if (c & V4L2_CAP_RADIO) {
        console("V4L2_CAP_RADIO ");
    }

    if (c & V4L2_CAP_MODULATOR) {
        console("V4L2_CAP_MODULATOR ");
    }

    if (c & V4L2_CAP_SDR_CAPTURE) {
        console("V4L2_CAP_SDR_CAPTURE ");
    }

    if (c & V4L2_CAP_EXT_PIX_FORMAT) {
        console("V4L2_CAP_EXT_PIX_FORMAT ");
    }

    if (c & V4L2_CAP_SDR_OUTPUT) {
        console("V4L2_CAP_SDR_OUTPUT ");
    }

    if (c & V4L2_CAP_READWRITE) {
        console("V4L2_CAP_READWRITE ");
    }

    if (c & V4L2_CAP_ASYNCIO) {
        console("V4L2_CAP_ASYNCIO ");
    }

    if (c & V4L2_CAP_STREAMING) {
        console("V4L2_CAP_STREAMING ");
    }

    if (c & V4L2_CAP_TOUCH) {
        console("V4L2_CAP_TOUCH ");
    }

    if (c & V4L2_CAP_DEVICE_CAPS) {
        console("V4L2_CAP_DEVICE_CAPS ");
    }
}

void print_pixelformat(uint32_t pixelformat) {
    console("%c:%c:%c:%c", (char)pixelformat & 0xff,
                                       (char)(pixelformat >> 8) & 0xff,
                                       (char)(pixelformat >> 16) & 0xff,
                                       (char)(pixelformat >> 24) & 0xff);
}

