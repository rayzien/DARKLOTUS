# DARKLOTUS - Comprehensive Blender 3D Pipeline Guide

## 1. Character Modeling Specifications

To ensure optimal performance and seamless integration with DarkLotus's Raylib/OpenGL 3D renderer:

- **Target Triangle Count**: 8,000 to 25,000 triangles per character model.
- **Mesh Topology**: Clean edge loops around articulation joints (Shoulders, Elbows, Knees, Hips) to prevent pinching during deep crouches and high kicks.
- **Pivot Point**: The origin of the mesh must be centered at `(0, 0, 0)` directly between the fighter's feet on the ground plane.
- **Materials**: Use Principled BSDF shaders with Base Color (Albedo), Metallic, Roughness, and Normal maps packed into glTF standard ORM (Occlusion, Roughness, Metallic) channels.

---

## 2. Rigging with DarkLotus Armature

1. Open Blender.
2. In the Python Console or Text Editor, run `blender/scripts/generate_mannequin.py` to create the standard reference skeleton.
3. Align the bones to your custom mesh in Edit Mode.
4. Parent the mesh to the armature (`Ctrl + P` -> *With Automatic Weights*).
5. Ensure vertex weight normalization (max 4 bone influences per vertex for real-time skeletal hardware skinning).

---

## 3. Creating Tekken Combat Animations

In Blender's **Dope Sheet** / **Action Editor**:
- Create individual actions prefixed with standard identifiers:
  - `IDLE`: Relaxed breathing stance, fists raised.
  - `WALK_FWD`: Forward foot shuffle.
  - `WALK_BACK`: Retreating guard step.
  - `SIDESTEP_L`: Lateral sidestep forward (+Z).
  - `SIDESTEP_R`: Lateral sidestep backward (-Z).
  - `1_JAB`: Left punch extending forward at Frame 10, retracting by Frame 20.
  - `2_CROSS`: Right straight punch with torso twist at Frame 12, recovering by Frame 24.
  - `LAUNCHER`: Powerful upward strike lifting off the ground at Frame 15.
  - `LOW_SWEEP`: Low sweeping kick rotating near the ground plane.
  - `KNOCKDOWN`: Falling backwards, impact on ground, and recovery getup.

---

## 4. Exporting to DarkLotus

Run the automated exporter script from inside Blender or from terminal:

```bash
blender my_fighter.blend --background --python /home/zero/DarkLotus/blender/scripts/export_fighter.py -- lotus.glb
```

The model will automatically be saved to `/home/zero/DarkLotus/assets/models/fighters/lotus.glb`.
DarkLotus will automatically detect and load it on next launch!
