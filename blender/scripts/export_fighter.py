"""
=============================================================================
DARKLOTUS - Blender to glTF/GLB Exporter Script
=============================================================================
This script exports the active rigged character in Blender directly into
the DarkLotus engine asset format (.glb) with skinning, materials, and animations.

Usage inside Blender:
  1. Open your character blend file.
  2. Switch to the Scripting workspace.
  3. Load and run this script.
Or run headlessly from command line:
  blender fighter.blend --background --python export_fighter.py -- output_name.glb
=============================================================================
"""

import sys
import os

try:
    import bpy
except ImportError:
    print("Error: This script must be run inside Blender's Python environment.")
    sys.exit(1)

def export_character(output_path):
    print(f"[DARKLOTUS] Exporting active character to: {output_path}")

    # Ensure output directory exists
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)

    # glTF 2.0 export configuration tailored for Raylib / DarkLotus 3D
    bpy.ops.export_scene.gltf(
        filepath=output_path,
        export_format='GLB',
        use_selection=False,
        export_yup=True,
        export_apply=False, # Preserve skeletal bind poses
        export_skins=True,
        export_all_influences=False,
        export_def_bones=True,
        export_animations=True,
        export_frame_range=True,
        export_force_sampling=True,
        export_nla_strips=True,
        export_image_format='AUTO'
    )
    print(f"[DARKLOTUS] Export successfully completed: {output_path}")

if __name__ == "__main__":
    # Check for CLI arguments after '--'
    argv = sys.argv
    if "--" in argv:
        args = argv[argv.index("--") + 1:]
        out_filename = args[0] if len(args) > 0 else "fighter.glb"
    else:
        out_filename = "fighter.glb"

    # Default export directory in DarkLotus
    engine_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    dest_path = os.path.join(engine_root, "assets", "models", "fighters", out_filename)

    export_character(dest_path)
