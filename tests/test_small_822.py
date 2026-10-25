# three small functions of the way to the main menu: whether two reals are
# close, one of seventeen things by a real, and something of a unit in a
# seat

import random
import struct

from difftest import Buffer, GlobalData, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(822)

    for n in range(400):
        a = random_.uniform(-2, 2)
        b = random_.choice((a, a + 0.0005, a - 0.002, a + 0.001, random_.uniform(-2, 2)))
        first = f32(a) if n % 9 else random_.choice((0x7F800000, 0xFF800000, 0x7FC00000))
        t.check("real_math_unknown_000a0200", first, f32(b))
        t.check("real_math_unknown_000a0200", f32(0.001), f32(0.0))

        t.check(
            "rasterizer_unknown_0001c290",
            f32(random_.choice((0.0, 1.0, -0.5, 1.5, 0.5, 1 / 32, 3 / 32, random_.uniform(-0.2, 1.2)))),
            globals={"unknown_00487288": GlobalData(random_.randbytes(17 * 0x80))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT + 0x13C, random_.choice((NONE, 0, 5)))
        told = random_.randrange(2)
        t.check(
            "unit_unknown_000c4970",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"unit_unknown_000c48f0": lambda harness, arguments, told=told: (told, tuple(arguments))},
        )
