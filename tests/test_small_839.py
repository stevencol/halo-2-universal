# two functions of the way to the main menu: a weight of two vectors and
# four reals added to a sum, and whether a cluster is of something

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(839)

    for n in range(400):
        t.check(
            "unknown_001317a0",
            Buffer(struct.pack("<12f", *(random_.uniform(-3, 3) for _ in range(12)))),
            Buffer(struct.pack("<3f", *(random_.uniform(-3, 3) for _ in range(3)))),
            Buffer(struct.pack("<3f", *(random_.uniform(-3, 3) for _ in range(3)))),
            f32(random_.choice((-0.5, 0.0, 0.3, 1.0, 1.5))),
            *(f32(random_.uniform(-3, 3)) for _ in range(4)),
        )

        # three clusters (byte at 0x70), a block at 0x68 of shorts (0x18 bytes), and a scenario block at 0x34C
        bsp = bytearray(random_.randbytes(0xC0 + 3 * 0xB0 + 3 * 0x18))
        struct.pack_into("<I", bsp, 0xA0, 0xC0)
        struct.pack_into("<I", bsp, 0x68, 0xC0 + 3 * 0xB0)
        for cluster in range(3):
            bsp[0xC0 + cluster * 0xB0 + 0x70] = random_.choice((0xFF, 0, 1, 0x80, 0x82))
        for element in range(3):
            struct.pack_into("<h", bsp, 0xC0 + 3 * 0xB0 + element * 0x18, random_.choice((-1, 0, 2)))
        scenario = bytearray(random_.randbytes(0x350 + 3 * 0x10))
        struct.pack_into("<I", scenario, 0x34C, 0x350)
        for element in range(3):
            struct.pack_into("<I", scenario, 0x350 + element * 0x10 + 8, random_.choice((5, 6)))
        t.check(
            "game_unknown_0016e210",
            random_.choice((NONE, 0, 1, 2)),
            random_.choice((NONE, 5, 6)),
            globals={
                "global_structure_bsp": random_.choice((0, Buffer(bytes(bsp), pointers=(0xA0, 0x68)), Buffer(bytes(bsp), pointers=(0xA0, 0x68)))),
                "global_scenario": random_.choice((0, Buffer(bytes(scenario), pointers=(0x34C,)), Buffer(bytes(scenario), pointers=(0x34C,)))),
                "unknown_004e64c8": random_.choice((0, 0, 0x10, 0xEF)),
            },
        )
