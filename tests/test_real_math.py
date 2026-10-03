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
    run_segments(t)
    run_quantize(t)
    run_angles(t)
    random_ = random.Random(3)

    vectors = list(VECTORS)
    for _ in range(300):
        scale = 10.0 ** random_.uniform(-6, 6)
        vectors.append(tuple(random_.uniform(-scale, scale) for _ in range(3)))

    for path in (b"", b"a", b"\\", b"a\\b", b"tags\\objects\\weapons\\rifle", b"no backslash here", b"ends\\", b"\\\\x"):
        t.check("path_remove_last_element", Buffer(path + b"\0"))

    for _ in range(20):
        bits = Buffer(random_.randbytes(16))
        for bit in (0, 1, 31, 32, 33, 63, 64, 127, *(random_.randrange(128) for _ in range(6))):
            t.check("bit_vector_test_flag", bits, bit)

    for vector in vectors:
        t.check("normalize3d_with_default", reals(*vector), reals(0.25, -0.5, 0.75))
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
        t.check("fast_vector_sphere_intersection", reals(*point), reals(*direction), reals(*center), f32(radius))

        length = random_.choice((1.0, 1.0, 0.01, 30.0))
        t.check("fast_quaternion_normalize", reals(*(random_.uniform(-1, 1) * length for _ in range(4))))

    for _ in range(200):
        length = random_.choice((1.0, 1.0, 0.01, 30.0, 0.0))
        t.check("quaternion_normalize", reals(*(random_.uniform(-1, 1) * length for _ in range(4))))

        t.check(
            "quaternion_multiply",
            reals(*(random_.uniform(-1, 1) for _ in range(4))),
            reals(*(random_.uniform(-1, 1) for _ in range(4))),
            reals(9, 9, 9, 9),
        )
        t.check(
            "fast_quaternion_interpolate_and_normalize",
            reals(*(random_.uniform(-1, 1) for _ in range(4))),
            reals(*(random_.uniform(-1, 1) for _ in range(4))),
            f32(random_.choice((0.0, 1.0, 0.5, random_.uniform(0, 1)))),
            reals(9, 9, 9, 9),
        )
        # two directions: any, the same one, and opposite ones
        direction = random_.choice(vectors[1:6])
        length = sum(component * component for component in direction) ** 0.5
        direction = [component / length for component in direction]
        other = random_.choice((direction, [-component for component in direction], [0.0, 0.6, 0.8], [1.0, 0.0, 0.0]))
        t.check("quaternion_from_vectors3d", reals(9, 9, 9, 9), reals(*direction), reals(*other))
        bounds = [random_.uniform(-50, 50) for _ in range(6)]
        t.check("rectangle3d_build_vertices", reals(*bounds), reals(*([9.0] * 24)))

        angles = [random_.uniform(-7, 7) for _ in range(3)]
        t.check("vectors3d_from_euler_angles3d", reals(9, 9, 9), reals(8, 8, 8), reals(*angles))
        t.check("vectors3d_from_euler_angles2d", reals(9, 9, 9), reals(8, 8, 8), reals(*angles[:2]))

        # a point and a plane, on each projection and each sign; some planes
        # do not face the axis of the projection at all
        point = [random_.uniform(-20, 20) for _ in range(3)]
        normal = [random_.choice((0.0, 0.00005, random_.uniform(-1, 1))) for _ in range(3)]
        plane = reals(*normal, random_.uniform(-10, 10))
        for projection in range(3):
            for sign in (0, 1):
                t.check("project_point3d", reals(*point), projection, sign, reals(9, 9))
                t.check("project_point2d", reals(*point[:2]), plane, projection, sign, reals(9, 9, 9))

    for seed in (0, 1, 0xFFFFFFFF, 0x80000000, 0x12345678, *(random_.getrandbits(32) for _ in range(200))):
        t.check("real_seed_random", Buffer(struct.pack("<I", seed)))
        lower = random_.uniform(-100, 100)
        upper = lower + random_.choice((0.0, 1.0, random_.uniform(0, 500), -random_.uniform(0, 5)))
        t.check("real_seed_random_range", Buffer(struct.pack("<I", seed)), f32(lower), f32(upper))


def run_angles(t) -> None:
    import math

    random_ = random.Random(116)

    def unit() -> list[float]:
        while True:
            v = [random_.uniform(-1, 1) for _ in range(3)]
            length = math.sqrt(sum(x * x for x in v))
            if length > 0.1:
                return [x / length for x in v]

    special = ([0.0, 0.0, 1.0], [0.0, 0.0, -1.0], [1.0, 0.0, 0.0], [-0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [2.0, 0.0, 0.0], [1e-5, 1e-5, 1.0])
    for n in range(400):
        a = random_.choice(special) if n % 5 == 0 else unit()
        choice = n % 4
        b = list(a) if choice == 0 else [-x for x in a] if choice == 1 else random_.choice(special) if n % 7 == 0 else unit()
        t.check("angle_between_vectors3d", reals(*a), reals(*b))
        # zeros with a sign show the order of the operations
        signed = ([0.0, 1.0, 0.0], [-0.0, 1.0, -0.0], [0.0, -1.0, 0.0], [-0.0, -1.0, 0.0])
        forward = random_.choice(signed) if n % 4 == 1 else random_.choice(special) if n % 3 == 0 else unit()
        t.check(
            "generate_up_vector3d",
            reals(*forward),
            # the bytes: a zero has a sign
            Buffer(struct.pack("<3f", 9.0, 9.0, 9.0)),
            globals={"global_forward3d": reals(*random_.choice(([1.0, 0.0, 0.0], [0.25, -2.0, 3.5])))},
        )
    # two vectors that are the same but for k
    for a, b in (([0.0, 0.0, 1.0], [0.0, 0.0, -1.0]), ([0.6, 0.8, 0.0], [0.6, 0.8, 1e-3]), ([0.0, 1.0, 0.0], [0.0, 1.0, -0.0])):
        t.check("angle_between_vectors3d", reals(*a), reals(*b))


def run_quantize(t) -> None:
    random_ = random.Random(117)

    def bits(value: float) -> int:
        return struct.unpack("<I", struct.pack("<f", value))[0]

    for n in range(1500):
        minimum = random_.choice((0.0, 0.0, -1.0, 2.5, -100.0))
        maximum = minimum + random_.choice((1.0, 1.0, 0.5, 100.0, 1e-5, 0.0, 1e-4, 1.0001e-4, -1.0))
        choice = random_.randrange(6)
        if choice == 0:
            value = minimum + (maximum - minimum) * random_.randrange(256) / 255.0
        elif choice == 1:
            value = random_.choice((minimum, maximum, minimum - 1.0, maximum + 1.0, maximum * 3.0 + 7.0))
        else:
            value = random_.uniform(minimum - 0.1, maximum + 0.1)
        t.check("quantize_real_to_byte", bits(minimum), bits(maximum), bits(value))
    # the most of ranges whose start and size do not add up to it exactly
    for n in range(400):
        minimum = struct.unpack("<f", struct.pack("<f", random_.uniform(-50, 50)))[0]
        maximum = struct.unpack("<f", struct.pack("<f", minimum + random_.uniform(0.001, 30)))[0]
        t.check("quantize_real_to_byte", bits(minimum), bits(maximum), bits(maximum))


def run_segments(t) -> None:
    random_ = random.Random(118)

    for n in range(800):
        start = [random_.uniform(-20, 20) for _ in range(3)]
        choice = n % 5
        if choice == 0:
            direction = random_.choice(([0.0, 0.0, 0.0], [1e-3, 0.0, 0.0], [0.0, 0.0, 0.01], [0.006, 0.006, 0.006]))
        else:
            direction = [random_.uniform(-10, 10) for _ in range(3)]
        # before the start, along the segment and past its end
        along = random_.choice((-0.5, 0.0, 0.25, 0.5, 1.0, 1.5, random_.uniform(-2, 3)))
        point = [s_ + d * along + random_.uniform(-3, 3) for s_, d in zip(start, direction)]
        if n % 17 == 16:
            point = list(start)
        t.check("point_to_segment_distance_squared3d", reals(*direction), reals(*start), reals(*point))
