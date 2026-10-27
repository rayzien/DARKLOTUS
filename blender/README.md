# DarkLotus - Blender 3D Art Pipeline

Welcome to the **DarkLotus 3D Graphics Pipeline**! This guide details how to create 3D character models, rigs, and martial arts animations in Blender and export them directly for the DarkLotus C++ engine.

---

## 1. Quick Start

1. **Open Blender** (Blender 3.6 LTS, 4.0+, or newer recommended).
2. Generate or import your character mesh.
3. Use `blender/scripts/generate_mannequin.py` inside Blender's Scripting workspace to auto-generate the standard DarkLotus combat skeleton.
4. Animate your actions (Idle, Walk, 1 Jab, 2 Cross, Uppercut launcher, Roundhouse kick, Knockdown).
5. Run `blender/scripts/export_fighter.py` or export manually as **glTF 2.0 (.glb)** into `assets/models/fighters/`.

---

## 2. Scale & Coordinate System

- **Units**: Metric, Meters.
- **Scale**: `1.0 Blender Unit = 1.0 Meter in DarkLotus`.
- **Character Height**: ~1.75m to 1.85m tall.
- **Forward Axis**: `-Y` in Blender -> `+Z` or facing right `+X` in engine. When exporting glTF, check **+Y Up** (glTF standard).

---

## 3. Armature Bone Hierarchy

To ensure automatic hurtbox tracking and hit registration, name your bones following this convention:

```
Root
 └── Pelvis
      ├── Spine
      │    └── Chest
      │         ├── Neck
      │         │    └── Head       (Head Hurtbox)
      │         ├── UpperArm.L -> Forearm.L -> Hand.L (Left Arm & Fist Hitbox)
      │         └── UpperArm.R -> Forearm.R -> Hand.R (Right Arm & Fist Hitbox)
      ├── Thigh.L -> Shin.L -> Foot.L (Left Leg & Kick Hitbox)
      └── Thigh.R -> Shin.R -> Foot.R (Right Leg & Kick Hitbox)
```

---

## 4. Animation Actions & Frame Counts

Standard 60 FPS frame data targets for DarkLotus:

| Action Name | Total Frames | Active Window | Description |
|---|---|---|---|
| `IDLE` | 60 | N/A | Relaxed fighting stance with subtle breathing |
| `WALK_FWD` | 40 | N/A | Forward approach step |
| `WALK_BACK` | 40 | N/A | Backward retreat & guard |
| `SIDESTEP_L` | 24 | N/A | Lateral evasion into foreground |
| `SIDESTEP_R` | 24 | N/A | Lateral evasion into background |
| `1_JAB` | 20 | Frames 10-12 | Fast High Left Punch |
| `2_CROSS` | 24 | Frames 12-15 | Mid Right Straight |
| `3_MIDKICK` | 28 | Frames 14-17 | Mid Front/Side Kick |
| `4_ROUNDHOUSE` | 32 | Frames 16-20 | High Roundhouse Kick |
| `LAUNCHER` | 34 | Frames 15-19 | Upward uppercut for juggle combos |
| `LOW_SWEEP` | 30 | Frames 16-19 | Low sweep taking opponent off feet |
| `RAGE_ART` | 90 | Frames 20-25 | Cinematic Rage Art super strike |
| `HIT_STUN` | 22 | N/A | Stumble hit reaction |
| `KNOCKDOWN` | 50 | N/A | Knocked onto back and rolling to feet |

---

## 5. Export Settings (glTF 2.0 / .glb)

When manually exporting from Blender (`File` -> `Export` -> `glTF 2.0 (.glb)`):
- **Format**: `glTF Binary (.glb)`
- **Include**: Selected Objects (Mesh + Armature)
- **Transform**: `+Y Up`
- **Geometry**: UVs, Normals, Tangents, Vertex Colors
- **Animation**:
  - `Animation`: Checked
  - `Shape Keys`: Checked (if using facial expressions)
  - `Skinning`: Checked
  - `Include All Bone Influences`: Unchecked (max 4 per vertex for performance)
  - `NLA Strips`: Checked
  - `Export Deformation Bones Only`: Checked
