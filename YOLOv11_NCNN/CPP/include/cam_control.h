#ifndef _CAM_CONTROL_H_
#define _CAM_CONTROL_H_

#include <stdint.h>
#include <string.h>
#include <cstring>
#include <vector>
#include <linux/videodev2.h>
#include <cstdio>
#include "img_proc.h"
#include "cam_proc_isp.h"
#include "yolo11.h"

#define LOGTAG "CAM_TEST"

//#define ISP
//#define SAVE_IMAGE_ISP
#define ISPtoRTSP
#define DEBUG_TIME

struct cam_format {
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    uint32_t cam_fps;
};

struct capture_format {
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    uint32_t fps;
};

struct cam_buffer {
    struct v4l2_buffer buffer;
    std::vector<void*> vData;
    std::vector<void*> dmaBuffer;
};

struct cam_control_params {
    int cam_fd;
    struct cam_format cam_fmt;
    struct cam_buffer* buffers;
    struct capture_format capture_fmt;
};

#endif // _CAM_CONTROL_H_