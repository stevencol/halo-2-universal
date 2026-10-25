# two functions of the way to the main menu (guided by halo2-decompiled): the
# value of a property of a particle, and the color of a particle system

import random
import struct

from difftest import Buffer, f32


def run(t) -> None:
    random_ = random.Random(864)

    for n in range(400):
        property_ = bytearray(random_.randbytes(0x1C))
        struct.pack_into(
            "<hhHh",
            property_,
            0,
            random_.randrange(8),
            random_.randrange(8),
            random_.choice((0, 1, 2, 3, 0x101)),
            random_.choice((-1, -2, 0, 3, 7)),
        )
        struct.pack_into("<iI", property_, 0x08, 1, 0x10)
        property_[0x11] = random_.choice((0, 0, 0x10, 0x0F))
        struct.pack_into("<2f", property_, 0x14, random_.uniform(-2, 2), random_.uniform(-2, 2))
        values = struct.pack("<8f", *(random_.choice((0.0, 1.0, -2.5, random_.uniform(-10, 10))) for _ in range(8)))
        value = f32(random_.choice((0.0, 0.25, 1.0, -0.5, 1.5, random_.uniform(-2, 2))))
        t.check(
            "particle_unknown_00246cd0",
            Buffer(bytes(property_), pointers=(0x0C,)),
            Buffer(values),
            externals={"unknown_0013b390": lambda harness, arguments, value=value: (value, tuple(arguments))},
        )

        t.check(
            "particle_unknown_00175a80",
            random_.choice((0, 1, 0x100, 0x12345601)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x60)),
            random_.choice((0, 1, 2, 0x100)),
        )
