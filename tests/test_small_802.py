# nine small functions: a name in a block of a tag, a counter and its time,
# objects of the kind 0 with 6 at 0x3DC, two things that get points, the
# oldest of four entries and a bit of two longs

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x400
NAMES = (0, 0x1234, 0x5678, 0x9ABC)


def run(t) -> None:
    random_ = random.Random(802)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def time_globals() -> Buffer:
        data = bytearray(random_.randbytes(0x24))
        struct.pack_into("<I", data, 4, f32(random_.choice((1 / 30, 1 / 60, 0.5))))
        struct.pack_into("<I", data, 8, random_.getrandbits(31))
        return Buffer(bytes(data))

    for n in range(400):
        tags = bytearray(random_.randbytes(0x40 + 0x60 + 4 * 0x0C))
        struct.pack_into("<I", tags, 8, 0x40)
        struct.pack_into("<iI", tags, 0x40 + 0x58, random_.choice((0, 1, 4, -1)), 0xA0)
        for element in range(4):
            struct.pack_into("<I", tags, 0xA0 + element * 0x0C, random_.choice(NAMES))
        t.check(
            "tag_unknown_001d8f00",
            random_.choice((NONE, random_.getrandbits(16) << 16)),
            random_.choice(NAMES + (NONE,)),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x40 + 0x5C))},
        )

        counter = bytes((random_.choice((0, 1, 2, 5, 9, 0x7F, 0x80, 0xFF)), random_.choice((0, 1, 5, 10, 0x7F, 0xFF)), random_.getrandbits(8), random_.choice((0, 1, 2, 3, 0xFC, 0xFD))))
        t.check("unit_counter_update", Buffer(counter))
        t.check("unit_counter_seconds", Buffer(counter), globals={"game_time_globals": time_globals()})

        table = some_objects(random_)
        headers = bytearray(table.data[:FIRST])
        objects = bytearray()
        for index in range(COUNT):
            headers[0x4C + index * 12 + 3] = random_.choice((0, 0, 1, 32))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            one = bytearray(random_.randbytes(BIG))
            one[0x3DC] = random_.choice((6, 6, 5))
            objects += one
        globals_ = lambda: {"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers)}
        t.check("unit_unknown_001dac00", salted(random_.randrange(COUNT)), globals=globals_())
        t.check("unit_unknown_001dac40", salted(random_.randrange(COUNT)), salted(random_.randrange(COUNT)), globals=globals_())

        source = bytearray(random_.randbytes(0x20))
        source[0x1D] = random_.choice((0, 1))
        t.check(
            "unit_unknown_001e67a0",
            Buffer(bytes(source)),
            Buffer(random_.randbytes(0xBC)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"global_up3d": Buffer(random_.randbytes(12))},
        )
        t.check(
            "unit_unknown_001e6850",
            Buffer(random_.randbytes(0xDC)),
            Buffer(random_.randbytes(12)),
            random_.getrandbits(32),
            f32(random_.uniform(-3, 3)),
            Buffer(random_.randbytes(12)),
        )

        entries = struct.pack("<8i", *(random_.choice((0, 5, 5, -1, random_.getrandbits(20))) for _ in range(8)))
        t.check(
            "unit_unknown_001e6980",
            Buffer(entries),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"game_time_globals": time_globals()},
        )
        t.check("unit_unknown_001e6a00", random_.getrandbits(32), random_.choice((0, 5, 31, 32, 0xFF)), Buffer(random_.randbytes(8)))
