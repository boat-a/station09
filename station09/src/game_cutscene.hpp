// STATION 09: ECHO  --  cutscenes and endings
#pragma once
#include "game_update.hpp"

namespace S9 {

static inline float yawToward(V3 from, V3 to) { return atan2f(to.x - from.x, to.z - from.z); }

inline void Game::startCutscene(int id) {
    mode = M_CUT; csId = id; cs.clear(); csI = 0; csT = 0; csStarted = false;
    ui = UI_NONE; hidden = false; hideObj = -1; holdT = 0;
    vq.clear(); vqT = 0;
    V3 z;
    const float B = 1.f;  // fully black
    switch (id) {
        case CS_INTRO: {
            csAdd(4.5f, "STATION 09\nDEEP RESEARCH FACILITY", z, 0, 0, z, 0, 0, B, B, S_GROAN);
            csAdd(5.0f, "MAINTENANCE LEVEL B1 - 02:00 HOURS\nTECHNICIAN R. MERCER - NIGHT SHIFT", z, 0, 0, z, 0, 0, B, B);
            csAdd(7.0f, "TONIGHT'S DUTIES:\n\nCHECK THE COMMUNICATION SYSTEMS\nREPAIR FAULTY ELECTRICAL EQUIPMENT\nRECORD THE NIGHTLY MAINTENANCE REPORT", z, 0, 0, z, 0, 0, B, B);
            V3 a(2.0f, 1.45f, 5.4f), b(2.6f, 1.45f, 5.2f), tgt(6.0f, 1.2f, 2.5f);
            csAdd(5.0f, "THE POWER HAS BEEN OUT FOR AN HOUR.\nTHE RADIO HAS BEEN SILENT FOR LONGER.", a, yawToward(a, tgt), -0.05f, b, yawToward(b, tgt), -0.02f, B, 0.f);
            csAdd(3.5f, "SOMETHING IS WRONG WITH THE QUIET.", b, yawToward(b, V3(7.f, 0, 1.5f)), -0.02f, V3(3.2f, 1.45f, 5.0f), yawToward(V3(3.2f, 0, 5.f), V3(8.f, 0, 3.5f)), 0.0f, 0.f, 0.f, S_DRIP, -1, false, 1);
            break;
        }
        case CS_GEN: {
            V3 a(6.5f, 1.5f, 5.9f), b(6.5f, 1.5f, 4.6f);
            csAdd(5.8f, "PRIMING THE ROTOR...", a, PI, -0.04f, b, PI, -0.02f, 0.f, 0.f, S_GENSTART, -1, false, 2);
            csAdd(3.2f, "MAIN POWER RESTORED", b, PI, -0.02f, V3(6.5f, 1.5f, 4.2f), PI, 0.0f, 0.f, 0.f, S_POWERON, -1, false, 9);
            csAdd(3.4f, "BEHIND YOU, SOMETHING EXHALES.", V3(6.5f, 1.5f, 4.2f), PI, 0.f, V3(6.5f, 1.5f, 4.2f), 0.f, 0.05f, 0.f, 0.f, S_BREATH, -1, false, 1);
            break;
        }
        case CS_SEC: {
            V3 a(29.5f, 1.5f, 3.9f), b(29.5f, 1.5f, 2.7f);
            csAdd(3.0f, "ACCESS GRANTED\nRESTRICTED LEVELS UNLOCKED", a, PI, 0.0f, b, PI, 0.02f, 0.f, 0.f, S_OK);
            csAdd(3.0f, "EMERGENCY ELEVATOR: LEVEL B3", b, PI, 0.02f, b, PI, 0.0f, 0.f, 0.f, -1, -1, false, 0);
            csAdd(2.0f, "", b, PI, 0.f, b, PI, 0.f, 0.f, 0.f, -1, L_E_DOWN, false, 1);
            csAdd(3.2f, "THE RADIO IS SMILING.\nYOU CAN HEAR IT.", b, PI, 0.f, V3(29.5f, 1.5f, 3.4f), PI + 0.4f, 0.f, 0.f, 0.f, S_WHISPER, -1, false, 1);
            break;
        }
        case CS_B3: {
            V3 a(14.5f, 1.55f, 2.2f), b(14.5f, 1.55f, 3.2f);
            csAdd(4.0f, "LEVEL B3\nRESTRICTED ACCESS - ECHO CHAMBER", a, 0.f, 0.0f, b, 0.f, 0.0f, B, 0.f, S_SLAM);
            csAdd(2.0f, "", b, 0.f, 0.f, V3(14.5f, 1.55f, 3.6f), 0.f, 0.f, 0.f, 0.f, -1, L_E_LOBBY, false, 1);
            break;
        }
        case CS_CHAMBER: {
            mimic.spawnAt(V3(15.5f, 0, 11.4f), MS_SCRIPT);
            mimic.yaw = PI;
            V3 a(14.5f, 1.6f, 9.3f), b(14.9f, 1.6f, 10.0f);
            csAdd(1.5f, "", a, 0.f, 0.f, a, 0.1f, 0.f, 0.f, 0.f, S_STING1, -1, false, 1);
            csAdd(2.0f, "", a, 0.1f, 0.f, b, 0.2f, 0.f, 0.f, 0.f, -1, L_C_1, true, 1);
            csAdd(2.0f, "", b, 0.2f, 0.f, b, 0.1f, 0.f, 0.f, 0.f, -1, L_C_2, true, 2);
            csAdd(1.6f, "", b, 0.1f, 0.f, b, 0.f, 0.f, 0.f, 0.f, S_SCREAM, -1, false, 2);
            break;
        }
        case CS_PURGE: {
            V3 a(15.5f, 1.6f, 11.4f), b(15.5f, 1.6f, 12.0f);
            csAdd(2.8f, "ARCHIVE PURGE INITIATED\nSONIC LATTICE: OVERLOAD", a, 0.f, 0.f, b, 0.f, 0.f, 0.f, 0.f, S_BREAKER, -1, false, 3);
            csAdd(3.2f, "ERASING 1,214 VOICES...", b, 0.f, 0.f, b, 0.15f, 0.f, 0.f, 0.f, S_GROAN, -1, false, 3);
            csAdd(2.6f, "", b, 0.15f, 0.f, b, -0.15f, 0.f, 0.f, 0.6f, S_SCREAM, -1, false, 2);
            break;
        }
        case CS_END_GOOD: {
            csAdd(3.0f, "THE DOORS CLOSE.", z, 0, 0, z, 0, 0, B, B, S_DING);
            csAdd(4.5f, "THE ELEVATOR CLIMBS.\nBEHIND YOU, THE STATION FALLS SILENT.", z, 0, 0, z, 0, 0, B, B, S_ELEVHUM);
            csAdd(4.0f, "NO VOICES. NO RADIO.\nONLY THE HUM OF THE CABLE.", z, 0, 0, z, 0, 0, B, B);
            csAdd(4.5f, "THE DOORS OPEN ON COLD AIR.\nDAWN. SNOW. A SKY WITHOUT A CEILING.", z, 0, 0, z, 0, 0, B, B, S_WIND);
            csAdd(2.5f, "IN YOUR POCKET, THE RECORDER\nCLICKS ON BY ITSELF.", z, 0, 0, z, 0, 0, B, B, S_CLICK);
            csAdd(2.0f, "", z, 0, 0, z, 0, 0, B, B, -1, L_END_HISS, true);
            csAdd(4.0f, "...", z, 0, 0, z, 0, 0, B, B, S_STING1);
            break;
        }
        case CS_END_BAD: {
            csAdd(3.0f, "THE DOORS CLOSE.", z, 0, 0, z, 0, 0, B, B, S_DING);
            csAdd(3.5f, "THE ELEVATOR CLIMBS.\nTHE VOICES FOLLOW.", z, 0, 0, z, 0, 0, B, B, S_ELEVHUM);
            csAdd(2.0f, "", z, 0, 0, z, 0, 0, B, B, -1, L_END_ELLIS, true);
            csAdd(3.5f, "YOU HEAR YOUR OWN VOICE.\nTHEN ANOTHER. THEN ANOTHER.", z, 0, 0, z, 0, 0, B, B, S_WHISPER);
            csAdd(2.0f, "", z, 0, 0, z, 0, 0, B, B, -1, L_END_MERCER, true);
            csAdd(4.5f, "THE SURFACE ANSWERS.\nTHEY ARE SENDING SOMEONE DOWN.", z, 0, 0, z, 0, 0, B, B, S_STING1);
            break;
        }
    }
}

inline void Game::updateCutscene(float dt) {
    if (cs.empty()) { endCutscene(); return; }
    if (csI >= cs.size()) { endCutscene(); return; }
    CutStep& s = cs[csI];
    if (!csStarted) {
        csStarted = true; csT = 0;
        if (s.sfx >= 0) audio.play(s.sfx, 0.9f);
        if (s.voice >= 0) {
            int snd = s.mimicVoice ? lineSndMimic[s.voice] : lineSnd[s.voice];
            audio.play(snd, 1.f);
            const LineDef& ld = LD[s.voice];
            float d = audio.duration(snd);
            pushSub(std::string(SPEAKER_NAME[ld.sp]) + ": " + ld.text, SPEAKER_COL[ld.sp], d + 0.8f);
            s.dur = maxf(s.dur, d + 0.7f);
        }
        if (s.fx == 9) {
            P.genOn = true; P.tripped = false; flickerBoost = 3.f;
            if (!aHum) aHum = audio.play(S_HUM, 0.22f, 1.f, nullptr, true);
        }
    }
    csT += dt;
    if (s.fx == 1) flickerBoost = 0.3f;
    if (s.fx == 2) shake = maxf(shake, 0.35f);
    if (csId != CS_END_GOOD && csId != CS_END_BAD && csId != CS_INTRO && (in->pressed(K_SPACE) || in->pressed(K_ENTER)) && csT > 0.4f) {
        // skipping: make sure side effects of every remaining step still happen
        for (size_t i = csI; i < cs.size(); i++) if (cs[i].fx == 9) { P.genOn = true; P.tripped = false; }
        endCutscene();
        return;
    }
    if (csId == CS_INTRO && (in->pressed(K_SPACE) || in->pressed(K_ENTER)) && csT > 0.4f) { endCutscene(); return; }
    if (csT >= s.dur) { csI++; csStarted = false; csT = 0; if (csI >= cs.size()) endCutscene(); }
}

inline void Game::endCutscene() {
    int id = csId;
    cs.clear(); csI = 0;
    switch (id) {
        case CS_INTRO:
            mode = M_PLAY;
            msgShow("FIND THE COMMS ROOM.  F: FLASHLIGHT   TAB: JOURNAL", 6.f);
            saveCheckpoint();
            break;
        case CS_GEN:
            mode = M_PLAY;
            P.genOn = true; P.tripped = false;
            if (!aHum) aHum = audio.play(S_HUM, 0.22f, 1.f, nullptr, true);
            queueLine(L_E_POWER);
            mimic.aggression = minf(1.f, mimic.aggression + 0.2f);
            mimicVoiceT = 12.f;
            objectiveChanged();
            break;
        case CS_SEC:
            mode = M_PLAY;
            msgShow("THE RESTRICTED STAIRWELL IS OPEN (EAST END OF B2).", 5.f);
            mimic.aggression = minf(1.f, mimic.aggression + 0.2f);
            objectiveChanged();
            break;
        case CS_B3:
            mode = M_PLAY;
            msgShow("FIND THE ECHO CHAMBER.", 4.f);
            mimic.despawn(35.f);
            break;
        case CS_CHAMBER:
            mode = M_PLAY;
            mimic.despawn(28.f);
            flickerBoost = 2.f;
            msgShow("THE CORE TERMINAL IS AT THE CENTRE OF THE CHAMBER.", 5.f);
            break;
        case CS_PURGE:
            mode = M_PLAY;
            P.coreDone = true; P.purged = true;
            unjamB3();
            mimic.enraged = true; mimic.aggression = 1.f; mimic.despawn(4.f);
            msgShow("THE ARCHIVE IS GONE. THE THING IS ENRAGED. GET TO THE ELEVATOR.", 6.f);
            objectiveChanged();
            break;
        case CS_END_GOOD: mode = M_END; endT = 0; endingKind = 1; audio.stopAll(); break;
        case CS_END_BAD: mode = M_END; endT = 0; endingKind = 2; audio.stopAll(); break;
    }
}

}  // namespace S9
