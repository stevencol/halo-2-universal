# source/math/matrix_math.c

import math
import random
import struct

from difftest import Buffer, f32


def reals(*values: float) -> Buffer:
    return Buffer(struct.pack(f"<{len(values)}f", *values), format=f"<{len(values)}f")


def garbage(random_: random.Random, count: int) -> Buffer:
    """Room for a result of some reals, with anything in it."""
    return reals(*(random_.uniform(-9, 9) for _ in range(count)))


def unit(random_: random.Random) -> tuple[float, float, float]:
    while True:
        v = [random_.uniform(-1, 1) for _ in range(3)]
        length = math.sqrt(sum(x * x for x in v))
        if length > 0.1:
            return tuple(x / length for x in v)


def rotation(random_: random.Random) -> list[float]:
    """The forward, left and up vectors of a rotation."""
    forward = unit(random_)
    while True:
        other = unit(random_)
        up = [
            forward[1] * other[2] - forward[2] * other[1],
            forward[2] * other[0] - forward[0] * other[2],
            forward[0] * other[1] - forward[1] * other[0],
        ]
        length = math.sqrt(sum(x * x for x in up))
        if length > 0.1:
            break
    up = [x / length for x in up]
    left = [
        up[1] * forward[2] - up[2] * forward[1],
        up[2] * forward[0] - up[0] * forward[2],
        up[0] * forward[1] - up[1] * forward[0],
    ]
    return [*forward, *left, *up]


def matrix4x3(random_: random.Random, scale: float | None = None) -> list[float]:
    if scale is None:
        scale = random_.choice((1.0, 1.0, 0.5, 2.0, 3.25, -1.0))
    # some matrices are not rotations
    n = rotation(random_) if random_.random() < 0.7 else [random_.uniform(-3, 3) for _ in range(9)]
    return [scale, *n, *(random_.uniform(-50, 50) for _ in range(3))]


def run(t) -> None:
    random_ = random.Random(4)

    for _ in range(150):
        matrix = matrix4x3(random_)
        other = matrix4x3(random_)
        vector = [random_.uniform(-20, 20) for _ in range(3)]

        t.check("matrix4x3_inverse", reals(*matrix), garbage(random_, 13))
        t.check("matrix4x3_multiply", reals(*matrix), reals(*other), garbage(random_, 13))
        t.check("matrix4x3_transform_point", reals(*matrix), reals(*vector), garbage(random_, 3))
        t.check("matrix4x3_transform_vector", reals(*matrix), reals(*vector), garbage(random_, 3))
        t.check("matrix4x3_inverse_transform_point", reals(*matrix), reals(*vector), garbage(random_, 3))
        t.check("matrix4x3_inverse_transform_vector", reals(*matrix), reals(*vector), garbage(random_, 3))
        t.check(
            "matrix4x3_inverse_transform_plane",
            reals(*matrix),
            reals(*unit(random_), random_.uniform(-30, 30)),
            garbage(random_, 4),
        )

        count = random_.randint(0, 5)
        points = [random_.uniform(-20, 20) for _ in range(3 * count)] or [0.0]
        t.check("matrix4x3_transform_points", reals(*matrix), count, reals(*points), garbage(random_, max(3 * count, 1)))

        t.check("matrix4x3_translation", garbage(random_, 13), reals(*vector))
        t.check("matrix4x3_rotation_from_vectors", garbage(random_, 13), reals(*unit(random_)), reals(*unit(random_)))
        t.check(
            "matrix4x3_from_point_and_vectors",
            garbage(random_, 13),
            reals(*vector),
            reals(*unit(random_)),
            reals(*unit(random_)),
        )
        t.check("matrix3x3_from_forward_and_up", garbage(random_, 9), reals(*unit(random_)), reals(*unit(random_)))

        angles = [f32(random_.uniform(-7, 7)) for _ in range(3)]
        t.check("matrix4x3_rotation_from_angles", garbage(random_, 13), *angles)
        t.check("matrix3x3_from_angles", garbage(random_, 9), *angles)

        matrix3 = matrix[1:10]
        t.check("matrix3x3_multiply", reals(*matrix3), reals(*other[1:10]), garbage(random_, 9))
        # a plane that faces any way, and the planes that face along an axis
        normal = random_.choice((unit(random_), (1.0, 0.0, 0.0), (0.0, -1.0, 0.0), (0.0, 0.0, 1.0)))
        t.check("matrix4x3_from_plane", garbage(random_, 13), reals(*normal, random_.uniform(-30, 30)))
        t.check("matrix3x3_transpose", reals(*matrix3), garbage(random_, 9))
        t.check("matrix3x3_inverse", reals(*matrix3), f32(random_.choice((1.0, -1.0, 0.5, 7.0))), garbage(random_, 9))
        t.check("matrix3x3_transform_vector", reals(*matrix3), reals(*vector), garbage(random_, 3))
        t.check("matrix3x3_rotation_to_quaternion", reals(*matrix3), garbage(random_, 4))

        # a quaternion of any length, and some too short to be a rotation
        length = random_.choice((1.0, 1.0, 0.3, 4.0, 0.001, 0.0))
        quaternion = [random_.uniform(-1, 1) * length for _ in range(4)]
        t.check("matrix3x3_rotation_from_quaternion", garbage(random_, 9), reals(*quaternion))
        t.check(
            "matrix4x3_from_orientation",
            garbage(random_, 13),
            reals(*quaternion, *vector, random_.choice((1.0, 0.5, 2.0))),
        )

    # the matrices that the functions treat apart: no scale and scale 1
    for scale in (0.0, 1.0):
        matrix = matrix4x3(random_, scale)
        vector = [1.5, -2.5, 4.0]
        t.check("matrix4x3_inverse", reals(*matrix), garbage(random_, 13))
        t.check("matrix4x3_transform_point", reals(*matrix), reals(*vector), garbage(random_, 3))
        t.check("matrix4x3_inverse_transform_point", reals(*matrix), reals(*vector), garbage(random_, 3))
        t.check("matrix4x3_inverse_transform_plane", reals(*matrix), reals(0, 0, 1, 5), garbage(random_, 4))

    # the rotations whose quaternion comes from each element of the diagonal
    for diagonal in ((1, 1, 1), (1, -1, -1), (-1, 1, -1), (-1, -1, 1), (-1, -1, -1), (0, 0, 0)):
        matrix3 = [diagonal[0], 0, 0, 0, diagonal[1], 0, 0, 0, diagonal[2]]
        t.check("matrix3x3_rotation_to_quaternion", reals(*matrix3), garbage(random_, 4))

    # ---------- matrices, quaternions and orientations

    PI = struct.unpack("<f", struct.pack("<f", math.pi))[0]

    def angles() -> dict:
        return {
            "real_math_pi": struct.pack("<f", random_.choice((math.pi, math.pi, 1.0, 3.5))),
            "real_math_two_pi": struct.pack("<f", random_.choice((2 * math.pi, 2 * math.pi, 7.0))),
        }

    def quaternion() -> list[float]:
        choice = random_.randrange(7)
        if choice == 6:
            # half a turn: exactly pi
            return random_.choice(([1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0], [0.0, 0.0, -1.0, 0.0]))
        if choice == 0:
            return [0.0, 0.0, 0.0, random_.choice((1.0, -1.0))]
        if choice == 1:
            return [random_.uniform(-2, 2) for _ in range(4)]
        q = [random_.uniform(-1, 1) for _ in range(4)]
        length = math.sqrt(sum(x * x for x in q)) or 1.0
        return [x / length for x in q]

    for _ in range(300):
        t.check("matrix4x3_from_point_and_quaternion", garbage(random_, 13), reals(*(random_.uniform(-50, 50) for _ in range(3))), reals(*quaternion()))
        t.check("matrix4x3_to_orientation", reals(*matrix4x3(random_)), garbage(random_, 8))
    for _ in range(300):
        t.check("quaternion_to_axis_angle", reals(*quaternion()), garbage(random_, 3), garbage(random_, 1), globals=angles())
    for _ in range(300):
        t.check(
            "matrix4x3_rotation_between",
            garbage(random_, 3),
            reals(*matrix4x3(random_, 1.0)),
            reals(*matrix4x3(random_, random_.choice((1.0, 1.0, 2.0)))),
            globals=angles(),
        )
