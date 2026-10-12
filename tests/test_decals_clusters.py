# the clusters of the structure bsp that goes tell the decals

import random
import struct

from difftest import Buffer

CLUSTERS = 0x100


def run(t) -> None:
    random_ = random.Random(668)

    for n in range(300):
        count = random_.choice((0, -1, 1, 2, 9, 30))
        bsp = bytearray(random_.randbytes(CLUSTERS + max(count, 1) * 0xB0))
        struct.pack_into("<I", bsp, 0xF4, random_.choice((0, 1, random_.getrandbits(32))))
        struct.pack_into("<iI", bsp, 0x9C, count, CLUSTERS)
        for index in range(max(count, 0)):
            at = CLUSTERS + index * 0xB0
            struct.pack_into("<hH", bsp, at + 0x7C, random_.choice((-1, 0, 5, -2)), random_.choice((0, 0, 1, 0x8000, 0xFFFF)))
        t.check(
            "decals_unknown_0023aa70",
            globals={"global_structure_bsp": Buffer(bytes(bsp), pointers=(0xA0,))},
            externals={"unknown_0017d710": lambda harness, arguments: 0},
        )
