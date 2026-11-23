// STATION 09: ECHO  --  rendering and UI
#pragma once
#include "game_cutscene.hpp"

namespace S9 {

static inline std::vector<std::string> wrapText(const std::string& s, int cols) {
    std::vector<std::string> out;
    std::string cur, word;
    auto flush = [&]() {
        if (cur.size() + word.size() + (cur.empty() ? 0 : 1) > (size_t)cols) { out.push_back(cur); cur = word; }
        else cur += (cur.empty() ? "" : " ") + word;
        word.clear();
    };
    for (char c : s) {
        if (c == ' ') { if (!word.empty()) flush(); }
        else if (c == '\n') { if (!word.empty()) flush(); out.push_back(cur); cur.clear(); }
        else word += c;
    }
    if (!word.empty()) flush();
    if (!cur.empty()) out.push_back(cur);
    return out;
}

inline void Game::drawDoor(const Door& d, const LightFn& lf) {
    V3 c(d.cx + 0.5f, 1.48f, d.cz + 0.5f);
    float slide = d.open * 0.96f;
    V3 half = d.alongX ? V3(0.5f, 1.48f, 0.07f) : V3(0.07f, 1.48f, 0.5f);
    if (d.alongX) c.x += slide; else c.z += slide;
    int tex = d.kind == DK_MANUAL ? T_DOOR : (d.kind == DK_POWER ? T_DOORP : T_DOORS);
    M3 R = M3::I();
    gfx.box(c, half, R, tex, lf, tex);
    // thin face on the other side: rotate a second time so both sides show the front texture
    gfx.box(c, half, M3::rotY(PI), tex, lf, tex);
    // status light
    bool ok = d.kind == DK_MANUAL ? true : (d.kind == DK_POWER ? power() : P.secUnlocked);
    if (d.kind != DK_MANUAL) {
        V3 lp(d.cx + 0.5f, 2.75f, d.cz + 0.5f);
        float r = ok ? 0.1f : 0.9f, g = ok ? 0.8f : 0.1f;
        gfx.glow(lp + (d.alongX ? V3(0, 0, 0.2f) : V3(0.2f, 0, 0)), 0.15f, r, g, 0.1f, 0.9f);
        gfx.glow(lp + (d.alongX ? V3(0, 0, -0.2f) : V3(-0.2f, 0, 0)), 0.15f, r, g, 0.1f, 0.9f);
    }
}

inline void Game::drawObj(const Obj& o, const LightFn& lf) {
    M3 R = M3::rotY(o.yaw);
    V3 c(o.pos.x, o.pos.y + o.half.y, o.pos.z);
    auto at = [&](V3 local) { return V3(o.pos.x, o.pos.y, o.pos.z) + R * local; };
    float pulse = 0.5f + 0.5f * sinf(t * 3.f + o.pos.x * 2.f);
    V3 none(0, 0, 0);
    switch (o.type) {
        case O_DESK:
            gfx.box(V3(c.x, 0.78f, c.z), V3(o.half.x, 0.04f, o.half.z), R, T_DESK, lf);
            gfx.box(at(V3(o.half.x - 0.05f, 0.38f, o.half.z - 0.05f)), V3(0.04f, 0.38f, 0.04f), R, T_BOX, lf);
            gfx.box(at(V3(-o.half.x + 0.05f, 0.38f, o.half.z - 0.05f)), V3(0.04f, 0.38f, 0.04f), R, T_BOX, lf);
            gfx.box(at(V3(o.half.x - 0.05f, 0.38f, -o.half.z + 0.05f)), V3(0.04f, 0.38f, 0.04f), R, T_BOX, lf);
            gfx.box(at(V3(-o.half.x + 0.05f, 0.38f, -o.half.z + 0.05f)), V3(0.04f, 0.38f, 0.04f), R, T_BOX, lf);
            break;
        case O_CRATE: gfx.box(c, o.half, R, T_CRATE, lf); break;
        case O_LOCKER: gfx.box(c, o.half, R, T_BOX, lf, T_LOCKER); break;
        case O_COMMS: case O_REPORT: case O_SECPC: {
            gfx.box(at(V3(0, 0.4f, 0)), V3(o.half.x, 0.4f, o.half.z), R, T_BOX, lf);
            bool on = o.type == O_SECPC ? power() : true;
            M3 tilt = R * M3::rotX(-0.25f);
            gfx.box(at(V3(0, 1.05f, 0.02f)), V3(o.half.x * 0.8f, 0.28f, 0.05f), tilt, T_BOX, lf, on ? T_TERM : T_TERMOFF, on ? V3(0.25f, 0.4f, 0.25f) : none);
            if (on) gfx.glow(at(V3(0, 1.05f, 0.3f)), 0.5f, 0.1f, 0.8f, 0.25f, 0.35f);
            if (o.type == O_REPORT && P.reportDone[P.cur]) gfx.glow(at(V3(0.3f, 0.82f, 0.3f)), 0.1f, 0.1f, 0.9f, 0.1f, 0.9f);
            break;
        }
        case O_TAPE:
            gfx.box(c, o.half, R, T_BLACK, lf, -1, V3(0.1f, 0.1f, 0.1f));
            gfx.box(c + V3(0, 0.05f, 0), V3(0.04f, 0.01f, 0.04f), R, T_WARN, lf, -1, V3(0.3f, 0.3f, 0.3f));
            gfx.glow(c + V3(0, 0.12f, 0), 0.18f, 1.f, 0.15f, 0.1f, 0.5f + 0.4f * pulse);
            break;
        case O_FUSE:
            gfx.box(c, o.half, R, T_LIGHT, lf, -1, V3(0.6f, 0.5f, 0.2f));
            gfx.glow(c + V3(0, 0.1f, 0), 0.3f, 1.f, 0.85f, 0.3f, 0.45f + 0.4f * pulse);
            break;
        case O_BATTERY:
            gfx.box(c, o.half, R, T_WARN, lf, -1, V3(0.2f, 0.2f, 0.05f));
            gfx.glow(c + V3(0, 0.15f, 0), 0.22f, 0.4f, 1.f, 0.3f, 0.3f + 0.3f * pulse);
            break;
        case O_MEDKIT:
            gfx.box(c, o.half, R, T_PAPER, lf, -1, V3(0.2f, 0.2f, 0.2f));
            gfx.box(c + V3(0, 0.1f, 0), V3(0.09f, 0.012f, 0.025f), R, T_FLESH, lf, -1, V3(0.5f, 0, 0));
            gfx.box(c + V3(0, 0.1f, 0), V3(0.025f, 0.012f, 0.09f), R, T_FLESH, lf, -1, V3(0.5f, 0, 0));
            gfx.glow(c + V3(0, 0.15f, 0), 0.22f, 1.f, 0.3f, 0.3f, 0.25f + 0.3f * pulse);
            break;
        case O_AMMO:
            gfx.box(c, o.half, R, T_WARN, lf, -1, V3(0.15f, 0.12f, 0.02f));
            gfx.glow(c + V3(0, 0.12f, 0), 0.2f, 1.f, 0.8f, 0.2f, 0.25f + 0.3f * pulse);
            break;
        case O_PISTOL:
            gfx.box(c, o.half, R, T_BLACK, lf, -1, V3(0.08f, 0.08f, 0.08f));
            gfx.box(c + R * V3(-0.04f, -0.02f, -0.07f), V3(0.025f, 0.05f, 0.025f), R, T_BLACK, lf, -1, V3(0.06f, 0.06f, 0.06f));
            gfx.glow(c + V3(0, 0.12f, 0), 0.25f, 0.8f, 0.9f, 1.f, 0.3f + 0.3f * pulse);
            break;
        case O_NOTE:
            gfx.box(c, o.half, R, T_PAPER, lf, -1, V3(0.25f, 0.25f, 0.22f));
            gfx.glow(c + V3(0, 0.12f, 0), 0.22f, 1.f, 1.f, 0.9f, 0.25f + 0.25f * pulse);
            break;
        case O_STAIRS: {
            for (int i = 0; i < 4; i++) gfx.box(at(V3(0, 0.09f + i * 0.09f, 0.32f - i * 0.2f)), V3(0.42f, 0.09f + i * 0.09f, 0.1f), R, T_METAL, lf);
            gfx.box(at(V3(0, 1.45f, -0.3f)), V3(0.5f, 1.45f, 0.05f), R, T_DOOR, lf, T_DOOR);
            gfx.glow(at(V3(0, 2.2f, 0.1f)), 0.25f, 0.2f, 1.f, 0.3f, 0.7f);
            break;
        }
        case O_BREAKER:
            gfx.box(c, o.half, R, T_BOX, lf, T_BOX);
            gfx.box(at(V3(0, 1.05f, 0.1f)), V3(0.3f, 0.35f, 0.02f), R, T_BOX, lf, T_TERMOFF);
            if (P.tripped) gfx.glow(at(V3(0, 1.3f, 0.3f)), 0.22f, 1.f, 0.1f, 0.1f, 0.5f + 0.5f * pulse);
            else gfx.glow(at(V3(0, 1.3f, 0.3f)), 0.2f, 0.1f, 1.f, 0.2f, 0.7f);
            break;
        case O_FUSEBOX:
            gfx.box(c, o.half, R, T_BOX, lf, T_BOX);
            gfx.glow(at(V3(0, 1.4f, 0.3f)), 0.2f, P.fuseInstalled ? 0.1f : 1.f, P.fuseInstalled ? 1.f : 0.3f, 0.1f, 0.8f);
            break;
        case O_GEN: {
            float vib = P.genOn ? 0.006f * sinf(t * 60.f) : 0.f;
            V3 cc = c + V3(0, vib, 0);
            gfx.box(cc, o.half, R, T_GEN, lf, T_GEN);
            gfx.box(at(V3(0, 1.62f, 0)), V3(0.8f, 0.04f, 0.6f), R, T_BOX, lf);
            gfx.glow(at(V3(0, 1.4f, 0.8f)), 0.3f, P.genOn ? 0.1f : 1.f, P.genOn ? 1.f : 0.15f, 0.1f, 0.7f);
            break;
        }
        case O_CORE: {
            gfx.box(c, o.half, R, T_FLESH, lf, T_FLESH);
            gfx.box(at(V3(0, 1.2f, 0.52f)), V3(0.38f, 0.28f, 0.03f), R, T_BOX, lf, T_TERM, V3(0.3f, 0.5f, 0.3f));
            gfx.box(at(V3(0.45f, 0.3f, 0.45f)), V3(0.12f, 0.3f, 0.12f), R, T_FLESH, lf);
            gfx.box(at(V3(-0.45f, 0.25f, -0.4f)), V3(0.15f, 0.25f, 0.15f), R, T_FLESH, lf);
            float k = P.purged ? 1.f : 0.f;
            gfx.glow(at(V3(0, 1.2f, 0.7f)), 0.7f, 0.2f + 0.8f * k, 0.9f - 0.8f * k, 0.3f - 0.2f * k, 0.4f + 0.2f * pulse);
            break;
        }
        case O_ELEV: {
            float open = o.t * 0.5f;
            gfx.box(at(V3(0, 1.5f, -0.02f)), V3(1.05f, 1.5f, 0.05f), R, T_BLACK, lf, -1, V3(P.elevReady ? 0.5f : 0.f, P.elevReady ? 0.45f : 0.f, P.elevReady ? 0.3f : 0.f));
            gfx.box(at(V3(-0.45f - open, 1.3f, 0.06f)), V3(0.45f, 1.3f, 0.06f), R, T_ELEV, lf, T_ELEV);
            gfx.box(at(V3(0.45f + open, 1.3f, 0.06f)), V3(0.45f, 1.3f, 0.06f), R, T_ELEV, lf, T_ELEV);
            bool good = P.elevReady, called = P.elevCalled;
            gfx.glow(at(V3(0, 2.75f, 0.2f)), 0.25f, good ? 0.1f : 1.f, good ? 1.f : (called ? 0.7f : 0.1f), 0.1f, 0.6f + 0.3f * pulse);
            gfx.box(at(V3(1.15f, 1.3f, 0.05f)), V3(0.1f, 0.2f, 0.03f), R, T_BOX, lf, T_TERM, V3(0.2f, 0.3f, 0.2f));
            break;
        }
        case O_CORPSE: {
            gfx.box(V3(o.pos.x, 0.01f, o.pos.z), V3(0.7f, 0.008f, 0.55f), M3::rotY(o.yaw), T_BLOOD, lf, T_BLOOD);
            if (o.state == 0) {
                gfx.box(at(V3(0, 0.12f, 0)), V3(0.22f, 0.1f, 0.14f), R, T_PAPER, lf);
                gfx.box(at(V3(0.34f, 0.1f, 0.02f)), V3(0.1f, 0.08f, 0.09f), R, T_SKIN, lf);
                gfx.box(at(V3(-0.3f, 0.07f, 0)), V3(0.22f, 0.07f, 0.1f), R, T_CONCD, lf);
            } else {
                float rise = o.state == 1 ? sat(o.t / 0.4f) : 1.f;
                M3 up = R * M3::rotZ(-1.45f * rise);
                gfx.box(at(V3(0, 0.12f + 0.5f * rise, 0)), V3(0.22f, 0.1f + 0.2f * rise, 0.14f), up, T_PAPER, lf);
                gfx.box(at(V3(0, 0.12f + 0.95f * rise, 0)), V3(0.1f, 0.1f, 0.09f), up, T_SKIN, lf, T_FACE);
                gfx.box(at(V3(-0.3f, 0.07f, 0)), V3(0.22f, 0.07f, 0.1f), R, T_CONCD, lf);
            }
            break;
        }
    }
}

inline void Game::renderWorld() {
    gfx.setCam(camPos, camYaw, camPitch);
    camFwd = gfx.fw;
    buildLights();
    gfx.clear(0xFF000000u);
    Level& l = L();
    int R = (int)l.th.fogEnd + 1;
    int cx0 = (int)camPos.x, cz0 = (int)camPos.z;
    for (int z = cz0 - R; z <= cz0 + R; z++)
        for (int x = cx0 - R; x <= cx0 + R; x++) {
            if (x < 0 || z < 0 || x >= l.W || z >= l.H) continue;
            auto& q = l.cq[z * l.W + x];
            if (q.empty()) continue;
            float dx = x + 0.5f - camPos.x, dz = z + 0.5f - camPos.z;
            float d2 = dx * dx + dz * dz;
            if (d2 > (float)(R * R)) continue;
            float dist = sqrtf(d2);
            if (dist > 3.2f && (dx * camFwd.x + dz * camFwd.z) / dist < 0.12f) {
                // outside the (approximate) view cone
                float fl = sqrtf(camFwd.x * camFwd.x + camFwd.z * camFwd.z);
                if (fl > 0.3f && (dx * camFwd.x + dz * camFwd.z) / (dist * fl) < 0.3f) continue;
            }
            int cell = z * l.W + x;
            for (const Quad& qd : q) {
                if (dot(qd.n, camPos - qd.p[0]) <= 0.f) continue;
                V3 Lc[4];
                for (int i = 0; i < 4; i++) Lc[i] = lightAt(qd.p[i], qd.n, cell);
                if (Lc[0].x + Lc[0].y + Lc[0].z + Lc[1].x + Lc[1].y + Lc[1].z + Lc[2].x + Lc[2].y + Lc[2].z + Lc[3].x + Lc[3].y + Lc[3].z < 0.02f) continue;
                gfx.quadW(qd.p, qd.u, qd.v, Lc, qd.tex);
            }
        }
    auto cellOf = [&](const V3& p) {
        int cx = clampi((int)p.x, 0, l.W - 1), cz = clampi((int)p.z, 0, l.H - 1);
        return cz * l.W + cx;
    };
    // ceiling lamp fixtures
    for (size_t i = 0; i < l.lights.size() && i < 64; i++) {
        const Light& lg = l.lights[i];
        if (dist2d(lg.p, camPos) > l.th.fogEnd) continue;
        bool bright = false;
        V3 colr;
        for (const NL& n : nl) if (fabsf(n.p.x - lg.p.x) < 0.01f && fabsf(n.p.z - lg.p.z) < 0.01f) { bright = true; colr = n.c; }
        LightFn lfx = [&](const V3&, const V3&) { return V3(0.02f, 0.02f, 0.02f); };
        V3 em = bright ? V3(clampf(colr.x, 0, 1), clampf(colr.y, 0, 1), clampf(colr.z, 0, 1)) : V3(0.01f, 0.01f, 0.01f);
        gfx.box(V3(lg.p.x, WALL_H - 0.04f, lg.p.z), V3(0.45f, 0.03f, 0.14f), M3::I(), T_LIGHT, lfx, -1, em);
        if (bright) gfx.glow(V3(lg.p.x, WALL_H - 0.12f, lg.p.z), 0.7f, colr.x * 0.5f, colr.y * 0.5f, colr.z * 0.5f, 0.5f);
    }
    for (const Door& d : l.doors) {
        float dd = dist2d(V3(d.cx + 0.5f, 0, d.cz + 0.5f), camPos);
        if (dd > l.th.fogEnd) continue;
        int cell = d.cz * l.W + d.cx;
        LightFn lf = [&, cell](const V3& p, const V3& n) { return lightAt(p, n, cell); };
        drawDoor(d, lf);
    }
    for (size_t oi = 0; oi < l.objs.size(); oi++) {
        const Obj& o = l.objs[oi];
        if (o.taken || (hidden && (int)oi == hideObj)) continue;
        float dd = dist2d(o.pos, camPos);
        if (dd > l.th.fogEnd) continue;
        int cell = cellOf(o.pos);
        LightFn lf = [&, cell](const V3& p, const V3& n) { return lightAt(p, n, cell); };
        drawObj(o, lf);
    }
    if (mimic.active() && dist2d(mimic.pos, camPos) < l.th.fogEnd) {
        int cell = cellOf(mimic.pos);
        LightFn lf = [&, cell](const V3& p, const V3& n) { return lightAt(p, n, cell); };
        drawMimicModel(mimic, lf, t);
    }
}

// ---------------------------------------------------------------------
inline void Game::drawFaceOverlay(float k) {
    float sc = lerpf(0.75f, 1.6f, smooth(0.f, 0.5f, k));
    int sz = (int)(210 * sc);
    int cx = SW / 2 + vrng.irange(-5, 5), cy = SH / 2 + 12 + vrng.irange(-5, 5);
    for (int y = 0; y < sz; y++) {
        int dy = cy - sz / 2 + y;
        if (dy < 0 || dy >= SH) continue;
        for (int x = 0; x < sz; x++) {
            int dx = cx - sz / 2 + x;
            if (dx < 0 || dx >= SW) continue;
            int tx = x * 64 / sz, ty = y * 64 / sz;
            u32 c = gfx.tex[T_FACE].px[ty * 64 + tx];
            float ex = (x - sz * 0.5f) / (sz * 0.5f), ey = (y - sz * 0.5f) / (sz * 0.5f);
            float r2 = ex * ex * 0.9f + ey * ey;
            if (r2 > 1.f) continue;
            float edge = 1.f - smooth(0.6f, 1.f, r2);
            int r = (int)((c >> 16 & 255) * 1.25f), g = (int)((c >> 8 & 255) * 1.1f), b = (int)((c & 255) * 1.1f);
            u32 d = gfx.fb[dy * SW + dx];
            int dr = d >> 16 & 255, dg = d >> 8 & 255, db = d & 255;
            float a = 0.95f * edge;
            gfx.fb[dy * SW + dx] = rgb8((int)(dr + (r - dr) * a), (int)(dg + (g - dg) * a), (int)(db + (b - db) * a));
        }
    }
}

inline void Game::renderHud() {
    const u32 txt = 0xFFB8B8B8u, dim = 0xFF707070u, red = 0xFFD04040u, yel = 0xFFE0C050u;
    // hiding in a locker: slats
    if (hidden) {
        gfx.rect(0, 0, SW, 70, 0xFF050505u);
        gfx.rect(0, 170, SW, 70, 0xFF050505u);
        gfx.rect(0, 70, 60, 100, 0xFF050505u);
        gfx.rect(SW - 60, 70, 60, 100, 0xFF050505u);
        for (int x = 60; x < SW - 60; x += 12) gfx.rect(x, 70, 5, 100, 0xFF050505u);
        gfx.textc(SW / 2, 180, "HOLD STILL. DON'T BREATHE.", 0xFF606060u, 1);
    }
    // crosshair
    if (ui == UI_NONE && !hidden) {
        gfx.rect(SW / 2 - 1, SH / 2, 3, 1, 0xFF909090u, 0.7f);
        gfx.rect(SW / 2, SH / 2 - 1, 1, 3, 0xFF909090u, 0.7f);
    }
    // objective
    std::string ob = objective();
    gfx.text(6, 6, "OBJECTIVE", dim, 1);
    gfx.text(6, 13, ob.c_str(), txt, 1);
    // vitals
    int hx = 6, hy = SH - 14;
    gfx.text(hx, hy - 7, "HEALTH", dim, 1);
    gfx.rect(hx, hy, 64, 5, 0xFF200808u);
    gfx.rect(hx, hy, (int)(64 * sat(P.health / 100.f)), 5, P.health < 35.f ? 0xFFE03030u : 0xFFA02020u);
    gfx.frame2d(hx - 1, hy - 1, 66, 7, 0xFF505050u);
    gfx.text(hx + 72, hy - 7, "BATTERY", dim, 1);
    gfx.rect(hx + 72, hy, 48, 5, 0xFF202008u);
    gfx.rect(hx + 72, hy, (int)(48 * sat(P.battery / 100.f)), 5, P.battery < 20.f ? 0xFFE03030u : 0xFFC0B030u);
    gfx.frame2d(hx + 71, hy - 1, 50, 7, 0xFF505050u);
    char b[48];
    snprintf(b, sizeof(b), "x%d", P.spare);
    gfx.text(hx + 124, hy - 1, b, txt, 1);
    if (stamina < 99.f) {
        gfx.rect(hx, hy + 8, 64, 2, 0xFF303030u);
        gfx.rect(hx, hy + 8, (int)(64 * stamina / 100.f), 2, 0xFFA0A0A0u);
    }
    // inventory (right)
    int ix = SW - 6, iy = SH - 14;
    std::string inv;
    if (P.hasPistol) { snprintf(b, sizeof(b), "AMMO %d", P.ammo); gfx.text(ix - gfx.textW(b), iy - 14, b, P.ammo > 0 ? txt : red, 1); }
    if (P.medkits > 0) { snprintf(b, sizeof(b), "MEDKIT x%d", P.medkits); gfx.text(ix - gfx.textW(b), iy - 7, b, txt, 1); }
    if (!P.genOn) { snprintf(b, sizeof(b), "FUSES %d/3", fusesHeld() + (P.fuseInstalled ? 3 - fusesHeld() : 0)); gfx.text(ix - gfx.textW(b), iy, b, yel, 1); }
    else if (!P.secUnlocked) { snprintf(b, sizeof(b), "TAPES %d/3", tapesFound()); gfx.text(ix - gfx.textW(b), iy, b, yel, 1); }
    if (P.elevCalled && !P.elevReady) {
        int s = (int)ceilf(maxf(P.elevTimer, 0.f));
        snprintf(b, sizeof(b), "ELEVATOR 0:%02d", s);
        gfx.textc(SW / 2, 22, b, (s < 15 && ((int)(t * 4) & 1)) ? 0xFFFF6060u : 0xFFE0A040u, 2);
    }
    // prompt + hold progress
    if (ui == UI_NONE && !prompt.empty()) {
        gfx.textc(SW / 2, SH / 2 + 22, prompt.c_str(), 0xFFE0E0E0u, 1);
        if (holdT > 0 && curTarget.obj >= 0) {
            float k = holdT / maxf(holdTime(L().objs[curTarget.obj]), 0.1f);
            gfx.rect(SW / 2 - 30, SH / 2 + 31, 60, 4, 0xFF202020u);
            gfx.rect(SW / 2 - 30, SH / 2 + 31, (int)(60 * sat(k)), 4, 0xFFD0D0D0u);
            gfx.frame2d(SW / 2 - 31, SH / 2 + 30, 62, 6, 0xFF606060u);
        }
    }
    if (msgT > 0.f && !msg.empty()) {
        u32 a = msgT < 0.6f ? 0xFF707070u : 0xFFD8D8D8u;
        auto lines = wrapText(msg, 70);
        int y = 40;
        for (auto& ln : lines) { gfx.textc(SW / 2, y, ln.c_str(), a, 1); y += 7; }
    }
    // subtitles
    int sy = SH - 36 - (int)subs.size() * 0;
    std::vector<std::pair<std::string, u32>> sl;
    for (auto& s : subs) {
        float fade = s.t < 0.2f ? s.t / 0.2f : (s.dur - s.t < 0.4f ? maxf(0.f, (s.dur - s.t) / 0.4f) : 1.f);
        u32 col = s.col;
        int r = (int)((col >> 16 & 255) * fade), g = (int)((col >> 8 & 255) * fade), bb = (int)((col & 255) * fade);
        for (auto& ln : wrapText(s.text, 74)) sl.push_back({ln, rgb8(r, g, bb)});
    }
    int total = (int)sl.size();
    sy = SH - 22 - total * 7;
    if (total) gfx.rect(0, sy - 3, SW, total * 7 + 5, 0xFF000000u, 0.45f);
    for (auto& p : sl) { gfx.textc(SW / 2, sy, p.first.c_str(), p.second, 1); sy += 7; }
    // pistol view-model
    if (P.hasPistol && !hidden && ui == UI_NONE && mode == M_PLAY) {
        int ox = SW - 78 + (int)(recoil * 60), oy = SH - 56 + (int)(recoil * 80) + (int)(sinf(bob) * 1.5f);
        gfx.rect(ox, oy, 46, 14, 0xFF1A1A1Cu);
        gfx.rect(ox + 2, oy + 2, 42, 4, 0xFF38383Cu);
        gfx.rect(ox + 28, oy + 14, 14, 30, 0xFF141416u);
        gfx.rect(ox + 30, oy + 14, 4, 30, 0xFF26262Au);
        gfx.rect(ox + 22, oy + 14, 8, 8, 0xFF101012u);
        if (muzzle > 0) {
            gfx.rect(ox - 12, oy - 6, 14, 18, 0xFFFFE080u, 0.9f);
            gfx.rect(ox - 6, oy - 2, 8, 10, 0xFFFFFFFFu, 0.9f);
        }
    }
}

inline void Game::drawNoteBox(const char* text, int w, int h, const char* title) {
    int x = (SW - w) / 2, y = (SH - h) / 2 - 6;
    gfx.rect(x - 2, y - 2, w + 4, h + 4, 0xFF000000u, 0.7f);
    gfx.rect(x, y, w, h, 0xFFB0A488u, 1.f);
    gfx.frame2d(x, y, w, h, 0xFF50482Cu);
    gfx.text(x + 8, y + 6, title, 0xFF4A3A20u, 1, false);
    int ly = y + 18;
    std::string s = text;
    size_t p = 0;
    for (;;) {
        size_t q = s.find('\n', p);
        std::string line = s.substr(p, q == std::string::npos ? std::string::npos : q - p);
        gfx.text(x + 8, ly, line.c_str(), 0xFF201810u, 1, false);
        ly += 7;
        if (q == std::string::npos) break;
        p = q + 1;
    }
    gfx.textc(SW / 2, y + h - 10, "[E] CLOSE", 0xFF4A3A20u, 1, false);
}

inline void Game::renderUi() {
    const u32 panel = 0xFF081408u, grn = 0xFF50E070u, dgrn = 0xFF1A6030u;
    char b[96];
    auto frame = [&](int x, int y, int w, int h, const char* title) {
        gfx.rect(x - 3, y - 3, w + 6, h + 6, 0xFF101010u);
        gfx.rect(x, y, w, h, panel);
        gfx.frame2d(x, y, w, h, dgrn);
        gfx.rect(x, y, w, 9, 0xFF0C2810u);
        gfx.text(x + 4, y + 2, title, grn, 1, false);
        for (int yy = y + 10; yy < y + h; yy += 2) gfx.rect(x, yy, w, 1, 0xFF000000u, 0.18f);
    };
    switch (ui) {
        case UI_NOTE: drawNoteBox(noteShown >= 0 && noteShown < 5 ? NOTES[noteShown] : "...", 250, 112, "NOTE"); break;
        case UI_COMMS: {
            frame(40, 40, 240, 140, "COMMS CONSOLE - BATTERY BACKUP");
            snprintf(b, sizeof(b), "FREQ  %.1f MHZ", tuneFreq);
            gfx.text(52, 58, b, grn, 2, false);
            float err = fabsf(tuneFreq - tuneTarget), strength = 1.f - sat(err / 3.5f);
            gfx.text(52, 78, "SIGNAL", dgrn, 1, false);
            gfx.rect(90, 78, 120, 6, 0xFF0A2010u);
            gfx.rect(90, 78, (int)(120 * strength), 6, strength > 0.9f ? 0xFFA0FFA0u : grn);
            gfx.frame2d(89, 77, 122, 8, dgrn);
            // waveform
            int my = 118, py = my;
            for (int x = 0; x < 216; x++) {
                float n = (vrng.f() - 0.5f) * 2.f * (1.f - strength) * 26.f;
                float s = sinf(x * 0.25f + t * 14.f) * strength * 18.f;
                int y = my + (int)(n + s);
                if (x) gfx.line(52 + x - 1, py, 52 + x, y, grn);
                py = y;
            }
            gfx.text(52, 150, "A / D : TUNE   HOLD ON A CLEAR SIGNAL", dgrn, 1, false);
            gfx.text(52, 160, "ESC : LEAVE", dgrn, 1, false);
            if (tuneHold > 0) gfx.rect(52, 170, (int)(216 * sat(tuneHold / 1.6f)), 2, grn);
            break;
        }
        case UI_REPORT: {
            const ReportDef& rd = REPORTS[P.cur];
            frame(30, 36, 260, 150, rd.header);
            if (repStage < 2) {
                gfx.text(40, 56, "SPEAKING INTO THE LOG...", dgrn, 1, false);
                gfx.text(40, 72, rd.prompt[repStage], grn, 1, false);
                gfx.text(40, 92, "[1]", grn, 1, false); gfx.text(60, 92, rd.opt[repStage][0], 0xFFC0F0C8u, 1, false);
                gfx.text(40, 108, "[2]", grn, 1, false); gfx.text(60, 108, rd.opt[repStage][1], 0xFFC0F0C8u, 1, false);
                gfx.text(40, 150, "THE STATION KEEPS ALL RECORDINGS.", dgrn, 1, false);
            } else {
                if ((int)(t * 2) & 1) { gfx.rect(40, 58, 5, 5, 0xFFE02020u); }
                gfx.text(50, 57, "REC", 0xFFE04040u, 1, false);
                gfx.text(40, 78, "\"", grn, 1, false);
                gfx.text(46, 78, rd.opt[0][P.reportPick[P.cur][0]], 0xFFC0F0C8u, 1, false);
                gfx.text(46, 92, rd.opt[1][P.reportPick[P.cur][1]], 0xFFC0F0C8u, 1, false);
                gfx.text(40, 130, "SOMETHING IS LISTENING.", 0xFFE04040u, 1, false);
                for (int x = 0; x < 216; x++) { int h = (int)(2 + 7 * fabsf(sinf(x * 0.4f + t * 20.f)) * (vrng.f())); gfx.rect(40 + x, 112 - h / 2, 1, h, grn); }
            }
            break;
        }
        case UI_SECPC: {
            frame(50, 44, 220, 130, "STATION 09 SECURITY");
            gfx.text(62, 62, "OVERRIDE CODE REQUIRED", grn, 1, false);
            for (int i = 0; i < 6; i++) {
                int x = 66 + i * 30;
                gfx.frame2d(x, 84, 22, 28, dgrn);
                if (i < (int)codeIn.size()) { char c[2] = {codeIn[i], 0}; gfx.text(x + 6, 92, c, grn, 2, false); }
                else if (i == (int)codeIn.size() && ((int)(t * 2) & 1)) gfx.rect(x + 6, 106, 10, 2, grn);
            }
            gfx.text(62, 124, "TYPE DIGITS  ENTER: SUBMIT  ESC: LEAVE", dgrn, 1, false);
            if (!uiMsg.empty()) gfx.textc(160, 144, uiMsg.c_str(), 0xFFE04040u, 1, false);
            break;
        }
        case UI_CORE: {
            frame(30, 30, 260, 160, "ECHO CORE - VOICE ARCHIVE");
            gfx.text(42, 48, "1,214 RECORDINGS. 3 STAFF VOICEPRINTS.", grn, 1, false);
            gfx.text(42, 62, "THE ARCHIVE IS WHAT TEACHES IT TO SPEAK.", grn, 1, false);
            gfx.text(42, 80, "PURGING WILL OVERLOAD THE SONIC LATTICE.", 0xFFE0A040u, 1, false);
            gfx.text(42, 90, "IT WILL NOT TAKE THIS KINDLY.", 0xFFE0A040u, 1, false);
            gfx.text(42, 118, "[1]  PURGE THE ARCHIVE", 0xFFFF6060u, 1, false);
            gfx.text(42, 134, "[2]  LEAVE IT ALONE", grn, 1, false);
            gfx.text(42, 166, "ESC: STEP AWAY", dgrn, 1, false);
            break;
        }
        case UI_JOURNAL: {
            frame(30, 22, 260, 190, "JOURNAL");
            int y = 38;
            gfx.text(40, y, "OBJECTIVE:", grn, 1, false); y += 8;
            for (auto& ln : wrapText(objective(), 60)) { gfx.text(40, y, ln.c_str(), 0xFFC0F0C8u, 1, false); y += 7; }
            y += 6;
            snprintf(b, sizeof(b), "RECORDINGS %d/3     FUSES %d/3     REPORTS %d/3", tapesFound(), fusesHeld() + (P.fuseInstalled ? 3 - fusesHeld() : 0), (int)P.reportDone[0] + (int)P.reportDone[1] + (int)P.reportDone[2]);
            gfx.text(40, y, b, grn, 1, false); y += 12;
            std::string code = "OVERRIDE CODE:  ";
            for (int k = 0; k < 3; k++) {
                if (P.tape[k]) { snprintf(b, sizeof(b), "%d%d ", TAPES[k].digitsA, TAPES[k].digitsB); code += b; }
                else code += "-- ";
            }
            gfx.text(40, y, code.c_str(), 0xFFE0C050u, 1, false); y += 12;
            for (int k = 0; k < 3; k++) if (P.tape[k]) { gfx.text(40, y, TAPES[k].title, dgrn, 1, false); y += 7; }
            y += 6;
            gfx.text(40, y, "WASD MOVE  SHIFT RUN  E INTERACT  F LIGHT", dgrn, 1, false); y += 7;
            gfx.text(40, y, "R BATTERY  H MEDKIT  CLICK FIRE  ESC PAUSE", dgrn, 1, false); y += 10;
            gfx.text(40, y, "IF THE RADIO SPEAKS - DO NOT TRUST IT.", 0xFFA04040u, 1, false); y += 7;
            gfx.text(40, y, "IT LEARNS WHEN YOU TALK.", 0xFFA04040u, 1, false);
            gfx.textc(160, 202, "TAB: CLOSE", dgrn, 1, false);
            break;
        }
    }
}

inline void Game::renderTitle() {
    V3 sp = pos; int sc = P.cur; V3 sstart = pos; bool sg = P.genOn, st = P.tripped;
    P.cur = 0; P.genOn = true; P.tripped = false; flickerBoost = 0.5f;
    pos = V3(3.5f, 0, 8.9f);
    camPos = V3(3.5f, 1.5f, 8.9f);
    camYaw = PI * 0.5f + 0.15f * sinf(t * 0.2f);
    camPitch = 0.02f * sinf(t * 0.31f);
    renderWorld();
    P.cur = sc; pos = sp; P.genOn = sg; P.tripped = st; flickerBoost = 0.f; (void)sstart;
    post.vign = 0.8f; post.grain = 0.1f;
    gfx.rect(0, 0, SW, SH, 0xFF000000u, 0.35f);
    gfx.textc(SW / 2, 30, "STATION 09", 0xFFC02020u, 5);
    gfx.textc(SW / 2, 66, "E C H O", 0xFF909090u, 3);
    gfx.textc(SW / 2, 100, "A SURVIVAL HORROR", 0xFF707070u, 1);
    if ((int)(t * 1.6f) & 1) gfx.textc(SW / 2, 150, "PRESS ENTER TO BEGIN", 0xFFD0D0D0u, 1);
    gfx.textc(SW / 2, 176, "WASD MOVE   MOUSE LOOK   SHIFT RUN   E INTERACT   F FLASHLIGHT\nR BATTERY   H MEDKIT   TAB JOURNAL   CLICK FIRE   ESC PAUSE", 0xFF808080u, 1);
    gfx.textc(SW / 2, 206, "PLAY IN THE DARK. WEAR HEADPHONES.", 0xFF603030u, 1);
}

inline void Game::renderCut() {
    if (cs.empty() || csI >= cs.size()) { gfx.clear(0xFF000000u); return; }
    const CutStep& s = cs[csI];
    float k = sat(csT / maxf(s.dur, 0.01f));
    float e = smooth(0.f, 1.f, k);
    float black = lerpf(s.b0, s.b1, e);
    bool hasCam = !(s.c0.x == 0 && s.c0.y == 0 && s.c0.z == 0);
    if (hasCam && black < 0.995f) {
        camPos = V3(lerpf(s.c0.x, s.c1.x, e), lerpf(s.c0.y, s.c1.y, e), lerpf(s.c0.z, s.c1.z, e));
        camYaw = s.y0 + angDiff(s.y0, s.y1) * e;
        camPitch = lerpf(s.p0, s.p1, e);
        renderWorld();
    } else gfx.clear(0xFF000000u);
    if (black > 0.001f) gfx.rect(0, 0, SW, SH, 0xFF000000u, clampf(black, 0.f, 1.f));
    // letterbox
    gfx.rect(0, 0, SW, 20, 0xFF000000u);
    gfx.rect(0, SH - 20, SW, 20, 0xFF000000u);
    if (s.fx == 3) post.red = 0.25f + 0.15f * sinf(t * 8.f);
    if (s.fx == 2) { post.shakeX = vrng.irange(-2, 2); post.shakeY = vrng.irange(-2, 2); post.noise = 0.02f; }
    if (!s.text.empty()) {
        int n = (int)(csT * 28.f);
        std::string shown = s.text.substr(0, std::min<size_t>(s.text.size(), (size_t)n));
        bool big = black > 0.9f && s.text.size() < 34 && s.text.find('\n') == std::string::npos;
        int scale = big ? 2 : 1;
        int ty = black > 0.5f ? 104 - (int)std::count(s.text.begin(), s.text.end(), '\n') * 4 : SH - 44;
        u32 col = black > 0.5f ? 0xFFD0D0D0u : 0xFFE0E0E0u;
        if (s.text.find("STATION 09") == 0) { gfx.textc(SW / 2, 86, "STATION 09", 0xFFC02020u, 4); gfx.textc(SW / 2, 118, "DEEP RESEARCH FACILITY", 0xFF808080u, 1); }
        else gfx.textc(SW / 2, ty, shown.c_str(), col, scale);
    }
    // subtitles
    int sy = SH - 34;
    for (auto& sb : subs) {
        auto ls = wrapText(sb.text, 74);
        for (auto& ln : ls) { gfx.textc(SW / 2, sy, ln.c_str(), sb.col, 1); sy -= 7; }
    }
    if (csId != CS_END_GOOD && csId != CS_END_BAD) gfx.text(SW - 72, SH - 12, "SPACE: SKIP", 0xFF505050u, 1, false);
    if (s.fx == 1 && hasCam) post.noise = 0.012f;
    post.vign = 0.75f;
}

inline void Game::renderDead() {
    if (deadT < 1.4f) {
        camPos = pos + V3(0, 1.55f, 0); camYaw = yaw; camPitch = pitch;
        renderWorld();
        drawFaceOverlay(deadT / 1.4f);
        post.noise = 0.1f; post.red = 0.4f; post.shakeX = vrng.irange(-3, 3); post.shakeY = vrng.irange(-3, 3);
    } else {
        gfx.clear(0xFF000000u);
        float a = sat((deadT - 1.4f) / 0.8f);
        post.fade = 0;
        if (a > 0.1f) gfx.textc(SW / 2, 96, "IT LEARNED YOUR VOICE.", 0xFFC02020u, 2);
        if (deadT > 3.5f && ((int)(t * 2) & 1)) gfx.textc(SW / 2, 150, "PRESS ENTER TO TRY AGAIN", 0xFF909090u, 1);
        if (deadT > 3.5f) gfx.textc(SW / 2, 164, "(RETURNS TO THE LAST CHECKPOINT)", 0xFF505050u, 1);
    }
}

inline void Game::renderEnd() {
    gfx.clear(0xFF000000u);
    post.vign = 0.7f;
    if (endT > 0.5f) {
        gfx.textc(SW / 2, 40, "STATION 09: ECHO", 0xFFC02020u, 3);
        if (endingKind == 1) {
            gfx.textc(SW / 2, 84, "ENDING: SILENCE", 0xFFD0D0D0u, 2);
            gfx.textc(SW / 2, 108, "YOU DESTROYED THE ARCHIVE. IT CANNOT SPEAK ANY MORE.\nBUT SOMETHING STILL REMEMBERS HOW YOU SOUND.", 0xFF909090u, 1);
        } else {
            gfx.textc(SW / 2, 84, "ENDING: ECHO", 0xFFD0D0D0u, 2);
            gfx.textc(SW / 2, 108, "YOU LEFT THE ARCHIVE INTACT. IT RODE UP WITH YOUR VOICE.\nSTATION 09 IS NO LONGER THE ONLY PLACE IT CAN SPEAK.", 0xFF909090u, 1);
        }
    }
    if (endT > 3.f) {
        char b[96];
        int m = (int)(playTime / 60.f), s = (int)playTime % 60;
        snprintf(b, sizeof(b), "TIME %d:%02d     DEATHS %d     REPORTS %d/3     RECORDINGS %d/3", m, s, deaths, (int)P.reportDone[0] + (int)P.reportDone[1] + (int)P.reportDone[2], tapesFound());
        gfx.textc(SW / 2, 150, b, 0xFF808080u, 1);
        gfx.textc(SW / 2, 170, endingKind == 1 ? "TRY THE OTHER CHOICE AT THE CORE FOR A DIFFERENT END." : "TRY PURGING THE ARCHIVE FOR A DIFFERENT END.", 0xFF606060u, 1);
    }
    if (endT > 6.f && ((int)(t * 2) & 1)) gfx.textc(SW / 2, 200, "PRESS ENTER", 0xFFB0B0B0u, 1);
}

inline void Game::render() {
    post = Post();
    switch (mode) {
        case M_TITLE: renderTitle(); break;
        case M_PLAY:
        case M_PAUSE: {
            float bobY = hidden ? 0.f : sinf(bob) * 0.03f * bobAmp * (sprinting ? 1.7f : 1.f);
            camPos = pos + V3(0, 1.55f + bobY + (hidden ? -0.1f : 0.f), 0);
            camYaw = yaw + (vrng.f() - 0.5f) * 0.03f * shake;
            camPitch = pitch + recoil + (vrng.f() - 0.5f) * 0.03f * shake;
            if (transTo >= 0 && transT < 0.5f && false) {}
            renderWorld();
            if (js > 0.f) drawFaceOverlay(1.f - js / 0.8f);
            renderHud();
            if (ui != UI_NONE) renderUi();
            // post parameters from the player's state
            post.vign = 0.55f + 0.35f * fear;
            post.grain = 0.07f + 0.05f * fear;
            post.chroma = fear > 0.55f ? 1.f : 0.f;
            post.desat = 0.35f * fear + (P.health < 35.f ? 0.3f : 0.f);
            post.red = hurt * 0.6f + (P.health < 30.f ? 0.08f * (0.5f + 0.5f * sinf(t * 6.f)) : 0.f);
            post.flash = flashFx * 0.6f;
            post.noise = (js > 0.f ? 0.12f : 0.f) + (mimic.active() && mimic.sees ? 0.01f : 0.f);
            int sh = (int)(shake * 3.f);
            if (sh > 0) { post.shakeX = vrng.irange(-sh, sh); post.shakeY = vrng.irange(-sh, sh); }
            if (transTo >= 0) post.fade = transT < 0.5f ? transT * 2.f : (1.f - transT) * 2.f;
            if (mode == M_PAUSE) {
                gfx.rect(0, 0, SW, SH, 0xFF000000u, 0.6f);
                gfx.textc(SW / 2, 96, "PAUSED", 0xFFD0D0D0u, 3);
                gfx.textc(SW / 2, 130, "ESC: RESUME     Q: QUIT TO TITLE", 0xFF909090u, 1);
            }
            break;
        }
        case M_CUT: renderCut(); break;
        case M_DEAD: renderDead(); break;
        case M_END: renderEnd(); break;
    }
    gfx.post(post);
}

}  // namespace S9
