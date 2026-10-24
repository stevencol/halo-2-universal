# twelve small functions: names and places of the weapons, what objects have
# at their short at 0x33E, the longs of what units have at 0x12A, where some
# bits are, three longs of an early mover and an object taken out of a list

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x400
TAG = 0x300
ELEMENTS = 0x40 + 4 * TAG


def run(t) -> None:
    random_ = random.Random(790)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags() -> Buffer:
        data = bytearray(random_.randbytes(ELEMENTS + 2 * 0xEC))
        pointers = []
        for tag in range(4):
            at = 0x40 + tag * TAG
            struct.pack_into("<I", data, tag * 0x10 + 8, at)
            struct.pack_into("<iI", data, at + 0x2C8, random_.choice((0, 1, 2, -1)), ELEMENTS)
            struct.pack_into("<iI", data, at + 0x2D0, random_.choice((0, 1, 2, -1)), ELEMENTS)
            pointers += [tag * 0x10 + 8, at + 0x2CC, at + 0x2D4]
        struct.pack_into("<hh", data, ELEMENTS + 6, random_.choice((1, 2, 2)), 0)
        struct.pack_into("<h", data, ELEMENTS + 0x18, random_.choice((0, 1, 1)))
        for entry in range(2):
            struct.pack_into("<I", data, ELEMENTS + entry * 0xEC + 0x30, random_.choice((0, NONE, 0x12345678)))
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(400):
        table = some_objects(random_)
        headers = bytearray(table.data[: 0x4C + COUNT * 12])
        objects = bytearray()
        for index in range(COUNT):
            one = bytearray(table.data[FIRST + index * OBJECT : FIRST + (index + 1) * OBJECT] + random_.randbytes(BIG - OBJECT))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            struct.pack_into("<H", one, 0xC0, random_.choice((0, 0x200, 0xFFFF, 0xFDFF)))
            one[0x12C] = random_.choice((0, 1))
            struct.pack_into("<I", one, 0x154, random_.choice((NONE, salted(random_.randrange(COUNT)))))
            one[0x20C] = random_.choice((0, 2, 3))
            for weapon in range(4):
                struct.pack_into("<I", one, 0x218 + weapon * 4, random_.choice((NONE, salted(random_.randrange(COUNT)))))
            # what it has at its short at 0x12A (0x60) and at 0x33E (0x2C0; its 0x7E is the short)
            struct.pack_into("<h", one, 0x12A, 0x60)
            struct.pack_into("<I", one, 0x60, random_.choice((NONE, 0, 1)))
            struct.pack_into("<h", one, 0x66, random_.choice((-1, 0, 2)))
            struct.pack_into("<I", one, 0x60 + 0x68, random_.choice((NONE, 1, 2)))
            struct.pack_into("<h", one, 0x33E, 0x2C0)
            data = 0x2C0
            one[data + 2] = random_.choice((0, 1, 6, 7, 0xFF))
            struct.pack_into("<h", one, data + 0x36, random_.choice((-1, 0, 3)))
            struct.pack_into("<I", one, data + 0x40, f32(random_.choice((0.0, 0.00005, 0.0001, 0.5, -1.0))))
            struct.pack_into("<I", one, data + 0x5C, random_.getrandbits(16) | random_.choice((0xFFFF, 0, 5)) << 16)
            for at in range(13):
                one[data + 0x6E + at] = 0 if random_.random() < 0.9 else 1
            for at in (0x9C, 0xBC, 0xDC):
                struct.pack_into("<I", one, data + at, random_.choice((NONE, 0, 1)))
                struct.pack_into("<h", one, data + at + 6, random_.choice((-1, 0, 2)))
                one[data + at + 0x11] = random_.choice((0, 1, 8, 6))
                one[data + at + 0x12] = random_.choice((0, 1, 3, 2))
            one[data] = random_.choice((0, 0x80, 0x7F, 0xFF))
            objects += one
        units = lambda: {"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers)}
        with_tags = lambda: {**units(), "global_tag_instances": tags()}
        unit = salted(random_.randrange(COUNT))

        t.check("weapon_unknown_00101b10", word(random_.choice((-1, 0, 1, 1, 2))), unit, globals=with_tags())
        # a weapon that is one of the weapons of its unit
        objects_ = bytearray(objects)
        owner = random_.randrange(COUNT)
        struct.pack_into("<I", objects_, owner * BIG + 0x218 + random_.randrange(4) * 4, unit)
        t.check(
            "weapon_unknown_00101f50",
            unit,
            globals={"object_header_data": Buffer(bytes(headers) + bytes(objects_), pointers=table.pointers)},
        )
        t.check("weapon_unknown_00106400", unit, random_.choice((0, 1, 0x100)), globals=with_tags())

        t.check("unit_unknown_0010db60", unit, globals=units())
        t.check("unit_unknown_0010edd0", unit, globals=units())
        t.check("unit_unknown_0010f820", unit, globals=units())
        t.check("unit_unknown_0010f890", unit, globals=units())
        t.check("unit_unknown_0010f630", unit, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)), globals=units())

        # something with its first long, what it points to at 0x04 and a byte at 0x08
        data = bytearray(random_.randbytes(0x100))
        struct.pack_into("<IIB", data, 0, 0x30, 0x10, random_.randrange(256))
        data[0x10] = random_.choice((0, 4, 0xFC))
        struct.pack_into("<h", data, 0x16, random_.choice((0, 8, -4)))
        struct.pack_into("<i", data, 0x1C, random_.choice((0, 4, 12)))
        t.check("unit_unknown_0010eea0", random_.getrandbits(16) << 16 | random_.randrange(256), Buffer(bytes(data), pointers=(0, 4)))
        t.check("unit_unknown_0010ee70", Buffer(bytes(data), pointers=(4,)))

        movers = bytearray(random_.randbytes(0x1D88))
        count = random_.choice((0, 1, 5, 32))
        struct.pack_into("<i", movers, 0x1D80, count)
        object_index = salted(random_.randrange(COUNT))
        for index in range(32):
            struct.pack_into("<I", movers, 0x1D00 + index * 4, random_.choice((NONE, object_index, salted(random_.randrange(COUNT)))))
        t.check(
            "object_early_movers_unknown_00109fd0",
            random_.choice((NONE, object_index, object_index)),
            Buffer(random_.randbytes(12)),
            globals={"object_early_movers_globals": Buffer(bytes(movers))},
        )

        globals_ = bytearray(random_.randbytes(0x304))
        for index in range(32):
            struct.pack_into("<I", globals_, 0x280 + index * 4, random_.choice((NONE, unit, unit ^ 0x10000, 5)))
        t.check(
            "object_unknown_0010a980_remove",
            random_.choice((NONE, unit, unit)),
            globals={**units(), "object_unknown_0010a980_globals": Buffer(bytes(globals_))},
        )
