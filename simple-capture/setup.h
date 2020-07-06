#define CAMERA_DEV  "/dev/video0"
#define CAMERA_MAJ_ID   81

int open_camera(char* camera);
int close_camera(int fd);
int show_camera_capabilities(int camera_fd);
int enumerate_camera_image_formats(int camera_fd);
int show_camera_image_format(int camera_fd);
int camera_set_yuyv(int camera_fd);

//Get size of capture buffer from format data
int query_buffer_size(int camera_fd);
