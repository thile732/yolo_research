cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(YOLO11)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 17)
  endif ()
endmacro(use_cxx11)

find_package(OpenCV REQUIRED)                              

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(EXTRA_LIBS ${OpenCV_LIBS} ${PROJECT_SOURCE_DIR}/lib/libncnn.a)

file(GLOB INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include/ncnn)

include_directories(include ${OpenCV_INCLUDE_DIRS} ${INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

add_link_options(-fopenmp -s -ldl -lpthread -lgomp -rdynamic)

add_executable(yolo11 ${SOURCES})

target_link_libraries(yolo11 ${EXTRA_LIBS})