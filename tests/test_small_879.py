# two functions of the way to the main menu (guided by halo2-decompiled): the
# next event of an effect, and the particle systems told their location

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SYSTEMS = 40
SYSTEM = 0x54


def run(t) -> None:
    random_ = random.Random(879)

    for n in range(400):
        effect = bytearray(random_.randbytes(0x60))
        struct.pack_into("<H", effect, 2, random_.getrandbits(16) & ~0x202 | random_.choice((0, 2, 0x200, 0x202)))
        struct.pack_into("<I", effect, 4, random_.randrange(2))
        events = random_.choice((0, 1, 3, 5))
        struct.pack_into("<h", effect, 0x5C, random_.choice((-1, 0, 1, 2, 4)))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x20 + 5 * 0x38))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x20)
            struct.pack_into("<Ih", tags, 0x20 + tag * 0x20, random_.getrandbits(32), random_.choice((-1, 0, 1, 2)))
            struct.pack_into("<iI", tags, 0x20 + tag * 0x20 + 0x14, events, 0x60)
        for event in range(5):
            struct.pack_into("<f", tags, 0x60 + event * 0x38 + 4, random_.choice((0.0, 0.0, 0.3, 0.7, 1.0, -0.5)))
        t.check(
            "effect_unknown_00179190",
            Buffer(bytes(effect)),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18, 0x20 + 0x18, 0x40 + 0x18)),
                "global_random_seed_reference": Buffer(random_.randbytes(8)),
            },
        )

        # the particle systems as a data array, and their locations
        table = bytearray(random_.randbytes(0x4C + 8 + SYSTEMS * SYSTEM))
        struct.pack_into("<i", table, 0x24, SYSTEM)
        struct.pack_into("<i", table, 0x38, random_.choice((0, 3, SYSTEMS)))
        struct.pack_into("<II", table, 0x44, 0x54, 0x4C)
        bsp_index = random_.choice((0, 1, -1))
        for system in range(SYSTEMS):
            at = 0x54 + system * SYSTEM
            struct.pack_into("<h", table, at + 0x22, random_.choice((bsp_index, 0, 1, -1)))
            struct.pack_into("<I", table, at + 0x30, random_.choice((NONE, random_.randrange(3))))
        locations = bytearray(random_.randbytes(0x48 + 3 * 0x34))
        struct.pack_into("<I", locations, 0x44, 0x48)
        bsp = bytearray(random_.randbytes(0x40 + 4 * 8))
        struct.pack_into("<I", bsp, 0x30, 0x40)
        leaves = [random_.choice((NONE, 0, 1, 3)) for _ in range(SYSTEMS)]

        def leaf(harness, arguments, leaves=leaves):
            told = sum(1 for name, _ in harness._calls if name == "bsp3d_test_point")
            return leaves[told % len(leaves)], (arguments[0], arguments[1], bytes(harness.mu.mem_read(arguments[2], 12)))

        t.check(
            "particle_systems_unknown_00173ee0",
            globals={
                "particle_system_data": Buffer(bytes(table), pointers=(0x44, 0x48)),
                "particle_location_data": Buffer(bytes(locations), pointers=(0x44,)),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x30,)),
                "global_structure_bsp_index": struct.pack("<h", bsp_index),
                "global_bsp3d": 0x80345670,
            },
            externals={
                "bsp3d_test_point": leaf,
                "particle_system_unknown_00175180": lambda harness, arguments: (0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], 8)))),
            },
        )
