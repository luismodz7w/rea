APP_PLATFORM := android-21
APP_ABI := arm64-v8a armeabi-v7a x86 x86_64
APP_STL := c++_shared
APP_CPPFLAGS := -std=c++17 -fexceptions -frtti -Wall -O2

NDK_TOOLCHAIN_VERSION := clang
APP_CFLAGS := -Wall -O2 -fPIC -fvisibility=hidden

# Enable optimization
APP_OPTIM := release

# Required for native code compatibility
APP_PLATFORM := android-21

# Use Clang compiler
NDK_TOOLCHAIN_VERSION := clang
