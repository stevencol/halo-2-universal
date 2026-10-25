# one function of the way to the main menu (guided by halo2-decompiled):
# whether a point falls in a polygon of a plane

import math
import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(884)

    for n in range(400):
        # the context: what 0x04 points to has the planes at 0x0C, what 0x08 points to the point at 0x60
        context = bytearray(random_.randbytes(0x100))
        struct.pack_into("<II", context, 0x04, 0x20, 0x30)
        struct.pack_into("<I", context, 0x20 + 0x0C, 0xA0)
        struct.pack_into("<3f", context, 0x30 + 0x60, *(random_.uniform(-2, 2) for _ in range(3)))
        for plane in range(3):
            normal = [random_.choice((0.0, random_.uniform(-1, 1))) for _ in range(3)]
            normal[random_.randrange(3)] = random_.choice((1.0, -1.0, 0.7))
            length = math.sqrt(sum(value * value for value in normal))
            struct.pack_into("<4f", context, 0xA0 + plane * 16, *(value / length for value in normal), random_.uniform(-1, 1))
        count = random_.choice((0, 3, 4, 6))
        polygon = bytearray(random_.randbytes(0x30 + 8 * 12))
        struct.pack_into("<I", polygon, 0x04, random_.randrange(3))
        struct.pack_into("<iI", polygon, 0x1C, count, 0x30)
        angle = random_.uniform(0, 1)
        for index in range(count):
            turn = angle + index * 2 * math.pi / max(count, 1)
            size = random_.choice((0.5, 2.0, 4.0))
            struct.pack_into("<3f", polygon, 0x30 + index * 12, math.cos(turn) * size, math.sin(turn) * size, -math.cos(turn) * size)
        t.check(
            "real_math_unknown_00173910",
            Buffer(bytes(context), pointers=(0x04, 0x08, 0x20 + 0x0C)),
            Buffer(bytes(polygon), pointers=(0x20,)),
        )
