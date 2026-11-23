# STATION 09: ECHO
A PS2 / early-2000s style survival-horror game. ~4,500 lines of C++, **zero dependencies**:
software renderer (320x240, dithered 15-bit colour, grain, interlace), procedural audio (every sound AND
every "voice" is synthesized at start-up), Win32 only. The final game is **one `.exe`** — nothing to install to play.

> You are Technician R. Mercer, night shift, Halvorsen Deep Research Station 09.
> The power is out, surface contact is gone, and something down here has learned to imitate human voices.

## Getting Station09.exe (pick ONE — only needed once)
| Option | What you do |
|---|---|
| **A. GitHub (no compiler at all)** | Make a GitHub repo, upload this folder, push. Actions tab → *Build Station09.exe* → download the artifact. (`.github/workflows/build.yml` is included.) |
| **B. Portable compiler (no installer)** | Download the `w64devkit` **.zip** from github.com/skeeto/w64devkit/releases, unzip it into this folder (so `w64devkit\bin\g++.exe` exists), double-click **`build.bat`**. |
| C. Visual Studio / Build Tools | Run `build_msvc.bat` in a Developer Command Prompt (untested). |
| D. Already have MinGW g++ | `build.bat` finds it automatically. |

Manual command: `g++ -std=c++17 -O2 -s -static -mwindows -o Station09.exe src/main_win32.cpp -lgdi32 -lwinmm -luser32`

## Controls
WASD move · Mouse look (arrow keys also turn) · Shift run · **E** interact (hold E for repairs) · **F** flashlight ·
**R** swap battery · **H** medkit · **Click** fire pistol · **Tab** journal · **Esc** pause · **F11** fullscreen

## The job
1. **Check the communication systems** – tune the console on B1 (A/D, hold a clear signal).
2. **Restore the power** – three fuses across B1/B2 → fuse box → hold E on the backup generator (B2).
   *Something keeps switching the power off whenever you get close* — find the breaker closet and reset it.
3. **Recover the security codes** – three employee recordings → security computer (B2).
4. **Unlock the emergency elevator** – go down to the restricted floor (B3), cross the Echo Chamber, call the elevator, survive.
Also: **record the nightly maintenance reports** at the report terminals. The thing is listening.

Features: stalker AI with hearing/sight, hiding in lockers, limited pistol (staggers, never kills), scripted jump scares,
7 cutscenes, mimic voice lures (it replays *your own* report lines back at you), 2 endings + death/checkpoints.

## Spoilers: the two endings
At the core terminal you choose to **purge** the voice archive or **leave it alone**. Same elevator, very different ride.

## How "imitating human voices" works
There are no audio files. `audio.hpp` contains a small formant speech synthesizer (glottal source → three band-pass
formant filters, vowels/fricatives/stops) with glitch/stutter and "entity" layering. Every line of dialogue and every
player report line is synthesized at load time. The result is deliberately robotic-uncanny. Want real voices? See
"Options" in the chat notes: drop-in WAV loading is the natural next step.

## Layout
`src/` core.hpp · gfx.hpp (renderer) · audio.hpp · world.hpp (maps) · entity.hpp (AI+model) · story.hpp · game*.hpp · main_win32.cpp
`tests/bot.cpp` plays the whole game to both endings headlessly (Linux/any g++), `tests/shots*.cpp` render screenshots.
