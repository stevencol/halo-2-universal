# three functions of the way to the main menu: two reals at a time, a
# counter that starts, and whether one of sixteen things has a bit set

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(838)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        data = bytearray(random_.randbytes(0x34))
        data[0] = random_.choice((0, 1))
        for at in (0x04, 0x1C):
            low = random_.choice((random_.uniform(0, 5), 1.0, 2.5))
            high = low + random_.choice((0.0, 0.00005, 1.0, 3.0))
            struct.pack_into("<6f", data, at, low, high, *(random_.uniform(-2, 2) for _ in range(4)))
        t.check(
            "unknown_000137a0",
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals={
                "unknown_0050934c": Buffer(bytes(data)),
                "unknown_004858a0": struct.pack("<d", random_.choice((0.0, 1.0, 2.5, 4.0, 9.0, random_.uniform(-1, 9)))),
            },
        )

        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", time, 2, random_.choice((30, 60)))
        counter = bytes((random_.randrange(0, 40), random_.choice((0, 5, 29, 59, 30)), 0, random_.randrange(4)))
        t.check(
            "unit_counter_start",
            Buffer(counter),
            random_.choice((0, 1)),
            f32(random_.choice((0.0, 0.02, 0.5, 1.0, 2.0, 10.0))),
            globals={"game_time_globals": Buffer(bytes(time))},
        )

        # sixteen entries; their places as in test_small_828, each list with elements of 0x48 bytes
        entries = bytearray(random_.randbytes(16 * 0x20))
        for entry in range(16):
            flags = random_.getrandbits(32) & ~0x1040 | random_.choice((0, 0x40, 0x40)) | random_.choice((0, 0x1000))
            bits = random_.randrange(2) << 9 | random_.randrange(2) << 18
            struct.pack_into("<IIiI", entries, entry * 0x20, flags, random_.choice((NONE, salted(0))), 0, bits)
        # one list for all: elements at 0x400, each with a first bit and how many
        lists = bytearray(random_.randbytes(0x400 + 4 * 0x48))
        for element in range(4):
            struct.pack_into("<hh", lists, 0x400 + element * 0x48 + 0x0A, random_.randrange(40), random_.randrange(5))
        tags = bytearray(random_.randbytes(0x10 + 0x30 + 2 * 0x5C))
        struct.pack_into("<I", tags, 8, 0x10)
        struct.pack_into("<I", tags, 0x10 + 0x28, 0x40)
        for place in range(2):
            struct.pack_into("<I", tags, 0x40 + place * 0x5C + 0x34, len(tags))
        bsp = bytearray(random_.randbytes(0x150 + 2 * 0x58 + 2 * 0xB0 + 3 * 0xC8))
        struct.pack_into("<I", bsp, 0xA0, 0x150 + 2 * 0x58)
        struct.pack_into("<I", bsp, 0x13C, 0x150 + 2 * 0x58 + 2 * 0xB0)
        struct.pack_into("<I", bsp, 0x144, 0x150)
        pointers = [0xA0, 0x13C, 0x144]
        for place in range(2):
            struct.pack_into("<h", bsp, 0x150 + place * 0x58 + 0x34, random_.randrange(3))
            struct.pack_into("<I", bsp, 0x150 + 2 * 0x58 + place * 0xB0 + 0x50, len(bsp))
            pointers.append(0x150 + 2 * 0x58 + place * 0xB0 + 0x50)
        for cluster in range(3):
            struct.pack_into("<I", bsp, 0x150 + 2 * 0x58 + 2 * 0xB0 + cluster * 0xC8 + 0x50, len(bsp) + len(lists))
            pointers.append(0x150 + 2 * 0x58 + 2 * 0xB0 + cluster * 0xC8 + 0x50)
        tags_pointers = (8, 0x10 + 0x28, 0x40 + 0x34, 0x40 + 0x5C + 0x34, len(tags) + 4)
        bsp_pointers = tuple(pointers) + (len(bsp) + 4, len(bsp) + len(lists) + 4)
        tags_lists = bytearray(lists)
        struct.pack_into("<I", tags_lists, 4, len(tags) + 0x400)
        bsp_lists = bytearray(lists)
        struct.pack_into("<I", bsp_lists, 4, len(bsp) + 0x400)
        cluster_lists = bytearray(random_.randbytes(0x400 + 4 * 0x48))
        for element in range(4):
            struct.pack_into("<hh", cluster_lists, 0x400 + element * 0x48 + 0x0A, random_.randrange(40), random_.randrange(5))
        struct.pack_into("<I", cluster_lists, 4, len(bsp) + len(bsp_lists) + 0x400)
        t.check(
            "unknown_000460d0",
            random_.randrange(4),
            Buffer(random_.randbytes(8)),
            random_.randrange(16) | random_.getrandbits(16) << 16,
            globals={
                "unknown_004ba138": bytes(entries),
                "global_tag_instances": Buffer(bytes(tags) + bytes(tags_lists), pointers=tags_pointers),
                "global_structure_bsp": Buffer(bytes(bsp) + bytes(bsp_lists) + bytes(cluster_lists), pointers=bsp_pointers),
            },
        )
