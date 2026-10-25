# four small functions of the way to the main menu: the particles of a chain
# of locations added, a chain of particles deleted, a pair of an effect
# cleared, and 3t^2-2t^3

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF
LOCATION = 0x34
EMITTER = 0x4C
PARTICLE = 0x40
EFFECT = 0x190


def run(t) -> None:
    random_ = random.Random(821)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def chain(data: bytearray, first: int, size: int, count: int, at: int) -> int:
        """Some of `count` entries chained by the long at `at`; the first of them, or NONE."""
        order = random_.sample(range(count), random_.randrange(count))
        for place, entry in enumerate(order):
            following = order[place + 1] if place + 1 < len(order) else None
            struct.pack_into("<I", data, first + entry * size + at, NONE if following is None else salted(following))
        return salted(order[0]) if order else NONE

    for n in range(400):
        # locations (each with its emitters at 0x04) and emitters (each with a short at 0x02)
        emitters = bytearray(random_.randbytes(0x50 + 6 * EMITTER))
        struct.pack_into("<I", emitters, 0x44, 0x50)
        locations = bytearray(random_.randbytes(0x50 + 4 * LOCATION))
        struct.pack_into("<I", locations, 0x44, 0x50)
        for location in range(4):
            struct.pack_into("<I", locations, 0x50 + location * LOCATION + 4, chain(emitters, 0x50, EMITTER, 6, 8))
        data = bytearray(random_.randbytes(0x34))
        struct.pack_into("<I", data, 0x30, chain(locations, 0x50, LOCATION, 4, 0x0C))
        t.check(
            "particle_unknown_001753a0",
            Buffer(bytes(data)),
            globals={
                "particle_location_data": Buffer(bytes(locations), pointers=(0x44,)),
                "particle_emitter_data": Buffer(bytes(emitters), pointers=(0x44,)),
            },
        )

        particles = bytearray(random_.randbytes(0x50 + 6 * PARTICLE))
        struct.pack_into("<I", particles, 0x44, 0x50)
        data = bytearray(random_.randbytes(8))
        struct.pack_into("<I", data, 4, chain(particles, 0x50, PARTICLE, 6, 4))
        t.check(
            "particle_unknown_002483b0",
            Buffer(bytes(data)),
            globals={"particle_data": Buffer(bytes(particles), pointers=(0x44,))},
            externals={"datum_delete": lambda harness, arguments: (0, tuple(arguments))},
        )

        effects = bytearray(random_.randbytes(0x50 + 3 * EFFECT))
        struct.pack_into("<I", effects, 0x44, 0x50)
        value = random_.choice((NONE, 7, 9))
        for effect in range(3):
            for pair in range(16):
                struct.pack_into("<I", effects, 0x50 + effect * EFFECT + 0x110 + pair * 8 + 4, random_.choice((value, 1, 2, 3, 4, 5, 6)))
        t.check(
            "effect_unknown_00178b80",
            salted(random_.randrange(3)),
            value,
            globals={"effect_data": Buffer(bytes(effects), pointers=(0x44,))},
        )

        t.check("real_math_unknown_0010e8e0", f32(random_.choice((0.0, 1.0, 0.5, -0.5, 1.5, random_.uniform(-2, 2)))))
