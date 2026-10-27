"""
DARKLOTUS - High-Fidelity OODIII JII Geometry Builder (Iteration 3)
"""
import math
import bpy
import bmesh
from mathutils import Vector, Matrix, Euler

def clear_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    col = bpy.data.collections.new("OodiiiJii")
    scene.collection.children.link(col)
    return col

def get_or_create_mat(name, base_color, metallic=0.0, roughness=0.4, transmission=0.0, ior=1.45, emission_color=None, emission_strength=0.0):
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
    if obj.data.materials:
        obj.data.materials[0] = mat
    else:
        obj.data.materials.append(mat)
    for poly in obj.data.polygons:
        poly.use_smooth = True

def setup_materials():
    mats = {}
    # Warm peach skin matching sketch: #F0C7B1 -> (0.941, 0.780, 0.694)
    mats['skin'] = get_or_create_mat("M_Skin", (0.941, 0.780, 0.694, 1.0), roughness=0.42)
    # White Hair & Beard: Silvery white -> (0.985, 0.985, 1.0)
    mats['white_hair'] = get_or_create_mat("M_WhiteHair", (0.985, 0.985, 1.0, 1.0), roughness=0.35)
    # Dark Eyebrows: Deep charcoal/black -> (0.13, 0.11, 0.13)
    mats['eyebrows'] = get_or_create_mat("M_Eyebrows", (0.13, 0.11, 0.13, 1.0), roughness=0.5)
    # Glasses Frame: Sleek dark acetate/wire -> (0.10, 0.08, 0.08)
    mats['glasses_frame'] = get_or_create_mat("M_GlassesFrame", (0.10, 0.08, 0.08, 1.0), metallic=0.7, roughness=0.25)
    # Glasses Lens: Transparent clear glass
    mats['glasses_lens'] = get_or_create_mat("M_GlassesLens", (0.92, 0.96, 1.0, 0.15), roughness=0.04, transmission=0.92)
    # Eyes
    mats['sclera'] = get_or_create_mat("M_Sclera", (0.96, 0.96, 0.97, 1.0), roughness=0.1)
    mats['iris'] = get_or_create_mat("M_Iris", (0.44, 0.30, 0.20, 1.0), roughness=0.2)
    mats['pupil'] = get_or_create_mat("M_Pupil", (0.02, 0.02, 0.02, 1.0), roughness=0.1)
    mats['catchlight'] = get_or_create_mat("M_Catchlight", (1.0, 1.0, 1.0, 1.0), emission_color=(1.0, 1.0, 1.0, 1.0), emission_strength=2.0)
    # Lips: Warm rose/pink -> (0.855, 0.486, 0.529)
    mats['lips'] = get_or_create_mat("M_Lips", (0.855, 0.486, 0.529, 1.0), roughness=0.35)
    # Fur Collar: Luxurious cream ivory -> (1.0, 0.976, 0.902)
    mats['fur_collar'] = get_or_create_mat("M_FurCollar", (1.0, 0.976, 0.902, 1.0), roughness=0.68)
    # Fur Edge Trim: Golden yellow accent -> (0.969, 0.894, 0.675)
    mats['fur_trim'] = get_or_create_mat("M_FurTrim", (0.969, 0.894, 0.675, 1.0), roughness=0.55)
    # Suit Jacket: Golden Saffron Yellow -> (1.0, 0.741, 0.004)
    mats['suit_jacket'] = get_or_create_mat("M_SuitJacket", (1.0, 0.741, 0.004, 1.0), roughness=0.38)
    # Gold Buttons & Accents -> (1.0, 0.84, 0.15)
    mats['gold_trim'] = get_or_create_mat("M_GoldTrim", (1.0, 0.84, 0.15, 1.0), metallic=0.88, roughness=0.2)
    # Suit Pants: Deep tailored charcoal / midnight
    mats['suit_pants'] = get_or_create_mat("M_SuitPants", (0.11, 0.12, 0.16, 1.0), roughness=0.52)
    # Shoes: Polished black leather
    mats['shoes'] = get_or_create_mat("M_Shoes", (0.04, 0.04, 0.05, 1.0), roughness=0.22, metallic=0.1)
    mats['shoe_sole'] = get_or_create_mat("M_ShoeSole", (0.08, 0.07, 0.06, 1.0), roughness=0.65)
    return mats

def build_head(col, mats):
    """Smooth cranium, forehead, cheeks, neck, and ears."""
    # Cranium: egg-shaped smooth head
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=24, radius=1.0, location=(0, -0.015, 1.68))
    head_obj = bpy.context.active_object
    head_obj.name = "Head_Base"
    head_obj.scale = (0.112, 0.126, 0.138)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(head_obj, mats['skin'])

    # Cheeks / Jaw base to support facial muscles
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=16, radius=1.0, location=(0, -0.045, 1.585))
    jaw = bpy.context.active_object
    jaw.name = "Jaw_Base"
    jaw.scale = (0.098, 0.095, 0.082)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(jaw, mats['skin'])

    # Ears: Left and Right
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_torus_add(major_radius=0.024, minor_radius=0.006, location=(side * 0.118, -0.015, 1.675))
        ear = bpy.context.active_object
        ear.name = f"Ear_{'L' if side > 0 else 'R'}"
        ear.rotation_euler = (math.radians(10), math.radians(side * 12), math.radians(side * 85))
        ear.scale = (1.0, 0.65, 1.4)
        bpy.ops.object.transform_apply(rotation=True, scale=True)
        assign_mat_and_smooth(ear, mats['skin'])

        # Ear lobe & inner concha
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=0.010, location=(side * 0.116, -0.015, 1.652))
        lobe = bpy.context.active_object
        lobe.name = f"Ear_Lobe_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(lobe, mats['skin'])

    # Neck
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=0.072, depth=0.16, location=(0, -0.015, 1.51))
    neck = bpy.context.active_object
    neck.name = "Neck"
    assign_mat_and_smooth(neck, mats['skin'])

def build_face_features(col, mats):
    """Nose, lips, eyes, dark arched eyebrows, glasses."""
    # Nose Tip
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.015, location=(0, -0.134, 1.648))
    nose_tip = bpy.context.active_object
    nose_tip.name = "Nose_Tip"
    nose_tip.scale = (1.05, 0.85, 0.85)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(nose_tip, mats['skin'])

    # Nostril wings
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=0.009, location=(side * 0.015, -0.128, 1.645))
        wing = bpy.context.active_object
        wing.name = f"Nostril_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(wing, mats['skin'])

    # Nose Bridge
    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.009, depth=0.045, location=(0, -0.122, 1.672))
    bridge = bpy.context.active_object
    bridge.name = "Nose_Bridge"
    bridge.rotation_euler = (math.radians(-25), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    assign_mat_and_smooth(bridge, mats['skin'])

    # Lips
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=0.010, location=(0, -0.130, 1.606))
    ulip = bpy.context.active_object
    ulip.name = "Upper_Lip"
    ulip.scale = (2.2, 0.7, 0.45)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(ulip, mats['lips'])

    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=0.012, location=(0, -0.128, 1.594))
    llip = bpy.context.active_object
    llip.name = "Lower_Lip"
    llip.scale = (1.9, 0.8, 0.55)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(llip, mats['lips'])

    # Eyes & Eyeballs
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=20, ring_count=16, radius=0.016, location=(side * 0.046, -0.110, 1.692))
        sclera = bpy.context.active_object
        sclera.name = f"Sclera_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(sclera, mats['sclera'])

        bpy.ops.mesh.primitive_cylinder_add(vertices=20, radius=0.009, depth=0.003, location=(side * 0.046, -0.124, 1.692))
        iris = bpy.context.active_object
        iris.name = f"Iris_{'L' if side > 0 else 'R'}"
        iris.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(iris, mats['iris'])

        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.0045, depth=0.004, location=(side * 0.046, -0.125, 1.692))
        pupil = bpy.context.active_object
        pupil.name = f"Pupil_{'L' if side > 0 else 'R'}"
        pupil.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(pupil, mats['pupil'])

        # Specular glint
        bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=6, radius=0.0028, location=(side * 0.043, -0.1265, 1.696))
        glint = bpy.context.active_object
        glint.name = f"Catchlight_{'L' if side > 0 else 'R'}"
        assign_mat_and_smooth(glint, mats['catchlight'])

    # Eyebrows: Continuous curved bmesh arched band
    for side in [1.0, -1.0]:
        bm = bmesh.new()
        # 5 keypoints along eyebrow arch
        pts = [
            (side * 0.024, -0.125, 1.724, 0.0055),  # inner
            (side * 0.042, -0.122, 1.736, 0.0065),  # rising
            (side * 0.060, -0.114, 1.741, 0.0068),  # peak arch
            (side * 0.078, -0.104, 1.734, 0.0055),  # falling
            (side * 0.092, -0.092, 1.722, 0.0035),  # outer tip
        ]
        # Build 3D quad-strip
        prev_v_top, prev_v_bot = None, None
        for x, y, z, r in pts:
            v_top = bm.verts.new((x, y, z + r))
            v_bot = bm.verts.new((x, y + 0.003, z - r))
            if prev_v_top and prev_v_bot:
                bm.faces.new((prev_v_bot, prev_v_top, v_top, v_bot))
            prev_v_top, prev_v_bot = v_top, v_bot

        # Extrude slightly forward for 3D thickness
        bmesh.ops.solidify(bm, geom=bm.faces[:] + bm.verts[:] + bm.edges[:], thickness=0.004)

        mesh_data = bpy.data.meshes.new(f"Eyebrow_{'L' if side > 0 else 'R'}")
        bm.to_mesh(mesh_data)
        bm.free()
        brow_obj = bpy.data.objects.new(mesh_data.name, mesh_data)
        col.objects.link(brow_obj)
        assign_mat_and_smooth(brow_obj, mats['eyebrows'])

    # Glasses: Left and Right rims
    for side in [1.0, -1.0]:
        bm = bmesh.new()
        # Parametric rounded rectangular / oval rim: 24 points
        num_pts = 24
        cx = side * 0.046
        cz = 1.692
        rx = 0.033
        rz = 0.020
        rim_v_inner, rim_v_outer = [], []
        for i in range(num_pts):
            theta = 2.0 * math.pi * i / num_pts
            # Rounded box formula: cos^3 / sin^3 or soft clamp
            ct = math.cos(theta)
            st = math.sin(theta)
            # Add slight boxiness
            sgn_c = 1.0 if ct >= 0 else -1.0
            sgn_s = 1.0 if st >= 0 else -1.0
            px = cx + rx * sgn_c * (abs(ct) ** 0.85)
            pz = cz + rz * sgn_s * (abs(st) ** 0.85)
            # Y follows face contour
            py = -0.129 + 0.006 * (px / 0.08) ** 2

            # Inner and outer ring vertices
            # Direction outward
            dir_x = (px - cx) / rx
            dir_z = (pz - cz) / rz
            length = math.hypot(dir_x, dir_z) + 1e-6
            dir_x /= length
            dir_z /= length

            w = 0.0028
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

        # Lens inside rim
        bm_lens = bmesh.new()
        lens_verts = []
        for i in range(num_pts):
            theta = 2.0 * math.pi * i / num_pts
            ct = math.cos(theta)
            st = math.sin(theta)
            sgn_c = 1.0 if ct >= 0 else -1.0
            sgn_s = 1.0 if st >= 0 else -1.0
            px = cx + (rx - 0.001) * sgn_c * (abs(ct) ** 0.85)
            pz = cz + (rz - 0.001) * sgn_s * (abs(st) ** 0.85)
            py = -0.129 + 0.006 * (px / 0.08) ** 2
            lens_verts.append(bm_lens.verts.new((px, py, pz)))
        bm_lens.faces.new(lens_verts)
        bmesh.ops.solidify(bm_lens, geom=bm_lens.faces[:], thickness=0.0015)
        mesh_lens = bpy.data.meshes.new(f"Glasses_Lens_{'L' if side > 0 else 'R'}")
        bm_lens.to_mesh(mesh_lens)
        bm_lens.free()
        lens_obj = bpy.data.objects.new(mesh_lens.name, mesh_lens)
        col.objects.link(lens_obj)
        assign_mat_and_smooth(lens_obj, mats['glasses_lens'])

        # Side temples (arms) running back over ears
        bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.0022, depth=0.13, location=(side * 0.082, -0.055, 1.690))
        arm = bpy.context.active_object
        arm.name = f"Glasses_Temple_{'L' if side > 0 else 'R'}"
        arm.rotation_euler = (math.radians(88), math.radians(side * -10), 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(arm, mats['glasses_frame'])

    # Central nose bridge
    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.0025, depth=0.024, location=(0, -0.129, 1.696))
    bridge = bpy.context.active_object
    bridge.name = "Glasses_Bridge"
    bridge.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    assign_mat_and_smooth(bridge, mats['glasses_frame'])

def build_hair_and_beard(col, mats):
    """Iconic hair tuft at forehead, side/back hair, and full white beard & mustache."""
    # 1. Iconic Center Forehead Hair Tuft / Crest
    # In sketch: a white crest that sits on the center of the forehead/hairline
    bm_tuft = bmesh.new()
    # Build layered fan of locks
    for strand_i, (offset_x, tilt_y, length, width) in enumerate([
        (0.000, 0.0, 0.065, 0.026),     # central lock
        (0.016, 18.0, 0.055, 0.020),    # right lock
        (-0.016, -18.0, 0.055, 0.020),  # left lock
        (0.028, 35.0, 0.042, 0.015),    # outer right
        (-0.028, -35.0, 0.042, 0.015),  # outer left
    ]):
        # Curved profile from forehead base to curved tip
        base_pt = Vector((offset_x, -0.108, 1.775))
        mid_pt = Vector((offset_x * 1.15, -0.118, 1.805))
        tip_pt = Vector((offset_x * 1.35, -0.102, 1.825 + length * 0.2))

        # Create tapered triangle/strip
        v0 = bm_tuft.verts.new(base_pt + Vector((-width * 0.5, 0, 0)))
        v1 = bm_tuft.verts.new(base_pt + Vector((width * 0.5, 0, 0)))
        v2 = bm_tuft.verts.new(mid_pt + Vector((width * 0.4, 0, 0)))
        v3 = bm_tuft.verts.new(mid_pt + Vector((-width * 0.4, 0, 0)))
        v4 = bm_tuft.verts.new(tip_pt)

        bm_tuft.faces.new((v0, v1, v2, v3))
        bm_tuft.faces.new((v3, v2, v4))

    bmesh.ops.solidify(bm_tuft, geom=bm_tuft.faces[:], thickness=0.010)
    mesh_tuft = bpy.data.meshes.new("Hair_Tuft_Forehead")
    bm_tuft.to_mesh(mesh_tuft)
    bm_tuft.free()
    tuft_obj = bpy.data.objects.new(mesh_tuft.name, mesh_tuft)
    col.objects.link(tuft_obj)
    assign_mat_and_smooth(tuft_obj, mats['white_hair'])

    # 2. Side & Back Hair (Receding hairline, hair covering temples, ears, back of head)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=28, ring_count=20, radius=1.0, location=(0, 0.020, 1.675))
    back_hair = bpy.context.active_object
    back_hair.name = "Back_Hair"
    back_hair.scale = (0.120, 0.126, 0.122)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(back_hair, mats['white_hair'])

    # Stylized flared hair locks along sides
    for side in [1.0, -1.0]:
        for tier, (z_off, y_off, rad, length, rot_y) in enumerate([
            (1.70, -0.01, 0.024, 0.070, side * 15),
            (1.64, 0.00, 0.028, 0.080, side * 24),
            (1.58, 0.01, 0.030, 0.075, side * 30),
        ]):
            bpy.ops.mesh.primitive_cone_add(vertices=12, radius1=rad, radius2=0.004, depth=length, location=(side * 0.122, y_off, z_off))
            lock = bpy.context.active_object
            lock.name = f"Side_Hair_Lock_{'L' if side > 0 else 'R'}_{tier}"
            lock.rotation_euler = (math.radians(10), math.radians(rot_y), math.radians(side * 8))
            bpy.ops.object.transform_apply(rotation=True)
            assign_mat_and_smooth(lock, mats['white_hair'])

    # 3. Mustache: Curved, sitting on upper lip above the mouth
    # Smooth curved bmesh ribbon
    bm_mustache = bmesh.new()
    for side in [1.0, -1.0]:
        pts = [
            (side * 0.005, -0.133, 1.622, 0.007),
            (side * 0.022, -0.131, 1.620, 0.009),
            (side * 0.040, -0.124, 1.612, 0.008),
            (side * 0.055, -0.114, 1.598, 0.004),
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

    # 4. Full White Beard: Framing the jawline, cheeks, and chin into rounded spade profile
    # Jawline and cheek beard (L & R)
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.024, depth=0.125, location=(side * 0.082, -0.055, 1.605))
        jaw_beard = bpy.context.active_object
        jaw_beard.name = f"Beard_Jaw_{'L' if side > 0 else 'R'}"
        jaw_beard.rotation_euler = (math.radians(30), math.radians(side * 20), math.radians(side * -28))
        jaw_beard.scale = (1.0, 0.8, 1.2)
        bpy.ops.object.transform_apply(rotation=True, scale=True)
        assign_mat_and_smooth(jaw_beard, mats['white_hair'])

    # Chin beard arc & spade tip (below the mouth, keeping mouth clearly visible)
    bpy.ops.mesh.primitive_torus_add(major_radius=0.055, minor_radius=0.024, location=(0, -0.075, 1.555))
    beard_arc = bpy.context.active_object
    beard_arc.name = "Beard_Arc"
    beard_arc.rotation_euler = (math.radians(35), 0, 0)
    beard_arc.scale = (1.2, 0.85, 1.0)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(beard_arc, mats['white_hair'])

    # Chin spade bottom
    bpy.ops.mesh.primitive_cone_add(vertices=20, radius1=0.065, radius2=0.015, depth=0.075, location=(0, -0.080, 1.495))
    beard_tip = bpy.context.active_object
    beard_tip.name = "Beard_Tip"
    beard_tip.rotation_euler = (math.radians(175), 0, 0)
    beard_tip.scale = (1.2, 0.8, 1.0)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(beard_tip, mats['white_hair'])

def build_fur_collar(col, mats):
    """
    Iconic Fluffy Ivory Fur Shawl Collar:
    Wraps gracefully around the neck and cascades over the shoulders and chest.
    Procedurally decorated with 36+ jagged fluffy tufts with golden edge accents.
    """
    # Main voluminous collar torus
    bpy.ops.mesh.primitive_torus_add(major_radius=0.175, minor_radius=0.048, location=(0, -0.040, 1.455))
    collar_core = bpy.context.active_object
    collar_core.name = "Fur_Collar_Core"
    collar_core.rotation_euler = (math.radians(15), 0, 0)
    collar_core.scale = (1.15, 0.95, 0.85)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(collar_core, mats['fur_collar'])

    # Procedural layered fluffy tufts radiating along the outer perimeter
    bm_tufts = bmesh.new()
    num_tufts = 36
    for i in range(num_tufts):
        angle = 2.0 * math.pi * i / num_tufts
        # Focus tufts on front and sides (angle from -135 to +135 deg)
        deg = math.degrees(angle)
        # Radius varying from 0.16m to 0.24m
        rx = 0.20 * math.cos(angle)
        ry = 0.17 * math.sin(angle) - 0.04
        rz = 1.455 - 0.04 * math.sin(angle)

        # Fluff direction: pointing outward and slightly downward
        dx = math.cos(angle)
        dy = math.sin(angle)
        dz = -0.35
        mag = math.hypot(dx, dy, dz) + 1e-6
        dx, dy, dz = dx / mag, dy / mag, dz / mag

        fluff_len = 0.045 + 0.015 * (math.sin(i * 3.5) ** 2)
        fluff_w = 0.018

        # Base and tip
        base_pos = Vector((rx, ry, rz))
        tip_pos = base_pos + Vector((dx * fluff_len, dy * fluff_len, dz * fluff_len))

        # Tangent vector for width
        tx = -dy * fluff_w
        ty = dx * fluff_w
        tz = 0.0

        v0 = bm_tufts.verts.new(base_pos + Vector((tx, ty, tz)))
        v1 = bm_tufts.verts.new(base_pos - Vector((tx, ty, tz)))
        v2 = bm_tufts.verts.new(tip_pos)
        bm_tufts.faces.new((v0, v1, v2))

    bmesh.ops.solidify(bm_tufts, geom=bm_tufts.faces[:], thickness=0.008)
    mesh_tufts = bpy.data.meshes.new("Fur_Collar_Tufts")
    bm_tufts.to_mesh(mesh_tufts)
    bm_tufts.free()
    tufts_obj = bpy.data.objects.new(mesh_tufts.name, mesh_tufts)
    col.objects.link(tufts_obj)
    assign_mat_and_smooth(tufts_obj, mats['fur_collar'])

    # Golden accent trim along the collar perimeter
    bpy.ops.mesh.primitive_torus_add(major_radius=0.220, minor_radius=0.006, location=(0, -0.045, 1.435))
    trim = bpy.context.active_object
    trim.name = "Fur_Collar_Gold_Trim"
    trim.rotation_euler = (math.radians(16), 0, 0)
    trim.scale = (1.18, 0.98, 0.85)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    assign_mat_and_smooth(trim, mats['fur_trim'])

def build_suit_jacket(col, mats):
    """Golden yellow suit jacket (Sherwani / blazer) with stand collar & gold buttons."""
    # Mandarin / Stand collar
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=0.082, depth=0.055, location=(0, -0.015, 1.505))
    mandarin = bpy.context.active_object
    mandarin.name = "Jacket_Stand_Collar"
    assign_mat_and_smooth(mandarin, mats['suit_jacket'])

    # Torso: Shoulders, chest, waist, and hips
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=1.0, depth=1.0, location=(0, 0, 1.18))
    torso = bpy.context.active_object
    torso.name = "Jacket_Torso"
    torso.scale = (0.245, 0.165, 0.62)
    bpy.ops.object.transform_apply(scale=True)
    assign_mat_and_smooth(torso, mats['suit_jacket'])

    # Shoulder pads / caps
    for side in [1.0, -1.0]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.075, location=(side * 0.22, 0.0, 1.47))
        sh = bpy.context.active_object
        sh.name = f"Shoulder_Cap_{'L' if side > 0 else 'R'}"
        sh.scale = (1.2, 0.9, 0.8)
        bpy.ops.object.transform_apply(scale=True)
        assign_mat_and_smooth(sh, mats['suit_jacket'])

    # Gold buttons down the front center placket
    for btn_i in range(5):
        bz = 1.38 - btn_i * 0.10
        bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.009, depth=0.005, location=(0, -0.170, bz))
        btn = bpy.context.active_object
        btn.name = f"Jacket_Button_{btn_i}"
        btn.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        assign_mat_and_smooth(btn, mats['gold_trim'])

def render_portrait(output_path="/tmp/portrait_test3.png"):
    cam_data = bpy.data.cameras.new("TestCam")
    cam_data.lens = 65
    cam_obj = bpy.data.objects.new("TestCam", cam_data)
    bpy.context.scene.collection.objects.link(cam_obj)
    bpy.context.scene.camera = cam_obj
    cam_obj.location = (0, -1.05, 1.60)
    cam_obj.rotation_euler = (math.radians(90), 0, 0)

    # 3-Point Studio Lighting
    light_key = bpy.data.lights.new(name="KeyLight", type='POINT')
    light_key.energy = 85.0
    light_key.color = (1.0, 0.96, 0.92)
    light_key_obj = bpy.data.objects.new("KeyLight", light_key)
    bpy.context.scene.collection.objects.link(light_key_obj)
    light_key_obj.location = (0.55, -0.95, 2.05)

    light_fill = bpy.data.lights.new(name="FillLight", type='POINT')
    light_fill.energy = 45.0
    light_fill.color = (0.88, 0.92, 1.0)
    light_fill_obj = bpy.data.objects.new("FillLight", light_fill)
    bpy.context.scene.collection.objects.link(light_fill_obj)
    light_fill_obj.location = (-0.55, -0.85, 1.65)

    light_rim = bpy.data.lights.new(name="RimLight", type='POINT')
    light_rim.energy = 70.0
    light_rim.color = (1.0, 1.0, 1.0)
    light_rim_obj = bpy.data.objects.new("RimLight", light_rim)
    bpy.context.scene.collection.objects.link(light_rim_obj)
    light_rim_obj.location = (0.0, 0.65, 2.15)

    world = bpy.data.worlds.new("StudioWorld")
    bpy.context.scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs['Color'].default_value = (0.89, 0.91, 0.93, 1.0)
        bg.inputs['Strength'].default_value = 0.85

    bpy.context.scene.render.filepath = output_path
    bpy.context.scene.render.resolution_x = 400
    bpy.context.scene.render.resolution_y = 400
    bpy.ops.render.render(write_still=True)
    print(f"[PORTRAIT] Render complete: {output_path}")

if __name__ == "__main__":
    col = clear_scene()
    mats = setup_materials()
    build_head(col, mats)
    build_face_features(col, mats)
    build_hair_and_beard(col, mats)
    build_fur_collar(col, mats)
    build_suit_jacket(col, mats)
    render_portrait("/tmp/portrait_test3.png")
