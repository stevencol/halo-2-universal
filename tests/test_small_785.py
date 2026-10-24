# eleven small functions of the objects: the bit 0x40 of the last parent, the
# tenths of something an object has, an entry of another, whether an object
# is another or one of its parents, a bit of the flags, an object taken out of
# a list, a mark of each time, a real and a short, an object taken out of
# unknown_004de2d0 and two of the liquids

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(785)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        object_index = salted(random_.randrange(COUNT))
        at = FIRST + (object_index & 0xFFFF) * OBJECT
        struct.pack_into("<hh", objects, at + 0x118, random_.choice((0, 9, 10, 25, 100, -15)), random_.choice((0x120, 0x180, 0x1F0)))
        struct.pack_into("<hh", objects, at + 0x124, random_.choice((0, 11, 12, 24, 48, 72, random_.randrange(73))), 0x130)
        struct.pack_into("<h", objects, at + 0xAC, random_.choice((-1, 0, 3)))
        globals_ = lambda: {"object_header_data": Buffer(bytes(objects), pointers=table.pointers)}

        t.check("object_unknown_000b9d20", object_index, globals=globals_())

        out = lambda: random_.choice((0, Buffer(random_.randbytes(4), places=(0,))))
        t.check("object_unknown_000ba690", object_index, Buffer(random_.randbytes(4)), out(), out(), out(), globals=globals_())

        t.check(
            "object_unknown_000bad50",
            object_index,
            word(random_.choice((-1, 0, 1, 2, 3))),
            Buffer(random_.randbytes(12)),
            globals=globals_(),
        )

        # the parents of the object, with their salts
        parents = []
        index = object_index
        while index != NONE:
            parents.append(index)
            index = struct.unpack_from("<I", objects, FIRST + (index & 0xFFFF) * OBJECT + 0x14)[0]
        other = random_.choice(parents + [NONE, salted(random_.randrange(COUNT)), parents[-1] ^ 0x10000])
        t.check("object_unknown_000bafb0", object_index, other, globals=globals_())
        t.check("object_unknown_000bafb0", NONE, random_.choice((0, 0, NONE, other)), globals=globals_())

        t.check("object_unknown_000bbf40", random_.choice((NONE, object_index)), random_.choice((0, 1, 0x100)), globals=globals_())

        # a list where each has the next at 0x0C, the next always a later one
        listed = bytearray(objects)
        for index in range(COUNT):
            struct.pack_into("<I", listed, FIRST + index * OBJECT + 0x0C, random_.choice([NONE] + [salted(later) for later in range(index + 1, COUNT)]))
        first = random_.choice((NONE, salted(random_.randrange(COUNT))))
        members = []
        index = first
        while index != NONE:
            members.append(index)
            index = struct.unpack_from("<I", listed, FIRST + (index & 0xFFFF) * OBJECT + 0x0C)[0]
        t.check(
            "object_unknown_000be650",
            Buffer(struct.pack("<I", first)),
            random_.choice(members + [salted(random_.randrange(COUNT))]) if members else object_index,
            globals={"object_header_data": Buffer(bytes(listed), pointers=table.pointers)},
        )

        time = random_.getrandbits(32)
        marks = bytearray(random_.randbytes(0x2000))
        index = random_.randrange(0x800)
        if n % 2:
            struct.pack_into("<I", marks, index * 4, time)
        t.check(
            "object_unknown_000bec70",
            random_.getrandbits(16) << 16 | index,
            globals={"unknown_004de2fc": time, "unknown_004de300": bytes(marks)},
        )

        t.check(
            "object_unknown_000bf830",
            object_index,
            f32(random_.uniform(-5, 5)),
            random_.getrandbits(16) << 16 | word(random_.choice((-1, 0, 1, 0x7FFE, 0x7FFF, -0x8000, random_.getrandbits(15)))) & 0xFFFF,
            globals=globals_(),
        )

        count = random_.choice((0, 1, 4, 8))
        scenario = bytearray(random_.randbytes(0x50))
        struct.pack_into("<i", scenario, 0x48, count)
        table_ = struct.pack("<8I", *(random_.choice((NONE, object_index, object_index, salted(object_index & 0xFFFF), 5)) for _ in range(8)))
        t.check(
            "unknown_004de2d0_remove",
            object_index,
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_scenario": Buffer(bytes(scenario)),
                "unknown_004de2d0": Buffer(table_),
            },
        )

        liquids = bytearray(random_.randbytes(0x50 + 4 * 0x110))
        liquids[0x29] = random_.choice((0, 1, 0x80))
        struct.pack_into("<I", liquids, 0x44, 0x50)
        liquid = lambda: {"liquid_data": Buffer(bytes(liquids), pointers=(0x44,)) if n % 9 else 0}
        liquid_index = random_.choice((NONE, salted(random_.randrange(4))))
        t.check("liquid_unknown_000c4210", liquid_index, random_.randrange(4), globals=liquid())
        t.check("liquid_unknown_000c4250", liquid_index, globals=liquid())
