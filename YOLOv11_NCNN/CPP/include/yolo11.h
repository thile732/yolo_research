#ifndef _YOLO11_H_
#define _YOLO11_H_

#include <memory>
#include <vector>
#include <algorithm>
#include "layer.h"
#include "net.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <float.h>
#include <stdio.h>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

int detect_yolo11(const cv::Mat& bgr, std::vector<Object>& objects);
void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects);

#endif // _YOLO11_H_
