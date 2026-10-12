# eleven more small accessors: of objects, and of their definitions

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
TAGS, DEFINITION = 4, 0x2A0


def run(t) -> None:
    random_ = random.Random(714)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        value = random_.getrandbits(32)
        definition_index = random_.randrange(TAGS)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                at = FIRST + one * OBJECT
                struct.pack_into("<I", data, at, random_.getrandbits(16) << 16 | definition_index)
                data[at + 0xAB] = random_.choice((0, 2, 2, 3))
                data[at + 0x10A] = random_.choice((0, 4, 0xFB, 0xFF))
                struct.pack_into("<I", data, at + 0x12C, random_.choice((NONE, NONE, random_.getrandbits(32))))
                struct.pack_into("<I", data, at + 0x248, NONE)
            at = FIRST + (index & 0xFFFF) * OBJECT
            struct.pack_into("<I", data, at + 0x144, random_.choice((value, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x248, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT))))
            tags = bytearray(random_.randbytes(TAGS * 16 + DEFINITION))
            struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
            struct.pack_into("<h", tags, TAGS * 16 + 0x1F0, random_.choice((6, 6, 0, 7)))
            struct.pack_into("<h", tags, TAGS * 16 + 0x1FE, random_.choice((0, 1, -1, 40)))
            return {
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
            }

        t.check("unit_unknown_0009dc90", index, globals=some())
        t.check("object_unknown_000a09b0", index, globals=some())
        t.check("unit_unknown_000cc380", index, globals=some())
        t.check("object_definition_get_unknown1EC_bit7", index, globals=some())
        t.check("object_definition_unknown1F0_is_not_6", index, globals=some())
        t.check("unit_clear_unknown144", index, value, globals=some())
        t.check("object_definition_has_unknown1FE", index, globals=some())
        t.check("object_definition_get_unknown12C_bit3", index, globals=some())
        t.check("object_definition_get_unknown290", index, globals=some())
        t.check("object_definition_get_unknown12F_bit0", random_.choice((NONE, index, index)), globals=some())
        t.check("object_definition_get_unknown12C_bit28", index, globals=some())
