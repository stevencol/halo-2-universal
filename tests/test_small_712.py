# eight more small accessors of objects and units

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(712)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        value = random_.getrandbits(32)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            at = FIRST + (index & 0xFFFF) * OBJECT
            data[at + 0xAA] = random_.choice((0, 0, 1, 2))
            struct.pack_into("<I", data, at + 0xD4, random_.choice((NONE, random_.getrandbits(32))))
            data[at + 0x212] = random_.choice((0xFF, 0, 1))
            data[at + 0x213] = random_.choice((0xFF, 0, 1))
            for weapon in range(4):
                struct.pack_into("<I", data, at + 0x218 + weapon * 4, random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x2A8, random_.choice((value, random_.getrandbits(32))))
            return {"object_header_data": Buffer(bytes(data), pointers=table.pointers)}

        t.check(
            "object_unknown_000a7a30",
            index,
            random_.getrandbits(32),
            globals=some(),
            externals={"unknown_000b58c0": lambda harness, arguments: 0},
        )
        t.check("object_get_unknownC4", index, Buffer(random_.randbytes(0xC + 4)), globals=some())
        t.check("unit_unknown_000c8a10", index, globals=some())
        t.check("unit_get_unknown168", Buffer(random_.randbytes(0xC + 4)), index, globals=some())
        t.check("unit_get_free_weapon_index", index, globals=some())
        t.check("unit_has_two_weapons_in_hand", index, globals=some())
        t.check("unit_clear_unknown2A8", index, value, globals=some())
        t.check("unit_clear_unknown34C", index, globals=some())
