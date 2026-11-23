// STATION 09: ECHO  --  fully procedural audio (no sound files needed)
// All sound effects and "voices" are synthesized at start-up. Voices use a small
// formant synthesizer so the mimic's speech sounds uncanny and almost-human.
#pragma once
#include "core.hpp"

namespace S9 {

static const int SR = 22050;
typedef std::vector<float> Buf;

enum SndId {
    S_DRONE, S_WIND, S_HUM, S_STATIC, S_HEART, S_FOOT0, S_FOOT1, S_FOOT2, S_FOOT3, S_GRATE0, S_GRATE1,
    S_EFOOT0, S_EFOOT1, S_CREAK, S_SLIDE, S_SLAM, S_PICKUP, S_FUSE, S_BREAKER, S_POWEROFF, S_POWERON,
    S_GENSTART, S_CLICK, S_TAPE, S_STING1, S_STING2, S_BOOM, S_SCREAM, S_HIT, S_SHOT, S_DRIP, S_GROAN,
    S_KNOCK, S_WHISPER, S_ALARM, S_ELEVHUM, S_DING, S_KEY, S_OK, S_ERR, S_BREATH, S_THUMP, S_RUSTLE,
    S_LOCKER, S_RADIOON, S_FLASHON, S_DEATH, S_BASE_COUNT
};

// ---------- DSP helpers ----------
struct LP {
    float a = 0.1f, y = 0;
    void set(float fc) { a = 1.f - expf(-2.f * PI * fc / SR); }
    float p(float x) { y += a * (x - y); return y; }
};
struct HP {
    LP l;
    void set(float fc) { l.set(fc); }
    float p(float x) { return x - l.p(x); }
};
struct BQ {  // band-pass biquad (constant 0 dB peak gain)
    float b0 = 0, b2 = 0, a1 = 0, a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    void set(float f, float Q) {
        f = clampf(f, 60.f, SR * 0.45f);
        float w = 2.f * PI * f / SR, al = sinf(w) / (2.f * Q), a0 = 1.f + al;
        b0 = al / a0; b2 = -al / a0; a1 = -2.f * cosf(w) / a0; a2 = (1.f - al) / a0;
    }
    float p(float x) {
        float y = b0 * x + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        return y;
    }
};

inline float nz() { return vrng.f() * 2.f - 1.f; }
inline Buf mkbuf(float sec) { return Buf((size_t)(sec * SR), 0.f); }
inline void normalize(Buf& b, float peak) {
    float m = 1e-6f;
    for (float v : b) m = maxf(m, fabsf(v));
    float k = peak / m;
    for (float& v : b) v *= k;
}
inline void loopify(Buf& b, int xf) {
    int n = (int)b.size();
    if (xf * 2 >= n) return;
    for (int i = 0; i < xf; i++) {
        float t = (float)i / xf;
        b[i] = b[i] * t + b[n - xf + i] * (1.f - t);
    }
    b.resize(n - xf);
}
inline float env(float t, float a, float d) { return t < a ? t / a : expf(-(t - a) / d); }
inline void addInto(Buf& dst, const Buf& src, float gain, size_t at = 0) {
    for (size_t i = 0; i < src.size() && i + at < dst.size(); i++) dst[i + at] += src[i] * gain;
}

// ---------- formant voice synthesizer ----------
struct Formant { float f1, f2, f3, amp; };
inline bool phon(char c, Formant& f, float& dur, int& kind) {
    // kind: 0 vowel, 1 voiced consonant, 2 fricative (noise), 3 stop, 4 pause
    switch (c) {
        case 'a': f = {730, 1090, 2440, 1}; dur = 0.115f; kind = 0; return true;
        case 'e': f = {530, 1840, 2480, 1}; dur = 0.10f; kind = 0; return true;
        case 'i': f = {270, 2290, 3010, 0.9f}; dur = 0.09f; kind = 0; return true;
        case 'y': f = {290, 2200, 2900, 0.8f}; dur = 0.08f; kind = 0; return true;
        case 'o': f = {570, 840, 2410, 1}; dur = 0.12f; kind = 0; return true;
        case 'u': f = {300, 870, 2240, 0.9f}; dur = 0.11f; kind = 0; return true;
        case 'm': case 'n': f = {250, 1200, 2200, 0.5f}; dur = 0.07f; kind = 1; return true;
        case 'l': f = {350, 1100, 2500, 0.7f}; dur = 0.06f; kind = 1; return true;
        case 'r': f = {400, 1250, 1650, 0.7f}; dur = 0.07f; kind = 1; return true;
        case 'w': f = {300, 700, 2200, 0.7f}; dur = 0.07f; kind = 1; return true;
        case 'v': case 'b': f = {300, 1000, 2300, 0.6f}; dur = 0.055f; kind = 1; return true;
        case 'd': case 'g': case 'j': f = {350, 1500, 2500, 0.6f}; dur = 0.055f; kind = 1; return true;
        case 'z': f = {300, 1400, 2600, 0.5f}; dur = 0.08f; kind = 2; return true;
        case 's': case 'c': f = {0, 6500, 0, 0.5f}; dur = 0.09f; kind = 2; return true;
        case 'f': case 'h': case 'x': f = {0, 3500, 0, 0.35f}; dur = 0.08f; kind = 2; return true;
        case 't': case 'k': case 'p': case 'q': f = {0, 3000, 0, 0.6f}; dur = 0.05f; kind = 3; return true;
        case ' ': dur = 0.07f; kind = 4; return true;
        case ',': dur = 0.15f; kind = 4; return true;
        case '.': case '!': case '?': case ';': case ':': dur = 0.26f; kind = 4; return true;
        default: return false;
    }
}

// fx: 0 clean, 1 radio, 2 tape, 3 entity
inline Buf voice(const char* text, float pitch, float fshift, float speed, float breath, float glitch, int fx,
                 float contour = 0.85f, uint32_t seed = 1) {
    RNG r;
    r.s = 2463534242u ^ (seed * 2654435761u);
    std::string s = text;
    for (auto& c : s) if (c >= 'A' && c <= 'Z') c += 32;
    Buf out;
    out.reserve(SR * 8);
    BQ b1, b2, b3;
    float cf1 = 500, cf2 = 1500, cf3 = 2500, phase = 0, pf = pitch;
    HP nhp; nhp.set(2500);
    float total = 0;
    for (char c : s) { Formant ff; float d; int k; if (phon(c, ff, d, k)) total += d / speed; }
    float tcur = 0;
    bool question = !s.empty() && s.back() == '?';
    int n = (int)s.size();
    for (int ci = 0; ci < n; ci++) {
        Formant F; float dur; int kind;
        if (!phon(s[ci], F, dur, kind)) continue;
        dur /= speed;
        dur *= 0.9f + 0.25f * r.f();
        int ns = (int)(dur * SR);
        size_t startIdx = out.size();
        float t0 = tcur;
        for (int i = 0; i < ns; i++) {
            float t = (float)i / ns;
            float tg = (t0 + dur * t) / maxf(total, 0.01f);  // 0..1 over utterance
            float contourk = lerpf(1.f, contour, tg);
            if (question) contourk *= 1.f + 0.35f * smooth(0.7f, 1.f, tg);
            float wob = 1.f + 0.025f * sinf(tcur * 21.f + i * 0.0007f) + glitch * 0.06f * sinf(tcur * 5.3f);
            float tp = pitch * contourk * wob * (fx == 3 ? 0.8f : 1.f);
            pf += (tp - pf) * 0.01f;
            if (!(i & 15)) {
                if (kind <= 1 || (kind == 2 && F.f1 > 0)) {
                    float t1 = F.f1 * fshift, t2 = F.f2 * fshift, t3 = F.f3 * fshift;
                    cf1 += (t1 - cf1) * 0.25f; cf2 += (t2 - cf2) * 0.25f; cf3 += (t3 - cf3) * 0.25f;
                    b1.set(cf1, 7.f); b2.set(cf2, 9.f); b3.set(cf3, 11.f);
                }
            }
            float e = 1.f;
            float edge = minf(t, 1.f - t);
            e = sat(edge * 7.f);
            float sample = 0;
            if (kind == 0 || kind == 1) {
                phase += pf / SR;
                if (phase >= 1.f) phase -= 1.f;
                float src = (1.f - 2.f * phase);
                src = src * src * src * 0.6f + src * 0.5f;
                src += (r.f() * 2.f - 1.f) * breath;
                float o = b1.p(src) * 1.2f + b2.p(src) * 0.8f + b3.p(src) * 0.35f;
                sample = o * e * F.amp * 1.8f;
            } else if (kind == 2) {
                float nsv = (r.f() * 2.f - 1.f);
                if (F.f1 > 0) {  // voiced fricative z
                    phase += pf / SR; if (phase >= 1.f) phase -= 1.f;
                    float src = (1.f - 2.f * phase) * 0.6f + nsv * 0.5f;
                    sample = (b1.p(src) + b2.p(src) * 0.8f) * e * F.amp * 2.f;
                } else {
                    nhp.set(F.f2 * 0.5f);
                    sample = nhp.p(nsv) * e * F.amp * 0.9f;
                }
            } else if (kind == 3) {
                if (t > 0.35f) sample = nhp.p(r.f() * 2.f - 1.f) * (1.f - t) * 1.4f * F.amp;
            }
            out.push_back(sample);
        }
        tcur += dur;
        // stutter / glitch: repeat the last phoneme chunk
        if (glitch > 0 && kind <= 1 && r.chance(glitch)) {
            size_t len = out.size() - startIdx;
            int reps = 1 + (int)(r.f() * 3);
            for (int rp = 0; rp < reps; rp++)
                for (size_t q = 0; q < len; q++) out.push_back(out[startIdx + q] * (0.9f - 0.15f * rp));
        }
    }
    // effects chain
    if (fx == 1) {  // radio
        HP h; h.set(450); LP l; l.set(3000);
        for (float& v : out) { float x = l.p(h.p(v)) * 1.6f; x += nz() * 0.05f; v = tanhf(x * 1.3f); }
    } else if (fx == 2) {  // tape
        LP l; l.set(3600); HP h; h.set(180);
        for (float& v : out) { float x = l.p(h.p(v)); x += nz() * 0.035f; v = tanhf(x * 1.2f); }
    } else if (fx == 3) {  // entity: layered, detuned, crushed
        Buf a = out;
        size_t n2 = out.size();
        for (size_t i = 0; i < n2; i++) {
            float x = a[i];
            size_t j = (size_t)(i * 0.75f);
            size_t k = (size_t)(i * 1.33f);
            if (j < n2) x += a[j] * 0.7f;
            if (k < n2) x += a[k] * 0.5f;
            float ring = sinf((float)i * 2.f * PI * 35.f / SR);
            x = x * (0.75f + 0.25f * ring);
            if ((i & 1) == 0) out[i] = x; else out[i] = out[i - 1] * 0.5f + x * 0.5f;
            out[i] = tanhf(out[i] * 1.5f);
        }
    }
    normalize(out, 0.85f);
    return out;
}

// ---------- the sound bank ----------
template <class F>
inline Buf gen(float sec, F f) {
    Buf b = mkbuf(sec);
    for (size_t i = 0; i < b.size(); i++) b[i] = f((float)i / SR, (int)i);
    return b;
}

struct Voice {
    int snd = -1;
    double pos = 0;
    float vol = 1, pitch = 1, fadeTarget = -1, fadeRate = 0;
    bool loop = false, spatial = false, alive = false;
    V3 src;
    int id = 0;
};

struct Audio {
    std::vector<Buf> bank;
    std::vector<Voice> vs;
    V3 lp;
    float lyaw = 0, master = 1.f;
    std::atomic_flag lk = ATOMIC_FLAG_INIT;
    int nextId = 1;
    // reverb
    std::vector<float> dl[4];
    int dp[4] = {0, 0, 0, 0};
    float dlp[4] = {0, 0, 0, 0};

    void lock() { while (lk.test_and_set(std::memory_order_acquire)) {} }
    void unlock() { lk.clear(std::memory_order_release); }

    int add(Buf&& b) { bank.push_back(std::move(b)); return (int)bank.size() - 1; }
    float duration(int snd) const { return snd >= 0 && snd < (int)bank.size() ? (float)bank[snd].size() / SR : 0.f; }

    int play(int snd, float vol = 1.f, float pitch = 1.f, const V3* src = nullptr, bool loop = false) {
        if (snd < 0 || snd >= (int)bank.size()) return 0;
        lock();
        Voice* slot = nullptr;
        for (auto& v : vs) if (!v.alive) { slot = &v; break; }
        if (!slot) { if (vs.size() > 48) { unlock(); return 0; } vs.push_back(Voice()); slot = &vs.back(); }
        *slot = Voice();
        slot->snd = snd; slot->vol = vol; slot->pitch = pitch; slot->loop = loop; slot->alive = true;
        if (src) { slot->spatial = true; slot->src = *src; }
        slot->id = nextId++;
        int id = slot->id;
        unlock();
        return id;
    }
    void stop(int id, float fade = 0.25f) {
        if (!id) return;
        lock();
        for (auto& v : vs) if (v.alive && v.id == id) {
            if (fade <= 0) v.alive = false; else { v.fadeTarget = 0; v.fadeRate = v.vol / fade; }
        }
        unlock();
    }
    void setVol(int id, float vol, float fade = 0.f) {
        if (!id) return;
        lock();
        for (auto& v : vs) if (v.alive && v.id == id) {
            if (fade <= 0) v.vol = vol; else { v.fadeTarget = vol; v.fadeRate = fabsf(vol - v.vol) / fade; }
        }
        unlock();
    }
    void setPitch(int id, float p) {
        lock();
        for (auto& v : vs) if (v.alive && v.id == id) v.pitch = p;
        unlock();
    }
    void setPos(int id, V3 p) {
        lock();
        for (auto& v : vs) if (v.alive && v.id == id) { v.src = p; v.spatial = true; }
        unlock();
    }
    bool playing(int id) {
        if (!id) return false;
        lock();
        bool r = false;
        for (auto& v : vs) if (v.alive && v.id == id) r = true;
        unlock();
        return r;
    }
    void stopAll() {
        lock();
        for (auto& v : vs) v.alive = false;
        unlock();
    }
    void setListener(V3 p, float yaw) { lock(); lp = p; lyaw = yaw; unlock(); }

    void mix(i16* out, int frames) {
        lock();
        if (dl[0].empty()) {
            static const int L[4] = {1051, 1399, 1801, 2203};
            for (int i = 0; i < 4; i++) dl[i].assign(L[i], 0.f);
        }
        std::vector<float> mL(frames, 0.f), mR(frames, 0.f);
        float dt = (float)frames / SR;
        V3 right(cosf(lyaw), 0, -sinf(lyaw));
        for (auto& v : vs) {
            if (!v.alive) continue;
            const Buf& b = bank[v.snd];
            if (v.fadeTarget >= 0) {
                if (v.vol > v.fadeTarget) v.vol = maxf(v.fadeTarget, v.vol - v.fadeRate * dt);
                else v.vol = minf(v.fadeTarget, v.vol + v.fadeRate * dt);
                if (v.fadeTarget == 0 && v.vol <= 0.001f) { v.alive = false; continue; }
            }
            float gl = v.vol * 0.7f, gr = v.vol * 0.7f;
            if (v.spatial) {
                V3 d = v.src - lp;
                float dist = vlen(d);
                float att = 1.f / (1.f + 0.28f * dist);
                float pan = dist > 0.5f ? dot(vnorm(V3(d.x, 0, d.z)), right) : 0.f;
                pan *= sat(dist / 2.5f);
                float ang = (pan * 0.5f + 0.5f) * PI * 0.5f;
                gl = v.vol * att * cosf(ang) * 1.2f + v.vol * att * 0.15f;
                gr = v.vol * att * sinf(ang) * 1.2f + v.vol * att * 0.15f;
            }
            double pos = v.pos;
            double n = (double)b.size();
            for (int i = 0; i < frames; i++) {
                if (pos >= n) {
                    if (v.loop) pos -= n; else { v.alive = false; break; }
                }
                size_t i0 = (size_t)pos;
                size_t i1 = i0 + 1 < b.size() ? i0 + 1 : (v.loop ? 0 : i0);
                float fr = (float)(pos - i0);
                float s = b[i0] * (1.f - fr) + b[i1] * fr;
                mL[i] += s * gl;
                mR[i] += s * gr;
                pos += v.pitch;
            }
            v.pos = pos;
        }
        // reverb + output
        for (int i = 0; i < frames; i++) {
            float in = (mL[i] + mR[i]) * 0.5f;
            float wl = 0, wr = 0;
            for (int k = 0; k < 4; k++) {
                float y = dl[k][dp[k]];
                dlp[k] += (y - dlp[k]) * 0.35f;
                dl[k][dp[k]] = in * 0.35f + dlp[k] * 0.62f;
                dp[k] = (dp[k] + 1) % (int)dl[k].size();
                if (k & 1) wr += dlp[k]; else wl += dlp[k];
            }
            float l = (mL[i] + wl * 0.55f) * master, r = (mR[i] + wr * 0.55f) * master;
            l = tanhf(l * 0.9f); r = tanhf(r * 0.9f);
            out[i * 2] = (i16)(l * 30000.f);
            out[i * 2 + 1] = (i16)(r * 30000.f);
        }
        unlock();
    }

    // ---------------------------------------------------------------
    void init() {
        bank.resize(S_BASE_COUNT);
        // --- ambience ---
        {   // low drone, seamless 8 s loop
            Buf b = gen(8.f, [](float t, int) {
                float lfo = 0.5f + 0.5f * sinf(2 * PI * 0.125f * t), lfo2 = 0.5f + 0.5f * sinf(2 * PI * 0.25f * t + 1.f);
                float s = 0.5f * sinf(2 * PI * 40.f * t) + 0.45f * sinf(2 * PI * 41.5f * t) + 0.3f * sinf(2 * PI * 55.f * t) * lfo +
                          0.2f * sinf(2 * PI * 82.5f * t) * lfo2 + 0.06f * powf(lfo2, 3.f) * sinf(2 * PI * 733.5f * t) +
                          0.03f * sinf(2 * PI * 1101.25f * t) * lfo * lfo;
                return s;
            });
            normalize(b, 0.55f);
            bank[S_DRONE] = b;
        }
        {   // wind / air
            Buf b = mkbuf(7.f);
            LP l; l.set(300); LP l2; l2.set(900);
            for (size_t i = 0; i < b.size(); i++) {
                float t = (float)i / SR;
                l.set(180.f + 140.f * (0.5f + 0.5f * sinf(t * 0.9f)));
                float x = nz();
                b[i] = l.p(x) * 2.2f * (0.6f + 0.4f * sinf(t * 0.7f + 2.f)) + l2.p(x) * 0.25f * (0.5f + 0.5f * sinf(t * 1.9f));
            }
            loopify(b, SR); normalize(b, 0.4f);
            bank[S_WIND] = b;
        }
        {   // generator / electrical hum
            Buf b = gen(2.f, [](float t, int) {
                return 0.5f * sinf(2 * PI * 50.f * t) + 0.35f * sinf(2 * PI * 100.f * t) + 0.25f * sinf(2 * PI * 150.f * t) +
                       0.15f * sinf(2 * PI * 200.f * t) + 0.06f * sinf(2 * PI * 350.f * t);
            });
            normalize(b, 0.4f);
            bank[S_HUM] = b;
        }
        {   // radio static
            Buf b = mkbuf(2.5f);
            BQ q; q.set(1800, 0.7f); HP h; h.set(600);
            for (size_t i = 0; i < b.size(); i++) { float x = nz(); b[i] = (q.p(x) * 1.5f + h.p(x) * 0.4f) * (0.6f + 0.4f * nz()); }
            loopify(b, 2000); normalize(b, 0.6f);
            bank[S_STATIC] = b;
        }
        {   // heartbeat
            Buf b = gen(0.95f, [](float t, int) {
                auto beat = [&](float tt, float a) {
                    if (tt < 0) return 0.f;
                    return a * sinf(2 * PI * (55.f - 18.f * tt) * tt * 1.0f + 0.f) * expf(-tt * 22.f);
                };
                return beat(t, 1.f) + beat(t - 0.27f, 0.7f);
            });
            normalize(b, 0.9f);
            bank[S_HEART] = b;
        }
        // --- footsteps ---
        for (int k = 0; k < 4; k++) {
            float cut = 500.f + k * 140.f;
            Buf b = gen(0.22f, [&](float t, int) {
                static LP l; l.set(cut);
                return l.p(nz()) * 2.f * expf(-t * 30.f) + sinf(2 * PI * (70.f + k * 6.f) * t) * expf(-t * 40.f);
            });
            normalize(b, 0.8f);
            bank[S_FOOT0 + k] = b;
        }
        for (int k = 0; k < 2; k++) {
            Buf b = gen(0.5f, [&](float t, int) {
                static LP l; l.set(900);
                float ring = sinf(2 * PI * (880.f + k * 90.f) * t) * 0.15f + sinf(2 * PI * (1370.f + k * 70.f) * t) * 0.1f + sinf(2 * PI * 2110.f * t) * 0.05f;
                return l.p(nz()) * 1.6f * expf(-t * 34.f) + ring * expf(-t * 9.f) + sinf(2 * PI * 85.f * t) * expf(-t * 40.f) * 0.8f;
            });
            normalize(b, 0.7f);
            bank[S_GRATE0 + k] = b;
        }
        for (int k = 0; k < 2; k++) {  // the thing's wet, heavy steps
            Buf b = gen(0.55f, [&](float t, int) {
                static LP l; l.set(260); static BQ q; q.set(180.f + 40.f * k, 3.f);
                float sq = q.p(nz()) * (0.6f + 0.4f * sinf(t * 70.f)) * expf(-(t - 0.05f) * 9.f);
                return l.p(nz()) * 2.6f * expf(-t * 18.f) + sinf(2 * PI * (48.f + 5.f * k) * t) * expf(-t * 14.f) + sq * 0.8f;
            });
            normalize(b, 0.95f);
            bank[S_EFOOT0 + k] = b;
        }
        // --- doors, machines ---
        {   // creaking door
            Buf b = mkbuf(1.6f);
            float ph = 0; BQ q;
            for (size_t i = 0; i < b.size(); i++) {
                float t = (float)i / SR;
                float f = 140.f + 90.f * sinf(t * 2.2f) + 60.f * t + 12.f * sinf(t * 31.f);
                ph += f / SR; if (ph > 1) ph -= 1;
                q.set(500.f + 400.f * sinf(t * 3.1f + 1.f), 5.f);
                float s = q.p((1.f - 2.f * ph) * (0.7f + 0.3f * nz()));
                b[i] = s * sinf(PI * t / 1.6f) * 3.f;
            }
            normalize(b, 0.7f); bank[S_CREAK] = b;
        }
        {   // powered sliding door
            Buf b = gen(0.95f, [](float t, int) {
                static HP h; h.set(2200);
                float hiss = h.p(nz()) * sinf(PI * minf(t / 0.6f, 1.f)) * (t < 0.6f ? 1.f : 0.f) * 0.7f;
                float thunk = sinf(2 * PI * 70.f * (t - 0.62f)) * expf(-(t - 0.62f) * 22.f) * (t > 0.62f ? 1.f : 0.f);
                float motor = sinf(2 * PI * (180.f + 120.f * t) * t) * 0.15f * (t < 0.6f ? 1.f : 0.f);
                return hiss + thunk + motor;
            });
            normalize(b, 0.7f); bank[S_SLIDE] = b;
        }
        bank[S_SLAM] = [] { Buf b = gen(0.9f, [](float t, int) {
            static LP l; l.set(1500);
            return l.p(nz()) * 2.f * expf(-t * 14.f) + sinf(2 * PI * 52.f * t) * expf(-t * 8.f) +
                   (sinf(2 * PI * 420.f * t) * 0.2f + sinf(2 * PI * 690.f * t) * 0.15f) * expf(-t * 7.f);
        }); normalize(b, 1.f); return b; }();
        bank[S_PICKUP] = [] { Buf b = gen(0.3f, [](float t, int) {
            return (sinf(2 * PI * 880.f * t) * env(t, 0.003f, 0.05f) + 0.7f * sinf(2 * PI * 1320.f * (t - 0.06f)) * (t > 0.06f ? env(t - 0.06f, 0.003f, 0.08f) : 0.f));
        }); normalize(b, 0.5f); return b; }();
        bank[S_FUSE] = [] { Buf b = gen(1.0f, [](float t, int) {
            static LP l; l.set(800);
            float clunk = l.p(nz()) * 2.f * expf(-t * 25.f) + sinf(2 * PI * 90.f * t) * expf(-t * 18.f);
            float ph = fmodf(t * 120.f, 1.f);
            float buzz = (1.f - 2.f * ph) * 0.25f * expf(-(t - 0.1f) * 4.f) * (t > 0.1f ? 1.f : 0.f);
            return clunk + buzz;
        }); normalize(b, 0.8f); return b; }();
        bank[S_BREAKER] = [] { Buf b = gen(1.1f, [](float t, int) {
            static LP l; l.set(1200); static HP h; h.set(3000);
            float th = l.p(nz()) * 2.f * expf(-t * 20.f) + sinf(2 * PI * 60.f * t) * expf(-t * 12.f);
            float zap = h.p(nz()) * expf(-(t - 0.18f) * 12.f) * (t > 0.18f ? 1.f : 0.f) * 0.6f;
            return th + zap;
        }); normalize(b, 0.9f); return b; }();
        bank[S_POWEROFF] = [] { Buf b = gen(2.0f, [](float t, int) {
            static LP l; l.set(700);
            float f = 330.f * expf(-t * 2.2f) + 28.f;
            static float ph = 0; ph += f / SR;
            float thunk = (t > 0.02f ? 1.f : 0.f) * (l.p(nz()) * 2.f + sinf(2 * PI * 45.f * t)) * expf(-(t - 0.02f) * 14.f);
            return sinf(2 * PI * ph) * 0.6f * expf(-t * 1.5f) + thunk;
        }); normalize(b, 0.9f); return b; }();
        bank[S_POWERON] = [] { Buf b = gen(3.0f, [](float t, int) {
            float f = 45.f + 75.f * smooth(0.f, 1.6f, t);
            static float ph = 0; ph += f / SR;
            float clack = ((t > 1.2f && t < 1.25f) || (t > 1.5f && t < 1.53f)) ? nz() * 0.8f : 0.f;
            float hum = (sinf(2 * PI * ph) * 0.5f + sinf(2 * PI * 100.f * t) * 0.3f * smooth(1.2f, 2.f, t) + sinf(2 * PI * 50.f * t) * 0.3f * smooth(1.2f, 2.f, t));
            return hum * minf(1.f, t * 3.f) * (t > 2.4f ? (3.f - t) / 0.6f : 1.f) + clack;
        }); normalize(b, 0.8f); return b; }();
        bank[S_GENSTART] = [] { Buf b = gen(5.0f, [](float t, int) {
            static LP l; l.set(400); static float ph = 0;
            float rate = 3.f + t * 4.f;
            float puls = powf(0.5f + 0.5f * sinf(2 * PI * rate * t), 3.f);
            float sputter = l.p(nz()) * 2.5f * puls * (t < 3.2f ? 1.f : 0.f);
            ph += (28.f + 22.f * smooth(2.5f, 4.f, t)) / SR;
            float roar = ((1.f - 2.f * fmodf(ph, 1.f)) * 0.7f + sinf(2 * PI * ph * 2.f) * 0.5f) * smooth(2.4f, 4.f, t);
            return sputter * (1.f - smooth(2.8f, 3.6f, t)) + roar * 0.8f;
        }); normalize(b, 0.9f); return b; }();
        bank[S_CLICK] = [] { Buf b = gen(0.06f, [](float t, int) { return nz() * expf(-t * 140.f) + sinf(2 * PI * 1800.f * t) * expf(-t * 120.f) * 0.5f; }); normalize(b, 0.6f); return b; }();
        bank[S_TAPE] = [] { Buf b = gen(0.5f, [](float t, int) {
            static HP h; h.set(2500);
            return nz() * expf(-t * 80.f) * 0.8f + h.p(nz()) * 0.08f * (t > 0.08f ? 1.f : 0.f) * expf(-(t - 0.08f) * 4.f) + sinf(2 * PI * 140.f * t) * expf(-t * 50.f) * 0.4f;
        }); normalize(b, 0.6f); return b; }();
        // --- stingers / scares ---
        bank[S_STING1] = [] { Buf b = gen(3.6f, [](float t, int) {
            static const float fr[6] = {233.1f, 246.9f, 311.1f, 329.6f, 440.f, 466.2f};
            float s = 0;
            for (int i = 0; i < 6; i++) s += sinf(2 * PI * fr[i] * t + 0.7f * sinf(t * (3.f + i))) * (0.25f - i * 0.02f);
            static LP l; l.set(2000);
            float e = env(t, 0.02f, 1.1f);
            return s * e * (1.f + 0.4f * sinf(t * 40.f)) + l.p(nz()) * 1.4f * expf(-t * 8.f) + sinf(2 * PI * 46.f * t) * expf(-t * 3.f) * 0.8f;
        }); normalize(b, 1.f); return b; }();
        bank[S_STING2] = [] { Buf b = gen(1.5f, [](float t, int) {
            static float ph = 0; float f = 1400.f + 600.f * sinf(2 * PI * 9.f * t) + 800.f * t;
            ph += f / SR;
            float fm = sinf(2 * PI * ph + 5.f * sinf(2 * PI * 233.f * t));
            return (fm * 0.7f + nz() * 0.5f) * env(t, 0.005f, 0.5f) + sinf(2 * PI * 60.f * t) * expf(-t * 4.f);
        }); normalize(b, 1.f); return b; }();
        bank[S_BOOM] = [] { Buf b = gen(2.2f, [](float t, int) {
            static LP l; l.set(250);
            return sinf(2 * PI * (50.f - 12.f * t) * t) * expf(-t * 2.2f) + l.p(nz()) * 2.5f * expf(-t * 5.f);
        }); normalize(b, 1.f); return b; }();
        bank[S_SCREAM] = voice("aaaaaiiiaaaa", 330.f, 1.25f, 1.5f, 0.5f, 0.35f, 3, 2.0f, 77);
        bank[S_HIT] = [] { Buf b = gen(0.6f, [](float t, int) {
            static LP l; l.set(700);
            return l.p(nz()) * 2.f * expf(-t * 20.f) + sinf(2 * PI * 55.f * t) * expf(-t * 10.f);
        }); normalize(b, 1.f); return b; }();
        bank[S_SHOT] = [] { Buf b = gen(0.9f, [](float t, int) {
            static LP l; l.set(2500);
            return nz() * expf(-t * 60.f) + l.p(nz()) * 1.5f * expf(-t * 9.f) + sinf(2 * PI * 70.f * t) * expf(-t * 18.f);
        }); normalize(b, 1.f); return b; }();
        bank[S_DRIP] = [] { Buf b = gen(0.5f, [](float t, int) {
            float f = 1500.f - 900.f * minf(t * 6.f, 1.f);
            static float ph = 0; ph += f / SR;
            return sinf(2 * PI * ph) * env(t, 0.002f, 0.04f) + 0.3f * sinf(2 * PI * ph * 1.5f) * env(t - 0.15f, 0.002f, 0.05f) * (t > 0.15f ? 1.f : 0.f);
        }); normalize(b, 0.5f); return b; }();
        bank[S_GROAN] = [] { Buf b = gen(3.5f, [](float t, int) {
            static float ph = 0; static BQ q;
            float f = 38.f + 14.f * sinf(t * 1.3f) + 8.f * t; ph += f / SR;
            q.set(220.f + 120.f * sinf(t * 2.1f), 4.f);
            return (q.p((1.f - 2.f * fmodf(ph, 1.f)) * 1.2f) * 3.f + sinf(2 * PI * ph) * 0.6f) * sinf(PI * t / 3.5f);
        }); normalize(b, 0.8f); return b; }();
        bank[S_KNOCK] = [] { Buf b = gen(0.45f, [](float t, int) {
            static LP l; l.set(600);
            auto k = [&](float tt) { return tt < 0 ? 0.f : (l.p(nz()) * 1.5f + sinf(2 * PI * 110.f * tt)) * expf(-tt * 40.f); };
            return k(t) + k(t - 0.2f) * 0.8f;
        }); normalize(b, 0.9f); return b; }();
        bank[S_WHISPER] = [] { Buf b = mkbuf(2.4f); BQ q1, q2; HP h; h.set(1500);
            for (size_t i = 0; i < b.size(); i++) {
                float t = (float)i / SR;
                q1.set(1800.f + 900.f * sinf(t * 7.f), 3.f); q2.set(3200.f + 700.f * sinf(t * 5.f + 1.f), 4.f);
                float x = nz();
                float syl = powf(0.5f + 0.5f * sinf(t * 9.f + sinf(t * 2.f) * 2.f), 2.f);
                b[i] = (q1.p(x) + q2.p(x) * 0.6f + h.p(x) * 0.2f) * syl * sinf(PI * t / 2.4f);
            }
            normalize(b, 0.55f); return b; }();
        bank[S_ALARM] = [] { Buf b = gen(2.0f, [](float t, int) {
            float f = fmodf(t, 1.f) < 0.5f ? 640.f : 880.f; static float ph = 0; ph += f / SR;
            float s = sinf(2 * PI * ph); return (s > 0 ? 1.f : -1.f) * 0.3f + s * 0.4f;
        }); normalize(b, 0.5f); return b; }();
        bank[S_ELEVHUM] = [] { Buf b = gen(2.0f, [](float t, int) { return sinf(2 * PI * 60.f * t) * 0.6f + sinf(2 * PI * 120.f * t) * 0.3f + sinf(2 * PI * 180.f * t) * 0.15f; }); normalize(b, 0.4f); return b; }();
        bank[S_DING] = [] { Buf b = gen(1.6f, [](float t, int) { return (sinf(2 * PI * 1175.f * t) + 0.5f * sinf(2 * PI * 1760.f * t)) * env(t, 0.003f, 0.5f); }); normalize(b, 0.6f); return b; }();
        bank[S_KEY] = [] { Buf b = gen(0.06f, [](float t, int) { return sinf(2 * PI * 1500.f * t) * env(t, 0.002f, 0.015f); }); normalize(b, 0.4f); return b; }();
        bank[S_OK] = [] { Buf b = gen(0.35f, [](float t, int) { return sinf(2 * PI * 900.f * t) * env(t, 0.002f, 0.04f) + (t > 0.12f ? sinf(2 * PI * 1350.f * t) * env(t - 0.12f, 0.002f, 0.07f) : 0.f); }); normalize(b, 0.5f); return b; }();
        bank[S_ERR] = [] { Buf b = gen(0.45f, [](float t, int) { float s = sinf(2 * PI * 170.f * t); return (s > 0 ? 1.f : -1.f) * 0.5f * env(t, 0.003f, 0.2f); }); normalize(b, 0.5f); return b; }();
        bank[S_BREATH] = [] { Buf b = mkbuf(3.2f); BQ q; q.set(900, 1.2f); HP h; h.set(400);
            for (size_t i = 0; i < b.size(); i++) {
                float t = (float)i / SR; float c = fmodf(t, 1.6f) / 1.6f;
                float e = c < 0.45f ? sinf(PI * c / 0.45f) : (c < 0.55f ? 0.f : sinf(PI * (c - 0.55f) / 0.45f) * 0.8f);
                float x = nz(); b[i] = (q.p(x) * 2.f + h.p(x) * 0.3f) * e;
            }
            normalize(b, 0.5f); return b; }();
        bank[S_THUMP] = [] { Buf b = gen(0.6f, [](float t, int) { static LP l; l.set(300); return (sinf(2 * PI * 48.f * t) + l.p(nz()) * 1.5f) * expf(-t * 12.f); }); normalize(b, 0.9f); return b; }();
        bank[S_RUSTLE] = [] { Buf b = gen(0.7f, [](float t, int) { static HP h; h.set(2500); return h.p(nz()) * (0.4f + 0.6f * powf(0.5f + 0.5f * sinf(t * 46.f), 3.f)) * sinf(PI * t / 0.7f); }); normalize(b, 0.5f); return b; }();
        bank[S_LOCKER] = [] { Buf b = gen(0.9f, [](float t, int) {
            static LP l; l.set(1800);
            return l.p(nz()) * 1.8f * expf(-t * 35.f) + (sinf(2 * PI * 410.f * t) * 0.3f + sinf(2 * PI * 680.f * t) * 0.25f + sinf(2 * PI * 1130.f * t) * 0.15f) * expf(-t * 6.f);
        }); normalize(b, 0.9f); return b; }();
        bank[S_RADIOON] = [] { Buf b = gen(0.35f, [](float t, int) { static HP h; h.set(1200); return h.p(nz()) * expf(-t * 10.f) * 0.7f + sinf(2 * PI * 1000.f * t) * env(t, 0.002f, 0.03f) * 0.4f; }); normalize(b, 0.5f); return b; }();
        bank[S_FLASHON] = bank[S_CLICK];
        bank[S_DEATH] = [] { Buf b = gen(3.0f, [](float t, int) {
            static LP l; l.set(400); static float ph = 0; ph += (90.f * expf(-t * 1.5f) + 20.f) / SR;
            return sinf(2 * PI * ph) * expf(-t * 1.2f) + l.p(nz()) * 2.f * expf(-t * 6.f);
        }); normalize(b, 1.f); return b; }();
    }
};

inline Audio audio;

}  // namespace S9
