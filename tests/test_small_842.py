# two functions of the way to the main menu: the objects of a list with a
# count in their tag go first, and how busy Havok is

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
DEFINITION = 0x80


def run(t) -> None:
    random_ = random.Random(842)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT, random_.choice((NONE, salted(0), salted(1), salted(2))))
        tags = bytearray(random_.randbytes(0x30 + 3 * DEFINITION))
        for tag in range(3):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x30 + tag * DEFINITION)
            at = 0x30 + tag * DEFINITION
            struct.pack_into("<I", tags, at + 4, random_.choice((NONE, salted(random_.randrange(3)))))
            struct.pack_into("<I", tags, at + 0x38, random_.choice((NONE, salted(random_.randrange(3)))))
            struct.pack_into("<i", tags, at + 0x74, random_.choice((0, 1, 2, -1)))
        count = random_.randrange(6)
        # the owner, its list (0x18 bytes) and the list's four arrays for six
        owner = bytearray(random_.randbytes(0x18 + 0x18 + 6 * 12))
        struct.pack_into("<I", owner, 0x14, 0x18)
        struct.pack_into("<iHh", owner, 0x18, 6, count, 0)
        arrays = 0x30
        struct.pack_into("<IIII", owner, 0x18 + 8, arrays, arrays + 12, arrays + 36, arrays + 60)
        for slot in range(6):
            struct.pack_into("<I", owner, arrays + 12 + slot * 4, random_.choice((NONE, 0, salted(random_.randrange(COUNT)))))
        t.check(
            "game_engine_unknown_00134240",
            Buffer(bytes(owner), pointers=(0x14, 0x20, 0x24, 0x28, 0x2C)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18, 0x28))},
        )

        def thing() -> object:
            data = bytearray(random_.randbytes(0x24))
            struct.pack_into("<i", data, 0x20, random_.choice((0, 1, -1)))
            return random_.choice((0, Buffer(bytes(data))))

        big = bytearray(0xF0400)
        struct.pack_into("<i", big, 0xF0340, random_.choice((0, 50, 94, 95, 200)))
        big[0x60:0xB0] = random_.randbytes(0x50)
        big[0xF0340 + 8:0xF03D0] = bytes(random_.randrange(256) if random_.random() < 0.3 else 0 for _ in range(0x88))
        t.check(
            "havok_unknown_00146fd0",
            globals={
                "unknown_004798a0": thing(),
                "unknown_0047989c": thing(),
                "unknown_00479898": random_.choice((0, Buffer(bytes(big)), Buffer(bytes(big)))),
            },
        )
