# nine more small accessors of what objects keep

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(711)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            at = FIRST + (index & 0xFFFF) * OBJECT
            data[at + 0x12C] = random_.choice((0, 1, 0x40, 0xFF))
            struct.pack_into("<h", data, at + 0x130, random_.choice((0, 5, -3, 0x7FFF)))
            struct.pack_into("<h", data, at + 0x346, random_.choice((0x350, 0x380, 0x3A0)))
            data[at + 0x348] = random_.choice((0, 1, 0xFE, 0xFF))
            data[at + 0x3DC] = random_.choice((0, 1, 1, 2))
            return {"object_header_data": Buffer(bytes(data), pointers=table.pointers)}

        t.check("unit_get_unknown346_unknown36", Buffer(random_.randbytes(8)), index, globals=some())
        t.check("unit_get_unknown346_unknownC", index, globals=some())
        t.check("unit_clear_unknown346_unknown2C", index, random_.getrandbits(32), globals=some())
        t.check("unit_set_unknown144", index, Buffer(random_.randbytes(8 + 4)), globals=some())
        t.check(
            "unit_raise_unknown130",
            index,
            random_.getrandbits(16) << 16 | random_.choice((0, 4, 5, 6, 0xFFFD, 0x8000)),
            globals=some(),
        )
        t.check("unit_get_unknown154", index, globals=some())
        t.check("unit_get_unknown12C", index, globals=some())
        t.check("object_set_flag16", index, random_.getrandbits(32), random_.getrandbits(32), globals=some())
        t.check("unit_unknown_0026be60", index, globals=some())
