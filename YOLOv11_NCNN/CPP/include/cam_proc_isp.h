#ifndef _CAM_PROC_ISP_H_
#define _CAM_PROC_ISP_H_

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <fstream>
#include <stdint.h>
#include <string>
#include "log.h"

#define BUFFER_COUNT 4
#define SENSOR_MAX_FPS 30
//#define USE_DMABUF

int cam_open(struct cam_control_params* cam_params);
int cam_close(struct cam_control_params* cam_params);
void cam_initDevice(struct cam_control_params* cam_params);
void cam_initMap(struct cam_control_params* cam_params);
void cam_start(struct cam_control_params* cam_params);
void cam_start_ISPtoRTSP(struct cam_control_params* cam_params, struct rtsp_stream_params* rtsp_params);
void cam_unmap(struct cam_control_params* cam_params);

#endif // _CAM_PROC_ISP_H_

