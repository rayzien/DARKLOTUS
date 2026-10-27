# 🌸 DARKLOTUS

> A high-performance, modular **Tekken 8-style 3D Fighting Game Engine** written in **C++17**, featuring Blender 3D graphics pipeline integration and clean, specialized managers.

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)]()
[![Graphics](https://img.shields.io/badge/Graphics-Raylib%20%2F%20OpenGL-red.svg)](https://www.raylib.com/)

---

## 🥋 Key Features

- **Tekken 3D Movement & Combat System**:
  - Full 3D lateral sidestepping (Z-axis evasion).
  - Authentic 4-button fighting layout: `1` (LP), `2` (RP), `3` (LK), `4` (RK).
  - High / Mid / Low attack heights and authentic guard dynamics (ducking highs, blocking mids, crushing lows).
  - **Air Juggle Combo Physics**: Launch opponents into the air with uppercuts and juggle them before they hit the floor.
  - **Counter Hit Engine**: 125% damage boost, hit advantage, and screen trauma when interrupting opponent startup frames.
  - **Tekken 8 Heat & Rage System**: Glowing Rage Aura below 25% HP enabling devastating Rage Arts.
  - **Wall Splat**: Octagonal arena boundaries trigger wall splat stuns when opponents are knocked into barriers.

- **Dynamic 3D Camera System**:
  - Automatically tracks the midpoint between both fighters.
  - Dynamic framing and zoom based on character proximity.
  - Screen trauma camera shake on heavy strikes and Counter Hits.
  - Cinematic slow-motion on round-deciding trades and K.O. finishes.

- **Blender 3D Graphics Pipeline**:
  - Ready for custom `.glb` (glTF 2.0) skeletal rigged models and animations.
  - Built-in procedural 3D martial arts mannequin so the game is fully playable in 3D right out of the box!
  - Automated Blender Python exporter (`blender/scripts/export_fighter.py`) and procedural mannequin rig generator (`blender/scripts/generate_mannequin.py`).

- **Procedural 3D Audio Synthesis**:
  - Real-time synthesized combat sound effects (punches, thuds, metallic guards, counter-hit bass booms, whooshes) built directly in C++ without requiring external asset dependencies.

---

## 🏛️ Specialized Manager Architecture

DarkLotus follows strict separation of concerns with dedicated, single-responsibility managers:

| Manager | Responsibility |
|---|---|
| **`ClickManager`** | Screen-to-world 3D raycasting, mouse clicks, drag deltas, and picking collision. |
| **`ButtonManager`** | UI button states (hover/click) and 60-frame input circular buffer for Tekken combo execution. |
| **`SettingsManager`** | Video, audio, keybindings, and tournament rule configurations with disk persistence (`.cfg`). |
| **`GraphicsManager`** | Window lifecycle, 3D tracking camera, trauma shake, slow-mo, and 3D hit spark particle system. |
| **`CharacterManager`** | Fighter roster (Lotus, Kage, Raijin), mesh loading, skeletal rigs, and 60 FPS update dispatch. |
| **`AnimationManager`** | Animation clip registration, bone interpolation, playback speed, and frame event triggers. |
| **`UIManager`** | Tekken 8 style HUD, lifebars with damage lag, round timer, Heat/Rage gauges, and combo counters. |
| **`CombatManager`** | 3D hitbox/hurtbox collision, guard checks (High/Mid/Low), counter-hits, juggles, and damage scaling. |
| **`SoundManager`** | Procedural waveform audio generation and external sound/music player. |
| **`StageManager`** | 3D fighting arena meshes, floor reflections, grid, and perimeter wall splat constraints. |
| **`GameManager`** | Master engine orchestrator coordinating tick updates, state transitions, and rendering. |

---

## 🚀 Building & Running

### Prerequisites
- GCC / G++ (supporting C++17) or Clang
- Make or CMake (3.16+)
- X11 / OpenGL libraries (`libx11-dev` on Debian/Ubuntu)

### Quick Build with Makefile:
```bash
cd /home/zero/DarkLotus
make -j$(nproc)
./bin/DarkLotus
```

### Build with CMake:
```bash
cd /home/zero/DarkLotus
mkdir -p build && cd build
cmake ..
cmake --build .
../bin/DarkLotus
```

---

## 🎮 Controls

### Player 1 (Keyboard)
- **Sidestep Up**: `W`
- **Sidestep Down / Crouch**: `S`
- **Retreat / Guard**: `A`
- **Advance**: `D`
- **1: Left Punch (LP)**: `U`
- **2: Right Punch (RP)**: `I`
- **3: Left Kick (LK)**: `J`
- **4: Right Kick (RK)**: `K`
- **1 + 2**: `O`
- **3 + 4**: `L`
- **Rage Art**: `SPACE`
- **Pause**: `ESCAPE`

*(Full Gamepad controls and Player 2 bindings detailed in [docs/CONTROLS.md](docs/CONTROLS.md))*

---

## 🎨 Blender Art Pipeline

See [blender/README.md](blender/README.md) and [docs/BLENDER_PIPELINE.md](docs/BLENDER_PIPELINE.md) for complete details on:
1. Rigging humanoid models with DarkLotus combat bones.
2. Setting up 60 FPS animation actions.
3. Exporting directly to `assets/models/fighters/` using `blender/scripts/export_fighter.py`.

---

## 👤 Author & Repository

- **Project**: DARKLOTUS
- **GitHub**: [RAYZIEN](https://github.com/rayzien)
- **License**: MIT
