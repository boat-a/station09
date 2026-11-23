// STATION 09: ECHO  --  software renderer with a PS2 / early-2000s look
// 320x240 internal resolution, nearest-neighbour 64x64 textures, vertex lighting,
// vertex snapping, heavy fog, 5-bit colour + ordered dithering, film grain.
#pragma once
#include "core.hpp"

namespace S9 {

static const int SW = 320;
static const int SH = 240;

enum TexId {
    T_CONC, T_CONCD, T_METAL, T_TILEW, T_WARN, T_FLESH, T_FLOORC, T_FLOORT, T_GRATE, T_BLOOD,
    T_CARPET, T_CEIL, T_CEILP, T_DOOR, T_DOORP, T_DOORS, T_CRATE, T_LOCKER, T_DESK, T_TERM,
    T_TERMOFF, T_GEN, T_BOX, T_ELEV, T_SKIN, T_FACE, T_PAPER, T_LIGHT, T_BLACK, T_FLESHF,
    T_HAND, T_COUNT
};

struct Tex { u32 px[64 * 64]; };
struct PV { float x, y, z, u, v, r, g, b; };
typedef std::function<V3(const V3&, const V3&)> LightFn;

// ---------- noise helpers (tileable) ----------
inline u32 hash3(u32 x, u32 y, u32 s) {
    u32 h = x * 374761393u + y * 668265263u + s * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
inline float hf(int x, int y, int s) { return (hash3((u32)x, (u32)y, (u32)s) & 0xFFFF) / 65535.0f; }
inline float tnoise(float u, float v, int freq, int seed) {
    float x = u * freq, y = v * freq;
    int xi = (int)floorf(x), yi = (int)floorf(y);
    float fx = x - xi, fy = y - yi;
    fx = fx * fx * (3 - 2 * fx);
    fy = fy * fy * (3 - 2 * fy);
    auto h = [&](int a, int b) { return hf(((a % freq) + freq) % freq, ((b % freq) + freq) % freq, seed); };
    return lerpf(lerpf(h(xi, yi), h(xi + 1, yi), fx), lerpf(h(xi, yi + 1), h(xi + 1, yi + 1), fx), fy);
}
inline float tfbm(float u, float v, int seed, int f0 = 4, int oct = 4) {
    float a = 0.5f, s = 0, n = 0;
    int f = f0;
    for (int i = 0; i < oct; i++) { s += a * tnoise(u, v, f, seed + i * 17); n += a; a *= 0.5f; f *= 2; }
    return s / n;
}
inline u32 packRGB(float r, float g, float b, float a = 1.f) {
    int R = clampi((int)(r * 255), 0, 255), G = clampi((int)(g * 255), 0, 255), B = clampi((int)(b * 255), 0, 255);
    return ((a > 0.5f ? 255u : 0u) << 24) | (R << 16) | (G << 8) | B;
}
inline u32 rgb8(int r, int g, int b) { return 0xFF000000u | (clampi(r, 0, 255) << 16) | (clampi(g, 0, 255) << 8) | clampi(b, 0, 255); }

// ---------- 3x5 pixel font ----------
struct Font {
    u16 g[128];
    Font() { memset(g, 0, sizeof(g)); init(); }
    void add(char c, const char* a, const char* b, const char* d, const char* e, const char* f) {
        const char* rows[5] = {a, b, d, e, f};
        u16 bits = 0;
        for (int r = 0; r < 5; r++)
            for (int k = 0; k < 3; k++)
                if (rows[r][k] == '#') bits |= 1 << (r * 3 + k);
        g[(int)c] = bits;
    }
    void init() {
        add('A', ".#.", "#.#", "###", "#.#", "#.#"); add('B', "##.", "#.#", "##.", "#.#", "##.");
        add('C', ".##", "#..", "#..", "#..", ".##"); add('D', "##.", "#.#", "#.#", "#.#", "##.");
        add('E', "###", "#..", "##.", "#..", "###"); add('F', "###", "#..", "##.", "#..", "#..");
        add('G', ".##", "#..", "#.#", "#.#", ".##"); add('H', "#.#", "#.#", "###", "#.#", "#.#");
        add('I', "###", ".#.", ".#.", ".#.", "###"); add('J', "..#", "..#", "..#", "#.#", ".#.");
        add('K', "#.#", "#.#", "##.", "#.#", "#.#"); add('L', "#..", "#..", "#..", "#..", "###");
        add('M', "#.#", "###", "###", "#.#", "#.#"); add('N', "##.", "#.#", "#.#", "#.#", "#.#");
        add('O', ".#.", "#.#", "#.#", "#.#", ".#."); add('P', "##.", "#.#", "##.", "#..", "#..");
        add('Q', ".#.", "#.#", "#.#", "##.", ".##"); add('R', "##.", "#.#", "##.", "#.#", "#.#");
        add('S', ".##", "#..", ".#.", "..#", "##."); add('T', "###", ".#.", ".#.", ".#.", ".#.");
        add('U', "#.#", "#.#", "#.#", "#.#", "###"); add('V', "#.#", "#.#", "#.#", "#.#", ".#.");
        add('W', "#.#", "#.#", "###", "###", "#.#"); add('X', "#.#", "#.#", ".#.", "#.#", "#.#");
        add('Y', "#.#", "#.#", ".#.", ".#.", ".#."); add('Z', "###", "..#", ".#.", "#..", "###");
        add('0', "###", "#.#", "#.#", "#.#", "###"); add('1', ".#.", "##.", ".#.", ".#.", "###");
        add('2', "##.", "..#", ".#.", "#..", "###"); add('3', "##.", "..#", ".#.", "..#", "##.");
        add('4', "#.#", "#.#", "###", "..#", "..#"); add('5', "###", "#..", "##.", "..#", "##.");
        add('6', ".##", "#..", "###", "#.#", "###"); add('7', "###", "..#", ".#.", ".#.", ".#.");
        add('8', "###", "#.#", "###", "#.#", "###"); add('9', "###", "#.#", "###", "..#", "##.");
        add('.', "...", "...", "...", "...", ".#."); add(',', "...", "...", "...", ".#.", "#..");
        add(':', "...", ".#.", "...", ".#.", "..."); add(';', "...", ".#.", "...", ".#.", "#..");
        add('!', ".#.", ".#.", ".#.", "...", ".#."); add('?', "##.", "..#", ".#.", "...", ".#.");
        add('-', "...", "...", "###", "...", "..."); add('+', "...", ".#.", "###", ".#.", "...");
        add('\'', ".#.", ".#.", "...", "...", "..."); add('"', "#.#", "#.#", "...", "...", "...");
        add('/', "..#", "..#", ".#.", "#..", "#.."); add('(', ".#.", "#..", "#..", "#..", ".#.");
        add(')', ".#.", "..#", "..#", "..#", ".#."); add('[', "##.", "#..", "#..", "#..", "##.");
        add(']', ".##", "..#", "..#", "..#", ".##"); add('_', "...", "...", "...", "...", "###");
        add('%', "#.#", "..#", ".#.", "#..", "#.#"); add('<', "..#", ".#.", "#..", ".#.", "..#");
        add('>', "#..", ".#.", "..#", ".#.", "#.."); add('=', "...", "###", "...", "###", "...");
        add('*', "#.#", ".#.", "###", ".#.", "#.#"); add('#', "#.#", "###", "#.#", "###", "#.#");
        add('@', ".#.", "#.#", "###", "#..", ".##");
    }
};

// ---------- post processing parameters ----------
struct Post {
    float fade = 0;      // 0 = none, 1 = black
    float vign = 0.55f;  // vignette strength
    float grain = 0.07f;
    float chroma = 0;    // chromatic aberration in pixels
    int shakeX = 0, shakeY = 0;
    float red = 0;       // red flash (damage)
    float noise = 0;     // static overlay
    float flash = 0;     // white flash
    float desat = 0;
    float interlace = 0.10f;
};

struct Gfx {
    u32 fb[SW * SH];
    u32 out[SW * SH];
    float zb[SW * SH];
    float vmask[SW * SH];
    Tex tex[T_COUNT];
    Font font;
    V3 cp, rt, upv, fw;
    float F = 208.f;
    u32 frame = 0;

    void init() {
        for (int y = 0; y < SH; y++)
            for (int x = 0; x < SW; x++) {
                float dx = (x - SW * 0.5f) / (SW * 0.5f), dy = (y - SH * 0.5f) / (SH * 0.5f);
                float d = sqrtf(dx * dx * 0.8f + dy * dy * 1.0f);
                vmask[y * SW + x] = sat((d - 0.35f) / 0.95f);
            }
        initTextures();
    }

    template <class FN>
    void mk(int id, FN f) {
        for (int y = 0; y < 64; y++)
            for (int x = 0; x < 64; x++) {
                float r = 0, g = 0, b = 0, a = 1;
                f(x / 64.f, y / 64.f, x, y, r, g, b, a);
                tex[id].px[y * 64 + x] = packRGB(r, g, b, a);
            }
    }

    void initTextures() {
        typedef float& R;
        mk(T_CONC, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 1, 4, 4), s = tfbm(u, v, 9, 8, 2);
            float k = 0.34f + 0.36f * n;
            if (x % 32 == 0 || y % 32 == 0) k *= 0.55f;
            if (x % 32 == 1 || y % 32 == 1) k *= 0.8f;
            k *= 0.8f + 0.3f * s;
            r = k * 0.92f; g = k * 0.95f; b = k;
        });
        mk(T_CONCD, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 3, 4, 4), st = tfbm(u, v, 11, 2, 3);
            float k = 0.28f + 0.3f * n;
            if (x % 32 == 0 || y % 32 == 0) k *= 0.5f;
            float stain = smooth(0.5f, 0.75f, st);
            r = k * (1.0f - 0.4f * stain) * 0.95f; g = k * (1.0f - 0.35f * stain); b = k * (1.0f - 0.55f * stain);
        });
        mk(T_METAL, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 5, 16, 2);
            float k = 0.36f + 0.16f * n;
            int px = x % 32, py = y % 32;
            if (px < 2 || py < 2) k *= 0.55f;
            if (px == 2 || py == 2) k *= 1.25f;
            if ((px == 5 || px == 26) && (py == 5 || py == 26)) k = 0.62f;
            r = k * 0.85f; g = k * 0.92f; b = k * 1.0f;
        });
        mk(T_TILEW, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 7, 4, 3);
            float k = 0.62f + 0.22f * n;
            if (x % 16 == 0 || y % 16 == 0) k *= 0.55f;
            if (tnoise(u, v, 16, 21) > 0.86f) k *= 0.7f;
            r = k * 0.9f; g = k * 0.95f; b = k * 0.85f;
        });
        mk(T_WARN, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 13, 4, 3);
            bool st = ((x + y) / 8) % 2 == 0;
            float k = 0.6f + 0.4f * n;
            if (st) { r = 0.75f * k; g = 0.62f * k; b = 0.08f * k; }
            else r = g = b = 0.08f * k;
        });
        mk(T_FLESH, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 17, 4, 4);
            float vein = fabsf(sinf((u * 3.f + n * 2.0f) * 2.f * PI));
            float veins = smooth(0.8f, 1.0f, 1.f - vein);
            float k = 0.22f + 0.33f * n;
            r = k * 1.1f + 0.25f * veins; g = k * 0.36f; b = k * 0.32f;
            if (tnoise(u, v, 8, 91) > 0.8f) { r += 0.15f; g += 0.05f; }
        });
        mk(T_FLESHF, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 19, 4, 4);
            float k = 0.16f + 0.22f * n;
            float p = smooth(0.6f, 0.75f, tfbm(u, v, 31, 3, 3));
            r = k * 1.2f + 0.2f * p; g = k * 0.4f; b = k * 0.38f;
            if (((x / 8) + (y / 8)) % 2 == 0) { r *= 0.85f; g *= 0.85f; b *= 0.85f; }
        });
        mk(T_FLOORC, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 23, 4, 4), c = tnoise(u, v, 12, 5);
            float k = 0.26f + 0.3f * n;
            if (c > 0.88f) k *= 0.5f;
            if (x % 32 == 0 || y % 32 == 0) k *= 0.6f;
            r = k * 0.95f; g = k; b = k * 1.02f;
        });
        mk(T_FLOORT, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 25, 8, 3);
            bool ch = ((x / 16) + (y / 16)) % 2 == 0;
            float k = (ch ? 0.52f : 0.22f) * (0.75f + 0.35f * n);
            if (x % 16 == 0 || y % 16 == 0) k *= 0.5f;
            r = k * 0.85f; g = k * 1.0f; b = k * 0.92f;
        });
        mk(T_GRATE, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 27, 8, 2);
            bool hole = (x % 8 > 1 && x % 8 < 7) && (y % 8 > 1 && y % 8 < 7);
            float k = hole ? 0.03f : 0.32f + 0.2f * n;
            r = k * 0.9f; g = k * 0.95f; b = k;
        });
        mk(T_BLOOD, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 23, 4, 4);
            float k = 0.26f + 0.3f * n;
            float s = tfbm(u, v, 41, 3, 4);
            float bl = smooth(0.48f, 0.58f, s);
            r = k * 0.95f * (1 - bl) + bl * (0.32f + 0.25f * n);
            g = k * (1 - bl) + bl * 0.02f; b = k * 1.02f * (1 - bl) + bl * 0.02f;
        });
        mk(T_CARPET, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 29, 16, 2), sp = hf(x, y, 5);
            float k = 0.22f + 0.2f * n + 0.06f * sp;
            r = k * 0.7f; g = k * 0.75f; b = k * 1.0f;
        });
        mk(T_CEIL, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 33, 4, 3);
            float k = 0.45f + 0.25f * n;
            if (x % 32 < 2 || y % 32 < 2) k *= 0.45f;
            float st = smooth(0.65f, 0.8f, tfbm(u, v, 37, 2, 3));
            k *= 1.f - 0.35f * st;
            r = k * 0.95f; g = k * 0.95f; b = k * 0.9f;
        });
        mk(T_CEILP, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 35, 8, 2);
            int py = y % 32;
            float k = 0.22f + 0.15f * n;
            if (py > 4 && py < 14) { float sh = 1.f - fabsf(py - 9) / 5.f; k = 0.28f + 0.3f * sh * (0.8f + 0.3f * n); }
            if (py > 20 && py < 26) { float sh = 1.f - fabsf(py - 23) / 3.f; k = 0.22f + 0.22f * sh; }
            if (x % 32 == 0 && py > 4 && py < 14) k *= 0.5f;
            r = k * 0.8f; g = k * 0.85f; b = k * 0.9f;
        });
        mk(T_DOOR, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 43, 8, 2);
            float k = 0.42f + 0.14f * n;
            if (x < 3 || x > 60 || y < 3 || y > 60) k *= 0.55f;
            if (x > 18 && x < 46 && y > 8 && y < 26) { float gr = 0.04f + 0.05f * (y - 8) / 18.f; r = g = b = gr; return; }
            if (x > 50 && x < 55 && y > 30 && y < 42) k = 0.7f;
            if (y % 16 == 0) k *= 0.7f;
            r = k * 0.78f; g = k * 0.85f; b = k * 0.9f;
        });
        mk(T_DOORP, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 43, 8, 2);
            float k = 0.38f + 0.14f * n;
            if (x < 3 || x > 60 || y < 3 || y > 60) k *= 0.55f;
            r = k * 0.7f; g = k * 0.8f; b = k * 0.9f;
            if (y > 4 && y < 9) { r = 0.8f; g = 0.1f; b = 0.08f; }
            if (x > 10 && x < 54 && y > 16 && y < 58 && ((y / 4) % 2 == 0)) { r *= 0.8f; g *= 0.8f; b *= 0.8f; }
        });
        mk(T_DOORS, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 47, 8, 2);
            float k = 0.34f + 0.12f * n;
            bool st = ((x + y) / 6) % 2 == 0;
            if (y < 10 || y > 54) { r = (st ? 0.7f : 0.07f) * (0.6f + n); g = (st ? 0.55f : 0.07f) * (0.6f + n); b = 0.06f; return; }
            if (x < 3 || x > 60) k *= 0.5f;
            r = k * 0.75f; g = k * 0.8f; b = k * 0.85f;
            if (x > 44 && x < 58 && y > 24 && y < 40) { r = 0.05f; g = 0.35f + 0.2f * (y % 2); b = 0.1f; }
        });
        mk(T_CRATE, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 51, 8, 3);
            float k = 0.32f + 0.25f * n;
            if (y % 16 == 0) k *= 0.55f;
            if (x < 4 || x > 59 || y < 4 || y > 59) k *= 0.7f;
            if (abs(x - y) < 3 || abs(x + y - 63) < 3) k *= 0.75f;
            r = k * 1.0f; g = k * 0.72f; b = k * 0.42f;
        });
        mk(T_LOCKER, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 53, 8, 2);
            float k = 0.34f + 0.12f * n;
            if (x < 3 || x > 60 || y < 3 || y > 60) k *= 0.55f;
            if (x == 32 || x == 31) k *= 0.5f;
            if (y > 8 && y < 22 && (x % 6 > 1)) k *= 0.35f;
            if ((x == 26 || x == 37) && y > 30 && y < 40) k = 0.7f;
            r = k * 0.6f; g = k * 0.8f; b = k * 0.82f;
        });
        mk(T_DESK, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 55, 4, 3);
            float grain = sinf((y + n * 20.f) * 0.9f) * 0.05f;
            float k = 0.3f + 0.2f * n + grain;
            if (x < 2 || x > 61 || y < 2 || y > 61) k *= 0.6f;
            r = k * 0.95f; g = k * 0.65f; b = k * 0.4f;
        });
        mk(T_TERM, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 57, 8, 2);
            bool frame = x < 4 || x > 59 || y < 4 || y > 59;
            if (frame) { r = g = b = 0.12f + 0.05f * n; return; }
            float line = ((y / 4) % 2 == 0 && hf(x / 4, y / 4, 3) > 0.35f) ? 1.f : 0.f;
            float scan = (y % 2) ? 0.85f : 1.f;
            r = 0.05f + 0.05f * line; g = (0.28f + 0.6f * line) * scan; b = 0.1f + 0.1f * line;
        });
        mk(T_TERMOFF, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 59, 4, 2);
            bool frame = x < 4 || x > 59 || y < 4 || y > 59;
            float k = frame ? 0.14f + 0.05f * n : 0.03f + 0.03f * n;
            r = g = b = k;
        });
        mk(T_GEN, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 61, 8, 3);
            float k = 0.3f + 0.2f * n;
            r = k * 0.6f; g = k * 0.75f; b = k * 0.45f;
            if (y % 16 == 0 || x % 32 == 0) { r *= 0.5f; g *= 0.5f; b *= 0.5f; }
            float dx = x - 20, dy = y - 20;
            if (dx * dx + dy * dy < 90) { r = g = b = 0.65f; if (dx * dx + dy * dy < 40) { r = 0.1f; g = 0.1f; b = 0.1f; if (fabsf(dx - dy) < 1.5f) r = 0.9f; } }
            if (x > 36 && y > 36 && (x % 4 < 2)) { r = g = b = 0.08f; }
            if (y > 4 && y < 8 && x > 40 && x < 56) { r = 0.9f; g = 0.5f; b = 0.1f; }
        });
        mk(T_BOX, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 63, 8, 3);
            float k = 0.3f + 0.18f * n;
            if (x < 3 || x > 60 || y < 3 || y > 60) k *= 0.6f;
            r = k * 0.8f; g = k * 0.85f; b = k * 0.9f;
        });
        mk(T_ELEV, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 65, 16, 2);
            float k = 0.48f + 0.2f * n + 0.06f * sinf(x * 1.3f);
            if (x == 32 || x == 31) k *= 0.3f;
            if (x < 3 || x > 60 || y < 3 || y > 60) k *= 0.5f;
            r = k * 0.88f; g = k * 0.9f; b = k * 0.95f;
            if (y < 8 && x > 24 && x < 40) { r = 0.8f; g = 0.15f; b = 0.1f; }
        });
        mk(T_SKIN, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 67, 8, 4), vn = tfbm(u, v, 71, 4, 3);
            float vein = smooth(0.62f, 0.74f, vn);
            float k = 0.55f + 0.25f * n;
            r = k * 0.88f - 0.2f * vein; g = k * 0.84f - 0.2f * vein; b = k * 0.82f - 0.05f * vein;
            if (vein > 0.5f) { r = 0.4f; g = 0.22f; b = 0.28f; }
        });
        mk(T_FACE, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 67, 8, 4);
            float k = 0.5f + 0.25f * n;
            r = k * 0.9f; g = k * 0.86f; b = k * 0.84f;
            float ex1 = (x - 20) * (x - 20) + (y - 22) * (y - 22), ex2 = (x - 44) * (x - 44) + (y - 24) * (y - 24);
            if (ex1 < 26 || ex2 < 26) r = g = b = 0.01f;
            if (ex1 < 6 || ex2 < 6) { r = 0.9f; g = 0.9f; b = 0.8f; }
            float mx = (x - 32) / 9.f, my = (y - 44) / 17.f;
            if (mx * mx + my * my < 1.f) {
                r = g = b = 0.02f;
                if (y > 36 && y < 52 && (x % 4 == 0)) { r = 0.75f; g = 0.72f; b = 0.6f; }
            }
        });
        mk(T_PAPER, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 73, 8, 2);
            float k = 0.78f + 0.12f * n;
            if (y % 8 == 4 && x > 6 && x < 58 && hf(x / 3, y, 9) > 0.25f) k = 0.25f;
            r = k; g = k * 0.97f; b = k * 0.88f;
        });
        mk(T_LIGHT, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float k = (x < 3 || x > 60 || y < 3 || y > 60) ? 0.6f : 1.f;
            r = k; g = k; b = k * 0.88f;
        });
        mk(T_BLACK, [&](float u, float v, int x, int y, R r, R g, R b, R a) { r = g = b = 0.01f; });
        mk(T_HAND, [&](float u, float v, int x, int y, R r, R g, R b, R a) {
            float n = tfbm(u, v, 79, 8, 3);
            float k = 0.3f + 0.3f * n;
            float p = smooth(0.52f, 0.62f, tfbm(u, v, 83, 4, 4));
            r = k * (1 - p) + p * 0.3f; g = k * (1 - p) + p * 0.02f; b = k * (1 - p) + p * 0.02f;
        });
    }

    // ---------- camera / frame ----------
    void setCam(V3 p, float yaw, float pitch) {
        cp = p;
        float cy = cosf(yaw), sy = sinf(yaw), cpch = cosf(pitch), sp = sinf(pitch);
        fw = V3(sy * cpch, sp, cy * cpch);
        rt = V3(cy, 0, -sy);
        upv = V3(-sy * sp, cpch, -cy * sp);
    }
    void clear(u32 col) {
        for (int i = 0; i < SW * SH; i++) { fb[i] = col; zb[i] = 0; }
    }
    V3 toView(const V3& w) const {
        V3 d = w - cp;
        return V3(dot(d, rt), dot(d, upv), dot(d, fw));
    }

    // ---------- triangle rasterizer ----------
    void raster(const PV& A, const PV& B, const PV& C, int tid, int blend, float alpha) {
        float izA = 1.f / A.z, izB = 1.f / B.z, izC = 1.f / C.z;
        float ax = floorf(SW * 0.5f + A.x * izA * F + 0.5f), ay = floorf(SH * 0.5f - A.y * izA * F + 0.5f);
        float bx = floorf(SW * 0.5f + B.x * izB * F + 0.5f), by = floorf(SH * 0.5f - B.y * izB * F + 0.5f);
        float cx = floorf(SW * 0.5f + C.x * izC * F + 0.5f), cy = floorf(SH * 0.5f - C.y * izC * F + 0.5f);
        float area = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
        if (fabsf(area) < 0.5f) return;
        int minx = (int)maxf(0, minf(ax, minf(bx, cx))), maxx = (int)minf(SW - 1, maxf(ax, maxf(bx, cx)));
        int miny = (int)maxf(0, minf(ay, minf(by, cy))), maxy = (int)minf(SH - 1, maxf(ay, maxf(by, cy)));
        if (minx > maxx || miny > maxy) return;
        float inv = 1.f / area;
        float x0 = minx + 0.5f, y0 = miny + 0.5f;
        float w0r = ((cx - bx) * (y0 - by) - (cy - by) * (x0 - bx)) * inv, d0x = -(cy - by) * inv, d0y = (cx - bx) * inv;
        float w1r = ((ax - cx) * (y0 - cy) - (ay - cy) * (x0 - cx)) * inv, d1x = -(ay - cy) * inv, d1y = (ax - cx) * inv;
        float w2r = ((bx - ax) * (y0 - ay) - (by - ay) * (x0 - ax)) * inv, d2x = -(by - ay) * inv, d2y = (bx - ax) * inv;
        float uzA = A.u * izA, uzB = B.u * izB, uzC = C.u * izC;
        float vzA = A.v * izA, vzB = B.v * izB, vzC = C.v * izC;
        const Tex& T = tex[tid];
        for (int y = miny; y <= maxy; y++) {
            float w0 = w0r, w1 = w1r, w2 = w2r;
            bool was = false;
            u32* frow = fb + y * SW;
            float* zrow = zb + y * SW;
            for (int x = minx; x <= maxx; x++, w0 += d0x, w1 += d1x, w2 += d2x) {
                if (w0 < 0 || w1 < 0 || w2 < 0) { if (was) break; continue; }
                was = true;
                float iz = w0 * izA + w1 * izB + w2 * izC;
                if (iz <= zrow[x]) continue;
                float z = 1.f / iz;
                float u = (w0 * uzA + w1 * uzB + w2 * uzC) * z, v = (w0 * vzA + w1 * vzB + w2 * vzC) * z;
                u32 c = T.px[(((int)floorf(v * 64.f) & 63) << 6) | ((int)floorf(u * 64.f) & 63)];
                if (!(c >> 24)) continue;
                float lr = w0 * A.r + w1 * B.r + w2 * C.r, lg = w0 * A.g + w1 * B.g + w2 * C.g, lb = w0 * A.b + w1 * B.b + w2 * C.b;
                int rr = (int)(((c >> 16) & 255) * lr), gg = (int)(((c >> 8) & 255) * lg), bb = (int)((c & 255) * lb);
                if (blend == 0) {
                    frow[x] = rgb8(rr, gg, bb);
                    zrow[x] = iz;
                } else if (blend == 1) {  // additive
                    u32 d = frow[x];
                    frow[x] = rgb8((int)((d >> 16 & 255) + rr * alpha), (int)((d >> 8 & 255) + gg * alpha), (int)((d & 255) + bb * alpha));
                } else {  // alpha
                    u32 d = frow[x];
                    int dr = d >> 16 & 255, dg = d >> 8 & 255, db = d & 255;
                    frow[x] = rgb8((int)(dr + (rr - dr) * alpha), (int)(dg + (gg - dg) * alpha), (int)(db + (bb - db) * alpha));
                }
            }
            w0r += d0y; w1r += d1y; w2r += d2y;
        }
    }

    static PV lerpPV(const PV& p, const PV& q, float t) {
        PV r;
        r.x = lerpf(p.x, q.x, t); r.y = lerpf(p.y, q.y, t); r.z = lerpf(p.z, q.z, t);
        r.u = lerpf(p.u, q.u, t); r.v = lerpf(p.v, q.v, t);
        r.r = lerpf(p.r, q.r, t); r.g = lerpf(p.g, q.g, t); r.b = lerpf(p.b, q.b, t);
        return r;
    }
    void tri(const PV& a, const PV& b, const PV& c, int tid, int blend = 0, float alpha = 1.f) {
        const float NEAR = 0.08f;
        const PV in[3] = {a, b, c};
        PV o[4];
        int n = 0;
        for (int i = 0; i < 3; i++) {
            const PV& p = in[i];
            const PV& q = in[(i + 1) % 3];
            bool pin = p.z >= NEAR, qin = q.z >= NEAR;
            if (pin) o[n++] = p;
            if (pin != qin) o[n++] = lerpPV(p, q, (NEAR - p.z) / (q.z - p.z));
        }
        if (n < 3) return;
        raster(o[0], o[1], o[2], tid, blend, alpha);
        if (n == 4) raster(o[0], o[2], o[3], tid, blend, alpha);
    }

    // world-space quad with per-corner light colour
    void quadW(const V3 p[4], const float uu[4], const float vv[4], const V3 L[4], int tid, int blend = 0, float alpha = 1.f) {
        PV pv[4];
        bool any = false;
        for (int i = 0; i < 4; i++) {
            V3 v = toView(p[i]);
            pv[i] = {v.x, v.y, v.z, uu[i], vv[i], L[i].x, L[i].y, L[i].z};
            if (v.z > 0.08f) any = true;
        }
        if (!any) return;
        tri(pv[0], pv[1], pv[2], tid, blend, alpha);
        tri(pv[0], pv[2], pv[3], tid, blend, alpha);
    }

    // oriented box. front-face (+Z local) may use a different texture
    void box(V3 c, V3 half, const M3& R, int tid, const LightFn& lf, int frontTex = -1, V3 emis = V3(0, 0, 0), float rep = 1.f) {
        static const int FS[6][4][3] = {
            {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},      // +Z
            {{1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}},  // -Z
            {{1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1}},      // +X
            {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}},  // -X
            {{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}, {-1, 1, 1}},      // +Y
            {{-1, -1, 1}, {1, -1, 1}, {1, -1, -1}, {-1, -1, -1}}   // -Y
        };
        static const float FN[6][3] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
        static const float UU[4] = {0, 1, 1, 0}, VV[4] = {1, 1, 0, 0};
        for (int f = 0; f < 6; f++) {
            V3 n = R * V3(FN[f][0], FN[f][1], FN[f][2]);
            V3 p[4];
            for (int i = 0; i < 4; i++)
                p[i] = c + R * V3(FS[f][i][0] * half.x, FS[f][i][1] * half.y, FS[f][i][2] * half.z);
            if (dot(n, cp - p[0]) <= 0) continue;
            V3 L[4];
            for (int i = 0; i < 4; i++) {
                L[i] = lf(p[i], n);
                L[i] = V3(L[i].x + emis.x, L[i].y + emis.y, L[i].z + emis.z);
            }
            float uu[4], vv[4];
            for (int i = 0; i < 4; i++) { uu[i] = UU[i] * rep; vv[i] = VV[i] * rep; }
            quadW(p, uu, vv, L, (f == 0 && frontTex >= 0) ? frontTex : tid);
        }
    }

    // additive glow sprite (depth tested at centre)
    void glow(V3 w, float size, float r, float g, float b, float amount = 1.f) {
        V3 v = toView(w);
        if (v.z < 0.3f) return;
        float sx = SW * 0.5f + v.x / v.z * F, sy = SH * 0.5f - v.y / v.z * F;
        int cxp = (int)sx, cyp = (int)sy;
        if (cxp < 0 || cxp >= SW || cyp < 0 || cyp >= SH) return;
        if (1.f / v.z < zb[cyp * SW + cxp] * 0.98f) return;
        float rad = clampf(size * F / v.z, 2.f, 40.f);
        int ri = (int)rad + 1;
        for (int y = maxi(0, cyp - ri); y <= mini(SH - 1, cyp + ri); y++)
            for (int x = maxi(0, cxp - ri); x <= mini(SW - 1, cxp + ri); x++) {
                float d = sqrtf((float)((x - cxp) * (x - cxp) + (y - cyp) * (y - cyp))) / rad;
                if (d >= 1.f) continue;
                float k = (1.f - d) * (1.f - d) * amount;
                u32 c = fb[y * SW + x];
                fb[y * SW + x] = rgb8((int)((c >> 16 & 255) + 255 * r * k), (int)((c >> 8 & 255) + 255 * g * k), (int)((c & 255) + 255 * b * k));
            }
    }
    static int maxi(int a, int b) { return a > b ? a : b; }
    static int mini(int a, int b) { return a < b ? a : b; }

    // ---------- 2D drawing ----------
    void put(int x, int y, u32 col) { if ((unsigned)x < SW && (unsigned)y < SH) fb[y * SW + x] = col; }
    void rect(int x, int y, int w, int h, u32 col, float alpha = 1.f) {
        int x0 = maxi(0, x), y0 = maxi(0, y), x1 = mini(SW, x + w), y1 = mini(SH, y + h);
        int cr = col >> 16 & 255, cg = col >> 8 & 255, cb = col & 255;
        for (int j = y0; j < y1; j++)
            for (int i = x0; i < x1; i++) {
                if (alpha >= 0.999f) { fb[j * SW + i] = col | 0xFF000000u; continue; }
                u32 d = fb[j * SW + i];
                int dr = d >> 16 & 255, dg = d >> 8 & 255, db = d & 255;
                fb[j * SW + i] = rgb8((int)(dr + (cr - dr) * alpha), (int)(dg + (cg - dg) * alpha), (int)(db + (cb - db) * alpha));
            }
    }
    void frame2d(int x, int y, int w, int h, u32 col) {
        rect(x, y, w, 1, col); rect(x, y + h - 1, w, 1, col); rect(x, y, 1, h, col); rect(x + w - 1, y, 1, h, col);
    }
    void line(int x0, int y0, int x1, int y1, u32 col) {
        int dx = abs(x1 - x0), dy = -abs(y1 - y0), sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx + dy;
        for (;;) {
            put(x0, y0, col);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
    void glyph(int x, int y, char c, u32 col, int s) {
        if (c >= 'a' && c <= 'z') c -= 32;
        if ((unsigned)c >= 128) return;
        u16 bits = font.g[(int)c];
        for (int r = 0; r < 5; r++)
            for (int k = 0; k < 3; k++)
                if (bits & (1 << (r * 3 + k))) rect(x + k * s, y + r * s, s, s, col);
    }
    int textW(const char* s, int sc = 1) const {
        int mx = 0, cur = 0;
        for (; *s; s++) {
            if (*s == '\n') { mx = std::max(mx, cur); cur = 0; } else cur++;
        }
        mx = std::max(mx, cur);
        return mx * 4 * sc - sc;
    }
    void text(int x, int y, const char* s, u32 col = 0xFFC8C8C8u, int sc = 1, bool shadow = true) {
        int cx = x, cy = y;
        for (; *s; s++) {
            if (*s == '\n') { cx = x; cy += 6 * sc; continue; }
            if (shadow) glyph(cx + sc, cy + sc, *s, 0xFF000000u, sc);
            glyph(cx, cy, *s, col, sc);
            cx += 4 * sc;
        }
    }
    void textc(int cxm, int y, const char* s, u32 col = 0xFFC8C8C8u, int sc = 1, bool shadow = true) {
        std::string tmp = s;
        size_t p = 0;
        int yy = y;
        for (;;) {
            size_t q = tmp.find('\n', p);
            std::string line = tmp.substr(p, q == std::string::npos ? std::string::npos : q - p);
            text(cxm - textW(line.c_str(), sc) / 2, yy, line.c_str(), col, sc, shadow);
            yy += 6 * sc;
            if (q == std::string::npos) break;
            p = q + 1;
        }
    }

    // ---------- post processing: PS2-like colour crush, dither, grain, vignette ----------
    void post(const Post& p) {
        static const int bayer[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5};
        frame++;
        int co = (int)roundf(p.chroma);
        for (int y = 0; y < SH; y++) {
            int sy = clampi(y + p.shakeY, 0, SH - 1);
            bool dark = ((y + frame) & 1) != 0;
            for (int x = 0; x < SW; x++) {
                int sx = clampi(x + p.shakeX, 0, SW - 1);
                u32 c = fb[sy * SW + sx];
                int r = c >> 16 & 255, g = c >> 8 & 255, b = c & 255;
                if (co) {
                    r = fb[sy * SW + clampi(sx - co, 0, SW - 1)] >> 16 & 255;
                    b = fb[sy * SW + clampi(sx + co, 0, SW - 1)] & 255;
                }
                float fr = (float)r, fg = (float)g, fbv = (float)b;
                if (p.desat > 0) {
                    float l = fr * 0.3f + fg * 0.59f + fbv * 0.11f;
                    fr = lerpf(fr, l, p.desat); fg = lerpf(fg, l, p.desat); fbv = lerpf(fbv, l, p.desat);
                }
                float vm = 1.f - p.vign * vmask[y * SW + x];
                fr *= vm; fg *= vm; fbv *= vm;
                if (dark) { float k = 1.f - p.interlace; fr *= k; fg *= k; fbv *= k; }
                float gr = (hf(x, y, (int)(frame * 7919u)) - 0.5f) * 255.f * p.grain;
                fr += gr; fg += gr; fbv += gr;
                if (p.noise > 0 && hf(x * 3 + 1, y, (int)(frame * 104729u)) < p.noise) {
                    float nv = hf(x, y * 5 + 3, (int)frame) * 255.f;
                    fr = nv; fg = nv; fbv = nv;
                }
                if (p.red > 0) { fr += 140 * p.red; fg *= 1.f - 0.5f * p.red; fbv *= 1.f - 0.5f * p.red; }
                if (p.flash > 0) { fr = lerpf(fr, 255, p.flash); fg = lerpf(fg, 255, p.flash); fbv = lerpf(fbv, 255, p.flash); }
                if (p.fade > 0) { float k = 1.f - p.fade; fr *= k; fg *= k; fbv *= k; }
                int d = bayer[(y & 3) * 4 + (x & 3)] - 8;
                int R = clampi((int)fr + d, 0, 255) >> 3, G = clampi((int)fg + d, 0, 255) >> 3, B = clampi((int)fbv + d, 0, 255) >> 3;
                out[y * SW + x] = ((R << 3 | R >> 2) << 16) | ((G << 3 | G >> 2) << 8) | (B << 3 | B >> 2);
            }
        }
    }
};

inline Gfx gfx;

}  // namespace S9
