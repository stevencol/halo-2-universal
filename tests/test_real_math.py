# source/math/real_math.c

import random
import struct

from difftest import Buffer


def vector3d(i: float, j: float, k: float) -> Buffer:
    return Buffer(struct.pack("<3f", i, j, k), format="<3f")


def run(t) -> None:
    random_ = random.Random(3)

    for i, j, k in (
        (0, 0, 0),
        (1, 0, 0),
        (0, -1, 0),
        (0, 0, 2),
        (3, 4, 0),
        (1, 1, 1),
        # around the length that is taken as zero
        (0.0001, 0, 0),
        (0.00009999, 0, 0),
        (0.00010001, 0, 0),
        (0.00005, 0.00005, 0.00005),
        (0.00006, 0.00006, 0.00006),
        (1e-20, 0, 0),
        (1e18, 1e18, 1e18),
        (-1e-3, 2e-3, -3e-3),
    ):
        t.check("normalize3d", vector3d(i, j, k))
    for _ in range(300):
        scale = 10.0 ** random_.uniform(-6, 6)
        t.check("normalize3d", vector3d(*(random_.uniform(-scale, scale) for _ in range(3))))
