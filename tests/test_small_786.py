# thirteen small functions of the units: names of what they have at their
# short at 0x12A, their children, their weapons, the bits of the elements of
# the block at 0x1C8 of their definitions, a byte of 0x23E, a list of objects
# and a point

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
NAMES = (0x0600008C, 0x0D000042, 0x0C000043, 0x12345678)
TAG = 0x1D0
ELEMENTS = 0x40 + 4 * TAG


def run(t) -> None:
    random_ = random.Random(786)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags() -> Buffer:
        """four definitions, all with the same four elements at 0x1C8"""
        data = bytearray(random_.randbytes(ELEMENTS + 4 * 0xB0))
        pointers = []
        for tag in range(4):
            struct.pack_into("<I", data, tag * 0x10 + 8, 0x40 + tag * TAG)
            struct.pack_into("<iI", data, 0x40 + tag * TAG + 0x1C8, random_.choice((0, 1, 4, 3)), ELEMENTS)
            pointers += [tag * 0x10 + 8, 0x40 + tag * TAG + 0x1CC]
        return Buffer(bytes(data), pointers=tuple(pointers))

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        names = [random_.choice(NAMES) for _ in range(5)]
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, random_.getrandbits(16) << 16 | random_.randrange(4))
            struct.pack_into("<I", objects, at + 0x0C, random_.choice([NONE] + [salted(later) for later in range(index + 1, COUNT)]))
            struct.pack_into("<I", objects, at + 0x10, random_.choice((NONE, salted(random_.randrange(COUNT)))))
            objects[at + 0xAA] = random_.choice((0, 1, 2, 3, 32, 33, 0xFF))
            struct.pack_into("<h", objects, at + 0x12A, 0x150)
            data = struct.pack(
                "<IIh", random_.choice((NONE, 0, 5)), random_.choice((NONE, 0, 7)), random_.choice((-1, 0, 2))
            )
            objects[at + 0x150 + 0x68 : at + 0x150 + 0x68 + 4] = struct.pack("<I", random_.choice((NONE, 1, 2)))
            objects[at + 0x150 : at + 0x150 + 4] = data[0:4]
            objects[at + 0x150 + 6 : at + 0x150 + 8] = data[8:10]
            struct.pack_into("<I", objects, at + 0x150 + 0x7C, random_.choice(NAMES + tuple(names)))
            struct.pack_into("<h", objects, at + 0x1FC, random_.choice((0, 1, 2, 3)))
            objects[at + 0x212] = random_.choice((0, 1, 3, 0xFF))
            objects[at + 0x213] = random_.choice((0, 2, 3, 0xFF))
            for weapon in range(4):
                struct.pack_into("<I", objects, at + 0x218 + weapon * 4, random_.choice((NONE, NONE, salted(random_.randrange(COUNT)))))
        globals_ = lambda: {"object_header_data": Buffer(bytes(objects), pointers=table.pointers)}
        with_tags = lambda: {"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "global_tag_instances": tags()}
        unit = salted(random_.randrange(COUNT))

        t.check("unit_unknown_000c7070", unit, globals={**globals_(), "unknown_00467430": struct.pack("<5I", *names)})
        t.check("unit_unknown_000c8f60", unit, random_.getrandbits(16) << 16 | random_.choice((0, 1, 2, 3)), globals=globals_())
        t.check("unit_unknown_000cbe60", unit, globals=globals_())
        t.check("unit_unknown_000cc3c0", unit, globals=globals_())
        t.check("unit_unknown_000cc410", unit, globals=globals_())
        index = random_.getrandbits(16) << 16 | word(random_.choice((-1, 0, 1, 2, 3, 4, -0x8000))) & 0xFFFF
        t.check("unit_unknown_000cc750", unit, index, globals=with_tags())
        t.check("unit_unknown_000cc7b0", unit, index, globals=with_tags())
        t.check(
            "unit_unknown_000cccd0",
            unit,
            random_.getrandbits(16) << 16 | random_.randrange(4),
            random_.getrandbits(32),
            globals=globals_(),
        )
        t.check(
            "unit_unknown_000cd5c0",
            unit,
            random_.choice((0, 1, 2, 3, struct.unpack_from("<I", objects, FIRST + random_.randrange(COUNT) * OBJECT)[0])),
            globals=globals_(),
        )
        t.check("unit_unknown_000cfe40", unit, globals=with_tags())

        iterator = bytearray(random_.randbytes(0x10))
        struct.pack_into("<I", iterator, 0x0C, random_.choice((NONE, salted(random_.randrange(COUNT)))))
        t.check("unit_iterator_next", Buffer(bytes(iterator)), globals=globals_())

        t.check("unit_unknown_000d0930", unit, Buffer(random_.randbytes(12)), globals=globals_())
        t.check("unit_unknown_000c48f0", random_.choice((NONE, unit)), globals=with_tags())
