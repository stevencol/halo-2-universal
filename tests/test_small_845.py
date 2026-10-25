# six functions of the way to the main menu (guided by halo2-decompiled):
# the resources of the animations of a graph asked for, two counters of an
# animation state, a type of a part, permanent memory given back and the
# particle locations of a particle system deleted

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
ANIMATION = 0x6C
RESOURCE = 0x14
LOCATION = 0x34


def run(t) -> None:
    random_ = random.Random(845)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def graph() -> Buffer:
        data = bytearray(random_.randbytes(0xC0 + 3 * ANIMATION + 4 * RESOURCE))
        struct.pack_into("<I", data, 0x30, 0xC0)
        struct.pack_into("<iI", data, 0xAC, random_.choice((0, 1, 4)), 0xC0 + 3 * ANIMATION)
        for animation in range(3):
            at = 0xC0 + animation * ANIMATION
            data[at + 0x16] = random_.choice((0, 0x01, 0x10, 0x13))
            struct.pack_into("<i", data, at + 0x24, random_.randrange(4))
        for resource in range(4):
            data[0xC0 + 3 * ANIMATION + resource * RESOURCE + 0x10] = random_.choice((0, 1))
        return Buffer(bytes(data), pointers=(0x30, 0xB0))

    predict = {"animation_cache_predict": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF))}
    for n in range(400):
        t.check("havok_unknown_001dd840", random_.randrange(3), graph(), externals=predict)
        t.check("havok_unknown_001ddab0", graph(), externals=predict)

        state = bytearray(random_.randbytes(0x70))
        for at in (0x60, 0x64):
            state[at:at + 4] = bytes((random_.randrange(4), random_.choice((0, 3)), 0, random_.randrange(4)))
        t.check("havok_unknown_001cd4e0", Buffer(bytes(state)))

        t.check("real_math_unknown_0013d9f0", random_.choice((0, 1, 2, 3, 4, 5, 6, 7, NONE)))

        t.check(
            "input_abstraction_unknown_001565e0",
            globals={
                "unknown_004e9188": bytes((random_.choice((0, 1)),)),
                "unknown_004e925c": random_.choice((0, random_.getrandbits(32))),
                "unknown_004e91a0": random_.getrandbits(32),
                "unknown_004e9264": random_.getrandbits(32),
            },
            externals={"texture_cache_return_memory": lambda harness, arguments: (0, tuple(arguments))},
        )

        locations = bytearray(random_.randbytes(0x50 + 4 * LOCATION))
        struct.pack_into("<I", locations, 0x44, 0x50)
        order = random_.sample(range(4), random_.randrange(5))
        for place, location in enumerate(order):
            struct.pack_into("<I", locations, 0x50 + location * LOCATION + 0x0C, salted(order[place + 1]) if place + 1 < len(order) else NONE)
        system = bytearray(random_.randbytes(0x54))
        struct.pack_into("<I", system, 0x30, salted(order[0]) if order else NONE)
        t.check(
            "particle_system_unknown_001753f0",
            Buffer(bytes(system)),
            globals={"particle_location_data": Buffer(bytes(locations), pointers=(0x44,))},
            externals={"particle_location_delete": lambda harness, arguments: (0, tuple(arguments))},
        )
