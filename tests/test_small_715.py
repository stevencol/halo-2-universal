# nine more small accessors, most of them of the weapons

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
TAGS, DEFINITION = 4, 0x100


def run(t) -> None:
    random_ = random.Random(715)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        definition_index = random_.randrange(TAGS)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            at = FIRST + (index & 0xFFFF) * OBJECT
            struct.pack_into("<I", data, at, random_.getrandbits(16) << 16 | definition_index)
            data[at + 0x10A] = random_.choice((0, 4, 0xFB, 0xFF))
            data[at + 0x12C] = random_.choice((0, 1, 2, 3, 0xFD, 0xFF))
            struct.pack_into("<I", data, at + 0x178, random_.choice((0, 0, 1, random_.getrandbits(32))))
            data[at + 0x20C] = random_.choice((0, 0, 1))
            data[at + 0x218] = random_.choice((0, 0, 1))
            for trigger in range(2):
                data[at + 0x1A4 + trigger * 0x34 + 1] = random_.choice((0, 1, 1, 2))
                struct.pack_into("<h", data, at + 0x224 + trigger * 0x10, random_.choice((0, 2, 2, 3)))
            struct.pack_into("<2h", data, at + 0x12A, random_.choice((0x260, 0x2C0)), 0)
            struct.pack_into("<h", data, at + 0x33E, random_.choice((0x260, 0x2C0)))
            for offset in (0x260, 0x2C0):
                for field in (0, 0x68, 0x9C):
                    struct.pack_into("<I", data, at + offset + field, random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))))
                for field in (6, 0x9C + 6):
                    struct.pack_into("<h", data, at + offset + field, random_.choice((-1, 0, 7)))
            tags = bytearray(random_.randbytes(TAGS * 16 + DEFINITION))
            struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
            return {
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
            }

        t.check("weapon_unknown_00101e80", index, globals=some())
        t.check("weapon_unknown_001029d0", index, random_.getrandbits(16) << 16 | random_.randrange(2), globals=some())
        t.check("weapon_unknown_00102f70", index, random_.getrandbits(16) << 16 | random_.choice((0, 1, 2, 0xFFFF)), globals=some())
        t.check("weapon_clear_unknown250", index, globals=some())
        t.check("object_definition_get_unknownC0", index, globals=some())
        t.check("object_unknown_0010cf50", index, globals=some())
        t.check("object_unknown_0010f5f0", index, globals=some())
        t.check("object_unknown_0010f8f0", index, globals=some())
        t.check("object_set_unknown12C_bit0", index, random_.choice((0, 1, 0x100, 0x80)), globals=some())
