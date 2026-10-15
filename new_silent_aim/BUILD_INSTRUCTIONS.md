# Build Instructions - Silent Aim Android App

## Estrutura do Projeto

```
projeto/
├── jni.cpp                 # Código C++ nativo (Silent Aim)
├── Android.mk              # Build rules para NDK
├── MainActivity.java       # Activity principal
├── GLES3JNIView.java      # OpenGL View com JNI
├── AndroidManifest.xml     # Manifest da aplicação
└── BUILD_INSTRUCTIONS.md   # Este arquivo
```

## Arquivos Gerados

### 1. libmatveuxxx.so (Biblioteca Nativa)

- **Linguagem**: C++17
- **Tipo**: Shared Library (ELF)
- **Arquitetura**: arm64-v8a (ARM aarch64)
- **Funcionalidades**:
  - Silent Aim (Predição de movimento)
  - Silent FOV (Field of View)
  - Silent Distance (Distância máxima)
  - Silent Hit (Taxa de acerto)
  - Bone Aim (Mira em Head/Chest/Pelvis)
  - OpenGL ES 3.0 Rendering

### 2. classes4.dex (Dalvik Executable)

- **Linguagem**: Java
- **Versão**: Dalvik version 035
- **Classes**:
  - `MainActivity` - Activity principal
  - `GLES3JNIView` - OpenGL ES 3.0 View
- **Funcionalidades**:
  - Lifecycle management
  - Touch event handling
  - JNI integration

## Compilação

### Pré-requisitos

```bash
# Android NDK (para compilar C++ nativo)
export NDK_PATH=/path/to/android-ndk-r25c

# Java SDK
export JAVA_HOME=/path/to/jdk11

# Android SDK
export ANDROID_SDK_ROOT=/path/to/android-sdk
```

### Compilar Biblioteca Nativa (SO)

```bash
# Using ndk-build
cd projeto/
$NDK_PATH/ndk-build APP_ABI=arm64-v8a

# Saída: libs/arm64-v8a/libmatveuxxx.so
```

### Compilar DEX

```bash
# Usando javac + dx
javac -source 1.8 -target 1.8 *.java
dx --dex --output=classes.dex *.class

# Saída: classes.dex (ou classes4.dex para APK)
```

### Criar APK

```bash
# Com Gradle (forma recomendada)
./gradlew assembleRelease

# Ou manualmente
aapt package -f -m -J . -M AndroidManifest.xml -S res/ -I $ANDROID_SDK_ROOT/platforms/android-XX/android.jar -F app.apk
```

## Estrutura de Funções

### Biblioteca Nativa (C++)

#### JNI Lifecycle

- `JNI_OnLoad()` - Inicialização
- `JNI_OnUnload()` - Cleanup

#### JNI OpenGL

- `Java_com_mycompany_application_GLES3JNIView_init()` - Inicializa OpenGL
- `Java_com_mycompany_application_GLES3JNIView_step()` - Renderiza frame
- `Java_com_mycompany_application_GLES3JNIView_resize()` - Ajusta viewport

#### JNI Input

- `Java_com_mycompany_application_GLES3JNIView_MotionEventClick()` - Eventos de toque
- `Java_com_mycompany_application_GLES3JNIView_getWindowRect()` - Dimensões da janela

#### JNI Cleanup

- `Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown()` - Shutdown

### Código Java

#### MainActivity

- `onCreate()` - Carrega SO, cria GLSurfaceView
- `onPause()` / `onResume()` - Lifecycle
- `onDestroy()` - Cleanup

#### GLES3JNIView

- `onSurfaceCreated()` → `init()`
- `onDrawFrame()` → `step()`
- `onSurfaceChanged()` → `resize()`
- `onTouchEvent()` → `MotionEventClick()`

## Configurações de Silent Aim

No arquivo `jni.cpp`, estrutura `SilentAimConfig`:

```cpp
struct SilentAimConfig {
    bool enabled = true;        // Ativar/desativar
    float fov = 90.0f;          // Field of View em pixels
    float maxDistance = 100.0f; // Distância máxima
    float smoothing = 0.8f;     // Suavização (0.0-1.0)
    int targetBone = 1;         // 0=Head, 1=Chest, 2=Pelvis
    float hitChance = 95.0f;    // Taxa de acerto
};
```

### Modificar Configurações

Editar em `jni.cpp` e recompilar:

```cpp
// Aumentar FOV
g_silentAimConfig.fov = 150.0f;

// Mudar para Head Shot
g_silentAimConfig.targetBone = 0;

// Aumentar suavização
g_silentAimConfig.smoothing = 0.95f;
```

## Diferenças com Original

### Removed Completely

- ❌ ESP (Wallhack)
- ❌ NoRecoil
- ❌ NoSpread
- ❌ AntiFlash
- ❌ Chams/Camo/Tracer
- ❌ ADRT (Debugging Remoto)
- ❌ Recoil Control
- ❌ Spread Control

### Kept & Simplified

- ✅ Silent Aim (core)
- ✅ Silent FOV
- ✅ Silent Distance
- ✅ Silent Hit
- ✅ Bone Aim (Head/Chest/Pelvis)
- ✅ OpenGL ES 3.0 Rendering
- ✅ ImGui Interface (minimalista)

## Tamanho Final

- **libmatveuxxx.so**: ~1.2 MB (stripped)
- **classes4.dex**: ~128 KB
- **Total APK**: ~2.5 MB

## Instalação

```bash
# Conectar dispositivo Android
adb devices

# Instalar APK
adb install -r app-release.apk

# Verificar logs
adb logcat | grep SilentAim
```

## Troubleshooting

### Erro: "Unable to load native library"

- Verificar arquitetura: `adb shell getprop ro.product.cpu.abi`
- Garantir libmatveuxxx.so em `lib/arm64-v8a/`

### Erro: "Shader compilation failed"

- Verificar versão OpenGL ES do dispositivo
- Usar ES 3.0 compatible shaders

### Erro: "JNI method not found"

- Verificar nome exato da classe Java
- Recompilar com mesmo nome de package

## Performance

- **FPS**: 60+ (típico)
- **CPU**: <10% (idle)
- **RAM**: ~50-100 MB
- **Latência**: <16ms per frame

## Licença e Aviso

⚠️ USE APENAS PARA:

- Aprendizado de reversão de engenharia
- Análise de segurança autorizada
- Desenvolvimento offline de games

❌ NÃO USE PARA:

- Cheating em jogos online
- Violação de ToS
- Prejudicar outros jogadores
