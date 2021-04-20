# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html
cmake_minimum_required(VERSION 3.6)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 设置 native_lib 目录
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../distribution)
set(dlib_path ${CMAKE_SOURCE_DIR}/src/main/cpp/dlib)
#set(OPENCV_ANDROID_SDK ${PROJECT_SOURCE_DIR}/../sdk)
#set(OPENCV_ANDROID_SDK /Users/.iwillow/WorkingSpace/Android-Work/video-component-facedetector/sdk)
set(ANDROID_OPENCV_COMPONENTS "opencv_java" CACHE STRING "")
message(STATUS "distribution_DIR = ${distribution_DIR}")
message(STATUS "OPENCV_ANDROID_SDK = ${OPENCV_ANDROID_SDK}")
message(STATUS "CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "ANDROID_OPENCV_COMPONENTS = ${ANDROID_OPENCV_COMPONENTS}")
message(STATUS "ANDROID_ABI=${ANDROID_ABI}")
message(STATUS "OpenCV_DIR=${OpenCV_DIR}")
message(STATUS "LOCAL_PATH=${LOCAL_PATH}")

# 设置dlib
include(${dlib_path}/cmake)
include_directories("${CMAKE_CURRENT_LIST_DIR}")
include_directories("${OpenCV_DIR}")
include_directories("${OpenCV_BASE_DIR}/native/libs/")

# 配置开关
set(DLIB_IN_PROJECT_BUILD false)
set(BUILD_SHARED_LIBS true)         # 编译成动态库
set(DLIB_NO_GUI_SUPPORT true)       # 不需要gui支持
set(ARM_NEON_IS_AVAILABLE true)     # neon优化：开
set(USE_SSE2_INSTRUCTIONS false)    # sse2优化：关
set(USE_SSE4_INSTRUCTIONS false)    # sse4优化：关
set(USE_AVX_INSTRUCTIONS false)     # avx优化：关
set(DLIB_PNG_SUPPORT false)         # PNG支持
add_definitions(-DDLIB_NO_GUI_SUPPORT)

# 添加自定义库
add_library(
        face_lib SHARED
        src/main/cpp/dlib/all/source.cpp
        src/main/cpp/jdata_complier.cpp
        src/main/cpp/face_detector.cpp
        src/main/cpp/face_swap.cpp
)

# 关联 log 库
find_library(log-lib log)

# 配置 OpenCV 代码参数
find_package(OpenCV REQUIRED COMPONENTS ${ANDROID_OPENCV_COMPONENTS})
include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(
        face_lib
        dlib
        ${OpenCV_LIBRARY}
        ${log-lib}
)


#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
set(CMAKE_BUILE_TYPE "RELEASE")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -s -O3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -s -O3 -Wall")

#[[
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
set(CMAKE_BUILE_TYPE "DEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
]]
