"""
DARKLOTUS - Ultra-Refined Fur Collar & Organic Beard Test
"""
import math
import bpy
import bmesh
from mathutils import Vector, Matrix, Euler

def build_plush_fur_collar(col, mats):
    """
    Constructs a plush, luxurious, cloud-like shearling fur shawl collar:
    - Voluminous contoured base following shoulders & upper chest
    - Over 60 procedural overlapping soft fluffy tufts radiating outward and downward
    - Golden edge tips matching the stylized cartoon sketch
    """
    created_objs = []

    # 1. Base crescent collar torus
    bpy.ops.mesh.primitive_torus_add(major_radius=0.19, minor_radius=0.065, location=(0, -0.045, 1.455))
    core = bpy.context.active_object
    core.name = "Fur_Collar_Base"
    core.rotation_euler = (math.radians(18), 0, 0)
    core.scale = (1.25, 1.05, 0.85)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    if core.data.materials:
        core.data.materials[0] = mats['fur_collar']
    else:
        core.data.materials.append(mats['fur_collar'])
    for p in core.data.polygons:
        p.use_smooth = True
    created_objs.append(core)

    # 2. Overlapping soft fluffy fur puffs clustered over the entire surface
    # Front chest drape, shoulder drape, and side fluffs
    bm_fluffs = bmesh.new()
    # Generate 64 plush fur fluffs along 2 concentric rings + outer rim
    for ring_r, z_offset, count, base_scale in [
        (0.20, 0.00, 20, 0.038),   # inner fluff ring
        (0.24, -0.015, 24, 0.042), # middle fluff ring
        (0.27, -0.030, 26, 0.035), # outer jagged edge ring
    ]:
        for i in range(count):
            ang = 2.0 * math.pi * i / count
            # Wrap around neck, shoulders, and chest
            cx = ring_r * 1.25 * math.cos(ang)
            cy = ring_r * 1.05 * math.sin(ang) - 0.045
            cz = 1.455 + z_offset - 0.05 * math.sin(ang)

            # Direction outward
            dx = math.cos(ang)
            dy = math.sin(ang)
            dz = -0.4
            length = math.hypot(dx, dy, dz) + 1e-6
            dx, dy, dz = dx / length, dy / length, dz / length

            # Create soft 8-segment teardrop / puff
            puff_len = base_scale * (1.2 + 0.3 * math.sin(i * 4.5))
            puff_w = base_scale * 0.75

            base_v = Vector((cx, cy, cz))
            tip_v = base_v + Vector((dx * puff_len, dy * puff_len, dz * puff_len))

            # Build soft cone/lozenge
            v_base = []
            for k in range(6):
                kang = 2.0 * math.pi * k / 6
                # Tangent vectors
                tx = -dy * math.cos(kang) * puff_w
                ty = dx * math.cos(kang) * puff_w
                tz = math.sin(kang) * puff_w * 0.8
                v_base.append(bm_fluffs.verts.new(base_v + Vector((tx, ty, tz))))

            v_tip = bm_fluffs.verts.new(tip_v)
            for k in range(6):
                nk = (k + 1) % 6
                bm_fluffs.faces.new((v_base[k], v_base[nk], v_tip))

    mesh_fluffs = bpy.data.meshes.new("Fur_Collar_Plush_Tufts")
    bm_fluffs.to_mesh(mesh_fluffs)
    bm_fluffs.free()
    fluff_obj = bpy.data.objects.new(mesh_fluffs.name, mesh_fluffs)
    col.objects.link(fluff_obj)
    if fluff_obj.data.materials:
        fluff_obj.data.materials[0] = mats['fur_collar']
    else:
        fluff_obj.data.materials.append(mats['fur_collar'])
    for p in fluff_obj.data.polygons:
        p.use_smooth = True
    created_objs.append(fluff_obj)

    # 3. Outer golden edge tufts (matching the sketch's yellow trim contour)
    bpy.ops.mesh.primitive_torus_add(major_radius=0.25, minor_radius=0.008, location=(0, -0.050, 1.425))
    trim = bpy.context.active_object
    trim.name = "Fur_Collar_Gold_Edge"
    trim.rotation_euler = (math.radians(18), 0, 0)
    trim.scale = (1.28, 1.06, 0.85)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    if trim.data.materials:
        trim.data.materials[0] = mats['fur_trim']
    else:
        trim.data.materials.append(mats['fur_trim'])
    for p in trim.data.polygons:
        p.use_smooth = True
    created_objs.append(trim)

    return created_objs
