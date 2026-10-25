# six functions of the way to the main menu (guided by halo2-decompiled): the
# rate of a particle, the animation of a name in the graph of a state, two
# parts of the animations of a unit made ready, a color between two, and
# the objects of the datums of the scenario

import random
import struct

from difftest import Buffer, f32

from test_small_859 import BIG, big_objects
from test_units import COUNT, OBJECT

NONE = 0xFFFFFFFF


def found_by_name(harness, arguments):
    """havok_unknown_001dd0b0: the id of a name is the name turned around."""
    name = arguments[2]
    harness.mu.mem_write(arguments[1], struct.pack("<I", (name * 0x9E3779B1) & 0xFFFFFFFF))
    return 0, (arguments[0], name)


def run(t) -> None:
    random_ = random.Random(866)

    for n in range(400):
        # the rate of a particle: its property at 0x08
        definition = bytearray(random_.randbytes(0x24))
        struct.pack_into("<hhHh", definition, 0x08, random_.randrange(8), random_.randrange(8), random_.choice((0, 1, 2)), random_.choice((-1, 0, 7)))
        struct.pack_into("<iI", definition, 0x10, 1, 0x18)
        definition[0x19] = random_.choice((0, 0x10))
        struct.pack_into("<2f", definition, 0x1C, random_.uniform(-2, 2), random_.uniform(-2, 2))
        t.check(
            "particle_unknown_00247100",
            Buffer(bytes(definition), pointers=(0x14,)),
            Buffer(struct.pack("<8f", *(random_.uniform(-5, 5) for _ in range(8)))),
            externals={"unknown_0013b390": lambda harness, arguments, value=f32(random_.uniform(-1, 2)): (value, tuple(arguments))},
        )

        # the animation of a name in the graph of a state
        state = bytearray(random_.randbytes(0x70))
        struct.pack_into("<I", state, 0x68, random_.randrange(2) | random_.getrandbits(16) << 16)
        tags = Buffer(random_.randbytes(2 * 16))
        t.check(
            "havok_unknown_001cbd10",
            Buffer(bytes(state)),
            Buffer(random_.randbytes(4)),
            random_.getrandbits(32),
            globals={"global_tag_instances": tags},
            externals={"havok_unknown_001dd0b0": found_by_name},
        )

        # two parts of the animations of a unit
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<hh", units, at + 0x12A, 0x200, 0)
        struct.pack_into("<h", units, at + 0x33E, 0x280)
        struct.pack_into("<I", units, at + 0x200 + 0x68, random_.randrange(2))
        units = Buffer(bytes(units), pointers=tuple(pointers))
        for name in ("unit_unknown_0010e920", "unit_unknown_0010dbc0"):
            t.check(
                name,
                random_.getrandbits(16) << 16 | index,
                globals={"object_header_data": units, "global_tag_instances": tags},
                externals={"havok_unknown_001dd0b0": found_by_name},
            )

        # a color between two: the game's hsv_to_rgb takes no hue of exactly 1 (its table of sectors has six), so t is
        # never 0 or 1 (with a gray color, its hue 0 gone up to 1 would be the one) and the colors of any value
        choices = (0.0, random_.uniform(0, 1), random_.uniform(0, 1), random_.uniform(0, 1))
        t.check(
            "real_math_unknown_00131c20",
            Buffer(struct.pack("<3f", *(random_.choice(choices) for _ in range(3)))),
            Buffer(struct.pack("<3f", *(random_.choice(choices) for _ in range(3)))),
            random_.choice((0, 1, 2, 3, 0xFFFFFFFC, 0xFFFFFFFD)),
            f32(random_.uniform(0.001, 0.999)),
            Buffer(random_.randbytes(12)),
        )

        # the objects of the datums of the scenario: a table of objects with its bits
        if n < 100:
            count = random_.choice((0, 3, COUNT))
            table = bytearray(random_.randbytes(0x4C + COUNT * 12 + 4 + COUNT * OBJECT))
            struct.pack_into("<i", table, 0x24, 12)
            struct.pack_into("<i", table, 0x38, count)
            struct.pack_into("<II", table, 0x44, 0x4C, 0x4C + COUNT * 12)
            struct.pack_into("<I", table, 0x4C + COUNT * 12, random_.getrandbits(32))
            table_pointers = [0x44, 0x48]
            for object_ in range(COUNT):
                entry = 0x4C + object_ * 12
                struct.pack_into("<hBBh", table, entry, random_.randrange(-3, 0x7FFF), random_.choice((0, 0x10, 0x01)), random_.choice((6, 6, 0, 33)), random_.choice((-1, 0, 2)))
                struct.pack_into("<I", table, entry + 8, 0x4C + COUNT * 12 + 4 + object_ * OBJECT)
                table_pointers.append(entry + 8)
                struct.pack_into("<h", table, 0x4C + COUNT * 12 + 4 + object_ * OBJECT + 0x1A, random_.choice((-1, 0, 1, 2, 3)))
            scenario = bytearray(random_.randbytes(0x60 + 4 * 0x5C))
            struct.pack_into("<I", scenario, 0x54, 0x60)
            for datum in range(4):
                struct.pack_into("<h", scenario, 0x60 + datum * 0x5C + 0x58, random_.choice((0, 1, 2, 7, 2000)))
            answers = [random_.choice((0, 1, 1)) for _ in range(COUNT)]

            def made(harness, arguments, answers=answers):
                told = sum(1 for name, _ in harness._calls if name == "unit_unknown_000d5910")
                return answers[told % len(answers)], (arguments[0], arguments[1] & 0xFF)

            t.check(
                "havok_unknown_001edc10",
                globals={
                    "object_header_data": Buffer(bytes(table), pointers=tuple(table_pointers)),
                    "global_scenario": Buffer(bytes(scenario), pointers=(0x54,)),
                    "unknown_0051e9cc": Buffer(random_.randbytes(8000)),
                },
                externals={"unit_unknown_000d5910": made},
            )
