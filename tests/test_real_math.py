# source/math/real_math.c and source/math/random_math.c

import random
import struct

from difftest import Buffer, f32


def reals(*values: float) -> Buffer:
    return Buffer(struct.pack(f"<{len(values)}f", *values), format=f"<{len(values)}f")


VECTORS = (
    (0, 0, 0),
    (1, 0, 0),
    (0, -1, 0),
    (0, 0, 2),
    (3, 4, 0),
    (1, 1, 1),
    (1, 1, 0),
    (0, 1, 1),
    (1, 0, 1),
    (-2, 2, 2),
    (2, -2, 3),
    # around the length that is taken as zero
    (0.0001, 0, 0),
    (0.00009999, 0, 0),
    (0.00010001, 0, 0),
    (0.00005, 0.00005, 0.00005),
    (0.00006, 0.00006, 0.00006),
    (1e-20, 0, 0),
    (1e18, 1e18, 1e18),
    (-1e-3, 2e-3, -3e-3),
)


def run(t) -> None:
    random_ = random.Random(3)

    vectors = list(VECTORS)
    for _ in range(300):
        scale = 10.0 ** random_.uniform(-6, 6)
        vectors.append(tuple(random_.uniform(-scale, scale) for _ in range(3)))

    for vector in vectors:
        t.check("normalize3d", reals(*vector))
        t.check("normalize2d", reals(*vector[:2]))
        t.check("perpendicular3d", reals(*vector), reals(9, 9, 9))
        # the game takes these two with the x87 instructions, which keep a
        # square too small or too large for a real; source/ does not, so a
        # vector that short or that long gives another result
        if all(component == 0 or 1e-15 < abs(component) < 1e15 for component in vector):
            t.check("magnitude3d", reals(*vector))
            other = random_.choice(vectors[:11])
            t.check("distance3d", reals(*vector), reals(*other))

    for seed in (0, 1, 0xFFFFFFFF, 0x80000000, 0x12345678, *(random_.getrandbits(32) for _ in range(200))):
        t.check("real_seed_random", Buffer(struct.pack("<I", seed)))
        lower = random_.uniform(-100, 100)
        upper = lower + random_.choice((0.0, 1.0, random_.uniform(0, 500), -random_.uniform(0, 5)))
        t.check("real_seed_random_range", Buffer(struct.pack("<I", seed)), f32(lower), f32(upper))
