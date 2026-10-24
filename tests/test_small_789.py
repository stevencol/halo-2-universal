# nine small functions of the weapons: bits of their entries, the shorts of
# their magazines, a real, the states of their triggers and bits of their
# definitions

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
    random_ = random.Random(789)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags() -> Buffer:
        """four definitions, sharing four elements of 0x40 bytes"""
        data = bytearray(random_.randbytes(ELEMENTS + 4 * 0x40))
        pointers = []
        for tag in range(4):
            at = 0x40 + tag * TAG
            struct.pack_into("<I", data, tag * 0x10 + 8, at)
            struct.pack_into("<I", data, at + 0x12C, random_.choice((0, 1 << 6, 1 << 18, 1 << 22, 1 << 23, random_.getrandbits(32))))
            struct.pack_into("<h", data, at + 0x294, random_.choice((0, 1)))
            struct.pack_into("<iI", data, at + 0x2C0, random_.choice((0, 1, 2, 4, -1)), ELEMENTS)
            struct.pack_into("<iI", data, at + 0x2C8, random_.choice((0, 1, 2, 4, -1)), ELEMENTS)
            struct.pack_into("<i", data, at + 0x2D0, random_.choice((0, 1, 2, 4, -1)))
            pointers += [tag * 0x10 + 8, at + 0x2C4, at + 0x2CC]
        for element in range(4):
            struct.pack_into("<hh", data, ELEMENTS + element * 0x40 + 6, random_.choice((4, 5, 5)), 0)
            struct.pack_into("<h", data, ELEMENTS + element * 0x40 + 0x0A, random_.choice((-1, 0, 1)))
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(400):
        table = some_objects(random_)
        headers = bytearray(table.data[: 0x4C + COUNT * 12])
        objects = bytearray()
        for index in range(COUNT):
            one = bytearray(table.data[FIRST + index * OBJECT : FIRST + (index + 1) * OBJECT] + random_.randbytes(BIG - OBJECT))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            struct.pack_into("<i", one, 0x178, random_.choice((4, 5, 7, 10, 11, -1)))
            struct.pack_into("<I", one, 0x184, f32(random_.choice((0.0, 0.5, 1.0, 2.0))))
            for entry in range(4):
                struct.pack_into("<h", one, 0x224 + entry * 0x10, random_.choice((-1, 0, 1, 6, 7)))
                one[0x20C + entry * 0x0C] = random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 8))
            struct.pack_into("<hh", one, 0x22A, random_.choice((0, 0, 1)), random_.choice((0, 0, 1)))
            objects += one
        globals_ = lambda: {"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers), "global_tag_instances": tags()}
        weapon = salted(random_.randrange(COUNT))

        t.check("weapon_unknown_00100390", weapon, random_.choice((0, 1, 2, 3)), globals=globals_())
        t.check("weapon_unknown_001003e0", weapon, random_.choice((0, 1, 2, 3)), globals=globals_())
        t.check("weapon_unknown_00100880", weapon, random_.choice((NONE, NONE, 0, 1, 2, 3, 5)), globals=globals_())
        t.check("weapon_unknown_00100f00", weapon, globals=globals_())
        t.check("weapon_unknown_00101160", weapon, globals=globals_())
        t.check("weapon_unknown_001011d0", weapon, globals=globals_())
        t.check("weapon_unknown_00101340", random_.choice((NONE, weapon, weapon)), globals=globals_())
        t.check("weapon_unknown_00101440", random_.choice((NONE, weapon, weapon)), globals=globals_())
        t.check("weapon_unknown_00101640", weapon, globals=globals_())
