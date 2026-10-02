# source/saved games/saved_game_files.c
#
# The console's library is stood in for: what it says of the hard disk is
# the test's

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
GLOBALS_SIZE = 0x1B0
UNIT_MASK = 8
VARIANT_SIZE = 0x130


def text(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


def input_globals(random_: random.Random, mask: int) -> bytes:
    data = bytearray(random_.randbytes(GLOBALS_SIZE))
    struct.pack_into("<H", data, UNIT_MASK, mask)
    # the letters of the memory units
    data[UNIT_MASK + 2 : UNIT_MASK + 10] = b"fghijklm"
    return bytes(data)


def run(t) -> None:
    random_ = random.Random(49)

    # ---------- the types of the files

    for engine in (1, 2, 3, 4, 7, 8, 9):
        data = bytearray(random_.randbytes(VARIANT_SIZE))
        struct.pack_into("<I", data, 0x44, engine)
        t.check("saved_game_file_type_from_variant", Buffer(bytes(data)))
    for game in range(7):
        t.check("saved_game_file_type_from_listed_game", game)
    for type_ in (1, 2, 4, 5, 7, 8, 9):
        t.check("saved_game_file_type_to_listed_game", type_)

    types = list(range(-2, 14)) + [NONE, 0x7FFFFFFF, 0x80000000, 0x100, 0x10000]
    for type_ in types:
        t.check("saved_game_file_type_get_string_id", type_)
        t.check("saved_game_file_type_get_size", type_)
        # a name is in the game's data or in that of source/: its text is compared
        t.check("saved_game_file_type_get_name", type_)

    # ---------- the room of the hard disk

    def cluster_size(size: int):
        return lambda harness, arguments, size=size: (size, (text(harness, arguments[0]),))

    for mask in (0, 1, 2, 3, 0xFFFE, 0xFFFF):
        for cluster in (0x4000, 0x1000, 0, 0x8000, 1, 0x10000):
            for size in (0, 1, 0x130, 0x1E0, 0x15CB8, 0x4000, 0x3FFF, 0x4001, NONE, 0x80000000, 0x7FFFFFFF & ~0xFFFFF, 0xFFFF0000):
                t.check(
                    "saved_game_file_size_to_blocks",
                    size,
                    globals={"input_globals": input_globals(random_, mask)},
                    externals={"XGetDiskClusterSize": cluster_size(cluster)},
                )
            for type_ in (0, 1, 5, 9, 10, 11, NONE, 0x80000000):
                t.check(
                    "saved_game_file_type_get_blocks",
                    type_,
                    globals={"input_globals": input_globals(random_, mask)},
                    externals={"XGetDiskClusterSize": cluster_size(cluster)},
                )

    def free_space(result: int, free: int):
        def handler(harness, arguments):
            root, available, total, total_free = arguments
            harness.mu.mem_write(available, struct.pack("<Q", free))
            harness.mu.mem_write(total, struct.pack("<Q", free ^ 0x5555555555555555))
            harness.mu.mem_write(total_free, struct.pack("<Q", free ^ 0xAAAAAAAAAAAAAAAA))
            # the three numbers are in the stack, which is not at the same place in the game and in source/
            return result, (text(harness, root),)

        return handler

    for blocks in (0, 1, 2, 0x100, 0x1FFFF, 0x20000, 0x3FFFF, 0x40000, 0x7FFFF, NONE, 0x80000000):
        needed = (blocks << 14) & 0xFFFFFFFF
        needed -= (1 << 32) if needed & 0x80000000 else 0
        needed &= 0xFFFFFFFFFFFFFFFF
        frees = {0, 1, needed, (needed - 1) & 0xFFFFFFFFFFFFFFFF, (needed + 1) & 0xFFFFFFFFFFFFFFFF}
        frees |= {needed ^ 0x100000000, (needed & 0xFFFFFFFF) | 0x100000000, 0xFFFFFFFFFFFFFFFF, 0x1_0000_0000, 0xFFFFFFFF}
        for free in sorted(frees):
            for result in (1, 0, 0x100):
                t.check(
                    "saved_game_files_hard_disk_has_blocks",
                    blocks,
                    globals={"input_globals": input_globals(random_, random_.choice((1, 1, 0, 3)))},
                    externals={"GetDiskFreeSpaceExA": free_space(result, free)},
                )

    # ---------- paths

    for unit in (0, 1, 2, 8, 15, 16, 31, 32, 33):
        for mask in (0, 1, 2, 4, 0x100, 0x8000, 0xFFFF):
            for size in (8, 4, 3, 1, 0x100):
                t.check(
                    "saved_game_files_get_unit_root",
                    unit,
                    Buffer(random_.randbytes(0x10)),
                    size | random_.getrandbits(16) << 16,
                    globals={"input_globals": input_globals(random_, mask)},
                )

    directories = (b"", b"u:\\saved\\", b"u:\\saved", b"d" * 0xF0, b"d" * 0xFC + b"\\", b"d" * 0xFF, b"d" * 0x120)
    for directory in directories:
        for type_ in (0, 1, 5, 9, 10, 11, NONE):
            t.check(
                "saved_game_file_get_type_path",
                Buffer(random_.randbytes(0x100)),
                Buffer(directory + b"\0"),
                type_,
            )
