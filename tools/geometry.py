# reads a geometry section: the vertices and the triangles of a piece of
# geometry, which a tag keeps in a block of resources apart from the tag; the
# layouts are those of source/geometry/geometry_definitions.h

import struct
from collections.abc import Callable
from dataclasses import dataclass

from cache_file import CacheFile, CacheFileError

SECTION_INFO_SIZE = 0x28
BLOCK_INFO_SIZE = 0x24
RESOURCE_SIZE = 0x10
PART_SIZE = 0x48
VERTEX_BUFFER_SIZE = 0x20

# the types of resource
RESOURCE_TAG_BLOCK = 0
RESOURCE_TAG_DATA = 1
RESOURCE_VERTEX_BUFFER = 2

# offsets in the section of its tag blocks
SECTION_PARTS_OFFSET = 0x00
SECTION_INDICES_OFFSET = 0x20
SECTION_VERTEX_BUFFERS_OFFSET = 0x38

# a part with this flag has a list of triangles, three indices each; a part
# without it has a triangle strip
PART_FLAG_TRIANGLE_LIST = 0x8

# the types of vertex buffer that the tools read, with the bytes of a vertex.
# A position is three reals, or three 16-bit integers if it is compressed, at
# the start of the vertex; the node indices and the node weights of a vertex
# that moves with a skeleton follow them
POSITION_TYPES = {(0x01, 12)}
COMPRESSED_POSITION_TYPES = {(0x02, 6), (0x04, 8), (0x06, 12), (0x08, 12), (0x0A, 16)}

# no level reaches this far from the origin: a position beyond it was read wrong
MAXIMUM_COORDINATE = 100000.0
# two reals, or two 16-bit integers
TEXCOORD_TYPES = {(0x18, 8)}
COMPRESSED_TEXCOORD_TYPES = {(0x19, 4)}


@dataclass
class Part:
    type: int
    flags: int
    material_index: int
    # the indices of the vertices of the part's triangles, three per triangle
    triangles: tuple[int, ...]


@dataclass
class Section:
    parts: list[Part]
    positions: list[tuple[float, float, float]]
    texcoords: list[tuple[float, float]]

    @property
    def triangle_count(self) -> int:
        return sum(len(part.triangles) for part in self.parts) // 3


def strip_to_triangles(strip: tuple[int, ...]) -> tuple[int, ...]:
    """The triangles of a triangle strip: each index makes one with the two before it.

    Every second triangle goes the other way around, and the triangles that
    repeat a vertex only join two pieces of the strip.
    """
    triangles: list[int] = []
    for n in range(len(strip) - 2):
        a, b, c = strip[n : n + 3]
        if a == b or b == c or a == c:
            continue
        triangles.extend((a, c, b) if n & 1 else (a, b, c))
    return tuple(triangles)


def decompress(value: int, low: float, high: float) -> float:
    """A compressed coordinate: the 16-bit integers cover the range of the bounds."""
    return (value + 32768) / 65535 * (high - low) + low


def read_section(
    cache: CacheFile,
    at: Callable[[int, int], bytes],
    section_info: bytes,
    block_info: bytes,
    bounds: tuple[float, ...] | None = None,
) -> Section:
    """Reads the section that a tag's section info and block info describe.

    `at` gives the tag's data at an address. `bounds` is the tag's
    compression info, for the compressed vertices: the least and the greatest
    x, y, z, u and v.
    """
    vertex_count, triangle_count, part_count = struct.unpack_from("<3H", section_info, 0)
    block_offset, block_size, section_size, resource_size, resource_count, resource_address = struct.unpack_from(
        "<4IiI", block_info, 0
    )

    if vertex_count == 0 and block_size == 0:
        return Section([], [], [])

    # the block: 'blkh', the size of the resources, the section, the
    # resources and 'blkf'; the resources start with 'rsrc', and a section
    # with no geometry has none
    block = cache.read_resource(block_offset, block_size)
    resources_start = 8 + section_size
    if (
        block[:4] != b"hklb"
        or block[-4:] != b"fklb"
        or (resource_size > 0 and block[resources_start : resources_start + 4] != b"crsr")
        or resources_start + resource_size + 4 != block_size
    ):
        raise CacheFileError("a geometry block does not have the layout expected")
    if vertex_count == 0:
        return Section([], [], [])
    section = block[8:resources_start]

    # each resource is the data of a field of the section: (type, offset of
    # the field, element size or vertex buffer index, size, offset in the resources)
    resources = list(struct.iter_unpack("<B3x2h2I", at(resource_address, resource_count * RESOURCE_SIZE)))

    def resource(type: int, primary: int, secondary: int | None = None) -> bytes:
        for r_type, r_primary, r_secondary, size, offset in resources:
            if (r_type, r_primary) == (type, primary) and secondary in (None, r_secondary):
                start = resources_start + offset
                if offset + size > resource_size:
                    raise CacheFileError("a geometry resource is outside its block")
                return block[start : start + size]
        return b""

    index_data = resource(RESOURCE_TAG_BLOCK, SECTION_INDICES_OFFSET)
    indices = struct.unpack(f"<{len(index_data) // 2}H", index_data)
    (index_count,) = struct.unpack_from("<i", section, SECTION_INDICES_OFFSET)
    if len(indices) != index_count:
        raise CacheFileError("a geometry section's indices do not match their count")
    if any(index >= vertex_count for index in indices):
        raise CacheFileError("a geometry section has an index beyond its vertices")

    parts = []
    for type, flags, material_index, first_index, part_index_count in struct.iter_unpack(
        f"<5H{PART_SIZE - 10}x", resource(RESOURCE_TAG_BLOCK, SECTION_PARTS_OFFSET)
    ):
        triangle_list = bool(flags & PART_FLAG_TRIANGLE_LIST)
        if first_index + part_index_count > index_count or (triangle_list and part_index_count % 3):
            raise CacheFileError("a geometry part's indices are not inside the section's, or are not whole triangles")
        part_indices = indices[first_index : first_index + part_index_count]
        parts.append(Part(type, flags, material_index, part_indices if triangle_list else strip_to_triangles(part_indices)))
    if len(parts) != part_count:
        raise CacheFileError("a geometry section's parts do not match their count")
    if sum(len(part.triangles) for part in parts) != triangle_count * 3:
        raise CacheFileError("a geometry section's triangles are not as many as its section info says")

    # the vertex buffers: the first byte of each is its type and the second
    # the bytes of a vertex
    positions: list = []
    texcoords: list = []
    definitions = resource(RESOURCE_TAG_BLOCK, SECTION_VERTEX_BUFFERS_OFFSET)
    for buffer_index in range(len(definitions) // VERTEX_BUFFER_SIZE):
        kind = (definitions[buffer_index * VERTEX_BUFFER_SIZE], definitions[buffer_index * VERTEX_BUFFER_SIZE + 1])
        data = resource(RESOURCE_VERTEX_BUFFER, SECTION_VERTEX_BUFFERS_OFFSET, buffer_index)
        if len(data) != vertex_count * kind[1]:
            raise CacheFileError("a vertex buffer's size does not match the vertex count")
        if kind in POSITION_TYPES:
            positions = list(struct.iter_unpack(f"<3f{kind[1] - 12}x", data))
        elif kind in TEXCOORD_TYPES:
            texcoords = list(struct.iter_unpack("<2f", data))
        elif kind in COMPRESSED_POSITION_TYPES or kind in COMPRESSED_TEXCOORD_TYPES:
            if not bounds:
                raise CacheFileError("a geometry section has compressed vertices and its tag has no compression info")
            x0, x1, y0, y1, z0, z1, u0, u1, v0, v1 = bounds[:10]
            if kind in COMPRESSED_POSITION_TYPES:
                positions = [
                    (decompress(x, x0, x1), decompress(y, y0, y1), decompress(z, z0, z1))
                    for x, y, z in struct.iter_unpack(f"<3h{kind[1] - 6}x", data)
                ]
            else:
                texcoords = [(decompress(u, u0, u1), decompress(v, v0, v1)) for u, v in struct.iter_unpack("<2h", data)]
    if len(positions) != vertex_count or len(texcoords) != vertex_count:
        raise CacheFileError("a geometry section has no positions or no texture coordinates of a type that is read")
    # a NaN fails the comparison too
    if not all(abs(coordinate) < MAXIMUM_COORDINATE for position in positions for coordinate in position):
        raise CacheFileError("a geometry section has positions that are not numbers or are too far away")

    return Section(parts, positions, texcoords)
