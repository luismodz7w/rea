LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := matveuxxx
LOCAL_SRC_FILES := jni/libmatveuxxx.cpp

LOCAL_CFLAGS := -Wall -O2 -fPIC -fvisibility=hidden -funwind-tables -fstack-protector-strong
LOCAL_CPPFLAGS := -std=c++17 -fexceptions -frtti

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_LDLIBS := -llog -landroid -lGLESv3 -lEGL -lm -ldl -lc++_shared

include $(BUILD_SHARED_LIBRARY)
