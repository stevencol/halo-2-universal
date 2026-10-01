#!/usr/bin/env python3

# writes the collision geometry of a map's structure bsps as a Wavefront OBJ
# file, with one object per structure bsp and one material per shader
#
# bsp2obj.py <map> <output.obj>
#
# The game's z axis points up. The file has y up, as OBJ readers expect:
# (x, y, z) is written as (x, z, -y).
#
# The layouts are those of source/scenario/scenario_definitions.h,
# source/structures/structure_bsp_definitions.h and
# source/physics/collision_bsp_definitions.h.

import struct
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

from cache_file import CacheFile, CacheFileError, group_tag

SCENARIO_STRUCTURE_BSPS_OFFSET = 0x210
SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE = 0x44
MAXIMUM_EDGES_PER_COLLISION_SURFACE = 8
PLANE_DESIGNATOR_NEGATED_FLAG = 0x8000


@dataclass
class Totals:
    bsps: int = 0
    vertices: int = 0
    surfaces: int = 0
    broken: int = 0
    reversed: int = 0


class Block:
    """A structure bsp's block of the cache file."""

    def __init__(self, data: bytes, address: int) -> None:
        self.data = data
        self.address = address

    def at(self, address: int, size: int) -> bytes:
        offset = address - self.address
        if offset < 0 or offset + size > len(self.data):
            raise CacheFileError(f"the structure bsp refers to data outside its block (address {address:#010x})")
        return self.data[offset : offset + size]

    def elements(self, tag_block: tuple[int, int], format: str) -> list[tuple]:
        count, address = tag_block
        size = struct.calcsize(format)
        return list(struct.iter_unpack(format, self.at(address, count * size))) if count > 0 else []


def obj_name(name: str) -> str:
    return name.replace("\\", "/").replace(" ", "_")


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


def export_structure_bsp(cache: CacheFile, reference: bytes, obj: TextIO, materials_seen: dict, totals: Totals) -> None:
    block_offset, block_size, block_address = struct.unpack_from("<3I", reference, 0)
    _, structure_bsp_index = struct.unpack_from("<2I", reference, 0x10)
    block = Block(cache.read(block_offset, block_size), block_address)

    # cache_file_structure_bsp_header
    _, structure_bsp_address, _, signature = struct.unpack("<4I", block.at(block_address, 16))
    if signature != group_tag("sbsp"):
        raise CacheFileError("the structure bsp block has no 'sbsp' signature")

    # structure_bsp
    structure_bsp = block.at(structure_bsp_address, 0x54)
    collision_materials = struct.unpack_from("<iI", structure_bsp, 0x0C)
    collision_bsp_count, collision_bsp_address = struct.unpack_from("<iI", structure_bsp, 0x14)
    if collision_bsp_count != 1:
        raise CacheFileError(f"the structure bsp has {collision_bsp_count} collision bsps")
    # structure_collision_material: old shader, global material, conveyor surface, shader
    materials = block.elements(collision_materials, "<2I2h2I")

    # collision_bsp: eight tag blocks
    tag_blocks = list(struct.iter_unpack("<iI", block.at(collision_bsp_address, 0x40)))
    planes = block.elements(tag_blocks[1], "<4f")
    surfaces = block.elements(tag_blocks[5], "<2H2Bh")
    edges = block.elements(tag_blocks[6], "<6H")
    vertices = block.elements(tag_blocks[7], "<3f2H")

    name = cache.tag_name(structure_bsp_index)
    obj.write(f"o {obj_name(name)}\n")
    for x, y, z, _, _ in vertices:
        obj.write(f"v {x:.6f} {z:.6f} {-y:.6f}\n")

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
        material = "none" if key is None else obj_name(cache.tag_name(materials[key][5]))
        materials_seen[material] = None
        obj.write(f"usemtl {material}\n")
        for ring in by_material[key]:
            # OBJ indices start at 1 and count the vertices of the whole file
            obj.write("f " + " ".join(str(totals.vertices + vertex + 1) for vertex in ring) + "\n")
        surface_count += len(by_material[key])

    print(f"  {name:<60} {len(vertices):6} vertices {surface_count:6} surfaces")
    totals.bsps += 1
    totals.vertices += len(vertices)
    totals.surfaces += surface_count


def export_map(map_path: Path, obj_path: Path) -> Totals:
    cache = CacheFile(map_path)
    totals = Totals()
    materials_seen: dict[str, None] = {}
    mtl_path = obj_path.with_suffix(".mtl")

    scenario = cache.scenario()
    count, address = struct.unpack_from("<iI", scenario, SCENARIO_STRUCTURE_BSPS_OFFSET)
    references = cache.tag_data(address, count * SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE) if count > 0 else b""

    print(f"{cache.name}: {max(count, 0)} structure bsps")
    with obj_path.open("w", encoding="utf-8", newline="\n") as obj:
        obj.write(f"# {cache.scenario_path}: collision geometry, written by bsp2obj.py\n")
        obj.write(f"mtllib {mtl_path.name}\n")
        for i in range(max(count, 0)):
            reference = references[i * SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE :][:SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE]
            export_structure_bsp(cache, reference, obj, materials_seen, totals)

    with mtl_path.open("w", encoding="utf-8", newline="\n") as mtl:
        for material in materials_seen:
            # a color from the name, to tell the materials apart
            color = zlib.crc32(material.encode())
            red, green, blue = (0.25 + 0.7 * ((color >> shift) & 0xFF) / 255 for shift in (0, 8, 16))
            mtl.write(f"newmtl {material}\nKd {red:.3f} {green:.3f} {blue:.3f}\n\n")

    cache.close()
    return totals


def main() -> None:
    if len(sys.argv) < 3:
        sys.exit("usage: bsp2obj.py <map> <output.obj>")
    try:
        totals = export_map(Path(sys.argv[1]), Path(sys.argv[2]))
    except CacheFileError as error:
        sys.exit(f"bsp2obj: {error}")
    print(f"total vertices     {totals.vertices}")
    print(f"total surfaces     {totals.surfaces}")
    print(f"broken surfaces    {totals.broken}")
    print(f"reversed surfaces  {totals.reversed}")


if __name__ == "__main__":
    main()
