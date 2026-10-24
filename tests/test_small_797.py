# six small functions: a row of the bits of the clusters of a structure bsp,
# the element of a block whose range has an index, a list of shorts by an
# index, two things made from points, colours and vectors, and 32 longs

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(797)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    for n in range(400):
        clusters = random_.choice((0, 1, 31, 32, 33, 64))
        longs = (clusters + 31) >> 5
        bsp = bytearray(random_.randbytes(0xA0))
        struct.pack_into("<i", bsp, 0x9C, clusters)
        struct.pack_into("<I", bsp, 0x54, random_.choice((0, clusters * longs * 4, clusters * longs * 4 + 1, 2 * clusters * longs * 4)))
        struct.pack_into("<I", bsp, 0x58, 0xA0)
        t.check(
            "structure_bsp_unknown_0016e150",
            random_.getrandbits(16) << 16 | random_.randrange(max(clusters, 1)),
            Buffer(bytes(bsp) + random_.randbytes(2 * 64 * 2 * 4), pointers=(0x58,)),
        )

        count = random_.choice((0, 1, 3, 4))
        block = bytearray(random_.randbytes(8 + 4 * 0x48))
        struct.pack_into("<iI", block, 0, count, 8)
        start = 0
        for element in range(4):
            size = random_.choice((0, 1, 5))
            struct.pack_into("<HH", block, 8 + element * 0x48 + 6, start, size)
            start += size + random_.choice((0, 0, 2))
        t.check(
            "structure_bsp_unknown_0016e1b0",
            word(random_.choice((-1, 0, 1, 4, 5, 6, 10, 20, random_.randrange(25)))),
            Buffer(bytes(block), pointers=(4,)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
        )

        t.check(
            "effect_unknown_00173b00",
            random_.getrandbits(16) << 16 | random_.randrange(512),
            random_.getrandbits(16) << 16 | random_.randrange(512),
            Buffer(random_.randbytes(0x840)),
        )

        t.check(
            "effect_unknown_00176330",
            Buffer(random_.randbytes(0x38)),
            Buffer(random_.randbytes(12)),
            globals={"global_down3d": Buffer(random_.randbytes(12)), "global_up3d": Buffer(random_.randbytes(12))},
        )

        t.check(
            "effect_unknown_00178240",
            Buffer(random_.randbytes(0x74)),
            random_.choice((0, Buffer(random_.randbytes(12)))),
            random_.choice((0, Buffer(random_.randbytes(12)))),
            f32(random_.uniform(-2, 2)),
            f32(random_.uniform(-2, 2)),
            globals={"player_color_default": Buffer(random_.randbytes(12))},
        )

        t.check("effect_unknown_0017b750", random_.getrandbits(32), Buffer(random_.randbytes(0x80)))
