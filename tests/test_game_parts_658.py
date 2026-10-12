# the sixteen numbers are bits, one for each thing of a block of the structure bsp

import random
import struct

from difftest import Buffer

ELEMENTS = 0x80


def run(t) -> None:
    random_ = random.Random(658)

    for n in range(300):
        count = random_.choice((0, -1, 1, 5, 31, 32, 33, 70, random_.randint(0, 512)))
        bsp = bytearray(random_.randbytes(ELEMENTS + max(count, 1) * 0x24))
        struct.pack_into("<i", bsp, 0x5C, count)
        struct.pack_into("<I", bsp, 0x60, ELEMENTS)
        for index in range(max(count, 0)):
            bsp[ELEMENTS + index * 0x24 + 0x18] = random_.choice((0, 8, 8, 0xF7, 0xFF, random_.randrange(0x100)))
        t.check(
            "game_unknown_00210db0_unknown_00210e20",
            globals={
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x60,)),
                "unknown_004f93a4": Buffer(random_.choice((b"\xff" * 0x40, random_.randbytes(0x40))) + random_.randbytes(0x10)),
            },
        )
