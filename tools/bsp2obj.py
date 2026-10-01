#!/usr/bin/env python3

# writes the geometry of a map's structure bsps as a Wavefront OBJ file, with
# one material per shader
#
# bsp2obj.py <map> <output.obj> [--collision] [--no-textures] [--no-objects]
#
# Without --collision the file has the geometry that the game draws, with its
# texture coordinates: one object per cluster and per instance of instanced
# geometry, and the objects that the scenario places in the level (scenery,
# machines, crates, vehicles and the like), unless --no-objects is given.
# The diffuse map of each shader is written as a PNG image to a folder next
# to the file, unless --no-textures is given. With --collision the file has
# the geometry that the physics uses: one object per structure bsp.
#
# The layouts are those of source/structures/structure_bsp_definitions.h and
# source/physics/collision_bsp_definitions.h.

import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import structure_bsp
from cache_file import NONE, CacheFile, CacheFileError
from geometry import BLOCK_INFO_SIZE, SECTION_INFO_SIZE, Section, read_section
from obj_writer import ObjWriter, TextureTotals, export_textures, obj_name
from render_model import RenderModel
from scenario_objects import object_render_model, scenario_objects
from structure_bsp import StructureBsp

MAXIMUM_EDGES_PER_COLLISION_SURFACE = 8
PLANE_DESIGNATOR_NEGATED_FLAG = 0x8000


@dataclass
class Totals(TextureTotals):
    bsps: int = 0
    vertices: int = 0
    # collision surfaces, or triangles of the geometry that is drawn
    surfaces: int = 0
    # collision only
    broken: int = 0
    reversed: int = 0
    # the geometry that is drawn only
    clusters: int = 0
    instances: int = 0
    # the objects that the scenario places: those written, those with no
    # model to write, and the triangles of those written
    objects: int = 0
    objects_without_model: int = 0
    object_triangles: int = 0


# ---------- collision


def surface_vertices(surface_index: int, first_edge: int, edges: list[tuple], vertex_count: int) -> list[int]:
    """The vertices of a surface, in order; empty if its edges do not make a ring."""
    ring = []
    edge_index = first_edge
    while True:
        if edge_index >= len(edges) or len(ring) >= MAXIMUM_EDGES_PER_COLLISION_SURFACE:
            return []
        # the surface on the left of an edge goes from the first vertex to the
        # second and continues with the first edge index; the surface on the
        # right goes the other way and continues with the second
        vertex_0, vertex_1, edge_0, edge_1, surface_0, surface_1 = edges[edge_index]
        if surface_0 == surface_index:
            vertex, edge_index = vertex_0, edge_0
        elif surface_1 == surface_index:
            vertex, edge_index = vertex_1, edge_1
        else:
            return []
        if vertex >= vertex_count:
            return []
        ring.append(vertex)
        if edge_index == first_edge:
            return ring


def faces_plane(ring: list[int], vertices: list[tuple], planes: list[tuple], plane_designator: int) -> bool:
    """Whether the vertices go counterclockwise seen from the front of the surface's plane."""
    plane_index = plane_designator & ~PLANE_DESIGNATOR_NEGATED_FLAG
    if plane_index >= len(planes):
        return False
    i = j = k = 0.0
    for n, vertex in enumerate(ring):
        ax, ay, az = vertices[vertex][:3]
        bx, by, bz = vertices[ring[(n + 1) % len(ring)]][:3]
        i += (ay - by) * (az + bz)
        j += (az - bz) * (ax + bx)
        k += (ax - bx) * (ay + by)
    plane = planes[plane_index]
    dot = i * plane[0] + j * plane[1] + k * plane[2]
    return dot < 0 if plane_designator & PLANE_DESIGNATOR_NEGATED_FLAG else dot > 0


def export_collision(bsp: StructureBsp, writer: ObjWriter, totals: Totals) -> None:
    cache = bsp.cache
    # structure_collision_material: old shader, global material, conveyor surface, shader
    materials = bsp.elements(bsp.tag_block(structure_bsp.COLLISION_MATERIALS_OFFSET), "<2I2h2I")
    collision_bsp_count, collision_bsp_address = bsp.tag_block(structure_bsp.COLLISION_BSP_OFFSET)
    if collision_bsp_count != 1:
        raise CacheFileError(f"the structure bsp has {collision_bsp_count} collision bsps")

    # collision_bsp: eight tag blocks
    tag_blocks = list(struct.iter_unpack("<iI", bsp.at(collision_bsp_address, 0x40)))
    planes = bsp.elements(tag_blocks[1], "<4f")
    surfaces = bsp.elements(tag_blocks[5], "<2H2Bh")
    edges = bsp.elements(tag_blocks[6], "<6H")
    vertices = bsp.elements(tag_blocks[7], "<3f2H")

    writer.object(bsp.name)
    first = writer.vertices([vertex[:3] for vertex in vertices])

    # the surfaces of each material together; None is the surfaces with no material
    by_material: dict[int | None, list[list[int]]] = {}
    for surface_index, (plane_designator, first_edge, _, _, material_index) in enumerate(surfaces):
        ring = surface_vertices(surface_index, first_edge, edges, len(vertices))
        if len(ring) < 3:
            totals.broken += 1
            continue
        if not faces_plane(ring, vertices, planes, plane_designator):
            totals.reversed += 1
        key = material_index if 0 <= material_index < len(materials) else None
        by_material.setdefault(key, []).append(ring)

    surface_count = 0
    for key in sorted(by_material, key=lambda k: -1 if k is None else k):
        writer.material("none" if key is None else obj_name(cache.tag_name(materials[key][5])))
        for ring in by_material[key]:
            writer.obj.write("f " + " ".join(str(first + vertex) for vertex in ring) + "\n")
        surface_count += len(by_material[key])

    print(f"  {bsp.name:<60} {len(vertices):6} vertices {surface_count:6} surfaces")
    totals.vertices += len(vertices)
    totals.surfaces += surface_count


# ---------- the geometry that is drawn


def bsp_section(bsp: StructureBsp, element: bytes) -> Section:
    """The section of a cluster or of an instanced geometry definition, which
    start with the section info and the block info."""
    return read_section(
        bsp.cache, bsp.at, element[:SECTION_INFO_SIZE], element[SECTION_INFO_SIZE : SECTION_INFO_SIZE + BLOCK_INFO_SIZE]
    )


def export_render(bsp: StructureBsp, writer: ObjWriter, totals: Totals) -> None:
    cache = bsp.cache
    # structure_material: old shader, shader, ...; the name and the shader tag of each
    materials = [
        (obj_name(cache.tag_name(material[3])), material[3])
        for material in bsp.elements(bsp.tag_block(structure_bsp.MATERIALS_OFFSET), "<4I16x")
    ]
    before = (writer.vertex_count, writer.triangle_count)

    clusters = bsp.raw_elements(bsp.tag_block(structure_bsp.CLUSTERS_OFFSET), structure_bsp.CLUSTER_SIZE)
    for cluster_index, cluster in enumerate(clusters):
        section = bsp_section(bsp, cluster)
        writer.section(f"{bsp.name}/cluster_{cluster_index}", section, section.positions, materials)

    # each definition is read once, when the first instance of it is written
    definitions = bsp.raw_elements(
        bsp.tag_block(structure_bsp.INSTANCED_GEOMETRY_DEFINITIONS_OFFSET),
        structure_bsp.INSTANCED_GEOMETRY_DEFINITION_SIZE,
    )
    sections: dict[int, Section] = {}
    # structure_instanced_geometry_instance: scale, forward, left, up, position, definition, flags, ...
    instances = bsp.elements(bsp.tag_block(structure_bsp.INSTANCED_GEOMETRY_INSTANCES_OFFSET), "<13fhH32x")
    for instance_index, instance in enumerate(instances):
        scale = instance[0]
        fx, fy, fz, lx, ly, lz, ux, uy, uz, px, py, pz = instance[1:13]
        definition_index = instance[13]
        if not 0 <= definition_index < len(definitions):
            raise CacheFileError(f"an instance uses the instanced geometry definition {definition_index}, which is not there")
        if definition_index not in sections:
            sections[definition_index] = bsp_section(bsp, definitions[definition_index])
        section = sections[definition_index]
        positions = [
            (
                px + scale * (x * fx + y * lx + z * ux),
                py + scale * (x * fy + y * ly + z * uy),
                pz + scale * (x * fz + y * lz + z * uz),
            )
            for x, y, z in section.positions
        ]
        writer.section(f"{bsp.name}/instance_{instance_index}", section, positions, materials)

    vertex_count = writer.vertex_count - before[0]
    triangle_count = writer.triangle_count - before[1]
    print(
        f"  {bsp.name:<60} {len(clusters):3} clusters {len(instances):4} instances "
        f"{vertex_count:7} vertices {triangle_count:7} triangles"
    )
    totals.vertices += vertex_count
    totals.surfaces += triangle_count
    totals.clusters += len(clusters)
    totals.instances += len(instances)


def export_objects(cache: CacheFile, writer: ObjWriter, totals: Totals) -> None:
    """Writes the objects that the scenario places, each as its render model in its place."""
    models: dict[int, RenderModel | None] = {}
    before = (writer.vertex_count, writer.triangle_count)
    for number, placement in enumerate(scenario_objects(cache)):
        model_index = object_render_model(cache, placement.tag_index)
        if model_index not in models:
            models[model_index] = RenderModel(cache, model_index) if model_index != NONE else None
        model = models[model_index]
        sections = model.default_sections() if model else []
        if not sections:
            totals.objects_without_model += 1
            continue
        name = f"{placement.kind}_{number}/{cache.tag_name(placement.tag_index)}"
        for section_name, section in sections:
            writer.section(
                f"{name}/{section_name}",
                section,
                [placement.transform(position) for position in section.positions],
                model.materials,
            )
        totals.objects += 1

    print(
        f"  {totals.objects} objects, {totals.objects_without_model} with no model: "
        f"{writer.vertex_count - before[0]} vertices {writer.triangle_count - before[1]} triangles"
    )
    totals.vertices += writer.vertex_count - before[0]
    totals.object_triangles += writer.triangle_count - before[1]


# ---------- the file


def export_map(
    map_path: Path,
    obj_path: Path,
    collision: bool = False,
    textures: bool = True,
    texture_folder: Path | None = None,
    written_textures: dict[str, str | None] | None = None,
    objects: bool = True,
) -> Totals:
    """Writes the OBJ file, its materials and, for the geometry that is drawn, its textures.

    The textures go to a folder next to the OBJ file, or to `texture_folder`,
    which has to be inside the OBJ file's folder.
    """
    cache = CacheFile(map_path)
    totals = Totals()
    mtl_path = obj_path.with_suffix(".mtl")

    with obj_path.open("w", encoding="utf-8", newline="\n") as obj:
        kind = "collision geometry" if collision else "geometry"
        obj.write(f"# {cache.scenario_path}: {kind}, written by bsp2obj.py\n")
        obj.write(f"mtllib {mtl_path.name}\n")
        writer = ObjWriter(obj)
        print(f"{cache.name}:")
        for bsp in structure_bsp.structure_bsps(cache):
            (export_collision if collision else export_render)(bsp, writer, totals)
            totals.bsps += 1
        if objects and not collision:
            export_objects(cache, writer, totals)

    material_textures: dict[str, str] = {}
    if textures and not collision:
        material_textures = export_textures(
            cache,
            writer.materials,
            texture_folder or obj_path.with_name(obj_path.stem + "_textures"),
            obj_path.parent,
            {} if written_textures is None else written_textures,
            totals,
        )
    writer.write_materials(mtl_path, material_textures)

    cache.close()
    return totals


def main() -> None:
    arguments = [argument for argument in sys.argv[1:] if not argument.startswith("--")]
    collision = "--collision" in sys.argv
    if len(arguments) < 2:
        sys.exit("usage: bsp2obj.py <map> <output.obj> [--collision] [--no-textures] [--no-objects]")
    try:
        totals = export_map(
            Path(arguments[0]),
            Path(arguments[1]),
            collision,
            "--no-textures" not in sys.argv,
            objects="--no-objects" not in sys.argv,
        )
    except CacheFileError as error:
        sys.exit(f"bsp2obj: {error}")

    print(f"structure bsps     {totals.bsps}")
    print(f"total vertices     {totals.vertices}")
    if collision:
        print(f"total surfaces     {totals.surfaces}")
        print(f"broken surfaces    {totals.broken}")
        print(f"reversed surfaces  {totals.reversed}")
    else:
        print(f"clusters           {totals.clusters}")
        print(f"instances          {totals.instances}")
        print(f"total triangles    {totals.surfaces}")
        print(f"objects            {totals.objects}")
        print(f"objects, no model  {totals.objects_without_model}")
        print(f"object triangles   {totals.object_triangles}")
        print(f"textures written   {totals.textures}")
        print(f"no diffuse map     {totals.untextured}")
        print(f"textures not read  {totals.texture_failures}")


if __name__ == "__main__":
    main()
