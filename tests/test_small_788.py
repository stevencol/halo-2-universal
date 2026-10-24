# eleven small functions of the objects and the units: the entries at the
# short at 0x11E, two reals and the up vector, a real of the definition, the
# sound of a definition, a weapon in a hand, sixteen shorts, the kind of a
# definition, a byte cleared, and a bit of 0x3B4

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x400


def run(t) -> None:
    random_ = random.Random(788)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def told(harness, arguments):
        return 0, tuple(arguments)

    def tags() -> Buffer:
        data = bytearray(random_.randbytes(0x40 + 4 * 0x240))
        for tag in range(4):
            at = 0x40 + tag * 0x240
            struct.pack_into("<I", data, tag * 0x10 + 8, at)
            struct.pack_into("<I", data, at + 0x138, random_.choice((NONE, 3, 7)))
            struct.pack_into("<h", data, at + 0x1F0, random_.choice((3, 4, 4, 5, -1)))
            struct.pack_into("<h", data, at + 0x218, random_.choice((0, 1, 1, 2)))
        return Buffer(bytes(data), pointers=tuple(tag * 0x10 + 8 for tag in range(4)))

    for n in range(400):
        table = some_objects(random_)
        headers = bytearray(table.data[: 0x4C + COUNT * 12])
        objects = bytearray()
        for index in range(COUNT):
            one = bytearray(table.data[FIRST + index * OBJECT : FIRST + (index + 1) * OBJECT] + random_.randbytes(BIG - OBJECT))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            struct.pack_into("<hh", one, 0x11C, random_.choice((0, 7, 8, 32, 40)), 0x300)
            struct.pack_into("<i", one, 0x14C, random_.choice((-1, 0, 1, 3, 4, 5, -2)))
            for entry in range(5):
                one[0x300 + entry * 8] = random_.choice((3, 3, 2))
            one[0x10A] = random_.choice((0, 4, 0xFB, 0xFF))
            one[0x16C] = random_.choice((0, 2, 4, 6, 0xFB))
            one[0x212] = random_.choice((0, 1, 3, 0xFF))
            one[0x213] = random_.choice((0, 2, 0xFF))
            for weapon in range(4):
                struct.pack_into("<I", one, 0x218 + weapon * 4, random_.choice((NONE, salted(random_.randrange(COUNT)))))
            objects += one
        units = lambda: {"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers)}
        unit = salted(random_.randrange(COUNT))

        t.check("object_unknown_000fd410", unit, globals=units())
        t.check("object_unknown_000fd460", unit, random_.getrandbits(32), globals=units())

        t.check(
            "object_unknown_000edfa0",
            unit,
            Buffer(struct.pack("<2I", f32(random_.uniform(-5, 5)), random_.getrandbits(32))),
            globals={**units(), "global_up3d": Buffer(random_.randbytes(12))},
        )
        t.check("object_unknown_000fbfd0", unit, globals={**units(), "global_tag_instances": tags()})
        t.check("object_definition_unknown_000f8160", salted(random_.randrange(4)), globals={"global_tag_instances": tags()}, externals={"sound_unknown_001896c0": told})
        t.check("object_unknown_000f8110", unit, globals={**units(), "global_tag_instances": tags()}, externals={"sound_unknown_001896c0": told})

        t.check("unit_unknown_000ee8a0", unit, random_.choice((0, 1)), globals=units())
        t.check("unit_unknown_000f06c0", Buffer(random_.randbytes(0xBC0 + 16 * 0xD8)))
        t.check("unit_unknown_000f1320", unit, globals={**units(), "global_tag_instances": tags()})

        data = bytearray(random_.randbytes(0xB90 + 4 * 0xD8 + 0x48))
        struct.pack_into("<I", data, 4, 0xB90 + 4 * 0xD8)
        struct.pack_into("<i", data, 0xB90 + 4 * 0xD8 + 0x44, random_.choice((0, 1, 4, -1)))
        for entry in range(4):
            data[0xB90 + entry * 0xD8] = random_.choice((0, 0, 0, 1))
        t.check("unit_unknown_000f1c80", unit, Buffer(bytes(data), pointers=(4,)), globals=units())

        t.check(
            "unit_unknown_000f5d10",
            unit,
            word(random_.choice((-1, 0, 1, 31, 32, 33, random_.randrange(32)))),
            random_.choice((0, 1, 0x100)),
            globals=units(),
        )
