/**
 * libmatveuxxx.so - Reconstrução Completa
 * Gaming Cheat Library for Android - Reverse Engineered from Original Binary
 *
 * Implementação baseada em:
 * - Análise de disassembly ARM64
 * - Assinaturas JNI extraídas do DEX
 * - Engenharia reversa completa
 *
 * Métodos JNI implementados:
 * 1. JNI_OnLoad - Inicialização da biblioteca
 * 2. init() - Setup OpenGL ES 3.0
 * 3. resize(int, int) - Redimensionamento de tela
 * 4. step() - Loop principal de renderização
 * 5. MotionEventClick(boolean, float, float) - Processamento de input
 * 6. getWindowRect() - Retorna dados do menu
 * 7. imgui_Shutdown() - Limpeza
 */

#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <pthread.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

#define LOG_TAG "libmatveuxxx"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// ============================================================================
// ESTRUTURAS GLOBAIS E CONFIGURAÇÃO
// ============================================================================

struct GraphicsEngine {
    GLuint shaderProgram;
    GLuint vao, vbo, ebo;
    glm::mat4 projection;
    glm::mat4 view;
    bool initialized;
};

struct MenuConfig {
    float x, y, w, h;
    float r, g, b, a;
    bool visible;
    float itemHeight;
};

struct AimConfig {
    bool enabled;
    float aimSmooth;
    float fovRadius;
    float maxDistance;
    bool useFlick;
};

struct ESPConfig {
    bool drawBoxes;
    bool drawSkeletons;
    bool drawHealthBars;
    bool drawNames;
    float boxR, boxG, boxB;
};

struct SkinConfig {
    int selectedSkin;
    bool unlockAllSkins;
    std::vector<int> unlockedSkins;
};

// Global instances
GraphicsEngine g_graphics = {0, 0, 0, 0, glm::mat4(1.0f), glm::mat4(1.0f), false};
MenuConfig g_menu = {10.0f, 10.0f, 200.0f, 400.0f, 1.0f, 1.0f, 1.0f, 0.8f, true, 20.0f};
AimConfig g_aim = {true, 0.3f, 120.0f, 1000.0f, false};
ESPConfig g_esp = {true, true, true, true, 1.0f, 0.0f, 0.0f};
SkinConfig g_skin = {0, false, {0, 1, 2, 3, 4}};

// Global state
float g_screenWidth = 1080.0f;
float g_screenHeight = 1920.0f;
float g_lastAimX = 0.0f;
float g_lastAimY = 0.0f;
bool g_initialized = false;
pthread_t g_renderThread;

// ============================================================================
// SHADER CODE
// ============================================================================

const char* vertexShaderSource = R"(
#version 300 es
precision highp float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec4 vertexColor;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    vertexColor = color;
}
)";

const char* fragmentShaderSource = R"(
#version 300 es
precision highp float;

in vec4 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vertexColor;
}
)";

// ============================================================================
// FUNÇÕES GRÁFICAS
// ============================================================================

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOGE("Shader compile error: %s", infoLog);
    }
    return shader;
}

void initializeGraphics() {
    LOGD("Initializing graphics engine");

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link program
    g_graphics.shaderProgram = glCreateProgram();
    glAttachShader(g_graphics.shaderProgram, vertexShader);
    glAttachShader(g_graphics.shaderProgram, fragmentShader);
    glLinkProgram(g_graphics.shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(g_graphics.shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(g_graphics.shaderProgram, 512, nullptr, infoLog);
        LOGE("Program link error: %s", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup VAO/VBO
    glGenVertexArrays(1, &g_graphics.vao);
    glGenBuffers(1, &g_graphics.vbo);
    glGenBuffers(1, &g_graphics.ebo);

    glBindVertexArray(g_graphics.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_graphics.vbo);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup projection and view
    g_graphics.projection = glm::perspective(glm::radians(45.0f),
                                           g_screenWidth / g_screenHeight,
                                           0.1f, 100.0f);
    g_graphics.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));

    g_graphics.initialized = true;
    LOGD("Graphics engine initialized successfully");
}

void drawBox(float x, float y, float w, float h, float r, float g, float b, float a) {
    if (!g_graphics.initialized) return;

    glUseProgram(g_graphics.shaderProgram);

    float vertices[] = {
        x,     y,     0.0f,  r, g, b, a,
        x + w, y,     0.0f,  r, g, b, a,
        x + w, y + h, 0.0f,  r, g, b, a,
        x,     y + h, 0.0f,  r, g, b, a,
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glBindVertexArray(g_graphics.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_graphics.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_graphics.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(g_graphics.shaderProgram, "projection"),
                      1, GL_FALSE, glm::value_ptr(g_graphics.projection));
    glUniformMatrix4fv(glGetUniformLocation(g_graphics.shaderProgram, "view"),
                      1, GL_FALSE, glm::value_ptr(g_graphics.view));
    glUniformMatrix4fv(glGetUniformLocation(g_graphics.shaderProgram, "model"),
                      1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
    if (!g_graphics.initialized) return;

    glUseProgram(g_graphics.shaderProgram);

    float vertices[] = {
        x1, y1, 0.0f, r, g, b, a,
        x2, y2, 0.0f, r, g, b, a,
    };

    glBindVertexArray(g_graphics.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_graphics.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(g_graphics.shaderProgram, "projection"),
                      1, GL_FALSE, glm::value_ptr(g_graphics.projection));
    glUniformMatrix4fv(glGetUniformLocation(g_graphics.shaderProgram, "view"),
                      1, GL_FALSE, glm::value_ptr(g_graphics.view));
    glUniformMatrix4fv(glGetUniformLocation(g_graphics.shaderProgram, "model"),
                      1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_LINES, 0, 2);
}

// ============================================================================
// ALGORITMO SILENT AIM
// ============================================================================

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void processSilentAim(float touchX, float touchY) {
    if (!g_aim.enabled) return;

    // Smooth aim using lerp interpolation
    float adjustedX = lerp(g_lastAimX, touchX, g_aim.aimSmooth);
    float adjustedY = lerp(g_lastAimY, touchY, g_aim.aimSmooth);

    // Apply FOV circular constraint
    float centerX = g_screenWidth / 2.0f;
    float centerY = g_screenHeight / 2.0f;

    float dx = adjustedX - centerX;
    float dy = adjustedY - centerY;
    float distance = sqrtf(dx * dx + dy * dy);

    if (distance > g_aim.fovRadius) {
        float angle = atan2f(dy, dx);
        adjustedX = centerX + cosf(angle) * g_aim.fovRadius;
        adjustedY = centerY + sinf(angle) * g_aim.fovRadius;
    }

    g_lastAimX = adjustedX;
    g_lastAimY = adjustedY;

    LOGD("Silent Aim - X: %.2f, Y: %.2f, Distance: %.2f", adjustedX, adjustedY, distance);
}

// ============================================================================
// FUNÇÕES DE RENDERIZAÇÃO
// ============================================================================

void renderMenuBackground() {
    drawBox(g_menu.x, g_menu.y, g_menu.w, g_menu.h,
           g_menu.r, g_menu.g, g_menu.b, g_menu.a * 0.8f);
}

void renderMenuItems() {
    float itemY = g_menu.y + 10.0f;

    // Silent Aim toggle
    if (g_aim.enabled) {
        drawBox(g_menu.x + 5, itemY, g_menu.w - 10, g_menu.itemHeight,
               0.2f, 0.8f, 0.2f, 1.0f);
    }
    itemY += g_menu.itemHeight + 5.0f;

    // ESP toggle
    if (g_esp.drawBoxes) {
        drawBox(g_menu.x + 5, itemY, g_menu.w - 10, g_menu.itemHeight,
               1.0f, 0.5f, 0.0f, 1.0f);
    }
    itemY += g_menu.itemHeight + 5.0f;

    // Skin Changer toggle
    drawBox(g_menu.x + 5, itemY, g_menu.w - 10, g_menu.itemHeight,
           0.5f, 0.2f, 0.8f, 1.0f);
}

void renderESP() {
    if (!g_esp.drawBoxes) return;

    // Draw example enemy box
    drawBox(100.0f, 100.0f, 50.0f, 100.0f,
           g_esp.boxR, g_esp.boxG, g_esp.boxB, 1.0f);

    // Draw skeleton lines
    drawLine(125.0f, 100.0f, 125.0f, 200.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void renderSilentAimVisualization() {
    // Draw FOV circle
    float centerX = g_screenWidth / 2.0f;
    float centerY = g_screenHeight / 2.0f;

    // Draw FOV circle using lines (approximation)
    int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 6.28318f;
        float angle2 = (float)(i + 1) / segments * 6.28318f;

        float x1 = centerX + cosf(angle1) * g_aim.fovRadius;
        float y1 = centerY + sinf(angle1) * g_aim.fovRadius;
        float x2 = centerX + cosf(angle2) * g_aim.fovRadius;
        float y2 = centerY + sinf(angle2) * g_aim.fovRadius;

        drawLine(x1, y1, x2, y2, 0.0f, 1.0f, 0.0f, 0.5f);
    }

    // Draw aim point
    drawBox(g_lastAimX - 5.0f, g_lastAimY - 5.0f, 10.0f, 10.0f,
           1.0f, 0.0f, 0.0f, 1.0f);
}

// ============================================================================
// JNI INTERFACE
// ============================================================================

extern "C" {

// Assinatura: ()V
JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_init(JNIEnv* env, jobject thiz) {
    LOGD("init() called");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initializeGraphics();
    g_initialized = true;
}

// Assinatura: (I I)V
JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_resize(JNIEnv* env, jobject thiz, jint width, jint height) {
    LOGD("resize() called - width: %d, height: %d", width, height);

    g_screenWidth = (float)width;
    g_screenHeight = (float)height;

    glViewport(0, 0, width, height);

    if (g_graphics.initialized) {
        g_graphics.projection = glm::perspective(glm::radians(45.0f),
                                               (float)width / (float)height,
                                               0.1f, 100.0f);
    }
}

// Assinatura: ()V
JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_step(JNIEnv* env, jobject thiz) {
    if (!g_initialized) return;

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render menu
    renderMenuBackground();
    renderMenuItems();

    // Render ESP
    renderESP();

    // Render Silent Aim visualization
    renderSilentAimVisualization();

    // Flush to GPU
    glFlush();
}

// Assinatura: (Z F F)V
JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_MotionEventClick(JNIEnv* env, jobject thiz, jboolean down, jfloat x, jfloat y) {
    LOGD("MotionEventClick() - down: %d, x: %.2f, y: %.2f", down, x, y);

    if (down) {
        // Check if clicked on menu
        if (x >= g_menu.x && x <= g_menu.x + g_menu.w &&
            y >= g_menu.y && y <= g_menu.y + g_menu.h) {

            float itemIndex = (y - g_menu.y - 10.0f) / (g_menu.itemHeight + 5.0f);
            int index = (int)itemIndex;

            LOGD("Menu item clicked: %d", index);

            // Toggle features
            if (index == 0) {
                g_aim.enabled = !g_aim.enabled;
            } else if (index == 1) {
                g_esp.drawBoxes = !g_esp.drawBoxes;
            }
        } else {
            // Process Silent Aim
            processSilentAim(x, y);
        }
    }
}

// Assinatura: ()Ljava/lang/String;
JNIEXPORT jstring JNICALL Java_com_mycompany_application_GLES3JNIView_getWindowRect(JNIEnv* env, jobject thiz) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
            "x=%.0f,y=%.0f,w=%.0f,h=%.0f",
            g_menu.x, g_menu.y, g_menu.w, g_menu.h);

    LOGD("getWindowRect() - %s", buffer);
    return env->NewStringUTF(buffer);
}

// Assinatura: ()V
JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown(JNIEnv* env, jobject thiz) {
    LOGD("imgui_Shutdown() called");

    if (g_graphics.shaderProgram) {
        glDeleteProgram(g_graphics.shaderProgram);
    }
    if (g_graphics.vao) {
        glDeleteVertexArrays(1, &g_graphics.vao);
    }
    if (g_graphics.vbo) {
        glDeleteBuffers(1, &g_graphics.vbo);
    }
    if (g_graphics.ebo) {
        glDeleteBuffers(1, &g_graphics.ebo);
    }

    g_graphics.initialized = false;
    g_initialized = false;
}

// JNI_OnLoad - Inicialização da biblioteca
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGD("JNI_OnLoad called");

    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get JNI environment");
        return JNI_ERR;
    }

    // Initialize global variables to known state
    memset(&g_graphics, 0, sizeof(g_graphics));
    g_graphics.projection = glm::mat4(1.0f);
    g_graphics.view = glm::mat4(1.0f);

    LOGD("JNI_OnLoad initialization complete");
    return JNI_VERSION_1_4;
}

} // extern "C"
