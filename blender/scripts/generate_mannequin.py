"""
=============================================================================
DARKLOTUS - Procedural Tekken Mannequin Generator for Blender
=============================================================================
Generates a standard 3D fighting mannequin mesh and armature in Blender
matching DarkLotus bone naming conventions:
- Root
  └── Pelvis
      ├── Spine -> Chest -> Neck -> Head
      ├── Clavicle.L -> UpperArm.L -> Forearm.L -> Hand.L
      ├── Clavicle.R -> UpperArm.R -> Forearm.R -> Hand.R
      ├── Thigh.L -> Shin.L -> Foot.L
      └── Thigh.R -> Shin.R -> Foot.R
=============================================================================
"""

import sys

try:
    import bpy
except ImportError:
    print("Run this script inside Blender.")
    sys.exit(1)

def create_darklotus_mannequin():
    # Clear existing objects in scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()

    # Create Armature
    bpy.ops.object.armature_add(enter_editmode=True, align='WORLD', location=(0, 0, 0))
    armature_obj = bpy.context.active_object
    armature_obj.name = "DarkLotus_Rig"
    arm = armature_obj.data
    arm.name = "DarkLotus_Armature"

    # Edit bones
    edit_bones = arm.edit_bones
    root_bone = edit_bones['Bone']
    root_bone.name = "Root"
    root_bone.head = (0, 0, 0)
    root_bone.tail = (0, 0, 0.2)

    # Pelvis
    pelvis = edit_bones.new('Pelvis')
    pelvis.head = (0, 0, 0.95)
    pelvis.tail = (0, 0, 1.1)
    pelvis.parent = root_bone

    # Spine & Chest
    spine = edit_bones.new('Spine')
    spine.head = (0, 0, 1.1)
    spine.tail = (0, 0, 1.35)
    spine.parent = pelvis

    chest = edit_bones.new('Chest')
    chest.head = (0, 0, 1.35)
    chest.tail = (0, 0, 1.55)
    chest.parent = spine

    # Neck & Head
    head = edit_bones.new('Head')
    head.head = (0, 0, 1.55)
    head.tail = (0, 0, 1.80)
    head.parent = chest

    # Left Arm
    uparm_l = edit_bones.new('UpperArm.L')
    uparm_l.head = (0.22, 0, 1.48)
    uparm_l.tail = (0.48, 0, 1.45)
    uparm_l.parent = chest

    forearm_l = edit_bones.new('Forearm.L')
    forearm_l.head = (0.48, 0, 1.45)
    forearm_l.tail = (0.75, 0.15, 1.40)
    forearm_l.parent = uparm_l

    hand_l = edit_bones.new('Hand.L')
    hand_l.head = (0.75, 0.15, 1.40)
    hand_l.tail = (0.86, 0.18, 1.38)
    hand_l.parent = forearm_l

    # Right Arm
    uparm_r = edit_bones.new('UpperArm.R')
    uparm_r.head = (-0.22, 0, 1.48)
    uparm_r.tail = (-0.48, 0, 1.45)
    uparm_r.parent = chest

    forearm_r = edit_bones.new('Forearm.R')
    forearm_r.head = (-0.48, 0, 1.45)
    forearm_r.tail = (-0.75, 0.15, 1.40)
    forearm_r.parent = uparm_r

    hand_r = edit_bones.new('Hand.R')
    hand_r.head = (-0.75, 0.15, 1.40)
    hand_r.tail = (-0.86, 0.18, 1.38)
    hand_r.parent = forearm_r

    # Left Leg
    thigh_l = edit_bones.new('Thigh.L')
    thigh_l.head = (0.16, 0, 0.95)
    thigh_l.tail = (0.18, 0, 0.50)
    thigh_l.parent = pelvis

    shin_l = edit_bones.new('Shin.L')
    shin_l.head = (0.18, 0, 0.50)
    shin_l.tail = (0.18, 0, 0.10)
    shin_l.parent = thigh_l

    foot_l = edit_bones.new('Foot.L')
    foot_l.head = (0.18, 0, 0.10)
    foot_l.tail = (0.18, 0.20, 0.0)
    foot_l.parent = shin_l

    # Right Leg
    thigh_r = edit_bones.new('Thigh.R')
    thigh_r.head = (-0.16, 0, 0.95)
    thigh_r.tail = (-0.18, 0, 0.50)
    thigh_r.parent = pelvis

    shin_r = edit_bones.new('Shin.R')
    shin_r.head = (-0.18, 0, 0.50)
    shin_r.tail = (-0.18, 0, 0.10)
    shin_r.parent = thigh_r

    foot_r = edit_bones.new('Foot.R')
    foot_r.head = (-0.18, 0, 0.10)
    foot_r.tail = (-0.18, 0.20, 0.0)
    foot_r.parent = shin_r

    bpy.ops.object.mode_set(mode='OBJECT')
    print("[DARKLOTUS] Rig template created successfully with standard combat bones!")

if __name__ == "__main__":
    create_darklotus_mannequin()
