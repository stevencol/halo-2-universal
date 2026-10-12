# eleven small accessors of the objects and the units

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(710)

    for n in range(200):
        index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)

        def some() -> dict:
            table = objects(random_)
            data = bytearray(table.data)
            at = FIRST + (index & 0xFFFF) * OBJECT
            data[at + 0xB3] = random_.choice((0, 0, 1, 0x80))
            data[at + 0x39D] = random_.choice((0, 0, 1, 0x80))
            struct.pack_into("<h", data, at + 0x112, random_.choice((-1, -1, 0x200)))
            struct.pack_into("<h", data, at + 0x116, random_.choice((-1, 0x200, 0x300)))
            struct.pack_into("<h", data, at + 0x12A, random_.choice((-1, -1, 0x200)))
            return {"object_header_data": Buffer(bytes(data), pointers=table.pointers)}

        t.check("object_unknown_0009d250", index, globals=some())
        t.check(
            "object_unknown_000a0930",
            Buffer(struct.pack("<3I", random_.getrandbits(32), random_.getrandbits(32), random_.choice((NONE, index)))),
            globals=some(),
        )
        t.check("object_get_unknown116", index, random_.getrandbits(16) << 16 | random_.randrange(8), globals=some())
        t.check("object_has_unknown12A", index, globals=some())
        t.check("object_has_unknown112", index, globals=some())
        t.check("object_unknown_000bfc90", random_.choice((NONE, index, index)), globals=some())
        t.check("object_unknown_000d0590", Buffer(random_.randbytes(0x10 + 4)), index, globals=some())
        t.check("unit_set_unknown1EC", index, random_.getrandbits(32), random_.getrandbits(32), globals=some())
        t.check(
            "unit_get_unknown23E",
            index,
            random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0, 1, 5)),
            globals=some(),
        )
        t.check("unit_get_unknown348", index, globals=some())
        t.check("unit_get_unknown39D", index, globals=some())
