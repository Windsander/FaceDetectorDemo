LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifdef OPENCV_ANDROID_SDK
  ifneq ("","$(wildcard $(OPENCV_ANDROID_SDK)/OpenCV.mk)")
    include ${OPENCV_ANDROID_SDK}/OpenCV.mk
  else
    include ${OPENCV_ANDROID_SDK}/sdk/native/jni/OpenCV.mk
  endif
else
  include ../sdk/native/jni/OpenCV.mk
endif

include $(BUILD_SHARED_LIBRARY)