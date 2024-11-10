# YOLO11 for Linux x86_64

## Introduction
This example uses v4l2 to capture image data from the webcam and then processes it using the YOLO11 NCNN model. It is written in C++ and tested on Ubuntu 20.04.

About [YOLOv11 Ultralytics](https://github.com/ultralytics/ultralytics).

About [Tencent ncnn framework](https://github.com/Tencent/ncnn).

## Required packages
1. NCNN packages: How to build [ncnn](https://github.com/Tencent/ncnn/wiki/how-to-build).
   ```sh
   git clone https://github.com/Tencent/ncnn.git
   cd ncnn
   git submodule update --init
   mkdir -p build
   cd build
   cmake -DNCNN_DISABLE_RTTI=OFF -DNCNN_BUILD_TOOLS=ON -DNCNN_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release -DNCNN_VULKAN=OFF -DNCNN_SHARED_LIB=OFF ..
   make -j$(nproc)
   make install
   ```
2. NCNN model: How to export [to NCNN from YOLO11](https://docs.ultralytics.com/integrations/ncnn/). The NCNN model is available in [here](https://github.com/thile732/yolo_research/tree/CPU_x86_64/YOLOv11_NCNN/CPP/model), converted from yolo11s.pt with imgsz=416.
3. OpenCV 64-bit
## Setup

To set up the project, follow these steps:

1. **Clone the repository:**
   ```sh
   git clone https://github.com/thile732/yolo_research.git
   ```
2. **Navigate to the project directory:**
   ```sh
   cd yolo_research
   ```
3. **Build the project using CMake:**
   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

To run for the detection, use the following command:

```sh
./yolo11
```

The default webcam is on port 0 (/dev/video0).
