#include "cam_control.h"
#include "cam_proc_isp.h"

using namespace std;
static std::mutex mtx;
std::mutex mtx_rtsp;

static const char* cam_port = "/dev/video0";

#define US_PER_SEC      1000000
#define STAT_ITVL_US    (10 * US_PER_SEC)
static void fpsStat() {
    uint64_t curUS;
    static uint64_t preUS;
    struct timeval val = {0};
    uint64_t itvl = 0;
    float fps;
    static uint64_t frames;

    frames++;
    gettimeofday(&val, NULL);
    curUS = val.tv_sec * US_PER_SEC + val.tv_usec;
    itvl = curUS - preUS;

    if(itvl >= STAT_ITVL_US) {
        fps = (float)(frames * US_PER_SEC) / itvl;
        if(preUS > 0)
            ALOGI("######## fps %.2f ########\n", fps);

        preUS = curUS;
        frames = 0;
    }

    return;
}

int cam_open(struct cam_control_params* cam_params) {
    cam_params->cam_fd = open(cam_port, O_RDWR | O_NONBLOCK);
    if (cam_params->cam_fd == -1) {
        perror("open");
        return -1;
    }
    return 0;
}

void cam_initDevice(struct cam_control_params* cam_params) {
    struct v4l2_capability cap;
    if (ioctl(cam_params->cam_fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("VIDIOC_QUERYCAP");
        exit(EXIT_FAILURE);
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "The device does not handle single-planar video capture.\n");
        exit(EXIT_FAILURE);
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "The device does not handle streaming.\n");
        exit(EXIT_FAILURE);
    }
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = cam_params->cam_fmt.width;
    fmt.fmt.pix.height = cam_params->cam_fmt.height;
    fmt.fmt.pix.pixelformat = cam_params->cam_fmt.pixelformat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(cam_params->cam_fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("VIDIOC_S_FMT");
        exit(EXIT_FAILURE);
    }

    struct v4l2_streamparm param;
    memset(&param, 0, sizeof(param));
    param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    param.parm.capture.timeperframe.numerator = 1;
    param.parm.capture.timeperframe.denominator = SENSOR_MAX_FPS;
    ALOGI("==== VIDIOC_S_PARM, fps %d\n", param.parm.capture.timeperframe.denominator);
    if (ioctl(cam_params->cam_fd, VIDIOC_S_PARM, &param) < 0) {
        ALOGW("%s: VIDIOC_S_PARM Failed: %s, fps %d", __func__, strerror(errno), param.parm.capture.timeperframe.denominator);
    }
}

void cam_initMap(struct cam_control_params* cam_params) {
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = BUFFER_COUNT;
    if (ioctl(cam_params->cam_fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
        perror("VIDIOC_REQBUFS");
        exit(EXIT_FAILURE);
    }

    cam_params->buffers = new cam_buffer();

    for (int i = 0; i < BUFFER_COUNT; i++) {
        memset(&cam_params->buffers->buffer, 0, sizeof(cam_params->buffers->buffer));
        cam_params->buffers->buffer.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        cam_params->buffers->buffer.memory   = V4L2_MEMORY_MMAP;
        cam_params->buffers->buffer.index    = i;
        if (ioctl(cam_params->cam_fd, VIDIOC_QUERYBUF, &cam_params->buffers->buffer) < 0) {
            ALOGE("VIDIOC_QUERYBUF: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        ALOGI("Buffer description:");
        ALOGI("  offset: %d", cam_params->buffers->buffer.m.offset);
        ALOGI("  length: %d", cam_params->buffers->buffer.length);
        if (ioctl(cam_params->cam_fd, VIDIOC_QBUF, &cam_params->buffers->buffer) < 0) {
            ALOGE("VIDIOC_QBUF: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        void* data = mmap(
                NULL,
                cam_params->buffers->buffer.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                cam_params->cam_fd,
                cam_params->buffers->buffer.m.offset
        );
        if (data == MAP_FAILED) {
            ALOGE("mmap: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        ALOGI("map buffer %p", data);
        cam_params->buffers->vData.push_back(data);

#ifdef USE_DMABUF
        int *dmafd = new int;

        struct v4l2_exportbuffer expbuf;

        memset(&expbuf, 0, sizeof(expbuf));
        expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        expbuf.index = i;
        if (ioctl(cam_params->cam_fd, VIDIOC_EXPBUF, &expbuf) == -1) {
            perror("VIDIOC_EXPBUF");
            return;
        }

        dmafd[i] = expbuf.fd;
        
        void* dmaData = mmap(NULL, cam_params->buffers->buffer.length, PROT_READ | PROT_WRITE, MAP_PRIVATE , dmafd[i], 0);
        if (dmaData == MAP_FAILED) {
            ALOGE("mmap: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        cam_params->buffers->dmaBuffer.push_back(dmaData);
#endif

    }
}

void cam_start(struct cam_control_params* cam_params) {
    int frame_interval = SENSOR_MAX_FPS / cam_params->capture_fmt.fps;
    int frame_num = 0;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct timeval start_thr, end_thr;
    if (ioctl(cam_params->cam_fd, VIDIOC_STREAMON, &type) < 0) {
        ALOGE("VIDIOC_STREAMON: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //IspControl isp_control;
    //isp_control.control_SetFps(cam_params, SENSOR_MAX_FPS);

    while (true) {
        fd_set fds;
        FD_ZERO (&fds);
        FD_SET(cam_params->cam_fd, &fds);
        select(cam_params->cam_fd + 1, &fds, NULL, NULL, NULL);
        if (ioctl(cam_params->cam_fd, VIDIOC_DQBUF, &cam_params->buffers->buffer) < 0) {
            ALOGE("VIDIOC_DQBUF: %s", strerror(errno));
        } else {
            //fpsStat();
            gettimeofday(&end_thr, NULL);
            ALOGI("Time loop from previous thread: %ld\n", (end_thr.tv_sec - start_thr.tv_sec)*1000000 + end_thr.tv_usec - start_thr.tv_usec);
            start_thr = end_thr;
            frame_num = cam_params->buffers->buffer.sequence;

            // Process the frame
            if (frame_num%frame_interval == 0) {
                cv::Mat img_for_detect = Void2CvMat((void *)cam_params->buffers->vData[cam_params->buffers->buffer.index], cam_params->buffers->buffer.length);
                if (!img_for_detect.empty()) {
                    std::vector<Object> objects;
                    detect_yolo11(img_for_detect, objects);
                    draw_objects(img_for_detect, objects);
                    cv::imshow("yolo11", img_for_detect);
                    cv::waitKey(1);
                } else {
                    ALOGE("Failed to convert image data to cv::Mat");
                }
            }

#ifdef USE_DMABUF
#ifdef SAVE_IMAGE_ISP
            if (frame_num%frame_interval == 0) {
                std::thread save_img_dma_thr([&]() {
                    size_t buffer_length = cam_params->buffers->buffer.length;
                    void* buffer_data = malloc(buffer_length);
                    mtx.lock();
                    memcpy(buffer_data, cam_params->buffers->dmaBuffer[cam_params->buffers->buffer.index], buffer_length);
                    mtx.unlock();
                    std::string filename = "frame_dma" + std::to_string(frame_num) + ".jpg";
                    FILE* file = fopen(filename.c_str(), "wb");
                    fwrite(buffer_data, 1, buffer_length, file);
                    fflush(file);
                    posix_fadvise(fileno(file), 0, 0, POSIX_FADV_DONTNEED);
                    fclose(file);
                    free(buffer_data);
                });
                save_img_dma_thr.detach();
            }
#endif
#endif

            mtx.lock();
            ioctl(cam_params->cam_fd, VIDIOC_QBUF, &cam_params->buffers->buffer);
            mtx.unlock();

            gettimeofday(&end_thr, NULL);
            ALOGI("Time end thread: %ld\n", (end_thr.tv_sec - start_thr.tv_sec)*1000000 + end_thr.tv_usec - start_thr.tv_usec);
            start_thr = end_thr;
        }
    }

    if (ioctl(cam_params->cam_fd, VIDIOC_STREAMOFF, &type) < 0) {
        ALOGE("VIDIOC_STREAMOFF: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void cam_unmap(struct cam_control_params* cam_params) {
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 0;
    if (ioctl(cam_params->cam_fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
        ALOGE("VIDIOC_REQBUFS: %s", strerror(errno));
        exit(0);
    }
    for (int i = 0; i < BUFFER_COUNT; i++) {
        if (munmap(cam_params->buffers->vData[i], cam_params->buffers->buffer.length) < 0) {
            ALOGE("munmap: %s", strerror(errno));
        }
    }
}

int cam_close(struct cam_control_params* cam_params) {
    if (close(cam_params->cam_fd) == -1) {
        ALOGE("CLOSE: %s", strerror(errno));
        return -1;
    }
    return 0;
}