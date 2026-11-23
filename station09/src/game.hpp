// STATION 09: ECHO  --  game logic
#pragma once
#include "entity.hpp"
#include "story.hpp"

namespace S9 {

enum Mode { M_TITLE, M_PLAY, M_CUT, M_DEAD, M_END, M_PAUSE };
enum UiKind { UI_NONE, UI_NOTE, UI_JOURNAL, UI_COMMS, UI_REPORT, UI_SECPC, UI_CORE };
enum CutId { CS_INTRO, CS_GEN, CS_SEC, CS_B3, CS_CHAMBER, CS_PURGE, CS_END_GOOD, CS_END_BAD };

struct Prog {
    int cur = 0;
    float health = 100, battery = 100;
    int spare = 1, medkits = 0, ammo = 0;
    bool hasPistol = false;
    bool fuseHeld[3] = {false, false, false};
    bool fuseInstalled = false, genOn = false, tripped = false;
    bool tape[3] = {false, false, false};
    bool secUnlocked = false, commsDone = false;
    bool reportDone[3] = {false, false, false};
    int reportPick[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    int tripsDone = 0;
    bool coreDone = false, purged = false, elevCalled = false, elevReady = false, sighted = false;
    float elevTimer = 0;
    bool visited[3] = {true, false, false};
    bool noteRead[5] = {false, false, false, false, false};
};

struct Sub { std::string text; u32 col; float t, dur; };
struct QLine { int id; bool mimic; bool spatial; V3 pos; };
struct CutStep {
    float dur;
    std::string text;
    int sfx, voice;
    bool mimicVoice;
    V3 c0, c1;
    float y0, p0, y1, p1, b0, b1;
    int fx;
};
struct Checkpoint {
    bool valid = false;
    Prog P;
    V3 pos;
    float yaw = 0;
    std::vector<Obj> objs[3];
    std::vector<Door> doors[3];
    std::vector<Trig> trigs[3];
};
struct NL { V3 p, c; float r; const std::vector<uint8_t>* vis; };

struct Game {
    Level lv[3];
    std::vector<uint8_t> lampVis[3][64];
    Mimic mimic;
    Prog P;
    Checkpoint cpt;
    Mode mode = M_TITLE;
    int ui = UI_NONE;
    Input* in = nullptr;
    bool quit = false, inited = false;

    // player
    V3 pos;
    float yaw = 0, pitch = 0, bob = 0, stamina = 100, staminaDelay = 0, stepAcc = 0;
    bool flashOn = false, sprinting = false, hidden = false;
    int hideObj = -1;
    float hideYaw = 0;
    float t = 0, levelT = 0, fear = 0, shake = 0, hurt = 0, flashFx = 0, lightsOut = 0, flickerBoost = 0, gunCd = 0, muzzle = 0, recoil = 0;
    float holdT = 0;
    int holdTarget = -1;
    std::string msg;
    float msgT = 0;
    std::vector<Sub> subs;
    std::vector<QLine> vq;
    float vqT = 0, mimicVoiceT = 25.f, tapeNoiseT = 0;
    // transitions
    int transTo = -1;
    float transT = 0;
    // terminals
    float tuneFreq = 98.f, tuneTarget = 93.7f, tuneHold = 0, tuneKeyT = 0;
    int repStage = 0, repIdx = 0;
    float repT = 0;
    std::string codeIn, uiMsg;
    float uiT = 0;
    int noteShown = -1;
    // cutscene
    std::vector<CutStep> cs;
    size_t csI = 0;
    float csT = 0;
    int csId = 0;
    bool csStarted = false;
    // jump scare / death / ending
    float js = 0;
    int jsType = 0;
    float deadT = 0;
    int deaths = 0;
    float endT = 0;
    int endingKind = 0;
    float playTime = 0;
    float corpseT = -1;
    // lighting
    std::vector<NL> nl;
    V3 camPos, camFwd;
    V3 amb;
    // audio
    int aDrone = 0, aWind = 0, aHum = 0, aStatic = 0, aHeart = 0, aAlarm = 0, aElev = 0;
    float heartT = 0;
    Post post;
    float ambT = 14.f, transPrev = 0.f, bobAmp = 0.f;

    Level& L() { return lv[P.cur]; }
    const Level& L() const { return lv[P.cur]; }

    // ------------------------------------------------------------------
    void init() {
        if (inited) return;
        inited = true;
        gfx.init();
        audio.init();
        synthAllVoices();
        for (int i = 0; i < 3; i++) {
            lv[i].load(i);
            for (size_t k = 0; k < lv[i].lights.size() && k < 64; k++) {
                auto& vis = lampVis[i][k];
                vis.assign(lv[i].W * lv[i].H, 0);
                V3 lp = lv[i].lights[k].p;
                for (int z = 0; z < lv[i].H; z++)
                    for (int x = 0; x < lv[i].W; x++) {
                        if (lv[i].wallAt(x, z)) continue;
                        V3 c(x + 0.5f, 1.f, z + 0.5f);
                        if (dist2d(lp, c) < 9.f && lv[i].los(lp, c)) vis[z * lv[i].W + x] = 1;
                    }
            }
        }
    }

    void msgShow(const std::string& s, float d = 3.f) { msg = s; msgT = d; }
    void pushSub(const std::string& s, u32 col, float dur) {
        subs.push_back({s, col, 0.f, dur});
        if (subs.size() > 3) subs.erase(subs.begin());
    }

    // ------------------------------------------------------------------
    void newGame() {
        P = Prog();
        for (int i = 0; i < 3; i++) lv[i].load(i);
        pos = lv[0].start; yaw = PI * 0.5f; pitch = 0;
        stamina = 100; flashOn = false; hidden = false; hideObj = -1;
        mimic = Mimic(); mimic.despawn(9999.f);
        subs.clear(); vq.clear(); vqT = 0; msg.clear();
        deaths = 0; playTime = 0; levelT = 0; ui = UI_NONE; js = 0; hurt = 0; lightsOut = 0;
        cpt.valid = false;
        mimicVoiceT = 30.f;
        audio.stopAll();
        startLoops();
        startCutscene(CS_INTRO);
    }

    void startLoops() {
        aDrone = audio.play(S_DRONE, 0.42f, 1.f, nullptr, true);
        aWind = audio.play(S_WIND, 0.22f, 1.f, nullptr, true);
        aHum = 0; aStatic = 0; aHeart = 0; aAlarm = 0; aElev = 0;
        if (P.genOn && !P.tripped) aHum = audio.play(S_HUM, 0.25f, 1.f, nullptr, true);
    }

    void saveCheckpoint() {
        cpt.valid = true;
        cpt.P = P; cpt.pos = pos; cpt.yaw = yaw;
        for (int i = 0; i < 3; i++) { cpt.objs[i] = lv[i].objs; cpt.doors[i] = lv[i].doors; cpt.trigs[i] = lv[i].trigs; }
    }
    void loadCheckpoint() {
        P = cpt.P; pos = cpt.pos; yaw = cpt.yaw; pitch = 0;
        for (int i = 0; i < 3; i++) { lv[i].objs = cpt.objs[i]; lv[i].doors = cpt.doors[i]; lv[i].trigs = cpt.trigs[i]; }
        P.health = maxf(P.health, 60.f);
        hidden = false; hideObj = -1; ui = UI_NONE; js = 0; hurt = 0; lightsOut = 0; holdT = 0; holdTarget = -1;
        mimic = Mimic(); mimic.despawn(35.f);
        mimic.enraged = P.purged;
        subs.clear(); vq.clear(); vqT = 0;
        audio.stopAll();
        startLoops();
        levelT = 0;
    }

    // ------------------------------------------------------------------
    //  lighting
    // ------------------------------------------------------------------
    bool power() const { return P.genOn && !P.tripped; }

    void buildLights() {
        nl.clear();
        Level& l = L();
        bool pw = power() && lightsOut <= 0.f;
        V3 col = P.cur == 0 ? V3(1.0f, 0.93f, 0.74f) : (P.cur == 1 ? V3(0.82f, 1.0f, 0.86f) : V3(0.78f, 0.9f, 1.0f));
        for (size_t i = 0; i < l.lights.size() && i < 64; i++) {
            const Light& lg = l.lights[i];
            if (dist2d(lg.p, pos) > 22.f) continue;
            NL n;
            n.p = lg.p; n.vis = &lampVis[P.cur][i];
            if (pw) {
                float f = 1.f;
                bool unstable = (lg.idx % 5 == 0);
                float tt = t * 9.f + lg.phase;
                if (unstable || flickerBoost > 0) {
                    float k = sinf(tt * 4.1f) * sinf(tt * 2.3f + 1.f) + sinf(tt * 13.f) * 0.4f;
                    float thr = flickerBoost > 0 ? -0.1f : 0.55f;
                    if (k > thr && (unstable || flickerBoost > 0)) f = (hf((int)(tt * 3.f), lg.idx, 7) > 0.5f) ? 0.12f : 0.9f;
                }
                if (P.cur == 2 && P.purged) { n.c = V3(1.0f, 0.1f, 0.08f) * (0.6f + 0.5f * sinf(t * 5.f)); }
                else n.c = col * (1.25f * f);
                n.r = 8.5f;
            } else {
                if (lg.idx % 3 != 0 || lightsOut > 0.f) continue;
                float pulse = 0.55f + 0.2f * sinf(t * 2.f + lg.phase);
                n.c = V3(0.9f, 0.06f, 0.04f) * pulse;
                n.r = 6.5f;
            }
            nl.push_back(n);
        }
        if (mode == M_CUT) {
            NL n;
            n.p = camPos + camFwd * 1.5f + V3(0, 0.4f, 0); n.vis = nullptr; n.r = 10.f;
            n.c = (csId == CS_PURGE) ? V3(0.55f, 0.12f, 0.1f) : V3(0.5f, 0.55f, 0.62f);
            nl.push_back(n);
        }
        amb = l.th.ambient * (pw ? 1.8f : 1.f);
        if (P.cur == 2 && P.purged) amb = V3(0.12f, 0.02f, 0.02f);
    }

    V3 lightAt(const V3& p, const V3& n, int cell) const {
        V3 c = amb;
        for (const NL& l : nl) {
            if (l.vis && cell >= 0 && !(*l.vis)[cell]) continue;
            V3 d = l.p - p;
            float dd = dot(d, d);
            if (dd > l.r * l.r) continue;
            float dist = sqrtf(dd);
            float att = 1.f - dist / l.r;
            att *= att;
            float ndl = dist > 0.001f ? dot(n, d * (1.f / dist)) : 1.f;
            ndl = clampf(ndl * 0.65f + 0.35f, 0.f, 1.f);
            c = c + l.c * (att * ndl);
        }
        // flashlight
        if (flashOn && !hidden && P.battery > 0.f && mode == M_PLAY) {
            V3 d = p - camPos;
            float dist = vlen(d);
            const float range = 13.f;
            if (dist < range && dist > 0.01f) {
                V3 dn = d * (1.f / dist);
                float cs = dot(dn, camFwd);
                if (cs > 0.72f) {
                    float spot = smooth(0.72f, 0.93f, cs);
                    float att = 1.f - dist / range;
                    att = att * att;
                    float ndl = clampf(-dot(n, dn) * 0.75f + 0.25f, 0.f, 1.f);
                    float strength = 1.9f;
                    if (P.battery < 15.f) strength *= (hf((int)(t * 20.f), 3, 1) > 0.35f ? 1.f : 0.2f);
                    c = c + V3(1.f, 0.95f, 0.8f) * (spot * att * ndl * strength);
                }
            }
        }
        if (muzzle > 0.f) {
            V3 d = camPos - p;
            float dist = vlen(d);
            if (dist < 7.f) c = c + V3(1.f, 0.8f, 0.5f) * ((1.f - dist / 7.f) * 1.3f * muzzle * 6.f);
        }
        float dc = vlen(p - camPos);
        float fog = 1.f - sat((dc - 1.5f) / (L().th.fogEnd - 1.5f));
        c = c * fog;
        return c;
    }

    // ------------------------------------------------------------------
    //  audio helpers
    // ------------------------------------------------------------------
    void sfx(int id, float vol = 1.f, const V3* at = nullptr, float pitch = 1.f) { audio.play(id, vol, pitch, at); }
    void makeNoise(V3 src, float radius) { if (mimic.active()) mimic.hear(L(), src, radius); }

    void queueLine(int id, bool mimicV = false, bool spatial = false, V3 at = V3()) { vq.push_back({id, mimicV, spatial, at}); }
    void updateVoiceQueue(float dt) {
        vqT -= dt;
        if (vqT > 0 || vq.empty()) return;
        QLine q = vq.front();
        vq.erase(vq.begin());
        int snd = q.mimic ? lineSndMimic[q.id] : lineSnd[q.id];
        const LineDef& ld = LD[q.id];
        float dur = audio.duration(snd);
        if (q.spatial) audio.play(snd, 1.0f, 1.f, &q.pos);
        else audio.play(snd, 0.95f);
        std::string name = SPEAKER_NAME[ld.sp];
        pushSub(name + ": " + ld.text, SPEAKER_COL[ld.sp], dur + 0.8f);
        vqT = dur + 0.35f;
    }

    // ------------------------------------------------------------------
    //  objectives
    // ------------------------------------------------------------------
    int fusesHeld() const { return (int)P.fuseHeld[0] + (int)P.fuseHeld[1] + (int)P.fuseHeld[2]; }
    int tapesFound() const { return (int)P.tape[0] + (int)P.tape[1] + (int)P.tape[2]; }

    std::string objective() const {
        char b[160];
        if (!P.commsDone) return "CHECK THE COMMUNICATION SYSTEMS";
        if (!P.genOn) {
            if (P.fuseInstalled) return "RESTART THE BACKUP GENERATOR (B2)";
            if (fusesHeld() < 3) { snprintf(b, sizeof(b), "RESTORE THE POWER: FIND FUSES (%d/3)", fusesHeld()); return b; }
            return "INSTALL THE FUSES AT THE FUSE BOX (B2)";
        }
        if (!P.secUnlocked) {
            if (P.tripped) return "RESET THE BREAKER IN THE B2 CLOSET";
            if (tapesFound() < 3) { snprintf(b, sizeof(b), "RECOVER SECURITY CODES: RECORDINGS (%d/3)", tapesFound()); return b; }
            return "ENTER THE CODE AT THE SECURITY COMPUTER (B2)";
        }
        if (P.cur < 2) return "GO DOWN TO THE RESTRICTED FLOOR (B3)";
        if (!P.coreDone) return "REACH THE ECHO CHAMBER CORE";
        if (!P.elevCalled) {
            if (P.tripped) return "RESET THE BREAKER IN THE EAST ROOM";
            return "ACTIVATE THE EMERGENCY ELEVATOR";
        }
        if (!P.elevReady) return "SURVIVE UNTIL THE ELEVATOR ARRIVES";
        return "GET IN THE ELEVATOR";
    }

    void objectiveChanged() {
        saveCheckpoint();
    }

    // ------------------------------------------------------------------
    //  level transitions
    // ------------------------------------------------------------------
    void goLevel(int n) { transTo = n; transT = 0; hidden = false; hideObj = -1; }

    void arrive(int n, int from) {
        (void)from;
        P.cur = n;
        levelT = 0;
        Level& l = lv[n];
        for (auto& o : l.objs) if (o.type == O_STAIRS && o.id == 0) {
            V3 f(sinf(o.yaw), 0, cosf(o.yaw));
            pos = o.pos + f * 1.0f; pos.y = 0;
            yaw = o.yaw;
        }
        pitch = 0;
        mimic.despawn(n == 1 ? 22.f : 30.f);
        mimic.enraged = P.purged;
        mimic.aggression = n == 2 ? 0.5f : 0.2f;
        flashFx = 0.6f;
        if (n == 2 && P.cur == 2) {
            for (auto& d : l.doors) if (!P.coreDone && ((d.cx == 5 && d.cz == 17) || (d.cx == 24 && d.cz == 17) || (d.cx == 15 && d.cz == 17))) d.jammed = true;
        }
        if (!P.visited[n]) {
            P.visited[n] = true;
            if (n == 2) { saveCheckpoint(); transTo = -1; transT = 0; startCutscene(CS_B3); return; }
            if (n == 1) { queueLine(L_E_STAIRS); msgShow("LEVEL B2 - MAINTENANCE", 3.5f); }
        }
        saveCheckpoint();
        stopHoldAudio();
    }
    void stopHoldAudio() {}

    // ------------------------------------------------------------------
    //  scares
    // ------------------------------------------------------------------
    void jumpscare(int type) {
        js = 0.8f; jsType = type;
        shake = 1.f; flashFx = 0.9f;
        sfx(S_STING2, 1.f); sfx(type == 0 ? S_SCREAM : S_BOOM, 1.f);
    }

    void triggerTrip() {
        P.tripped = true; P.tripsDone++;
        if (aHum) { audio.stop(aHum, 0.2f); aHum = 0; }
        sfx(S_POWEROFF, 1.f);
        lightsOut = 0.f; flickerBoost = 0.f;
        shake = 0.5f;
        queueLine(L_E_TRIP);
        mimic.aggression = minf(1.f, mimic.aggression + 0.3f);
        V3 spawnFrom = pos;
        if (!mimic.active() || mimic.state == MS_INACTIVE) {
            mimic.spawnAway(L(), spawnFrom, 12.f);
        }
        mimic.state = MS_INVESTIGATE; mimic.goal = pos; mimic.hasGoal = true; mimic.st = 0;
        msgShow("THE POWER WENT OUT. SOMETHING IS IN HERE WITH YOU.", 4.f);
    }

    void restorePower() {
        P.tripped = false;
        sfx(S_POWERON, 1.f);
        if (!aHum) aHum = audio.play(S_HUM, 0.25f, 1.f, nullptr, true);
        flickerBoost = 3.f;
        msgShow("POWER RESTORED", 3.f);
        objectiveChanged();
    }

    bool doTrigger(int id) {
        Level& l = L();
        V3 behind = pos - V3(sinf(yaw), 0, cosf(yaw)) * 6.f;
        switch (P.cur * 10 + id) {
            case 1:  // B1 corridor: distant door slam
                sfx(S_SLAM, 1.f, &behind); shake = 0.5f; flickerBoost = 1.5f;
                queueLine(L_M_OVER, true, true, behind);
                break;
            case 2:  // B1 break room: the corpse sits up
                for (auto& o : l.objs) if (o.type == O_CORPSE && o.pos.x > 20) { o.state = 1; o.t = 0; }
                corpseT = 0.f;
                break;
            case 3: {  // B1 restroom: a locker bangs open
                V3 lk(11.5f, 1.f, 11.5f);
                sfx(S_LOCKER, 1.f, &lk); shake = 0.6f; flickerBoost = 2.f;
                queueLine(L_M_HELP, true, true, lk);
                break;
            }
            case 4:  // B1 east corridor: first sighting
                if (!P.sighted) {
                    P.sighted = true;
                    mimic.spawnAt(V3(28.5f, 0, 8.5f), MS_SCRIPT);
                    mimic.yaw = -PI * 0.5f;
                    sfx(S_GROAN, 0.9f); sfx(S_STING1, 0.7f);
                    flickerBoost = 4.f; lightsOut = 0.f;
                    msgShow("...", 2.f);
                }
                break;
            case 5:  // B1 stair room: lights die
                lightsOut = 3.5f;
                sfx(S_WHISPER, 0.9f, &behind); sfx(S_POWEROFF, 0.4f);
                break;
            case 11:  // B2: vent crash
                { V3 v(24.5f, 2.5f, 6.5f); sfx(S_BOOM, 1.f, &v); sfx(S_CREAK, 0.8f, &v); shake = 0.8f; flickerBoost = 2.5f; }
                break;
            case 13: {  // B2 dorm: lockers rattle, a woman's voice
                V3 lk(10.5f, 1.f, 13.5f);
                sfx(S_KNOCK, 1.f, &lk); sfx(S_LOCKER, 0.8f, &lk);
                queueLine(L_M_BED, true, true, lk);
                flickerBoost = 2.f; shake = 0.4f;
                break;
            }
            case 15:  // B2: the power is cut when you reach for the security door
                if (P.genOn && P.tripsDone == 0 && !P.tripped) { triggerTrip(); return true; }
                return false;
            case 21:  // B3: face at the observation window
                if (js <= 0) { jumpscare(1); }
                break;
            case 23:  // B3: chamber
                if (!P.coreDone) { startCutscene(CS_CHAMBER); return true; }
                return false;
            case 24:  // B3: your own voice from the dark
                if (P.reportDone[0] || P.reportDone[1]) {
                    int k = P.reportDone[1] ? 1 : 0;
                    queueLine(REPORTS[k].line[0][P.reportPick[k][0]], true, true, V3(15.5f, 1.f, 22.5f));
                } else queueLine(L_M_HELLO, true, true, V3(15.5f, 1.f, 18.5f));
                flickerBoost = 2.f;
                break;
            case 25:  // B3: lobby - power cut when you reach the elevator
                if (P.coreDone && P.tripsDone == 1 && !P.tripped && !P.elevCalled) { triggerTrip(); return true; }
                return false;
        }
        return true;
    }

    // ------------------------------------------------------------------
    //  mimic voice lures
    // ------------------------------------------------------------------
    void mimicSpeak() {
        if (!mimic.active() || mimic.state == MS_HUNT || mimic.state == MS_SCRIPT) return;
        Level& l = L();
        int line = MIMIC_POOL[grng.irange(0, MIMIC_POOL_N - 1)];
        // it prefers things it learned from you
        bool mine = false;
        for (int k = 0; k < 3; k++) if (P.reportDone[k] && grng.chance(0.45f)) {
            line = REPORTS[k].line[grng.irange(0, 1)][P.reportPick[k][grng.irange(0, 1)]];
            // pick an actual recorded line
            int a = grng.irange(0, 1);
            line = REPORTS[k].line[a][P.reportPick[k][a]];
            mine = true;
            break;
        }
        (void)mine;
        V3 at = mimic.pos;
        if (l.los(mimic.pos, pos) || dist2d(mimic.pos, pos) < 5.f) {
            if (!l.waypoints.empty()) {
                for (int tries = 0; tries < 6; tries++) {
                    V3 w = l.waypoints[grng.irange(0, (int)l.waypoints.size() - 1)];
                    float d = dist2d(w, pos);
                    if (d > 6.f && d < 18.f && !l.los(w, pos)) { at = w; break; }
                }
            }
        }
        at.y = 1.5f;
        queueLine(line, true, true, at);
        mimic.speakT = 2.5f;
        // a lure: it sometimes lingers where the voice came from
        if (grng.chance(0.35f)) mimic.hear(l, pos, 30.f);
    }

    // ------------------------------------------------------------------
    //  damage / death
    // ------------------------------------------------------------------
    void hurtPlayer(float d) {
        P.health -= d;
        hurt = 1.f; shake = 1.f;
        sfx(S_HIT, 1.f);
        if (P.health <= 0) die();
    }
    void die() {
        mode = M_DEAD; deadT = 0; deaths++;
        ui = UI_NONE; hidden = false;
        audio.stopAll();
        sfx(S_DEATH, 1.f); sfx(S_SCREAM, 0.9f); sfx(S_STING2, 0.8f);
        js = 1.2f; jsType = 0;
    }

    // ------------------------------------------------------------------
    //  interactions
    // ------------------------------------------------------------------
    struct Target { int obj = -1; Door* door = nullptr; float score = 0; };
    Target curTarget;
    std::string prompt;
    float camYaw = 0, camPitch = 0;

    Target findTarget() {
        Target best;
        Level& l = L();
        V3 f(sinf(yaw), 0, cosf(yaw));
        for (size_t i = 0; i < l.objs.size(); i++) {
            const Obj& o = l.objs[i];
            if (o.taken) continue;
            switch (o.type) {
                case O_DESK: case O_CRATE: case O_CORPSE: continue;
                default: break;
            }
            V3 d = o.pos - pos; d.y = 0;
            float dist = vlen(d);
            float reach = 2.1f + (o.type == O_GEN || o.type == O_ELEV || o.type == O_CORE ? 0.7f : 0.f);
            if (dist > reach) continue;
            float dd = dist > 0.01f ? dot(d * (1.f / dist), f) : 1.f;
            float need = dist < 1.0f ? 0.3f : 0.82f;
            if (dd < need) continue;
            float sc = dd * 2.f - dist * 0.3f;
            if (sc > best.score || best.obj < 0) { best.obj = (int)i; best.score = sc; best.door = nullptr; }
        }
        for (float s = 0.6f; s <= 2.0f; s += 0.4f) {
            int cx = (int)floorf(pos.x + f.x * s), cz = (int)floorf(pos.z + f.z * s);
            Door* d = l.doorAtCell(cx, cz);
            if (d) {
                float sc = 3.f - s * 0.2f;
                if (best.obj < 0 || sc > best.score) { best.obj = -1; best.door = d; best.score = sc; }
                break;
            }
        }
        return best;
    }

    std::string promptFor(const Target& tg) {
        Level& l = L();
        if (tg.door) {
            if (tg.door->jammed) return "[E] DOOR JAMMED";
            if (tg.door->kind == DK_POWER && !power()) return "[E] DOOR - NO POWER";
            if (tg.door->kind == DK_SECURITY && !P.secUnlocked) return "[E] SECURITY LOCK";
            return tg.door->target > 0.5f ? "[E] CLOSE DOOR" : "[E] OPEN DOOR";
        }
        if (tg.obj < 0) return "";
        const Obj& o = l.objs[tg.obj];
        switch (o.type) {
            case O_COMMS: return P.commsDone ? "COMMS ARRAY: ONLINE" : "[E] USE COMMS CONSOLE";
            case O_REPORT: return P.reportDone[P.cur] ? "REPORT FILED" : "[E] RECORD NIGHTLY REPORT";
            case O_TAPE: return "[E] TAKE RECORDING";
            case O_FUSE: return "[E] TAKE FUSE";
            case O_BATTERY: return "[E] TAKE BATTERY";
            case O_MEDKIT: return "[E] TAKE MEDKIT";
            case O_AMMO: return "[E] TAKE AMMO";
            case O_PISTOL: return "[E] TAKE PISTOL";
            case O_NOTE: return "[E] READ NOTE";
            case O_STAIRS: return "[E] USE STAIRS";
            case O_LOCKER: return hidden ? "[E] LEAVE LOCKER" : "[E] HIDE";
            case O_BREAKER: return P.tripped ? "[HOLD E] RESET BREAKER" : "BREAKER: NORMAL";
            case O_FUSEBOX: return P.fuseInstalled ? "FUSE BOX: INSTALLED" : "[HOLD E] INSTALL FUSES";
            case O_GEN: return P.genOn ? "GENERATOR: RUNNING" : (P.fuseInstalled ? "[HOLD E] START GENERATOR" : "GENERATOR: NO FUSES");
            case O_SECPC: return P.secUnlocked ? "SECURITY: UNLOCKED" : "[E] USE SECURITY COMPUTER";
            case O_CORE: return P.coreDone ? "CORE TERMINAL: DECIDED" : "[E] USE CORE TERMINAL";
            case O_ELEV:
                if (P.elevReady) return "[E] ENTER ELEVATOR";
                if (P.elevCalled) return "ELEVATOR: ARRIVING";
                return power() ? "[HOLD E] CALL ELEVATOR" : "ELEVATOR: NO POWER";
            default: return "";
        }
    }

    float holdTime(const Obj& o) const {
        switch (o.type) {
            case O_BREAKER: return 3.0f;
            case O_FUSEBOX: return 2.0f;
            case O_GEN: return 3.0f;
            case O_ELEV: return 2.0f;
            default: return 0.f;
        }
    }
    bool isHold(const Obj& o) const {
        switch (o.type) {
            case O_BREAKER: return P.tripped;
            case O_FUSEBOX: return !P.fuseInstalled && fusesHeld() >= 3;
            case O_GEN: return P.fuseInstalled && !P.genOn;
            case O_ELEV: return !P.elevCalled && power() && !P.elevReady;
            default: return false;
        }
    }

    void toggleDoor(Door* d) {
        if (d->jammed) { sfx(S_ERR, 0.6f); msgShow("THE DOOR WON'T MOVE.", 2.f); return; }
        if (d->kind == DK_POWER && !power()) { sfx(S_ERR, 0.6f); msgShow("NO POWER.", 2.f); return; }
        if (d->kind == DK_SECURITY && !P.secUnlocked) { sfx(S_ERR, 0.6f); msgShow("SECURITY LOCK ENGAGED.", 2.f); return; }
        d->target = d->target > 0.5f ? 0.f : 1.f;
        V3 at(d->cx + 0.5f, 1.2f, d->cz + 0.5f);
        sfx(d->kind == DK_MANUAL ? S_CREAK : S_SLIDE, 0.8f, &at, d->kind == DK_MANUAL ? 1.f + grng.range(-0.1f, 0.1f) : 1.f);
        makeNoise(at, d->kind == DK_MANUAL ? 6.f : 4.f);
    }

    void playTape(int k) {
        sfx(S_TAPE, 0.9f);
        P.tape[k] = true;
        for (int i = 0; i < 4; i++) queueLine(TAPES[k].lines[i]);
        msgShow(std::string("RECORDING FOUND - ") + TAPES[k].title, 4.f);
        tapeNoiseT = 0.f;
    }

    void interact(Target tg) {
        Level& l = L();
        if (tg.door) { toggleDoor(tg.door); return; }
        if (tg.obj < 0) return;
        Obj& o = l.objs[tg.obj];
        switch (o.type) {
            case O_COMMS:
                if (!P.commsDone) { ui = UI_COMMS; tuneFreq = 90.f + grng.range(0.f, 3.f); tuneTarget = 93.f + grng.range(0.f, 12.f); tuneHold = 0; if (!aStatic) aStatic = audio.play(S_STATIC, 0.5f, 1.f, nullptr, true); }
                break;
            case O_REPORT:
                if (!P.reportDone[P.cur]) { ui = UI_REPORT; repStage = 0; repT = 0; sfx(S_RADIOON, 0.6f); }
                break;
            case O_TAPE: o.taken = true; playTape(o.id); objectiveChanged(); break;
            case O_FUSE: o.taken = true; P.fuseHeld[o.id] = true; sfx(S_PICKUP); msgShow("PICKED UP: REPLACEMENT FUSE", 3.f); objectiveChanged(); break;
            case O_BATTERY: o.taken = true; P.spare++; sfx(S_PICKUP); msgShow("PICKED UP: BATTERY (R TO SWAP)", 3.f); break;
            case O_MEDKIT: o.taken = true; P.medkits++; sfx(S_PICKUP); msgShow("PICKED UP: MEDKIT (H TO USE)", 3.f); break;
            case O_AMMO: o.taken = true; P.ammo += 6; sfx(S_PICKUP); msgShow("PICKED UP: PISTOL AMMO", 3.f); break;
            case O_PISTOL: o.taken = true; P.hasPistol = true; P.ammo += 6; sfx(S_PICKUP); sfx(S_CLICK); msgShow("PICKED UP: PISTOL (CLICK TO FIRE)", 4.f); break;
            case O_NOTE: ui = UI_NOTE; noteShown = o.id; if (o.id < 5) P.noteRead[o.id] = true; sfx(S_RUSTLE, 0.7f); break;
            case O_STAIRS:
                if (P.cur == 0 && !P.commsDone) { msgShow("CHECK THE COMMS FIRST. THE POWER IS OUT.", 3.f); sfx(S_ERR, 0.5f); }
                else if (P.cur == 2) { msgShow("THE DOOR LOCKED BEHIND YOU.", 3.f); sfx(S_ERR, 0.5f); }
                else { goLevel(o.target); }
                break;
            case O_LOCKER: toggleHide(tg.obj); break;
            case O_SECPC:
                if (P.secUnlocked) break;
                if (!power()) { msgShow("NO POWER.", 2.f); sfx(S_ERR, 0.6f); }
                else { ui = UI_SECPC; codeIn.clear(); uiMsg.clear(); sfx(S_RADIOON, 0.5f); }
                break;
            case O_CORE: if (!P.coreDone) { ui = UI_CORE; sfx(S_RADIOON, 0.5f); } break;
            case O_ELEV: if (P.elevReady) { startCutscene(P.purged ? CS_END_GOOD : CS_END_BAD); } break;
            default: break;
        }
    }

    void completeHold(Obj& o) {
        switch (o.type) {
            case O_BREAKER:
                if (P.tripped) restorePower();
                break;
            case O_FUSEBOX:
                P.fuseInstalled = true; sfx(S_FUSE, 1.f); msgShow("FUSES INSTALLED. NOW START THE GENERATOR.", 4.f);
                objectiveChanged(); makeNoise(pos, 8.f);
                break;
            case O_GEN:
                startCutscene(CS_GEN);
                break;
            case O_ELEV:
                P.elevCalled = true; P.elevTimer = 55.f;
                sfx(S_ELEVHUM, 0.6f); sfx(S_DING, 0.8f);
                aElev = audio.play(S_ELEVHUM, 0.35f, 1.f, nullptr, true);
                mimic.aggression = 1.f; mimic.respawn = 0.f;
                msgShow("ELEVATOR CALLED. HOLD OUT.", 4.f);
                objectiveChanged();
                break;
            default: break;
        }
    }

    void toggleHide(int objIdx) {
        Level& l = L();
        Obj& o = l.objs[objIdx];
        if (!hidden) {
            // anyone watching?
            if (mimic.active() && mimic.sees && dist2d(mimic.pos, pos) < 9.f) mimic.knowsHide = true;
            hidden = true; hideObj = objIdx;
            V3 f(sinf(o.yaw), 0, cosf(o.yaw));
            hideYaw = atan2f(f.x, f.z);
            yaw = hideYaw; pitch = 0;
            pos = o.pos; pos.y = 0;
            sfx(S_LOCKER, 0.7f);
            flashOn = false;
        } else {
            hidden = false;
            V3 f(sinf(o.yaw), 0, cosf(o.yaw));
            pos = o.pos + f * 0.9f; pos.y = 0;
            hideObj = -1;
            sfx(S_LOCKER, 0.6f);
        }
    }

    // ------------------------------------------------------------------
    //  cutscene scripting (defined in game_cutscene.hpp)
    // ------------------------------------------------------------------
    void startCutscene(int id);
    void csAdd(float dur, const std::string& text, V3 c0, float y0, float p0, V3 c1, float y1, float p1, float b0, float b1, int sfxId = -1, int voice = -1, bool mimicV = false, int fx = 0) {
        CutStep s;
        s.dur = dur; s.text = text; s.sfx = sfxId; s.voice = voice; s.mimicVoice = mimicV;
        s.c0 = c0; s.c1 = c1; s.y0 = y0; s.y1 = y1; s.p0 = p0; s.p1 = p1; s.b0 = b0; s.b1 = b1; s.fx = fx;
        cs.push_back(s);
    }
    void updateCutscene(float dt);
    void endCutscene();

    // ------------------------------------------------------------------
    //  terminals
    // ------------------------------------------------------------------
    void closeUi() { ui = UI_NONE; if (aStatic) { audio.stop(aStatic, 0.3f); aStatic = 0; } }

    void updateUi(float dt) {
        uiT += dt;
        if (in->pressed(K_ESC) && ui != UI_REPORT) { closeUi(); return; }
        switch (ui) {
            case UI_NOTE:
                if (in->pressed('E') || in->pressed(K_ENTER) || in->pressed(K_SPACE)) closeUi();
                break;
            case UI_JOURNAL:
                if (in->pressed(K_TAB)) closeUi();
                break;
            case UI_COMMS: {
                tuneKeyT -= dt;
                int dir = 0;
                if (in->down('A') || in->down(K_LEFT)) dir = -1;
                if (in->down('D') || in->down(K_RIGHT)) dir = 1;
                if (dir && tuneKeyT <= 0) { tuneFreq = clampf(tuneFreq + dir * 0.1f, 88.f, 108.f); tuneKeyT = 0.06f; sfx(S_KEY, 0.2f); }
                float err = fabsf(tuneFreq - tuneTarget);
                float strength = 1.f - sat(err / 3.5f);
                if (aStatic) audio.setVol(aStatic, 0.12f + 0.55f * (1.f - strength));
                if (err < 0.16f) tuneHold += dt; else tuneHold = maxf(0.f, tuneHold - dt * 2.f);
                if (tuneHold > 1.6f) {
                    P.commsDone = true;
                    closeUi();
                    sfx(S_RADIOON, 0.7f);
                    queueLine(L_E_HELLO); queueLine(L_E_TUNED); queueLine(L_E_TASK); queueLine(L_E_REPORT);
                    msgShow("COMMS ARRAY ONLINE", 3.f);
                    objectiveChanged();
                }
                break;
            }
            case UI_REPORT: {
                // stage 0,1: choose; 2: recording playback
                if (repStage < 2) {
                    int pick = -1;
                    if (in->pressed('1')) pick = 0;
                    if (in->pressed('2')) pick = 1;
                    if (in->pressed(K_ESC)) { closeUi(); break; }
                    if (pick >= 0) {
                        P.reportPick[P.cur][repStage] = pick;
                        sfx(S_KEY, 0.5f);
                        repStage++;
                        if (repStage == 2) {
                            repIdx = 0;
                            sfx(S_TAPE, 0.8f);
                            float need = maxf(vqT, 0.f) + 1.0f;
                            for (int a = 1; a >= 0; a--) {
                                int id = REPORTS[P.cur].line[a][P.reportPick[P.cur][a]];
                                vq.insert(vq.begin(), QLine{id, false, false, V3()});
                                need += audio.duration(lineSnd[id]) + 0.35f;
                            }
                            repT = 7.5f - need;
                            makeNoise(pos, 9.f);
                        }
                    }
                } else {
                    repT += dt;
                    if (repT > 7.5f) {
                        P.reportDone[P.cur] = true;
                        closeUi();
                        msgShow("REPORT FILED. IT IS LISTENING.", 3.5f);
                        objectiveChanged();
                    }
                }
                break;
            }
            case UI_SECPC: {
                for (int k = 0; k < 10; k++) if (in->pressed('0' + k) && codeIn.size() < 6) { codeIn += (char)('0' + k); sfx(S_KEY, 0.4f); }
                if (in->pressed(K_BACK) && !codeIn.empty()) { codeIn.pop_back(); sfx(S_KEY, 0.3f); }
                if (in->pressed(K_ENTER)) {
                    if (codeIn == SECURITY_CODE) {
                        P.secUnlocked = true;
                        closeUi();
                        startCutscene(CS_SEC);
                    } else {
                        uiMsg = "ACCESS DENIED"; codeIn.clear(); sfx(S_ERR, 0.8f);
                        makeNoise(pos, 14.f);
                    }
                }
                break;
            }
            case UI_CORE: {
                if (in->pressed('1')) { closeUi(); startCutscene(CS_PURGE); }
                if (in->pressed('2')) {
                    closeUi();
                    P.coreDone = true; P.purged = false;
                    unjamB3();
                    queueLine(L_C_3, true, true, V3(15.5f, 1.5f, 12.5f));
                    msgShow("YOU LEFT IT ALONE. THE LOCKS RELEASED.", 4.f);
                    objectiveChanged();
                }
                break;
            }
        }
    }

    void unjamB3() {
        for (auto& d : lv[2].doors) d.jammed = false;
    }

    // ------------------------------------------------------------------
    //  weapon
    // ------------------------------------------------------------------
    void fire() {
        if (!P.hasPistol || gunCd > 0 || hidden) return;
        if (P.ammo <= 0) { sfx(S_CLICK, 0.8f); gunCd = 0.4f; msgShow("OUT OF AMMO", 1.5f); return; }
        P.ammo--;
        gunCd = 0.55f; muzzle = 0.09f; recoil = 0.08f;
        sfx(S_SHOT, 1.f);
        makeNoise(pos, 22.f);
        if (mimic.active() && mimic.state != MS_SCRIPT) {
            V3 f(sinf(yaw) * cosf(pitch), sinf(pitch), cosf(yaw) * cosf(pitch));
            V3 to = V3(mimic.pos.x, 1.3f, mimic.pos.z) - V3(pos.x, 1.5f, pos.z);
            float along = dot(to, f);
            if (along > 0.3f && along < 16.f) {
                V3 perp = to - f * along;
                if (vlen(perp) < 0.55f && L().los(pos, mimic.pos)) {
                    mimic.stagger();
                    sfx(S_SCREAM, 0.7f, &mimic.pos, 0.8f);
                    if (mimic.hits >= 3) { mimic.despawn(P.purged ? 14.f : 28.f); msgShow("IT FLED INTO THE DARK.", 3.f); }
                }
            }
        }
    }

    // ------------------------------------------------------------------
    //  per-frame update
    // ------------------------------------------------------------------
    void update(float dt, Input& input);
    void updatePlay(float dt);
    void updateAudio(float dt);
    void updateTitle(float dt);
    void updateEntity(float dt);
    void updateObjects(float dt);

    // rendering
    void render();
    void renderWorld();
    void renderHud();
    void renderUi();
    void drawObj(const Obj& o, const LightFn& lf);
    void drawDoor(const Door& d, const LightFn& lf);
    void drawFaceOverlay(float k);
    void renderTitle();
    void renderCut();
    void renderDead();
    void renderEnd();
    void drawNoteBox(const char* text, int w, int h, const char* title);
};

}  // namespace S9
