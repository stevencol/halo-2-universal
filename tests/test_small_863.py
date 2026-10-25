# two functions of the way to the main menu (guided by halo2-decompiled): one
# of some choices picked at random by their weights, and the vibrations of an
# object going a tick toward one

import random
import struct

from difftest import Buffer

from test_small_859 import BIG, big_objects
from test_units import COUNT


def run(t) -> None:
    random_ = random.Random(863)

    for n in range(400):
        count = random_.choice((0, 1, 2, 3, 4, 5, 8, 9, -1))
        choices = bytearray(random_.randbytes(0x1C * max(count, 1)))
        for index in range(max(count, 0)):
            struct.pack_into("<f", choices, index * 0x1C + 0x18, random_.choice((0.0, 1.0, 0.5, 3.0, -1.0, random_.uniform(0, 10))))
        t.check(
            "unknown_0016dce0",
            count,
            Buffer(bytes(choices)),
            globals={"global_random_seed_reference": Buffer(random_.randbytes(4))},
        )

        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<h", units, at + 0x33E, 0x300)
        data = at + 0x300
        struct.pack_into(
            "<3f",
            units,
            data + 0x60,
            random_.choice((0.0, -0.5, 0.05, 0.25, 0.5, 1.0, 2.0)),
            random_.choice((0.0, 1.0, 3.0, 19.0, random_.uniform(0, 30))),
            random_.choice((0.0, 0.5, 1.0, 1.5, 2.0, 2.0, 2.004, random_.uniform(-1, 3))),
        )
        struct.pack_into("<h", units, data + 0x6C, random_.choice((-1, 0, 5, 12, 13)))
        t.check(
            "unit_unknown_0010eaf0",
            random_.getrandbits(16) << 16 | index,
            globals={
                "object_header_data": Buffer(bytes(units), pointers=tuple(pointers)),
                "unknown_0054764c": struct.pack("<f", random_.choice((1 / 128, 0.01, 1.0 / 255, 0.02))),
            },
        )
