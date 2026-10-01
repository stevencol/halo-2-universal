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

    for vector in vectors:
        t.check("projection_from_vector3d", reals(*vector))
        if all(component == 0 or 1e-15 < abs(component) < 1e15 for component in vector):
            t.check("euler_angles2d_from_vector3d", reals(9, 9), reals(*vector))
            # a length under the vector's, over it, and none
            for length in (0.0, 0.5, 1.0, 7.0, random_.uniform(0, 20)):
                t.check("limit3d", reals(*vector), f32(length))
    for _ in range(400):
        # a point, a direction, and a sphere near them or far
        point = [random_.uniform(-5, 5) for _ in range(3)]
        direction = [random_.uniform(-8, 8) for _ in range(3)]
        center = [random_.uniform(-5, 5) for _ in range(3)]
        radius = random_.choice((0.0, 0.5, 2.0, 6.0, random_.uniform(0, 10)))
        t.check("fast_vector_intersects_sphere", reals(*point), reals(*direction), reals(*center), f32(radius))

        length = random_.choice((1.0, 1.0, 0.01, 30.0))
        t.check("fast_quaternion_normalize", reals(*(random_.uniform(-1, 1) * length for _ in range(4))))

    for seed in (0, 1, 0xFFFFFFFF, 0x80000000, 0x12345678, *(random_.getrandbits(32) for _ in range(200))):
        t.check("real_seed_random", Buffer(struct.pack("<I", seed)))
        lower = random_.uniform(-100, 100)
        upper = lower + random_.choice((0.0, 1.0, random_.uniform(0, 500), -random_.uniform(0, 5)))
        t.check("real_seed_random_range", Buffer(struct.pack("<I", seed)), f32(lower), f32(upper))
