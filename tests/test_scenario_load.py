# loading and unloading the tags of a map (source/cache/cache_files.c), and
# source/memory/physical_memory.c
#
# The functions of the game that read the disc and those of the console are
# not decompiled: the test stands in for them, with a map of its own

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer
from test_cache_files import BUILD

NONE = 0xFFFFFFFF
MAP_FILE_SIZE = 0x804
MAP_FILE_COUNT = 6
OPEN_MAP_INDEX = "cached_map_file_globals+0x3018"

# where the map has its tags, and the memory that the game gets for them
TAG_DATA_OFFSET = 0x800
LOW_ADDRESS = 0x00010000
TAG_MEMORY = PHYSICAL_ADDRESS | LOW_ADDRESS


def map_header(
    index_size: int, data_size: int, memory_size: int, signature: bytes = b"daeh", build: bytes = BUILD
) -> bytes:
    data = bytearray(0x800)
    data[0:4] = signature
    struct.pack_into("<2i", data, 4, 8, 0x4000)
    struct.pack_into("<4I", data, 0x10, TAG_DATA_OFFSET, index_size, data_size, memory_size)
    data[0x120 : 0x120 + len(build)] = build
    struct.pack_into("<h", data, 0x140, 1)
    data[0x198:0x1A0] = b"lockout\0"
    data[0x7FC:0x800] = b"toof"
    return bytes(data)


def tag_index(
    random_: random.Random,
    size: int,
    instances: int = TAG_MEMORY + 0x20,
    tag_count: int = 4,
    signature: int = 0x74616773,
    scenario_index: int = 0xE1740002,
) -> bytes:
    """A tag header, with four tag instances after it."""
    header = struct.pack("<IiIIIIiI", TAG_MEMORY + 0x20, 0, instances, scenario_index, 0xE1750001, 0, tag_count, signature)
    tags = b"".join(
        struct.pack("<4I", random_.getrandbits(32), 0xE1730000 + n * 0x10001, 0x80020000 + n * 0x100, 0x100)
        for n in range(4)
    )
    return (header + tags).ljust(size, b"\x11")


def physical_memory(stage: int, low: int, high: int) -> bytes:
    lows = [0x01000000 + n for n in range(5)]
    highs = [0x02000000 + n for n in range(5)]
    lows[stage], highs[stage] = low, high
    return struct.pack("<13I", stage, 0x61000, 0x3145000, *lows, *highs)


def run(t) -> None:
    random_ = random.Random(14)
    name = b"scenarios\\multi\\lockout\\lockout\0"

    def case(
        header: bytes,
        index: bytes,
        map_index: int = 2,
        stage: int = 0,
        low: int = LOW_ADDRESS,
        high: int = 0x00100000,
        short_read: int = NONE,
    ) -> None:
        index_size, data_size = struct.unpack_from("<2I", header, 0x14)
        file_data = bytes(TAG_DATA_OFFSET) + index + random_.randbytes(data_size)
        reads = []

        def find_map(harness, arguments):
            # the memory of the tags stays from the call before
            harness.mu.mem_write(TAG_MEMORY, b"\xCD" * 0x4000)
            reads.clear()
            return map_index

        def read_async(harness, arguments):
            offset, size, buffer, bytes_read, done, unknown1, unknown2 = arguments
            count = max(size - 1, 0) if len(reads) == short_read else size
            reads.append(offset)
            harness.mu.mem_write(buffer, file_data[offset : offset + size].ljust(size, b"\xAB")[:count])
            harness.mu.mem_write(bytes_read, struct.pack("<i", count))
            harness.mu.mem_write(done, b"\1")
            return 0, (offset, size, buffer, unknown1, unknown2)

        files = bytearray(random_.randbytes(MAP_FILE_SIZE * MAP_FILE_COUNT))
        if 0 <= map_index < MAP_FILE_COUNT:
            files[map_index * MAP_FILE_SIZE + 4 : (map_index + 1) * MAP_FILE_SIZE] = header
        t.check(
            "scenario_load",
            Buffer(name),
            globals={
                "cache_file_globals": random_.randbytes(0x810),
                "cached_map_file_globals": bytes(files) + struct.pack("<I", 5),
                "physical_memory_globals": physical_memory(stage, low, high),
                "global_tag_instances": 0x11111111,
                "global_scenario_index": 0x22222222,
                "global_scenario": 0x33333333,
                "global_game_globals": 0x44444444,
                "global_preferences_globals+0x1F0": 9809,
                "global_preferences_globals+0x1": b"\0",
            },
            externals={
                "cached_map_files_find_map": find_map,
                "cache_file_block_until_not_busy": lambda harness, arguments: 0,
                "cache_file_read_async": read_async,
                "XPhysicalProtect": lambda harness, arguments: 0,
            },
        )

    # maps that load, with sizes that are whole sectors and whole pages and
    # sizes that are not, from every place of the table and every stage
    for index_size, data_size, memory_size in ((0x400, 0x600, 0x3000), (0x333, 0x5F1, 0x2801), (0x200, 1, 0x1000)):
        header = map_header(index_size, data_size, memory_size)
        for map_index in range(MAP_FILE_COUNT):
            case(header, tag_index(random_, index_size), map_index=map_index, stage=map_index % 5)
        # no scenario
        case(header, tag_index(random_, index_size, scenario_index=NONE))

        # a header that is not good, and a build that is not this one
        case(map_header(index_size, data_size, memory_size, signature=b"head"), tag_index(random_, index_size))
        case(map_header(index_size, data_size, memory_size, build=b"02.06.28.07902"), tag_index(random_, index_size))

        # memory: just enough, one page short, none, and the address zero
        pages = (memory_size + 0xFFF) & ~0xFFF
        for stage in (0, 3):
            case(header, tag_index(random_, index_size), stage=stage, high=LOW_ADDRESS + pages)
            case(header, tag_index(random_, index_size), stage=stage, high=LOW_ADDRESS + pages - 0x1000)
            case(header, tag_index(random_, index_size), stage=stage, high=0)
            case(header, tag_index(random_, index_size), stage=stage, low=0)
            case(header, tag_index(random_, index_size), stage=stage, high=0x80000000 | LOW_ADDRESS)

        # a read that comes short: that of the tag index, and that of the tag data
        for short_read in (0, 1):
            case(header, tag_index(random_, index_size), short_read=short_read)

        # a tag index that is not good
        case(header, tag_index(random_, index_size, instances=0))
        for tag_count in (0, -1, 1):
            case(header, tag_index(random_, index_size, tag_count=tag_count))
        case(header, tag_index(random_, index_size, signature=0x73676174))

    # unloading, with a map open and with none
    for open_map_index in (NONE, 0, 3, 5):
        t.check(
            "scenario_tags_unload",
            globals={
                "cache_file_globals": random_.randbytes(0x810),
                OPEN_MAP_INDEX: open_map_index,
                "global_tag_instances": 0x11111111,
            },
            externals={"cache_file_block_until_not_busy": lambda harness, arguments: 0},
        )

    # memory from the high end: enough, just enough, a page short, none
    # free, and down to the address zero
    for size1, size2 in ((0x1000, 0), (0x800, 0x801), (0, 0), (1, 0), (0x12345, 0x7000), (0xFFFFF001, 0x2000)):
        pages = (size1 + size2 + 0xFFF) & 0xFFFFF000
        for stage in (0, 4):
            for low, high in (
                (LOW_ADDRESS, 0x00100000),
                (LOW_ADDRESS, LOW_ADDRESS + pages),
                (LOW_ADDRESS, LOW_ADDRESS + pages - 0x1000),
                (LOW_ADDRESS, LOW_ADDRESS),
                (0, pages),
                (0x80000000 | LOW_ADDRESS, 0x00100000),
            ):
                t.check(
                    "game_state_allocate_buffer",
                    size1,
                    size2,
                    globals={
                        "physical_memory_globals": physical_memory(stage, low, high & 0xFFFFFFFF),
                        "xbox_game_state_globals": random_.randbytes(0x28),
                    },
                    externals={"XPhysicalProtect": lambda harness, arguments: 0},
                )

    for _ in range(3):
        t.check(
            "physical_memory_initialize",
            globals={"physical_memory_globals": random_.randbytes(0x34)},
            externals={"XPhysicalAlloc": lambda harness, arguments: 0x80061000},
        )
