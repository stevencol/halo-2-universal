# the tags of source/cache/cache_files.c, and source/memory/binary_search.c

import random
import struct

from difftest import Buffer, FunctionPointer, Pointer

TAG_HEADER_SIZE = 0x20
NONE = 0xFFFFFFFF
# the offset of the tag header in the cache file globals
TAG_HEADER_GLOBAL = "cache_file_globals+0x808"


def group_tag(random_: random.Random) -> int:
    """Four characters, as a group tag is. The game compares two by their
    difference, which is right only for values that near."""
    return int.from_bytes(bytes(random_.randrange(0x20, 0x7F) for _ in range(4)), "big")


def group_tags(random_: random.Random, count: int) -> list[int]:
    """Group tags in the order the game keeps them."""
    return sorted({group_tag(random_) for _ in range(count)})


def tags(random_: random.Random, group_count: int, tag_count: int) -> tuple[Buffer, int, list[int], list[tuple]]:
    """A tag header with its groups and its instances after it, the offset of
    the instances, the group tags, and (index, group tag) of each tag that is there."""
    groups = group_tags(random_, group_count)
    group_data = bytearray()
    parents: dict[int, tuple[int, int]] = {}
    for group in groups:
        parents[group] = (random_.choice([NONE, *groups]), random_.choice([NONE, *groups]))
        group_data += struct.pack("<3I", group, *parents[group])

    instances_offset = TAG_HEADER_SIZE + len(group_data)
    instance_data = bytearray()
    present: list[tuple] = []
    for n in range(tag_count):
        if random_.random() < 0.15:
            # an empty place
            instance_data += struct.pack("<4I", NONE, NONE, 0, 0)
            continue
        tag_index = (random_.randrange(0xE000, 0xFFFF) << 16) | n
        group = random_.choice(groups)
        instance_data += struct.pack("<4I", group, tag_index, 0x80100000 + n * 0x40, 0x40)
        present.append((tag_index, group, parents[group]))

    header = struct.pack("<IiIIIIiI", TAG_HEADER_SIZE, len(groups), instances_offset, 0, 0, 0, tag_count, 0x74616773)
    return Buffer(header + bytes(group_data) + bytes(instance_data), pointers=(0, 8)), instances_offset, groups, present


def run(t) -> None:
    random_ = random.Random(10)

    # a search in arrays of any length; the elements are tag groups
    for count in (0, 1, 2, 3, 7, 8, 30, 118):
        groups = group_tags(random_, count)
        data = Buffer(b"".join(struct.pack("<3I", group, 1, 2) for group in groups) + bytes(12))
        keys = [*groups[:12], *(group_tag(random_) for _ in range(8)), 0x20202020, 0x7E7E7E7E]
        for key in keys:
            t.check(
                "binary_search",
                len(groups),
                Buffer(struct.pack("<3I", key, NONE, NONE)),
                data,
                12,
                FunctionPointer("cache_file_tag_group_compare"),
                0,
            )
    for _ in range(60):
        a, b = random_.getrandbits(32), random_.getrandbits(32)
        t.check("cache_file_tag_group_compare", Buffer(struct.pack("<I", a)), Buffer(struct.pack("<I", b)), 0)

    for _ in range(40):
        data, instances_offset, groups, present = tags(random_, random_.choice((1, 5, 40, 118)), random_.choice((1, 6, 60)))

        def globals_(buffer: Buffer = data, offset: int = instances_offset) -> dict:
            return {TAG_HEADER_GLOBAL: buffer, "global_tag_instances": Pointer(buffer, offset)}

        for group in [*groups[:10], group_tag(random_), 0x20202020]:
            t.check("cache_file_tag_group_get", group, globals=globals_())

        for tag_index, group, parents in present[:12]:
            # the tag's group, its parents, another group, and an index with another identifier
            for wanted in (group, *parents, random_.choice(groups), group_tag(random_)):
                t.check("tag_get", wanted, tag_index, globals=globals_())
            t.check("tag_get", group, tag_index ^ 0x10000, globals=globals_())
        for tag_index in (NONE, 0, 0xFFFF, 0x7FFF, 0xE1740000 + 1000):
            t.check("tag_get", groups[0], tag_index, globals=globals_())

        # every tag of a group, of a parent and of all groups; one step from each place
        for wanted in (NONE, groups[0], random_.choice(groups), group_tag(random_)):
            for start in range(0, 62, 7):
                iterator = Buffer(struct.pack("<2I2iI", 0x11111111, 0x22222222, 0x33333333, start, wanted))
                t.check("tag_iterator_next", iterator, globals=globals_())

    # no cache file loaded
    t.check("cache_file_tag_group_get", 0x62697064, globals={TAG_HEADER_GLOBAL: 0})
