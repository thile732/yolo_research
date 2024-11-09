#ifndef _IMG_PROC_H_
#define _IMG_PROC_H_

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <jpeglib.h>
#include <opencv2/opencv.hpp>

cv::Mat Void2CvMat(void* jpegData, size_t dataSize);


#endif // _IMG_PROC_H_