// STATION 09: ECHO  --  story text. Every spoken line is synthesized at start-up.
#pragma once
#include "audio.hpp"

namespace S9 {

enum Speaker { SP_ELLIS, SP_SAM, SP_OKAFOR, SP_PARK, SP_DANA, SP_MERCER, SP_ENTITY, SP_COUNT };
static const char* SPEAKER_NAME[SP_COUNT] = {"ELLIS", "SAM", "DR OKAFOR", "CHIEF PARK", "DANA", "MERCER", "???"};
static const u32 SPEAKER_COL[SP_COUNT] = {0xFF9FD0FFu, 0xFFE0C080u, 0xFFD0A0E0u, 0xFFA0E0A0u, 0xFFFFB0C8u, 0xFFFFFFFFu, 0xFFFF4040u};

struct LineDef { int sp; const char* text; };

// id, speaker, text
#define LINES(X) \
    X(E_HELLO, SP_ELLIS, "Station zero nine, this is Surface Control. Do you read? Over.") \
    X(E_TUNED, SP_ELLIS, "There you are, Mercer. We lost your telemetry an hour ago. Stay calm. We need main power restored.") \
    X(E_TASK, SP_ELLIS, "Find three replacement fuses on the maintenance floors. Then restart the backup generator on level B two.") \
    X(E_REPORT, SP_ELLIS, "And do not forget your nightly report. Procedure is procedure. I like to hear you speak.") \
    X(E_STAIRS, SP_ELLIS, "Good. Keep going. I will be right here. Always right here.") \
    X(E_POWER, SP_ELLIS, "The lights are on. Wonderful. Now find the security codes. There are recordings. Listen carefully.") \
    X(E_TRIP, SP_ELLIS, "Mercer, the power is failing. Reset the breaker in the closet. Quickly. Before it finds you.") \
    X(E_LIE, SP_ELLIS, "Do not listen to the tapes, Mercer. They are old. They are lies. Come down to me.") \
    X(E_DOWN, SP_ELLIS, "Access granted. Take the stairs to B three. The elevator is waiting. I will meet you there.") \
    X(E_LOBBY, SP_ELLIS, "The elevator will need power. It is shy of you. Be patient. Be quiet. Be mine.") \
    X(T1A, SP_SAM, "Personal log. Sam Whitaker. Night forty one. The intercom is speaking to me again.") \
    X(T1B, SP_SAM, "It uses my wife's voice. Dana has been dead three years, but it knows exactly how she laughed.") \
    X(T1C, SP_SAM, "I wrote the override code on my hand so I would not forget. The first two digits are four, seven.") \
    X(T1D, SP_SAM, "If you hear someone calling your name down here, do not answer. Do not answer.") \
    X(T2A, SP_OKAFOR, "Doctor Okafor. Project Echo. Final entry. The organism does not speak. It remembers sound, and it plays it back.") \
    X(T2B, SP_OKAFOR, "Every time we answered, it learned. Our voices. Our fear. What we most wanted to hear.") \
    X(T2C, SP_OKAFOR, "The elevator is voice locked. It needs a living staff voice to open. That is why it is so patient.") \
    X(T2D, SP_OKAFOR, "The code continues. One, three. Whoever finds this, destroy the voice archive. Do not let it reach the surface.") \
    X(T3A, SP_PARK, "Security chief Park. Surface has not answered us for two years.") \
    X(T3B, SP_PARK, "If the radio talks to you, that is not Surface. That is it. Ellis died on the first night.") \
    X(T3C, SP_PARK, "It cuts the breakers when we get close to the lift. It likes us afraid. Afraid people keep talking.") \
    X(T3D, SP_PARK, "The last digits are zero, nine. Station nine. We should never have given it a name.") \
    X(M_OVER, SP_ELLIS, "Mercer? I can see you. Over here.") \
    X(M_HELP, SP_SAM, "Help me. The door is stuck. Please. Please help me.") \
    X(M_BED, SP_DANA, "Come to bed, Sam. It is so late.") \
    X(M_DONTGO, SP_OKAFOR, "Do not go down there. Please. Do not go down there.") \
    X(M_HELLO, SP_MERCER, "Is anybody there? Hello? Hello?") \
    X(M_WHY, SP_ELLIS, "Why are you not answering me, Mercer?") \
    X(M_SAY, SP_ENTITY, "Say something. Say something. Say something.") \
    X(M_BEHIND, SP_PARK, "It is right behind you.") \
    X(M_LISTEN, SP_DANA, "I know you are listening.") \
    X(M_NAME, SP_ELLIS, "Mercer. Mercer. Mercer. Mercer.") \
    X(R1A1, SP_MERCER, "Maintenance report. Level B one. I am alone down here.") \
    X(R1A2, SP_MERCER, "Maintenance report, night three. All quiet on B one.") \
    X(R1B1, SP_MERCER, "Power is out. Emergency lights only.") \
    X(R1B2, SP_MERCER, "All systems nominal. Nothing to report.") \
    X(R2A1, SP_MERCER, "I heard a voice. It knew my name.") \
    X(R2A2, SP_MERCER, "No disturbances. Nobody is down here with me.") \
    X(R2B1, SP_MERCER, "Generator running. The power keeps failing.") \
    X(R2B2, SP_MERCER, "Generator running. I am not afraid.") \
    X(R3A1, SP_MERCER, "If anyone hears this, do not answer the radio.") \
    X(R3A2, SP_MERCER, "Everything is fine. Send the elevator.") \
    X(R3B1, SP_MERCER, "I am leaving Station nine. Do not send anyone down.") \
    X(R3B2, SP_MERCER, "Please. Please let me go home.") \
    X(C_1, SP_ENTITY, "You came all the way down. I am so glad.") \
    X(C_2, SP_ENTITY, "I know your voice now. Say it again.") \
    X(C_3, SP_ENTITY, "Everyone talks to me in the end.") \
    X(END_ELLIS, SP_ELLIS, "Welcome to the surface, Mercer. We have been waiting so long.") \
    X(END_MERCER, SP_MERCER, "All clear. Sending the next shift down.") \
    X(END_HISS, SP_MERCER, "Maintenance report. Night three. All systems nominal.")

enum LineId {
#define X(id, sp, t) L_##id,
    LINES(X)
#undef X
    L_COUNT
};
static const LineDef LD[] = {
#define X(id, sp, t) {sp, t},
    LINES(X)
#undef X
};

static int lineSnd[L_COUNT];       // sound-bank index for the "real" version of each line
static int lineSndMimic[L_COUNT];  // glitchy, uncanny version (for the entity)

struct VoicePreset { float pitch, fshift, speed, breath; int fx; };
static const VoicePreset VP[SP_COUNT] = {
    {112, 0.95f, 1.2f, 0.10f, 1},  // ellis (radio)
    {100, 0.92f, 1.1f, 0.10f, 2},  // sam (tape)
    {188, 1.15f, 1.1f, 0.12f, 2},  // okafor
    {150, 1.05f, 1.2f, 0.10f, 2},  // park
    {215, 1.22f, 1.0f, 0.25f, 0},  // dana
    {125, 1.00f, 1.15f, 0.10f, 2}, // mercer
    {90, 0.85f, 0.9f, 0.30f, 3},   // entity
};

inline void synthAllVoices() {
    for (int i = 0; i < L_COUNT; i++) {
        const VoicePreset& v = VP[LD[i].sp];
        lineSnd[i] = audio.add(voice(LD[i].text, v.pitch, v.fshift, v.speed, v.breath, 0.0f, v.fx, 0.85f, i + 1));
        int fx = LD[i].sp == SP_ENTITY ? 3 : 0;
        lineSndMimic[i] = audio.add(voice(LD[i].text, v.pitch * 0.97f, v.fshift * 1.03f, v.speed * 0.9f, v.breath + 0.12f, 0.32f, fx, 0.7f, i + 500));
    }
}

// ---------- tapes ----------
struct TapeDef { const char* title; int lines[4]; int digitsA, digitsB; };
static const TapeDef TAPES[3] = {
    {"TAPE 1: S. WHITAKER", {L_T1A, L_T1B, L_T1C, L_T1D}, 4, 7},
    {"TAPE 2: DR OKAFOR", {L_T2A, L_T2B, L_T2C, L_T2D}, 1, 3},
    {"TAPE 3: CHIEF PARK", {L_T3A, L_T3B, L_T3C, L_T3D}, 0, 9},
};
// code = 47 13 09  ->  471309
static const char* SECURITY_CODE = "471309";

// ---------- notes ----------
static const char* NOTES[5] = {
    "NIGHT SHIFT NOTICE\n\nAll nightly maintenance reports\nmust be VOICE-LOGGED at the report\nterminal. Do not skip a report.\nStation procedure is procedure.\n\n- STATION MANAGEMENT",
    "DO NOT ANSWER THE INTERCOM\nAFTER MIDNIGHT.\n\nI do not care what it sounds like.\n\n- S.W.",
    "Pump 3 is making that sound again.\nLike someone humming in the pipes.\nTold maintenance. They said they would\nsend someone. Nobody came.\nNobody ever comes.",
    "RESTRICTED - LEVEL B3\nECHO CHAMBER\n\nSONIC LATTICE MUST REMAIN POWERED AT\nALL TIMES. ELEVATOR IS VOICE-LOCKED:\nSTAFF ONLY.",
    "IT LEARNS WHEN YOU TALK.\nIT LEARNS WHEN YOU TALK.\nIT LEARNS WHEN YOU TALK.\n\nSTOP TALKING\nSTOP TALKING\nSTOP",
};

// ---------- nightly reports (voice-logged by the player; the thing steals the audio) ----------
struct ReportDef {
    const char* header;
    const char* prompt[2];
    const char* opt[2][2];
    int line[2][2];
};
static const ReportDef REPORTS[3] = {
    {"NIGHTLY REPORT 1 / 3", {"STATE YOUR LOCATION AND STATUS.", "EQUIPMENT STATUS?"},
     {{"I AM ALONE DOWN HERE.", "ALL QUIET ON B ONE."}, {"POWER IS OUT. EMERGENCY LIGHTS.", "ALL SYSTEMS NOMINAL."}},
     {{L_R1A1, L_R1A2}, {L_R1B1, L_R1B2}}},
    {"NIGHTLY REPORT 2 / 3", {"ANY SOUNDS OR DISTURBANCES?", "GENERATOR STATUS?"},
     {{"I HEARD A VOICE. IT KNEW MY NAME.", "NOBODY IS DOWN HERE WITH ME."}, {"RUNNING. THE POWER KEEPS FAILING.", "RUNNING. I AM NOT AFRAID."}},
     {{L_R2A1, L_R2A2}, {L_R2B1, L_R2B2}}},
    {"NIGHTLY REPORT 3 / 3", {"FINAL ENTRY. MESSAGE FOR SURFACE?", "ANYTHING ELSE TO RECORD?"},
     {{"DO NOT ANSWER THE RADIO.", "EVERYTHING IS FINE. SEND THE LIFT."}, {"DO NOT SEND ANYONE DOWN.", "PLEASE LET ME GO HOME."}},
     {{L_R3A1, L_R3A2}, {L_R3B1, L_R3B2}}},
};

// short mimic lure phrases the thing cycles through (it prefers what it has learned from you)
static const int MIMIC_POOL[] = {L_M_OVER, L_M_HELP, L_M_BED, L_M_DONTGO, L_M_HELLO, L_M_WHY, L_M_SAY, L_M_BEHIND, L_M_LISTEN, L_M_NAME};
static const int MIMIC_POOL_N = sizeof(MIMIC_POOL) / sizeof(int);

}  // namespace S9
