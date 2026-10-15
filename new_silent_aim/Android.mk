LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := matveuxxx
LOCAL_SRC_FILES := jni.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS := -Wall -Wextra -O3 -std=c++17

LOCAL_LDLIBS := -llog -lEGL -lGLESv3 -landroid -lz

include $(BUILD_SHARED_LIBRARY)
