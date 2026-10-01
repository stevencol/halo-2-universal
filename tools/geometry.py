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

# the types of vertex buffer that the tools read, with the bytes of a vertex
VERTEX_BUFFER_POSITION = (0x01, 12)
VERTEX_BUFFER_TEXCOORD = (0x18, 8)


@dataclass
class Part:
    type: int
    flags: int
    material_index: int
    first_index: int
    index_count: int


@dataclass
class Section:
    parts: list[Part]
    indices: tuple[int, ...]
    positions: list[tuple[float, float, float]]
    texcoords: list[tuple[float, float]]


def read_section(cache: CacheFile, at: Callable[[int, int], bytes], element: bytes) -> Section:
    """Reads the section of a tag element that starts with the section info and the block info.

    `at` gives the tag's data at an address.
    """
    vertex_count, triangle_count, part_count = struct.unpack_from("<3H", element, 0)
    block_offset, block_size, section_size, resource_size, resource_count, resource_address = struct.unpack_from(
        "<4IiI", element, SECTION_INFO_SIZE
    )

    if vertex_count == 0 and block_size == 0:
        return Section([], (), [], [])

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
        return Section([], (), [], [])
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

    parts = [
        Part(type, flags, material_index, first_index, index_count)
        for type, flags, material_index, first_index, index_count in struct.iter_unpack(
            f"<5H{PART_SIZE - 10}x", resource(RESOURCE_TAG_BLOCK, SECTION_PARTS_OFFSET)
        )
    ]
    index_data = resource(RESOURCE_TAG_BLOCK, SECTION_INDICES_OFFSET)
    indices = struct.unpack(f"<{len(index_data) // 2}H", index_data)
    (index_count,) = struct.unpack_from("<i", section, SECTION_INDICES_OFFSET)

    if len(parts) != part_count or len(indices) != index_count:
        raise CacheFileError("a geometry section's parts or indices do not match its counts")
    if index_count != triangle_count * 3:
        raise CacheFileError("a geometry section is not a list of triangles (triangle strips are not read yet)")
    if any(index >= vertex_count for index in indices):
        raise CacheFileError("a geometry section has an index beyond its vertices")
    for part in parts:
        if part.first_index + part.index_count > index_count or part.index_count % 3:
            raise CacheFileError("a geometry part is not a whole number of triangles inside the indices")

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
        if kind == VERTEX_BUFFER_POSITION:
            positions = list(struct.iter_unpack("<3f", data))
        elif kind == VERTEX_BUFFER_TEXCOORD:
            texcoords = list(struct.iter_unpack("<2f", data))
    if len(positions) != vertex_count or len(texcoords) != vertex_count:
        raise CacheFileError("a geometry section has no positions or no texture coordinates of a type that is read")

    return Section(parts, indices, positions, texcoords)
