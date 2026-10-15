# Quick Start - libmatveuxxx.so

## ⚡ Compilação Rápida

```bash
# Setup NDK
export NDK_HOME=/path/to/ndk-r27
export PATH=$NDK_HOME/bin:$PATH

# Build
cd /tmp/rea/libmatveuxxx
ndk-build

# Output
ls libs/arm64-v8a/libmatveuxxx.so
# ~ 1.5 MB
```

## 📦 Integração em Projeto Android

```bash
# Copy to Android project
cp libs/arm64-v8a/libmatveuxxx.so \
   ../android-project/app/src/main/jniLibs/arm64-v8a/

# In MainActivity.java
static {
    System.loadLibrary("matveuxxx");
}

// Call from GLRenderer
GLES3JNIView view = new GLES3JNIView(context);
view.init();
view.resize(width, height);
view.step();  // Every frame
```

## 🎮 Funcionalidades

### Silent Aim
- Smooth interpolation: `lerp(lastX, targetX, 0.3f)`
- FOV constraint: 120 pixel radius
- Math: `atan2f`, `cosf`, `sinf` for angle calculation

### ESP
- Enemy boxes: `drawBox(x, y, w, h, r, g, b, a)`
- Skeleton lines: `drawLine(x1, y1, x2, y2, color)`
- Health bars visualization

### Menu
- Click detection in 10x10 to 210x410 area
- 4 item toggles for features
- Rendered every frame

## 🔧 Build Configuration

**Android.mk** - NDK build script
```makefile
LOCAL_MODULE := matveuxxx
LOCAL_SRC_FILES := libmatveuxxx.cpp
LOCAL_CFLAGS := -Wall -O2 -fPIC -fvisibility=hidden
LOCAL_CPPFLAGS := -std=c++17
LOCAL_LDLIBS := -llog -lGLESv3 -lEGL -lm -lc++_shared
```

**Application.mk** - Project settings
```makefile
APP_PLATFORM := android-21
APP_ABI := arm64-v8a armeabi-v7a x86 x86_64
APP_STL := c++_shared
APP_OPTIM := release
```

## 📊 JNI Methods

| Method | Signature | Purpose |
|--------|-----------|---------|
| `init` | `()V` | OpenGL setup |
| `resize` | `(II)V` | Viewport resize |
| `step` | `()V` | Main render loop |
| `MotionEventClick` | `(ZFF)V` | Input (down, x, y) |
| `getWindowRect` | `()Ljava/lang/String;` | Menu bounds |
| `imgui_Shutdown` | `()V` | Cleanup |

## 🔍 Global Structures

```cpp
GraphicsEngine g_graphics
MenuConfig g_menu           // x=10, y=10, w=200, h=400
AimConfig g_aim            // smooth=0.3, fov=120
ESPConfig g_esp            // drawBoxes, drawSkeletons
SkinConfig g_skin          // selectedSkin, unlockedSkins
```

## 📈 File Sizes

| Architecture | Size |
|---|---|
| arm64-v8a | ~1.5 MB |
| armeabi-v7a | ~1.2 MB |
| x86 | ~1.3 MB |
| x86_64 | ~1.6 MB |

## ✅ Verification

```bash
# Check library loaded
adb logcat | grep "libmatveuxxx"

# Expected output
# libmatveuxxx: JNI_OnLoad called
# libmatveuxxx: init() called
# libmatveuxxx: Graphics engine initialized successfully
```

## 🚀 Troubleshooting

**NDK not found**
```bash
# Download from:
# https://developer.android.com/ndk/downloads
# Set path: export NDK_HOME=/path/to/ndk
```

**Compilation error: GLES3/gl3.h not found**
```bash
# Use NDK toolchain directly:
ndk-build APP_PLATFORM=android-21
```

**Library not loading**
```bash
# Check:
1. Library copied to jniLibs/arm64-v8a/
2. System.loadLibrary called before JNI methods
3. Android API >= 21
```

## 📚 Documentation

- **RECONSTRUCTION_ANALYSIS.md** - Technical deep dive
- **README.md** - Compilation guide
- **RECONSTRUCAO_FINAL.md** - Summary of implementation

---

**Status**: ✅ Ready to compile and test
