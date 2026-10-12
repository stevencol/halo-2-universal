# nine more small functions of objects and units

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(713)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        value = random_.getrandbits(32)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                at = FIRST + one * OBJECT
                # the flags of the place, and of the object; and the object that it is on: a later one, or none
                data[0x4C + one * 12 + 2] = random_.choice((0, 0, 0, 0x10, 0xEF))
                data[at + 4] = random_.choice((0, 0, 0, 1, 0xFE))
                later = NONE if one + 1 >= COUNT or random_.random() < 0.4 else random_.getrandbits(16) << 16 | random_.randrange(one + 1, COUNT)
                struct.pack_into("<I", data, at + 0x14, later)
            at = FIRST + (index & 0xFFFF) * OBJECT
            struct.pack_into("<I", data, at + 0xD4, random_.choice((NONE, random_.getrandbits(32))))
            struct.pack_into("<2h", data, at + 0x114, random_.choice((0, 0x34, 0x68, 0x100, -1)), random_.choice((0x200, 0x300)))
            struct.pack_into("<h", data, at + 0x130, random_.choice((0, 1, 2, 3, -1)))
            struct.pack_into("<I", data, at + 0x238, random_.choice((NONE, random_.getrandbits(32))))
            struct.pack_into("<h", data, at + 0x33E, random_.choice((0x350, 0x380)))
            struct.pack_into("<I", data, at + 0x350 + 0x38, random_.choice((value, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x380 + 0x38, random_.choice((value, random_.getrandbits(32))))
            return {"object_header_data": Buffer(bytes(data), pointers=table.pointers)}

        game = bytearray(random_.randbytes(0x1200))
        game[0xC] = random_.choice((0, 1, 4, 4, 5))

        t.check("object_get_unknown30", Buffer(random_.randbytes(16)), index, Buffer(random_.randbytes(8)), globals=some())
        t.check("object_unknown_000a0dd0", index, globals=some())
        t.check("object_unknown_000a7670", index, globals={**some(), "main_game_globals": Buffer(bytes(game))})
        t.check("object_get_unknown116_and_count", index, Buffer(random_.randbytes(8)), globals=some())
        t.check("object_unknown_000b9ce0", index, globals=some())
        place = random_.randrange(8)
        table = b"".join(struct.pack("<I", random_.choice((NONE, NONE, random_.getrandbits(32)))) for _ in range(8))
        t.check(
            "object_unknown_000bf050",
            random_.getrandbits(16) << 16 | place,
            index,
            globals={**some(), "unknown_004de2d0": Buffer(table)},
        )
        t.check("unit_delete_unknown238", index, globals=some(), externals={"unknown_000b8540": lambda harness, arguments: 0})
        t.check("unit_clear_unknown2A0", index, globals=some())
        t.check("unit_clear_unknown33E_unknown38", index, value, globals=some())
