/* camera.c - all main camera control, knobs
    burin (c) 2020

    other parts from:
        https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/vidioc-querycap.html#vidioc-querycap
        https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/standard.html
*/
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev

#include <linux/kdev_t.h> //MAJOR/MINOR

#include "camera.h"
#include "camera_buffer.h"
#include "setup.h"
#include "v4l2_capabilities.h"

int open_camera(char* camera, video_t *v) {
    v->camera_fd = -1;

    struct stat s_test;
    if (stat(camera, &s_test) != 0) {
        perror("Couldn't open camera");
        return -1;
    }

    console("Device node %s exists\n", camera);
    console("Devid: Major %u Minor %u\n", (unsigned int)MAJOR(s_test.st_rdev),
                                         (unsigned int)MINOR(s_test.st_rdev));
    /*Note: could use CONFIG_VIDEO_FIXED_MINOR_RANGES to fix minor number */

    if (MAJOR(s_test.st_rdev) != CAMERA_MAJ_ID) {
        console("This doesn't appear to be a camera device\n");
        return -1; 
    }

    /* Let's go for the gravy and open the device
        According to the documentation O_EXCL is not required
    */ 
    int camera_fd;
    if ((camera_fd = open(camera, O_RDWR)) == -1) {
        perror("Couldn't open camera");
        return -1;
    }

    v->camera_fd = camera_fd;
return 0;
}

int close_camera(int camera_fd) {
    return close(camera_fd);
}

int show_camera_capabilities(int camera_fd) {

    struct v4l2_capability camera_caps;
    int api_version = 0;
    if ((api_version = ioctl(camera_fd, VIDIOC_QUERYCAP, &camera_caps)) == -1) {
        perror("Couldn't get camera version");
        return -1;
    }

    // https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/vidioc-querycap.html#vidioc-querycap
    uint32_t version = camera_caps.version;
    console("[Camera version is %u.%u.%u]\n", (version >> 16) & 0xff,
                                           (version >> 8) & 0xff,
                                            version & 0xff);
    console("     Driver: %-16s\n", camera_caps.driver);
    console("       card: %-32s\n", camera_caps.card);
    console("        bus: %-32s\n", camera_caps.bus_info);
    console("       caps: 0x%x\n", camera_caps.capabilities);
    print_caps(camera_caps.capabilities);
    console("\n");

    console("device caps: 0x%x\n", camera_caps.device_caps);
    print_caps(camera_caps.device_caps);
    console("\n");

    /* Find current video input (assuming this camera only has one)
        These are modeled as video cards with connectors, so I'm assumiing
        the web cameras "input" is a virtual connection from the camera -> driver
    */
    struct v4l2_input camera_input;
    if (ioctl(camera_fd, VIDIOC_G_INPUT, &camera_input.index) == -1) {
        perror("Couldn't get camera INPUT index");
        return -1;
    }

    if (ioctl(camera_fd, VIDIOC_ENUMINPUT, &camera_input) == -1) {
        perror("ioctl VIDIOC_ENUMINPUT failed");
        return -1;
    }

    //I'm guessing .std isn't set because a USB camera?
    console("\n[INPUT %s index = %u std(0x%.16llx)]\n",
                camera_input.name, camera_input.index, camera_input.std);

return 0;
}


#if 0 //Don't think this works on USB cameras

/* Enumerate standards */

// taken from examples here: https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/standard.html
struct v4l2_standard standard;
memset(&standard, 0, sizeof(struct v4l2_standard));
standard.index = 0; //start enumeration here

while (ioctl(camera_fd, VIDIOC_ENUMSTD, &standard) == 0) {
    if (standard.id & camera_input.std) {
        console("[%s]", standard.name);
    }
    standard.index++;
}

console("\n");

if (errno != EINVAL) {
    perror("Couldn't enumerate camera INPUT standards");
    return -1;
}

if (standard.index == 0) {
    console("No camera input standards found. (is this really a camera device?)\n");
    return -1;
}
#endif

int enumerate_camera_image_formats(int camera_fd) {

    struct v4l2_fmtdesc image_formats;
    memset(&image_formats, 0, sizeof(struct v4l2_fmtdesc));
    image_formats.index = 0; //start enumeration here 
    image_formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //start enumeration here 

    while (ioctl(camera_fd, VIDIOC_ENUM_FMT, &image_formats) == 0) {
        console("\t[%u]%-32s ", image_formats.index, image_formats.description);
        console("pixelformat ");
        print_pixelformat(image_formats.pixelformat);
        console(" ");

        if (image_formats.flags & V4L2_FMT_FLAG_COMPRESSED) {
            console(" V4L2_FMT_FLAG_COMPRESSED");
        }
        if (image_formats.flags & V4L2_FMT_FLAG_EMULATED) {
            console(" V4L2_FMT_FLAG_EMULATED");
        }
        image_formats.index++;
        console("\n");
    }

    if (errno != EINVAL) {
        perror("Couldn't enumerate camera image formats");
        return -1;
    }

    if (image_formats.index == 0) {
        console("No camera image formats found!\n");
        return -1;
    }
return 0;
}

int show_camera_image_format(int camera_fd) {
    struct v4l2_format camera_format;
    struct v4l2_pix_format *pix = &camera_format.fmt.pix;

    memset(&camera_format, 0, sizeof(struct v4l2_format));
    camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(camera_fd, VIDIOC_G_FMT, &camera_format) == -1) {
        perror("ioctl VIDIOC_G_FMT failed");
        return -1;
    }

    console("[video %u x %u]\n", pix->width, pix->height);
    console("\tpixelformat: ");
    print_pixelformat(pix->pixelformat);
    console("\n");
    console("\tfield: %u\n", pix->field);
    console("\tbytesperline: %u\n", pix->bytesperline);
    console("\tbuffer size %u\n", pix->sizeimage);
    console("\t\tv4l2_colorspace %u\n", pix->colorspace);
    console("\t\tv4l2_ycbcr_encoding %u\n", pix->ycbcr_enc);
    console("\t\tv4l2_quantization %u\n", pix->quantization);
    console("\t\tv4l2_xfer_func %u\n", pix->xfer_func);

return 0;
}

int query_buffer_size(int camera_fd) {
    struct v4l2_format camera_format;
    struct v4l2_pix_format *pix = &camera_format.fmt.pix;

    memset(&camera_format, 0, sizeof(struct v4l2_format));
    camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(camera_fd, VIDIOC_G_FMT, &camera_format) == -1) {
        perror("ioctl VIDIOC_G_FMT failed");
        return -1;
    }

return pix->sizeimage;
}

int camera_set_yuyv(video_t *v, int width, int height) {

    //Change to YUYV
    console("Change to YUYV %dx%d\n", width, height);

    struct v4l2_format camera_format;
    struct v4l2_pix_format *pix = &camera_format.fmt.pix;
    memset(&camera_format, 0, sizeof(struct v4l2_format));

    camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        pix->width = width;
        pix->height = height;
        pix->pixelformat = v4l2_fourcc('Y','U','Y','V');
        //pix->pixelformat = v4l2_fourcc('M','J','P','G');

    if (ioctl(v->camera_fd, VIDIOC_S_FMT, &camera_format) == -1) {
        perror("ioctl VIDIOC_G_FMT failed");
        return -1;
    }

    memset(&camera_format, 0, sizeof(struct v4l2_format));
    camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(v->camera_fd, VIDIOC_G_FMT, &camera_format) == -1) {
        perror("ioctl VIDIOC_G_FMT failed");
        return -1;
    }

    v->width = pix->width;
    v->height = pix->height;
    return 0;
}

int start_streaming(video_t *v) {
    assert(v->camera_fd);
    int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    return ioctl(v->camera_fd, VIDIOC_STREAMON, &stream_type);
}

int stop_streaming(video_t *v) {
    assert(v->camera_fd);
    int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    return ioctl(v->camera_fd, VIDIOC_STREAMOFF, &stream_type);
}

int try_refocus(int camera_fd) {
    struct v4l2_ext_control c[1];
    struct v4l2_ext_controls ext;
    memset(&ext, 0, sizeof(struct v4l2_ext_controls));
    ext.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ext.count = 1;
    ext.controls = c;

    console("[autofocus].");
    fflush(stdout);

    memset(&c[0], 0, sizeof(struct v4l2_ext_control));
    c[0].id = V4L2_CID_FOCUS_AUTO;
    c[0].value = 0; //Set autofocus to off

    if (ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ext) == -1) {
        perror("ioctl V4L2_CID_FOCUS_AUTO failed");
        return -1;
    }
    console("off.");
    fflush(stdout);

    sleep(1); //

    memset(&c[0], 0, sizeof(struct v4l2_ext_control));
    c[0].id = V4L2_CID_FOCUS_ABSOLUTE;
    c[0].value = CAMERA_STATIC_FOCUS;
    //c[0].value = 0; //infinity

    if (ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ext) == -1) {
        perror("ioctl V4L2_CID_FOCUS_ABSOLUTE failed");
        return -1;
    }
    console("focus forced to %d.", CAMERA_STATIC_FOCUS);
    //console("infinity.");
    fflush(stdout);

#if 1 //TODO - can't figure out autofocus grrrr.
    sleep(1); //

    memset(&c[0], 0, sizeof(struct v4l2_ext_control));
    c[0].id = V4L2_CID_FOCUS_AUTO;
    c[0].value = 1; //Set autofocus to on

    if (ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ext) == -1) {
        perror("ioctl V4L2_CID_FOCUS_AUTO failed");
        return -1;
    }
    console("on.");
    fflush(stdout);


    sleep(2); //TODO - find real documentation
#endif

#if 0 //status doesn't work
    memset(&c[0], 0, sizeof(struct v4l2_ext_control));
    c[0].id = V4L2_CID_AUTO_FOCUS_STATUS;

    int try = 1000;
    while(try) {
        if (ioctl(camera_fd, VIDIOC_G_EXT_CTRLS, &ext) == -1) {
            perror("ioctl V4L2_CID_AUTO_FOCUS_STATUS failed");
            try=0;
            break;
        }
        if (c[0].value & V4L2_AUTO_FOCUS_STATUS_BUSY) {
            console(".");
            fflush(stdout);
            try--;
            continue;
        }
        if (c[0].value & V4L2_AUTO_FOCUS_STATUS_REACHED) {
            console("done ");
            break;
        }
        if (c[0].value & V4L2_AUTO_FOCUS_STATUS_IDLE) {
            console("idle ");
            break;
        }
        if (c[0].value & V4L2_AUTO_FOCUS_STATUS_FAILED) {
            console("failed ");
            break;
        }

        try--;

    }

    if (try) {
        console(" succeeded\n");
    } else {
        console(" failed! (0x%x)\n", c[0].value);
    }
#endif

#if 1
    memset(&c[0], 0, sizeof(struct v4l2_ext_control));
    c[0].id = V4L2_CID_FOCUS_AUTO;
    c[0].value = 0; //Set autofocus to off

    if (ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ext) == -1) {
        perror("ioctl V4L2_CID_FOCUS_AUTO failed");
        return -1;
    }
    console("off.");
    fflush(stdout);
#endif

    //Autofocus must be off to read this control
    memset(&c[0], 0, sizeof(struct v4l2_ext_control));
    c[0].id = V4L2_CID_FOCUS_ABSOLUTE;
    if (ioctl(camera_fd, VIDIOC_G_EXT_CTRLS, &ext) == -1) {
        perror("ioctl VIDIOC_G_EXT_CTRLS failed");
        return -1;
    }
    console("[refocused at %d]\n", c[0].value);

return 0;
}

//Camera initialization routines

void video_error_cleanup(int state, video_t *v) {
    switch(state) {
        case ERROR_FULL_INIT:
            if (stop_streaming(v) == -1) {
                perror("Couldn't stop stream");
            }
        case ERROR_LEVEL_3:
            deallocate_single_wo_buffer();
            deallocate_single_er_buffer();
#ifdef SHARPEN_ON
            deallocate_sharpen_buffer();
#endif
        case ERROR_LEVEL_2:
            camera_deallocate_internal_buffers(v);
        case ERROR_LEVEL_1:
            if (close_camera(v->camera_fd) == -1) {
                console("problem closing fd=%d\n", v->camera_fd);
                perror(NULL);
            } else {
                console("closed camera device\n");
            }
        case ERROR_LEVEL_0:
            break;

    };
}

int camera_check_init(video_t *v) {

    /* This can all be setup/checked with v4l2-ctl also */
    if (show_camera_capabilities(v->camera_fd) == -1) {
        goto error;
    }

    if (enumerate_camera_image_formats(v->camera_fd) == -1) {
        goto error;
    }

    if (show_camera_image_format(v->camera_fd) == -1) {
        goto error;
    }

    if (camera_set_yuyv(v, X_RES, Y_RES) == -1) {
        goto error;
    }

    if (v->width != X_RES) {
        console("Requested width %d not set (returned %d)\n", X_RES, v->width);
        goto error;
    }

    if (v->height != Y_RES) {
        console("Requested height %d not set (returned %d)\n", Y_RES, v->height);
        goto error;
    }

    if (show_camera_image_format(v->camera_fd) == -1) {
        goto error;
    }
    /* End - This can all be setup/checked with v4l2-ctl also */

    //All good
    return 0;

error:
return -1;
}
