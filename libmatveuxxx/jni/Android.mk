LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := matveuxxx
LOCAL_SRC_FILES := libmatveuxxx.cpp

LOCAL_CFLAGS := -Wall -O0 -fPIC
LOCAL_CPPFLAGS := -std=c++17 -fexceptions -frtti

LOCAL_C_INCLUDES := $(LOCAL_PATH)

# Tentar linkar bibliotecas do NDK (mesmo que dinamicamente)
LOCAL_LDLIBS := -llog -landroid -lGLESv3 -lEGL -lm -ldl
LOCAL_STATIC_LIBRARIES := c++_static

include $(BUILD_SHARED_LIBRARY)
