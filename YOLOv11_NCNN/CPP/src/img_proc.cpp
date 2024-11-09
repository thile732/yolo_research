#include <stdint.h>
#include <vector>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "img_proc.h"

cv::Mat Void2CvMat(void* jpegData, size_t dataSize) {
    // Create a vector from the void pointer
    std::vector<uchar> buffer(static_cast<uchar*>(jpegData), static_cast<uchar*>(jpegData) + dataSize);

    // Decode the JPEG data into a cv::Mat
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);

    return image;
}