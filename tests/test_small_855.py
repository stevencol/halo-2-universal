# four functions of the way to the main menu (guided by halo2-decompiled): a
# direction picked at random, the datum of the scenario of an object type,
# the texture wanted at a stage and one of four that starts over

import random
import struct

from difftest import Buffer, GlobalData, f32

from test_object_types_more import after

NONE = 0xFFFFFFFF
TYPES = 13
TYPE = 0x10
ENTRY = 0x140


def run(t) -> None:
    random_ = random.Random(855)

    for n in range(400):
        seed = random_.getrandbits(32)
        t.check(
            "seed_random_direction3d",
            Buffer(struct.pack("<I", seed)),
            Buffer(random_.randbytes(12)),
            globals={"random_directions": random_.randbytes(1026 * 12)},
        )

        # the scenario has two blocks: a count and where its elements are
        scenario = bytearray(random_.randbytes(0x40))
        for block in (0x10, 0x20):
            struct.pack_into("<iI", scenario, block, random_.choice((0, 1, 3, 7)), 0x30)
        scenario = Buffer(bytes(scenario), pointers=(0x14, 0x24))
        base = after(scenario)
        types = bytearray(random_.randbytes(TYPES * TYPE))
        for index in range(TYPES):
            struct.pack_into(
                "<hhh",
                types,
                index * TYPE + 0x0A,
                random_.choice((0x10, 0x20, 0x20, -1)),
                random_.choice((5, 5, 5, -1)),
                random_.choice((1, 4, 0x18, -3)),
            )
        t.check(
            "object_unknown_000d5260",
            random_.randrange(TYPES),
            random_.choice((-1, 0, 1, 2, 3, 6, 7, 0x40000000)),
            globals={
                "global_scenario": scenario,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
            },
            extra=[Buffer(bytes(types))],
        )

        bitmap = random_.choice((None, Buffer(random_.randbytes(0x20))))
        t.check(
            "rasterizer_unknown_00014390",
            random_.randrange(4),
            bitmap if bitmap else 0,
            f32(random_.choice((0.0, 1.0, -0.5, random_.uniform(-4, 4)))),
            globals={
                "unknown_0051f3c8": random_.randbytes(0x20),
                "unknown_00485af4": random_.randbytes(0x20),
            },
            externals={
                "shell_bitmap_get_texture_blocking": lambda harness, arguments, texture=random_.getrandbits(32): (
                    texture,
                    tuple(arguments[1:]),
                )
            },
        )

        # the one at 0x0C of each gets "unknown_0023c110", which is only pointed to
        storage = bytearray(random_.randbytes(0x50C))
        struct.pack_into("<i", storage, 4, random_.choice((0, 0, 1, -1)))
        t.check(
            "unknown_00155710",
            random_.randrange(4),
            globals={"unknown_004e8c38": GlobalData(bytes(storage), places=tuple(0x18 + index * ENTRY for index in range(4)))},
        )
