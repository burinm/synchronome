#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h> //sudo apt-get install libv4l-dev

#include <linux/kdev_t.h> //MAJOR/MINOR


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
    exit(0);
}

// https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/vidioc-querycap.html#vidioc-querycap
uint32_t version = camera_caps.version;
printf("[Camera version is %u.%u.%u]\n", (version >> 16) & 0xff,
                                       (version >> 8) & 0xff,
                                        version & 0xff);
printf("  Driver: %-16s\n", camera_caps.driver);
printf("    card: %-32s\n", camera_caps.card);
printf("     bus: %-32s\n", camera_caps.bus_info);







error:
close(camera_fd);
}
