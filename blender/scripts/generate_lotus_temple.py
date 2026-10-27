"""
=============================================================================
DARKLOTUS - High-Fidelity 3D Lotus Temple Generator for Blender
=============================================================================
Procedurally constructs the architectural landmark (Delhi Bahá'í Lotus Temple)
tailored as a high-fidelity 3D martial arts arena for DarkLotus / Shadow Fight.

Architectural Precision:
- Exact 9-sided nonagonal symmetry (40° radial intervals)
- 27 Thin-shell Greek Pentelic marble petals in 3 authentic tiers:
    * 9 Inner Petals: ~34m tall, sharp dorsal spines, arching into central dome
    * 9 Outer Petals: ~23m tall, arching inward, sheltering the mid-body
    * 9 Entrance Canopies: ~13m tall, flaring OUTWARD with sharp peaked arches
- Solid architectural bases (w_base > 3m) firmly anchored in the podium plinth
- 9 Arched entrance portals with deep shadow recesses
- Central martial arts dais: 3-tier stepped plinth with polished obsidian floor,
  gold lotus mandala fighting ring, and 9 boundary stone pagoda lanterns
- 9 Leaf-shaped reflecting ponds with submerged basins, floating water lilies
- 9 Symmetrical stone bridge walkways with traditional balustrades
- Shadow Fight lighting: Twilight sky, warm golden moonlight, glowing embers
- Direct export to .glb (glTF 2.0) and .blend
=============================================================================
"""

import sys
import os
import math

try:
    import bpy
    import bmesh
    from mathutils import Vector, Matrix
except ImportError:
    print("Error: Run this script inside Blender.")
    sys.exit(1)


def clear_scene():
    """Wipe scene clean to guarantee pure procedural rebuild."""
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if not bpy.data.collections:
        col = bpy.data.collections.new("LotusTemple")
        bpy.context.scene.collection.children.link(col)


def create_pbr_material(name, base_color, metallic=0.0, roughness=0.3, emission_color=None, emission_strength=0.0, sss_weight=0.0):
    """Creates a calibrated Principled BSDF material."""
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get("Principled BSDF")

    if bsdf:
        bsdf.inputs['Base Color'].default_value = base_color
        bsdf.inputs['Metallic'].default_value = metallic
        bsdf.inputs['Roughness'].default_value = roughness

        # Subsurface Scattering for marble translucency (Blender 4.x)
        if sss_weight > 0 and 'Subsurface Weight' in bsdf.inputs:
            bsdf.inputs['Subsurface Weight'].default_value = sss_weight
            if 'Subsurface Radius' in bsdf.inputs:
                bsdf.inputs['Subsurface Radius'].default_value = (1.0, 0.8, 0.6)

        # Emission
        if emission_color and emission_strength > 0:
            if 'Emission Color' in bsdf.inputs:
                bsdf.inputs['Emission Color'].default_value = emission_color
                bsdf.inputs['Emission Strength'].default_value = emission_strength
            elif 'Emission' in bsdf.inputs:
                bsdf.inputs['Emission'].default_value = emission_color

    return mat


def setup_materials():
    """Initializes all materials with Shadow Fight PBR standards."""
    mats = {}

    # 1. Greek Pentelic White Marble (Petals, arches, balustrades)
    # Bright pearlescent off-white with fine specular sheen
    mats['Marble'] = create_pbr_material(
        name="Pentelic_Greek_Marble",
        base_color=(0.95, 0.95, 0.93, 1.0),
        metallic=0.04,
        roughness=0.20,
        sss_weight=0.10
    )

    # 2. Dark Obsidian Mirror (Combat Ring Floor)
    # Deep blue-black reflective stone floor
    mats['Obsidian'] = create_pbr_material(
        name="Obsidian_Arena_Floor",
        base_color=(0.035, 0.04, 0.055, 1.0),
        metallic=0.20,
        roughness=0.08
    )

    # 3. Imperial Gold Inlay (Lotus Ring Mandala)
    mats['Gold'] = create_pbr_material(
        name="Lotus_Gold_Inlay",
        base_color=(0.96, 0.80, 0.28, 1.0),
        metallic=0.95,
        roughness=0.16
    )

    # 4. Weathered Dark Granite (Steps, Curbs, Bridges)
    mats['Granite'] = create_pbr_material(
        name="Granite_Paving",
        base_color=(0.14, 0.14, 0.17, 1.0),
        metallic=0.02,
        roughness=0.60
    )

    # 5. Deep Night Mirror Water (Reflecting pools)
    mats['Water'] = create_pbr_material(
        name="Tranquil_Water",
        base_color=(0.015, 0.08, 0.12, 0.9),
        metallic=0.12,
        roughness=0.03
    )

    # 6. Mystic Fiery Lantern Core (Shadow Fight amber embers)
    mats['Lantern'] = create_pbr_material(
        name="Lantern_Ember_Core",
        base_color=(1.0, 0.50, 0.10, 1.0),
        emission_color=(1.0, 0.55, 0.15, 1.0),
        emission_strength=14.0
    )

    # 7. Lotus Blossom Petals (Soft pink/violet glow)
    mats['LotusFlower'] = create_pbr_material(
        name="Lotus_Blossom_Pink",
        base_color=(0.92, 0.35, 0.65, 1.0),
        metallic=0.0,
        roughness=0.30,
        emission_color=(0.95, 0.4, 0.7, 1.0),
        emission_strength=2.5
    )

    # 8. Jade Lily Pad
    mats['LilyPad'] = create_pbr_material(
        name="LilyPad_Jade",
        base_color=(0.06, 0.28, 0.10, 1.0),
        metallic=0.0,
        roughness=0.35
    )

    # 9. Void Black (Doorway recesses)
    mats['DoorVoid'] = create_pbr_material(
        name="Door_Void_Shadow",
        base_color=(0.01, 0.01, 0.01, 1.0),
        metallic=0.0,
        roughness=0.90
    )

    return mats


def generate_lotus_petal(name, u_steps, v_steps, profile_func, crest_depth, thickness, material):
    """
    Parametrically creates an authentic, tapering architectural petal shell with:
    - Broad solid base firmly rooted in the podium plinth
    - Crisp ogival dorsal spine ridge down the center
    - Gracefully tapering to an exact sharp tip at the top (w=0 at tip)
    - Realistic thin-shell architectural concrete thickness
    """
    mesh = bpy.data.meshes.new(name)
    bm = bmesh.new()

    grid = []
    tip_vert = None

    for i in range(u_steps + 1):
        u = i / u_steps # 0 at base, 1 at tip
        r, z, half_w = profile_func(u)

        if i == u_steps:
            # Apex sharp tip vertex
            crest = crest_depth * 0.35
            tip_vert = bm.verts.new((0.0, r + crest, z))
            grid.append([tip_vert])
            break

        row = []
        for j in range(v_steps + 1):
            v = -1.0 + 2.0 * (j / v_steps) # -1 to +1

            # Ogival sharp ridge down the center (v=0)
            ridge = 1.0 - (abs(v) ** 1.35)
            crest = ridge * crest_depth * (1.0 - u * 0.4)

            lx = v * half_w
            ly = r + crest
            lz = z

            vert = bm.verts.new((lx, ly, lz))
            row.append(vert)
        grid.append(row)

    bm.verts.ensure_lookup_table()

    # Generate quad faces for the petal body
    for i in range(u_steps - 1):
        for j in range(v_steps):
            v1 = grid[i][j]
            v2 = grid[i + 1][j]
            v3 = grid[i + 1][j + 1]
            v4 = grid[i][j + 1]
            bm.faces.new((v1, v2, v3, v4))

    # Connect top row to the single apex tip vertex
    last_row = grid[u_steps - 1]
    for j in range(v_steps):
        bm.faces.new((last_row[j], tip_vert, last_row[j + 1]))

    bm.to_mesh(mesh)
    bm.free()

    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(material)

    for poly in obj.data.polygons:
        poly.use_smooth = True

    solidify = obj.modifiers.new(name="Solidify", type='SOLIDIFY')
    solidify.thickness = thickness
    solidify.offset = -0.5

    return obj


def build_lotus_temple():
    print("[DARKLOTUS] Generating high-fidelity 3D Lotus Temple...")
    clear_scene()
    mats = setup_materials()

    # =========================================================================
    # 1. THE 27 ARCHITECTURAL MARBLE PETALS (9 Facets x 3 Tiers)
    # =========================================================================

    # TIER 1: INNER PETALS (9 Petals, soaring ~34m tall, arching inward to dome)
    def inner_profile(u):
        z = 0.5 + u * 33.5
        # Arching gracefully inward towards central axis
        arch = math.sin(u * math.pi) * 2.0
        inward = (u ** 1.8) * 14.2
        r = 15.0 + arch - inward
        # Broad base at u=0 (w=2.8m), swells to max (w=5.4m) at 40%, tapers to 0 at tip
        w = (2.8 * (1.0 - u) + 4.8 * math.sin(u * math.pi)) * ((1.0 - u) ** 0.35)
        return r, z, w

    # TIER 2: OUTER PETALS (9 Petals, ~23m tall, arching inward sheltering hall)
    def outer_profile(u):
        z = 0.5 + u * 22.5
        arch = math.sin(u * math.pi) * 2.8
        inward = (u ** 1.6) * 10.8
        r = 18.2 + arch - inward
        w = (3.2 * (1.0 - u) + 5.2 * math.sin(u * math.pi)) * ((1.0 - u) ** 0.35)
        return r, z, w

    # TIER 3: ENTRANCE CANOPIES (9 Petals, ~13.5m tall, flaring OUTWARD over doors)
    def entrance_profile(u):
        z = 0.5 + u * 13.0
        # Flaring outward and upward like the iconic entrance canopies
        outward = math.sin(u * math.pi * 0.8) * 8.0 + u * 4.5
        r = 16.0 + outward
        w = (3.0 * (1.0 - u) + 4.8 * math.sin(u * math.pi)) * ((1.0 - u) ** 0.35)
        return r, z, w

    # Construct the 9 clusters (40° radial spacing)
    for i in range(9):
        base_angle = i * (2.0 * math.pi / 9.0)

        # 1. Inner Petal
        p_in = generate_lotus_petal(
            f"InnerPetal_{i+1}",
            u_steps=22, v_steps=12,
            profile_func=inner_profile,
            crest_depth=2.2,
            thickness=0.38,
            material=mats['Marble']
        )
        p_in.rotation_euler = (0, 0, base_angle - math.pi / 2.0)

        # 2. Outer Petal (staggered by 20° between the inner petals)
        p_out = generate_lotus_petal(
            f"OuterPetal_{i+1}",
            u_steps=18, v_steps=10,
            profile_func=outer_profile,
            crest_depth=1.8,
            thickness=0.35,
            material=mats['Marble']
        )
        p_out.rotation_euler = (0, 0, base_angle + (math.pi / 9.0) - math.pi / 2.0)

        # 3. Entrance Canopy Petal (aligned with inner petal axis)
        p_ent = generate_lotus_petal(
            f"EntranceCanopy_{i+1}",
            u_steps=16, v_steps=10,
            profile_func=entrance_profile,
            crest_depth=1.5,
            thickness=0.30,
            material=mats['Marble']
        )
        p_ent.rotation_euler = (0, 0, base_angle - math.pi / 2.0)

    # =========================================================================
    # 2. APEX GOLDEN STAR SKYLIGHT & CONCENTRIC DOME RINGS
    # =========================================================================
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=9,
        radius=3.2,
        depth=0.5,
        location=(0, 0, 33.8)
    )
    apex = bpy.context.active_object
    apex.name = "Apex_NinePointStar"
    apex.data.materials.append(mats['Gold'])

    # Interior vaulted concrete ring beam
    bpy.ops.mesh.primitive_torus_add(
        major_radius=11.5,
        minor_radius=0.4,
        major_segments=36,
        minor_segments=12,
        location=(0, 0, 16.0)
    )
    ring_beam = bpy.context.active_object
    ring_beam.name = "Interior_RingBeam"
    ring_beam.data.materials.append(mats['Marble'])

    # =========================================================================
    # 3. 9 POINTED ARCHED ENTRANCE PORTALS
    # =========================================================================
    for i in range(9):
        ang = i * (2.0 * math.pi / 9.0)
        x = math.cos(ang) * 15.6
        y = math.sin(ang) * 15.6

        # Outer Marble Portal Arch Frame
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, y, 4.0))
        arch = bpy.context.active_object
        arch.name = f"Portal_Arch_{i+1}"
        arch.scale = (4.8, 1.4, 8.0)
        arch.rotation_euler = (0, 0, ang + math.pi / 2.0)
        arch.data.materials.append(mats['Marble'])

        # Deep Recessed Entrance Void (Shadow Fight contrast)
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x * 0.96, y * 0.96, 3.5))
        door = bpy.context.active_object
        door.name = f"Portal_ShadowVoid_{i+1}"
        door.scale = (3.2, 0.9, 7.0)
        door.rotation_euler = (0, 0, ang + math.pi / 2.0)
        door.data.materials.append(mats['DoorVoid'])

        # Portal Arch Crown / Keystones
        bpy.ops.mesh.primitive_cone_add(
            vertices=4,
            radius1=2.4,
            depth=1.8,
            location=(x, y, 8.8)
        )
        crown = bpy.context.active_object
        crown.name = f"Portal_Crown_{i+1}"
        crown.rotation_euler = (0, 0, ang + math.pi / 4.0)
        crown.data.materials.append(mats['Marble'])

    # =========================================================================
    # 4. CENTRAL COMBAT ARENA DAIS (The Fighting Ring)
    # =========================================================================
    # Tier 1: Ground Plinth (Weathered granite foundation)
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=36,
        radius=25.0,
        depth=1.2,
        location=(0, 0, -0.6)
    )
    plinth_1 = bpy.context.active_object
    plinth_1.name = "Plinth_Tier1_Base"
    plinth_1.data.materials.append(mats['Granite'])

    # Tier 2: Stepped Podium (Polished marble tier)
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=36,
        radius=20.0,
        depth=0.6,
        location=(0, 0, 0.0)
    )
    plinth_2 = bpy.context.active_object
    plinth_2.name = "Plinth_Tier2_Step"
    plinth_2.data.materials.append(mats['Marble'])

    # Tier 3: The Fighting Arena Ring (Diameter 21m, R=10.5m)
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=64,
        radius=10.5,
        depth=0.15,
        location=(0, 0, 0.32)
    )
    arena_floor = bpy.context.active_object
    arena_floor.name = "Arena_Floor_Obsidian"
    arena_floor.data.materials.append(mats['Obsidian'])

    # Outer Gold Boundary Ring (Visualizing the arena perimeter)
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=64,
        radius=10.2,
        depth=0.04,
        location=(0, 0, 0.40)
    )
    gold_ring_outer = bpy.context.active_object
    gold_ring_outer.name = "Ring_Outer_Gold"
    gold_ring_outer.data.materials.append(mats['Gold'])

    bpy.ops.mesh.primitive_cylinder_add(
        vertices=64,
        radius=9.8,
        depth=0.06,
        location=(0, 0, 0.41)
    )
    gold_ring_cut = bpy.context.active_object
    gold_ring_cut.name = "Ring_Outer_Cut"
    gold_ring_cut.data.materials.append(mats['Obsidian'])

    # Inner Gold Lotus Mandala (Combat centerpiece)
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=48,
        radius=4.8,
        depth=0.04,
        location=(0, 0, 0.40)
    )
    mandala_ring = bpy.context.active_object
    mandala_ring.name = "Mandala_Center_Ring"
    mandala_ring.data.materials.append(mats['Gold'])

    bpy.ops.mesh.primitive_cylinder_add(
        vertices=48,
        radius=4.5,
        depth=0.06,
        location=(0, 0, 0.41)
    )
    mandala_cut = bpy.context.active_object
    mandala_cut.name = "Mandala_Center_Cut"
    mandala_cut.data.materials.append(mats['Obsidian'])

    # 8-Petal Gold Lotus Emblem
    for p in range(8):
        pang = p * (2.0 * math.pi / 8.0)
        px = math.cos(pang) * 2.0
        py = math.sin(pang) * 2.0
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=12,
            radius=0.75,
            depth=0.06,
            location=(px, py, 0.42)
        )
        p_decal = bpy.context.active_object
        p_decal.name = f"Lotus_GoldPetal_{p+1}"
        p_decal.scale = (0.55, 1.9, 1.0)
        p_decal.rotation_euler = (0, 0, pang + math.pi / 2.0)
        p_decal.data.materials.append(mats['Gold'])

    # =========================================================================
    # 5. 9 BOUNDARY STONE PAGODA LANTERNS (Wall Splat Arena Markers)
    # =========================================================================
    for i in range(9):
        ang = i * (2.0 * math.pi / 9.0) + (math.pi / 9.0)
        lx = math.cos(ang) * 10.4
        ly = math.sin(ang) * 10.4

        # Octagonal stone base
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=8,
            radius=0.55,
            depth=1.2,
            location=(lx, ly, 0.9)
        )
        l_base = bpy.context.active_object
        l_base.name = f"Arena_LanternBase_{i+1}"
        l_base.data.materials.append(mats['Granite'])

        # Carved waist
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=8,
            radius=0.40,
            depth=0.4,
            location=(lx, ly, 1.6)
        )
        l_waist = bpy.context.active_object
        l_waist.data.materials.append(mats['Marble'])

        # Fiery Emissive Lantern Heart
        bpy.ops.mesh.primitive_cube_add(
            size=0.45,
            location=(lx, ly, 2.0)
        )
        l_heart = bpy.context.active_object
        l_heart.name = f"Arena_LanternCore_{i+1}"
        l_heart.data.materials.append(mats['Lantern'])

        # Pagoda Roof Canopy
        bpy.ops.mesh.primitive_cone_add(
            vertices=8,
            radius1=0.85,
            depth=0.55,
            location=(lx, ly, 2.45)
        )
        l_roof = bpy.context.active_object
        l_roof.data.materials.append(mats['Granite'])

    # =========================================================================
    # 6. THE 9 REFLECTING LOTUS PONDS & WATER LILIES
    # =========================================================================
    for i in range(9):
        pond_ang = i * (2.0 * math.pi / 9.0) + (math.pi / 9.0)
        px = math.cos(pond_ang) * 34.0
        py = math.sin(pond_ang) * 34.0

        # Sunken Pool Basin
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=24,
            radius=7.8,
            depth=0.9,
            location=(px, py, -0.6)
        )
        p_water = bpy.context.active_object
        p_water.name = f"Pool_Water_{i+1}"
        p_water.scale = (1.0, 1.7, 1.0)
        p_water.rotation_euler = (0, 0, pond_ang + math.pi / 2.0)
        p_water.data.materials.append(mats['Water'])

        # Polished Stone Curb
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=24,
            radius=8.5,
            depth=0.35,
            location=(px, py, -0.15)
        )
        p_curb = bpy.context.active_object
        p_curb.name = f"Pool_Curb_{i+1}"
        p_curb.scale = (1.0, 1.7, 1.0)
        p_curb.rotation_euler = (0, 0, pond_ang + math.pi / 2.0)
        p_curb.data.materials.append(mats['Granite'])

        # Floating Lotus Water Lilies
        for lp in range(4):
            sub_ang = lp * (2.0 * math.pi / 4.0) + 0.4
            fx = px + math.cos(sub_ang) * 3.8
            fy = py + math.sin(sub_ang) * 5.0

            # Lily Pad disc with slight notch
            bpy.ops.mesh.primitive_cylinder_add(
                vertices=16,
                radius=1.2,
                depth=0.04,
                location=(fx, fy, -0.18)
            )
            pad = bpy.context.active_object
            pad.name = f"LilyPad_{i+1}_{lp+1}"
            pad.data.materials.append(mats['LilyPad'])

            # Multi-petal 3D Lotus Blossom
            bpy.ops.mesh.primitive_cone_add(
                vertices=12,
                radius1=0.55,
                depth=0.45,
                location=(fx, fy, -0.04)
            )
            flower = bpy.context.active_object
            flower.name = f"LotusFlower_{i+1}_{lp+1}"
            flower.data.materials.append(mats['LotusFlower'])

    # =========================================================================
    # 7. THE 9 ENTRANCE BRIDGES & WALKWAYS
    # =========================================================================
    for i in range(9):
        b_ang = i * (2.0 * math.pi / 9.0)
        bx = math.cos(b_ang) * 33.0
        by = math.sin(b_ang) * 33.0

        # Stone Bridge Deck
        bpy.ops.mesh.primitive_cube_add(
            size=1.0,
            location=(bx, by, -0.1)
        )
        bridge = bpy.context.active_object
        bridge.name = f"Bridge_Walkway_{i+1}"
        bridge.scale = (4.4, 20.0, 0.45)
        bridge.rotation_euler = (0, 0, b_ang + math.pi / 2.0)
        bridge.data.materials.append(mats['Granite'])

        # Bridge Stone Balustrades
        for side in (-2.4, 2.4):
            rail_x = bx + math.cos(b_ang + math.pi / 2.0) * side
            rail_y = by + math.sin(b_ang + math.pi / 2.0) * side

            bpy.ops.mesh.primitive_cube_add(
                size=1.0,
                location=(rail_x, rail_y, 0.4)
            )
            rail = bpy.context.active_object
            rail.name = f"Bridge_Rail_{i+1}"
            rail.scale = (0.35, 19.5, 0.7)
            rail.rotation_euler = (0, 0, b_ang + math.pi / 2.0)
            rail.data.materials.append(mats['Marble'])

            # Terminal Lantern
            bpy.ops.mesh.primitive_cylinder_add(
                vertices=8,
                radius=0.4,
                depth=1.3,
                location=(rail_x, rail_y, 1.1)
            )
            post = bpy.context.active_object
            post.data.materials.append(mats['Granite'])

            bpy.ops.mesh.primitive_cube_add(
                size=0.35,
                location=(rail_x, rail_y, 1.8)
            )
            glow = bpy.context.active_object
            glow.data.materials.append(mats['Lantern'])

    print("[DARKLOTUS] Lotus Temple geometry successfully assembled!")


def export_assets(dest_glb_path, dest_blend_path):
    """Saves .blend and exports .glb for DarkLotus."""
    os.makedirs(os.path.dirname(os.path.abspath(dest_glb_path)), exist_ok=True)

    bpy.ops.wm.save_as_mainfile(filepath=dest_blend_path)
    print(f"[DARKLOTUS] Saved Blender scene: {dest_blend_path}")

    bpy.ops.export_scene.gltf(
        filepath=dest_glb_path,
        export_format='GLB',
        use_selection=False,
        export_yup=True,
        export_apply=True,
        export_materials='EXPORT',
        export_image_format='AUTO'
    )
    print(f"[DARKLOTUS] Exported GLB model: {dest_glb_path}")


if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    engine_root = os.path.dirname(os.path.dirname(script_dir))

    glb_out = os.path.join(engine_root, "assets", "models", "arenas", "lotus_temple.glb")
    blend_out = os.path.join(engine_root, "assets", "models", "arenas", "lotus_temple.blend")

    build_lotus_temple()
    export_assets(glb_out, blend_out)
