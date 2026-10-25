# four functions of the way to the main menu (guided by halo2-decompiled):
# whether a datum of the scenario makes an object, whether a weapon has a
# count, whether the animations of a unit have a bit, and what is wanted of
# an animation

import random
import struct

from difftest import Buffer

from test_object_types_more import after
from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
BIG = 0x400


def big_objects(random_) -> tuple[bytearray, list[int]]:
    """A table of objects of 0x400 bytes each."""
    table = bytearray(random_.randbytes(0x4C + COUNT * 12))
    struct.pack_into("<I", table, 0x44, 0x4C)
    pointers = [0x44]
    for index in range(COUNT):
        struct.pack_into("<I", table, 0x4C + index * 12 + 8, 0x4C + COUNT * 12 + index * BIG)
        pointers.append(0x4C + index * 12 + 8)
    return table + random_.randbytes(COUNT * BIG), pointers


def run(t) -> None:
    random_ = random.Random(859)

    for n in range(400):
        # a datum of the scenario: its palette entry, its kind kept and its number are the test's
        datum = bytearray(random_.randbytes(0x34))
        datum[4] = random_.choice((0, 1, 2, 3))
        struct.pack_into("<h", datum, 2, random_.choice((-1, -1, 0, 3)))
        datum = Buffer(bytes(datum))
        scenario = Buffer(random_.randbytes(0x10))
        entry = bytearray(random_.randbytes(8))
        struct.pack_into("<I", entry, 4, random_.choice((NONE, random_.getrandbits(32))))
        place = after(datum, scenario)
        t.check(
            "unit_unknown_000d59f0",
            datum,
            globals={"global_scenario": scenario},
            extra=[Buffer(bytes(entry))],
            externals={
                "object_unknown_000d58a0": lambda harness, arguments, found=random_.choice((0, place)): (found, tuple(arguments)),
                "unit_unknown_000d5990": lambda harness, arguments, kept=random_.choice((0, 1)): (kept, (arguments[0] & 0xFF, arguments[1])),
                "unit_unknown_000d5a70": lambda harness, arguments, number=random_.choice((NONE, 0, 1, 1, 2)): (number, tuple(arguments)),
            },
        )

        # a weapon and the definition of its tag
        table = some_objects(random_)
        objects = bytearray(table.data)
        index = random_.randrange(COUNT)
        struct.pack_into("<I", objects, 0x4C + COUNT * 12 + index * OBJECT, random_.randrange(4) | random_.getrandbits(16) << 16)
        tags = bytearray(random_.randbytes(4 * 16 + 4 * 0x200))
        for tag in range(4):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x40 + tag * 0x200)
            struct.pack_into("<h", tags, 0x40 + tag * 0x200 + 0x1FE, random_.choice((0, 0, 1, -1, 7)))
        t.check(
            "weapon_unknown_00100f70",
            random_.getrandbits(16) << 16 | index,
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag * 16 + 8 for tag in range(4))),
            },
            externals={"weapon_unknown_00100880": lambda harness, arguments, has=random_.choice((0, 1)): (has, tuple(arguments))},
        )

        # the two channels of a unit: their animations are the test's
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<h", units, at + 0x12A, 0x200)
        struct.pack_into("<h", units, at + 0x33E, 0x280)
        for channel in (at + 0x200, at + 0x280 + 0x9C):
            struct.pack_into("<IHH", units, channel, random_.randrange(4), random_.getrandbits(16), random_.choice((0xFFFF, 0, 1, 2)))
        units = Buffer(bytes(units), pointers=tuple(pointers))
        animations = bytearray(random_.randbytes(0x40))
        struct.pack_into("<HH", animations, 0x18, random_.getrandbits(16), random_.getrandbits(16))
        place = after(units)
        answers = [random_.choice((0, place, place + 2)) for _ in range(2)]

        def animation(harness, arguments, answers=answers):
            told = sum(1 for name, _ in harness._calls if name == "havok_unknown_001daea0")
            return answers[told % 2], tuple(arguments)

        t.check(
            "unit_unknown_00111590",
            random_.getrandbits(16) << 16 | index,
            random_.randrange(16),
            globals={"object_header_data": units, "global_tag_instances": Buffer(random_.randbytes(4 * 16))},
            extra=[Buffer(bytes(animations))],
            externals={"havok_unknown_001daea0": animation},
        )

        # what is wanted of an animation
        data = Buffer(random_.randbytes(0x0C))
        graph = Buffer(random_.randbytes(0x40))
        place = after(data, graph)
        t.check(
            "havok_unknown_001ddb40",
            data,
            graph,
            random_.choice((0xFFFF, 0, 1, 2)) << 16 | random_.getrandbits(16),
            extra=[Buffer(random_.randbytes(0x40))],
            externals={
                "havok_unknown_001daea0": lambda harness, arguments, place=place: (place, tuple(arguments)),
                "havok_unknown_001dd7c0": lambda harness, arguments, bytes_=random_.getrandbits(32): (bytes_, tuple(arguments)),
            },
        )
