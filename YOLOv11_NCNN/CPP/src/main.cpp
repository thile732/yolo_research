#include <iostream>
#include "cam_control.h"

int main() {
    struct cam_control_params cam_params;

    cam_params.capture_fmt.fps = SENSOR_MAX_FPS;
    cam_params.cam_fmt.width = 1920;
    cam_params.cam_fmt.height = 1080;
    cam_params.cam_fmt.pixelformat = V4L2_PIX_FMT_MJPEG;

    if (cam_open(&cam_params) == -1) {
        return -1;
    }
    cam_initDevice(&cam_params);
    cam_initMap(&cam_params);
    cam_start(&cam_params);
    cam_unmap(&cam_params);
    if (cam_close(&cam_params) == -1) {
        return -1;
    }
    printf("ISP closed\n");

    return 0;
}