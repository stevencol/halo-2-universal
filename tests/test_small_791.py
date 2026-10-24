# eleven small functions: names and bytes of what objects have at their
# short at 0x33E, a long of a tag, something of 0x30 bytes, bits of objects,
# places and clusters of the structure bsp and a name of a list

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x400
NAMES = (0x0A000066, 0x09000008, 0x09000009, 0x0F00067F, 0x09000007, 0x0900000A, 0x0A000065, 0x0A000067, 0x0E0000C2)


def run(t) -> None:
    random_ = random.Random(791)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags() -> Buffer:
        data = bytearray(random_.randbytes(0x40 + 4 * 0x20))
        for tag in range(4):
            struct.pack_into("<I", data, tag * 0x10 + 8, 0x40 + tag * 0x20)
        return Buffer(bytes(data), pointers=tuple(tag * 0x10 + 8 for tag in range(4)))

    def bsp() -> Buffer:
        """leaves (8 bytes) at 0x30, rows of bits at 0x58, clusters (0xB0) at 0xA0 and elements (0x64) at 0xD4"""
        data = bytearray(random_.randbytes(0x100 + 4 * 8 + 4 * 0xB0 + 4 * 0x64 + 0x100))
        leaves, clusters, elements, bits = 0x100, 0x120, 0x120 + 4 * 0xB0, 0x120 + 4 * 0xB0 + 4 * 0x64
        struct.pack_into("<I", data, 0x30, leaves)
        struct.pack_into("<I", data, 0x58, bits)
        struct.pack_into("<i", data, 0x9C, random_.choice((1, 31, 32, 33, 64)))
        struct.pack_into("<I", data, 0xA0, clusters)
        struct.pack_into("<iI", data, 0xD4, random_.choice((0, 1, 3, 4)), elements)
        for cluster in range(4):
            struct.pack_into("<h", data, clusters + cluster * 0xB0 + 0x72, random_.choice((-1, 0, 1, 2, 3)))
        for element in range(4):
            struct.pack_into("<I", data, elements + element * 0x64 + 0x24, random_.choice((NONE, salted(random_.randrange(4)))))
        return Buffer(bytes(data), pointers=(0x30, 0x58, 0xA0, 0xD8))

    for n in range(400):
        table = some_objects(random_)
        headers = bytearray(table.data[: 0x4C + COUNT * 12])
        objects = bytearray()
        for index in range(COUNT):
            one = bytearray(table.data[FIRST + index * OBJECT : FIRST + (index + 1) * OBJECT] + random_.randbytes(BIG - OBJECT))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            struct.pack_into("<h", one, 0x33E, 0x2C0)
            struct.pack_into("<I", one, 0x2C0 + 0xA4, random_.choice(NAMES + (random_.getrandbits(32),)))
            one[0x2C0 + 0x89] = random_.choice((0, 1, 0x80))
            one[0x2C0 + 0x8B] = random_.choice((0, 1, 2, 3, 0xFC))
            struct.pack_into("<h", one, 0x342, 0x3A0)
            struct.pack_into("<I", one, 0x3A0, random_.choice((NONE, salted(random_.randrange(4)))))
            one[0x10A] = random_.choice((0, 4, 0xFB))
            one[0x12C] = random_.choice((0, 0x10, 0x20, 0x30, 0xCF))
            struct.pack_into("<h", one, 0x12A, 0x60)
            struct.pack_into("<I", one, 0x60, random_.choice((NONE, 0, 1)))
            struct.pack_into("<h", one, 0x66, random_.choice((-1, 0, 2)))
            struct.pack_into("<I", one, 0x60 + 0x68, random_.choice((NONE, 1, 2)))
            struct.pack_into("<I", one, 0x60 + 0x7C, random_.choice(NAMES))
            objects += one
        units = lambda: {"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers)}
        unit = salted(random_.randrange(COUNT))

        t.check("unit_unknown_00110fc0", unit, globals=units())
        t.check("unit_unknown_00113da0", unit, globals=units())
        t.check("unit_unknown_00113df0", unit, globals=units())
        t.check("unit_unknown_00114440", unit, globals={**units(), "global_tag_instances": tags()})
        t.check(
            "unit_unknown_00118f90",
            Buffer(random_.randbytes(0x30)),
            globals={"global_forward3d": Buffer(random_.randbytes(12)), "global_up3d": Buffer(random_.randbytes(12))},
        )
        t.check("unit_unknown_00119bc0", unit, globals=units())
        t.check("unit_unknown_0011b930", random_.choice((NONE, unit, unit)), globals=units())

        structure = lambda: {"global_structure_bsp": bsp(), "global_structure_bsp_index": struct.pack("<h", random_.choice((-1, 0, 3))), "global_tag_instances": tags()}
        t.check("scenario_location_from_leaf", Buffer(random_.randbytes(8)), random_.choice((NONE, 0, 1, 3)), globals=structure())
        location = struct.pack("<ihh", random_.getrandbits(31), random_.randrange(4), random_.getrandbits(15))
        t.check("scenario_location_unknown_0011bf30", Buffer(location), globals=structure())
        t.check(
            "structure_bsp_unknown_0011c010",
            random_.getrandbits(16) << 16 | random_.randrange(64),
            random_.getrandbits(16) << 16 | random_.randrange(4),
            globals=structure(),
        )

        words = [b"alpha", b"beta", b"", b"alphabet", b"gamma"]
        count = random_.choice((0, 1, 3, 5))
        data = bytearray(random_.randbytes(0x50 + 5 * 0x24))
        struct.pack_into("<iI", data, 0x48, count, 0x50)
        for index in range(5):
            text = random_.choice(words)
            data[0x50 + index * 0x24 : 0x50 + index * 0x24 + len(text) + 1] = text + b"\0"
        name = random_.choice(words + [b"alph", b"delta"])
        t.check("scenario_unknown_0011c0b0", Buffer(bytes(data), pointers=(0x4C,)), Buffer(name + b"\0"))
