# reads a Halo 2 cache file for the Python tools; the layouts are those of
# source/cache/cache_files.h and source/tag_files/tag_groups.h

import struct
from pathlib import Path

HEADER_SIZE = 0x800
VERSION = 8
SCENARIO_TYPES = ("solo", "multiplayer", "main menu", "shared", "single player shared")
# source/cache/cache_files.c, cache_file_maximum_length
MAXIMUM_LENGTHS = (0x11800000, 0x05000000, 0x05000000, 0x0B400000, 0x20800000)
NONE = 0xFFFFFFFF
# the files that hold resources, by the two high bits of a resource's offset; 0 is the map itself
RESOURCE_FILES = {1: "mainmenu.map", 2: "shared.map", 3: "single_player_shared.map"}


def group_tag(text: str) -> int:
    return int.from_bytes(text.encode("latin-1"), "big")


def group_tag_to_string(tag: int) -> str:
    return tag.to_bytes(4, "big").decode("latin-1")


class CacheFileError(Exception):
    pass


class CacheFile:
    def __init__(self, path: Path) -> None:
        self.path = Path(path)
        self.file = self.path.open("rb")
        self._shared: dict = {}

        header = self.file.read(HEADER_SIZE)
        self.header = header
        signature, self.version, self.file_length = struct.unpack_from("<I2i", header, 0)
        self.tag_data_offset, self.tag_index_size, self.tag_data_size, self.tag_memory_size = struct.unpack_from(
            "<4I", header, 0x10
        )
        self.build = self._string(header, 0x120, 32)
        (scenario_type,) = struct.unpack_from("<h", header, 0x140)
        self.name = self._string(header, 0x198, 32)
        self.scenario_path = self._string(header, 0x1BC, 256)
        tag_name_count, tag_name_data_offset, tag_name_data_size, tag_name_index_offset = struct.unpack_from(
            "<4I", header, 0x2C0
        )
        (footer,) = struct.unpack_from("<I", header, 0x7FC)

        # source/cache/cache_files.c, cache_file_header_verify
        if (
            len(header) != HEADER_SIZE
            or signature != group_tag("head")
            or footer != group_tag("foot")
            or not 0 <= scenario_type < len(SCENARIO_TYPES)
            or not 0 <= self.file_length <= MAXIMUM_LENGTHS[scenario_type]
            or b"\0" not in header[0x198 : 0x198 + 32]
            or self.version != VERSION
        ):
            raise CacheFileError(f"'{self.path}' does not appear to be a Halo 2 cache file")
        self.scenario_type = SCENARIO_TYPES[scenario_type]

        self.tag_index = self.read(self.tag_data_offset, self.tag_index_size)
        (
            tag_groups_address,
            self.tag_group_count,
            tag_instances_address,
            self.scenario_index,
            self.globals_index,
            _,
            self.tag_count,
            tag_signature,
        ) = struct.unpack_from("<IiIIIIiI", self.tag_index, 0)
        if tag_signature != group_tag("tags") or self.tag_count <= 0:
            raise CacheFileError("the tag header is not valid")

        # the tag groups follow the tag header
        self.base_address = tag_groups_address - 0x20
        offset = tag_instances_address - self.base_address
        if offset < 0 or offset + self.tag_count * 16 > self.tag_index_size:
            raise CacheFileError("the tag instances are outside the tag index")
        # (group tag, tag index, address, size) per tag
        self.tag_instances = list(struct.iter_unpack("<4I", self.tag_index[offset : offset + self.tag_count * 16]))
        # (group tag, parent, parent of the parent) per tag group
        self.tag_groups = list(struct.iter_unpack("<3I", self.tag_index[0x20 : 0x20 + self.tag_group_count * 12]))
        self.string_id_count = struct.unpack_from("<i", header, 0x164)[0]

        # the tag data follows the tag index in the file; in memory it is at
        # the end of the block, after the space for the structure bsp
        self.tag_data_address = self.base_address + self.tag_memory_size - self.tag_data_size
        self._tag_data = self.read(self.tag_data_offset + self.tag_index_size, self.tag_data_size)

        string_id_count, string_id_data_size, string_id_index_offset, string_id_data_offset = struct.unpack_from(
            "<4I", header, 0x164
        )
        self._string_id_data = self.read(string_id_data_offset, string_id_data_size)
        self._string_id_index = struct.unpack(
            f"<{string_id_count}I", self.read(string_id_index_offset, string_id_count * 4)
        )

        self._tag_name_data = self.read(tag_name_data_offset, tag_name_data_size)
        self._tag_name_index = struct.unpack(
            f"<{tag_name_count}I", self.read(tag_name_index_offset, tag_name_count * 4)
        )

    @staticmethod
    def _string(data: bytes, offset: int, size: int) -> str:
        return data[offset : offset + size].split(b"\0", 1)[0].decode("latin-1")

    def close(self) -> None:
        self.file.close()
        for shared in self._shared.values():
            shared.close()
        self._shared.clear()

    def read(self, offset: int, size: int) -> bytes:
        self.file.seek(offset)
        data = self.file.read(size)
        if len(data) != size:
            raise CacheFileError("the file is shorter than its header says")
        return data

    def read_resource(self, offset: int, size: int) -> bytes:
        """Reads a block of resources. The two high bits of the offset tell the file that holds it."""
        location = offset >> 30
        offset &= 0x3FFFFFFF
        if location == 0:
            return self.read(offset, size)
        if location not in self._shared:
            path = self.path.with_name(RESOURCE_FILES[location])
            if not path.exists():
                raise CacheFileError(f"'{path.name}' is needed and is not next to '{self.path.name}'")
            self._shared[location] = path.open("rb")
        shared = self._shared[location]
        shared.seek(offset)
        data = shared.read(size)
        if len(data) != size:
            raise CacheFileError(f"'{RESOURCE_FILES[location]}' is shorter than the resource needs")
        return data

    def tag_data(self, address: int, size: int) -> bytes:
        offset = address - self.tag_data_address
        if offset < 0 or offset + size > self.tag_data_size:
            raise CacheFileError(f"address {address:#010x} is outside the tag data")
        return self._tag_data[offset : offset + size]

    def tag_name(self, tag_index: int) -> str:
        """Takes a tag index or the index of a tag reference."""
        if tag_index == NONE or (tag_index & 0xFFFF) >= len(self._tag_name_index):
            return "<none>"
        return self._string(self._tag_name_data, self._tag_name_index[tag_index & 0xFFFF], 1024)

    def string(self, string_id: int) -> str:
        """The text of a string id: its high byte is the length of the text and the rest is its index."""
        index = string_id & 0xFFFFFF
        if index >= len(self._string_id_index):
            return "<none>"
        return self._string(self._string_id_data, self._string_id_index[index], 256)

    def find_tag(self, group: str, name: str) -> int:
        """The index of the first tag of a group whose name is, or ends with, the name; NONE if there is none."""
        wanted = name.replace("/", "\\")
        tag = group_tag(group)
        matches = [
            index
            for index, instance in enumerate(self.tag_instances)
            if instance[0] == tag and self.tag_name(index).endswith(wanted)
        ]
        exact = [index for index in matches if self.tag_name(index) == wanted]
        return (exact or matches or [NONE])[0]

    def scenario(self) -> bytes:
        _, _, address, size = self.tag_instances[self.scenario_index & 0xFFFF]
        return self.tag_data(address, size)
