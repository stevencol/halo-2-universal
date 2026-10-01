#!/usr/bin/env python3

# writes the geometry of a map's structure bsps as a Wavefront OBJ file, with
# one material per shader
#
# bsp2obj.py <map> <output.obj> [--collision] [--no-textures]
#
# Without --collision the file has the geometry that the game draws, with its
# texture coordinates: one object per cluster and per instance of instanced
# geometry. The diffuse map of each shader is written as a PNG image to a
# folder next to the file, unless --no-textures is given. With --collision
# the file has the geometry that the physics uses: one object per structure
# bsp.
#
# The game's z axis points up. The file has y up, as OBJ readers expect:
# (x, y, z) is written as (x, z, -y).
#
# The layouts are those of source/structures/structure_bsp_definitions.h and
# source/physics/collision_bsp_definitions.h.

import struct
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

import structure_bsp
from bitmap import read_bitmap, shader_diffuse_bitmap
from cache_file import NONE, CacheFile, CacheFileError
from geometry import Section, read_section
from png import write_png
from structure_bsp import StructureBsp, obj_name

MAXIMUM_EDGES_PER_COLLISION_SURFACE = 8
PLANE_DESIGNATOR_NEGATED_FLAG = 0x8000


@dataclass
class Totals:
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
    # the textures written, the materials whose shader has no diffuse map
    # and the materials whose diffuse map could not be read
    textures: int = 0
    untextured: int = 0
    texture_failures: int = 0


class ObjWriter:
    def __init__(self, obj: TextIO) -> None:
        self.obj = obj
        self.vertex_count = 0
        # the materials used, with the index of the shader tag of each
        self.materials: dict[str, int] = {}

    def object(self, name: str) -> None:
        self.obj.write(f"o {obj_name(name)}\n")

    def material(self, name: str, shader_index: int = NONE) -> None:
        self.materials[name] = shader_index
        self.obj.write(f"usemtl {name}\n")

    def vertices(self, positions: list[tuple], texcoords: list[tuple] | None = None) -> int:
        """Writes the vertices and returns the OBJ index of the first one."""
        first = self.vertex_count + 1
        for x, y, z in positions:
            self.obj.write(f"v {x:.6f} {z:.6f} {-y:.6f}\n")
        # the texture coordinates have v down; OBJ has it up
        for u, v in texcoords or ():
            self.obj.write(f"vt {u:.6f} {1 - v:.6f}\n")
        self.vertex_count += len(positions)
        return first

    def write_materials(self, path: Path, textures: dict[str, str]) -> None:
        """Writes the materials; `textures` gives the image of each material that has one."""
        with path.open("w", encoding="utf-8", newline="\n") as mtl:
            for material in self.materials:
                if material in textures:
                    mtl.write(f"newmtl {material}\nKd 1.000 1.000 1.000\nmap_Kd {textures[material]}\n\n")
                    continue
                # a color from the name, to tell the materials apart
                color = zlib.crc32(material.encode())
                red, green, blue = (0.25 + 0.7 * ((color >> shift) & 0xFF) / 255 for shift in (0, 8, 16))
                mtl.write(f"newmtl {material}\nKd {red:.3f} {green:.3f} {blue:.3f}\n\n")


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


def write_section(
    writer: ObjWriter,
    name: str,
    section: Section,
    positions: list[tuple],
    materials: list[tuple[str, int]],
    totals: Totals,
) -> None:
    if not section.parts:
        return
    writer.object(name)
    first = writer.vertices(positions, section.texcoords)

    # the triangles of each material together
    by_material: dict[int, list[int]] = {}
    for part in section.parts:
        by_material.setdefault(part.material_index, []).extend(
            section.indices[part.first_index : part.first_index + part.index_count]
        )
    for material_index in sorted(by_material):
        writer.material(*(materials[material_index] if material_index < len(materials) else ("none", NONE)))
        indices = by_material[material_index]
        for n in range(0, len(indices), 3):
            a, b, c = (first + index for index in indices[n : n + 3])
            writer.obj.write(f"f {a}/{a} {b}/{b} {c}/{c}\n")
        totals.surfaces += len(indices) // 3
    totals.vertices += len(positions)


def export_render(bsp: StructureBsp, writer: ObjWriter, totals: Totals) -> None:
    cache = bsp.cache
    # structure_material: old shader, shader, ...; the name and the shader tag of each
    material_names = [
        (obj_name(cache.tag_name(material[3])), material[3])
        for material in bsp.elements(bsp.tag_block(structure_bsp.MATERIALS_OFFSET), "<4I16x")
    ]
    before = (totals.vertices, totals.surfaces)

    clusters = bsp.raw_elements(bsp.tag_block(structure_bsp.CLUSTERS_OFFSET), structure_bsp.CLUSTER_SIZE)
    for cluster_index, cluster in enumerate(clusters):
        section = read_section(cache, bsp.at, cluster)
        write_section(writer, f"{bsp.name}/cluster_{cluster_index}", section, section.positions, material_names, totals)

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
            sections[definition_index] = read_section(cache, bsp.at, definitions[definition_index])
        section = sections[definition_index]
        positions = [
            (
                px + scale * (x * fx + y * lx + z * ux),
                py + scale * (x * fy + y * ly + z * uy),
                pz + scale * (x * fz + y * lz + z * uz),
            )
            for x, y, z in section.positions
        ]
        write_section(writer, f"{bsp.name}/instance_{instance_index}", section, positions, material_names, totals)

    print(
        f"  {bsp.name:<60} {len(clusters):3} clusters {len(instances):4} instances "
        f"{totals.vertices - before[0]:7} vertices {totals.surfaces - before[1]:7} triangles"
    )
    totals.clusters += len(clusters)
    totals.instances += len(instances)


# ---------- the file


def export_textures(
    cache: CacheFile, materials: dict[str, int], folder: Path, obj_folder: Path, written: dict[str, str | None], totals: Totals
) -> dict[str, str]:
    """Writes the diffuse map of each material's shader as a PNG image in the folder.

    Returns the image of each material that has one, as a path from the OBJ
    file's folder. `written` keeps the bitmaps written already, by tag name,
    so that maps exported to the same folder decode each bitmap once.
    """
    folder.mkdir(parents=True, exist_ok=True)
    textures: dict[str, str] = {}
    for material, shader_index in materials.items():
        bitmap_index = shader_diffuse_bitmap(cache, shader_index)
        if bitmap_index == NONE:
            totals.untextured += 1
            continue
        name = cache.tag_name(bitmap_index)
        if name not in written:
            path = folder / (name.replace("\\", "_").replace(" ", "_") + ".png")
            try:
                image = read_bitmap(cache, bitmap_index)
            except CacheFileError as error:
                print(f"  texture not written: {error}")
                written[name] = None
            else:
                # without the alpha, which is not transparency in most diffuse maps
                rgb = bytes(value for n, value in enumerate(image.pixels) if n % 4 != 3)
                write_png(path, image.width, image.height, rgb, 3)
                written[name] = path.relative_to(obj_folder).as_posix()
                totals.textures += 1
        if written[name]:
            textures[material] = written[name]
        else:
            totals.texture_failures += 1
    return textures


def export_map(
    map_path: Path,
    obj_path: Path,
    collision: bool = False,
    textures: bool = True,
    texture_folder: Path | None = None,
    written_textures: dict[str, str | None] | None = None,
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
        sys.exit("usage: bsp2obj.py <map> <output.obj> [--collision] [--no-textures]")
    try:
        totals = export_map(Path(arguments[0]), Path(arguments[1]), collision, "--no-textures" not in sys.argv)
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
        print(f"textures written   {totals.textures}")
        print(f"no diffuse map     {totals.untextured}")
        print(f"textures not read  {totals.texture_failures}")


if __name__ == "__main__":
    main()
