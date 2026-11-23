// STATION 09: ECHO  --  per-frame update
#pragma once
#include "game.hpp"

namespace S9 {

inline void Game::update(float dt, Input& input) {
    in = &input;
    dt = minf(dt, 0.05f);
    t += dt;
    // shared effect decay
    hurt = maxf(0.f, hurt - dt * 1.2f);
    flashFx = maxf(0.f, flashFx - dt * 1.8f);
    shake = maxf(0.f, shake - dt * 1.4f);
    js = maxf(0.f, js - dt);
    muzzle = maxf(0.f, muzzle - dt);
    recoil = maxf(0.f, recoil - dt * 0.4f);
    gunCd = maxf(0.f, gunCd - dt);
    msgT = maxf(0.f, msgT - dt);
    for (auto& s : subs) s.t += dt;
    subs.erase(std::remove_if(subs.begin(), subs.end(), [](const Sub& s) { return s.t > s.dur; }), subs.end());
    if (lightsOut > 0.f) lightsOut = maxf(0.f, lightsOut - dt);
    if (flickerBoost > 0.f) flickerBoost = maxf(0.f, flickerBoost - dt);

    switch (mode) {
        case M_TITLE: updateTitle(dt); break;
        case M_PLAY: updatePlay(dt); break;
        case M_CUT:
            updateCutscene(dt);
            updateVoiceQueue(dt);
            break;
        case M_PAUSE:
            if (in->pressed(K_ESC)) mode = M_PLAY;
            if (in->pressed('Q')) { mode = M_TITLE; audio.stopAll(); }
            break;
        case M_DEAD:
            deadT += dt;
            if (deadT > 3.5f && (in->pressed(K_ENTER) || in->pressed(K_SPACE))) {
                if (cpt.valid) { loadCheckpoint(); mode = M_PLAY; }
                else newGame();
            }
            break;
        case M_END:
            endT += dt;
            if (endT > 6.f && in->pressed(K_ENTER)) { mode = M_TITLE; audio.stopAll(); }
            break;
    }
    in->endFrame();
}

inline void Game::updateTitle(float dt) {
    (void)dt;
    if (in->pressed(K_ENTER) || in->pressed(K_SPACE)) newGame();
    if (in->pressed(K_ESC)) quit = true;
}

inline void Game::updatePlay(float dt) {
    // ---- level transition fade ----
    if (transTo >= 0) {
        transPrev = transT;
        transT += dt * 1.6f;
        if (transPrev < 0.5f && transT >= 0.5f) { int to = transTo; int from = P.cur; arrive(to, from); }
        if (transT >= 1.f) { transTo = -1; transT = 0; }
        updateAudio(dt);
        return;
    }
    if (in->pressed(K_ESC) && ui == UI_NONE) { mode = M_PAUSE; return; }
    playTime += dt;
    levelT += dt;
    Level& l = L();

    // ---- camera basis for this frame (used by the input code below and by rendering) ----
    // ---- UI ----
    if (ui != UI_NONE) updateUi(dt);

    // ---- player ----
    if (ui == UI_NONE) {
        float sens = 0.0023f;
        yaw += in->mdx * sens;
        pitch -= in->mdy * sens;
        yaw += ((in->down(K_RIGHT) ? 1.f : 0.f) - (in->down(K_LEFT) ? 1.f : 0.f)) * 1.9f * dt;
        pitch += ((in->down(K_UP) ? 1.f : 0.f) - (in->down(K_DOWN) ? 1.f : 0.f)) * 1.3f * dt;
        pitch = clampf(pitch, -1.2f, 1.2f);
        if (yaw > PI) yaw -= 2 * PI;
        if (yaw < -PI) yaw += 2 * PI;

        if (hidden) {
            yaw = hideYaw + clampf(angDiff(hideYaw, yaw), -0.8f, 0.8f);
            pitch = clampf(pitch, -0.45f, 0.45f);
            sprinting = false;
        } else {
            V3 f(sinf(yaw), 0, cosf(yaw)), r(cosf(yaw), 0, -sinf(yaw));
            V3 wish;
            if (in->down('W')) wish = wish + f;
            if (in->down('S')) wish = wish - f;
            if (in->down('D')) wish = wish + r;
            if (in->down('A')) wish = wish - r;
            bool moving = vlen(wish) > 0.01f;
            bobAmp += ((moving ? 1.f : 0.f) - bobAmp) * minf(1.f, dt * 8.f);
            if (moving) wish = vnorm(wish);
            sprinting = moving && in->down(K_SHIFT) && stamina > 3.f && in->down('W');
            float spd = sprinting ? 4.0f : 2.3f;
            if (sprinting) { stamina -= 22.f * dt; staminaDelay = 1.2f; }
            else { staminaDelay -= dt; if (staminaDelay <= 0) stamina = minf(100.f, stamina + 16.f * dt); }
            if (stamina < 3.f) sprinting = false;
            if (moving) {
                l.move(pos, wish.x * spd * dt, wish.z * spd * dt, 0.28f);
                bob += dt * spd * 3.2f;
                stepAcc += spd * dt;
                float stride = sprinting ? 1.7f : 2.1f;
                if (stepAcc > stride) {
                    stepAcc = 0;
                    char fc = l.ch((int)pos.x, (int)pos.z);
                    if (fc == 'g') sfx(S_GRATE0 + grng.irange(0, 1), sprinting ? 0.8f : 0.5f);
                    else sfx(S_FOOT0 + grng.irange(0, 3), sprinting ? 0.8f : 0.45f);
                    makeNoise(pos, sprinting ? 9.f : 4.f);
                }
            }
        }

        // flashlight, battery, heal
        if (in->pressed('F') && !hidden) {
            if (P.battery > 0.f) { flashOn = !flashOn; sfx(S_CLICK, 0.7f); }
            else { msgShow("BATTERY DEAD - PRESS R IF YOU HAVE A SPARE", 2.5f); sfx(S_CLICK, 0.4f); }
        }
        if (in->pressed('R') && P.spare > 0 && P.battery < 90.f) {
            P.spare--; P.battery = 100.f; sfx(S_CLICK, 0.8f); sfx(S_TAPE, 0.3f);
            msgShow("BATTERY REPLACED", 2.f);
        }
        if (in->pressed('H') && P.medkits > 0 && P.health < 100.f) {
            P.medkits--; P.health = minf(100.f, P.health + 50.f); sfx(S_RUSTLE, 0.8f); msgShow("HEALED", 1.5f);
        }
        if (in->pressed(K_TAB)) { ui = UI_JOURNAL; uiT = 0; }
        if (in->mpressed(0) && !hidden) fire();

        // targets & interaction
        curTarget = findTarget();
        prompt = promptFor(curTarget);
        bool holdable = false;
        if (curTarget.obj >= 0) {
            Obj& o = l.objs[curTarget.obj];
            if (hidden && o.type != O_LOCKER) { prompt.clear(); curTarget = Target(); }
            holdable = !curTarget.door && isHold(o);
        }
        if (hidden && hideObj >= 0) {  // only way out is the locker itself
            prompt = "[E] LEAVE LOCKER";
            curTarget = Target(); curTarget.obj = hideObj;
        }
        if (holdable) {
            Obj& o = l.objs[curTarget.obj];
            if (in->down('E')) {
                if (holdTarget != curTarget.obj) { holdTarget = curTarget.obj; holdT = 0; }
                holdT += dt;
                if (fmodf(holdT, 1.0f) < dt) { makeNoise(o.pos, 7.f); if (o.type == O_BREAKER) sfx(S_CLICK, 0.5f); }
                if (holdT >= holdTime(o)) { holdT = 0; holdTarget = -1; completeHold(o); }
            } else { holdT = 0; holdTarget = -1; }
        } else {
            holdT = 0; holdTarget = -1;
            if (in->pressed('E')) {
                if (curTarget.obj >= 0) {
                    Obj& o = l.objs[curTarget.obj];
                    if (o.type == O_FUSEBOX && !P.fuseInstalled && fusesHeld() < 3) {
                        char b[64]; snprintf(b, sizeof(b), "THE PANEL NEEDS 3 FUSES (%d/3).", fusesHeld()); msgShow(b, 3.f); sfx(S_ERR, 0.5f);
                    } else if (o.type == O_GEN && !P.fuseInstalled && !P.genOn) { msgShow("THE GENERATOR HAS NO FUSES.", 3.f); sfx(S_ERR, 0.5f); }
                    else if (o.type == O_ELEV && !power() && !P.elevReady) { msgShow("NO POWER TO THE ELEVATOR.", 3.f); sfx(S_ERR, 0.5f); }
                    else interact(curTarget);
                } else if (curTarget.door) interact(curTarget);
            }
        }
    } else {
        holdT = 0; holdTarget = -1; curTarget = Target(); prompt.clear();
    }

    // battery drain
    if (flashOn) {
        P.battery -= 0.45f * dt;
        if (P.battery <= 0.f) { P.battery = 0.f; flashOn = false; msgShow("THE FLASHLIGHT DIED.", 3.f); sfx(S_CLICK, 0.6f); }
    }

    // ---- triggers ----
    if (!hidden) {
        for (auto& tr : l.trigs) {
            if (tr.done) continue;
            if ((int)pos.x == tr.cx && (int)pos.z == tr.cz) { if (doTrigger(tr.id)) tr.done = true; if (mode != M_PLAY) return; }
        }
    }

    updateObjects(dt);
    updateEntity(dt);

    // ---- elevator countdown ----
    if (P.elevCalled && !P.elevReady) {
        P.elevTimer -= dt;
        mimic.aggression = 1.f;
        if (mimic.state == MS_INACTIVE) mimic.respawn = minf(mimic.respawn, 6.f);
        if (P.elevTimer <= 0.f) {
            P.elevReady = true;
            sfx(S_DING, 1.f); sfx(S_SLIDE, 0.8f);
            if (aElev) { audio.stop(aElev, 0.5f); aElev = 0; }
            for (auto& o : l.objs) if (o.type == O_ELEV) o.state = 1;
            for (auto& d : l.doors) (void)d;
            msgShow("THE ELEVATOR HAS ARRIVED. GET IN.", 4.f);
            objectiveChanged();
        }
    }

    // ---- mimic lures + ambience ----
    if (P.cur >= 1 || P.sighted) {
        mimicVoiceT -= dt;
        if (mimicVoiceT <= 0.f) {
            mimicSpeak();
            mimicVoiceT = grng.range(16.f, 34.f) * (1.f - 0.5f * mimic.aggression);
        }
    }
    ambT -= dt;
    if (ambT <= 0.f) {
        ambT = grng.range(10.f, 26.f);
        V3 at = pos + V3(grng.range(-9.f, 9.f), 1.f, grng.range(-9.f, 9.f));
        int k = grng.irange(0, 5);
        const int amb[6] = {S_KNOCK, S_DRIP, S_GROAN, S_CREAK, S_WHISPER, S_RUSTLE};
        if (!l.wallAt((int)at.x, (int)at.z)) sfx(amb[k], k == 4 ? 0.6f : 0.8f, &at);
        if (k == 2) flickerBoost = 1.2f;
    }

    // ---- fear ----
    float target = 0.f;
    if (mimic.active() && mimic.state != MS_SCRIPT) {
        float d = dist2d(mimic.pos, pos);
        if (d < 13.f) target = sat(1.f - d / 13.f) * (mimic.sees ? 1.f : 0.65f);
        if (mimic.state == MS_HUNT) target = maxf(target, 0.9f);
    }
    fear += (target - fear) * minf(1.f, dt * 1.8f);

    updateAudio(dt);
    updateVoiceQueue(dt);
}

inline void Game::updateObjects(float dt) {
    Level& l = L();
    for (auto& d : l.doors) {
        float spd = d.kind == DK_MANUAL ? 1.6f : 2.4f;
        if (d.open < d.target) d.open = minf(d.target, d.open + spd * dt);
        else if (d.open > d.target) d.open = maxf(d.target, d.open - spd * dt);
        // the security door opens by itself once the code has been accepted and you walk up
        if (d.kind == DK_SECURITY && P.secUnlocked && d.target < 0.5f) {
            V3 c(d.cx + 0.5f, 0, d.cz + 0.5f);
            if (dist2d(c, pos) < 2.2f) { d.target = 1.f; V3 at = c; at.y = 1.2f; sfx(S_SLIDE, 0.9f, &at); }
        }
    }
    for (auto& o : l.objs) {
        if (o.type == O_CORPSE && o.state == 1) {
            o.t += dt;
            if (o.t > 0.45f) o.state = 2;
        }
        if (o.type == O_ELEV && o.state == 1) o.t = minf(1.f, o.t + dt * 0.8f);
    }
    if (corpseT >= 0.f) {
        corpseT += dt;
        if (corpseT > 0.4f) { corpseT = -1.f; jumpscare(2); flickerBoost = 1.5f; }
    }
}

inline void Game::updateEntity(float dt) {
    Level& l = L();
    // dormant on B1 until the first sighting
    bool dormant = (P.cur == 0 && !P.sighted);
    if (!dormant && mimic.state == MS_INACTIVE && mimic.respawn <= 0.f && mode == M_PLAY) {
        if (mimic.spawnAway(l, pos, 13.f)) {
            mimic.aggression = maxf(mimic.aggression, 0.3f);
            if (grng.chance(0.6f)) sfx(S_GROAN, 0.35f, &mimic.pos);
        } else mimic.respawn = 5.f;
    }
    if (mimic.state == MS_SCRIPT) {
        mimic.st += dt;
        V3 d = pos - mimic.pos;
        mimic.yaw += angDiff(mimic.yaw, atan2f(d.x, d.z)) * minf(1.f, dt * 3.f);
        if (mimic.st > 3.4f) { mimic.despawn(P.cur == 0 ? 45.f : 25.f); flickerBoost = 1.f; lightsOut = 0.6f; sfx(S_THUMP, 0.8f); }
        return;
    }
    if (!dormant) mimic.aggression = maxf(mimic.aggression, sat(levelT / 360.f) * 0.8f);

    PlayerInfo pi;
    pi.pos = pos; pi.hidden = hidden; pi.flash = flashOn && P.battery > 0.f; pi.sprint = sprinting; pi.dead = false;
    if (hidden && hideObj >= 0) pi.hideSpot = l.objs[hideObj].pos;
    MimicEvents ev;
    mimic.update(l, dt, pi, ev);

    if (ev.step) { V3 at = mimic.pos; at.y = 0.2f; sfx(S_EFOOT0 + grng.irange(0, 1), ev.stepVol, &at); }
    if (ev.doorOpened) { sfx(S_CREAK, 0.9f, &ev.doorPos, 0.9f); }
    if (ev.spotted) {
        V3 at = mimic.pos; at.y = 1.5f;
        sfx(S_STING1, 0.9f); sfx(S_SCREAM, 0.8f, &at);
        shake = 0.6f; flashFx = 0.3f;
    }
    if (ev.attack) {
        hurtPlayer(45.f);
        if (mode != M_PLAY) return;
        V3 away = vnorm(V3(pos.x - mimic.pos.x, 0, pos.z - mimic.pos.z));
        l.move(pos, away.x * 0.9f, away.z * 0.9f, 0.28f);
    }
    if (ev.pulledLocker) {
        hidden = false; hideObj = -1;
        sfx(S_LOCKER, 1.f); jumpscare(0);
        hurtPlayer(50.f);
        if (mode != M_PLAY) return;
        mimic.knowsHide = false;
    }
    // hearing: the thing comes to look when you take noisy actions (handled in makeNoise)
    (void)dt;
}

inline void Game::updateAudio(float dt) {
    camPos = pos + V3(0, 1.55f + (sprinting ? 0.05f : 0.f), 0);
    audio.setListener(pos, yaw);
    // drone gets louder with fear
    audio.setVol(aDrone, 0.38f + 0.35f * fear);
    audio.setVol(aWind, 0.2f + 0.1f * sinf(t * 0.3f));
    // heartbeat
    heartT -= dt;
    if (fear > 0.12f && heartT <= 0.f) {
        heartT = lerpf(1.2f, 0.42f, fear);
        sfx(S_HEART, 0.35f + 0.65f * fear);
    }
    // hum when the generator is running
    if (power() && !aHum) aHum = audio.play(S_HUM, 0.22f, 1.f, nullptr, true);
    if (!power() && aHum) { audio.stop(aHum, 0.4f); aHum = 0; }
    // radio static creeps in when the thing is near
    if (ui != UI_COMMS) {
        float want = 0.f;
        if (mimic.active() && mimic.state != MS_SCRIPT) { float d = dist2d(mimic.pos, pos); if (d < 10.f) want = (1.f - d / 10.f) * 0.45f; }
        if (want > 0.02f) {
            if (!aStatic) aStatic = audio.play(S_STATIC, 0.f, 1.f, nullptr, true);
            audio.setVol(aStatic, want);
        } else if (aStatic) { audio.stop(aStatic, 0.5f); aStatic = 0; }
    }
    if (P.purged && P.cur == 2 && mode == M_PLAY) {
        if (!aAlarm) aAlarm = audio.play(S_ALARM, 0.22f, 1.f, nullptr, true);
    } else if (aAlarm) { audio.stop(aAlarm, 0.5f); aAlarm = 0; }
}

}  // namespace S9
