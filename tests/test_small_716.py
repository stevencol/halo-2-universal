# eight more small functions of objects

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
TAGS, DEFINITION = 4, 0x200


def run(t) -> None:
    random_ = random.Random(716)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        other = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        definition_index = random_.randrange(TAGS)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                at = FIRST + one * OBJECT
                data[0x4C + one * 12 + 2] = random_.choice((0, 1, 0x10, 0x11, 0xEE))
                struct.pack_into("<I", data, at, random_.getrandbits(16) << 16 | definition_index)
                data[at + 4] = random_.choice((0, 0, 0, 1))
                later = NONE if one + 1 >= COUNT or random_.random() < 0.3 else random_.getrandbits(16) << 16 | random_.randrange(one + 1, COUNT)
                struct.pack_into("<I", data, at + 0x14, later)
                data[at + 0xAA] = random_.choice((1, 1, 1, 0, 2))
                data[at + 0xC0] = random_.choice((0, 1, 0xFE, 0xFF))
                data[at + 0x10A] = random_.choice((0, 0, 4, 0xFB, 0xFF))
                struct.pack_into("<h", data, at + 0x346, random_.choice((0x350, 0x380)))
                for offset in (0x350, 0x380):
                    struct.pack_into("<I", data, at + offset + 8, random_.choice((0, 1 << 18, random_.getrandbits(32))))
            tags = bytearray(random_.randbytes(TAGS * 16 + DEFINITION))
            struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
            struct.pack_into("<h", tags, TAGS * 16 + 0x1F0, random_.choice((6, 6, 0, 7)))
            return {
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
            }

        game = bytearray(random_.randbytes(0x1200))
        game[0xC] = random_.choice((0, 1, 4, 4, 5))

        t.check("unit_unknown_0011a960", random_.choice((NONE, index, index)), globals=some())
        t.check("unit_unknown_002013c0", index, globals=some())
        t.check("object_definition_get_unknown124", index, globals=some())
        t.check("object_definition_unknown1F0_is_6", index, globals=some())
        t.check(
            "object_unknown_0015b1e0",
            index,
            Buffer(random_.randbytes(0x40)),
            random_.randrange(8),
            globals={**some(), "main_game_globals": Buffer(bytes(game))},
        )
        t.check("object_unknown_001794a0", index, other, globals=some())
        t.check("object_unknown_001b8c80", random_.choice((NONE, index, index)), globals=some())
        t.check(
            "object_unknown_001c3980",
            index,
            globals={**some(), "unknown_0051e9a0": Buffer(random_.randbytes(8))},
        )
