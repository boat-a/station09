// STATION 09: ECHO  --  core utilities (math, RNG, input)
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <atomic>

namespace S9 {

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t i16;

static const float PI = 3.14159265358979f;

inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
inline int clampi(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float sat(float v) { return clampf(v, 0.f, 1.f); }
inline float smooth(float a, float b, float x) {
    float t = sat((x - a) / (b - a));
    return t * t * (3.f - 2.f * t);
}
inline float maxf(float a, float b) { return a > b ? a : b; }
inline float minf(float a, float b) { return a < b ? a : b; }
inline float angDiff(float a, float b) {  // shortest signed difference b-a
    float d = fmodf(b - a + PI, 2.f * PI);
    if (d < 0) d += 2.f * PI;
    return d - PI;
}

struct V3 {
    float x, y, z;
    V3() : x(0), y(0), z(0) {}
    V3(float a, float b, float c) : x(a), y(b), z(c) {}
};
inline V3 operator+(V3 a, V3 b) { return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline V3 operator-(V3 a, V3 b) { return V3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline V3 operator*(V3 a, float s) { return V3(a.x * s, a.y * s, a.z * s); }
inline float dot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline V3 cross(V3 a, V3 b) { return V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
inline float vlen(V3 a) { return sqrtf(dot(a, a)); }
inline V3 vnorm(V3 a) {
    float l = vlen(a);
    return l > 1e-6f ? a * (1.f / l) : V3(0, 0, 0);
}
inline float dist2d(V3 a, V3 b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

// 3x3 rotation matrix (row-major)
struct M3 {
    float m[9];
    static M3 I() { M3 r = {{1, 0, 0, 0, 1, 0, 0, 0, 1}}; return r; }
    static M3 rotY(float a) { float c = cosf(a), s = sinf(a); M3 r = {{c, 0, s, 0, 1, 0, -s, 0, c}}; return r; }
    static M3 rotX(float a) { float c = cosf(a), s = sinf(a); M3 r = {{1, 0, 0, 0, c, -s, 0, s, c}}; return r; }
    static M3 rotZ(float a) { float c = cosf(a), s = sinf(a); M3 r = {{c, -s, 0, s, c, 0, 0, 0, 1}}; return r; }
    V3 operator*(V3 v) const {
        return V3(m[0] * v.x + m[1] * v.y + m[2] * v.z, m[3] * v.x + m[4] * v.y + m[5] * v.z,
                  m[6] * v.x + m[7] * v.y + m[8] * v.z);
    }
    M3 operator*(const M3& o) const {
        M3 r;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                float s = 0;
                for (int k = 0; k < 3; k++) s += m[i * 3 + k] * o.m[k * 3 + j];
                r.m[i * 3 + j] = s;
            }
        return r;
    }
};

struct RNG {
    u32 s = 2463534242u;
    u32 next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float f() { return (next() & 0xFFFFFF) / 16777216.0f; }
    float range(float a, float b) { return a + (b - a) * f(); }
    int irange(int a, int b) { return a + (int)(next() % (u32)(b - a + 1)); }
    bool chance(float p) { return f() < p; }
};
inline RNG grng;  // gameplay RNG
inline RNG vrng;  // visual-only RNG

// Key codes (match Windows virtual-key codes for the ones we use)
enum Key {
    K_BACK = 8, K_TAB = 9, K_ENTER = 13, K_SHIFT = 16, K_CTRL = 17, K_ESC = 27, K_SPACE = 32,
    K_LEFT = 37, K_UP = 38, K_RIGHT = 39, K_DOWN = 40, K_F11 = 0x7A
};

struct Input {
    bool key[256], prev[256];
    bool mouse[3], mprev[3];
    float mdx, mdy;
    Input() { memset(this, 0, sizeof(*this)); }
    bool down(int k) const { return key[k & 255]; }
    bool pressed(int k) const { return key[k & 255] && !prev[k & 255]; }
    bool mdown(int b) const { return mouse[b]; }
    bool mpressed(int b) const { return mouse[b] && !mprev[b]; }
    void endFrame() {
        memcpy(prev, key, sizeof(key));
        memcpy(mprev, mouse, sizeof(mouse));
        mdx = mdy = 0;
    }
};

}  // namespace S9
