// STATION 09: ECHO  --  levels, geometry, collision, navigation
#pragma once
#include "gfx.hpp"

namespace S9 {

static const float WALL_H = 3.0f;

enum ObjType {
    O_DESK, O_CRATE, O_LOCKER, O_COMMS, O_REPORT, O_TAPE, O_FUSE, O_BATTERY, O_MEDKIT, O_AMMO, O_PISTOL,
    O_NOTE, O_STAIRS, O_BREAKER, O_FUSEBOX, O_GEN, O_SECPC, O_CORE, O_ELEV, O_CORPSE
};

struct Obj {
    ObjType type = O_DESK;
    int id = 0;
    V3 pos;
    float yaw = 0;
    V3 half;           // local half extents
    float hw = 0.4f, hd = 0.4f;  // world-space xz half extents (for collision)
    bool solid = true;
    bool taken = false;
    int state = 0;
    float t = 0;
    int tex = T_BOX;
    int target = 0;    // stairs: target level
    char glyph = ' ';
};

enum DoorKind { DK_MANUAL, DK_POWER, DK_SECURITY };
struct Door {
    int cx = 0, cz = 0, kind = DK_MANUAL;
    bool alongX = true;   // panel slides along X (passage runs N-S)
    float open = 0, target = 0;
    bool jammed = false;  // scripted: cannot be opened for a while
};

struct Light { V3 p; float phase = 0; int idx = 0; };
struct Trig { int cx, cz, id; bool done = false; };

struct Quad {
    V3 p[4];
    float u[4], v[4];
    V3 n;
    u8 tex;
};

struct Theme {
    int wallTex, floorTex, ceilTex;
    V3 ambient;
    float fogEnd;
    int noteBase;
};

// ------------------------------------------------------------------
//  MAPS.  Rows are built from adjacent string literals so each room is
//  easy to read.  Legend (see parseObjects):
//   # wall  M metal  T tile  F flesh  W window       . floor  , tile  g grating  x blood  r carpet
//   D door  P powered door  S security door
//   k desk  C crate  L locker  n comms  R report terminal  t/u/q tapes  f/j/h fuses
//   b battery  m medkit  a ammo  p pistol  o note  v/V stairs  B breaker  X fuse box  Y generator
//   N security PC  Q core terminal  E elevator  z corpse  w waypoint  1-9 triggers  @ start
// ------------------------------------------------------------------
static const char* MAP_B1[] = {
    "################################",
    "#" "kkrrrrL" "#" ",k,n,k," "#" ".C....L" "#" "....v." "#",
    "#" "rrrrrrr" "#" ",,,,,,," "#" "......." "#" "......" "#",
    "#" "Rrrr@rr" "D" ",,,,,,," "D" "...kk.." "D" "..o..." "#",
    "#" "rrrrrrr" "#" ",,,,,,," "#" "...t..." "#" "..5..." "#",
    "#" "rorrrrr" "#" ",,,,,,," "#" "....2.z" "#" "......" "#",
    "#" "kkrrrrr" "#" ",C,,,,," "#" "bkk...." "#" "C....C" "#",
    "#" "###D###" "#" "###D###" "#" "###D###" "#" "##D###" "#",
    "#" "..w......." ".x........" "........w." "#",
    "#" ".........1" ".........." "...4......" "#",
    "#" "###D###" "#" "###D###" "#" "###D###" "#" "##D###" "#",
    "#" "C.C...b" "#" "L,L,,,L" "#" "CC....." "#" "L....L" "#",
    "#" "C......" "#" ",,,,,,," "#" "......." "#" "......" "#",
    "#" "......." "D" ",,,,,,," "D" "......." "D" "......" "#",
    "#" "..CC..." "#" ",,,3,,," "#" "...w..." "#" "L....L" "#",
    "#" "......C" "#" ",,,,,,," "#" "......." "#" "......" "#",
    "#" "fC....w" "#" ",,,,,,," "#" "k....mC" "#" "LLL..b" "#",
    "################################",
};

static const char* MAP_B2[] = {
    "####################################",
    "#" ".....Y....X.." "#" "ggggCggj" "#" "L....N...kk" "#",
    "#" "............." "#" "gggggggg" "#" ".........p." "#",
    "#" "..M.......M.." "#" "ggMgggMg" "#" "..kq......." "#",
    "#" "............." "#" "gggggggg" "#" "..........." "#",
    "#" "............." "D" "ggggxggg" "#" "..........." "#",
    "#" "..M.......M.." "#" "ggMgggMg" "#" "...z......m" "#",
    "#" "C..........C." "#" "gggggggg" "#" "..........a" "#",
    "#" "w...x.....w.." "#" "wggggogg" "#" "w.........." "#",
    "#" "######D######" "#" "####D###" "#" "#####P#####" "#",
    "#" ".....w...." ".........." ".......5.5" "...." "#",
    "#" "x........." ".........." "...1......" "...." "#",
    "#" "###D####" "#" "####D####" "#" "##D##" "#" "##D##" "#" "#S#" "#",
    "#" "kk.....k" "#" "L.L...L.L" "#" ".C..B" "#" "....." "#" "..." "#",
    "#" ".......R" "#" "kk.....kk" "#" "....." "#" ".C..." "#" "..." "#",
    "#" "........" "#" "........." "#" "....." "#" "....." "#" "..z" "#",
    "#" "...C...." "D" "....3...." "D" "....." "D" "....." "#" "..." "#",
    "#" "..C....." "#" "kk.....kk" "#" "C...." "#" "....." "#" "..." "#",
    "#" "........" "#" "........." "#" "....." "#" "....." "#" "..." "#",
    "#" "hC.....b" "#" "ku.....mk" "#" "..w.." "#" "L.v.L" "#" ".V." "#",
    "####################################",
};

static const char* MAP_B3[] = {
    "##############################",
    "###########" "...v...." "###########",
    "###########" "........" "###########",
    "###########" ".o......" "###########",
    "###########" "........" "###########",
    "##############" "D" "###############",
    "#" "..w..1........" "..........w..." "#",
    "#" "x......x......" "......x......." "#",
    "#" "WWWWDWWWW" "#" "WWWDWWWW" "#" "WWWDWWWWW" "#",
    "#" "Lkk..k..L" "#" "ggg3gggg" "#" "L...k..kL" "#",
    "#" "........." "F" "gggggggg" "F" "........." "#",
    "#" "...xx...." "F" "ggFgggFg" "F" "..xx....." "#",
    "#" "R.......x" "D" "gggggggg" "D" "........." "#",
    "#" "..zz....." "F" "ggggQggg" "F" "..z......" "#",
    "#" "........." "F" "gggggggg" "F" ".o......a" "#",
    "#" ".L.....m." "F" "ggFgggFg" "F" "........." "#",
    "#" "bk..w.k.." "F" "gggwgggg" "F" "Lm..w...b" "#",
    "#" "####D####" "#" "FFFFDFFF" "#" "####D####" "#",
    "#" "..w..x........" "........x..w.." "#",
    "#" ".....x........" "........4....." "#",
    "#" "####D####" "#" "####D###" "#" "####D####" "#",
    "#" "CC..w..CC" "#" "L......L" "#" "L...w...." "#",
    "#" "........." "#" "...5...." "#" "........B" "#",
    "#" ".C......C" "#" "..k..k.." "#" "C......C." "#",
    "#" "b.......m" "#" "...E...." "#" "C...b...." "#",
    "##############################",
};

struct Level {
    int W = 0, H = 0, idx = 0;
    std::vector<char> g;         // terrain chars (objects replaced by floor char)
    std::vector<std::vector<Quad>> cq;
    std::vector<Obj> objs;
    std::vector<Door> doors;
    std::vector<int> doorIdx;    // per cell, -1 if none
    std::vector<Light> lights;
    std::vector<Trig> trigs;
    std::vector<V3> waypoints;
    std::vector<char> navBlock;
    V3 start;
    Theme th;

    static bool isWallCh(char c) { return c == '#' || c == 'M' || c == 'T' || c == 'F' || c == 'W' || c == ' '; }
    bool wallAt(int x, int z) const { return x < 0 || z < 0 || x >= W || z >= H || isWallCh(g[z * W + x]); }
    char ch(int x, int z) const { return (x < 0 || z < 0 || x >= W || z >= H) ? '#' : g[z * W + x]; }
    Door* doorAtCell(int x, int z) {
        if (x < 0 || z < 0 || x >= W || z >= H) return nullptr;
        int i = doorIdx[z * W + x];
        return i >= 0 ? &doors[i] : nullptr;
    }

    void load(int level) {
        idx = level;
        const char* const* rows = nullptr;
        int n = 0;
        if (level == 0) { rows = MAP_B1; n = sizeof(MAP_B1) / sizeof(char*); th = {T_CONC, T_FLOORC, T_CEIL, V3(0.05f, 0.05f, 0.06f), 15.f, 0}; }
        if (level == 1) { rows = MAP_B2; n = sizeof(MAP_B2) / sizeof(char*); th = {T_CONCD, T_FLOORC, T_CEILP, V3(0.05f, 0.055f, 0.05f), 16.f, 2}; }
        if (level == 2) { rows = MAP_B3; n = sizeof(MAP_B3) / sizeof(char*); th = {T_TILEW, T_FLOORT, T_CEIL, V3(0.04f, 0.06f, 0.07f), 17.f, 3}; }
        H = n;
        W = 0;
        for (int i = 0; i < n; i++) W = std::max(W, (int)strlen(rows[i]));
        g.assign(W * H, '#');
        for (int z = 0; z < H; z++)
            for (int x = 0; x < (int)strlen(rows[z]); x++) g[z * W + x] = rows[z][x];
        doorIdx.assign(W * H, -1);
        parseObjects();
        buildMesh();
        placeLamps();
        buildNav();
    }

    char floorGuess(int x, int z) {
        int cnt[128] = {0};
        const int dx[8] = {1, -1, 0, 0, 1, 1, -1, -1}, dz[8] = {0, 0, 1, -1, 1, -1, 1, -1};
        for (int k = 0; k < 8; k++) {
            char c = ch(x + dx[k], z + dz[k]);
            if (c == ',' || c == 'g' || c == 'x' || c == 'r' || c == '.') cnt[(int)c]++;
        }
        char best = '.';
        int bc = 0;
        const char order[5] = {'.', ',', 'g', 'r', 'x'};
        for (char c : order) if (cnt[(int)c] > bc) { bc = cnt[(int)c]; best = c; }
        return best;
    }

    float faceYaw(int x, int z, bool& found, V3& awayDir) {
        found = true;
        if (wallAt(x, z - 1)) { awayDir = V3(0, 0, 1); return 0.f; }
        if (wallAt(x, z + 1)) { awayDir = V3(0, 0, -1); return PI; }
        if (wallAt(x - 1, z)) { awayDir = V3(1, 0, 0); return PI * 0.5f; }
        if (wallAt(x + 1, z)) { awayDir = V3(-1, 0, 0); return -PI * 0.5f; }
        found = false;
        awayDir = V3(0, 0, 1);
        return 0.f;
    }

    void parseObjects() {
        objs.clear(); doors.clear(); trigs.clear(); waypoints.clear(); lights.clear();
        int noteCounter = 0;
        for (int z = 0; z < H; z++)
            for (int x = 0; x < W; x++) {
                char c = g[z * W + x];
                if (isWallCh(c) || c == '.' || c == ',' || c == 'g' || c == 'x' || c == 'r') continue;
                V3 cc(x + 0.5f, 0, z + 0.5f);
                if (c == 'D' || c == 'P' || c == 'S') {
                    Door d;
                    d.cx = x; d.cz = z;
                    d.kind = c == 'D' ? DK_MANUAL : (c == 'P' ? DK_POWER : DK_SECURITY);
                    d.alongX = wallAt(x - 1, z) && wallAt(x + 1, z);
                    doorIdx[z * W + x] = (int)doors.size();
                    doors.push_back(d);
                    continue;
                }
                char fc = floorGuess(x, z);
                g[z * W + x] = fc;
                if (c == '@') { start = cc; continue; }
                if (c == 'w') { waypoints.push_back(cc); continue; }
                if (c >= '1' && c <= '9') { trigs.push_back({x, z, c - '0'}); continue; }
                Obj o;
                o.pos = cc; o.glyph = c;
                bool wallFound;
                V3 away;
                o.yaw = faceYaw(x, z, wallFound, away);
                bool mount = false;
                switch (c) {
                    case 'k': o.type = O_DESK; o.half = V3(0.45f, 0.42f, 0.4f); o.tex = T_DESK; break;
                    case 'C': o.type = O_CRATE; o.half = V3(0.42f, hf(x, z, 3) > 0.55f ? 0.8f : 0.42f, 0.42f); o.tex = T_CRATE; o.yaw = hf(x, z, 4) * 0.5f; wallFound = false; break;
                    case 'L': o.type = O_LOCKER; o.half = V3(0.36f, 0.95f, 0.34f); o.tex = T_LOCKER; mount = true; break;
                    case 'n': o.type = O_COMMS; o.half = V3(0.55f, 0.55f, 0.3f); o.tex = T_BOX; mount = true; break;
                    case 'R': o.type = O_REPORT; o.half = V3(0.4f, 0.6f, 0.28f); o.tex = T_BOX; mount = true; break;
                    case 't': case 'u': case 'q': o.type = O_TAPE; o.id = c == 't' ? 0 : (c == 'u' ? 1 : 2); o.half = V3(0.14f, 0.05f, 0.1f); o.solid = false; o.tex = T_BLACK; o.pos.y = 0.0f; break;
                    case 'f': case 'j': case 'h': o.type = O_FUSE; o.id = c == 'f' ? 0 : (c == 'j' ? 1 : 2); o.half = V3(0.05f, 0.14f, 0.05f); o.solid = false; o.tex = T_LIGHT; break;
                    case 'b': o.type = O_BATTERY; o.half = V3(0.06f, 0.1f, 0.06f); o.solid = false; o.tex = T_WARN; break;
                    case 'm': o.type = O_MEDKIT; o.half = V3(0.17f, 0.1f, 0.12f); o.solid = false; o.tex = T_PAPER; break;
                    case 'a': o.type = O_AMMO; o.half = V3(0.1f, 0.05f, 0.07f); o.solid = false; o.tex = T_WARN; break;
                    case 'p': o.type = O_PISTOL; o.half = V3(0.12f, 0.04f, 0.07f); o.solid = false; o.tex = T_BLACK; break;
                    case 'o': o.type = O_NOTE; o.id = th.noteBase + noteCounter++; o.half = V3(0.14f, 0.01f, 0.18f); o.solid = false; o.tex = T_PAPER; break;
                    case 'v': case 'V': o.type = O_STAIRS; o.half = V3(0.5f, 0.5f, 0.42f); o.tex = T_METAL; o.id = c == 'V' ? 1 : 0;
                        o.target = (idx == 0) ? 1 : (idx == 1 ? (c == 'V' ? 2 : 0) : 1); mount = true; break;
                    case 'B': o.type = O_BREAKER; o.id = idx == 1 ? 0 : 1; o.half = V3(0.4f, 0.55f, 0.15f); o.tex = T_BOX; mount = true; break;
                    case 'X': o.type = O_FUSEBOX; o.half = V3(0.45f, 0.6f, 0.15f); o.tex = T_BOX; mount = true; break;
                    case 'Y': o.type = O_GEN; o.half = V3(0.95f, 0.8f, 0.7f); o.tex = T_GEN; mount = true; break;
                    case 'N': o.type = O_SECPC; o.half = V3(0.55f, 0.6f, 0.3f); o.tex = T_BOX; mount = true; break;
                    case 'Q': o.type = O_CORE; o.half = V3(0.5f, 0.7f, 0.5f); o.tex = T_BOX; o.yaw = 0; wallFound = false; break;
                    case 'E': o.type = O_ELEV; o.half = V3(0.9f, 1.3f, 0.12f); o.tex = T_ELEV; mount = true; break;
                    case 'z': o.type = O_CORPSE; o.half = V3(0.4f, 0.12f, 0.2f); o.tex = T_SKIN; o.yaw = hf(x, z, 8) * PI; wallFound = false; break;
                    default: continue;
                }
                o.half.y = o.half.y;
                o.pos.y = (o.type == O_TAPE || o.type == O_FUSE || o.type == O_BATTERY || o.type == O_MEDKIT || o.type == O_AMMO || o.type == O_PISTOL || o.type == O_NOTE) ? 0.f : 0.f;
                if (mount && wallFound) o.pos = o.pos - away * (0.5f - o.half.z - 0.02f);
                float cy = cosf(o.yaw), sy = sinf(o.yaw);
                o.hw = fabsf(cy) * o.half.x + fabsf(sy) * o.half.z;
                o.hd = fabsf(sy) * o.half.x + fabsf(cy) * o.half.z;
                objs.push_back(o);
            }
    }

    int wallTexFor(char c, int x, int z) const {
        switch (c) {
            case 'M': return T_METAL;
            case 'T': return T_TILEW;
            case 'F': return T_FLESH;
            case 'W': return T_TERMOFF;
            default:
                if (th.wallTex == T_CONC && hf(x, z, 12) > 0.8f) return T_CONCD;
                return th.wallTex;
        }
    }
    int floorTexFor(char c) const {
        switch (c) {
            case ',': return T_FLOORT;
            case 'g': return T_GRATE;
            case 'x': return T_BLOOD;
            case 'r': return T_CARPET;
            default: return th.floorTex;
        }
    }

    void buildMesh() {
        cq.assign(W * H, std::vector<Quad>());
        for (int z = 0; z < H; z++)
            for (int x = 0; x < W; x++) {
                char c = g[z * W + x];
                if (isWallCh(c)) continue;
                auto& q = cq[z * W + x];
                int ft = floorTexFor(c);
                int ct = th.ceilTex;
                if (ct == T_CEILP && ((x + z) % 3 == 0)) ct = T_CEIL;
                if (idx == 2 && (c == 'g')) ct = T_FLESHF;
                for (int sz = 0; sz < 2; sz++)
                    for (int sx = 0; sx < 2; sx++) {
                        float x0 = x + sx * 0.5f, z0 = z + sz * 0.5f, x1 = x0 + 0.5f, z1 = z0 + 0.5f;
                        Quad f;
                        f.p[0] = V3(x0, 0, z0); f.p[1] = V3(x1, 0, z0); f.p[2] = V3(x1, 0, z1); f.p[3] = V3(x0, 0, z1);
                        f.n = V3(0, 1, 0); f.tex = (u8)ft;
                        const float fu[4] = {x0, x1, x1, x0}, fv[4] = {z0, z0, z1, z1};
                        for (int i = 0; i < 4; i++) { f.u[i] = fu[i] * 0.5f; f.v[i] = fv[i] * 0.5f; }
                        q.push_back(f);
                        Quad cl = f;
                        for (int i = 0; i < 4; i++) cl.p[i].y = WALL_H;
                        cl.n = V3(0, -1, 0); cl.tex = (u8)ct;
                        q.push_back(cl);
                    }
                // walls
                struct Dir { int dx, dz; };
                const Dir dirs[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
                for (int d = 0; d < 4; d++) {
                    int nx = x + dirs[d].dx, nz = z + dirs[d].dz;
                    if (!wallAt(nx, nz)) continue;
                    int wt = wallTexFor(ch(nx, nz), nx, nz);
                    for (int s = 0; s < 2; s++)
                        for (int r = 0; r < 4; r++) {
                            float a0 = s * 0.5f, a1 = a0 + 0.5f, y0 = r * (WALL_H / 4), y1 = y0 + WALL_H / 4;
                            Quad w;
                            w.tex = (u8)wt;
                            if (d == 0) { float zz = (float)z; w.p[0] = V3(x + a0, y0, zz); w.p[1] = V3(x + a1, y0, zz); w.p[2] = V3(x + a1, y1, zz); w.p[3] = V3(x + a0, y1, zz); w.n = V3(0, 0, 1);
                                float al[4] = {x + a0, x + a1, x + a1, x + a0}; for (int i = 0; i < 4; i++) w.u[i] = al[i] * 0.5f; }
                            if (d == 1) { float zz = (float)(z + 1); w.p[0] = V3(x + a1, y0, zz); w.p[1] = V3(x + a0, y0, zz); w.p[2] = V3(x + a0, y1, zz); w.p[3] = V3(x + a1, y1, zz); w.n = V3(0, 0, -1);
                                float al[4] = {x + a1, x + a0, x + a0, x + a1}; for (int i = 0; i < 4; i++) w.u[i] = al[i] * 0.5f; }
                            if (d == 2) { float xx = (float)x; w.p[0] = V3(xx, y0, z + a1); w.p[1] = V3(xx, y0, z + a0); w.p[2] = V3(xx, y1, z + a0); w.p[3] = V3(xx, y1, z + a1); w.n = V3(1, 0, 0);
                                float al[4] = {z + a1, z + a0, z + a0, z + a1}; for (int i = 0; i < 4; i++) w.u[i] = al[i] * 0.5f; }
                            if (d == 3) { float xx = (float)(x + 1); w.p[0] = V3(xx, y0, z + a0); w.p[1] = V3(xx, y0, z + a1); w.p[2] = V3(xx, y1, z + a1); w.p[3] = V3(xx, y1, z + a0); w.n = V3(-1, 0, 0);
                                float al[4] = {z + a0, z + a1, z + a1, z + a0}; for (int i = 0; i < 4; i++) w.u[i] = al[i] * 0.5f; }
                            float vv[4] = {y0, y0, y1, y1};
                            for (int i = 0; i < 4; i++) w.v[i] = (WALL_H - vv[i]) * 0.5f;
                            q.push_back(w);
                        }
                }
            }
    }

    void placeLamps() {
        lights.clear();
        for (int z = 0; z < H; z++)
            for (int x = 0; x < W; x++) {
                char c = g[z * W + x];
                if (isWallCh(c) || doorIdx[z * W + x] >= 0) continue;
                V3 p(x + 0.5f, WALL_H - 0.25f, z + 0.5f);
                bool near = false;
                for (auto& l : lights) { float dx = l.p.x - p.x, dz = l.p.z - p.z; if (dx * dx + dz * dz < 4.6f * 4.6f) { near = true; break; } }
                if (near) continue;
                Light l;
                l.p = p; l.idx = (int)lights.size(); l.phase = hf(x, z, 99) * 100.f;
                lights.push_back(l);
            }
    }

    void buildNav() {
        navBlock.assign(W * H, 0);
        for (int z = 0; z < H; z++)
            for (int x = 0; x < W; x++) if (wallAt(x, z)) navBlock[z * W + x] = 1;
        for (auto& o : objs)
            if (o.solid) {
                int cx = (int)o.pos.x, cz = (int)o.pos.z;
                if (cx >= 0 && cz >= 0 && cx < W && cz < H) navBlock[cz * W + cx] = 1;
                if (o.type == O_GEN) {
                    for (int dz = -1; dz <= 1; dz++)
                        for (int dx = -1; dx <= 1; dx++) {
                            int nx = cx + dx, nz = cz + dz;
                            if (nx >= 0 && nz >= 0 && nx < W && nz < H && fabsf(o.pos.x - (nx + 0.5f)) < o.hw + 0.3f && fabsf(o.pos.z - (nz + 0.5f)) < o.hd + 0.3f) navBlock[nz * W + nx] = 1;
                        }
                }
            }
    }

    // ---------- collision ----------
    bool circleBox(float cx, float cz, float r, float bx0, float bz0, float bx1, float bz1) const {
        float px = clampf(cx, bx0, bx1), pz = clampf(cz, bz0, bz1);
        float dx = cx - px, dz = cz - pz;
        return dx * dx + dz * dz < r * r;
    }
    bool collides(float x, float z, float r, bool ignoreDoors = false) const {
        int x0 = (int)floorf(x - r - 0.01f), x1 = (int)floorf(x + r + 0.01f);
        int z0 = (int)floorf(z - r - 0.01f), z1 = (int)floorf(z + r + 0.01f);
        for (int cz = z0; cz <= z1; cz++)
            for (int cx = x0; cx <= x1; cx++) {
                if (wallAt(cx, cz)) { if (circleBox(x, z, r, (float)cx, (float)cz, cx + 1.f, cz + 1.f)) return true; continue; }
                if (!ignoreDoors) {
                    int di = doorIdx[cz * W + cx];
                    if (di >= 0 && doors[di].open < 0.75f) {
                        const Door& d = doors[di];
                        float hx = d.alongX ? 0.5f : 0.12f, hz = d.alongX ? 0.12f : 0.5f;
                        if (circleBox(x, z, r, cx + 0.5f - hx, cz + 0.5f - hz, cx + 0.5f + hx, cz + 0.5f + hz)) return true;
                    }
                }
            }
        for (const auto& o : objs) {
            if (!o.solid || o.taken) continue;
            if (fabsf(o.pos.x - x) > o.hw + r + 0.1f || fabsf(o.pos.z - z) > o.hd + r + 0.1f) continue;
            if (circleBox(x, z, r, o.pos.x - o.hw, o.pos.z - o.hd, o.pos.x + o.hw, o.pos.z + o.hd)) return true;
        }
        return false;
    }
    void move(V3& p, float dx, float dz, float r, bool ignoreDoors = false) {
        if (!collides(p.x + dx, p.z, r, ignoreDoors)) p.x += dx;
        if (!collides(p.x, p.z + dz, r, ignoreDoors)) p.z += dz;
    }
    bool blockedForSight(float x, float z) const {
        int cx = (int)floorf(x), cz = (int)floorf(z);
        if (wallAt(cx, cz)) return true;
        int di = (cx >= 0 && cz >= 0 && cx < W && cz < H) ? doorIdx[cz * W + cx] : -1;
        if (di >= 0 && doors[di].open < 0.5f) return true;
        return false;
    }
    bool los(V3 a, V3 b) const {
        V3 d = b - a;
        d.y = 0;
        float len = vlen(d);
        int steps = (int)(len / 0.2f) + 1;
        for (int i = 1; i < steps; i++) {
            float t = (float)i / steps;
            if (blockedForSight(a.x + d.x * t, a.z + d.z * t)) return false;
        }
        return true;
    }

    // BFS distance field from a target cell. Doors are passable (the thing opens them).
    void bfs(int tx, int tz, std::vector<short>& dist) const {
        dist.assign(W * H, 9999);
        if (tx < 0 || tz < 0 || tx >= W || tz >= H) return;
        std::vector<int> q;
        q.reserve(W * H);
        dist[tz * W + tx] = 0;
        q.push_back(tz * W + tx);
        for (size_t h = 0; h < q.size(); h++) {
            int c = q[h], cx = c % W, cz = c / W;
            const int dx[4] = {1, -1, 0, 0}, dz[4] = {0, 0, 1, -1};
            for (int k = 0; k < 4; k++) {
                int nx = cx + dx[k], nz = cz + dz[k];
                if (nx < 0 || nz < 0 || nx >= W || nz >= H) continue;
                int ni = nz * W + nx;
                if (navBlock[ni] || dist[ni] <= dist[c] + 1) continue;
                dist[ni] = dist[c] + 1;
                q.push_back(ni);
            }
        }
    }
    bool nearestOpenCell(float x, float z, int& ox, int& oz) const {
        int cx = (int)x, cz = (int)z;
        for (int r = 0; r < 4; r++)
            for (int dz = -r; dz <= r; dz++)
                for (int dx = -r; dx <= r; dx++) {
                    int nx = cx + dx, nz = cz + dz;
                    if (nx < 0 || nz < 0 || nx >= W || nz >= H || navBlock[nz * W + nx]) continue;
                    ox = nx; oz = nz;
                    return true;
                }
        return false;
    }
};

}  // namespace S9
