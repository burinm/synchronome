#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev

#include <linux/kdev_t.h> //MAJOR/MINOR

#include "v4l2_capabilities.h"


#define CAMERA_DEV  "/dev/video0"
#define CAMERA_MAJ_ID   81

int main() {

struct stat s_test;
if (stat(CAMERA_DEV, &s_test) != 0) {
    perror("Couldn't open camera");
    exit(-1);
}

printf("Device node %s exists\n", CAMERA_DEV);
printf("Devid: Major %u Minor %u\n", MAJOR(s_test.st_rdev), MINOR(s_test.st_rdev));
/*Note: could use CONFIG_VIDEO_FIXED_MINOR_RANGES to fix minor number */

if (MAJOR(s_test.st_rdev) != CAMERA_MAJ_ID) {
    printf("This doesn't appear to be a camera device\n");
    exit(0);
}

//Let's go for the gravy and open the device
//According to the documentation O_EXCL is not required
int camera_fd;
if ((camera_fd = open(CAMERA_DEV, O_RDWR)) == -1) {
    perror("Couldn't open camera");
    exit(-1);
}

struct v4l2_capability camera_caps;
int api_version = 0;
if ((api_version = ioctl(camera_fd, VIDIOC_QUERYCAP, &camera_caps)) == -1) {
    perror("Couldn't get camera version");
    goto error;
}

// https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/vidioc-querycap.html#vidioc-querycap
uint32_t version = camera_caps.version;
printf("[Camera version is %u.%u.%u]\n", (version >> 16) & 0xff,
                                       (version >> 8) & 0xff,
                                        version & 0xff);
printf("     Driver: %-16s\n", camera_caps.driver);
printf("       card: %-32s\n", camera_caps.card);
printf("        bus: %-32s\n", camera_caps.bus_info);
printf("       caps: 0x%x\n", camera_caps.capabilities);
print_caps(camera_caps.capabilities);
printf("\n");

printf("device caps: 0x%x\n", camera_caps.device_caps);
print_caps(camera_caps.device_caps);
printf("\n");

//Find current video input (assuming this camera only has one)
// These are modeled as video cards with connectors, so I'm assumiing
// the web cameras "input" is a virtual connection from the camera -> driver
struct v4l2_input camera_input;
if (ioctl(camera_fd, VIDIOC_G_INPUT, &camera_input.index) == -1) {
    perror("Couldn't get camera INPUT index");
    goto error;
}

if (ioctl(camera_fd, VIDIOC_ENUMINPUT, &camera_input) == -1) {
    perror("ioctl VIDIOC_ENUMINPUT failed");
    goto error;
}


//I'm guessing .std isn't set because a USB camera?
printf("\n[INPUT %s index = %u std(0x%.16llx)]\n", camera_input.name, camera_input.index, camera_input.std);


#if 0 //Don't think this works on USB cameras
// Enumerate standards
// taken from examples here: https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/standard.html
struct v4l2_standard standard;
memset(&standard, 0, sizeof(struct v4l2_standard));
standard.index = 0; //start enumeration here 

while (ioctl(camera_fd, VIDIOC_ENUMSTD, &standard) == 0) {
    if (standard.id & camera_input.std) {
        printf("[%s]", standard.name); 
    }
    standard.index++;
}

printf("\n");

if (errno != EINVAL) {
    perror("Couldn't enumerate camera INPUT standards");
    goto error;
}

if (standard.index == 0) {
    printf("No camera input standards found. (is this really a camera device?)\n");
    goto error;
}
#endif

struct v4l2_fmtdesc image_formats;
memset(&image_formats, 0, sizeof(struct v4l2_fmtdesc));
image_formats.index = 0; //start enumeration here 
image_formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //start enumeration here 

while (ioctl(camera_fd, VIDIOC_ENUM_FMT, &image_formats) == 0) {
    printf("\t[%u]%-32s ", image_formats.index, image_formats.description); 
    printf("pixelformat "); 
    print_pixelformat(image_formats.pixelformat);
    printf(" "); 

    if (image_formats.flags & V4L2_FMT_FLAG_COMPRESSED) {
        printf(" V4L2_FMT_FLAG_COMPRESSED");
    }
    if (image_formats.flags & V4L2_FMT_FLAG_EMULATED) {
        printf(" V4L2_FMT_FLAG_EMULATED");
    }
    image_formats.index++;
    printf("\n");
}

printf("\nTODO: which capture type is set?\n");

if (errno != EINVAL) {
    perror("Couldn't enumerate camera image formats");
    goto error;
}

if (image_formats.index == 0) {
    printf("No camera image formats found!\n");
    goto error;
}

struct v4l2_format camera_format;
struct v4l2_pix_format *pix = &camera_format.fmt.pix;

memset(&camera_format, 0, sizeof(struct v4l2_format));
camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

if (ioctl(camera_fd, VIDIOC_G_FMT, &camera_format) == -1) {
    perror("ioctl VIDIOC_G_FMT failed");
    goto error;
}

printf("video (%u x %u) ", pix->width, pix->height);
print_pixelformat(pix->pixelformat);
printf(" field(%u) bytesperline(%u)\n", pix->field, pix->bytesperline);

//Change to YUYV
printf("Change to YUYV 320x240\n");
memset(&camera_format, 0, sizeof(struct v4l2_format));
camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    pix->width = 320;
    pix->height = 240; 
    pix->pixelformat = v4l2_fourcc('Y','U','Y','V');
    //pix->pixelformat = v4l2_fourcc('M','J','P','G');

if (ioctl(camera_fd, VIDIOC_S_FMT, &camera_format) == -1) {
    perror("ioctl VIDIOC_G_FMT failed");
    goto error;
}

//Print out current format again
memset(&camera_format, 0, sizeof(struct v4l2_format));
camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

if (ioctl(camera_fd, VIDIOC_G_FMT, &camera_format) == -1) {
    perror("ioctl VIDIOC_G_FMT failed");
    goto error;
}

printf("video (%u x %u) ", pix->width, pix->height);
print_pixelformat(pix->pixelformat);
printf(" field(%u) bytesperline(%u)\n", pix->field, pix->bytesperline);
//End print out current format


error:
close(camera_fd);
}


