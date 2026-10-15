#pragma once
#include <cmath>

namespace glm {

struct vec3 {
    float x, y, z;
    vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct mat4 {
    float m[16];

    mat4(float val = 1.0f) {
        for(int i = 0; i < 16; i++) {
            m[i] = (i % 5 == 0) ? val : 0.0f;
        }
    }

    mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33) {
        m[0] = m00; m[4] = m01; m[8]  = m02; m[12] = m03;
        m[1] = m10; m[5] = m11; m[9]  = m12; m[13] = m13;
        m[2] = m20; m[6] = m21; m[10] = m22; m[14] = m23;
        m[3] = m30; m[7] = m31; m[11] = m32; m[15] = m33;
    }
};

inline float radians(float degrees) {
    return degrees * 3.14159265359f / 180.0f;
}

inline mat4 perspective(float fov, float aspect, float near, float far) {
    mat4 result(0.0f);
    float f = 1.0f / std::tan(fov / 2.0f);

    result.m[0] = f / aspect;
    result.m[5] = f;
    result.m[10] = (far + near) / (near - far);
    result.m[14] = (2.0f * far * near) / (near - far);
    result.m[11] = -1.0f;

    return result;
}

inline mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
    return mat4(1.0f);
}

inline float* value_ptr(mat4& m) {
    return m.m;
}

}
