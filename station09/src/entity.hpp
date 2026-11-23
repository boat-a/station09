// STATION 09: ECHO  --  "the Mimic": AI and model
#pragma once
#include "world.hpp"

namespace S9 {

enum MState { MS_INACTIVE, MS_PATROL, MS_INVESTIGATE, MS_HUNT, MS_STAGGER, MS_SEARCH, MS_RECOVER, MS_SCRIPT };

struct PlayerInfo {
    V3 pos;
    bool hidden = false, flash = false, sprint = false, dead = false;
    V3 hideSpot;
};

struct MimicEvents {
    bool attack = false, spotted = false, step = false, doorOpened = false, pulledLocker = false;
    float stepVol = 1.f;
    V3 doorPos;
};

struct Mimic {
    V3 pos;
    float yaw = 0;
    int state = MS_INACTIVE;
    float st = 0, respawn = 30.f, repathT = 0, waitT = 0, stepT = 0, anim = 0;
    float attackCd = 0, aggression = 0, seeAcc = 0, lostT = 0, spasm = 0, speakT = 0, moveSpeed = 0;
    V3 goal, lastSeen, hideSpot;
    bool hasGoal = false, sees = false, enraged = false, knowsHide = false, moving = false;
    int hits = 0, wpIdx = -1;
    std::vector<short> dist;
    int dtx = -1, dty = -1;

    bool active() const { return state != MS_INACTIVE; }

    void despawn(float respawnTime) { state = MS_INACTIVE; respawn = respawnTime; hits = 0; knowsHide = false; seeAcc = 0; }

    void spawnAt(V3 p, int newState = MS_PATROL) {
        pos = p; pos.y = 0;
        state = newState; st = 0; hasGoal = false; seeAcc = 0; lostT = 0; knowsHide = false; waitT = 0; attackCd = 1.5f;
    }

    // choose a spawn waypoint far from, and out of sight of, the player
    bool spawnAway(Level& lv, V3 player, float minDist = 14.f) {
        std::vector<int> ok;
        float far = -1; int farI = -1;
        for (size_t i = 0; i < lv.waypoints.size(); i++) {
            float d = dist2d(lv.waypoints[i], player);
            if (d > far) { far = d; farI = (int)i; }
            if (d >= minDist && !lv.los(lv.waypoints[i], player)) ok.push_back((int)i);
        }
        int pick = ok.empty() ? farI : ok[grng.irange(0, (int)ok.size() - 1)];
        if (pick < 0) return false;
        spawnAt(lv.waypoints[pick], MS_PATROL);
        wpIdx = pick;
        return true;
    }

    void hear(Level& lv, V3 src, float radius) {
        if (state != MS_PATROL && state != MS_INVESTIGATE && !(state == MS_HUNT && !sees)) return;
        if (dist2d(pos, src) > radius) return;
        (void)lv;
        goal = src; hasGoal = true; waitT = 0;
        if (state != MS_HUNT) { state = MS_INVESTIGATE; st = 0; }
        else lastSeen = src;
    }

    void stagger() {
        hits++;
        state = MS_STAGGER; st = 0; seeAcc = 0;
    }

    int pickWaypoint(Level& lv, V3 player) {
        if (lv.waypoints.empty()) return -1;
        int best = -1; float bd = 1e9f;
        bool chase = grng.chance(0.3f + 0.5f * aggression);
        int tries = chase ? 4 : 1;
        for (int k = 0; k < tries; k++) {
            int i = grng.irange(0, (int)lv.waypoints.size() - 1);
            float d = dist2d(lv.waypoints[i], player);
            if (i == wpIdx) continue;
            if (chase && d < 4.f) continue;
            if (d < bd) { bd = d; best = i; }
        }
        if (best < 0) best = grng.irange(0, (int)lv.waypoints.size() - 1);
        return best;
    }

    // walk along the BFS gradient towards cell (tx,tz). returns true on arrival
    bool stepToward(Level& lv, int tx, int tz, float spd, float dt, MimicEvents& ev) {
        moving = false;
        if (tx != dtx || tz != dty || repathT <= 0) { lv.bfs(tx, tz, dist); dtx = tx; dty = tz; repathT = 0.7f; }
        int cx = (int)pos.x, cz = (int)pos.z;
        if (cx == tx && cz == tz) return true;
        int W = lv.W;
        short best = dist[cz * W + cx];
        int bx = cx, bz = cz;
        for (int dz = -1; dz <= 1; dz++)
            for (int dx = -1; dx <= 1; dx++) {
                if (!dx && !dz) continue;
                int nx = cx + dx, nz = cz + dz;
                if (nx < 0 || nz < 0 || nx >= lv.W || nz >= lv.H) continue;
                if (lv.navBlock[nz * W + nx]) continue;
                if (dx && dz && (lv.navBlock[cz * W + nx] || lv.navBlock[nz * W + cx])) continue;
                short d = dist[nz * W + nx];
                if (d < best) { best = d; bx = nx; bz = nz; }
            }
        if (bx == cx && bz == cz) {
            // standing in an odd spot: nudge towards the target cell centre
            V3 c(tx + 0.5f, 0, tz + 0.5f);
            V3 d = c - pos; float l = vlen(d);
            if (l > 0.05f) lv.move(pos, d.x / l * spd * dt, d.z / l * spd * dt, 0.28f, true);
            return false;
        }
        // doors
        Door* door = lv.doorAtCell(bx, bz);
        if (door && door->open < 0.7f) {
            if (door->target < 1.f) { door->target = 1.f; ev.doorOpened = true; ev.doorPos = V3(bx + 0.5f, 1.f, bz + 0.5f); }
            door->jammed = false;
            return false;
        }
        V3 gp(bx + 0.5f, 0, bz + 0.5f);
        V3 d = gp - pos;
        float l = vlen(d);
        if (l < 0.001f) return false;
        d = d * (1.f / l);
        float want = atan2f(d.x, d.z);
        yaw += angDiff(yaw, want) * minf(1.f, dt * 8.f);
        lv.move(pos, d.x * spd * dt, d.z * spd * dt, 0.28f, true);
        moving = true;
        moveSpeed = spd;
        return false;
    }

    void directToward(Level& lv, V3 target, float spd, float dt) {
        V3 d = target - pos; d.y = 0;
        float l = vlen(d);
        if (l < 0.01f) return;
        d = d * (1.f / l);
        yaw += angDiff(yaw, atan2f(d.x, d.z)) * minf(1.f, dt * 10.f);
        lv.move(pos, d.x * spd * dt, d.z * spd * dt, 0.28f, true);
        moving = true; moveSpeed = spd;
    }

    void update(Level& lv, float dt, const PlayerInfo& pl, MimicEvents& ev) {
        if (state == MS_INACTIVE) { respawn -= dt; return; }
        st += dt; attackCd -= dt; repathT -= dt; moving = false;
        float dpl = dist2d(pos, pl.pos);
        float baseSpd = enraged ? 1.2f : 1.0f;

        // ---- perception ----
        bool canSee = false;
        float vr = pl.flash ? 13.f : 6.5f;
        if (pl.sprint) vr += 3.f;
        if (pl.hidden) vr = knowsHide ? 2.5f : 0.f;
        if (!pl.dead && state != MS_STAGGER && state != MS_SCRIPT && dpl < vr) {
            if (lv.los(pos, pl.pos)) {
                V3 d = pl.pos - pos;
                float diff = fabsf(angDiff(yaw, atan2f(d.x, d.z)));
                if (diff < 1.3f || dpl < 2.6f) canSee = true;
            }
        }
        sees = canSee;
        if (canSee) { seeAcc += dt * (1.f + (1.f - dpl / maxf(vr, 1.f)) * 3.f); lastSeen = pl.pos; lostT = 0; }
        else { seeAcc = maxf(0.f, seeAcc - dt * 0.5f); lostT += dt; }

        if ((state == MS_PATROL || state == MS_INVESTIGATE) && seeAcc > 0.55f && !pl.dead) {
            state = MS_HUNT; st = 0; ev.spotted = true; hasGoal = false;
        }

        switch (state) {
            case MS_PATROL: {
                if (!hasGoal) {
                    waitT -= dt;
                    if (waitT <= 0) {
                        wpIdx = pickWaypoint(lv, pl.pos);
                        if (wpIdx >= 0) { goal = lv.waypoints[wpIdx]; hasGoal = true; }
                    }
                    break;
                }
                if (stepToward(lv, (int)goal.x, (int)goal.z, 1.35f * baseSpd, dt, ev)) { hasGoal = false; waitT = grng.range(1.5f, 4.f); }
                break;
            }
            case MS_INVESTIGATE: {
                if (hasGoal) {
                    if (stepToward(lv, (int)goal.x, (int)goal.z, 2.0f * baseSpd, dt, ev)) { hasGoal = false; waitT = 2.5f; }
                } else {
                    waitT -= dt;
                    yaw += dt * 1.2f * sinf(st * 2.f);
                    if (waitT <= 0) { state = MS_PATROL; st = 0; waitT = 0.5f; }
                }
                break;
            }
            case MS_HUNT: {
                if (pl.hidden && knowsHide) {
                    if (dist2d(pos, pl.hideSpot) < 1.5f) { state = MS_SEARCH; st = 0; break; }
                    stepToward(lv, (int)pl.hideSpot.x, (int)pl.hideSpot.z, 3.0f * baseSpd, dt, ev);
                    break;
                }
                float spd = (enraged ? 3.9f : 3.3f);
                if (canSee) {
                    if (dpl < 1.15f && attackCd <= 0 && !pl.dead) { ev.attack = true; attackCd = 2.5f; state = MS_RECOVER; st = 0; break; }
                    if (dpl < 3.5f) directToward(lv, pl.pos, spd, dt);
                    else stepToward(lv, (int)pl.pos.x, (int)pl.pos.z, spd, dt, ev);
                } else {
                    bool arrived = stepToward(lv, (int)lastSeen.x, (int)lastSeen.z, spd * 0.9f, dt, ev);
                    if (arrived || lostT > 7.f) { state = MS_INVESTIGATE; goal = lastSeen; hasGoal = false; waitT = 3.f; st = 0; }
                }
                break;
            }
            case MS_SEARCH: {
                yaw += dt * 0.8f * sinf(st * 3.f);
                if (!pl.hidden) { state = MS_HUNT; st = 0; break; }
                if (st > 5.5f) {
                    if (grng.chance(0.5f)) { ev.pulledLocker = true; state = MS_RECOVER; st = 0; attackCd = 2.f; }
                    else { state = MS_PATROL; knowsHide = false; hasGoal = false; waitT = 1.f; }
                }
                break;
            }
            case MS_RECOVER: {
                if (st > 3.2f) { state = canSee ? MS_HUNT : MS_PATROL; st = 0; hasGoal = false; waitT = 0.5f; }
                break;
            }
            case MS_STAGGER: {
                if (st > 4.5f) { state = MS_HUNT; st = 0; lastSeen = pl.pos; lostT = 0; }
                break;
            }
            case MS_SCRIPT: {
                V3 d = pl.pos - pos;
                yaw += angDiff(yaw, atan2f(d.x, d.z)) * minf(1.f, dt * 3.f);
                break;
            }
        }
        if (moving) {
            anim += dt * moveSpeed * 2.6f;
            stepT -= dt;
            if (stepT <= 0) { stepT = 1.5f / maxf(moveSpeed, 0.8f) * 0.55f; ev.step = true; ev.stepVol = clampf(moveSpeed / 3.f, 0.4f, 1.f); }
        }
        speakT = maxf(0.f, speakT - dt);
        spasm = state == MS_HUNT ? 1.f : (state == MS_STAGGER ? 1.5f : 0.25f);
    }
};

// ---------- low-poly model: wrong proportions on purpose ----------
inline void drawMimicModel(const Mimic& m, const LightFn& lf, float time) {
    M3 R0 = M3::rotY(m.yaw);
    float walk = m.moving ? sinf(m.anim) : 0.f;
    float bobY = m.moving ? fabsf(cosf(m.anim)) * 0.06f : 0.f;
    bool hunt = m.state == MS_HUNT, stag = m.state == MS_STAGGER, script = m.state == MS_SCRIPT;
    float lean = hunt ? 0.45f : (stag ? 1.0f : (m.state == MS_SEARCH ? 0.5f : 0.08f));
    float drop = stag ? 0.55f : (hunt ? 0.12f : 0.f);
    float jx = (vrng.f() - 0.5f) * 0.06f * m.spasm, jz = (vrng.f() - 0.5f) * 0.06f * m.spasm;
    float headTilt = 0.35f * sinf(time * 0.7f) + (script ? 0.5f : 0.f) + (vrng.f() - 0.5f) * 0.3f * m.spasm;
    V3 root = m.pos + V3(jx, bobY - drop, jz);
    M3 body = R0 * M3::rotX(lean);

    // torso (rotates with lean about the hips)
    V3 hip(0, 0.85f, 0);
    auto bodyPt = [&](V3 lc) { return root + R0 * hip + body * (lc - hip); };
    auto bpart = [&](V3 lc, V3 half, const M3& extra, int tex, int front = -1) {
        gfx.box(bodyPt(lc), half, body * extra, tex, lf, front, V3(0.03f, 0.03f, 0.035f));
    };
    bpart(V3(0, 1.3f, 0), V3(0.2f, 0.45f, 0.12f), M3::I(), T_SKIN);
    bpart(V3(0, 0.95f, 0), V3(0.17f, 0.12f, 0.1f), M3::I(), T_SKIN);
    // neck + head: the neck is too long
    float neck = 0.3f + (m.speakT > 0 ? 0.12f : 0.f);
    bpart(V3(0, 1.75f + neck * 0.5f, 0.02f), V3(0.045f, neck * 0.5f + 0.1f, 0.045f), M3::I(), T_SKIN);
    M3 hrot = M3::rotZ(headTilt) * M3::rotX(-0.2f * lean);
    V3 headC = V3(0, 1.95f + neck, 0.05f);
    gfx.box(bodyPt(headC), V3(0.13f, 0.19f, 0.13f), body * hrot, T_SKIN, lf, T_FACE, V3(0.04f, 0.04f, 0.045f));
    float jaw = m.speakT > 0 ? 0.12f + 0.06f * sinf(time * 30.f) : (hunt ? 0.1f : 0.f);
    if (jaw > 0.f) gfx.box(bodyPt(headC + V3(0, -0.22f - jaw * 0.6f, 0.02f)), V3(0.1f, 0.05f + jaw * 0.3f, 0.1f), body * hrot, T_BLACK, lf);
    // arms: long, dragging
    for (int s = -1; s <= 1; s += 2) {
        float sw = s * walk * 0.7f * (hunt ? 1.4f : 1.f);
        float reach = hunt ? -1.15f : (stag ? -0.4f : 0.f);
        M3 ua = body * M3::rotX(sw + reach) * M3::rotZ(s * 0.08f);
        V3 sh = bodyPt(V3(s * 0.28f, 1.62f, 0));
        float L1 = 0.62f, L2 = 0.7f;
        gfx.box(sh + ua * V3(0, -L1 * 0.5f, 0), V3(0.05f, L1 * 0.5f, 0.05f), ua, T_SKIN, lf, -1, V3(0.03f, 0.03f, 0.035f));
        V3 el = sh + ua * V3(0, -L1, 0);
        M3 fa = ua * M3::rotX(-0.3f - (hunt ? 0.4f : 0.f));
        gfx.box(el + fa * V3(0, -L2 * 0.5f, 0), V3(0.04f, L2 * 0.5f, 0.04f), fa, T_SKIN, lf, -1, V3(0.03f, 0.03f, 0.035f));
        V3 hd = el + fa * V3(0, -L2 - 0.08f, 0);
        gfx.box(hd, V3(0.05f, 0.1f, 0.03f), fa, T_SKIN, lf, -1, V3(0.03f, 0.03f, 0.035f));
    }
    // legs
    for (int s = -1; s <= 1; s += 2) {
        float sw = s * walk * 0.8f;
        M3 th = R0 * M3::rotX(sw - (stag ? 1.2f : 0.f));
        V3 hp = root + R0 * V3(s * 0.12f, 0.85f, 0);
        float L1 = 0.42f, L2 = 0.45f;
        gfx.box(hp + th * V3(0, -L1 * 0.5f, 0), V3(0.065f, L1 * 0.5f, 0.065f), th, T_SKIN, lf, -1, V3(0.03f, 0.03f, 0.035f));
        V3 kn = hp + th * V3(0, -L1, 0);
        M3 sh = th * M3::rotX(0.35f + (walk * s > 0 ? 0.3f : 0.f) + (stag ? 1.8f : 0.f));
        gfx.box(kn + sh * V3(0, -L2 * 0.5f, 0), V3(0.05f, L2 * 0.5f, 0.05f), sh, T_SKIN, lf, -1, V3(0.03f, 0.03f, 0.035f));
    }
}

}  // namespace S9
