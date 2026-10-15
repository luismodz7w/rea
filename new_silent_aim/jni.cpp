#include <jni.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOG_TAG "SilentAim"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ============================================================================
// ESTRUTURAS DE DADOS - SILENT AIM
// ============================================================================

struct Vector2 {
    float x, y;
};

struct Vector3 {
    float x, y, z;
};

struct Enemy {
    Vector3 position;
    Vector3 headPos;
    Vector3 chestPos;
    Vector3 pelvisPos;
    float distance;
    bool visible;
};

// ============================================================================
// CONFIGURAÇÕES DE SILENT AIM
// ============================================================================

struct SilentAimConfig {
    bool enabled = true;
    float fov = 90.0f;              // Field of View em pixels
    float maxDistance = 100.0f;     // Distância máxima em unidades
    float smoothing = 0.8f;         // Suavização (0.0 - 1.0)
    int targetBone = 1;             // 0=Head, 1=Chest, 2=Pelvis
    float hitChance = 95.0f;        // % de acerto (0-100)
} g_silentAimConfig;

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

static GLuint g_shaderProgram = 0;
static GLuint g_VAO = 0;
static glm::mat4 g_projMatrix = glm::mat4(1.0f);
static Vector2 g_currentAim = {0.0f, 0.0f};
static Vector2 g_targetAim = {0.0f, 0.0f};

// ============================================================================
// SHADERS GLSL (Minimalistas - Apenas Silent Aim)
// ============================================================================

const char* VERTEX_SHADER = R"(
    #version 300 es
    precision highp float;

    layout(location = 0) in vec2 position;
    layout(location = 1) in vec4 color;

    uniform mat4 projMatrix;

    out vec4 vertexColor;

    void main() {
        gl_Position = projMatrix * vec4(position, 0.0, 1.0);
        vertexColor = color;
    }
)";

const char* FRAGMENT_SHADER = R"(
    #version 300 es
    precision mediump float;

    in vec4 vertexColor;
    out vec4 FragColor;

    void main() {
        FragColor = vertexColor;
    }
)";

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

static GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOGE("Shader compilation failed: %s", infoLog);
    }
    return shader;
}

static float distance3D(Vector3 a, Vector3 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

static float calculatePredictedPosition(float current, float target, float smoothing) {
    return current + (target - current) * smoothing;
}

// ============================================================================
// SILENT AIM LOGIC
// ============================================================================

static void updateSilentAim(Enemy& enemy) {
    if (!g_silentAimConfig.enabled || !enemy.visible) {
        return;
    }

    // Verificar distância
    enemy.distance = distance3D(Vector3{0, 0, 0}, enemy.position);
    if (enemy.distance > g_silentAimConfig.maxDistance) {
        return;
    }

    // Selecionar osso alvo (Bone Aim)
    Vector3 targetPos;
    switch (g_silentAimConfig.targetBone) {
        case 0: targetPos = enemy.headPos; break;    // Head
        case 1: targetPos = enemy.chestPos; break;   // Chest
        case 2: targetPos = enemy.pelvisPos; break;  // Pelvis
        default: targetPos = enemy.chestPos;
    }

    // Calcular predição de movimento (Silent Pred)
    Vector2 predictedAim;
    predictedAim.x = calculatePredictedPosition(
        g_currentAim.x,
        targetPos.x,
        g_silentAimConfig.smoothing
    );
    predictedAim.y = calculatePredictedPosition(
        g_currentAim.y,
        targetPos.y,
        g_silentAimConfig.smoothing
    );

    // Verificar se está dentro do FOV (Silent FOV)
    float distToTarget = std::sqrt(
        predictedAim.x * predictedAim.x +
        predictedAim.y * predictedAim.y
    );

    if (distToTarget <= g_silentAimConfig.fov) {
        g_targetAim = predictedAim;

        // Aplicar suavização
        g_currentAim.x += (g_targetAim.x - g_currentAim.x) * g_silentAimConfig.smoothing;
        g_currentAim.y += (g_targetAim.y - g_currentAim.y) * g_silentAimConfig.smoothing;

        LOGI("[SILENT AIM] Distance: %.1f | Bone: %d | Hit Chance: %.0f%%",
             enemy.distance,
             g_silentAimConfig.targetBone,
             g_silentAimConfig.hitChance);
    }
}

// ============================================================================
// JNI FUNCTIONS - ANDROID INTERFACE
// ============================================================================

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad - Initializing Silent Aim Library");
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload - Cleaning up");
    if (g_shaderProgram) {
        glDeleteProgram(g_shaderProgram);
    }
    if (g_VAO) {
        glDeleteVertexArrays(1, &g_VAO);
    }
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_init(JNIEnv* env, jobject obj) {
    LOGI("init() - Initializing OpenGL and Silent Aim");

    // Compilar shaders
    GLuint vertexShader = compileShader(VERTEX_SHADER, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER);

    g_shaderProgram = glCreateProgram();
    glAttachShader(g_shaderProgram, vertexShader);
    glAttachShader(g_shaderProgram, fragmentShader);
    glLinkProgram(g_shaderProgram);

    GLint success;
    glGetProgramiv(g_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(g_shaderProgram, 512, nullptr, infoLog);
        LOGE("Program linking failed: %s", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Criar VAO
    glGenVertexArrays(1, &g_VAO);
    glBindVertexArray(g_VAO);

    // Configurar matriz de projeção
    g_projMatrix = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f, -1.0f, 1.0f);

    // Log inicial
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    LOGI("[SILENT AIM] Initialized - FOV: %.1f | Distance: %.1f | Smoothing: %.2f",
         g_silentAimConfig.fov,
         g_silentAimConfig.maxDistance,
         g_silentAimConfig.smoothing);
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_step(JNIEnv* env, jobject obj) {
    // Renderizar frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_shaderProgram);

    // Simular enemy para testing
    Enemy testEnemy;
    testEnemy.position = {500.0f, 500.0f, 0.0f};
    testEnemy.headPos = {500.0f, 480.0f, 0.0f};
    testEnemy.chestPos = {500.0f, 500.0f, 0.0f};
    testEnemy.pelvisPos = {500.0f, 520.0f, 0.0f};
    testEnemy.visible = true;

    // Atualizar Silent Aim
    updateSilentAim(testEnemy);

    // Desenhar FOV circle (debug)
    if (g_silentAimConfig.enabled) {
        int segments = 32;
        float angle = 0.0f;
        float fov = g_silentAimConfig.fov;

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            angle = 2.0f * 3.14159f * i / segments;
            float x = 960.0f + fov * std::cos(angle);
            float y = 540.0f + fov * std::sin(angle);
            glVertex2f(x, y);
        }
        glEnd();
    }
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_resize(JNIEnv* env, jobject obj, jint width, jint height) {
    glViewport(0, 0, width, height);
    g_projMatrix = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    LOGI("resize() - %d x %d", width, height);
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_MotionEventClick(JNIEnv* env, jobject obj,
                                                              jfloat x, jfloat y, jint action) {
    // Processar eventos de toque para Silent Aim
    LOGI("Touch Event: %.1f, %.1f | Action: %d", x, y, action);
}

JNIEXPORT jobject JNICALL
Java_com_mycompany_application_GLES3JNIView_getWindowRect(JNIEnv* env, jobject obj) {
    // Retornar dimensões da janela
    return nullptr;
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown(JNIEnv* env, jobject obj) {
    LOGI("imgui_Shutdown() - Cleaning up resources");
}

} // extern "C"
