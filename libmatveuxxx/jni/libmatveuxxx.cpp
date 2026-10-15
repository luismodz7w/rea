#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>
#include <pthread.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include "glm.hpp"

#define LOG_TAG "libmatveuxxx"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// ============================================================================
// ENEMY STRUCTURE - From disassembly analysis
// ============================================================================

struct PlayerEntity {
    float x, y, z;
    float width, height;
    float health;
    float maxHealth;
    bool visible;
    bool alive;
    int type;
    int skin;
    std::string name;
    float headX, headY;
    float bodyX, bodyY;
    float legX, legY;
};

struct BoneConnection {
    int bone1;
    int bone2;
};

// Global enemy list for ESP rendering
static std::vector<PlayerEntity> g_enemies;
static const int MAX_ENEMIES = 32;

// ============================================================================
// GRAPHICS STRUCTURES
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
    bool predictMode;
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

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================

GraphicsEngine g_graphics = {0, 0, 0, 0, glm::mat4(1.0f), glm::mat4(1.0f), false};
MenuConfig g_menu = {10.0f, 10.0f, 200.0f, 400.0f, 1.0f, 1.0f, 1.0f, 0.8f, true, 20.0f};
AimConfig g_aim = {true, 0.3f, 120.0f, 1000.0f, false};
ESPConfig g_esp = {true, true, true, true, 1.0f, 0.0f, 0.0f};
SkinConfig g_skin = {0, false, {0, 1, 2, 3, 4}};

float g_screenWidth = 1080.0f;
float g_screenHeight = 1920.0f;
float g_lastAimX = 0.0f;
float g_lastAimY = 0.0f;
bool g_initialized = false;
JavaVM* g_javaVM = nullptr;

// ============================================================================
// SHADER SOURCES
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
// SHADER COMPILATION
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

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

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

    glGenVertexArrays(1, &g_graphics.vao);
    glGenBuffers(1, &g_graphics.vbo);
    glGenBuffers(1, &g_graphics.ebo);

    glBindVertexArray(g_graphics.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_graphics.vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    g_graphics.projection = glm::perspective(glm::radians(45.0f),
                                           g_screenWidth / g_screenHeight,
                                           0.1f, 100.0f);
    g_graphics.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));

    g_graphics.initialized = true;
    LOGD("Graphics engine initialized successfully");
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

void drawBox(float x, float y, float w, float h, float r, float g, float b, float a) {
    if (!g_graphics.initialized) return;

    glUseProgram(g_graphics.shaderProgram);

    float vertices[] = {
        x,     y,     0.0f,  r, g, b, a,
        x + w, y,     0.0f,  r, g, b, a,
        x + w, y + h, 0.0f,  r, g, b, a,
        x,     y + h, 0.0f,  r, g, b, a,
    };

    unsigned int indices[] = {0, 1, 2, 0, 2, 3};

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
// SILENT AIM ALGORITHM
// ============================================================================

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float clamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

void processSilentAim(float touchX, float touchY) {
    if (!g_aim.enabled) return;

    float adjustedX = lerp(g_lastAimX, touchX, g_aim.aimSmooth);
    float adjustedY = lerp(g_lastAimY, touchY, g_aim.aimSmooth);

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

    if (distance <= g_aim.maxDistance) {
        g_lastAimX = adjustedX;
        g_lastAimY = adjustedY;
    }

    LOGD("Silent Aim: X=%.2f Y=%.2f Dist=%.2f", adjustedX, adjustedY, distance);
}

// ============================================================================
// ESP RENDERING
// ============================================================================

void updateEnemyList() {
    g_enemies.clear();
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        PlayerEntity enemy;
        enemy.x = 100.0f + i * 50.0f;
        enemy.y = 200.0f + i * 30.0f;
        enemy.width = 40.0f;
        enemy.height = 80.0f;
        enemy.health = 100.0f;
        enemy.maxHealth = 100.0f;
        enemy.visible = true;
        enemy.alive = true;
        enemy.headX = enemy.x + enemy.width / 2.0f;
        enemy.headY = enemy.y;
        enemy.bodyX = enemy.x + enemy.width / 2.0f;
        enemy.bodyY = enemy.y + enemy.height / 2.0f;
        
        g_enemies.push_back(enemy);
    }
}

void renderESP() {
    if (!g_esp.drawBoxes && !g_esp.drawSkeletons && !g_esp.drawHealthBars) return;

    for (const auto& enemy : g_enemies) {
        if (!enemy.visible || !enemy.alive) continue;

        // Draw bounding box
        if (g_esp.drawBoxes) {
            drawBox(enemy.x, enemy.y, enemy.width, enemy.height,
                   g_esp.boxR, g_esp.boxG, g_esp.boxB, 0.8f);
        }

        // Draw skeleton
        if (g_esp.drawSkeletons) {
            drawLine(enemy.headX, enemy.headY, enemy.bodyX, enemy.bodyY,
                    0.0f, 1.0f, 0.0f, 1.0f);
            drawLine(enemy.bodyX, enemy.bodyY, enemy.legX, enemy.legY,
                    0.0f, 1.0f, 0.0f, 1.0f);
        }

        // Draw health bar
        if (g_esp.drawHealthBars) {
            float healthPercent = enemy.health / enemy.maxHealth;
            drawBox(enemy.x, enemy.y - 15.0f, enemy.width, 5.0f,
                   1.0f - healthPercent, healthPercent, 0.0f, 1.0f);
        }
    }
}

void renderMenuBackground() {
    drawBox(g_menu.x, g_menu.y, g_menu.w, g_menu.h,
           g_menu.r * 0.3f, g_menu.g * 0.3f, g_menu.b * 0.3f, 0.9f);
}

void renderMenuItems() {
    float itemY = g_menu.y + 10.0f;
    const char* items[] = {
        "Silent Aim",
        "ESP",
        "Skin Changer",
        "Settings"
    };

    for (int i = 0; i < 4; i++) {
        float itemR = 0.5f;
        float itemG = 0.5f;
        float itemB = 0.5f;

        if (i == 0 && g_aim.enabled) {
            itemR = 0.2f; itemG = 0.8f; itemB = 0.2f;
        } else if (i == 1 && g_esp.drawBoxes) {
            itemR = 1.0f; itemG = 0.5f; itemB = 0.0f;
        }

        drawBox(g_menu.x + 5.0f, itemY, g_menu.w - 10.0f, g_menu.itemHeight,
               itemR, itemG, itemB, 1.0f);

        itemY += g_menu.itemHeight + 5.0f;
    }
}

void renderMenu() {
    if (!g_menu.visible) return;
    renderMenuBackground();
    renderMenuItems();
}

// ============================================================================
// ADVANCED RENDERING & CALCULATIONS
// ============================================================================

struct BoundingBox {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

struct RaycastResult {
    bool hit;
    float distance;
    int objectId;
};

BoundingBox calculateBoundingBox(const PlayerEntity& entity) {
    BoundingBox box;
    box.minX = entity.x;
    box.minY = entity.y;
    box.minZ = entity.z - 0.1f;
    box.maxX = entity.x + entity.width;
    box.maxY = entity.y + entity.height;
    box.maxZ = entity.z + 0.1f;
    return box;
}

RaycastResult raycastTest(float rayX, float rayY, float rayDist) {
    RaycastResult result = {false, 999999.0f, -1};

    for (size_t i = 0; i < g_enemies.size(); i++) {
        const auto& enemy = g_enemies[i];
        if (!enemy.alive || !enemy.visible) continue;

        BoundingBox box = calculateBoundingBox(enemy);

        float dx = rayX - enemy.x;
        float dy = rayY - enemy.y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist < rayDist && dist < result.distance) {
            result.hit = true;
            result.distance = dist;
            result.objectId = i;
        }
    }

    return result;
}

void applyPredictiveAim(float* aimX, float* aimY, const PlayerEntity& targetEnemy) {
    if (!g_aim.predictMode) return;

    // Simple lead calculation
    float leadX = targetEnemy.headX;
    float leadY = targetEnemy.headY - 5.0f; // Aim slightly above head

    *aimX = lerp(*aimX, leadX, g_aim.aimSmooth * 0.5f);
    *aimY = lerp(*aimY, leadY, g_aim.aimSmooth * 0.5f);
}

void updateEnemyTargeting() {
    for (auto& enemy : g_enemies) {
        // Update bone positions based on entity position
        enemy.headX = enemy.x + enemy.width / 2.0f;
        enemy.headY = enemy.y;
        enemy.bodyX = enemy.x + enemy.width / 2.0f;
        enemy.bodyY = enemy.y + enemy.height * 0.6f;
        enemy.legX = enemy.x + enemy.width / 2.0f;
        enemy.legY = enemy.y + enemy.height;

        // Simulate health damage
        if (enemy.health > 0) {
            enemy.health -= 0.5f;
        } else {
            enemy.alive = false;
        }
    }
}

void renderAdvancedESP() {
    if (!g_esp.drawBoxes && !g_esp.drawSkeletons) return;

    for (size_t i = 0; i < g_enemies.size(); i++) {
        const auto& enemy = g_enemies[i];
        if (!enemy.visible || !enemy.alive) continue;

        // Render 3D bounding box outline
        if (g_esp.drawBoxes) {
            float boxColor = 1.0f - (enemy.health / 100.0f);
            drawBox(enemy.x, enemy.y, enemy.width, enemy.height,
                   1.0f, boxColor * 0.5f, boxColor * 0.2f, 0.95f);

            // Distance indicator
            float centerX = g_screenWidth / 2.0f;
            float centerY = g_screenHeight / 2.0f;
            float dx = enemy.x - centerX;
            float dy = enemy.y - centerY;
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance < g_aim.maxDistance) {
                drawLine(centerX, centerY, enemy.x, enemy.y,
                        0.0f, 1.0f, 1.0f, 0.5f);
            }
        }

        // Advanced skeleton with multiple bones
        if (g_esp.drawSkeletons) {
            // Spine
            drawLine(enemy.headX, enemy.headY,
                    enemy.bodyX, enemy.bodyY,
                    0.0f, 1.0f, 0.0f, 1.0f);

            // Legs
            drawLine(enemy.bodyX, enemy.bodyY,
                    enemy.legX, enemy.legY,
                    0.2f, 0.8f, 0.2f, 1.0f);

            // Left arm
            drawLine(enemy.bodyX - 15.0f, enemy.bodyY - 15.0f,
                    enemy.bodyX - 30.0f, enemy.bodyY + 10.0f,
                    0.1f, 1.0f, 0.1f, 0.8f);

            // Right arm
            drawLine(enemy.bodyX + 15.0f, enemy.bodyY - 15.0f,
                    enemy.bodyX + 30.0f, enemy.bodyY + 10.0f,
                    0.1f, 1.0f, 0.1f, 0.8f);
        }

        // Health visualization
        if (g_esp.drawHealthBars) {
            float healthPercent = enemy.health / 100.0f;
            healthPercent = clamp(healthPercent, 0.0f, 1.0f);

            // Background
            drawBox(enemy.x, enemy.y - 20.0f, enemy.width, 8.0f,
                   0.2f, 0.2f, 0.2f, 0.8f);

            // Health bar
            drawBox(enemy.x + 2.0f, enemy.y - 18.0f,
                   (enemy.width - 4.0f) * healthPercent, 4.0f,
                   1.0f - healthPercent, healthPercent, 0.0f, 1.0f);
        }

        // Player name/info
        if (g_esp.drawNames) {
            // Draw a small indicator box
            drawBox(enemy.x + enemy.width / 2.0f - 10.0f, enemy.y - 35.0f,
                   20.0f, 8.0f, 0.7f, 0.7f, 0.7f, 0.7f);
        }
    }
}

void skinChangerUpdate() {
    if (g_skin.selectedSkin >= 0 && g_skin.selectedSkin < (int)g_skin.unlockedSkins.size()) {
        for (auto& enemy : g_enemies) {
            enemy.skin = g_skin.unlockedSkins[g_skin.selectedSkin];
        }
    }
}

// ============================================================================
// JNI FUNCTIONS
// ============================================================================

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGD("JNI_OnLoad called");
    g_javaVM = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_init(
    JNIEnv* env, jobject thiz) {
    
    LOGD("init() called");
    if (g_initialized) return;

    initializeGraphics();
    updateEnemyList();
    g_initialized = true;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_resize(
    JNIEnv* env, jobject thiz, jint width, jint height) {
    
    LOGD("resize(%d, %d) called", width, height);
    if (!g_initialized) return;

    g_screenWidth = (float)width;
    g_screenHeight = (float)height;
    glViewport(0, 0, width, height);
}

JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_step(
    JNIEnv* env, jobject thiz) {

    if (!g_initialized) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update game state
    updateEnemyTargeting();
    skinChangerUpdate();

    // Render all visuals
    renderAdvancedESP();
    renderESP();
    renderMenu();

    glFlush();
}

JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_MotionEventClick(
    JNIEnv* env, jobject thiz, jboolean down, jfloat x, jfloat y) {
    
    if (!g_initialized) return;

    LOGD("MotionEventClick: down=%d x=%.2f y=%.2f", down, x, y);

    if (down) {
        // Check menu click
        if (x >= g_menu.x && x <= g_menu.x + g_menu.w &&
            y >= g_menu.y && y <= g_menu.y + g_menu.h) {
            int itemIndex = (int)((y - g_menu.y - 10.0f) / (g_menu.itemHeight + 5.0f));
            LOGD("Menu item clicked: %d", itemIndex);

            switch (itemIndex) {
                case 0: g_aim.enabled = !g_aim.enabled; break;
                case 1: g_esp.drawBoxes = !g_esp.drawBoxes; break;
                case 2: g_skin.selectedSkin = (g_skin.selectedSkin + 1) % 5; break;
            }
        }

        processSilentAim(x, y);
    }
}

JNIEXPORT jstring JNICALL Java_com_mycompany_application_GLES3JNIView_getWindowRect(
    JNIEnv* env, jobject thiz) {
    
    if (!g_initialized) {
        return env->NewStringUTF("");
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer),
        "{\"width\":%.0f,\"height\":%.0f,\"enemies\":%zu}",
        g_screenWidth, g_screenHeight, g_enemies.size());

    return env->NewStringUTF(buffer);
}

JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown(
    JNIEnv* env, jobject thiz) {
    
    LOGD("imgui_Shutdown() called");
    if (!g_initialized) return;

    if (g_graphics.shaderProgram) {
        glDeleteProgram(g_graphics.shaderProgram);
        g_graphics.shaderProgram = 0;
    }
    if (g_graphics.vao) {
        glDeleteVertexArrays(1, &g_graphics.vao);
        g_graphics.vao = 0;
    }
    if (g_graphics.vbo) {
        glDeleteBuffers(1, &g_graphics.vbo);
        g_graphics.vbo = 0;
    }
    if (g_graphics.ebo) {
        glDeleteBuffers(1, &g_graphics.ebo);
        g_graphics.ebo = 0;
    }

    g_initialized = false;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGD("JNI_OnUnload called");
}

} // extern "C"
