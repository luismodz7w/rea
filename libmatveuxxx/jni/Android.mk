LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := matveuxxx
LOCAL_SRC_FILES := libmatveuxxx.cpp

# Compiler flags
LOCAL_CFLAGS := -Wall -O2 -fPIC -fvisibility=hidden
LOCAL_CPPFLAGS := -std=c++17 -fexceptions -frtti -Wall -O2

# Include directories
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include

# Libraries to link
LOCAL_LDLIBS := -llog -landroid -lGLESv3 -lEGL -lm -ldl -lc++_shared

# Enable debugging symbols
LOCAL_LDFLAGS := -Wl,--build-id=sha1

include $(BUILD_SHARED_LIBRARY)
