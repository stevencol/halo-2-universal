# thirteen more small functions of actors

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
ACTORS, ACTOR = 3, 0x888
PROPS, PROP = 3, 0x3C
TRACKS, TRACK = 3, 0x124


def run(t) -> None:
    random_ = random.Random(724)

    for n in range(200):
        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(ACTORS):
                at = 0x4C + index * ACTOR
                struct.pack_into("<I", data, at + 0x18, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT))))
                struct.pack_into("<I", data, at + 0x338, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(PROPS))))
                struct.pack_into("<h", data, at + 0x3B0, random_.choice((-1, 0, 1, 5)))
                struct.pack_into("<I", data, at + 0x3B4, random_.choice((NONE, random_.getrandbits(32))))
                struct.pack_into("<h", data, at + 0x504, random_.choice((2, 2, 0, 1)))
                struct.pack_into("<f", data, at + 0x7C0, random_.choice((0.0, -1.0, 0.5, 2.0, float("nan"))))
            return Buffer(bytes(data), pointers=(0x44,))

        def some_objects() -> Buffer:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                data[FIRST + one * OBJECT + 0x213] = random_.choice((0xFF, 0, 1, 3))
            return Buffer(bytes(data), pointers=table.pointers)

        def props() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + PROPS * PROP))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(PROPS):
                at = 0x4C + index * PROP
                struct.pack_into("<I", data, at + 0x14, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TRACKS))))
                struct.pack_into("<h", data, at + 0x24, random_.choice((0, 1, 2, 3, 4, -1)))
            return Buffer(bytes(data), pointers=(0x44,))

        def tracks() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + TRACKS * TRACK))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(TRACKS):
                data[0x4C + index * TRACK + 0xD9] = random_.choice((0, 0, 1))
            return Buffer(bytes(data), pointers=(0x44,))

        def thing() -> Buffer:
            data = bytearray(random_.randbytes(0x3C))
            data[4] = random_.choice((0, 8, 8, 0xF7, 0xFF))
            data[0xD] = random_.choice((0, 0, 1))
            data[0x1B] = random_.choice((0, 1, 1))
            data[0x20] = random_.choice((0, 1, 1))
            return Buffer(bytes(data))

        actor_index = random_.getrandbits(16) << 16 | random_.randrange(ACTORS)
        answer = lambda: {"unknown_0046fbe4": random_.getrandbits(32)}

        t.check("actor_clear_unknown5AC", actor_index, globals={"actor_data": actors()})
        for name in ("actor_unknown_001b1eb0", "actor_unknown_001b3ee0", "actor_unknown_001c10f0", "actor_unknown_001c0230", "actor_unknown_001c14b0"):
            t.check(name, actor_index, thing(), globals={"actor_data": actors()})
        t.check("actor_unknown_001b4f60", actor_index, random_.getrandbits(32), globals={"actor_data": actors(), **answer()})
        t.check("actor_unknown_001b4f90", actor_index, thing(), globals={"actor_data": actors(), **answer()})
        t.check(
            "actor_unknown_001b5a20",
            actor_index,
            globals={"actor_data": actors(), "prop_ref_data": props(), "tracking_data": tracks()},
        )
        for name in ("actor_unknown_001bf360", "actor_unknown_001c22c0"):
            t.check(name, actor_index, random_.getrandbits(32), globals={"actor_data": actors(), "prop_ref_data": props()})
        t.check(
            "actor_unknown_001c8440",
            random_.choice((NONE, actor_index, actor_index, actor_index)),
            Buffer(struct.pack("<f", random_.uniform(-10.0, 10.0))),
            thing(),
            globals={"actor_data": actors()},
        )
        t.check(
            "actor_get_object_unknown218",
            actor_index,
            globals={"actor_data": actors(), "object_header_data": some_objects()},
        )
