# four functions of the way to the main menu (guided by halo2-decompiled):
# the animations of a weapon asked for, a blend of an animation state that
# starts, a color from hue, saturation and value, and an entry of an object
# set

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(848)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        t.check(
            "havok_unknown_001ddc70",
            random_.choice((0, 1)),
            Buffer(random_.randbytes(0x40)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice((0, 1)),
            externals={"havok_unknown_001ddb90": lambda harness, arguments: (0, (arguments[0] & 0xFF,) + tuple(arguments[1:5]) + (arguments[5] & 0xFF,))},
        )

        state = bytearray(random_.randbytes(0x70))
        state[0x64:0x68] = bytes((random_.randrange(4), random_.choice((0, 3)), 0, random_.randrange(4)))
        count = random_.choice((0, 1, 3))
        length = random_.choice((0.5, 1.0, 2.0))
        t.check(
            "havok_unknown_001cb5f0",
            count,
            Buffer(bytes(state)),
            f32(random_.choice((0.0, 0.5, 1.0, 2.0, -1.0))),
            Buffer(random_.randbytes(3 * 0x20)),
            Buffer(random_.randbytes(3 * 0x20)),
            externals={
                "unit_counter_seconds": lambda harness, arguments, length=length: (f32(length), tuple(arguments)),
                "unit_counter_start": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF, arguments[2])),
            },
        )

        hsv = [random_.choice((0.0, 0.1, 0.4, 0.5, 0.7, 0.9, random_.uniform(0, 0.999))), random_.choice((0.0, 0.5, 1.0)), random_.uniform(0, 1)]
        t.check("real_math_hsv_to_rgb", Buffer(struct.pack("<3f", *hsv)), Buffer(random_.randbytes(12)))

        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x100))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<hh", objects, at + 0x124, random_.choice((0, 24, 48, 72)), 0x140)
        t.check(
            "object_unknown_000bacc0",
            salted(random_.randrange(COUNT)),
            random_.choice((-1, 0, 1, 2, 3)),
            Buffer(random_.randbytes(12)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"cached_object_render_state_unknown_0003dd10": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF))},
        )
