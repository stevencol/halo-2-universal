# reads the structure bsps of a map for the Python tools; the layouts are those
# of source/scenario/scenario_definitions.h and
# source/structures/structure_bsp_definitions.h

import struct
from collections.abc import Iterator

from cache_file import CacheFile, CacheFileError, group_tag

SCENARIO_STRUCTURE_BSPS_OFFSET = 0x210
SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE = 0x44

# offsets in the structure bsp of its tag blocks
COLLISION_MATERIALS_OFFSET = 0x0C
COLLISION_BSP_OFFSET = 0x14
CLUSTERS_OFFSET = 0x9C
MATERIALS_OFFSET = 0xA4
INSTANCED_GEOMETRY_DEFINITIONS_OFFSET = 0x138
INSTANCED_GEOMETRY_INSTANCES_OFFSET = 0x140

CLUSTER_SIZE = 0xB0
MATERIAL_SIZE = 0x20
INSTANCED_GEOMETRY_DEFINITION_SIZE = 0xC8
INSTANCED_GEOMETRY_INSTANCE_SIZE = 0x58


def obj_name(name: str) -> str:
    """A name with no characters that end a name in an OBJ file."""
    return name.replace("\\", "/").replace(" ", "_")


class StructureBsp:
    """A structure bsp: its block of the cache file, read to the address the scenario gives."""

    def __init__(self, cache: CacheFile, reference: bytes) -> None:
        block_offset, block_size, self.block_address = struct.unpack_from("<3I", reference, 0)
        _, tag_index = struct.unpack_from("<2I", reference, 0x10)
        self.cache = cache
        self.name = cache.tag_name(tag_index)
        self.data = cache.read(block_offset, block_size)

        # cache_file_structure_bsp_header
        _, self.address, _, signature = struct.unpack("<4I", self.at(self.block_address, 16))
        if signature != group_tag("sbsp"):
            raise CacheFileError("the structure bsp block has no 'sbsp' signature")

    def at(self, address: int, size: int) -> bytes:
        offset = address - self.block_address
        if offset < 0 or offset + size > len(self.data):
            raise CacheFileError(f"the structure bsp refers to data outside its block (address {address:#010x})")
        return self.data[offset : offset + size]

    def tag_block(self, offset: int) -> tuple[int, int]:
        """The tag block (count, address) at an offset in the structure bsp."""
        return struct.unpack("<iI", self.at(self.address + offset, 8))

    def elements(self, tag_block: tuple[int, int], format: str) -> list[tuple]:
        count, address = tag_block
        size = struct.calcsize(format)
        return list(struct.iter_unpack(format, self.at(address, count * size))) if count > 0 else []

    def raw_elements(self, tag_block: tuple[int, int], size: int) -> list[bytes]:
        count, address = tag_block
        data = self.at(address, count * size) if count > 0 else b""
        return [data[i * size : (i + 1) * size] for i in range(max(count, 0))]


def structure_bsps(cache: CacheFile) -> Iterator[StructureBsp]:
    scenario = cache.scenario()
    count, address = struct.unpack_from("<iI", scenario, SCENARIO_STRUCTURE_BSPS_OFFSET)
    if count <= 0:
        return
    references = cache.tag_data(address, count * SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE)
    for i in range(count):
        start = i * SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE
        yield StructureBsp(cache, references[start : start + SCENARIO_STRUCTURE_BSP_REFERENCE_SIZE])
