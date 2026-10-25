# two functions of the way to the main menu (guided by halo2-decompiled): two
# reals of a unit going a tick toward 1 or 0, and the clusters of the
# structure bsp near a point

import random
import struct

from difftest import Buffer, f32

from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF
CLUSTERS = 6
PORTALS = 8


def run(t) -> None:
    random_ = random.Random(875)

    for n in range(400):
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", units, at + 0x12C, random_.choice((NONE, 0, 3)))
        struct.pack_into("<I", units, at + 0x134, random_.getrandbits(32))
        struct.pack_into("<h", units, at + 0x138, random_.choice((-1, 0, 3, 15, 16, -5)))
        struct.pack_into(
            "<3f",
            units,
            at + 0x2B0,
            random_.choice((0.0, 0.5, 0.99, 1.0, 0.01)),
            random_.choice((0.0, 0.5, 0.99, 1.0, 0.01)),
            random_.choice((0.5, 1.0, 3.0, 30.0)),
        )
        struct.pack_into("<I", units, at + 0xD4, random_.choice((NONE, 7)))
        game = bytearray(random_.randbytes(0x10))
        struct.pack_into("<i", game, 8, random_.choice((1, 1, 0, 2)))
        times = bytearray(random_.randbytes(0x24))
        struct.pack_into("<f", times, 4, random_.choice((1 / 30, 1 / 60, 0.1)))
        t.check(
            "unit_unknown_000c6810",
            random_.getrandbits(16) << 16 | index,
            globals={
                "object_header_data": Buffer(bytes(units), pointers=tuple(pointers)),
                "main_game_globals": Buffer(bytes(game)),
                "unknown_004f55dc": random_.randbytes(0x10),
                "unknown_004f55ec": Buffer(random_.randbytes(0xD0)),
                "game_time_globals": Buffer(bytes(times)),
            },
            externals={"unknown_000b58c0": lambda harness, arguments: (0, tuple(arguments))},
        )

        # a structure bsp of six clusters and eight portals between them
        clusters = 0x100
        indices = clusters + CLUSTERS * 0xB0
        portals = indices + CLUSTERS * 8 * 2
        bsp = bytearray(random_.randbytes(portals + PORTALS * 0x24))
        struct.pack_into("<I", bsp, 0x60, portals)
        struct.pack_into("<I", bsp, 0xA0, clusters)
        bsp_pointers = [0x60, 0xA0]
        for cluster in range(CLUSTERS):
            struct.pack_into("<iI", bsp, clusters + cluster * 0xB0 + 0x8C, random_.choice((0, 1, 2, 4, 8)), indices + cluster * 16)
            bsp_pointers.append(clusters + cluster * 0xB0 + 0x90)
            for slot in range(8):
                struct.pack_into("<h", bsp, indices + cluster * 16 + slot * 2, random_.randrange(PORTALS))
        for portal in range(PORTALS):
            struct.pack_into("<hh", bsp, portals + portal * 0x24, random_.choice((-1, *range(CLUSTERS))), random_.choice((-1, *range(CLUSTERS))))
        distances = [f32(random_.choice((0.0, 2.0, 5.0, 9.9, 10.0, 12.0, random_.uniform(0, 15)))) for _ in range(64)]

        def distance(harness, arguments, distances=distances):
            told = sum(1 for name, _ in harness._calls if name == "unknown_0014b240")
            return distances[told % len(distances)], tuple(arguments)

        t.check(
            "scenario_unknown_0012e960",
            random_.randrange(CLUSTERS),
            Buffer(struct.pack("<3f", 1.0, 2.0, 3.0)),
            Buffer(random_.randbytes(0x18 * CLUSTERS)),
            globals={"global_structure_bsp": Buffer(bytes(bsp), pointers=tuple(bsp_pointers))},
            externals={"unknown_0014b240": distance},
        )
