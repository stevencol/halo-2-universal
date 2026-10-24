# fourteen small functions near the units: kinds of objects, the tag that a
# definition names at 0x38, something of 0x88 bytes, a number by a name, the
# longs from 0x350 of a unit, its bits and some of its bytes

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x400
NAMES = (0x0400000C, 0x0900000E, 0x0A00000F, 0x0800001E, 0x09000015, 0x09000016, 0x0A000014, 0x0A000017, 0x0D000042)


def run(t) -> None:
    random_ = random.Random(787)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags(empty: bool = False) -> Buffer:
        """eight tags of 0x280 bytes: those of 0 to 3 are definitions that name one of 4 to 7 (or NONE) at 0x38; the
        fourth may not be loaded"""
        data = bytearray(random_.randbytes(0x80 + 8 * 0x280 + 0x100))
        pointers = []
        for tag in range(8):
            address = 0 if empty and tag == 3 and random_.random() < 0.5 else 0x80 + tag * 0x280
            struct.pack_into("<I", data, tag * 0x10 + 8, address)
            if address:
                pointers.append(tag * 0x10 + 8)
        for tag in range(4):
            struct.pack_into("<I", data, 0x80 + tag * 0x280 + 0x38, random_.choice((NONE, salted(4 + tag), salted(4 + random_.randrange(4)))))
        for tag in range(4, 8):
            at = 0x80 + tag * 0x280
            struct.pack_into("<IiI", data, at + 0x5C, 0x80 + 8 * 0x280, random_.choice((-1, 0, 1, 2)), random_.getrandbits(32))
            pointers.append(at + 0x5C)
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(400):
        # big objects, for the units' longs at 0x350 and after
        table = some_objects(random_)
        headers = bytearray(table.data[: 0x4C + COUNT * 12])
        objects = bytearray()
        for index in range(COUNT):
            one = bytearray(table.data[FIRST + index * OBJECT : FIRST + (index + 1) * OBJECT] + random_.randbytes(BIG - OBJECT))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            one[0x10A] = random_.choice((0, 4, 0xFB, 0xFF))
            for at in (0x354, 0x35C, 0x360, 0x3E4, 0x3E8):
                struct.pack_into("<I", one, at, random_.choice((NONE, 0, 1, 3)))
            one[0x3DC] = random_.choice((0, 6, 6, 7))
            one[0x3F8] = random_.choice((0, 1))
            struct.pack_into("<h", one, 0x346, 0x100)
            objects += one
        units = lambda: {"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers)}
        unit = salted(random_.randrange(COUNT))

        t.check("unit_unknown_000d5990", random_.choice((0, 1, 5, 6, 6, 6, 9, 32, 38, 0xFF)), salted(random_.randrange(4)), globals={"global_tag_instances": tags(True)})

        data = bytearray(random_.randbytes(0x34))
        data[0x30] = random_.choice((0, 0, 1, 2, 3, 0xFF))
        data[0x2F] = random_.choice((0, 1, 1, 2, 0xFF))
        struct.pack_into("<h", data, 0x2C, random_.choice((-1, 0, 5)))
        data[0x2E] = random_.choice((0, 3, 5, 11, 12, 13, 32, 37))
        t.check("unit_unknown_000d5a70", Buffer(bytes(data)))

        t.check("unit_unknown_000d5b60", random_.choice((NONE, unit, unit)), globals={**units(), "global_tag_instances": tags()})

        t.check(
            "unit_unknown_000d6660",
            Buffer(random_.randbytes(0x88)),
            random_.getrandbits(32),
            globals={"unknown_0047d8e0": struct.pack("<h", random_.choice((-1, 0, 7))), "global_structure_bsp_index": struct.pack("<h", random_.choice((-1, 0, 2)))},
        )

        count = random_.choice((0, 1, 3, 4))
        data = bytearray(random_.randbytes(0x80 + 4 * 0x60 + 4 * 0x44))
        struct.pack_into("<Ii", data, 0x70, 0x80, count)
        pointers = [0x70]
        for element in range(4):
            target = 0x80 + 4 * 0x60 + element * 0x44
            struct.pack_into("<I", data, 0x80 + element * 0x60 + 0x40, target)
            data[target + 0x40] = random_.choice((0, 0, 0, 1))
            pointers.append(0x80 + element * 0x60 + 0x40)
        t.check("unit_unknown_000d74b0", Buffer(bytes(data), pointers=tuple(pointers)))

        t.check(
            "unit_unknown_000d8ae0",
            random_.choice((NONE, unit, unit)),
            Buffer(random_.randbytes(2)),
            random_.randrange(8),
            globals={**units(), "global_tag_instances": tags(), "unknown_0047d8e0": struct.pack("<h", random_.choice((-1, 9)))},
        )

        t.check("unit_unknown_000dd300", word(random_.choice(NAMES + (0x0A000010, 0x0900000F, 0x09000014, 0x09000017, 0, -1, random_.getrandbits(32)) + tuple(name + 1 for name in NAMES))))

        bsp = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", bsp, 0x2C, 0x40)
        t.check(
            "unit_unknown_000ddf60",
            unit,
            globals={**units(), "global_collision_bsp": Buffer(bytes(bsp) + random_.randbytes(0x20), pointers=(0x2C,))},
        )

        t.check("unit_unknown_000dfd70", unit, globals=units())
        t.check("unit_unknown_000e0dc0", unit, globals=units())
        t.check("unit_unknown_000e58e0", unit, globals=units())
        t.check("unit_unknown_000e5980", unit, globals={**units(), "global_tag_instances": tags()})

        data = bytes((0,)) * 0x11 + bytes((random_.choice((0, 1, 8, 9, 6)), random_.choice((0, 1, 2, 3))))
        t.check("unit_unknown_000e6800", Buffer(data))

        t.check("unit_unknown_000e6fe0", unit, word(random_.choice((0, 1, 31, 32, 63, 100, random_.randrange(256)))), globals=units())
