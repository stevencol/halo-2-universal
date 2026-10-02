# the tags of source/cache/cache_files.c, and source/memory/binary_search.c

import random
import struct

from difftest import Buffer, FunctionPointer, Mock, Pointer, mock_address

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

    # bitmap groups: some tag instances, each with the address of a group
    # after them in the buffer, or with none; a group has some bitmaps
    for _ in range(30):
        tag_count = random_.choice((1, 4, 20))
        groups_offset = tag_count * 16
        instance_data = bytearray()
        group_data = bytearray()
        pointers = []
        for n in range(tag_count):
            if random_.random() < 0.2:
                instance_data += struct.pack("<4I", 0x6269746D, 0xE1000000 | n, 0, 0)
                continue
            bitmap_count = random_.choice((0, 1, 3))
            group_offset = groups_offset + len(group_data)
            pointers.append(n * 16 + 8)
            instance_data += struct.pack("<4I", 0x6269746D, 0xE1000000 | n, group_offset, 0x4C)
            # the group, with its bitmaps after it
            group = bytearray(random_.randbytes(0x4C))
            struct.pack_into("<iI", group, 0x44, bitmap_count, group_offset + 0x4C)
            pointers.append(group_offset + 0x48)
            group_data += group + random_.randbytes(bitmap_count * 0x74)
        data = Buffer(bytes(instance_data) + bytes(group_data), pointers=tuple(pointers))
        for n in range(tag_count):
            for bitmap_index in (-1, 0, 1, 2, 3, 0x7FFF, -0x8000):
                # the identifier of the index is not looked at
                t.check(
                    "bitmap_group_get_bitmap",
                    random_.getrandbits(16) << 16 | n,
                    bitmap_index,
                    globals={"global_tag_instances": data},
                )

    # reading the cache file. The function that starts a read is not
    # decompiled: the test stands in for it, with a file of its own
    file_data = random_.randbytes(0x1000)

    def read_async(short_by: int = 0):
        def handler(harness, arguments):
            offset, size, buffer, bytes_read, done, unknown1, unknown2 = arguments
            count = max(size - short_by, 0)
            harness.mu.mem_write(buffer, file_data[offset & 0xFFF :][:count])
            harness.mu.mem_write(bytes_read, struct.pack("<i", count))
            harness.mu.mem_write(done, b"\1")
            # the addresses of where the results go are not the same in the game and in source/
            return 0, (offset, size, buffer, unknown1, unknown2)

        return handler

    for size in (0, 1, 0x200, 0x333):
        for offset in (0, 0x200, 0x40000400, 0xC0000000):
            for short_by in (0, 1):
                t.check(
                    "cache_file_blocking_read",
                    -1,
                    offset,
                    Buffer(random_.randbytes(0x340)),
                    size,
                    externals={"cache_file_read_async": read_async(short_by)},
                )

    # a structure bsp: the reference of the scenario, with the memory that
    # the block is read to after it in the buffer, and some tag instances
    for block_size in (0x10, 0x200, 0x201, 0x3FF, 0x400):
        for signature in (b"psbs", b"sbsp", b"\0\0\0\0"):
            for lightmap_index in (0xE3B50002, NONE):
                for short_by in (0, 1):
                    # the block that the file has at the offset 0: the header of a structure bsp
                    block = struct.pack("<3I", block_size, 0x80101000, 0x80102000) + signature
                    reference = struct.pack("<4I", 0, block_size, 0x50, 0) + struct.pack(
                        "<4I", 0x73627370, 0xE3B40001, 0x6C746D70, lightmap_index
                    )
                    data = Buffer(reference.ljust(0x50, b"\0") + random_.randbytes(0x400), pointers=(8,))
                    instances = Buffer(random_.randbytes(16 * 4))

                    def handler(harness, arguments, block=block, short_by=short_by):
                        offset, size, buffer, bytes_read, done, unknown1, unknown2 = arguments
                        count = max(size - short_by, 0)
                        harness.mu.mem_write(buffer, block.ljust(size, b"\xAB")[:count])
                        harness.mu.mem_write(bytes_read, struct.pack("<i", count))
                        harness.mu.mem_write(done, b"\1")
                        return 0, (offset, size, buffer, unknown1, unknown2)

                    t.check(
                        "cache_file_structure_bsp_load",
                        data,
                        globals={"global_tag_instances": instances, "cache_file_globals+0x80c": None},
                        externals={"cache_file_read_async": handler},
                    )

    # no cache file loaded
    t.check("cache_file_tag_group_get", 0x62697064, globals={TAG_HEADER_GLOBAL: 0})

    # ---------- a number among numbers of 4 bytes that are in order

    def difference(call) -> int:
        key, element, context = call.stack
        signed = lambda value: value - (1 << 32) if value & 0x80000000 else value
        return (signed(key) > signed(element)) - (signed(key) < signed(element))

    def far(call) -> int:
        # not 1 and -1: any number with the sign
        return difference(call) * 0x7FFFFFFF & 0xFFFFFFFF

    for count in (0, 1, 2, 3, 4, 7, 8, 9, 31, 100):
        for n in range(12):
            numbers = sorted(random_.sample(range(-(1 << 31), 1 << 31), count + 2))
            # the numbers that are searched, and one after them that is not
            data = Buffer(struct.pack(f"<{count + 2}i", *numbers))
            keys = [*numbers[:count][:3], *numbers[max(count - 2, 0) :], random_.randrange(-(1 << 31), 1 << 31), numbers[0] - 1 if numbers[0] > -(1 << 31) else 0]
            key = random_.choice(keys)
            t.check(
                "binary_search_4byte",
                count,
                key & 0xFFFFFFFF,
                data,
                mock_address(0),
                random_.getrandbits(32),
                mocks=[Mock(far if n % 2 else difference, stack_arguments=3)],
            )
