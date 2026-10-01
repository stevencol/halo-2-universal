# reads a render model (mode) for the Python tools; the layouts are those of
# source/models/render_model_definitions.h

import struct
from dataclasses import dataclass

from cache_file import CacheFile, CacheFileError, group_tag
from geometry import BLOCK_INFO_SIZE, SECTION_INFO_SIZE, Section, read_section
from obj_writer import obj_name

# offsets in the render model of its tag blocks
COMPRESSION_INFO_OFFSET = 0x14
REGIONS_OFFSET = 0x1C
SECTIONS_OFFSET = 0x24
NODES_OFFSET = 0x48
MATERIALS_OFFSET = 0x60

REGION_SIZE = 0x10
PERMUTATION_SIZE = 0x10
SECTION_SIZE = 0x5C
MATERIAL_SIZE = 0x20

# offsets in a section of the render model
SECTION_SECTION_INFO_OFFSET = 0x04
SECTION_BLOCK_INFO_OFFSET = 0x38

LEVELS_OF_DETAIL = 6


@dataclass
class Permutation:
    name: str
    # the section of each level of detail, the least detailed first
    section_indices: tuple[int, ...]


@dataclass
class Region:
    name: str
    permutations: list[Permutation]


class RenderModel:
    def __init__(self, cache: CacheFile, tag_index: int) -> None:
        group, _, self.address, size = cache.tag_instances[tag_index & 0xFFFF]
        if group != group_tag("mode"):
            raise CacheFileError(f"'{cache.tag_name(tag_index)}' is not a render model")
        self.cache = cache
        self.name = cache.tag_name(tag_index)
        data = cache.tag_data(self.address, size)

        def elements(offset: int, format: str) -> list[tuple]:
            count, address = struct.unpack_from("<iI", data, offset)
            item_size = struct.calcsize(format)
            return list(struct.iter_unpack(format, cache.tag_data(address, count * item_size))) if count > 0 else []

        # the least and the greatest x, y, z, u and v of the compressed vertices
        compression = elements(COMPRESSION_INFO_OFFSET, "<10f16x")
        self.bounds = compression[0] if compression else None

        self.regions: list[Region] = []
        for name, _, _, count, address in elements(REGIONS_OFFSET, "<I2hiI"):
            permutations = []
            if count > 0:
                permutations = [
                    Permutation(cache.string(permutation[0]), permutation[1:])
                    for permutation in struct.iter_unpack(
                        f"<I{LEVELS_OF_DETAIL}h", cache.tag_data(address, count * PERMUTATION_SIZE)
                    )
                ]
            self.regions.append(Region(cache.string(name), permutations))

        # a model can be empty
        count, address = struct.unpack_from("<iI", data, SECTIONS_OFFSET)
        section_data = cache.tag_data(address, count * SECTION_SIZE) if count > 0 else b""
        self._section_elements = [section_data[n * SECTION_SIZE : (n + 1) * SECTION_SIZE] for n in range(max(count, 0))]
        self._sections: dict[int, Section] = {}

        # the name and the shader tag of each material
        self.materials = [
            (obj_name(cache.tag_name(material[3])), material[3]) for material in elements(MATERIALS_OFFSET, "<4I16x")
        ]

    @property
    def section_count(self) -> int:
        return len(self._section_elements)

    def section(self, index: int) -> Section:
        if index not in self._sections:
            element = self._section_elements[index]
            self._sections[index] = read_section(
                self.cache,
                self.cache.tag_data,
                element[SECTION_SECTION_INFO_OFFSET : SECTION_SECTION_INFO_OFFSET + SECTION_INFO_SIZE],
                element[SECTION_BLOCK_INFO_OFFSET : SECTION_BLOCK_INFO_OFFSET + BLOCK_INFO_SIZE],
                bounds=self.bounds,
            )
        return self._sections[index]

    def most_detailed_section(self, permutation: Permutation) -> int:
        """The section of the permutation's highest level of detail, or -1."""
        valid = [index for index in permutation.section_indices if 0 <= index < self.section_count]
        return valid[-1] if valid else -1

    def default_sections(self) -> list[tuple[str, Section]]:
        """The model as it is when nothing chooses its form: the first permutation
        of each region, at its highest level of detail. Each with a name."""
        sections = []
        for region in self.regions:
            if not region.permutations:
                continue
            section_index = self.most_detailed_section(region.permutations[0])
            if section_index >= 0:
                sections.append((f"{region.name}/{region.permutations[0].name}", self.section(section_index)))
        return sections
