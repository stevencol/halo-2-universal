# six small functions of the way to the main menu: something of an effect
# as it starts, the particle systems of an effect of two kinds, entries
# from some things, the next effect location of a kind, things of a havok
# component marked with the time, and the sides of a plane a point is on

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF
SYSTEM = 0x54
LOCATION = 0x3C
COMPONENT = 0xA0


def run(t) -> None:
    random_ = random.Random(831)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        t.check(
            "effect_unknown_00176a50",
            Buffer(random_.randbytes(0x60)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice((0, 1, 2)),
        )

        systems = bytearray(random_.randbytes(0x50 + 4 * SYSTEM))
        struct.pack_into("<I", systems, 0x44, 0x50)
        order = random_.sample(range(4), random_.randrange(5))
        for place, system in enumerate(order):
            at = 0x50 + system * SYSTEM
            struct.pack_into("<i", systems, at + 0x14, random_.choice((1, 2, 3)))
            struct.pack_into("<I", systems, at + 0x38, salted(order[place + 1]) if place + 1 < len(order) else NONE)
        effect = bytearray(random_.randbytes(0x100))
        struct.pack_into("<I", effect, 0xFC, salted(order[0]) if order else NONE)
        struct.pack_into("<hh", effect, 0x5C, 1, 2)
        struct.pack_into("<f", effect, 0x64, random_.choice((0.0, 0.5, 2.0, -1.0)))
        t.check("effect_unknown_0017add0", Buffer(bytes(effect)), globals={"particle_system_data": Buffer(bytes(systems), pointers=(0x44,))})

        count = random_.randrange(4)
        data = bytearray(random_.randbytes(0x50 + 3 * 0x60))
        struct.pack_into("<iI", data, 0x48, count, 0x50)
        t.check("unknown_0016d940", Buffer(bytes(data), pointers=(0x4C,)), Buffer(random_.randbytes(3 * 0x20)))

        locations = bytearray(random_.randbytes(0x50 + 4 * LOCATION))
        struct.pack_into("<I", locations, 0x44, 0x50)
        order = random_.sample(range(4), random_.randrange(5))
        for place, location in enumerate(order):
            at = 0x50 + location * LOCATION
            struct.pack_into("<hI", locations, at + 2, random_.choice((-1, 0, 5, -0x8000, -2)), salted(order[place + 1]) if place + 1 < len(order) else NONE)
        t.check(
            "effect_location_unknown_0017ae50",
            random_.getrandbits(32),
            Buffer(struct.pack("<I", salted(order[0]) if order else NONE)),
            random_.choice((0, 1, 2, 3)) | random_.getrandbits(16) << 16,
            globals={"effect_location_data": Buffer(bytes(locations), pointers=(0x44,))},
        )

        # a havok component with three elements, each pointing at something that may have a thing at 0x44
        components = bytearray(random_.randbytes(0x50 + 2 * COMPONENT + 3 * 0x60 + 3 * 0x50 + 3 * 0xB0))
        struct.pack_into("<I", components, 0x44, 0x50)
        elements = 0x50 + 2 * COMPONENT
        holders = elements + 3 * 0x60
        things = holders + 3 * 0x50
        pointers = [0x44]
        for component in range(2):
            struct.pack_into("<Ii", components, 0x50 + component * COMPONENT + 0x70, elements, random_.choice((0, 1, 3)))
            pointers.append(0x50 + component * COMPONENT + 0x70)
        time = random_.getrandbits(32)
        for element in range(3):
            struct.pack_into("<I", components, elements + element * 0x60 + 0x40, holders + element * 0x50)
            pointers.append(elements + element * 0x60 + 0x40)
            if random_.random() < 0.8:
                struct.pack_into("<I", components, holders + element * 0x50 + 0x44, things + element * 0xB0)
                pointers.append(holders + element * 0x50 + 0x44)
            else:
                struct.pack_into("<I", components, holders + element * 0x50 + 0x44, 0)
            struct.pack_into("<I", components, things + element * 0xB0 + 0xA8, random_.choice((time, time - 1)))
        clock = bytearray(random_.randbytes(0x24))
        struct.pack_into("<I", clock, 8, time)
        t.check(
            "havok_component_unknown_001c4fc0",
            random_.choice((NONE, salted(0), salted(1))),
            globals={"havok_component_data": Buffer(bytes(components), pointers=tuple(pointers)), "game_time_globals": Buffer(bytes(clock))},
        )

        data = bytearray(random_.randbytes(0x2A98))
        struct.pack_into("<I", data, 0x2A60, random_.choice((0, 0x200, 0x3FF)))
        if random_.random() < 0.5:
            plane = (0.0, 0.0, 1.0, 1.0)
            point = (random_.uniform(-2, 2), random_.uniform(-2, 2), random_.choice((1.0, 1.0, 0.9, 1.1)))
        else:
            plane = (random_.uniform(-1, 1), random_.uniform(-1, 1), random_.uniform(-1, 1), random_.uniform(-1, 1))
            point = tuple(random_.uniform(-2, 2) for _ in range(3))
        struct.pack_into("<4f", data, 0x2A88, *plane)
        t.check(
            "unknown_00132fd0",
            Buffer(bytes(data)),
            Buffer(struct.pack("<3f", *point)),
            Buffer(random_.randbytes(1)),
            Buffer(random_.randbytes(1)),
            f32(random_.choice((0.0, 0.1, 0.5, -1.0))),
        )
