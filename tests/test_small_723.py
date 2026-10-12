# ten more small functions of actors

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
ACTORS, ACTOR = 3, 0x888
PROPS, PROP = 3, 0x3C
CLUMPS, CLUMP = 3, 0x50


def run(t) -> None:
    random_ = random.Random(723)

    for n in range(200):
        def an_object() -> int:
            return random_.getrandbits(16) << 16 | random_.randrange(COUNT)

        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(ACTORS):
                at = 0x4C + index * ACTOR
                data[at + 7] = random_.choice((0, 0, 1))
                struct.pack_into("<I", data, at + 0x18, an_object())
                struct.pack_into("<I", data, at + 0x7C, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CLUMPS))))
                struct.pack_into("<h", data, at + 0x190, random_.choice((-1, 0, 1, 2, 3, 4)))
                data[at + 0x225] = random_.choice((0, 1, 1, 2))
                data[at + 0x227] = random_.choice((0, 1, 1, 0xFF))
                struct.pack_into("<I", data, at + 0x26C, random_.choice((NONE, NONE, 5)))
                struct.pack_into("<h", data, at + 0x328, random_.choice((0, 11, 12, 13, -1)))
                struct.pack_into("<I", data, at + 0x338, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(PROPS))))
                struct.pack_into("<h", data, at + 0x504, random_.choice((1, 1, 0, 2)))
                data[at + 0x50C] = random_.choice((0, 1, 1, 0xFF))
                struct.pack_into("<I", data, at + 0x5AC, random_.choice((NONE, an_object(), an_object())))
                struct.pack_into("<h", data, at + 0x5B0, random_.choice((6, 6, 6, 5)))
            return Buffer(bytes(data), pointers=(0x44,))

        def some_objects() -> Buffer:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                at = FIRST + one * OBJECT
                struct.pack_into("<f", data, at + 0xEC, random_.choice((0.0, -1.0, 1.0, 0.001, float("nan"))))
                struct.pack_into("<h", data, at + 0x1FC, random_.choice((-1, -1, 0, 3)))
                struct.pack_into("<h", data, at + 0x346, random_.choice((0x350, 0x380)))
                for offset in (0x350, 0x380):
                    struct.pack_into("<h", data, at + offset + 0x36, random_.choice((5, 7, 0, 6)))
            return Buffer(bytes(data), pointers=table.pointers)

        def table(count: int, size: int, fill) -> Buffer:
            data = bytearray(random_.randbytes(0x4C + count * size))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(count):
                fill(data, 0x4C + index * size)
            return Buffer(bytes(data), pointers=(0x44,))

        def a_prop(data, at) -> None:
            data[at + 0x27] = random_.choice((0, 1, 2, 3, 0x7F, 0x80, 0xFF))

        def a_clump(data, at) -> None:
            struct.pack_into("<h", data, at + 0x26, random_.choice((1, 1, 0, 2)))

        actor_index = random_.getrandbits(16) << 16 | random_.randrange(ACTORS)
        both = lambda: {"actor_data": actors(), "object_header_data": some_objects()}

        t.check("actor_unknown_001aa6e0", actor_index, random_.getrandbits(32), globals={"actor_data": actors()})
        t.check("actor_unknown_001ac380", actor_index, random_.getrandbits(32), globals=both())
        t.check("actor_unknown_001b09b0", actor_index, random_.getrandbits(32), globals={"actor_data": actors()})
        t.check(
            "actor_unknown_001b4330",
            actor_index,
            globals={"actor_data": actors(), "clump_data": table(CLUMPS, CLUMP, a_clump)},
        )
        t.check(
            "actor_unknown_001b8540",
            actor_index,
            Buffer(random_.randbytes(0x14)),
            globals={"actor_data": actors(), "prop_ref_data": table(PROPS, PROP, a_prop)},
        )
        t.check("actor_unknown_001be040", actor_index, globals=both())
        t.check(
            "actor_unknown_001a9b50",
            actor_index,
            random_.getrandbits(32),
            globals={**both(), "unknown_0046fbe4": random_.getrandbits(32)},
        )

        thing = bytearray(random_.randbytes(0x2C))
        thing[0x27] = random_.choice((0, 1, 2, 0x80, 0xFF))
        struct.pack_into("<f", thing, 0x28, random_.choice((1.0, 2.0, 3.0, -5.0, float("nan"))))
        t.check("actor_unknown_001acd30", actor_index, Buffer(bytes(thing)), globals=both())

        named = bytearray(random_.randbytes(0x80))
        for index in range(10):
            struct.pack_into("<I", named, index * 12 + 4, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(ACTORS))))
            struct.pack_into("<h", named, index * 12 + 8, random_.choice((3, 3, 3, 2)))
        struct.pack_into("<h", named, 0x7C, random_.choice((0, 5, 10)))
        t.check(
            "unknown_get_actor_unknown90",
            random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0, 1, 4, 5, 9, 10)),
            Buffer(bytes(named)),
            globals={"actor_data": actors()},
        )

        t.check(
            "actor_unknown_001b0710",
            an_object(),
            actor_index,
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            globals=both(),
        )
