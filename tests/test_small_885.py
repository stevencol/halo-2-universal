# one function of the way to the main menu (guided by halo2-decompiled): where
# a point is from a polygon, by a state

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(885)

    for n in range(400):
        context = bytearray(random_.randbytes(0x100))
        struct.pack_into("<II", context, 0x04, 0x20, 0x30)
        struct.pack_into("<I", context, 0x20 + 0x0C, 0xA0)
        point = [random_.uniform(-2, 2) for _ in range(3)]
        struct.pack_into("<3f", context, 0x30 + 0x60, *point)
        plane = [random_.choice((0.0, 1.0, -1.0, random_.uniform(-1, 1))) for _ in range(3)]
        # the plane's distance makes the point on it, just off it or far
        offset = random_.choice((0.0, 0.01, -0.01, 0.015625, -0.015625, 0.0157, -0.02, 1.0, -3.0))
        d = sum(a * b for a, b in zip(point, plane)) - offset
        struct.pack_into("<4f", context, 0xA0, *plane, d)
        polygon = bytearray(random_.randbytes(0x30))
        struct.pack_into("<I", polygon, 0x04, 0)
        t.check(
            "real_math_unknown_00173890",
            Buffer(bytes(context), pointers=(0x04, 0x08, 0x20 + 0x0C)),
            Buffer(bytes(polygon)),
            random_.choice((-1, 0, 1, 2, 3, 4)),
            Buffer(random_.randbytes(1)),
            externals={"real_math_unknown_00173910": lambda harness, arguments, inside=random_.choice((0, 1)): (inside, tuple(arguments))},
        )
