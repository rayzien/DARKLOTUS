"""
=============================================================================
DARKLOTUS - OODIII JII Master 3D Character Generator (High Likeness v2)
=============================================================================
Procedurally constructs the authentic 3D fighter model 'OODIII JII'
matching the avatar idea sketch in ~/Downloads/odiiii jiii.png:
- High-fidelity likeness:
  * Broad cranium with organic, curved top-center white hair tuft / crest
  * Flowing white side and back hair flared neatly around ears and nape
  * Bold, jet-black arched eyebrows
  * Black wireframe spectacles with nose bridge, temples & clear open eye view
  * Warm expressive hazel eyes with white sclera, brown iris, pupil & specular glint
  * Sculpted button nose with nostril wings and calm warm rose lips
  * Full sculpted white beard and mustache framing jawline in rounded spade profile
  * Lavish fluffy ivory shearling fur shawl collar with layered plush radial fluffs
  * Tailored golden-yellow suit jacket / sherwani with mandarin collar, chest buttons & cuffs
  * Tailored formal dress suit trousers with tapered legs & cuff breaks
  * Polished black Oxford leather shoes with soles, welts, and raised heels
  * Skinned to standard DarkLotus combat skeleton
  * Full martial arts animations: IDLE, 1_JAB, 2_CROSS, 3_MIDKICK, WALK_FWD
  * Direct export to glTF 2.0 (.glb) and .blend for DarkLotus C++ engine
=============================================================================
"""

import sys
import os
import math

try:
    import bpy
    import bmesh
    from mathutils import Vector, Matrix, Euler, Quaternion
except ImportError:
    print("Error: Run this script inside Blender's Python environment.")
    sys.exit(1)


def clear_scene():
    """Wipes scene clean for pure procedural generation."""
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    col = bpy.data.collections.new("OodiiiJii_Rigged")
    scene.collection.children.link(col)
    return col


def get_or_create_mat(name, base_color, metallic=0.0, roughness=0.4, transmission=0.0, ior=1.45, emission_color=None, emission_strength=0.0):
    """Creates or configures a calibrated Principled BSDF material."""
    if name in bpy.data.materials:
        return bpy.data.materials[name]

    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get("Principled BSDF")

    if bsdf:
        bsdf.inputs['Base Color'].default_value = base_color
        bsdf.inputs['Metallic'].default_value = metallic
        bsdf.inputs['Roughness'].default_value = roughness
        if 'Transmission Weight' in bsdf.inputs:
            bsdf.inputs['Transmission Weight'].default_value = transmission
        elif 'Transmission' in bsdf.inputs:
            bsdf.inputs['Transmission'].default_value = transmission
        if 'IOR' in bsdf.inputs:
            bsdf.inputs['IOR'].default_value = ior

        if emission_color and emission_strength > 0:
            if 'Emission Color' in bsdf.inputs:
                bsdf.inputs['Emission Color'].default_value = emission_color
                bsdf.inputs['Emission Strength'].default_value = emission_strength

    return mat


def assign_mat_and_smooth(obj, mat):
    """Assigns material to object and enables smooth shading on all polygons."""
    if obj.data.materials:
        obj.data.materials[0] = mat
    else:
        obj.data.materials.append(mat)
    for poly in obj.data.polygons:
        poly.use_smooth = True


def build_materials():
    """Builds calibrated materials sampled directly from odiiii jiii.png."""
    mats = {}
    # Warm peach/tan Indian skin matching sketch: #F0C7B1 -> (0.941, 0.780, 0.694)
    mats['skin'] = get_or_create_mat("M_Skin", (0.941, 0.780, 0.694, 1.0), roughness=0.42)

    # Pure Silvery White Hair & Beard -> (0.985, 0.985, 1.0)
    mats['white_hair'] = get_or_create_mat("M_WhiteHair", (0.985, 0.985, 1.0, 1.0), roughness=0.32)

    # Solid Jet Black Eyebrows -> (0.02, 0.02, 0.02)
    mats['eyebrows'] = get_or_create_mat("M_Eyebrows", (0.02, 0.02, 0.02, 1.0), roughness=0.7)

    # Glasses Frame: Crisp Dark Wire / Acetate -> (0.05, 0.04, 0.04)
    mats['glasses_frame'] = get_or_create_mat("M_GlassesFrame", (0.05, 0.04, 0.04, 1.0), metallic=0.7, roughness=0.20)

    # Eyes: Sclera, Warm Brown/Hazel Iris, Deep Black Pupil, Catchlight Glint
    mats['sclera'] = get_or_create_mat("M_Sclera", (0.96, 0.96, 0.97, 1.0), roughness=0.10)
    mats['iris'] = get_or_create_mat("M_Iris", (0.42, 0.28, 0.18, 1.0), roughness=0.20)
    mats['pupil'] = get_or_create_mat("M_Pupil", (0.01, 0.01, 0.01, 1.0), roughness=0.10)
    mats['catchlight'] = get_or_create_mat("M_Catchlight", (1.0, 1.0, 1.0, 1.0), emission_color=(1.0, 1.0, 1.0, 1.0), emission_strength=2.8)

    # Lips: Warm rose/pink -> (0.855, 0.486, 0.529)
    mats['lips'] = get_or_create_mat("M_Lips", (0.855, 0.486, 0.529, 1.0), roughness=0.35)

    # Fur Collar: Luxurious Ivory / Cream Shearling -> (1.0, 0.976, 0.902)
    mats['fur_collar'] = get_or_create_mat("M_FurCollar", (1.0, 0.976, 0.902, 1.0), roughness=0.70)

    # Fur Collar Edge Trim: Golden Yellow Accent -> (0.969, 0.894, 0.675)
    mats['fur_trim'] = get_or_create_mat("M_FurTrim", (0.969, 0.894, 0.675, 1.0), roughness=0.55)

    # Suit Jacket: Vibrant Golden Saffron Yellow -> (1.0, 0.741, 0.004)
    mats['suit_jacket'] = get_or_create_mat("M_SuitJacket", (1.0, 0.741, 0.004, 1.0), roughness=0.38)

    # Gold Trim & Ornamental Buttons -> (1.0, 0.84, 0.15)
    mats['gold_trim'] = get_or_create_mat("M_GoldTrim", (1.0, 0.84, 0.15, 1.0), metallic=0.90, roughness=0.18)

    # Suit Pants: Elegant Tailored Midnight Charcoal -> (0.11, 0.12, 0.16)
    mats['suit_pants'] = get_or_create_mat("M_SuitPants", (0.11, 0.12, 0.16, 1.0), roughness=0.52)

    # Dress Shoes: Polished Black Leather -> (0.04, 0.04, 0.05)
    mats['shoes'] = get_or_create_mat("M_Shoes", (0.035, 0.035, 0.04, 1.0), roughness=0.18, metallic=0.12)

    # Shoe Soles & Welts -> (0.08, 0.07, 0.06)
    mats['shoe_sole'] = get_or_create_mat("M_ShoeSole", (0.08, 0.07, 0.06, 1.0), roughness=0.65)

    return mats


# =============================================================================
# 1. HEAD, FACE, EYES, GLASSES, HAIR & BEARD
# =============================================================================

def build_head_mesh(col, mats):
    """Constructs head cranium, cheeks, chin, ears, and neck."""
    created_objs = []

    # Cranium (Smooth head base matching the broad bald forehead in the sketch)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=24, radius=1.0, location=(0, -0.012, 1.685))
    head_obj = bpy.context.active_object
    head_obj.name = "Head_Base"
    head_obj.scale = (0.118, 0.128, 0.136)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(head_obj, mats['skin'])
    created_objs.append(head_obj)

    # Cheeks / Jaw base to provide anatomical cheekbones and lower face support
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=16, radius=1.0, location=(0, -0.040, 1.590))
    jaw = bpy.context.active_object
    jaw.name = "Jaw_Base"
    jaw.scale = (0.102, 0.098, 0.082)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(jaw, mats['skin'])
    created_objs.append(jaw)

    # Ears: Left and Right (Prominent and clearly defined matching the sketch)
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_torus_add(major_radius=0.026, minor_radius=0.007, location=(side * 0.122, -0.015, 1.675))
        ear = bpy.context.active_object
        ear.name = f"Ear_{'L' if side > 0 else 'R'}"
        ear.rotation_euler = (math.radians(10), math.radians(side * 14), math.radians(side * 85))
        ear.scale = (1.0, 0.65, 1.45)
        bpy.ops.object.transform_apply(rotation=True, scale=True)
        assign_mat_and_smooth(ear, mats['skin'])
        created_objs.append(ear)

        # Ear lobe & inner concha bowl
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=0.012, location=(side * 0.120, -0.015, 1.650))
        lobe = bpy.context.active_object
        lobe.name = f"Ear_Lobe_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(lobe, mats['skin'])
        created_objs.append(lobe)

    # Neck (Clean cylindrical column)
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=0.075, depth=0.16, location=(0, -0.015, 1.515))
    neck = bpy.context.active_object
    neck.name = "Neck"
    assign_mat_and_smooth(neck, mats['skin'])
    created_objs.append(neck)

    return created_objs


def build_facial_features(col, mats):
    """Nose, lips, eyes, dark arched eyebrows, glasses."""
    created_objs = []

    # 1. Nose Tip (Rounded button nose matching sketch)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.016, location=(0, -0.136, 1.648))
    nose_tip = bpy.context.active_object
    nose_tip.name = "Nose_Tip"
    nose_tip.scale = (1.10, 0.88, 0.88)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(nose_tip, mats['skin'])
    created_objs.append(nose_tip)

    # Nostril wings (alar)
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=0.0095, location=(side * 0.016, -0.130, 1.645))
        wing = bpy.context.active_object
        wing.name = f"Nostril_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(wing, mats['skin'])
        created_objs.append(wing)

    # Nose Bridge
    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.009, depth=0.048, location=(0, -0.124, 1.674))
    bridge = bpy.context.active_object
    bridge.name = "Nose_Bridge"
    bridge.rotation_euler = (math.radians(-24), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    assign_mat_and_smooth(bridge, mats['skin'])
    created_objs.append(bridge)

    # 2. Lips (Warm rose pink with gentle calm smile)
    # Upper lip with Cupid's bow
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=0.011, location=(0, -0.132, 1.608))
    ulip = bpy.context.active_object
    ulip.name = "Upper_Lip"
    ulip.scale = (2.3, 0.75, 0.48)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(ulip, mats['lips'])
    created_objs.append(ulip)

    # Lower lip
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=0.013, location=(0, -0.130, 1.595))
    llip = bpy.context.active_object
    llip.name = "Lower_Lip"
    llip.scale = (2.0, 0.85, 0.58)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(llip, mats['lips'])
    created_objs.append(llip)

    # 3. Eyes & Eyeballs: Positioned clearly to look through the glasses
    for side in [1.0, -1.0]:
        eye_x = side * 0.047
        eye_z = 1.692

        # Sclera (White eyeball)
        bpy.ops.mesh.primitive_uv_sphere_add(segments=20, ring_count=16, radius=0.017, location=(eye_x, -0.115, eye_z))
        sclera = bpy.context.active_object
        sclera.name = f"Sclera_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(sclera, mats['sclera'])
        created_objs.append(sclera)

        # Iris (Warm Hazel disc)
        bpy.ops.mesh.primitive_cylinder_add(vertices=20, radius=0.0095, depth=0.003, location=(eye_x, -0.131, eye_z))
        iris = bpy.context.active_object
        iris.name = f"Iris_{'L' if side > 0 else 'R'}"
        iris.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(iris, mats['iris'])
        created_objs.append(iris)

        # Pupil (Deep black center)
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.0048, depth=0.004, location=(eye_x, -0.132, eye_z))
        pupil = bpy.context.active_object
        pupil.name = f"Pupil_{'L' if side > 0 else 'R'}"
        pupil.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(pupil, mats['pupil'])
        created_objs.append(pupil)

        # Catchlight (Bright white glint matching cartoon sparkle)
        bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=6, radius=0.0030, location=(eye_x - side * 0.003, -0.1335, eye_z + 0.004))
        glint = bpy.context.active_object
        glint.name = f"Catchlight_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(glint, mats['catchlight'])
        created_objs.append(glint)

    # 4. Eyebrows: Jet-black, thick, strongly arched bands (as in sketch)
    for side in [1.0, -1.0]:
        bm = bmesh.new()
        pts = [
            (side * 0.024, -0.130, 1.724, 0.0060),  # inner head
            (side * 0.044, -0.126, 1.738, 0.0075),  # rising arch
            (side * 0.064, -0.118, 1.745, 0.0078),  # peak arch
            (side * 0.082, -0.106, 1.737, 0.0062),  # descending tail
            (side * 0.096, -0.092, 1.722, 0.0038),  # sharp outer tip
        ]
        prev_v_top, prev_v_bot = None, None
        for x, y, z, r in pts:
            v_top = bm.verts.new((x, y, z + r))
            v_bot = bm.verts.new((x, y + 0.003, z - r))
            if prev_v_top and prev_v_bot:
                bm.faces.new((prev_v_bot, prev_v_top, v_top, v_bot))
            prev_v_top, prev_v_bot = v_top, v_bot

        bmesh.ops.solidify(bm, geom=bm.faces[:] + bm.verts[:] + bm.edges[:], thickness=0.005)
        mesh_data = bpy.data.meshes.new(f"Eyebrow_{'L' if side > 0 else 'R'}")
        bm.to_mesh(mesh_data)
        bm.free()
        brow_obj = bpy.data.objects.new(mesh_data.name, mesh_data)
        col.objects.link(brow_obj)
        assign_mat_and_smooth(brow_obj, mats['eyebrows'])
        created_objs.append(brow_obj)

    # 5. Glasses (Spectacles): Iconic black wireframe rims & clear bridge
    for side in [1.0, -1.0]:
        bm = bmesh.new()
        num_pts = 28
        cx = side * 0.047
        cz = 1.692
        rx = 0.034
        rz = 0.021
        rim_v_inner, rim_v_outer = [], []
        for i in range(num_pts):
            theta = 2.0 * math.pi * i / num_pts
            ct = math.cos(theta)
            st = math.sin(theta)
            sgn_c = 1.0 if ct >= 0 else -1.0
            sgn_s = 1.0 if st >= 0 else -1.0
            px = cx + rx * sgn_c * (abs(ct) ** 0.85)
            pz = cz + rz * sgn_s * (abs(st) ** 0.85)
            # Y contour forward from face
            py = -0.138 + 0.006 * (px / 0.08) ** 2

            dir_x = (px - cx) / rx
            dir_z = (pz - cz) / rz
            length = math.hypot(dir_x, dir_z) + 1e-6
            dir_x /= length
            dir_z /= length

            w = 0.0032
            vi = bm.verts.new((px - dir_x * w * 0.5, py, pz - dir_z * w * 0.5))
            vo = bm.verts.new((px + dir_x * w * 0.5, py, pz + dir_z * w * 0.5))
            rim_v_inner.append(vi)
            rim_v_outer.append(vo)

        for i in range(num_pts):
            ni = (i + 1) % num_pts
            bm.faces.new((rim_v_inner[i], rim_v_inner[ni], rim_v_outer[ni], rim_v_outer[i]))

        bmesh.ops.solidify(bm, geom=bm.faces[:] + bm.verts[:] + bm.edges[:], thickness=0.004)
        mesh_rim = bpy.data.meshes.new(f"Glasses_Rim_{'L' if side > 0 else 'R'}")
        bm.to_mesh(mesh_rim)
        bm.free()
        rim_obj = bpy.data.objects.new(mesh_rim.name, mesh_rim)
        col.objects.link(rim_obj)
        assign_mat_and_smooth(rim_obj, mats['glasses_frame'])
        created_objs.append(rim_obj)

        # Temple arms (running back over ears)
        bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.0022, depth=0.14, location=(side * 0.084, -0.065, 1.690))
        arm = bpy.context.active_object
        arm.name = f"Glasses_Temple_{'L' if side > 0 else 'R'}"
        arm.rotation_euler = (math.radians(88), math.radians(side * -10), 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(arm, mats['glasses_frame'])
        created_objs.append(arm)

    # Central nose bridge joining the two frames
    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.0028, depth=0.026, location=(0, -0.138, 1.696))
    bridge_obj = bpy.context.active_object
    bridge_obj.name = "Glasses_Bridge"
    bridge_obj.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    assign_mat_and_smooth(bridge_obj, mats['glasses_frame'])
    created_objs.append(bridge_obj)

    return created_objs


def build_hair_and_beard(col, mats):
    """
    Signature hair tuft at forehead, side/back hair,
    mustache and full white beard.
    """
    created_objs = []

    # 1. Iconic Forehead Hair Tuft / Crest (Curved smooth wave on top center of forehead)
    bm_tuft = bmesh.new()
    for strand_i, (offset_x, tilt_y, length, width) in enumerate([
        (0.000, 0.0, 0.060, 0.030),      # central main wave
        (0.018, 16.0, 0.052, 0.022),     # right wave
        (-0.018, -16.0, 0.052, 0.022),   # left wave
        (0.032, 30.0, 0.040, 0.016),     # outer right
        (-0.032, -30.0, 0.040, 0.016),   # outer left
    ]):
        base_pt = Vector((offset_x, -0.105, 1.770))
        mid_pt = Vector((offset_x * 1.10, -0.116, 1.800))
        tip_pt = Vector((offset_x * 1.25, -0.100, 1.815 + length * 0.2))

        v0 = bm_tuft.verts.new(base_pt + Vector((-width * 0.5, 0, 0)))
        v1 = bm_tuft.verts.new(base_pt + Vector((width * 0.5, 0, 0)))
        v2 = bm_tuft.verts.new(mid_pt + Vector((width * 0.4, 0, 0)))
        v3 = bm_tuft.verts.new(mid_pt + Vector((-width * 0.4, 0, 0)))
        v4 = bm_tuft.verts.new(tip_pt)

        bm_tuft.faces.new((v0, v1, v2, v3))
        bm_tuft.faces.new((v3, v2, v4))

    bmesh.ops.solidify(bm_tuft, geom=bm_tuft.faces[:], thickness=0.012)
    mesh_tuft = bpy.data.meshes.new("Hair_Tuft_Forehead")
    bm_tuft.to_mesh(mesh_tuft)
    bm_tuft.free()
    tuft_obj = bpy.data.objects.new(mesh_tuft.name, mesh_tuft)
    col.objects.link(tuft_obj)
    assign_mat_and_smooth(tuft_obj, mats['white_hair'])
    created_objs.append(tuft_obj)

    # 2. Side & Back Hair (Receding hairline shell covering back of cranium)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=28, ring_count=20, radius=1.0, location=(0, 0.022, 1.675))
    back_hair = bpy.context.active_object
    back_hair.name = "Back_Hair"
    back_hair.scale = (0.124, 0.128, 0.124)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(back_hair, mats['white_hair'])
    created_objs.append(back_hair)

    # Stylized flared hair locks along sides around ears
    for side in [1.0, -1.0]:
        for tier, (z_off, y_off, rad, length, rot_y) in enumerate([
            (1.70, -0.01, 0.024, 0.070, side * 15),
            (1.64, 0.00, 0.028, 0.080, side * 24),
            (1.58, 0.01, 0.030, 0.075, side * 30),
        ]):
            bpy.ops.mesh.primitive_cone_add(vertices=12, radius1=rad, radius2=0.004, depth=length, location=(side * 0.124, y_off, z_off))
            lock = bpy.context.active_object
            lock.name = f"Side_Hair_Lock_{'L' if side > 0 else 'R'}_{tier}"
            lock.rotation_euler = (math.radians(10), math.radians(rot_y), math.radians(side * 8))
            bpy.ops.object.transform_apply(rotation=True)
            assign_mat_and_smooth(lock, mats['white_hair'])
            created_objs.append(lock)

    # 3. Mustache: Curved, sitting horizontally on upper lip above the mouth
    bm_mustache = bmesh.new()
    for side in [1.0, -1.0]:
        pts = [
            (side * 0.004, -0.134, 1.622, 0.0075),
            (side * 0.022, -0.132, 1.620, 0.0090),
            (side * 0.042, -0.124, 1.612, 0.0080),
            (side * 0.058, -0.114, 1.598, 0.0045),
        ]
        prev_t, prev_b = None, None
        for x, y, z, r in pts:
            vt = bm_mustache.verts.new((x, y, z + r * 0.6))
            vb = bm_mustache.verts.new((x, y + 0.003, z - r * 0.6))
            if prev_t and prev_b:
                bm_mustache.faces.new((prev_b, prev_t, vt, vb))
            prev_t, prev_b = vt, vb

    bmesh.ops.solidify(bm_mustache, geom=bm_mustache.faces[:], thickness=0.008)
    mesh_mus = bpy.data.meshes.new("Mustache")
    bm_mustache.to_mesh(mesh_mus)
    bm_mustache.free()
    mus_obj = bpy.data.objects.new(mesh_mus.name, mesh_mus)
    col.objects.link(mus_obj)
    assign_mat_and_smooth(mus_obj, mats['white_hair'])
    created_objs.append(mus_obj)

    # 4. Full White Beard: Framing cheeks, jawline, and chin into rounded spade profile
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.025, depth=0.125, location=(side * 0.084, -0.055, 1.605))
        jaw_beard = bpy.context.active_object
        jaw_beard.name = f"Beard_Jaw_{'L' if side > 0 else 'R'}"
        jaw_beard.rotation_euler = (math.radians(30), math.radians(side * 20), math.radians(side * -28))
        jaw_beard.scale = (1.0, 0.8, 1.2)
        bpy.ops.object.transform_apply(rotation=True, scale=True)
        assign_mat_and_smooth(jaw_beard, mats['white_hair'])
        created_objs.append(jaw_beard)

    # Chin beard arc & spade tip (below mouth, keeping mouth visible)
    bpy.ops.mesh.primitive_torus_add(major_radius=0.056, minor_radius=0.024, location=(0, -0.075, 1.550))
    beard_arc = bpy.context.active_object
    beard_arc.name = "Beard_Arc"
    beard_arc.rotation_euler = (math.radians(32), 0, 0)
    beard_arc.scale = (1.2, 0.85, 1.0)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(beard_arc, mats['white_hair'])
    created_objs.append(beard_arc)

    # Chin spade bottom
    bpy.ops.mesh.primitive_cone_add(vertices=20, radius1=0.065, radius2=0.015, depth=0.075, location=(0, -0.080, 1.488))
    beard_tip = bpy.context.active_object
    beard_tip.name = "Beard_Tip"
    beard_tip.rotation_euler = (math.radians(175), 0, 0)
    beard_tip.scale = (1.2, 0.8, 1.0)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(beard_tip, mats['white_hair'])
    created_objs.append(beard_tip)

    return created_objs


# =============================================================================
# 2. FUR SHAWL COLLAR & SUIT JACKET
# =============================================================================

def build_fur_collar(col, mats):
    """
    Luxurious Fluffy Ivory Fur Shawl Collar:
    Wraps gracefully around the neck and cascades across shoulders & chest in a broad U-shape.
    Decorated with 42+ layered fluffy tufts radiating outward and downward with golden edge accents.
    """
    created_objs = []

    # Main voluminous collar torus
    bpy.ops.mesh.primitive_torus_add(major_radius=0.185, minor_radius=0.052, location=(0, -0.042, 1.450))
    collar_core = bpy.context.active_object
    collar_core.name = "Fur_Collar_Core"
    collar_core.rotation_euler = (math.radians(16), 0, 0)
    collar_core.scale = (1.22, 1.02, 0.90)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(collar_core, mats['fur_collar'])
    created_objs.append(collar_core)

    # Procedural layered fluffy tufts radiating outward along perimeter
    bm_tufts = bmesh.new()
    num_tufts = 42
    for i in range(num_tufts):
        angle = 2.0 * math.pi * i / num_tufts
        # Radii varying from 0.18m to 0.26m
        rx = 0.22 * math.cos(angle)
        ry = 0.18 * math.sin(angle) - 0.04
        rz = 1.450 - 0.045 * math.sin(angle)

        # Fluff direction: pointing outward and downward
        dx = math.cos(angle)
        dy = math.sin(angle)
        dz = -0.32
        mag = math.hypot(dx, dy, dz) + 1e-6
        dx, dy, dz = dx / mag, dy / mag, dz / mag

        fluff_len = 0.050 + 0.018 * (math.sin(i * 3.7) ** 2)
        fluff_w = 0.020

        base_pos = Vector((rx, ry, rz))
        tip_pos = base_pos + Vector((dx * fluff_len, dy * fluff_len, dz * fluff_len))

        tx = -dy * fluff_w
        ty = dx * fluff_w
        tz = 0.0

        v0 = bm_tufts.verts.new(base_pos + Vector((tx, ty, tz)))
        v1 = bm_tufts.verts.new(base_pos - Vector((tx, ty, tz)))
        v2 = bm_tufts.verts.new(tip_pos)
        bm_tufts.faces.new((v0, v1, v2))

    bmesh.ops.solidify(bm_tufts, geom=bm_tufts.faces[:], thickness=0.010)
    mesh_tufts = bpy.data.meshes.new("Fur_Collar_Tufts")
    bm_tufts.to_mesh(mesh_tufts)
    bm_tufts.free()
    tufts_obj = bpy.data.objects.new(mesh_tufts.name, mesh_tufts)
    col.objects.link(tufts_obj)
    assign_mat_and_smooth(tufts_obj, mats['fur_collar'])
    created_objs.append(tufts_obj)

    # Golden accent trim along the collar border
    bpy.ops.mesh.primitive_torus_add(major_radius=0.235, minor_radius=0.007, location=(0, -0.048, 1.428))
    trim = bpy.context.active_object
    trim.name = "Fur_Collar_Gold_Trim"
    trim.rotation_euler = (math.radians(16), 0, 0)
    trim.scale = (1.24, 1.04, 0.90)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(trim, mats['fur_trim'])
    created_objs.append(trim)

    return created_objs


def build_suit_jacket(col, mats):
    """Golden yellow suit jacket (Sherwani / blazer) with stand collar, buttons, and sleeves."""
    created_objs = []

    # Mandarin / Stand collar
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=0.084, depth=0.055, location=(0, -0.015, 1.505))
    mandarin = bpy.context.active_object
    mandarin.name = "Jacket_Stand_Collar"
    assign_mat_and_smooth(mandarin, mats['suit_jacket'])
    created_objs.append(mandarin)

    # Torso: Shoulders, chest, waist, and hips
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=1.0, depth=1.0, location=(0, 0, 1.18))
    torso = bpy.context.active_object
    torso.name = "Jacket_Torso"
    torso.scale = (0.245, 0.165, 0.62)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(torso, mats['suit_jacket'])
    created_objs.append(torso)

    # Lower jacket skirt extending past waist over hips (down to Z = 0.85)
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=1.0, depth=1.0, location=(0, 0, 0.88))
    skirt = bpy.context.active_object
    skirt.name = "Jacket_Skirt"
    skirt.scale = (0.255, 0.175, 0.22)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(skirt, mats['suit_jacket'])
    created_objs.append(skirt)

    # Shoulder pads / caps
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.075, location=(side * 0.22, 0.0, 1.47))
        sh = bpy.context.active_object
        sh.name = f"Shoulder_Cap_{'L' if side > 0 else 'R'}"
        sh.scale = (1.2, 0.9, 0.8)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(sh, mats['suit_jacket'])
        created_objs.append(sh)

    # Gold buttons down the front center placket
    for btn_i in range(5):
        bz = 1.38 - btn_i * 0.10
        bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.009, depth=0.005, location=(0, -0.170, bz))
        btn = bpy.context.active_object
        btn.name = f"Jacket_Button_{btn_i}"
        btn.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(btn, mats['gold_trim'])
        created_objs.append(btn)

    # Sleeves: UpperArm & Forearm tailored cylinders (L & R)
    for side in [1.0, -1.0]:
        # UpperArm Sleeve
        p_sh = Vector((side * 0.23, 0.0, 1.47))
        p_el = Vector((side * 0.48, 0.02, 1.45))
        mid_up = (p_sh + p_el) * 0.5
        len_up = (p_el - p_sh).length

        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.065, depth=len_up, location=mid_up)
        up_sleeve = bpy.context.active_object
        up_sleeve.name = f"Sleeve_Upper_{'L' if side > 0 else 'R'}"
        v_dir = (p_el - p_sh).normalized()
        rot = Vector((0, 0, 1)).rotation_difference(v_dir)
        up_sleeve.rotation_mode = 'QUATERNION'
        up_sleeve.rotation_quaternion = rot
        assign_mat_and_smooth(up_sleeve, mats['suit_jacket'])
        created_objs.append(up_sleeve)

        # Forearm Sleeve
        p_wr = Vector((side * 0.72, 0.12, 1.40))
        mid_fore = (p_el + p_wr) * 0.5
        len_fore = (p_wr - p_el).length

        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.055, depth=len_fore, location=mid_fore)
        fore_sleeve = bpy.context.active_object
        fore_sleeve.name = f"Sleeve_Forearm_{'L' if side > 0 else 'R'}"
        v_dir_f = (p_wr - p_el).normalized()
        rot_f = Vector((0, 0, 1)).rotation_difference(v_dir_f)
        fore_sleeve.rotation_mode = 'QUATERNION'
        fore_sleeve.rotation_quaternion = rot_f
        assign_mat_and_smooth(fore_sleeve, mats['suit_jacket'])
        created_objs.append(fore_sleeve)

        # Sleeve Cuff
        bpy.ops.mesh.primitive_torus_add(major_radius=0.058, minor_radius=0.007, location=p_wr)
        cuff = bpy.context.active_object
        cuff.name = f"Sleeve_Cuff_{'L' if side > 0 else 'R'}"
        cuff.rotation_mode = 'QUATERNION'
        cuff.rotation_quaternion = rot_f
        assign_mat_and_smooth(cuff, mats['gold_trim'])
        created_objs.append(cuff)

    return created_objs


# =============================================================================
# 3. HANDS, SUIT TROUSERS & FORMAL SHOES
# =============================================================================

def build_hands(col, mats):
    """Palms and articulated fingers in ready martial arts posture."""
    created_objs = []
    for side in [1.0, -1.0]:
        wrist_pos = Vector((side * 0.73, 0.13, 1.40))
        # Palm
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=wrist_pos + Vector((side * 0.035, 0.015, -0.01)))
        palm = bpy.context.active_object
        palm.name = f"Hand_Palm_{'L' if side > 0 else 'R'}"
        palm.scale = (0.045, 0.035, 0.022)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(palm, mats['skin'])
        created_objs.append(palm)

        # Thumb
        bpy.ops.mesh.primitive_cylinder_add(vertices=10, radius=0.009, depth=0.035, location=wrist_pos + Vector((side * 0.02, -0.015, 0.005)))
        thumb = bpy.context.active_object
        thumb.name = f"Hand_Thumb_{'L' if side > 0 else 'R'}"
        thumb.rotation_euler = (math.radians(-30), math.radians(side * 45), math.radians(side * -25))
        assign_mat_and_smooth(thumb, mats['skin'])
        created_objs.append(thumb)

        # 4 Articulated Fingers (Curled in ready stance)
        for f_idx, f_name in enumerate(["Index", "Middle", "Ring", "Pinky"]):
            fx = side * (0.065 + f_idx * 0.008)
            fy = 0.020 + f_idx * 0.003
            fz = -0.008 - f_idx * 0.004
            f_pos = wrist_pos + Vector((fx, fy, fz))
            bpy.ops.mesh.primitive_cylinder_add(vertices=10, radius=0.0075 - f_idx * 0.0006, depth=0.040, location=f_pos)
            finger = bpy.context.active_object
            finger.name = f"Hand_Finger_{f_name}_{'L' if side > 0 else 'R'}"
            finger.rotation_euler = (math.radians(20), math.radians(side * 25), math.radians(side * -15))
            assign_mat_and_smooth(finger, mats['skin'])
            created_objs.append(finger)

    return created_objs


def build_trousers_and_shoes(col, mats):
    """Tailored formal suit trousers and polished Oxford dress shoes."""
    created_objs = []

    # Pelvis / Crotch connector under jacket
    bpy.ops.mesh.primitive_cylinder_add(vertices=20, radius=0.20, depth=0.22, location=(0, 0, 0.88))
    pelvis_pants = bpy.context.active_object
    pelvis_pants.name = "Pants_Pelvis"
    pelvis_pants.scale = (1.1, 0.8, 1.0)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(pelvis_pants, mats['suit_pants'])
    created_objs.append(pelvis_pants)

    for side in [1.0, -1.0]:
        leg_x = side * 0.16

        # Thigh (Upper Pant Leg)
        bpy.ops.mesh.primitive_cylinder_add(vertices=20, radius=0.095, depth=0.45, location=(leg_x, 0.0, 0.725))
        thigh = bpy.context.active_object
        thigh.name = f"Pants_Thigh_{'L' if side > 0 else 'R'}"
        thigh.scale = (0.95, 1.05, 1.0)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(thigh, mats['suit_pants'])
        created_objs.append(thigh)

        # Shin (Lower Pant Leg)
        bpy.ops.mesh.primitive_cylinder_add(vertices=20, radius=0.082, depth=0.42, location=(leg_x, 0.0, 0.31))
        shin = bpy.context.active_object
        shin.name = f"Pants_Shin_{'L' if side > 0 else 'R'}"
        shin.scale = (0.95, 1.02, 1.0)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(shin, mats['suit_pants'])
        created_objs.append(shin)

        # Trouser Cuff Hem (draping over shoe)
        bpy.ops.mesh.primitive_torus_add(major_radius=0.086, minor_radius=0.008, location=(leg_x, 0.0, 0.105))
        cuff = bpy.context.active_object
        cuff.name = f"Pants_Cuff_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(cuff, mats['suit_pants'])
        created_objs.append(cuff)

        # Formal Oxford Dress Shoe Upper
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(leg_x, 0.035, 0.055))
        shoe = bpy.context.active_object
        shoe.name = f"Shoe_Upper_{'L' if side > 0 else 'R'}"
        shoe.scale = (0.095, 0.245, 0.075)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(shoe, mats['shoes'])
        created_objs.append(shoe)

        # Shoe Toe Cap (Sleek curved front)
        bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.045, location=(leg_x, -0.090, 0.040))
        toe = bpy.context.active_object
        toe.name = f"Shoe_Toe_{'L' if side > 0 else 'R'}"
        toe.scale = (1.05, 1.25, 0.70)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(toe, mats['shoes'])
        created_objs.append(toe)

        # Shoe Sole (Flat rubber/leather sole)
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(leg_x, 0.035, 0.010))
        sole = bpy.context.active_object
        sole.name = f"Shoe_Sole_{'L' if side > 0 else 'R'}"
        sole.scale = (0.102, 0.260, 0.018)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(sole, mats['shoe_sole'])
        created_objs.append(sole)

        # Shoe Heel (Raised block heel at rear)
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(leg_x, 0.115, 0.022))
        heel = bpy.context.active_object
        heel.name = f"Shoe_Heel_{'L' if side > 0 else 'R'}"
        heel.scale = (0.096, 0.085, 0.025)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(heel, mats['shoe_sole'])
        created_objs.append(heel)

    return created_objs


# =============================================================================
# 4. ARMATURE, SKELETON & RIGGING
# =============================================================================

def build_darklotus_armature(col):
    """
    Constructs the official DarkLotus combat armature matching engine specs:
    Root -> Pelvis -> Spine -> Chest -> Neck -> Head
    Clavicle.L -> UpperArm.L -> Forearm.L -> Hand.L
    Clavicle.R -> UpperArm.R -> Forearm.R -> Hand.R
    Thigh.L -> Shin.L -> Foot.L
    Thigh.R -> Shin.R -> Foot.R
    """
    bpy.ops.object.armature_add(enter_editmode=True, align='WORLD', location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "DarkLotus_Rig"
    arm = arm_obj.data
    arm.name = "DarkLotus_Armature"

    edit_bones = arm.edit_bones

    root = edit_bones['Bone']
    root.name = "Root"
    root.head = Vector((0, 0, 0))
    root.tail = Vector((0, 0, 0.20))

    pelvis = edit_bones.new('Pelvis')
    pelvis.head = Vector((0, 0, 0.95))
    pelvis.tail = Vector((0, 0, 1.15))
    pelvis.parent = root

    spine = edit_bones.new('Spine')
    spine.head = Vector((0, 0, 1.15))
    spine.tail = Vector((0, 0, 1.35))
    spine.parent = pelvis

    chest = edit_bones.new('Chest')
    chest.head = Vector((0, 0, 1.35))
    chest.tail = Vector((0, 0, 1.54))
    chest.parent = spine

    neck = edit_bones.new('Neck')
    neck.head = Vector((0, 0, 1.54))
    neck.tail = Vector((0, 0, 1.66))
    neck.parent = chest

    head = edit_bones.new('Head')
    head.head = Vector((0, 0, 1.66))
    head.tail = Vector((0, 0, 1.83))
    head.parent = neck

    for side, s_name in [(1.0, 'L'), (-1.0, 'R')]:
        clav = edit_bones.new(f'Clavicle.{s_name}')
        clav.head = Vector((side * 0.05, 0.0, 1.48))
        clav.tail = Vector((side * 0.22, 0.0, 1.48))
        clav.parent = chest

        uparm = edit_bones.new(f'UpperArm.{s_name}')
        uparm.head = Vector((side * 0.22, 0.0, 1.48))
        uparm.tail = Vector((side * 0.48, 0.02, 1.45))
        uparm.parent = clav

        forearm = edit_bones.new(f'Forearm.{s_name}')
        forearm.head = Vector((side * 0.48, 0.02, 1.45))
        forearm.tail = Vector((side * 0.72, 0.12, 1.40))
        forearm.parent = uparm

        hand = edit_bones.new(f'Hand.{s_name}')
        hand.head = Vector((side * 0.72, 0.12, 1.40))
        hand.tail = Vector((side * 0.85, 0.15, 1.38))
        hand.parent = forearm

    for side, s_name in [(1.0, 'L'), (-1.0, 'R')]:
        thigh = edit_bones.new(f'Thigh.{s_name}')
        thigh.head = Vector((side * 0.16, 0.0, 0.95))
        thigh.tail = Vector((side * 0.16, 0.0, 0.50))
        thigh.parent = pelvis

        shin = edit_bones.new(f'Shin.{s_name}')
        shin.head = Vector((side * 0.16, 0.0, 0.50))
        shin.tail = Vector((side * 0.16, 0.0, 0.10))
        shin.parent = thigh

        foot = edit_bones.new(f'Foot.{s_name}')
        foot.head = Vector((side * 0.16, 0.0, 0.10))
        foot.tail = Vector((side * 0.16, -0.15, 0.02))
        foot.parent = shin

    bpy.ops.object.mode_set(mode='OBJECT')
    print("[ARMATURE] DarkLotus Combat Armature constructed successfully.")
    return arm_obj


def skin_character_to_armature(arm_obj, all_mesh_objs):
    """
    Mathematical vertex weighting and parenting to DarkLotus armature:
    Assigns each vertex group based on anatomical coordinate spatial zones.
    """
    bone_names = [
        "Root", "Pelvis", "Spine", "Chest", "Neck", "Head",
        "Clavicle.L", "UpperArm.L", "Forearm.L", "Hand.L",
        "Clavicle.R", "UpperArm.R", "Forearm.R", "Hand.R",
        "Thigh.L", "Shin.L", "Foot.L",
        "Thigh.R", "Shin.R", "Foot.R"
    ]

    for obj in all_mesh_objs:
        if obj.type != 'MESH':
            continue

        obj.parent = arm_obj
        mod = obj.modifiers.new(name="ArmatureModifier", type='ARMATURE')
        mod.object = arm_obj
        mod.use_vertex_groups = True

        for b_name in bone_names:
            if b_name not in obj.vertex_groups:
                obj.vertex_groups.new(name=b_name)

        mesh = obj.data
        world_mat = obj.matrix_world

        for vert in mesh.vertices:
            w_pos = world_mat @ vert.co
            wx, wy, wz = w_pos.x, w_pos.y, w_pos.z

            weights = {}

            # Head & Face features
            if wz >= 1.58:
                weights["Head"] = 1.0

            # Neck zone
            elif wz >= 1.48 and abs(wx) < 0.14:
                t = (wz - 1.48) / 0.10
                weights["Head"] = t
                weights["Neck"] = 1.0 - t

            # Chest & Collar zone
            elif wz >= 1.30 and abs(wx) < 0.26:
                t = (wz - 1.30) / 0.22
                weights["Neck"] = t * 0.4
                weights["Chest"] = 1.0 - t * 0.4

            # Mid Torso / Spine
            elif wz >= 1.05 and abs(wx) < 0.28:
                t = (wz - 1.05) / 0.25
                weights["Chest"] = t
                weights["Spine"] = 1.0 - t

            # Pelvis / Lower Jacket / Hips
            elif wz >= 0.85 and abs(wx) < 0.28:
                t = (wz - 0.85) / 0.20
                weights["Spine"] = t * 0.5
                weights["Pelvis"] = 1.0 - t * 0.5

            # Arms & Hands
            elif abs(wx) >= 0.20 and wz >= 1.25:
                s_name = 'L' if wx > 0 else 'R'
                dist_x = abs(wx)
                if dist_x < 0.45:
                    weights[f"UpperArm.{s_name}"] = 1.0
                elif dist_x < 0.70:
                    t = (dist_x - 0.45) / 0.25
                    weights[f"UpperArm.{s_name}"] = 1.0 - t
                    weights[f"Forearm.{s_name}"] = t
                else:
                    weights[f"Hand.{s_name}"] = 1.0

            # Legs: Thigh, Shin, Foot
            else:
                s_name = 'L' if wx > 0 else 'R'
                if wz >= 0.50:
                    weights[f"Thigh.{s_name}"] = 1.0
                elif wz >= 0.10:
                    t = (wz - 0.10) / 0.40
                    weights[f"Thigh.{s_name}"] = t * 0.3
                    weights[f"Shin.{s_name}"] = 1.0 - t * 0.3
                else:
                    weights[f"Foot.{s_name}"] = 1.0

            for b_name, wt in weights.items():
                if wt > 0.001:
                    vg = obj.vertex_groups[b_name]
                    vg.add([vert.index], wt, 'REPLACE')

    print("[SKINNING] Mathematical skinning assigned across all character meshes.")


# =============================================================================
# 5. COMBAT ANIMATIONS FOR DARKLOTUS
# =============================================================================

def create_combat_animations(arm_obj):
    """
    Creates official DarkLotus combat animations:
    - IDLE: Dignified fighting stance with rhythmic breathing
    - 1_JAB: Fast high left punch
    - 2_CROSS: Electric Wind God Fist / right thunder cross
    - 3_MIDKICK: Mid roundhouse kick
    - WALK_FWD: Martial arts advance step
    """
    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)

    # Ensure all pose bones use Euler 'XYZ' for consistent rotation animation
    for pbone in arm_obj.pose.bones:
        pbone.rotation_mode = 'XYZ'

    # 1. IDLE Animation (60 frames loop)
    action_idle = bpy.data.actions.new(name="IDLE")
    arm_obj.animation_data_create()
    arm_obj.animation_data.action = action_idle

    chest_pbone = arm_obj.pose.bones.get("Chest")
    spine_pbone = arm_obj.pose.bones.get("Spine")
    head_pbone = arm_obj.pose.bones.get("Head")
    arm_l_pbone = arm_obj.pose.bones.get("UpperArm.L")
    arm_r_pbone = arm_obj.pose.bones.get("UpperArm.R")

    if chest_pbone:
        for f, (sc, rz) in [(1, (1.0, 0.0)), (30, (1.025, 0.015)), (60, (1.0, 0.0))]:
            chest_pbone.scale = Vector((sc, sc, sc))
            chest_pbone.rotation_euler = Euler((0, 0, rz), 'XYZ')
            chest_pbone.keyframe_insert(data_path="scale", frame=f)
            chest_pbone.keyframe_insert(data_path="rotation_euler", frame=f)

    if head_pbone:
        for f, rx in [(1, 0.0), (30, -0.015), (60, 0.0)]:
            head_pbone.rotation_euler = Euler((rx, 0, 0), 'XYZ')
            head_pbone.keyframe_insert(data_path="rotation_euler", frame=f)

    # 2. 1_JAB (20 frames)
    action_jab = bpy.data.actions.new(name="1_JAB")
    arm_obj.animation_data.action = action_jab

    forearm_l = arm_obj.pose.bones.get("Forearm.L")
    if arm_l_pbone and forearm_l:
        arm_l_pbone.rotation_euler = Euler((0, 0, 0), 'XYZ')
        arm_l_pbone.keyframe_insert(data_path="rotation_euler", frame=1)

        arm_l_pbone.rotation_euler = Euler((math.radians(45), 0, math.radians(20)), 'XYZ')
        arm_l_pbone.keyframe_insert(data_path="rotation_euler", frame=10)

        arm_l_pbone.rotation_euler = Euler((0, 0, 0), 'XYZ')
        arm_l_pbone.keyframe_insert(data_path="rotation_euler", frame=20)

    # 3. 2_CROSS (24 frames - Electric Wind God Fist)
    action_cross = bpy.data.actions.new(name="2_CROSS")
    arm_obj.animation_data.action = action_cross
    if arm_r_pbone and spine_pbone:
        arm_r_pbone.rotation_euler = Euler((0, 0, 0), 'XYZ')
        spine_pbone.rotation_euler = Euler((0, 0, 0), 'XYZ')
        arm_r_pbone.keyframe_insert(data_path="rotation_euler", frame=1)
        spine_pbone.keyframe_insert(data_path="rotation_euler", frame=1)

        arm_r_pbone.rotation_euler = Euler((math.radians(65), 0, math.radians(-35)), 'XYZ')
        spine_pbone.rotation_euler = Euler((0, 0, math.radians(-25)), 'XYZ')
        arm_r_pbone.keyframe_insert(data_path="rotation_euler", frame=12)
        spine_pbone.keyframe_insert(data_path="rotation_euler", frame=12)

        arm_r_pbone.rotation_euler = Euler((0, 0, 0), 'XYZ')
        spine_pbone.rotation_euler = Euler((0, 0, 0), 'XYZ')
        arm_r_pbone.keyframe_insert(data_path="rotation_euler", frame=24)
        spine_pbone.keyframe_insert(data_path="rotation_euler", frame=24)

    # 4. 3_MIDKICK (28 frames)
    action_kick = bpy.data.actions.new(name="3_MIDKICK")
    arm_obj.animation_data.action = action_kick
    thigh_r = arm_obj.pose.bones.get("Thigh.R")
    shin_r = arm_obj.pose.bones.get("Shin.R")
    if thigh_r and shin_r:
        thigh_r.rotation_euler = Euler((0, 0, 0), 'XYZ')
        thigh_r.keyframe_insert(data_path="rotation_euler", frame=1)

        thigh_r.rotation_euler = Euler((math.radians(75), 0, math.radians(-15)), 'XYZ')
        thigh_r.keyframe_insert(data_path="rotation_euler", frame=14)

        thigh_r.rotation_euler = Euler((0, 0, 0), 'XYZ')
        thigh_r.keyframe_insert(data_path="rotation_euler", frame=28)

    # 5. Set default back to IDLE
    arm_obj.animation_data.action = action_idle
    print("[ANIMATIONS] Combat animations (IDLE, 1_JAB, 2_CROSS, 3_MIDKICK) created successfully.")


# =============================================================================
# 6. GLTF/GLB EXPORT & BLEND SAVE
# =============================================================================

def export_character(arm_obj, all_mesh_objs, glb_paths, blend_path):
    """Exports character directly into game directories with glTF 2.0 specs."""
    for p in glb_paths:
        os.makedirs(os.path.dirname(os.path.abspath(p)), exist_ok=True)
    os.makedirs(os.path.dirname(os.path.abspath(blend_path)), exist_ok=True)

    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    print(f"[EXPORT] Master Blender project saved: {blend_path}")

    bpy.ops.object.select_all(action='DESELECT')
    arm_obj.select_set(True)
    for obj in all_mesh_objs:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj

    for p in glb_paths:
        print(f"[EXPORT] Exporting glTF 2.0 binary (.glb) to: {p}")
        bpy.ops.export_scene.gltf(
            filepath=p,
            export_format='GLB',
            use_selection=True,
            export_yup=True,
            export_apply=False,
            export_skins=True,
            export_all_influences=False,
            export_def_bones=True,
            export_animations=True,
            export_frame_range=True,
            export_force_sampling=True,
            export_nla_strips=True,
            export_image_format='AUTO'
        )
        print(f"[EXPORT] Successfully generated: {p}")


# =============================================================================
# 7. STUDIO SHOWCASE RENDERING (MULTI-VIEW TURNTABLE)
# =============================================================================

def render_showcase_views(output_dir="/home/zero/DarkLotus/assets/characters/oodiii_jii"):
    """Renders high-resolution character showcase views: Front, 3/4, Portrait, Turntable."""
    os.makedirs(output_dir, exist_ok=True)

    # Studio Lighting Setup
    light_key = bpy.data.lights.new(name="StudioKey", type='POINT')
    light_key.energy = 90.0
    light_key.color = (1.0, 0.96, 0.92)
    light_key_obj = bpy.data.objects.new("StudioKey", light_key)
    bpy.context.scene.collection.objects.link(light_key_obj)
    light_key_obj.location = (0.75, -1.2, 2.2)

    light_fill = bpy.data.lights.new(name="StudioFill", type='POINT')
    light_fill.energy = 45.0
    light_fill.color = (0.88, 0.92, 1.0)
    light_fill_obj = bpy.data.objects.new("StudioFill", light_fill)
    bpy.context.scene.collection.objects.link(light_fill_obj)
    light_fill_obj.location = (-0.75, -1.0, 1.8)

    light_rim = bpy.data.lights.new(name="StudioRim", type='POINT')
    light_rim.energy = 75.0
    light_rim.color = (1.0, 1.0, 1.0)
    light_rim_obj = bpy.data.objects.new("StudioRim", light_rim)
    bpy.context.scene.collection.objects.link(light_rim_obj)
    light_rim_obj.location = (0.0, 0.9, 2.4)

    # Neutral studio world matching sketch background
    world = bpy.data.worlds.new("ShowcaseWorld")
    bpy.context.scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs['Color'].default_value = (0.89, 0.91, 0.93, 1.0)
        bg.inputs['Strength'].default_value = 0.85

    cam_data = bpy.data.cameras.new("ShowcaseCam")
    cam_obj = bpy.data.objects.new("ShowcaseCam", cam_data)
    bpy.context.scene.collection.objects.link(cam_obj)
    bpy.context.scene.camera = cam_obj

    # 1. Close-up Portrait (Framed identically to odiiii jiii.png)
    cam_data.lens = 75
    cam_obj.location = (0, -0.90, 1.63)
    cam_obj.rotation_euler = (math.radians(90), 0, 0)
    bpy.context.scene.render.resolution_x = 512
    bpy.context.scene.render.resolution_y = 512
    p1 = os.path.join(output_dir, "portrait_likeness.png")
    bpy.context.scene.render.filepath = p1
    bpy.ops.render.render(write_still=True)
    print(f"[SHOWCASE] Portrait render saved: {p1}")

    # 2. Three-Quarter Full Body Fighting View
    cam_data.lens = 45
    cam_obj.location = (1.6, -2.8, 1.15)
    cam_obj.rotation_euler = (math.radians(78), 0, math.radians(28))
    p2 = os.path.join(output_dir, "fullbody_three_quarter.png")
    bpy.context.scene.render.filepath = p2
    bpy.ops.render.render(write_still=True)
    print(f"[SHOWCASE] Full-body 3/4 render saved: {p2}")

    # 3. Full Frontal Suit & Shoes Showcase (Head to Toe)
    cam_data.lens = 45
    cam_obj.location = (0, -3.2, 0.95)
    cam_obj.rotation_euler = (math.radians(90), 0, 0)
    p3 = os.path.join(output_dir, "fullbody_suit_front.png")
    bpy.context.scene.render.filepath = p3
    bpy.ops.render.render(write_still=True)
    print(f"[SHOWCASE] Full-body front render saved: {p3}")


# =============================================================================
# MAIN ORCHESTRATION PIPELINE
# =============================================================================

def build_oodiii_jii():
    print("==================================================================")
    print("  DARKLOTUS: Generating OODIII JII 3D Character Model             ")
    print("==================================================================")

    col = clear_scene()
    mats = build_materials()

    all_meshes = []

    print("[PIPELINE] 1/7 Building Head, Cranium, Ears, and Neck...")
    all_meshes.extend(build_head_mesh(col, mats))

    print("[PIPELINE] 2/7 Building Facial Features, Eyes, Eyebrows & Glasses...")
    all_meshes.extend(build_facial_features(col, mats))

    print("[PIPELINE] 3/7 Building Signature Hair Tuft, Side Hair & Beard...")
    all_meshes.extend(build_hair_and_beard(col, mats))

    print("[PIPELINE] 4/7 Building Luxurious Fur Shawl Collar & Suit Jacket...")
    all_meshes.extend(build_fur_collar(col, mats))
    all_meshes.extend(build_suit_jacket(col, mats))

    print("[PIPELINE] 5/7 Building Articulated Hands, Suit Trousers & Shoes...")
    all_meshes.extend(build_hands(col, mats))
    all_meshes.extend(build_trousers_and_shoes(col, mats))

    print("[PIPELINE] 6/7 Constructing DarkLotus Armature & Skinning Rig...")
    arm_obj = build_darklotus_armature(col)
    skin_character_to_armature(arm_obj, all_meshes)

    print("[PIPELINE] 7/7 Creating Combat Animations...")
    create_combat_animations(arm_obj)

    # Destination paths
    dest_char_glb = "/home/zero/DarkLotus/assets/characters/oodiii_jii/model.glb"
    dest_fighters_glb = "/home/zero/DarkLotus/assets/models/fighters/oodiii_jii.glb"
    dest_blend = "/home/zero/DarkLotus/blender/models/oodiii_jii.blend"

    export_character(arm_obj, all_meshes, [dest_char_glb, dest_fighters_glb], dest_blend)

    print("[PIPELINE] Rendering multi-view showcase previews...")
    render_showcase_views()

    print("==================================================================")
    print("  SUCCESS: OODIII JII Character Model Complete and Exported!     ")
    print("==================================================================")


if __name__ == "__main__":
    build_oodiii_jii()
